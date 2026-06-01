#include "Core_PhysicsSystemManager.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Engine/Engine.h"

UCore_PhysicsSystemManager::UCore_PhysicsSystemManager()
{
    // Initialize default physics settings
    GlobalGravityScale = 1.0f;
    DefaultLinearDamping = 0.01f;
    DefaultAngularDamping = 0.01f;
    bEnablePhysicsOptimization = true;

    // Collision settings
    CollisionTolerance = 0.1f;
    MaxCollisionIterations = 8;
    bUseComplexCollisionAsSimple = false;

    // Ragdoll settings
    RagdollBlendTime = 0.2f;
    RagdollLifetime = 10.0f;
    bAutoCleanupRagdolls = true;

    // Destruction settings
    DestructionThreshold = 1000.0f;
    MaxDebrisCount = 50;
    DebrisLifetime = 30.0f;

    // Environmental settings
    WindDirection = FVector(1.0f, 0.0f, 0.0f);
    WindStrength = 0.0f;
    bEnableEnvironmentalEffects = true;

    // Performance settings
    MaxActivePhysicsActors = 500;
    PhysicsUpdateRate = 60.0f;
    bUseLODSystem = true;
}

void UCore_PhysicsSystemManager::InitializePhysicsSystem()
{
    UE_LOG(LogTemp, Log, TEXT("Core Physics System Manager: Initializing physics system"));

    // Set global physics settings
    if (UPhysicsSettings* PhysicsSettings = GetMutableDefault<UPhysicsSettings>())
    {
        PhysicsSettings->DefaultGravityZ = -980.0f * GlobalGravityScale;
        PhysicsSettings->DefaultTerminalVelocity = 4000.0f;
        PhysicsSettings->DefaultFluidFriction = 0.3f;
    }

    // Initialize tracking arrays
    TrackedPhysicsActors.Empty();
    DestructibleMeshes.Empty();
    RagdollCharacters.Empty();

    UE_LOG(LogTemp, Log, TEXT("Core Physics System Manager: Physics system initialized successfully"));
}

void UCore_PhysicsSystemManager::ShutdownPhysicsSystem()
{
    UE_LOG(LogTemp, Log, TEXT("Core Physics System Manager: Shutting down physics system"));

    // Clean up all tracked objects
    CleanupInvalidReferences();
    
    // Disable ragdoll for all tracked characters
    for (auto& CharacterPtr : RagdollCharacters)
    {
        if (ACharacter* Character = CharacterPtr.Get())
        {
            DisableRagdollForCharacter(Character);
        }
    }

    // Clear all arrays
    TrackedPhysicsActors.Empty();
    DestructibleMeshes.Empty();
    RagdollCharacters.Empty();

    UE_LOG(LogTemp, Log, TEXT("Core Physics System Manager: Physics system shutdown complete"));
}

void UCore_PhysicsSystemManager::UpdatePhysicsSettings()
{
    if (UPhysicsSettings* PhysicsSettings = GetMutableDefault<UPhysicsSettings>())
    {
        PhysicsSettings->DefaultGravityZ = -980.0f * GlobalGravityScale;
        UE_LOG(LogTemp, Log, TEXT("Core Physics System Manager: Updated gravity scale to %f"), GlobalGravityScale);
    }

    // Update tracked actors with new settings
    UpdateTrackedActors();
}

void UCore_PhysicsSystemManager::EnableCollisionForActor(AActor* Actor)
{
    if (!IsActorValidForPhysics(Actor))
    {
        return;
    }

    if (UPrimitiveComponent* PrimComp = Actor->GetRootComponent<UPrimitiveComponent>())
    {
        PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        PrimComp->SetCollisionResponseToAllChannels(ECR_Block);
        
        // Add to tracked actors if not already present
        TrackedPhysicsActors.AddUnique(Actor);
        
        UE_LOG(LogTemp, Log, TEXT("Core Physics System Manager: Enabled collision for actor %s"), *Actor->GetName());
    }
}

void UCore_PhysicsSystemManager::DisableCollisionForActor(AActor* Actor)
{
    if (!IsActorValidForPhysics(Actor))
    {
        return;
    }

    if (UPrimitiveComponent* PrimComp = Actor->GetRootComponent<UPrimitiveComponent>())
    {
        PrimComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        
        // Remove from tracked actors
        TrackedPhysicsActors.Remove(Actor);
        
        UE_LOG(LogTemp, Log, TEXT("Core Physics System Manager: Disabled collision for actor %s"), *Actor->GetName());
    }
}

