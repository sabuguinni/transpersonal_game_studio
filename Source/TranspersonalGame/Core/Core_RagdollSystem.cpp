#include "Core_RagdollSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UCore_RagdollSystem::UCore_RagdollSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Default Configuration
    TransitionTime = 0.5f;
    RecoveryTime = 2.0f;
    bAutoRecover = true;
    MinVelocityForRagdoll = 500.0f;
    MaxRagdollTime = 10.0f;
    
    // Initial State
    CurrentState = ECore_RagdollState::Disabled;
    StateTimer = 0.0f;
    bIsRagdollActive = false;
    RagdollTimer = 0.0f;
    LastImpactForce = 0.0f;
    LastImpactLocation = FVector::ZeroVector;
    
    // Component References
    SkeletalMeshComponent = nullptr;
    CachedAnimInstance = nullptr;
}

void UCore_RagdollSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Find skeletal mesh component
    AActor* Owner = GetOwner();
    if (Owner)
    {
        SkeletalMeshComponent = Owner->FindComponentByClass<USkeletalMeshComponent>();
        if (SkeletalMeshComponent)
        {
            CachedAnimInstance = SkeletalMeshComponent->GetAnimInstance();
            InitializeRagdollBones();
            
            UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Initialized for %s"), *Owner->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Core_RagdollSystem: No SkeletalMeshComponent found on %s"), *Owner->GetName());
        }
    }
}

void UCore_RagdollSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (CurrentState != ECore_RagdollState::Disabled)
    {
        UpdateRagdollState(DeltaTime);
    }
    
    // Update ragdoll timer
    if (bIsRagdollActive)
    {
        RagdollTimer += DeltaTime;
        
        // Auto-recovery after max time
        if (bAutoRecover && RagdollTimer >= MaxRagdollTime)
        {
            StartRecovery();
        }
    }
}

void UCore_RagdollSystem::ActivateRagdoll(float Force, FVector ImpactLocation)
{
    if (!ValidateSkeletalMesh() || bIsRagdollActive)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Activating ragdoll with force %f at location %s"), 
           Force, *ImpactLocation.ToString());
    
    // Save current pose
    SaveCurrentPose();
    
    // Store impact data
    LastImpactForce = Force;
    LastImpactLocation = ImpactLocation;
    
    // Disable character movement
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
        {
            Movement->SetMovementMode(MOVE_None);
        }
    }
    
    // Enable physics simulation
    SkeletalMeshComponent->SetSimulatePhysics(true);
    SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    
    // Apply initial impact force
    if (Force > 0.0f && ImpactLocation != FVector::ZeroVector)
    {
        ApplyImpactForce(FVector(Force, 0, 0), ImpactLocation);
    }
    
    // Set state
    SetRagdollState(ECore_RagdollState::Active);
    bIsRagdollActive = true;
    RagdollTimer = 0.0f;
}

void UCore_RagdollSystem::DeactivateRagdoll()
{
    if (!bIsRagdollActive)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Deactivating ragdoll"));
    
    // Disable physics simulation
    if (SkeletalMeshComponent)
    {
        SkeletalMeshComponent->SetSimulatePhysics(false);
        SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    }
    
    // Restore character movement
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
        {
            Movement->SetMovementMode(MOVE_Walking);
        }
    }
    
    // Restore animation
    RestoreSavedPose();
    
    // Reset state
    SetRagdollState(ECore_RagdollState::Disabled);
    bIsRagdollActive = false;
    RagdollTimer = 0.0f;
}

void UCore_RagdollSystem::SetRagdollState(ECore_RagdollState NewState)
{
    if (CurrentState != NewState)
    {
        CurrentState = NewState;
        StateTimer = 0.0f;
        
        UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: State changed to %d"), (int32)NewState);
    }
}

bool UCore_RagdollSystem::IsRagdollActive() const
{
    return bIsRagdollActive;
}

void UCore_RagdollSystem::ConfigureBonePhysics(FName BoneName, float Mass, float LinearDamping, float AngularDamping)
{
    if (!ValidateSkeletalMesh())
    {
        return;
    }
    
    // Find or create bone configuration
    FCore_RagdollBone* ExistingBone = RagdollBones.FindByPredicate([BoneName](const FCore_RagdollBone& Bone)
    {
        return Bone.BoneName == BoneName;
    });
    
    if (ExistingBone)
    {
        ExistingBone->Mass = Mass;
        ExistingBone->LinearDamping = LinearDamping;
        ExistingBone->AngularDamping = AngularDamping;
    }
    else
    {
        FCore_RagdollBone NewBone;
        NewBone.BoneName = BoneName;
        NewBone.Mass = Mass;
        NewBone.LinearDamping = LinearDamping;
        NewBone.AngularDamping = AngularDamping;
        RagdollBones.Add(NewBone);
    }
    
    // Apply to skeletal mesh if ragdoll is active
    if (bIsRagdollActive)
    {
        SkeletalMeshComponent->SetBoneLinearDamping(BoneName, LinearDamping);
        SkeletalMeshComponent->SetBoneAngularDamping(BoneName, AngularDamping);
    }
}

