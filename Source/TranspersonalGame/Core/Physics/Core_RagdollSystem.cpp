#include "Core_RagdollSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Animation/AnimInstanceProxy.h"

UCore_RagdollSystem::UCore_RagdollSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize default values
    RagdollState = ECore_RagdollState::Inactive;
    ActivationThreshold = 50.0f;
    AutoRecoveryTime = 5.0f;
    ImpulseMultiplier = 1.0f;
    CurrentRagdollTime = 0.0f;
    TriggerDamage = 0.0f;
    TriggerLocation = FVector::ZeroVector;
    
    // Setup critical bones for humanoid characters
    CriticalBones.Add(TEXT("spine_01"));
    CriticalBones.Add(TEXT("spine_02"));
    CriticalBones.Add(TEXT("spine_03"));
    CriticalBones.Add(TEXT("head"));
    CriticalBones.Add(TEXT("pelvis"));
    CriticalBones.Add(TEXT("thigh_l"));
    CriticalBones.Add(TEXT("thigh_r"));
    CriticalBones.Add(TEXT("upperarm_l"));
    CriticalBones.Add(TEXT("upperarm_r"));
}

void UCore_RagdollSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Auto-detect skeletal mesh component on owner
    if (AActor* Owner = GetOwner())
    {
        if (USkeletalMeshComponent* FoundMesh = Owner->FindComponentByClass<USkeletalMeshComponent>())
        {
            InitializeRagdoll(FoundMesh);
        }
    }
}

void UCore_RagdollSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (RagdollState == ECore_RagdollState::Active)
    {
        CurrentRagdollTime += DeltaTime;
        
        // Check for bone impacts during ragdoll
        if (TargetMesh.IsValid())
        {
            for (const FName& BoneName : CriticalBones)
            {
                FVector BoneVelocity = GetBoneVelocity(BoneName);
                if (BoneVelocity.Size() > 500.0f) // High velocity impact threshold
                {
                    FVector BoneLocation = TargetMesh->GetBoneLocation(BoneName);
                    OnBoneImpact.Broadcast(BoneName, BoneLocation, BoneVelocity);
                }
            }
        }
    }
}

void UCore_RagdollSystem::InitializeRagdoll(USkeletalMeshComponent* InTargetMesh)
{
    if (!InTargetMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_RagdollSystem: Cannot initialize with null mesh"));
        return;
    }
    
    TargetMesh = InTargetMesh;
    SetupPhysicsConstraints();
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Initialized for mesh %s"), *InTargetMesh->GetName());
}

bool UCore_RagdollSystem::ActivateRagdoll(float Damage, const FVector& ImpactPoint, const FVector& ImpactForce)
{
    if (!TargetMesh.IsValid() || RagdollState == ECore_RagdollState::Active)
    {
        return false;
    }
    
    // Check damage threshold
    if (Damage < ActivationThreshold)
    {
        UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Damage %.2f below threshold %.2f"), Damage, ActivationThreshold);
        return false;
    }
    
    // Activate ragdoll physics
    TargetMesh->SetSimulatePhysics(true);
    TargetMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    
    // Store trigger information
    TriggerDamage = Damage;
    TriggerLocation = ImpactPoint;
    RagdollState = ECore_RagdollState::Active;
    CurrentRagdollTime = 0.0f;
    
    // Apply impulse forces
    ApplyImpulseForces(ImpactPoint, ImpactForce);
    
    // Setup auto recovery timer
    if (AutoRecoveryTime > 0.0f)
    {
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().SetTimer(RecoveryTimerHandle, this, &UCore_RagdollSystem::HandleAutoRecovery, AutoRecoveryTime, false);
        }
    }
    
    // Broadcast activation event
    OnRagdollActivated.Broadcast(Damage, ImpactPoint, ImpactForce);
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Ragdoll activated with damage %.2f"), Damage);
    return true;
}

