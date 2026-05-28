#include "AdvancedRagdollComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UAdvancedRagdollComponent::UAdvancedRagdollComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
    
    // Default ragdoll settings
    RagdollBlendWeight = 0.0f;
    TargetRagdollBlendWeight = 0.0f;
    RagdollBlendSpeed = 5.0f;
    bIsRagdollActive = false;
    bCanGetUp = true;
    GetUpThreshold = 0.5f;
    StabilityCheckRadius = 50.0f;
    MinVelocityForRagdoll = 300.0f;
    MaxRagdollTime = 10.0f;
    CurrentRagdollTime = 0.0f;
    
    // Bone tracking
    bTrackBoneVelocities = true;
    MaxTrackedBones = 20;
    VelocityHistorySize = 10;
    
    // Recovery settings
    RecoveryBlendTime = 1.5f;
    RecoveryAnimationSpeed = 1.0f;
    bUseRecoveryAnimation = true;
    
    // Impact settings
    ImpactForceMultiplier = 1.0f;
    MinImpactForce = 100.0f;
    MaxImpactForce = 5000.0f;
}

void UAdvancedRagdollComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache owner character
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        SkeletalMeshComponent = OwnerCharacter->GetMesh();
        if (SkeletalMeshComponent)
        {
            // Store original collision settings
            OriginalCollisionEnabled = SkeletalMeshComponent->GetCollisionEnabled();
            OriginalCollisionObjectType = SkeletalMeshComponent->GetCollisionObjectType();
            
            // Initialize bone tracking
            InitializeBoneTracking();
        }
        
        // Cache capsule component
        CapsuleComponent = OwnerCharacter->GetCapsuleComponent();
    }
    
    // Initialize ragdoll state
    RagdollState = ERagdollState::Inactive;
    LastRagdollActivationTime = 0.0f;
}

void UAdvancedRagdollComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!SkeletalMeshComponent || !OwnerCharacter)
        return;
    
    // Update ragdoll blend weight
    UpdateRagdollBlending(DeltaTime);
    
    // Track bone velocities if enabled
    if (bTrackBoneVelocities && bIsRagdollActive)
    {
        UpdateBoneVelocityTracking(DeltaTime);
    }
    
    // Check for automatic recovery conditions
    if (bIsRagdollActive)
    {
        CurrentRagdollTime += DeltaTime;
        CheckRecoveryConditions(DeltaTime);
    }
    
    // Update ragdoll state machine
    UpdateRagdollStateMachine(DeltaTime);
}