void UCore_RagdollSystem::SetupDefaultRagdollBones()
{
    RagdollBones.Empty();
    
    // Default bone setup for humanoid characters
    ConfigureBonePhysics(TEXT("pelvis"), 10.0f, 0.1f, 0.1f);
    ConfigureBonePhysics(TEXT("spine_01"), 5.0f, 0.15f, 0.15f);
    ConfigureBonePhysics(TEXT("spine_02"), 4.0f, 0.15f, 0.15f);
    ConfigureBonePhysics(TEXT("spine_03"), 3.0f, 0.15f, 0.15f);
    ConfigureBonePhysics(TEXT("head"), 2.0f, 0.2f, 0.2f);
    
    // Arms
    ConfigureBonePhysics(TEXT("upperarm_l"), 3.0f, 0.1f, 0.1f);
    ConfigureBonePhysics(TEXT("lowerarm_l"), 2.0f, 0.1f, 0.1f);
    ConfigureBonePhysics(TEXT("hand_l"), 1.0f, 0.2f, 0.2f);
    ConfigureBonePhysics(TEXT("upperarm_r"), 3.0f, 0.1f, 0.1f);
    ConfigureBonePhysics(TEXT("lowerarm_r"), 2.0f, 0.1f, 0.1f);
    ConfigureBonePhysics(TEXT("hand_r"), 1.0f, 0.2f, 0.2f);
    
    // Legs
    ConfigureBonePhysics(TEXT("thigh_l"), 8.0f, 0.1f, 0.1f);
    ConfigureBonePhysics(TEXT("calf_l"), 4.0f, 0.1f, 0.1f);
    ConfigureBonePhysics(TEXT("foot_l"), 2.0f, 0.15f, 0.15f);
    ConfigureBonePhysics(TEXT("thigh_r"), 8.0f, 0.1f, 0.1f);
    ConfigureBonePhysics(TEXT("calf_r"), 4.0f, 0.1f, 0.1f);
    ConfigureBonePhysics(TEXT("foot_r"), 2.0f, 0.15f, 0.15f);
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Default ragdoll bones configured"));
}

void UCore_RagdollSystem::CreatePhysicsConstraints()
{
    if (!ValidateSkeletalMesh())
    {
        return;
    }
    
    CleanupPhysicsConstraints();
    
    // Create constraints based on configuration
    for (const FCore_RagdollConstraint& ConstraintConfig : RagdollConstraints)
    {
        UPhysicsConstraintComponent* Constraint = NewObject<UPhysicsConstraintComponent>(GetOwner());
        if (Constraint)
        {
            Constraint->AttachToComponent(SkeletalMeshComponent, FAttachmentTransformRules::KeepWorldTransform);
            
            // Configure constraint
            Constraint->SetConstrainedComponents(SkeletalMeshComponent, ConstraintConfig.ParentBone,
                                               SkeletalMeshComponent, ConstraintConfig.ChildBone);
            
            // Set limits
            Constraint->SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Limited, ConstraintConfig.AngularSwing1Limit);
            Constraint->SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Limited, ConstraintConfig.AngularSwing2Limit);
            Constraint->SetAngularTwistLimit(EAngularConstraintMotion::ACM_Limited, ConstraintConfig.AngularTwistLimit);
            
            // Set break forces
            Constraint->SetLinearBreakable(true, ConstraintConfig.LinearBreakForce);
            Constraint->SetAngularBreakable(true, ConstraintConfig.AngularBreakTorque);
            
            PhysicsConstraints.Add(Constraint);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Created %d physics constraints"), PhysicsConstraints.Num());
}

void UCore_RagdollSystem::ApplyImpactForce(FVector Force, FVector Location, FName BoneName)
{
    if (!ValidateSkeletalMesh() || !bIsRagdollActive)
    {
        return;
    }
    
    if (BoneName != NAME_None)
    {
        SkeletalMeshComponent->AddImpulseAtLocation(Force, Location, BoneName);
    }
    else
    {
        SkeletalMeshComponent->AddImpulseAtLocation(Force, Location);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Applied impact force %s at %s"), 
           *Force.ToString(), *Location.ToString());
}

void UCore_RagdollSystem::ApplyRadialImpulse(FVector Origin, float Radius, float Strength)
{
    if (!ValidateSkeletalMesh() || !bIsRagdollActive)
    {
        return;
    }
    
    SkeletalMeshComponent->AddRadialImpulse(Origin, Radius, Strength, RIF_Linear, true);
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Applied radial impulse at %s with radius %f and strength %f"), 
           *Origin.ToString(), Radius, Strength);
}

