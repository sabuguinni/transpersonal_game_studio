#include "Core_RagdollPhysicsManager.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Engine/Engine.h"

UCore_RagdollPhysicsManager::UCore_RagdollPhysicsManager()
{
    // Initialize default ragdoll settings
    RagdollSettings.MinActivationForce = 500.0f;
    RagdollSettings.MaxRagdollDuration = 10.0f;
    RagdollSettings.RecoveryBlendTime = 2.0f;
    RagdollSettings.bAutoRecover = true;
}

void UCore_RagdollPhysicsManager::ActivateRagdoll(USkeletalMeshComponent* SkeletalMesh, FVector ImpactForce)
{
    if (!ValidateSkeletalMesh(SkeletalMesh))
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid SkeletalMesh for ragdoll activation"));
        return;
    }

    // Check if ragdoll is already active
    if (IsRagdollActive(SkeletalMesh))
    {
        UE_LOG(LogTemp, Log, TEXT("Ragdoll already active for mesh"));
        return;
    }

    // Initialize ragdoll physics
    InitializeRagdollPhysics(SkeletalMesh);

    // Create and store ragdoll state
    FCore_RagdollState NewState;
    NewState.bIsRagdollActive = true;
    NewState.RagdollStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    NewState.LastKnownVelocity = ImpactForce;
    NewState.ImpactForce = ImpactForce.Size();

    ActiveRagdolls.Add(SkeletalMesh, NewState);

    // Set physics simulation on
    SkeletalMesh->SetSimulatePhysics(true);
    SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    // Apply initial impact force
    if (ImpactForce.SizeSquared() > 0.0f)
    {
        SkeletalMesh->AddImpulse(ImpactForce, NAME_None, true);
    }

    UE_LOG(LogTemp, Log, TEXT("Ragdoll activated with impact force: %s"), *ImpactForce.ToString());
}

void UCore_RagdollPhysicsManager::DeactivateRagdoll(USkeletalMeshComponent* SkeletalMesh)
{
    if (!ValidateSkeletalMesh(SkeletalMesh))
    {
        return;
    }

    // Remove from active ragdolls
    ActiveRagdolls.Remove(SkeletalMesh);

    // Disable physics simulation
    SkeletalMesh->SetSimulatePhysics(false);
    SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

    // Reset physics blend weight
    SkeletalMesh->SetPhysicsBlendWeight(0.0f);

    // Cleanup ragdoll physics
    CleanupRagdollPhysics(SkeletalMesh);

    UE_LOG(LogTemp, Log, TEXT("Ragdoll deactivated"));
}

bool UCore_RagdollPhysicsManager::IsRagdollActive(USkeletalMeshComponent* SkeletalMesh) const
{
    if (!ValidateSkeletalMesh(SkeletalMesh))
    {
        return false;
    }

    const FCore_RagdollState* State = ActiveRagdolls.Find(SkeletalMesh);
    return State && State->bIsRagdollActive;
}

void UCore_RagdollPhysicsManager::ApplyImpactForce(USkeletalMeshComponent* SkeletalMesh, FVector Force, FVector Location, FName BoneName)
{
    if (!ValidateSkeletalMesh(SkeletalMesh) || !IsRagdollActive(SkeletalMesh))
    {
        return;
    }

    if (BoneName != NAME_None)
    {
        SkeletalMesh->AddImpulseAtLocation(Force, Location, BoneName);
    }
    else
    {
        SkeletalMesh->AddImpulseAtLocation(Force, Location);
    }

    // Update ragdoll state with new impact
    FCore_RagdollState* State = ActiveRagdolls.Find(SkeletalMesh);
    if (State)
    {
        State->LastKnownVelocity = Force;
        State->ImpactForce = FMath::Max(State->ImpactForce, Force.Size());
    }

    UE_LOG(LogTemp, Log, TEXT("Applied impact force: %s at location: %s"), *Force.ToString(), *Location.ToString());
}

void UCore_RagdollPhysicsManager::ApplyRadialForce(USkeletalMeshComponent* SkeletalMesh, FVector Origin, float Radius, float Strength)
{
    if (!ValidateSkeletalMesh(SkeletalMesh) || !IsRagdollActive(SkeletalMesh))
    {
        return;
    }

    SkeletalMesh->AddRadialImpulse(Origin, Radius, Strength, ERadialImpulseFalloff::RIF_Linear, true);

    UE_LOG(LogTemp, Log, TEXT("Applied radial force: Origin=%s, Radius=%f, Strength=%f"), 
           *Origin.ToString(), Radius, Strength);
}

FCore_RagdollState UCore_RagdollPhysicsManager::GetRagdollState(USkeletalMeshComponent* SkeletalMesh) const
{
    if (!ValidateSkeletalMesh(SkeletalMesh))
    {
        return FCore_RagdollState();
    }

    const FCore_RagdollState* State = ActiveRagdolls.Find(SkeletalMesh);
    return State ? *State : FCore_RagdollState();
}

void UCore_RagdollPhysicsManager::UpdateRagdollSettings(const FCore_RagdollSettings& NewSettings)
{
    RagdollSettings = NewSettings;
    UE_LOG(LogTemp, Log, TEXT("Ragdoll settings updated"));
}

void UCore_RagdollPhysicsManager::StartRagdollRecovery(USkeletalMeshComponent* SkeletalMesh)
{
    if (!ValidateSkeletalMesh(SkeletalMesh) || !IsRagdollActive(SkeletalMesh))
    {
        return;
    }

    // Begin blending back to animation
    SkeletalMesh->SetPhysicsBlendWeight(0.0f);
    
    // Start recovery process - this would typically involve animation blending
    UE_LOG(LogTemp, Log, TEXT("Starting ragdoll recovery"));
    
    // For now, simply deactivate after blend time
    // In a full implementation, this would use a timer or animation system
    DeactivateRagdoll(SkeletalMesh);
}

