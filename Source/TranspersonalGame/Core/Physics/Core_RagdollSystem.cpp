#include "Core_RagdollSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Animation/AnimInstance.h"
#include "Kismet/KismetMathLibrary.h"

UCore_RagdollSystem::UCore_RagdollSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize default values
    RagdollState = ECore_RagdollState::Inactive;
    RagdollTime = 0.0f;
    MaxRagdollTime = 10.0f;
    ImpactForceMultiplier = 1.0f;
    MinImpactForce = 500.0f;
    LinearDamping = 0.1f;
    AngularDamping = 0.1f;
    RagdollBlendWeight = 0.0f;
    TransitionSpeed = 2.0f;
    bUsePartialRagdoll = false;
    bCanRecoverFromRagdoll = true;
    StabilityTimeRequired = 2.0f;
    MaxStableVelocity = 50.0f;
    StabilityTimer = 0.0f;
    TargetMesh = nullptr;
    RagdollPhysicsAsset = nullptr;
    OriginalAnimInstance = nullptr;

    // Initialize default bone force multipliers for human-like skeleton
    BoneForceMultipliers.Add(TEXT("head"), 0.5f);
    BoneForceMultipliers.Add(TEXT("neck"), 0.7f);
    BoneForceMultipliers.Add(TEXT("spine"), 1.0f);
    BoneForceMultipliers.Add(TEXT("pelvis"), 1.5f);
    BoneForceMultipliers.Add(TEXT("thigh"), 1.2f);
    BoneForceMultipliers.Add(TEXT("calf"), 1.0f);
    BoneForceMultipliers.Add(TEXT("upperarm"), 0.8f);
    BoneForceMultipliers.Add(TEXT("forearm"), 0.6f);
}

void UCore_RagdollSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Auto-detect skeletal mesh component if not set
    if (!TargetMesh)
    {
        if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
        {
            TargetMesh = OwnerCharacter->GetMesh();
        }
        else
        {
            TargetMesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
        }
    }
    
    if (TargetMesh)
    {
        InitializeRagdollPhysics();
        OriginalAnimInstance = TargetMesh->GetAnimInstance();
    }
}

void UCore_RagdollSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (RagdollState == ECore_RagdollState::Inactive)
    {
        return;
    }
    
    RagdollTime += DeltaTime;
    
    // Update blending during transition
    if (RagdollState == ECore_RagdollState::Activating || RagdollState == ECore_RagdollState::Deactivating)
    {
        UpdateRagdollBlending(DeltaTime);
    }
    
    // Check for stability and potential recovery
    if (RagdollState == ECore_RagdollState::Active && bCanRecoverFromRagdoll)
    {
        CheckStabilityForRecovery(DeltaTime);
    }
    
    // Auto-deactivate after max time
    if (RagdollTime >= MaxRagdollTime && RagdollState == ECore_RagdollState::Active)
    {
        DeactivateRagdoll();
    }
}

void UCore_RagdollSystem::ActivateRagdoll(const FVector& ImpactForce, const FVector& ImpactLocation, const FName& ImpactBone)
{
    if (!TargetMesh || RagdollState != ECore_RagdollState::Inactive)
    {
        return;
    }
    
    // Cache current bone transforms for potential recovery
    CachedBoneTransforms.Empty();
    if (TargetMesh->GetSkeletalMeshAsset())
    {
        const FReferenceSkeleton& RefSkeleton = TargetMesh->GetSkeletalMeshAsset()->GetRefSkeleton();
        for (int32 BoneIndex = 0; BoneIndex < RefSkeleton.GetNum(); ++BoneIndex)
        {
            FName BoneName = RefSkeleton.GetBoneName(BoneIndex);
            FTransform BoneTransform = TargetMesh->GetBoneTransform(BoneIndex);
            CachedBoneTransforms.Add(BoneName, BoneTransform);
        }
    }
    
    // Set ragdoll state
    RagdollState = ECore_RagdollState::Activating;
    RagdollTime = 0.0f;
    StabilityTimer = 0.0f;
    
    // Enable physics simulation
    TargetMesh->SetSimulatePhysics(true);
    TargetMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    
    // Apply physics settings
    ApplyPhysicsSettings();
    
    // Setup partial ragdoll if enabled
    if (bUsePartialRagdoll)
    {
        SetupPartialRagdoll();
    }
    
    // Apply impact force if provided
    if (!ImpactForce.IsZero() && ImpactForce.Size() >= MinImpactForce)
    {
        FVector ScaledForce = ImpactForce * ImpactForceMultiplier;
        
        if (ImpactBone != NAME_None)
        {
            // Apply force to specific bone with multiplier
            float BoneMultiplier = BoneForceMultipliers.Contains(ImpactBone) ? BoneForceMultipliers[ImpactBone] : 1.0f;
            ScaledForce *= BoneMultiplier;
            
            if (ImpactLocation.IsZero())
            {
                TargetMesh->AddImpulseToAllBodiesBelow(ScaledForce, ImpactBone, true);
            }
            else
            {
                TargetMesh->AddImpulseAtLocation(ScaledForce, ImpactLocation, ImpactBone);
            }
        }
        else
        {
            // Apply force to center of mass
            TargetMesh->AddImpulse(ScaledForce, NAME_None, true);
        }
    }
    
    // Broadcast activation event
    OnRagdollActivated.Broadcast();
    
    UE_LOG(LogTemp, Log, TEXT("RagdollSystem: Activated ragdoll for %s"), *GetOwner()->GetName());
}

