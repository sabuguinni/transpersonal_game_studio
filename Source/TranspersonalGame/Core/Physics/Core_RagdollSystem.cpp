#include "Core_RagdollSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Engine/World.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY(LogRagdollSystem);

UCore_RagdollSystem::UCore_RagdollSystem()
{
    MeshComponent = nullptr;
    bRagdollActive = false;
    CurrentTransition = ECore_RagdollTransition::None;
    RagdollActiveTime = 0.0f;
    OriginalCollisionResponse = ECR_Block;
    bOriginalSimulatePhysics = false;
    
    // Default ragdoll configuration
    RagdollConfig = FCore_RagdollConfig();
}

void UCore_RagdollSystem::InitializeRagdoll(USkeletalMeshComponent* InMeshComponent, const FCore_RagdollConfig& InConfig)
{
    if (!InMeshComponent)
    {
        UE_LOG(LogRagdollSystem, Error, TEXT("InitializeRagdoll: Invalid mesh component"));
        return;
    }

    MeshComponent = InMeshComponent;
    RagdollConfig = InConfig;
    
    // Store original settings
    OriginalCollisionResponse = MeshComponent->GetCollisionResponseToChannel(ECC_Pawn);
    bOriginalSimulatePhysics = MeshComponent->IsSimulatingPhysics();
    
    UE_LOG(LogRagdollSystem, Log, TEXT("Ragdoll system initialized for mesh: %s"), 
           *MeshComponent->GetName());
}

void UCore_RagdollSystem::ActivateRagdoll(ECore_RagdollTransition TransitionType, const FVector& ImpactForce, const FVector& ImpactLocation)
{
    if (!ValidateRagdollSetup())
    {
        UE_LOG(LogRagdollSystem, Error, TEXT("ActivateRagdoll: Invalid ragdoll setup"));
        return;
    }

    if (bRagdollActive)
    {
        UE_LOG(LogRagdollSystem, Warning, TEXT("ActivateRagdoll: Ragdoll already active"));
        return;
    }

    CurrentTransition = TransitionType;
    bRagdollActive = true;
    RagdollActiveTime = 0.0f;

    // Disable animation blueprint
    MeshComponent->SetAnimationMode(EAnimationMode::AnimationSingleNode);
    
    // Enable physics simulation
    MeshComponent->SetSimulatePhysics(true);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, RagdollConfig.CollisionResponse);
    
    // Configure physics bodies
    ConfigurePhysicsBodies();
    
    // Apply transition-specific settings
    ApplyTransitionSettings(TransitionType);
    
    // Apply impact force if provided
    if (!ImpactForce.IsZero())
    {
        ApplyImpactForce(ImpactForce, ImpactLocation);
    }
    
    UE_LOG(LogRagdollSystem, Log, TEXT("Ragdoll activated with transition type: %d"), 
           static_cast<int32>(TransitionType));
}

void UCore_RagdollSystem::DeactivateRagdoll()
{
    if (!bRagdollActive || !MeshComponent)
    {
        return;
    }

    // Restore animation state
    RestoreAnimationState();
    
    bRagdollActive = false;
    CurrentTransition = ECore_RagdollTransition::None;
    RagdollActiveTime = 0.0f;
    
    UE_LOG(LogRagdollSystem, Log, TEXT("Ragdoll deactivated"));
}

void UCore_RagdollSystem::ForceCleanup()
{
    if (!MeshComponent)
    {
        return;
    }

    DeactivateRagdoll();
    
    // Additional cleanup if needed
    if (AActor* Owner = MeshComponent->GetOwner())
    {
        Owner->Destroy();
    }
    
    UE_LOG(LogRagdollSystem, Log, TEXT("Ragdoll force cleanup completed"));
}

void UCore_RagdollSystem::UpdateRagdoll(float DeltaTime)
{
    if (!bRagdollActive)
    {
        return;
    }

    RagdollActiveTime += DeltaTime;
    
    // Handle cleanup timer
    HandleCleanupTimer(DeltaTime);
    
    // Update physics properties if needed
    if (MeshComponent && MeshComponent->IsSimulatingPhysics())
    {
        // Apply continuous damping
        TArray<FName> BoneNames;
        MeshComponent->GetBoneNames(BoneNames);
        
        for (const FName& BoneName : BoneNames)
        {
            if (FBodyInstance* BodyInstance = MeshComponent->GetBodyInstance(BoneName))
            {
                BodyInstance->SetLinearDamping(RagdollConfig.LinearDamping);
                BodyInstance->SetAngularDamping(RagdollConfig.AngularDamping);
                BodyInstance->SetMaxAngularVelocityInRadians(FMath::DegreesToRadians(RagdollConfig.MaxAngularVelocity));
            }
        }
    }
}