bool UCore_RagdollPhysicsManager::CanRecoverFromRagdoll(USkeletalMeshComponent* SkeletalMesh) const
{
    if (!ValidateSkeletalMesh(SkeletalMesh) || !IsRagdollActive(SkeletalMesh))
    {
        return false;
    }

    const FCore_RagdollState* State = ActiveRagdolls.Find(SkeletalMesh);
    if (!State)
    {
        return false;
    }

    // Check if enough time has passed and velocity is low enough
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    float RagdollDuration = CurrentTime - State->RagdollStartTime;
    
    bool bTimeCondition = RagdollDuration >= 1.0f; // Minimum ragdoll time
    bool bVelocityCondition = State->LastKnownVelocity.Size() < 100.0f; // Low velocity threshold
    
    return bTimeCondition && bVelocityCondition;
}

void UCore_RagdollPhysicsManager::SetRagdollPhysicsBlend(USkeletalMeshComponent* SkeletalMesh, float BlendWeight)
{
    if (!ValidateSkeletalMesh(SkeletalMesh))
    {
        return;
    }

    BlendWeight = FMath::Clamp(BlendWeight, 0.0f, 1.0f);
    SkeletalMesh->SetPhysicsBlendWeight(BlendWeight);
}

FVector UCore_RagdollPhysicsManager::GetRagdollCenterOfMass(USkeletalMeshComponent* SkeletalMesh) const
{
    if (!ValidateSkeletalMesh(SkeletalMesh))
    {
        return FVector::ZeroVector;
    }

    // Calculate approximate center of mass based on bone positions
    FVector CenterOfMass = FVector::ZeroVector;
    int32 BoneCount = 0;

    for (int32 BoneIndex = 0; BoneIndex < SkeletalMesh->GetNumBones(); ++BoneIndex)
    {
        FVector BoneLocation = SkeletalMesh->GetBoneLocation(SkeletalMesh->GetBoneName(BoneIndex));
        CenterOfMass += BoneLocation;
        BoneCount++;
    }

    if (BoneCount > 0)
    {
        CenterOfMass /= BoneCount;
    }

    return CenterOfMass;
}

void UCore_RagdollPhysicsManager::TickRagdollManager(float DeltaTime)
{
    TArray<USkeletalMeshComponent*> ToRemove;

    for (auto& RagdollPair : ActiveRagdolls)
    {
        USkeletalMeshComponent* SkeletalMesh = RagdollPair.Key;
        FCore_RagdollState& State = RagdollPair.Value;

        if (!IsValid(SkeletalMesh))
        {
            ToRemove.Add(SkeletalMesh);
            continue;
        }

        UpdateRagdollState(SkeletalMesh, DeltaTime);

        // Auto-recovery check
        if (RagdollSettings.bAutoRecover && CanRecoverFromRagdoll(SkeletalMesh))
        {
            float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
            float RagdollDuration = CurrentTime - State.RagdollStartTime;
            
            if (RagdollDuration >= RagdollSettings.MaxRagdollDuration)
            {
                StartRagdollRecovery(SkeletalMesh);
                ToRemove.Add(SkeletalMesh);
            }
        }
    }

    // Clean up invalid or recovered ragdolls
    for (USkeletalMeshComponent* SkeletalMesh : ToRemove)
    {
        ActiveRagdolls.Remove(SkeletalMesh);
    }
}

void UCore_RagdollPhysicsManager::InitializeRagdollPhysics(USkeletalMeshComponent* SkeletalMesh)
{
    if (!SkeletalMesh)
    {
        return;
    }

    // Ensure physics asset is available
    UPhysicsAsset* PhysicsAsset = SkeletalMesh->GetPhysicsAsset();
    if (!PhysicsAsset)
    {
        UE_LOG(LogTemp, Warning, TEXT("No physics asset found for skeletal mesh"));
        return;
    }

    // Set up physics properties
    SkeletalMesh->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
    SkeletalMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
    SkeletalMesh->SetNotifyRigidBodyCollision(true);
}

void UCore_RagdollPhysicsManager::CleanupRagdollPhysics(USkeletalMeshComponent* SkeletalMesh)
{
    if (!SkeletalMesh)
    {
        return;
    }

    // Reset collision settings
    SkeletalMesh->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
    SkeletalMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
    SkeletalMesh->SetNotifyRigidBodyCollision(false);
}

bool UCore_RagdollPhysicsManager::ValidateSkeletalMesh(USkeletalMeshComponent* SkeletalMesh) const
{
    if (!SkeletalMesh || !IsValid(SkeletalMesh))
    {
        return false;
    }

    if (!SkeletalMesh->GetPhysicsAsset())
    {
        UE_LOG(LogTemp, Warning, TEXT("SkeletalMesh has no physics asset"));
        return false;
    }

    return true;
}

void UCore_RagdollPhysicsManager::UpdateRagdollState(USkeletalMeshComponent* SkeletalMesh, float DeltaTime)
{
    FCore_RagdollState* State = ActiveRagdolls.Find(SkeletalMesh);
    if (!State)
    {
        return;
    }

    // Update velocity tracking
    if (SkeletalMesh->GetPhysicsLinearVelocity().SizeSquared() > 0.0f)
    {
        State->LastKnownVelocity = SkeletalMesh->GetPhysicsLinearVelocity();
    }
}