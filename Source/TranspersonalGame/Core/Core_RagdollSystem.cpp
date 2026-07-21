#include "Core_RagdollSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Animation/AnimInstance.h"
#include "Kismet/KismetMathLibrary.h"

UCore_RagdollSystem::UCore_RagdollSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize default values
    GlobalMassMultiplier = 1.0f;
    GlobalForceMultiplier = 1.0f;
    GlobalDampingMultiplier = 1.0f;
    
    bEnableConstraintProjection = true;
    ConstraintProjectionLinearTolerance = 5.0f;
    ConstraintProjectionAngularTolerance = 10.0f;
    
    DeathTwitchingDuration = 3.0f;
    DeathTwitchingIntensity = 0.5f;
    TwitchingFrequency = 2.0f;
    
    RecoveryBlendTime = 2.0f;
    MaxRecoveryAngle = 45.0f;
    MinRecoveryVelocity = 50.0f;
    
    TwitchingTimer = 0.0f;
    TwitchingPhase = 0.0f;
    TwitchingMesh = nullptr;
}

void UCore_RagdollSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Core_RagdollSystem: BeginPlay - Ragdoll physics system initialized"));
    
    // Clear arrays
    ActiveRagdolls.Empty();
    RecoveringMeshes.Empty();
}

void UCore_RagdollSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update death twitching effects
    if (TwitchingMesh && TwitchingTimer > 0.0f)
    {
        UpdateDeathTwitching(DeltaTime);
    }
    
    // Update recovery processes
    if (RecoveringMeshes.Num() > 0)
    {
        UpdateRecovery(DeltaTime);
    }
}

void UCore_RagdollSystem::ActivateRagdoll(USkeletalMeshComponent* SkeletalMesh, bool bPreserveVelocity)
{
    if (!SkeletalMesh)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_RagdollSystem: ActivateRagdoll - SkeletalMesh is null"));
        return;
    }
    
    // Store current velocity if needed
    FVector CurrentVelocity = FVector::ZeroVector;
    if (bPreserveVelocity && SkeletalMesh->GetOwner())
    {
        CurrentVelocity = SkeletalMesh->GetOwner()->GetVelocity();
    }
    
    // Set collision to ragdoll
    SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    SkeletalMesh->SetCollisionResponseToAllChannels(ECR_Block);
    
    // Enable physics simulation
    SkeletalMesh->SetSimulatePhysics(true);
    SkeletalMesh->SetAllBodiesSimulatePhysics(true);
    
    // Configure physics properties
    if (UPhysicsAsset* PhysicsAsset = SkeletalMesh->GetPhysicsAsset())
    {
        // Apply global multipliers to all bodies
        for (int32 BodyIndex = 0; BodyIndex < PhysicsAsset->SkeletalBodySetups.Num(); BodyIndex++)
        {
            if (UBodySetup* BodySetup = PhysicsAsset->SkeletalBodySetups[BodyIndex])
            {
                FName BoneName = BodySetup->BoneName;
                
                // Get body instance
                FBodyInstance* BodyInstance = SkeletalMesh->GetBodyInstance(BoneName);
                if (BodyInstance)
                {
                    // Apply mass multiplier
                    BodyInstance->SetMassScale(GlobalMassMultiplier);
                    
                    // Apply damping
                    BodyInstance->LinearDamping *= GlobalDampingMultiplier;
                    BodyInstance->AngularDamping *= GlobalDampingMultiplier;
                    
                    // Enable constraint projection if configured
                    if (bEnableConstraintProjection)
                    {
                        BodyInstance->bEnableGravity = true;
                    }
                }
            }
        }
    }
    
    // Apply preserved velocity
    if (bPreserveVelocity && !CurrentVelocity.IsZero())
    {
        SkeletalMesh->SetAllPhysicsLinearVelocity(CurrentVelocity * GlobalForceMultiplier);
    }
    
    // Add to active ragdolls
    ActiveRagdolls.AddUnique(SkeletalMesh);
    
    UE_LOG(LogTemp, Warning, TEXT("Core_RagdollSystem: Ragdoll activated for %s"), 
           SkeletalMesh->GetOwner() ? *SkeletalMesh->GetOwner()->GetName() : TEXT("Unknown"));
}