void UCore_RagdollSystem::DeactivateRagdoll()
{
    if (!TargetMesh || RagdollState == ECore_RagdollState::Inactive)
    {
        return;
    }
    
    RagdollState = ECore_RagdollState::Deactivating;
    
    // Gradually blend back to animation
    // The actual deactivation will happen when blend weight reaches 0
    
    UE_LOG(LogTemp, Log, TEXT("RagdollSystem: Deactivating ragdoll for %s"), *GetOwner()->GetName());
}

void UCore_RagdollSystem::ApplyForceToRagdoll(const FVector& Force, const FName& BoneName, bool bAccelChange)
{
    if (!TargetMesh || RagdollState != ECore_RagdollState::Active)
    {
        return;
    }
    
    FVector ScaledForce = Force;
    if (BoneForceMultipliers.Contains(BoneName))
    {
        ScaledForce *= BoneForceMultipliers[BoneName];
    }
    
    TargetMesh->AddForce(ScaledForce, BoneName, bAccelChange);
}

void UCore_RagdollSystem::ApplyImpulseToRagdoll(const FVector& Impulse, const FName& BoneName, bool bVelChange)
{
    if (!TargetMesh || RagdollState != ECore_RagdollState::Active)
    {
        return;
    }
    
    FVector ScaledImpulse = Impulse;
    if (BoneForceMultipliers.Contains(BoneName))
    {
        ScaledImpulse *= BoneForceMultipliers[BoneName];
    }
    
    TargetMesh->AddImpulse(ScaledImpulse, BoneName, bVelChange);
}

bool UCore_RagdollSystem::IsRagdollActive() const
{
    return RagdollState == ECore_RagdollState::Active || RagdollState == ECore_RagdollState::Activating;
}

bool UCore_RagdollSystem::IsRagdollStable() const
{
    if (!TargetMesh || !IsRagdollActive())
    {
        return false;
    }
    
    FVector CenterOfMassVelocity = GetRagdollCenterOfMassVelocity();
    return CenterOfMassVelocity.Size() <= MaxStableVelocity;
}

ECore_RagdollState UCore_RagdollSystem::GetRagdollState() const
{
    return RagdollState;
}

void UCore_RagdollSystem::SetTargetMesh(USkeletalMeshComponent* NewTargetMesh)
{
    if (NewTargetMesh != TargetMesh)
    {
        TargetMesh = NewTargetMesh;
        if (TargetMesh)
        {
            InitializeRagdollPhysics();
            OriginalAnimInstance = TargetMesh->GetAnimInstance();
        }
    }
}

void UCore_RagdollSystem::SetBoneForceMultiplier(const FName& BoneName, float Multiplier)
{
    BoneForceMultipliers.Add(BoneName, FMath::Clamp(Multiplier, 0.1f, 10.0f));
}

FVector UCore_RagdollSystem::GetBoneVelocity(const FName& BoneName) const
{
    if (!TargetMesh || !IsRagdollActive())
    {
        return FVector::ZeroVector;
    }
    
    FBodyInstance* BodyInstance = TargetMesh->GetBodyInstance(BoneName);
    if (BodyInstance)
    {
        return BodyInstance->GetUnrealWorldVelocity();
    }
    
    return FVector::ZeroVector;
}

FVector UCore_RagdollSystem::GetRagdollCenterOfMassVelocity() const
{
    if (!TargetMesh || !IsRagdollActive())
    {
        return FVector::ZeroVector;
    }
    
    // Calculate average velocity of all physics bodies
    FVector TotalVelocity = FVector::ZeroVector;
    int32 BodyCount = 0;
    
    if (TargetMesh->GetSkeletalMeshAsset())
    {
        const FReferenceSkeleton& RefSkeleton = TargetMesh->GetSkeletalMeshAsset()->GetRefSkeleton();
        for (int32 BoneIndex = 0; BoneIndex < RefSkeleton.GetNum(); ++BoneIndex)
        {
            FName BoneName = RefSkeleton.GetBoneName(BoneIndex);
            FBodyInstance* BodyInstance = TargetMesh->GetBodyInstance(BoneName);
            if (BodyInstance && BodyInstance->IsValidBodyInstance())
            {
                TotalVelocity += BodyInstance->GetUnrealWorldVelocity();
                BodyCount++;
            }
        }
    }
    
    return BodyCount > 0 ? TotalVelocity / BodyCount : FVector::ZeroVector;
}