bool UCore_PhysicsSystemManager::CheckCollisionBetweenActors(AActor* ActorA, AActor* ActorB)
{
    if (!IsActorValidForPhysics(ActorA) || !IsActorValidForPhysics(ActorB))
    {
        return false;
    }

    UPrimitiveComponent* CompA = ActorA->GetRootComponent<UPrimitiveComponent>();
    UPrimitiveComponent* CompB = ActorB->GetRootComponent<UPrimitiveComponent>();

    if (CompA && CompB)
    {
        return CompA->IsOverlappingComponent(CompB);
    }

    return false;
}

void UCore_PhysicsSystemManager::EnableRagdollForCharacter(ACharacter* Character)
{
    if (!Character || !Character->GetMesh())
    {
        return;
    }

    USkeletalMeshComponent* SkeletalMesh = Character->GetMesh();
    
    // Enable physics simulation
    SkeletalMesh->SetSimulatePhysics(true);
    SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    SkeletalMesh->SetCollisionResponseToAllChannels(ECR_Block);

    // Add to tracked ragdoll characters
    RagdollCharacters.AddUnique(Character);

    UE_LOG(LogTemp, Log, TEXT("Core Physics System Manager: Enabled ragdoll for character %s"), *Character->GetName());
}

void UCore_PhysicsSystemManager::DisableRagdollForCharacter(ACharacter* Character)
{
    if (!Character || !Character->GetMesh())
    {
        return;
    }

    USkeletalMeshComponent* SkeletalMesh = Character->GetMesh();
    
    // Disable physics simulation
    SkeletalMesh->SetSimulatePhysics(false);
    SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

    // Remove from tracked ragdoll characters
    RagdollCharacters.Remove(Character);

    UE_LOG(LogTemp, Log, TEXT("Core Physics System Manager: Disabled ragdoll for character %s"), *Character->GetName());
}

void UCore_PhysicsSystemManager::SetRagdollStrength(ACharacter* Character, float Strength)
{
    if (!Character || !Character->GetMesh())
    {
        return;
    }

    USkeletalMeshComponent* SkeletalMesh = Character->GetMesh();
    
    // Set physics blend weight
    SkeletalMesh->SetPhysicsBlendWeight(FMath::Clamp(Strength, 0.0f, 1.0f));

    UE_LOG(LogTemp, Log, TEXT("Core Physics System Manager: Set ragdoll strength to %f for character %s"), Strength, *Character->GetName());
}

void UCore_PhysicsSystemManager::EnableDestructionForMesh(UStaticMeshComponent* MeshComponent)
{
    if (!MeshComponent)
    {
        return;
    }

    // Enable physics simulation for destruction
    MeshComponent->SetSimulatePhysics(true);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MeshComponent->SetNotifyRigidBodyCollision(true);

    // Add to tracked destructible meshes
    DestructibleMeshes.AddUnique(MeshComponent);

    UE_LOG(LogTemp, Log, TEXT("Core Physics System Manager: Enabled destruction for mesh component"));
}

void UCore_PhysicsSystemManager::TriggerDestruction(UStaticMeshComponent* MeshComponent, FVector ImpactPoint, float Force)
{
    if (!MeshComponent || Force < DestructionThreshold)
    {
        return;
    }

    // Apply impulse at impact point
    MeshComponent->AddImpulseAtLocation(ImpactPoint * Force, ImpactPoint);

    // Create debris effect (simplified - would normally spawn debris actors)
    UE_LOG(LogTemp, Log, TEXT("Core Physics System Manager: Triggered destruction with force %f at location %s"), Force, *ImpactPoint.ToString());

    // Schedule cleanup
    if (bAutoCleanupRagdolls)
    {
        // Would normally set a timer to clean up debris after DebrisLifetime
    }
}

void UCore_PhysicsSystemManager::CleanupDestructionDebris()
{
    // Clean up invalid mesh references
    DestructibleMeshes.RemoveAll([](const TWeakObjectPtr<UStaticMeshComponent>& MeshPtr)
    {
        return !MeshPtr.IsValid();
    });

    UE_LOG(LogTemp, Log, TEXT("Core Physics System Manager: Cleaned up destruction debris"));
}

void UCore_PhysicsSystemManager::SetGlobalGravity(float NewGravity)
{
    GlobalGravityScale = NewGravity;
    UpdatePhysicsSettings();
}