void UAdvancedRagdollComponent::ActivateRagdoll(const FVector& ImpactForce, const FVector& ImpactLocation, FName BoneName)
{
    if (!SkeletalMeshComponent || bIsRagdollActive)
        return;
    
    UE_LOG(LogTemp, Log, TEXT("AdvancedRagdollComponent: Activating ragdoll with impact force %s"), *ImpactForce.ToString());
    
    // Set ragdoll state
    bIsRagdollActive = true;
    RagdollState = ERagdollState::Activating;
    TargetRagdollBlendWeight = 1.0f;
    CurrentRagdollTime = 0.0f;
    LastRagdollActivationTime = GetWorld()->GetTimeSeconds();
    
    // Disable character movement
    if (OwnerCharacter)
    {
        OwnerCharacter->GetCharacterMovement()->SetMovementMode(MOVE_None);
        
        // Disable capsule collision
        if (CapsuleComponent)
        {
            CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
    }
    
    // Enable physics simulation
    SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    SkeletalMeshComponent->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
    SkeletalMeshComponent->SetSimulatePhysics(true);
    SkeletalMeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
    
    // Apply impact force if specified
    if (!ImpactForce.IsZero())
    {
        ApplyImpactForce(ImpactForce, ImpactLocation, BoneName);
    }
    
    // Broadcast activation event
    OnRagdollActivated.Broadcast();
    
    // Store ragdoll activation data
    RagdollActivationData.ImpactForce = ImpactForce;
    RagdollActivationData.ImpactLocation = ImpactLocation;
    RagdollActivationData.ImpactBone = BoneName;
    RagdollActivationData.ActivationTime = GetWorld()->GetTimeSeconds();
}

void UAdvancedRagdollComponent::DeactivateRagdoll(bool bForceImmediate)
{
    if (!bIsRagdollActive)
        return;
    
    UE_LOG(LogTemp, Log, TEXT("AdvancedRagdollComponent: Deactivating ragdoll"));
    
    if (bForceImmediate)
    {
        // Immediate deactivation
        CompleteRagdollDeactivation();
    }
    else
    {
        // Gradual deactivation
        RagdollState = ERagdollState::Deactivating;
        TargetRagdollBlendWeight = 0.0f;
    }
}

void UAdvancedRagdollComponent::CompleteRagdollDeactivation()
{
    if (!SkeletalMeshComponent || !OwnerCharacter)
        return;
    
    // Reset ragdoll state
    bIsRagdollActive = false;
    RagdollState = ERagdollState::Inactive;
    RagdollBlendWeight = 0.0f;
    TargetRagdollBlendWeight = 0.0f;
    CurrentRagdollTime = 0.0f;
    
    // Disable physics simulation
    SkeletalMeshComponent->SetSimulatePhysics(false);
    
    // Restore original collision settings
    SkeletalMeshComponent->SetCollisionEnabled(OriginalCollisionEnabled);
    SkeletalMeshComponent->SetCollisionObjectType(OriginalCollisionObjectType);
    
    // Re-enable character movement
    OwnerCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
    
    // Re-enable capsule collision
    if (CapsuleComponent)
    {
        CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }
    
    // Broadcast deactivation event
    OnRagdollDeactivated.Broadcast();
}

void UAdvancedRagdollComponent::ApplyImpactForce(const FVector& Force, const FVector& Location, FName BoneName)
{
    if (!SkeletalMeshComponent || !bIsRagdollActive)
        return;
    
    // Clamp force magnitude
    FVector ClampedForce = Force;
    float ForceMagnitude = Force.Size();
    if (ForceMagnitude > MaxImpactForce)
    {
        ClampedForce = Force.GetSafeNormal() * MaxImpactForce;
    }
    else if (ForceMagnitude < MinImpactForce)
    {
        ClampedForce = Force.GetSafeNormal() * MinImpactForce;
    }
    
    // Apply force multiplier
    ClampedForce *= ImpactForceMultiplier;
    
    // Apply force to specific bone or root
    if (BoneName != NAME_None && SkeletalMeshComponent->GetBoneIndex(BoneName) != INDEX_NONE)
    {
        SkeletalMeshComponent->AddImpulseAtLocation(ClampedForce, Location, BoneName);
    }
    else
    {
        SkeletalMeshComponent->AddImpulseAtLocation(ClampedForce, Location);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Applied impact force %s at location %s to bone %s"), 
           *ClampedForce.ToString(), *Location.ToString(), *BoneName.ToString());
}

bool UAdvancedRagdollComponent::CanRecoverFromRagdoll() const
{
    if (!bCanGetUp || !SkeletalMeshComponent)
        return false;
    
    // Check if ragdoll has been active long enough
    if (CurrentRagdollTime < GetUpThreshold)
        return false;
    
    // Check if character is stable (low velocity)
    FVector RootVelocity = SkeletalMeshComponent->GetPhysicsLinearVelocity();
    if (RootVelocity.Size() > 100.0f) // Still moving too fast
        return false;
    
    // Check if character is in a reasonable position to get up
    FVector RootLocation = SkeletalMeshComponent->GetComponentLocation();
    FVector UpVector = FVector::UpVector;
    
    // Perform stability check
    return IsPositionStable(RootLocation);
}

bool UAdvancedRagdollComponent::IsPositionStable(const FVector& Position) const
{
    if (!GetWorld())
        return false;
    
    // Check for ground contact within stability radius
    FHitResult HitResult;
    FVector StartLocation = Position + FVector(0, 0, 50);
    FVector EndLocation = Position - FVector(0, 0, 200);
    
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    
    bool bHitGround = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECollisionChannel::ECC_WorldStatic,
        QueryParams
    );
    
    if (bHitGround)
    {
        // Check if the ground is relatively flat
        FVector GroundNormal = HitResult.Normal;
        float GroundAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(GroundNormal, FVector::UpVector)));
        
        return GroundAngle < 45.0f; // Ground is not too steep
    }
    
    return false;
}

void UAdvancedRagdollComponent::UpdateRagdollBlending(float DeltaTime)
{
    if (FMath::IsNearlyEqual(RagdollBlendWeight, TargetRagdollBlendWeight, 0.01f))
        return;
    
    // Interpolate blend weight
    RagdollBlendWeight = FMath::FInterpTo(RagdollBlendWeight, TargetRagdollBlendWeight, DeltaTime, RagdollBlendSpeed);
    
    // Apply blend weight to animation
    if (SkeletalMeshComponent && SkeletalMeshComponent->GetAnimInstance())
    {
        // This would typically be handled in the animation blueprint
        // For now, we'll just store the blend weight for the animation system to use
    }
}

void UAdvancedRagdollComponent::InitializeBoneTracking()
{
    if (!SkeletalMeshComponent || !bTrackBoneVelocities)
        return;
    
    // Get important bones to track
    TArray<FName> BonesToTrack = {
        TEXT("pelvis"),
        TEXT("spine_01"),
        TEXT("spine_02"),
        TEXT("spine_03"),
        TEXT("head"),
        TEXT("upperarm_l"),
        TEXT("upperarm_r"),
        TEXT("lowerarm_l"),
        TEXT("lowerarm_r"),
        TEXT("thigh_l"),
        TEXT("thigh_r"),
        TEXT("calf_l"),
        TEXT("calf_r")
    };
    
    // Initialize tracking data
    BoneVelocityHistory.Empty();
    for (const FName& BoneName : BonesToTrack)
    {
        if (SkeletalMeshComponent->GetBoneIndex(BoneName) != INDEX_NONE)
        {
            FBoneVelocityData VelocityData;
            VelocityData.BoneName = BoneName;
            VelocityData.VelocityHistory.Reserve(VelocityHistorySize);
            BoneVelocityHistory.Add(VelocityData);
            
            if (BoneVelocityHistory.Num() >= MaxTrackedBones)
                break;
        }
    }
}