void UCore_RagdollSystem::DeactivateRagdoll(USkeletalMeshComponent* SkeletalMesh)
{
    if (!SkeletalMesh)
    {
        return;
    }
    
    // Disable physics simulation
    SkeletalMesh->SetSimulatePhysics(false);
    SkeletalMesh->SetAllBodiesSimulatePhysics(false);
    
    // Reset collision
    SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    SkeletalMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
    SkeletalMesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
    
    // Remove from active ragdolls
    ActiveRagdolls.Remove(SkeletalMesh);
    RecoveringMeshes.Remove(SkeletalMesh);
    
    // Stop twitching if this was the twitching mesh
    if (TwitchingMesh == SkeletalMesh)
    {
        TwitchingMesh = nullptr;
        TwitchingTimer = 0.0f;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Core_RagdollSystem: Ragdoll deactivated for %s"), 
           SkeletalMesh->GetOwner() ? *SkeletalMesh->GetOwner()->GetName() : TEXT("Unknown"));
}

bool UCore_RagdollSystem::IsRagdollActive(USkeletalMeshComponent* SkeletalMesh) const
{
    return ActiveRagdolls.Contains(SkeletalMesh);
}

void UCore_RagdollSystem::ApplyImpactToBone(USkeletalMeshComponent* SkeletalMesh, const FString& BoneName, const FVector& ImpactForce, const FVector& ImpactLocation)
{
    if (!SkeletalMesh || BoneName.IsEmpty())
    {
        return;
    }
    
    FName BoneNameFName(*BoneName);
    
    // Apply impulse at location
    FVector ScaledForce = ImpactForce * GlobalForceMultiplier;
    SkeletalMesh->AddImpulseAtLocation(ScaledForce, ImpactLocation, BoneNameFName);
    
    UE_LOG(LogTemp, Warning, TEXT("Core_RagdollSystem: Impact applied to bone %s with force %s"), 
           *BoneName, *ScaledForce.ToString());
}

void UCore_RagdollSystem::ApplyRadialImpulse(USkeletalMeshComponent* SkeletalMesh, const FVector& Origin, float Radius, float Strength)
{
    if (!SkeletalMesh)
    {
        return;
    }
    
    float ScaledStrength = Strength * GlobalForceMultiplier;
    SkeletalMesh->AddRadialImpulse(Origin, Radius, ScaledStrength, ERadialImpulseFalloff::RIF_Linear, true);
    
    UE_LOG(LogTemp, Warning, TEXT("Core_RagdollSystem: Radial impulse applied at %s with strength %f"), 
           *Origin.ToString(), ScaledStrength);
}

void UCore_RagdollSystem::SimulateDeathImpact(USkeletalMeshComponent* SkeletalMesh, const FVector& DeathDirection, float ImpactStrength)
{
    if (!SkeletalMesh)
    {
        return;
    }
    
    // Activate ragdoll first
    ActivateRagdoll(SkeletalMesh, true);
    
    // Apply death impact to spine/chest area
    FVector NormalizedDirection = DeathDirection.GetSafeNormal();
    FVector ImpactForce = NormalizedDirection * ImpactStrength * GlobalForceMultiplier;
    
    // Try common spine bone names
    TArray<FString> SpineBones = {TEXT("spine_02"), TEXT("spine_01"), TEXT("Spine2"), TEXT("Spine1"), TEXT("chest")};
    
    for (const FString& BoneName : SpineBones)
    {
        if (SkeletalMesh->GetBoneIndex(*BoneName) != INDEX_NONE)
        {
            FVector BoneLocation = SkeletalMesh->GetBoneLocation(*BoneName);
            ApplyImpactToBone(SkeletalMesh, BoneName, ImpactForce, BoneLocation);
            break;
        }
    }
    
    // Start death twitching effect
    StartDeathTwitching(SkeletalMesh, DeathTwitchingDuration, DeathTwitchingIntensity);
    
    UE_LOG(LogTemp, Warning, TEXT("Core_RagdollSystem: Death impact simulated with direction %s and strength %f"), 
           *DeathDirection.ToString(), ImpactStrength);
}