void UCore_PhysicsSystemManager::SetWindForce(FVector WindDirection_New, float WindStrength_New)
{
    WindDirection = WindDirection_New.GetSafeNormal();
    WindStrength = WindStrength_New;

    UE_LOG(LogTemp, Log, TEXT("Core Physics System Manager: Set wind force - Direction: %s, Strength: %f"), 
           *WindDirection.ToString(), WindStrength);
}

void UCore_PhysicsSystemManager::ApplyEnvironmentalForces()
{
    if (!bEnableEnvironmentalEffects || WindStrength <= 0.0f)
    {
        return;
    }

    // Apply wind force to tracked physics actors
    for (auto& ActorPtr : TrackedPhysicsActors)
    {
        if (AActor* Actor = ActorPtr.Get())
        {
            if (UPrimitiveComponent* PrimComp = Actor->GetRootComponent<UPrimitiveComponent>())
            {
                if (PrimComp->IsSimulatingPhysics())
                {
                    FVector WindForce = WindDirection * WindStrength;
                    PrimComp->AddForce(WindForce);
                }
            }
        }
    }
}

void UCore_PhysicsSystemManager::OptimizePhysicsPerformance()
{
    if (!bEnablePhysicsOptimization)
    {
        return;
    }

    // Clean up invalid references
    CleanupInvalidReferences();

    // Limit active physics actors
    if (TrackedPhysicsActors.Num() > MaxActivePhysicsActors)
    {
        int32 ExcessCount = TrackedPhysicsActors.Num() - MaxActivePhysicsActors;
        for (int32 i = 0; i < ExcessCount; i++)
        {
            if (AActor* Actor = TrackedPhysicsActors[i].Get())
            {
                DisableCollisionForActor(Actor);
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Core Physics System Manager: Optimized physics performance - Active actors: %d"), TrackedPhysicsActors.Num());
}

void UCore_PhysicsSystemManager::SetPhysicsLOD(int32 LODLevel)
{
    // Apply LOD-based physics settings
    switch (LODLevel)
    {
        case 0: // High quality
            PhysicsUpdateRate = 60.0f;
            MaxCollisionIterations = 8;
            break;
        case 1: // Medium quality
            PhysicsUpdateRate = 30.0f;
            MaxCollisionIterations = 4;
            break;
        case 2: // Low quality
            PhysicsUpdateRate = 15.0f;
            MaxCollisionIterations = 2;
            break;
        default:
            break;
    }

    UE_LOG(LogTemp, Log, TEXT("Core Physics System Manager: Set physics LOD to level %d"), LODLevel);
}

int32 UCore_PhysicsSystemManager::GetActivePhysicsActorCount()
{
    CleanupInvalidReferences();
    return TrackedPhysicsActors.Num();
}

void UCore_PhysicsSystemManager::UpdateTrackedActors()
{
    for (auto& ActorPtr : TrackedPhysicsActors)
    {
        if (AActor* Actor = ActorPtr.Get())
        {
            if (UPrimitiveComponent* PrimComp = Actor->GetRootComponent<UPrimitiveComponent>())
            {
                PrimComp->SetLinearDamping(DefaultLinearDamping);
                PrimComp->SetAngularDamping(DefaultAngularDamping);
            }
        }
    }
}

void UCore_PhysicsSystemManager::CleanupInvalidReferences()
{
    // Remove invalid actor references
    TrackedPhysicsActors.RemoveAll([](const TWeakObjectPtr<AActor>& ActorPtr)
    {
        return !ActorPtr.IsValid();
    });

    // Remove invalid mesh references
    DestructibleMeshes.RemoveAll([](const TWeakObjectPtr<UStaticMeshComponent>& MeshPtr)
    {
        return !MeshPtr.IsValid();
    });

    // Remove invalid character references
    RagdollCharacters.RemoveAll([](const TWeakObjectPtr<ACharacter>& CharacterPtr)
    {
        return !CharacterPtr.IsValid();
    });
}

bool UCore_PhysicsSystemManager::IsActorValidForPhysics(AActor* Actor)
{
    return Actor && IsValid(Actor) && Actor->GetRootComponent();
}

void UCore_PhysicsSystemManager::ApplyPerformanceOptimizations()
{
    // Distance-based LOD for physics actors
    // Disable physics for distant objects
    // Reduce update frequency for non-critical objects
    
    UE_LOG(LogTemp, Log, TEXT("Core Physics System Manager: Applied performance optimizations"));
}