void UCore_RagdollSystem::DeactivateRagdoll()
{
    if (RagdollState == ECore_RagdollState::Inactive)
    {
        return;
    }
    
    // Clear auto recovery timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(RecoveryTimerHandle);
    }
    
    // Start blend to animation
    RagdollState = ECore_RagdollState::BlendingToAnimation;
    BlendToAnimation(1.0f);
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Ragdoll deactivated after %.2f seconds"), CurrentRagdollTime);
}

void UCore_RagdollSystem::ForceRagdollActivation(const FVector& ImpactPoint, const FVector& ImpactForce)
{
    // Bypass damage threshold for forced activation
    float OriginalThreshold = ActivationThreshold;
    ActivationThreshold = 0.0f;
    
    ActivateRagdoll(1.0f, ImpactPoint, ImpactForce);
    
    // Restore original threshold
    ActivationThreshold = OriginalThreshold;
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Forced ragdoll activation"));
}

bool UCore_RagdollSystem::IsRagdollActive() const
{
    return RagdollState == ECore_RagdollState::Active;
}

ECore_RagdollState UCore_RagdollSystem::GetRagdollState() const
{
    return RagdollState;
}

void UCore_RagdollSystem::ApplyBoneImpulse(const FName& BoneName, const FVector& Impulse)
{
    if (!TargetMesh.IsValid() || !ValidateBoneName(BoneName))
    {
        return;
    }
    
    if (UPrimitiveComponent* BoneBody = TargetMesh->GetBodyInstance(BoneName)->GetPrimitiveComponent())
    {
        BoneBody->AddImpulseAtLocation(Impulse, TargetMesh->GetBoneLocation(BoneName));
        UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Applied impulse %.2f to bone %s"), Impulse.Size(), *BoneName.ToString());
    }
}

void UCore_RagdollSystem::SetBoneConstraint(const FName& BoneName, const FCore_RagdollConstraint& Constraint)
{
    if (!ValidateBoneName(BoneName))
    {
        return;
    }
    
    BoneConstraints.Add(BoneName, Constraint);
    
    // Apply constraint if ragdoll is active
    if (RagdollState == ECore_RagdollState::Active && TargetMesh.IsValid())
    {
        // Implementation would involve setting up physics constraints
        // This is a simplified version for the core system
        UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Set constraint for bone %s"), *BoneName.ToString());
    }
}

FVector UCore_RagdollSystem::GetBoneVelocity(const FName& BoneName) const
{
    if (!TargetMesh.IsValid() || !ValidateBoneName(BoneName))
    {
        return FVector::ZeroVector;
    }
    
    if (FBodyInstance* BodyInstance = TargetMesh->GetBodyInstance(BoneName))
    {
        return BodyInstance->GetUnrealWorldVelocity();
    }
    
    return FVector::ZeroVector;
}

bool UCore_RagdollSystem::IsBoneMoving(const FName& BoneName, float VelocityThreshold) const
{
    FVector Velocity = GetBoneVelocity(BoneName);
    return Velocity.Size() > VelocityThreshold;
}

void UCore_RagdollSystem::BlendToAnimation(float BlendTime)
{
    if (!TargetMesh.IsValid())
    {
        return;
    }
    
    // Disable physics simulation
    TargetMesh->SetSimulatePhysics(false);
    TargetMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    
    // Reset state
    RagdollState = ECore_RagdollState::Inactive;
    CurrentRagdollTime = 0.0f;
    TriggerDamage = 0.0f;
    TriggerLocation = FVector::ZeroVector;
    
    // Broadcast deactivation event
    OnRagdollDeactivated.Broadcast(BlendTime);
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Blending to animation over %.2f seconds"), BlendTime);
}

void UCore_RagdollSystem::SetAutoRecoveryTime(float NewTime)
{
    AutoRecoveryTime = FMath::Clamp(NewTime, 0.0f, 30.0f);
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Auto recovery time set to %.2f seconds"), AutoRecoveryTime);
}