void UAdvancedRagdollComponent::UpdateBoneVelocityTracking(float DeltaTime)
{
    if (!SkeletalMeshComponent || BoneVelocityHistory.IsEmpty())
        return;
    
    for (FBoneVelocityData& VelocityData : BoneVelocityHistory)
    {
        // Get current bone location
        FVector CurrentLocation = SkeletalMeshComponent->GetBoneLocation(VelocityData.BoneName);
        
        // Calculate velocity
        FVector Velocity = FVector::ZeroVector;
        if (VelocityData.VelocityHistory.Num() > 0)
        {
            FVector PreviousLocation = VelocityData.VelocityHistory.Last().Location;
            Velocity = (CurrentLocation - PreviousLocation) / DeltaTime;
        }
        
        // Store velocity data
        FBoneVelocityFrame VelocityFrame;
        VelocityFrame.Location = CurrentLocation;
        VelocityFrame.Velocity = Velocity;
        VelocityFrame.Timestamp = GetWorld()->GetTimeSeconds();
        
        VelocityData.VelocityHistory.Add(VelocityFrame);
        
        // Maintain history size
        if (VelocityData.VelocityHistory.Num() > VelocityHistorySize)
        {
            VelocityData.VelocityHistory.RemoveAt(0);
        }
    }
}

void UAdvancedRagdollComponent::CheckRecoveryConditions(float DeltaTime)
{
    if (RagdollState != ERagdollState::Active)
        return;
    
    // Check if we've exceeded maximum ragdoll time
    if (CurrentRagdollTime > MaxRagdollTime)
    {
        UE_LOG(LogTemp, Warning, TEXT("Ragdoll exceeded maximum time, forcing recovery"));
        DeactivateRagdoll(false);
        return;
    }
    
    // Check if conditions are met for natural recovery
    if (CanRecoverFromRagdoll())
    {
        UE_LOG(LogTemp, Log, TEXT("Ragdoll recovery conditions met, initiating recovery"));
        DeactivateRagdoll(false);
    }
}

void UAdvancedRagdollComponent::UpdateRagdollStateMachine(float DeltaTime)
{
    switch (RagdollState)
    {
        case ERagdollState::Inactive:
            // Nothing to do
            break;
            
        case ERagdollState::Activating:
            if (RagdollBlendWeight >= 0.95f)
            {
                RagdollState = ERagdollState::Active;
            }
            break;
            
        case ERagdollState::Active:
            // Active ragdoll state - handled in CheckRecoveryConditions
            break;
            
        case ERagdollState::Deactivating:
            if (RagdollBlendWeight <= 0.05f)
            {
                CompleteRagdollDeactivation();
            }
            break;
    }
}

FVector UAdvancedRagdollComponent::GetBoneVelocity(FName BoneName) const
{
    for (const FBoneVelocityData& VelocityData : BoneVelocityHistory)
    {
        if (VelocityData.BoneName == BoneName && !VelocityData.VelocityHistory.IsEmpty())
        {
            return VelocityData.VelocityHistory.Last().Velocity;
        }
    }
    
    return FVector::ZeroVector;
}

float UAdvancedRagdollComponent::GetAverageVelocityMagnitude() const
{
    if (BoneVelocityHistory.IsEmpty())
        return 0.0f;
    
    float TotalVelocity = 0.0f;
    int32 ValidBones = 0;
    
    for (const FBoneVelocityData& VelocityData : BoneVelocityHistory)
    {
        if (!VelocityData.VelocityHistory.IsEmpty())
        {
            TotalVelocity += VelocityData.VelocityHistory.Last().Velocity.Size();
            ValidBones++;
        }
    }
    
    return ValidBones > 0 ? TotalVelocity / ValidBones : 0.0f;
}

void UAdvancedRagdollComponent::SetRagdollBlendSpeed(float NewSpeed)
{
    RagdollBlendSpeed = FMath::Max(0.1f, NewSpeed);
}

void UAdvancedRagdollComponent::SetMaxRagdollTime(float NewMaxTime)
{
    MaxRagdollTime = FMath::Max(1.0f, NewMaxTime);
}

void UAdvancedRagdollComponent::EnableBoneVelocityTracking(bool bEnable)
{
    bTrackBoneVelocities = bEnable;
    if (bEnable && BoneVelocityHistory.IsEmpty())
    {
        InitializeBoneTracking();
    }
    else if (!bEnable)
    {
        BoneVelocityHistory.Empty();
    }
}