void UCore_RagdollSystem::ConfigureBonePhysics(USkeletalMeshComponent* SkeletalMesh, const FString& BoneName, float Mass, float LinearDamping, float AngularDamping)
{
    if (!SkeletalMesh || BoneName.IsEmpty())
    {
        return;
    }
    
    FName BoneNameFName(*BoneName);
    FBodyInstance* BodyInstance = SkeletalMesh->GetBodyInstance(BoneNameFName);
    
    if (BodyInstance)
    {
        BodyInstance->SetMassScale(Mass * GlobalMassMultiplier);
        BodyInstance->LinearDamping = LinearDamping * GlobalDampingMultiplier;
        BodyInstance->AngularDamping = AngularDamping * GlobalDampingMultiplier;
        
        UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Configured physics for bone %s"), *BoneName);
    }
}

void UCore_RagdollSystem::SetPhysicsMultipliers(float MassMultiplier, float ForceMultiplier, float DampingMultiplier)
{
    GlobalMassMultiplier = MassMultiplier;
    GlobalForceMultiplier = ForceMultiplier;
    GlobalDampingMultiplier = DampingMultiplier;
    
    UE_LOG(LogTemp, Warning, TEXT("Core_RagdollSystem: Physics multipliers set - Mass: %f, Force: %f, Damping: %f"), 
           MassMultiplier, ForceMultiplier, DampingMultiplier);
}

void UCore_RagdollSystem::SetConstraintLimits(USkeletalMeshComponent* SkeletalMesh, bool bEnableProjection, float ProjectionLinearTolerance, float ProjectionAngularTolerance)
{
    bEnableConstraintProjection = bEnableProjection;
    ConstraintProjectionLinearTolerance = ProjectionLinearTolerance;
    ConstraintProjectionAngularTolerance = ProjectionAngularTolerance;
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Constraint limits configured"));
}

void UCore_RagdollSystem::StartDeathTwitching(USkeletalMeshComponent* SkeletalMesh, float Duration, float Intensity)
{
    if (!SkeletalMesh)
    {
        return;
    }
    
    TwitchingMesh = SkeletalMesh;
    TwitchingTimer = Duration;
    DeathTwitchingIntensity = Intensity;
    TwitchingPhase = 0.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("Core_RagdollSystem: Death twitching started for %f seconds with intensity %f"), 
           Duration, Intensity);
}

void UCore_RagdollSystem::StopProceduralEffects()
{
    TwitchingMesh = nullptr;
    TwitchingTimer = 0.0f;
    TwitchingPhase = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: All procedural effects stopped"));
}

void UCore_RagdollSystem::AttemptRecovery(USkeletalMeshComponent* SkeletalMesh, float RecoveryTime)
{
    if (!SkeletalMesh || !CanRecover(SkeletalMesh))
    {
        return;
    }
    
    RecoveringMeshes.AddUnique(SkeletalMesh);
    RecoveryBlendTime = RecoveryTime;
    
    UE_LOG(LogTemp, Warning, TEXT("Core_RagdollSystem: Recovery attempt started for %s"), 
           SkeletalMesh->GetOwner() ? *SkeletalMesh->GetOwner()->GetName() : TEXT("Unknown"));
}

bool UCore_RagdollSystem::CanRecover(USkeletalMeshComponent* SkeletalMesh) const
{
    if (!SkeletalMesh || !IsRagdollActive(SkeletalMesh))
    {
        return false;
    }
    
    // Check if the mesh is relatively stable (low velocity)
    FVector CurrentVelocity = SkeletalMesh->GetPhysicsLinearVelocity();
    if (CurrentVelocity.Size() > MinRecoveryVelocity)
    {
        return false;
    }
    
    // Check if the mesh is not too tilted
    FRotator CurrentRotation = SkeletalMesh->GetComponentRotation();
    if (FMath::Abs(CurrentRotation.Pitch) > MaxRecoveryAngle || FMath::Abs(CurrentRotation.Roll) > MaxRecoveryAngle)
    {
        return false;
    }
    
    return true;
}