void UCore_RagdollSystem::ForceRecovery()
{
    if (IsRagdollActive())
    {
        OnRecoveryStarted.Broadcast();
        DeactivateRagdoll();
    }
}

bool UCore_RagdollSystem::CanRecover() const
{
    return bCanRecoverFromRagdoll && IsRagdollStable() && StabilityTimer >= StabilityTimeRequired;
}

void UCore_RagdollSystem::InitializeRagdollPhysics()
{
    if (!TargetMesh)
    {
        return;
    }
    
    // Use the mesh's physics asset if we don't have one set
    if (!RagdollPhysicsAsset && TargetMesh->GetPhysicsAsset())
    {
        RagdollPhysicsAsset = TargetMesh->GetPhysicsAsset();
    }
    
    // Ensure physics asset is set
    if (RagdollPhysicsAsset)
    {
        TargetMesh->SetPhysicsAsset(RagdollPhysicsAsset);
    }
}

void UCore_RagdollSystem::UpdateRagdollBlending(float DeltaTime)
{
    float TargetWeight = 0.0f;
    
    if (RagdollState == ECore_RagdollState::Activating)
    {
        TargetWeight = 1.0f;
    }
    else if (RagdollState == ECore_RagdollState::Deactivating)
    {
        TargetWeight = 0.0f;
    }
    
    // Interpolate blend weight
    RagdollBlendWeight = FMath::FInterpTo(RagdollBlendWeight, TargetWeight, DeltaTime, TransitionSpeed);
    
    // Update state based on blend completion
    if (RagdollState == ECore_RagdollState::Activating && FMath::IsNearlyEqual(RagdollBlendWeight, 1.0f, 0.01f))
    {
        RagdollState = ECore_RagdollState::Active;
        RagdollBlendWeight = 1.0f;
    }
    else if (RagdollState == ECore_RagdollState::Deactivating && FMath::IsNearlyEqual(RagdollBlendWeight, 0.0f, 0.01f))
    {
        // Complete deactivation
        RagdollState = ECore_RagdollState::Inactive;
        RagdollBlendWeight = 0.0f;
        RagdollTime = 0.0f;
        
        // Disable physics simulation
        TargetMesh->SetSimulatePhysics(false);
        TargetMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        
        // Restore original animation instance
        if (OriginalAnimInstance)
        {
            TargetMesh->SetAnimInstanceClass(OriginalAnimInstance->GetClass());
        }
        
        // Broadcast deactivation event
        OnRagdollDeactivated.Broadcast();
    }
}

void UCore_RagdollSystem::CheckStabilityForRecovery(float DeltaTime)
{
    if (IsRagdollStable())
    {
        StabilityTimer += DeltaTime;
        
        if (StabilityTimer >= StabilityTimeRequired)
        {
            // Ragdoll is stable enough for recovery
            if (!OnRagdollStabilized.IsBound() || OnRagdollStabilized.IsBound())
            {
                OnRagdollStabilized.Broadcast();
            }
            
            // Auto-recovery if enabled
            if (bCanRecoverFromRagdoll)
            {
                ForceRecovery();
            }
        }
    }
    else
    {
        StabilityTimer = 0.0f;
    }
}

void UCore_RagdollSystem::ApplyPhysicsSettings()
{
    if (!TargetMesh)
    {
        return;
    }
    
    // Apply damping settings to all physics bodies
    if (TargetMesh->GetSkeletalMeshAsset())
    {
        const FReferenceSkeleton& RefSkeleton = TargetMesh->GetSkeletalMeshAsset()->GetRefSkeleton();
        for (int32 BoneIndex = 0; BoneIndex < RefSkeleton.GetNum(); ++BoneIndex)
        {
            FName BoneName = RefSkeleton.GetBoneName(BoneIndex);
            FBodyInstance* BodyInstance = TargetMesh->GetBodyInstance(BoneName);
            if (BodyInstance && BodyInstance->IsValidBodyInstance())
            {
                BodyInstance->LinearDamping = LinearDamping;
                BodyInstance->AngularDamping = AngularDamping;
                BodyInstance->UpdateDampingProperties();
            }
        }
    }
}

void UCore_RagdollSystem::SetupPartialRagdoll()
{
    if (!TargetMesh || ExcludedBones.Num() == 0)
    {
        return;
    }
    
    // Disable physics simulation for excluded bones
    for (const FName& BoneName : ExcludedBones)
    {
        FBodyInstance* BodyInstance = TargetMesh->GetBodyInstance(BoneName);
        if (BodyInstance)
        {
            BodyInstance->SetInstanceSimulatePhysics(false);
        }
    }
}