float UCore_RagdollSystem::GetRecoveryTimeRemaining() const
{
    if (RagdollState != ECore_RagdollState::Active || AutoRecoveryTime <= 0.0f)
    {
        return 0.0f;
    }
    
    return FMath::Max(0.0f, AutoRecoveryTime - CurrentRagdollTime);
}

void UCore_RagdollSystem::SetupPhysicsConstraints()
{
    if (!TargetMesh.IsValid())
    {
        return;
    }
    
    // Setup default constraints for critical bones
    for (const FName& BoneName : CriticalBones)
    {
        if (!BoneConstraints.Contains(BoneName))
        {
            FCore_RagdollConstraint DefaultConstraint;
            DefaultConstraint.LinearLimit = 100.0f;
            DefaultConstraint.AngularLimit = 45.0f;
            DefaultConstraint.LinearStiffness = 1000.0f;
            DefaultConstraint.AngularStiffness = 500.0f;
            DefaultConstraint.LinearDamping = 50.0f;
            DefaultConstraint.AngularDamping = 25.0f;
            
            BoneConstraints.Add(BoneName, DefaultConstraint);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Setup physics constraints for %d bones"), BoneConstraints.Num());
}

void UCore_RagdollSystem::ApplyImpulseForces(const FVector& ImpactPoint, const FVector& ImpactForce)
{
    if (!TargetMesh.IsValid())
    {
        return;
    }
    
    // Calculate impulse distribution across critical bones
    TMap<FName, FVector> ImpulseDistribution = CalculateImpulseDistribution(ImpactPoint, ImpactForce);
    
    // Apply impulses to bones
    for (const auto& ImpulsePair : ImpulseDistribution)
    {
        ApplyBoneImpulse(ImpulsePair.Key, ImpulsePair.Value);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Applied impulse forces to %d bones"), ImpulseDistribution.Num());
}

void UCore_RagdollSystem::HandleAutoRecovery()
{
    if (RagdollState == ECore_RagdollState::Active)
    {
        UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Auto recovery triggered"));
        DeactivateRagdoll();
    }
}

bool UCore_RagdollSystem::ValidateBoneName(const FName& BoneName) const
{
    if (!TargetMesh.IsValid())
    {
        return false;
    }
    
    return TargetMesh->GetBoneIndex(BoneName) != INDEX_NONE;
}

TMap<FName, FVector> UCore_RagdollSystem::CalculateImpulseDistribution(const FVector& ImpactPoint, const FVector& ImpactForce) const
{
    TMap<FName, FVector> Distribution;
    
    if (!TargetMesh.IsValid())
    {
        return Distribution;
    }
    
    FVector ScaledForce = ImpactForce * ImpulseMultiplier;
    
    // Find closest bone to impact point
    FName ClosestBone = NAME_None;
    float ClosestDistance = FLT_MAX;
    
    for (const FName& BoneName : CriticalBones)
    {
        FVector BoneLocation = TargetMesh->GetBoneLocation(BoneName);
        float Distance = FVector::Dist(BoneLocation, ImpactPoint);
        
        if (Distance < ClosestDistance)
        {
            ClosestDistance = Distance;
            ClosestBone = BoneName;
        }
    }
    
    // Distribute force based on distance from impact
    for (const FName& BoneName : CriticalBones)
    {
        FVector BoneLocation = TargetMesh->GetBoneLocation(BoneName);
        float Distance = FVector::Dist(BoneLocation, ImpactPoint);
        
        // Calculate force falloff (closer bones get more force)
        float ForceFalloff = FMath::Clamp(1.0f - (Distance / 500.0f), 0.1f, 1.0f);
        
        if (BoneName == ClosestBone)
        {
            ForceFalloff = 1.0f; // Full force to closest bone
        }
        
        Distribution.Add(BoneName, ScaledForce * ForceFalloff);
    }
    
    return Distribution;
}