void UCore_RagdollSystem::UpdateDeathTwitching(float DeltaTime)
{
    if (!TwitchingMesh)
    {
        return;
    }
    
    TwitchingTimer -= DeltaTime;
    TwitchingPhase += DeltaTime * TwitchingFrequency;
    
    if (TwitchingTimer <= 0.0f)
    {
        // Stop twitching
        TwitchingMesh = nullptr;
        TwitchingTimer = 0.0f;
        return;
    }
    
    // Apply random twitching impulses
    if (FMath::Sin(TwitchingPhase) > 0.8f)
    {
        ApplyTwitchImpulse(TwitchingMesh);
    }
}

void UCore_RagdollSystem::UpdateRecovery(float DeltaTime)
{
    // Simple recovery update - in a full implementation, this would blend back to animation
    for (int32 i = RecoveringMeshes.Num() - 1; i >= 0; i--)
    {
        USkeletalMeshComponent* RecoveringMesh = RecoveringMeshes[i];
        if (!RecoveringMesh)
        {
            RecoveringMeshes.RemoveAt(i);
            continue;
        }
        
        // For now, just deactivate ragdoll after recovery time
        // In a full implementation, this would gradually blend back to animation
        DeactivateRagdoll(RecoveringMesh);
        RecoveringMeshes.RemoveAt(i);
    }
}

void UCore_RagdollSystem::ApplyTwitchImpulse(USkeletalMeshComponent* SkeletalMesh)
{
    if (!SkeletalMesh)
    {
        return;
    }
    
    // Generate random impulse
    FVector RandomDirection = FVector(
        FMath::RandRange(-1.0f, 1.0f),
        FMath::RandRange(-1.0f, 1.0f),
        FMath::RandRange(0.0f, 1.0f)
    ).GetSafeNormal();
    
    float ImpulseStrength = DeathTwitchingIntensity * FMath::RandRange(50.0f, 200.0f);
    FVector TwitchImpulse = RandomDirection * ImpulseStrength;
    
    // Apply to a random bone
    TArray<FString> TwitchBones = {TEXT("spine_01"), TEXT("upperarm_l"), TEXT("upperarm_r"), TEXT("thigh_l"), TEXT("thigh_r")};
    if (TwitchBones.Num() > 0)
    {
        FString RandomBone = TwitchBones[FMath::RandRange(0, TwitchBones.Num() - 1)];
        if (SkeletalMesh->GetBoneIndex(*RandomBone) != INDEX_NONE)
        {
            FVector BoneLocation = SkeletalMesh->GetBoneLocation(*RandomBone);
            ApplyImpactToBone(SkeletalMesh, RandomBone, TwitchImpulse, BoneLocation);
        }
    }
}

float UCore_RagdollSystem::GetBoneMassForCreature(USkeletalMeshComponent* SkeletalMesh, const FString& BoneName) const
{
    // Base mass calculation based on creature size and bone type
    float BaseMass = 1.0f;
    
    if (SkeletalMesh && SkeletalMesh->GetOwner())
    {
        FVector ActorScale = SkeletalMesh->GetOwner()->GetActorScale3D();
        float ScaleFactor = (ActorScale.X + ActorScale.Y + ActorScale.Z) / 3.0f;
        BaseMass *= ScaleFactor;
    }
    
    // Adjust mass based on bone type
    if (BoneName.Contains(TEXT("spine")) || BoneName.Contains(TEXT("chest")))
    {
        BaseMass *= 3.0f; // Heavy torso
    }
    else if (BoneName.Contains(TEXT("head")))
    {
        BaseMass *= 2.0f; // Heavy head
    }
    else if (BoneName.Contains(TEXT("thigh")) || BoneName.Contains(TEXT("upperarm")))
    {
        BaseMass *= 1.5f; // Medium limbs
    }
    
    return BaseMass * GlobalMassMultiplier;
}

FVector UCore_RagdollSystem::CalculateRecoveryPosition(USkeletalMeshComponent* SkeletalMesh) const
{
    if (!SkeletalMesh)
    {
        return FVector::ZeroVector;
    }
    
    // Calculate optimal recovery position based on current ragdoll state
    FVector CurrentLocation = SkeletalMesh->GetComponentLocation();
    FVector GroundLocation = CurrentLocation;
    GroundLocation.Z = 0.0f; // Simplified ground detection
    
    return GroundLocation;
}