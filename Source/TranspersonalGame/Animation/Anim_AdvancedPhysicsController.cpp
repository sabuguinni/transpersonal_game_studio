#include "Anim_AdvancedPhysicsController.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Animation/AnimBlueprint.h"

UAnim_AdvancedPhysicsController::UAnim_AdvancedPhysicsController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
    
    // Initialize default physics settings
    PhysicsSettings = FAnim_PhysicsSettings();
    CurrentPhysicsState = EAnim_PhysicsState::Disabled;
    bIsRagdollActive = false;
    CurrentRagdollTime = 0.0f;
    PhysicsBlendAlpha = 0.0f;
    
    // Initialize impact thresholds
    ImpactThresholds.Add(EAnim_ImpactType::Light, 200.0f);
    ImpactThresholds.Add(EAnim_ImpactType::Medium, 500.0f);
    ImpactThresholds.Add(EAnim_ImpactType::Heavy, 1000.0f);
    ImpactThresholds.Add(EAnim_ImpactType::Extreme, 2000.0f);
    ImpactThresholds.Add(EAnim_ImpactType::Environmental, 1500.0f);
    
    // Initialize critical bones for prehistoric character
    CriticalBones.Add(TEXT("spine_01"));
    CriticalBones.Add(TEXT("spine_02"));
    CriticalBones.Add(TEXT("spine_03"));
    CriticalBones.Add(TEXT("neck_01"));
    CriticalBones.Add(TEXT("head"));
    
    // Initialize partial physics bones
    PartialPhysicsBones.Add(TEXT("upperarm_l"));
    PartialPhysicsBones.Add(TEXT("upperarm_r"));
    PartialPhysicsBones.Add(TEXT("lowerarm_l"));
    PartialPhysicsBones.Add(TEXT("lowerarm_r"));
    PartialPhysicsBones.Add(TEXT("thigh_l"));
    PartialPhysicsBones.Add(TEXT("thigh_r"));
    PartialPhysicsBones.Add(TEXT("calf_l"));
    PartialPhysicsBones.Add(TEXT("calf_r"));
}

void UAnim_AdvancedPhysicsController::BeginPlay()
{
    Super::BeginPlay();
    
    CacheComponentReferences();
    InitializePhysicsSettings();
    SetupBoneConstraints();
}

void UAnim_AdvancedPhysicsController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!SkeletalMeshComponent || !OwnerCharacter)
    {
        return;
    }
    
    UpdatePhysicsBlending(DeltaTime);
    UpdateRagdollTimer(DeltaTime);
    
    // Auto-recovery check
    if (bIsRagdollActive && PhysicsSettings.bAutoRecover && CanRecover())
    {
        if (CurrentRagdollTime >= PhysicsSettings.MinRagdollTime)
        {
            StartRecovery();
        }
    }
}

void UAnim_AdvancedPhysicsController::CacheComponentReferences()
{
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        SkeletalMeshComponent = OwnerCharacter->GetMesh();
        MovementComponent = OwnerCharacter->GetCharacterMovement();
    }
}

void UAnim_AdvancedPhysicsController::InitializePhysicsSettings()
{
    if (!SkeletalMeshComponent)
    {
        return;
    }
    
    // Set initial physics state
    SkeletalMeshComponent->SetSimulatePhysics(false);
    SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    
    // Configure physics asset if available
    if (UPhysicsAsset* PhysicsAsset = SkeletalMeshComponent->GetPhysicsAsset())
    {
        // Enable physics simulation for specific bones
        for (const FName& BoneName : PartialPhysicsBones)
        {
            int32 BoneIndex = SkeletalMeshComponent->GetBoneIndex(BoneName);
            if (BoneIndex != INDEX_NONE)
            {
                SkeletalMeshComponent->SetAllBodiesBelowSimulatePhysics(BoneName, false);
            }
        }
    }
}

void UAnim_AdvancedPhysicsController::SetupBoneConstraints()
{
    if (!SkeletalMeshComponent || !bConstrainCriticalBones)
    {
        return;
    }
    
    // Setup constraints for critical bones to prevent excessive movement
    for (const FName& BoneName : CriticalBones)
    {
        int32 BoneIndex = SkeletalMeshComponent->GetBoneIndex(BoneName);
        if (BoneIndex != INDEX_NONE)
        {
            // Apply constraints to limit movement range
            SkeletalMeshComponent->SetConstraintProfileForAll(TEXT("Constrained"), true);
        }
    }
}

