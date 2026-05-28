#include "Core_RagdollSystem.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"
#include "Engine/Engine.h"

UCore_RagdollSystem::UCore_RagdollSystem()
{
    // Default ragdoll physics settings
    DefaultRagdollMass = 100.0f;
    DefaultLinearDamping = 0.1f;
    DefaultAngularDamping = 0.1f;
    bEnableGravity = true;

    // Dinosaur mass settings (realistic prehistoric weights)
    TRexMass = 7000.0f;        // 7 tons
    VelociraptorMass = 15.0f;  // 15 kg
    BrachiosaurusMass = 50000.0f; // 50 tons

    // Initialize dinosaur mass overrides
    DinosaurMassOverrides.Add(TEXT("TRex"), TRexMass);
    DinosaurMassOverrides.Add(TEXT("Velociraptor"), VelociraptorMass);
    DinosaurMassOverrides.Add(TEXT("Brachiosaurus"), BrachiosaurusMass);
    DinosaurMassOverrides.Add(TEXT("Triceratops"), 12000.0f); // 12 tons
    DinosaurMassOverrides.Add(TEXT("Ankylosaurus"), 6000.0f); // 6 tons
}

void UCore_RagdollSystem::EnableRagdoll(USkeletalMeshComponent* SkeletalMesh)
{
    if (!SkeletalMesh || !SkeletalMesh->GetPhysicsAsset())
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_RagdollSystem: Cannot enable ragdoll - invalid mesh or physics asset"));
        return;
    }

    // Store current animation state
    StoreAnimationState(SkeletalMesh);

    // Enable physics simulation
    SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    SkeletalMesh->SetSimulatePhysics(true);
    SkeletalMesh->SetCollisionResponseToAllChannels(ECR_Block);

    // Configure ragdoll physics properties
    ConfigureRagdollPhysics(SkeletalMesh);

    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Ragdoll enabled for %s"), 
           SkeletalMesh->GetOwner() ? *SkeletalMesh->GetOwner()->GetName() : TEXT("Unknown"));
}

void UCore_RagdollSystem::DisableRagdoll(USkeletalMeshComponent* SkeletalMesh)
{
    if (!SkeletalMesh)
    {
        return;
    }

    // Disable physics simulation
    SkeletalMesh->SetSimulatePhysics(false);
    SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

    // Restore animation state
    RestoreAnimationState(SkeletalMesh);

    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Ragdoll disabled for %s"), 
           SkeletalMesh->GetOwner() ? *SkeletalMesh->GetOwner()->GetName() : TEXT("Unknown"));
}

bool UCore_RagdollSystem::IsRagdollActive(USkeletalMeshComponent* SkeletalMesh) const
{
    if (!SkeletalMesh)
    {
        return false;
    }

    return SkeletalMesh->IsSimulatingPhysics();
}

void UCore_RagdollSystem::EnableDinosaurRagdoll(ACharacter* DinosaurCharacter, float ImpactForce)
{
    if (!DinosaurCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_RagdollSystem: Invalid dinosaur character"));
        return;
    }

    USkeletalMeshComponent* SkeletalMesh = DinosaurCharacter->GetMesh();
    if (!SkeletalMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_RagdollSystem: Dinosaur has no skeletal mesh"));
        return;
    }

    // Determine dinosaur type from actor name for mass override
    FString ActorName = DinosaurCharacter->GetName();
    float DinosaurMass = DefaultRagdollMass;

    for (const auto& MassOverride : DinosaurMassOverrides)
    {
        if (ActorName.Contains(MassOverride.Key))
        {
            DinosaurMass = MassOverride.Value;
            break;
        }
    }

    // Set dinosaur-specific properties before enabling ragdoll
    SetDinosaurRagdollProperties(SkeletalMesh, DinosaurMass, DefaultLinearDamping);

    // Enable ragdoll
    EnableRagdoll(SkeletalMesh);

    // Apply impact force if specified
    if (ImpactForce > 0.0f)
    {
        FVector ImpactDirection = DinosaurCharacter->GetActorForwardVector();
        ApplyRagdollImpulse(SkeletalMesh, ImpactDirection * ImpactForce, TEXT("spine_01"));
    }

    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Dinosaur ragdoll enabled - %s (Mass: %.1f)"), 
           *ActorName, DinosaurMass);
}