void UCore_RagdollSystem::SetRagdollConfig(const FCore_RagdollConfig& NewConfig)
{
    RagdollConfig = NewConfig;
    
    // Apply new configuration if ragdoll is active
    if (bRagdollActive)
    {
        ConfigurePhysicsBodies();
    }
    
    UE_LOG(LogRagdollSystem, Log, TEXT("Ragdoll configuration updated"));
}

void UCore_RagdollSystem::ApplyTransitionSettings(ECore_RagdollTransition TransitionType)
{
    if (!MeshComponent)
    {
        return;
    }

    switch (TransitionType)
    {
        case ECore_RagdollTransition::InstantDeath:
            // Immediate full ragdoll activation
            MeshComponent->SetAllBodiesSimulatePhysics(true);
            break;
            
        case ECore_RagdollTransition::GradualCollapse:
            // Start with upper body, gradually enable lower body
            MeshComponent->SetAllBodiesSimulatePhysics(false);
            // Enable spine and head first
            MeshComponent->SetBodyNotifyRigidBodyCollision(FName("spine_01"), true);
            MeshComponent->SetBodyNotifyRigidBodyCollision(FName("head"), true);
            break;
            
        case ECore_RagdollTransition::ImpactDeath:
            // Full ragdoll with high initial velocity
            MeshComponent->SetAllBodiesSimulatePhysics(true);
            // Impact settings will be applied separately
            break;
            
        case ECore_RagdollTransition::Unconscious:
            // Partial ragdoll - keep some constraint
            MeshComponent->SetAllBodiesSimulatePhysics(true);
            // Reduce mass scale for lighter feel
            break;
            
        default:
            MeshComponent->SetAllBodiesSimulatePhysics(true);
            break;
    }
}

void UCore_RagdollSystem::ConfigurePhysicsBodies()
{
    if (!MeshComponent || !MeshComponent->GetPhysicsAsset())
    {
        return;
    }

    // Configure all physics bodies
    TArray<FName> BoneNames;
    MeshComponent->GetBoneNames(BoneNames);
    
    for (const FName& BoneName : BoneNames)
    {
        if (FBodyInstance* BodyInstance = MeshComponent->GetBodyInstance(BoneName))
        {
            // Apply mass scaling
            BodyInstance->SetMassScale(RagdollConfig.MassScale);
            
            // Apply damping
            BodyInstance->SetLinearDamping(RagdollConfig.LinearDamping);
            BodyInstance->SetAngularDamping(RagdollConfig.AngularDamping);
            
            // Set max angular velocity
            BodyInstance->SetMaxAngularVelocityInRadians(FMath::DegreesToRadians(RagdollConfig.MaxAngularVelocity));
            
            // Enable collision
            BodyInstance->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        }
    }
    
    UE_LOG(LogRagdollSystem, Log, TEXT("Physics bodies configured for %d bones"), BoneNames.Num());
}

void UCore_RagdollSystem::RestoreAnimationState()
{
    if (!MeshComponent)
    {
        return;
    }

    // Disable physics simulation
    MeshComponent->SetSimulatePhysics(bOriginalSimulatePhysics);
    MeshComponent->SetAllBodiesSimulatePhysics(false);
    
    // Restore collision settings
    MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, OriginalCollisionResponse);
    
    // Re-enable animation blueprint
    MeshComponent->SetAnimationMode(EAnimationMode::AnimationBlueprint);
    
    UE_LOG(LogRagdollSystem, Log, TEXT("Animation state restored"));
}

void UCore_RagdollSystem::HandleCleanupTimer(float DeltaTime)
{
    if (RagdollActiveTime >= RagdollConfig.CleanupTime && RagdollConfig.CleanupTime > 0.0f)
    {
        // Time to clean up the ragdoll
        ForceCleanup();
    }
}

void UCore_RagdollSystem::ApplyImpactForce(const FVector& Force, const FVector& Location)
{
    if (!MeshComponent || !MeshComponent->IsSimulatingPhysics())
    {
        return;
    }

    // Apply force to the closest bone
    FName ClosestBone = MeshComponent->FindClosestBone(Location);
    if (ClosestBone != NAME_None)
    {
        MeshComponent->AddImpulseAtLocation(Force, Location, ClosestBone);
        UE_LOG(LogRagdollSystem, Log, TEXT("Impact force applied to bone: %s"), *ClosestBone.ToString());
    }
    else
    {
        // Fallback: apply to root bone
        MeshComponent->AddImpulse(Force);
        UE_LOG(LogRagdollSystem, Log, TEXT("Impact force applied to root"));
    }
}

bool UCore_RagdollSystem::ValidateRagdollSetup() const
{
    if (!MeshComponent)
    {
        UE_LOG(LogRagdollSystem, Error, TEXT("ValidateRagdollSetup: No mesh component"));
        return false;
    }

    if (!MeshComponent->GetPhysicsAsset())
    {
        UE_LOG(LogRagdollSystem, Error, TEXT("ValidateRagdollSetup: No physics asset on mesh component"));
        return false;
    }

    return true;
}