void UAnim_AdvancedPhysicsController::EnableRagdoll(bool bImmediate)
{
    if (!SkeletalMeshComponent || !OwnerCharacter)
    {
        return;
    }
    
    if (bIsRagdollActive)
    {
        return;
    }
    
    bIsRagdollActive = true;
    CurrentRagdollTime = 0.0f;
    
    // Disable character movement
    if (MovementComponent)
    {
        MovementComponent->SetMovementMode(MOVE_None);
        MovementComponent->StopMovementImmediately();
    }
    
    // Disable capsule collision
    if (UCapsuleComponent* CapsuleComp = OwnerCharacter->GetCapsuleComponent())
    {
        CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    
    // Enable physics simulation
    SkeletalMeshComponent->SetSimulatePhysics(true);
    SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    SkeletalMeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
    
    // Set physics state
    EAnim_PhysicsState NewState = bImmediate ? EAnim_PhysicsState::FullRagdoll : EAnim_PhysicsState::Blending;
    SetPhysicsState(NewState);
    
    if (!bImmediate)
    {
        BlendToPhysics(PhysicsSettings.RagdollBlendTime);
    }
    else
    {
        PhysicsBlendAlpha = 1.0f;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Ragdoll enabled for character: %s"), *OwnerCharacter->GetName());
}

void UAnim_AdvancedPhysicsController::DisableRagdoll(bool bImmediate)
{
    if (!SkeletalMeshComponent || !OwnerCharacter || !bIsRagdollActive)
    {
        return;
    }
    
    bIsRagdollActive = false;
    
    // Re-enable character movement
    if (MovementComponent)
    {
        MovementComponent->SetMovementMode(MOVE_Walking);
    }
    
    // Re-enable capsule collision
    if (UCapsuleComponent* CapsuleComp = OwnerCharacter->GetCapsuleComponent())
    {
        CapsuleComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }
    
    // Disable physics simulation
    SkeletalMeshComponent->SetSimulatePhysics(false);
    SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    
    if (!bImmediate)
    {
        SetPhysicsState(EAnim_PhysicsState::Recovery);
        BlendToAnimation(PhysicsSettings.RecoveryBlendTime);
    }
    else
    {
        SetPhysicsState(EAnim_PhysicsState::Disabled);
        PhysicsBlendAlpha = 0.0f;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Ragdoll disabled for character: %s"), *OwnerCharacter->GetName());
}

void UAnim_AdvancedPhysicsController::SetPhysicsState(EAnim_PhysicsState NewState)
{
    if (CurrentPhysicsState != NewState)
    {
        EAnim_PhysicsState PreviousState = CurrentPhysicsState;
        CurrentPhysicsState = NewState;
        
        OnPhysicsStateChanged.Broadcast(NewState);
        
        UE_LOG(LogTemp, Log, TEXT("Physics state changed from %d to %d"), 
               static_cast<int32>(PreviousState), static_cast<int32>(NewState));
    }
}

void UAnim_AdvancedPhysicsController::BlendToPhysics(float BlendTime)
{
    if (BlendTime <= 0.0f)
    {
        PhysicsBlendAlpha = 1.0f;
        bIsBlending = false;
        return;
    }
    
    TargetBlendAlpha = 1.0f;
    BlendSpeed = 1.0f / BlendTime;
    bIsBlending = true;
}

void UAnim_AdvancedPhysicsController::BlendToAnimation(float BlendTime)
{
    if (BlendTime <= 0.0f)
    {
        PhysicsBlendAlpha = 0.0f;
        bIsBlending = false;
        return;
    }
    
    TargetBlendAlpha = 0.0f;
    BlendSpeed = 1.0f / BlendTime;
    bIsBlending = true;
}

void UAnim_AdvancedPhysicsController::UpdatePhysicsBlending(float DeltaTime)
{
    if (!bIsBlending)
    {
        return;
    }
    
    float BlendDirection = (TargetBlendAlpha > PhysicsBlendAlpha) ? 1.0f : -1.0f;
    PhysicsBlendAlpha += BlendDirection * BlendSpeed * DeltaTime;
    
    // Clamp to target
    if (BlendDirection > 0.0f && PhysicsBlendAlpha >= TargetBlendAlpha)
    {
        PhysicsBlendAlpha = TargetBlendAlpha;
        bIsBlending = false;
        
        if (PhysicsBlendAlpha >= 1.0f)
        {
            SetPhysicsState(EAnim_PhysicsState::FullRagdoll);
        }
    }
    else if (BlendDirection < 0.0f && PhysicsBlendAlpha <= TargetBlendAlpha)
    {
        PhysicsBlendAlpha = TargetBlendAlpha;
        bIsBlending = false;
        
        if (PhysicsBlendAlpha <= 0.0f)
        {
            SetPhysicsState(EAnim_PhysicsState::Disabled);
        }
    }
    
    // Update physics settings blend weight
    PhysicsSettings.PhysicsBlendWeight = PhysicsBlendAlpha;
}

void UAnim_AdvancedPhysicsController::UpdateRagdollTimer(float DeltaTime)
{
    if (bIsRagdollActive)
    {
        CurrentRagdollTime += DeltaTime;
        
        // Force recovery if max time exceeded
        if (CurrentRagdollTime >= PhysicsSettings.MaxRagdollTime)
        {
            StartRecovery();
        }
    }
}

void UAnim_AdvancedPhysicsController::ProcessImpact(const FAnim_ImpactData& ImpactData)
{
    LastImpactData = ImpactData;
    LastImpactTime = GetWorld()->GetTimeSeconds();
    
    OnImpactReceived.Broadcast(ImpactData);
    
    // Determine if ragdoll should be triggered
    if (ShouldTriggerRagdoll(ImpactData))
    {
        EnableRagdoll(false);
        
        // Apply impact force
        if (SkeletalMeshComponent && ImpactData.ImpactForce > 0.0f)
        {
            FVector ForceToApply = ImpactData.ImpactVelocity.GetSafeNormal() * ImpactData.ImpactForce;
            
            if (ImpactData.BoneName != NAME_None)
            {
                SkeletalMeshComponent->AddImpulseAtLocation(ForceToApply, ImpactData.ImpactLocation, ImpactData.BoneName);
            }
            else
            {
                SkeletalMeshComponent->AddImpulseAtLocation(ForceToApply, ImpactData.ImpactLocation);
            }
        }
    }
    else
    {
        // Play impact reaction animation if available
        if (ImpactReactionMontage && OwnerCharacter)
        {
            if (UAnimInstance* AnimInstance = SkeletalMeshComponent->GetAnimInstance())
            {
                AnimInstance->Montage_Play(ImpactReactionMontage);
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Impact processed: Force=%.2f, Type=%d, ShouldRagdoll=%s"), 
           ImpactData.ImpactForce, static_cast<int32>(ImpactData.ImpactType), 
           ImpactData.bShouldTriggerRagdoll ? TEXT("Yes") : TEXT("No"));
}

void UAnim_AdvancedPhysicsController::ApplyImpactForce(FVector Force, FVector Location, FName BoneName)
{
    if (!SkeletalMeshComponent)
    {
        return;
    }
    
    FAnim_ImpactData ImpactData;
    ImpactData.ImpactLocation = Location;
    ImpactData.ImpactVelocity = Force.GetSafeNormal();
    ImpactData.ImpactForce = Force.Size();
    ImpactData.BoneName = BoneName;
    ImpactData.ImpactType = CalculateImpactType(ImpactData.ImpactForce);
    ImpactData.bShouldTriggerRagdoll = ShouldTriggerRagdoll(ImpactData);
    
    ProcessImpact(ImpactData);
}

bool UAnim_AdvancedPhysicsController::ShouldTriggerRagdoll(const FAnim_ImpactData& ImpactData) const
{
    if (ImpactData.bShouldTriggerRagdoll)
    {
        return true;
    }
    
    // Check impact force threshold
    if (ImpactData.ImpactForce >= PhysicsSettings.ImpactThreshold)
    {
        return true;
    }
    
    // Check impact type thresholds
    if (const float* Threshold = ImpactThresholds.Find(ImpactData.ImpactType))
    {
        return ImpactData.ImpactForce >= *Threshold;
    }
    
    return false;
}

EAnim_ImpactType UAnim_AdvancedPhysicsController::CalculateImpactType(float ImpactForce) const
{
    if (ImpactForce >= 2000.0f)
    {
        return EAnim_ImpactType::Extreme;
    }
    else if (ImpactForce >= 1000.0f)
    {
        return EAnim_ImpactType::Heavy;
    }
    else if (ImpactForce >= 500.0f)
    {
        return EAnim_ImpactType::Medium;
    }
    else
    {
        return EAnim_ImpactType::Light;
    }
}

void UAnim_AdvancedPhysicsController::StartRecovery()
{
    if (!bIsRagdollActive)
    {
        return;
    }
    
    SetPhysicsState(EAnim_PhysicsState::Recovery);
    
    // Play recovery animation
    UAnimMontage* RecoveryMontage = GetRecoveryMontage();
    if (RecoveryMontage && SkeletalMeshComponent)
    {
        if (UAnimInstance* AnimInstance = SkeletalMeshComponent->GetAnimInstance())
        {
            AnimInstance->Montage_Play(RecoveryMontage);
        }
    }
    
    // Start blending back to animation
    DisableRagdoll(false);
    
    OnRagdollRecovery.Broadcast();
    
    UE_LOG(LogTemp, Log, TEXT("Recovery started for character: %s"), 
           OwnerCharacter ? *OwnerCharacter->GetName() : TEXT("Unknown"));
}

bool UAnim_AdvancedPhysicsController::CanRecover() const
{
    if (!OwnerCharacter || !SkeletalMeshComponent)
    {
        return false;
    }
    
    // Check if character is relatively stable (low velocity)
    FVector Velocity = SkeletalMeshComponent->GetPhysicsLinearVelocity();
    if (Velocity.Size() > 100.0f) // Still moving too fast
    {
        return false;
    }
    
    // Check if character is close to ground
    float GroundDistance = CalculateGroundDistance();
    if (GroundDistance > 50.0f) // Too far from ground
    {
        return false;
    }
    
    return true;
}

UAnimMontage* UAnim_AdvancedPhysicsController::GetRecoveryMontage() const
{
    bool bFacingUp = IsCharacterFacingUp();
    
    if (bFacingUp && GetUpFromBackMontage)
    {
        return GetUpFromBackMontage;
    }
    else if (!bFacingUp && GetUpFromFrontMontage)
    {
        return GetUpFromFrontMontage;
    }
    
    // Fallback to any available recovery animation
    if (RecoveryAnimations.Num() > 0)
    {
        // Convert AnimSequence to Montage if needed (simplified)
        return GetUpFromBackMontage; // Fallback
    }
    
    return nullptr;
}

bool UAnim_AdvancedPhysicsController::IsCharacterFacingUp() const
{
    if (!SkeletalMeshComponent)
    {
        return true;
    }
    
    FVector CharacterUp = GetCharacterUpVector();
    FVector WorldUp = FVector::UpVector;
    
    float DotProduct = FVector::DotProduct(CharacterUp, WorldUp);
    return DotProduct > 0.0f; // Facing up if dot product is positive
}

FVector UAnim_AdvancedPhysicsController::GetCharacterUpVector() const
{
    if (!SkeletalMeshComponent)
    {
        return FVector::UpVector;
    }
    
    // Get the spine bone transform to determine character orientation
    FName SpineBone = TEXT("spine_02");
    int32 BoneIndex = SkeletalMeshComponent->GetBoneIndex(SpineBone);
    
    if (BoneIndex != INDEX_NONE)
    {
        FTransform BoneTransform = SkeletalMeshComponent->GetBoneTransform(BoneIndex);
        return BoneTransform.GetUnitAxis(EAxis::Z);
    }
    
    return SkeletalMeshComponent->GetUpVector();
}

float UAnim_AdvancedPhysicsController::CalculateGroundDistance() const
{
    if (!OwnerCharacter)
    {
        return 0.0f;
    }
    
    FVector StartLocation = OwnerCharacter->GetActorLocation();
    FVector EndLocation = StartLocation - FVector(0.0f, 0.0f, 1000.0f);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_WorldStatic, QueryParams))
    {
        return HitResult.Distance;
    }
    
    return 1000.0f; // Max distance if no ground found
}

void UAnim_AdvancedPhysicsController::SetBonePhysicsState(FName BoneName, bool bSimulatePhysics)
{
    if (!SkeletalMeshComponent)
    {
        return;
    }
    
    SkeletalMeshComponent->SetAllBodiesBelowSimulatePhysics(BoneName, bSimulatePhysics);
}

void UAnim_AdvancedPhysicsController::SetPartialPhysics(const TArray<FName>& BoneNames)
{
    if (!SkeletalMeshComponent)
    {
        return;
    }
    
    // Disable all physics first
    SkeletalMeshComponent->SetSimulatePhysics(false);
    
    // Enable physics for specified bones
    for (const FName& BoneName : BoneNames)
    {
        SetBonePhysicsState(BoneName, true);
    }
    
    SetPhysicsState(EAnim_PhysicsState::Partial);
}

void UAnim_AdvancedPhysicsController::ConstrainBone(FName BoneName, bool bConstrain)
{
    if (!SkeletalMeshComponent)
    {
        return;
    }
    
    int32 BoneIndex = SkeletalMeshComponent->GetBoneIndex(BoneName);
    if (BoneIndex != INDEX_NONE)
    {
        if (bConstrain)
        {
            SkeletalMeshComponent->SetConstraintProfile(BoneName, TEXT("Constrained"), true);
        }
        else
        {
            SkeletalMeshComponent->SetConstraintProfile(BoneName, TEXT(""), true);
        }
    }
}