void UCore_RagdollSystem::SetDinosaurRagdollProperties(USkeletalMeshComponent* SkeletalMesh, float Mass, float Damping)
{
    if (!SkeletalMesh || !SkeletalMesh->GetPhysicsAsset())
    {
        return;
    }

    // Set mass and damping for all bodies in the physics asset
    TArray<FName> BoneNames;
    SkeletalMesh->GetBoneNames(BoneNames);

    for (const FName& BoneName : BoneNames)
    {
        if (SkeletalMesh->GetBodyInstance(BoneName))
        {
            FBodyInstance* BodyInstance = SkeletalMesh->GetBodyInstance(BoneName);
            if (BodyInstance)
            {
                BodyInstance->SetMassOverride(Mass / BoneNames.Num()); // Distribute mass
                BodyInstance->LinearDamping = Damping;
                BodyInstance->AngularDamping = Damping;
                BodyInstance->bEnableGravity = bEnableGravity;
            }
        }
    }
}

void UCore_RagdollSystem::ApplyRagdollImpulse(USkeletalMeshComponent* SkeletalMesh, FVector Impulse, FName BoneName)
{
    if (!SkeletalMesh || !IsRagdollActive(SkeletalMesh))
    {
        return;
    }

    if (BoneName == NAME_None)
    {
        // Apply to root body
        SkeletalMesh->AddImpulse(Impulse);
    }
    else
    {
        // Apply to specific bone
        SkeletalMesh->AddImpulseAtLocation(Impulse, SkeletalMesh->GetBoneLocation(BoneName), BoneName);
    }

    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Applied impulse %.1f to bone %s"), 
           Impulse.Size(), *BoneName.ToString());
}

void UCore_RagdollSystem::ApplyRadialImpulse(USkeletalMeshComponent* SkeletalMesh, FVector Origin, float Radius, float Strength)
{
    if (!SkeletalMesh || !IsRagdollActive(SkeletalMesh))
    {
        return;
    }

    SkeletalMesh->AddRadialImpulse(Origin, Radius, Strength, RIF_Linear, true);

    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Applied radial impulse - Radius: %.1f, Strength: %.1f"), 
           Radius, Strength);
}

void UCore_RagdollSystem::ConfigureRagdollPhysics(USkeletalMeshComponent* SkeletalMesh)
{
    if (!SkeletalMesh)
    {
        return;
    }

    // Set collision properties
    SkeletalMesh->SetCollisionObjectType(ECC_Pawn);
    SkeletalMesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
    SkeletalMesh->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
    SkeletalMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

    // Enable complex collision for detailed physics
    SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    SkeletalMesh->SetNotifyRigidBodyCollision(true);
}

void UCore_RagdollSystem::StoreAnimationState(USkeletalMeshComponent* SkeletalMesh)
{
    if (!SkeletalMesh)
    {
        return;
    }

    UAnimInstance* AnimInstance = SkeletalMesh->GetAnimInstance();
    if (AnimInstance)
    {
        StoredAnimInstances.Add(SkeletalMesh, AnimInstance);
    }
}

void UCore_RagdollSystem::RestoreAnimationState(USkeletalMeshComponent* SkeletalMesh)
{
    if (!SkeletalMesh)
    {
        return;
    }

    if (UAnimInstance** StoredAnimInstance = StoredAnimInstances.Find(SkeletalMesh))
    {
        if (*StoredAnimInstance)
        {
            SkeletalMesh->SetAnimInstanceClass((*StoredAnimInstance)->GetClass());
        }
        StoredAnimInstances.Remove(SkeletalMesh);
    }
}