FVector UCore_RagdollSystem::GetBoneVelocity(FName BoneName) const
{
    if (!ValidateSkeletalMesh())
    {
        return FVector::ZeroVector;
    }
    
    return SkeletalMeshComponent->GetBoneLinearVelocity(BoneName);
}

void UCore_RagdollSystem::StartRecovery()
{
    if (CurrentState == ECore_RagdollState::Active)
    {
        SetRagdollState(ECore_RagdollState::Recovering);
        UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Starting recovery"));
    }
}

bool UCore_RagdollSystem::CanRecover() const
{
    if (!bIsRagdollActive)
    {
        return false;
    }
    
    // Check if character is relatively stable
    FVector RootVelocity = GetBoneVelocity(TEXT("pelvis"));
    return RootVelocity.Size() < MinVelocityForRagdoll;
}

void UCore_RagdollSystem::BlendToAnimation(float BlendTime)
{
    if (!ValidateSkeletalMesh() || !CachedAnimInstance)
    {
        return;
    }
    
    // Gradually reduce physics influence
    SkeletalMeshComponent->SetAllBodiesPhysicsBlendWeight(0.0f, false, true);
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Blending to animation over %f seconds"), BlendTime);
}

void UCore_RagdollSystem::SaveCurrentPose()
{
    if (!ValidateSkeletalMesh())
    {
        return;
    }
    
    SavedBoneTransforms.Empty();
    
    // Save transforms for all bones
    for (const FCore_RagdollBone& Bone : RagdollBones)
    {
        FTransform BoneTransform = SkeletalMeshComponent->GetBoneTransform(SkeletalMeshComponent->GetBoneIndex(Bone.BoneName));
        SavedBoneTransforms.Add(Bone.BoneName, BoneTransform);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Saved pose with %d bones"), SavedBoneTransforms.Num());
}

void UCore_RagdollSystem::RestoreSavedPose()
{
    if (!ValidateSkeletalMesh() || SavedBoneTransforms.Num() == 0)
    {
        return;
    }
    
    // Restore bone transforms
    for (const auto& BonePair : SavedBoneTransforms)
    {
        int32 BoneIndex = SkeletalMeshComponent->GetBoneIndex(BonePair.Key);
        if (BoneIndex != INDEX_NONE)
        {
            SkeletalMeshComponent->SetBoneTransformByName(BonePair.Key, BonePair.Value, EBoneSpaces::WorldSpace);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Restored saved pose"));
}

float UCore_RagdollSystem::GetRagdollIntensity() const
{
    if (!bIsRagdollActive)
    {
        return 0.0f;
    }
    
    // Calculate intensity based on velocity and time
    FVector RootVelocity = GetBoneVelocity(TEXT("pelvis"));
    float VelocityFactor = FMath::Clamp(RootVelocity.Size() / 1000.0f, 0.0f, 1.0f);
    float TimeFactor = FMath::Clamp(RagdollTimer / MaxRagdollTime, 0.0f, 1.0f);
    
    return VelocityFactor * (1.0f - TimeFactor);
}

void UCore_RagdollSystem::UpdateRagdollState(float DeltaTime)
{
    StateTimer += DeltaTime;
    
    switch (CurrentState)
    {
        case ECore_RagdollState::Transitioning:
            ProcessTransition(DeltaTime);
            break;
            
        case ECore_RagdollState::Active:
            // Check for auto-recovery conditions
            if (bAutoRecover && CanRecover() && RagdollTimer > RecoveryTime)
            {
                StartRecovery();
            }
            break;
            
        case ECore_RagdollState::Recovering:
            ProcessRecovery(DeltaTime);
            break;
            
        default:
            break;
    }
}

void UCore_RagdollSystem::ProcessTransition(float DeltaTime)
{
    if (StateTimer >= TransitionTime)
    {
        SetRagdollState(ECore_RagdollState::Active);
    }
}

void UCore_RagdollSystem::ProcessRecovery(float DeltaTime)
{
    if (StateTimer >= RecoveryTime)
    {
        DeactivateRagdoll();
    }
    else
    {
        // Gradually blend back to animation
        float BlendAlpha = StateTimer / RecoveryTime;
        SkeletalMeshComponent->SetAllBodiesPhysicsBlendWeight(1.0f - BlendAlpha, false, true);
    }
}

void UCore_RagdollSystem::InitializeRagdollBones()
{
    if (RagdollBones.Num() == 0)
    {
        SetupDefaultRagdollBones();
    }
    
    CreatePhysicsConstraints();
}

void UCore_RagdollSystem::CleanupPhysicsConstraints()
{
    for (UPhysicsConstraintComponent* Constraint : PhysicsConstraints)
    {
        if (Constraint)
        {
            Constraint->DestroyComponent();
        }
    }
    PhysicsConstraints.Empty();
}

bool UCore_RagdollSystem::ValidateSkeletalMesh() const
{
    return SkeletalMeshComponent != nullptr && SkeletalMeshComponent->GetSkeletalMeshAsset() != nullptr;
}