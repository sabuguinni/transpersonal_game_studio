#include "Core_PhysicsSystemManager.h"
#include "Core_CollisionSystem.h"
#include "Core_RagdollSystem.h"
#include "Core_DestructionSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "DrawDebugHelpers.h"

UCore_PhysicsSystemManager::UCore_PhysicsSystemManager()
{
    CollisionSystem = nullptr;
    RagdollSystem = nullptr;
    DestructionSystem = nullptr;
    GlobalPhysicsMultiplier = 1.0f;
    bEnablePhysicsDebug = false;
    bAutoSetupDinosaurPhysics = true;
    bSystemsInitialized = false;
}

void UCore_PhysicsSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Initializing..."));
    
    InitializePhysicsSystems();
    RegisterPhysicsEvents();
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Initialization complete"));
}

void UCore_PhysicsSystemManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Deinitializing..."));
    
    UnregisterPhysicsEvents();
    ShutdownPhysicsSystems();
    
    Super::Deinitialize();
}

void UCore_PhysicsSystemManager::InitializePhysicsSystems()
{
    if (bSystemsInitialized)
    {
        return;
    }

    // Create collision system
    CollisionSystem = NewObject<UCore_CollisionSystem>(this);
    if (CollisionSystem)
    {
        CollisionSystem->InitializeCollisionSystem();
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Collision system initialized"));
    }

    // Create ragdoll system
    RagdollSystem = NewObject<UCore_RagdollSystem>(this);
    if (RagdollSystem)
    {
        RagdollSystem->InitializeRagdollSystem();
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Ragdoll system initialized"));
    }

    // Create destruction system
    DestructionSystem = NewObject<UCore_DestructionSystem>(this);
    if (DestructionSystem)
    {
        DestructionSystem->InitializeDestructionSystem();
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Destruction system initialized"));
    }

    bSystemsInitialized = true;
}

void UCore_PhysicsSystemManager::ShutdownPhysicsSystems()
{
    if (!bSystemsInitialized)
    {
        return;
    }

    if (DestructionSystem)
    {
        DestructionSystem->ShutdownDestructionSystem();
        DestructionSystem = nullptr;
    }

    if (RagdollSystem)
    {
        RagdollSystem->ShutdownRagdollSystem();
        RagdollSystem = nullptr;
    }

    if (CollisionSystem)
    {
        CollisionSystem->ShutdownCollisionSystem();
        CollisionSystem = nullptr;
    }

    TrackedPhysicsActors.Empty();
    bSystemsInitialized = false;
}

void UCore_PhysicsSystemManager::SetGlobalPhysicsMultiplier(float Multiplier)
{
    GlobalPhysicsMultiplier = FMath::Clamp(Multiplier, 0.1f, 10.0f);
    
    // Apply to all systems
    if (CollisionSystem)
    {
        CollisionSystem->SetPhysicsMultiplier(GlobalPhysicsMultiplier);
    }
    
    if (RagdollSystem)
    {
        RagdollSystem->SetPhysicsMultiplier(GlobalPhysicsMultiplier);
    }
    
    if (DestructionSystem)
    {
        DestructionSystem->SetPhysicsMultiplier(GlobalPhysicsMultiplier);
    }
}

void UCore_PhysicsSystemManager::EnablePhysicsForActor(AActor* Actor, bool bEnableRagdoll)
{
    if (!Actor || !IsValid(Actor))
    {
        return;
    }

    // Enable physics on static mesh components
    UStaticMeshComponent* StaticMeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
    if (StaticMeshComp)
    {
        StaticMeshComp->SetSimulatePhysics(true);
        StaticMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        StaticMeshComp->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
        StaticMeshComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
    }

    // Enable ragdoll on skeletal mesh components if requested
    if (bEnableRagdoll)
    {
        USkeletalMeshComponent* SkeletalMeshComp = Actor->FindComponentByClass<USkeletalMeshComponent>();
        if (SkeletalMeshComp && RagdollSystem)
        {
            RagdollSystem->EnableRagdoll(SkeletalMeshComp);
        }
    }

    // Track this actor
    TrackedPhysicsActors.AddUnique(Actor);

    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Physics enabled for actor %s"), *Actor->GetName());
}

void UCore_PhysicsSystemManager::DisablePhysicsForActor(AActor* Actor)
{
    if (!Actor || !IsValid(Actor))
    {
        return;
    }

    // Disable physics on static mesh components
    UStaticMeshComponent* StaticMeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
    if (StaticMeshComp)
    {
        StaticMeshComp->SetSimulatePhysics(false);
        StaticMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    }

    // Disable ragdoll on skeletal mesh components
    USkeletalMeshComponent* SkeletalMeshComp = Actor->FindComponentByClass<USkeletalMeshComponent>();
    if (SkeletalMeshComp && RagdollSystem)
    {
        RagdollSystem->DisableRagdoll(SkeletalMeshComp);
    }

    // Remove from tracking
    TrackedPhysicsActors.RemoveSingle(Actor);

    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Physics disabled for actor %s"), *Actor->GetName());
}

void UCore_PhysicsSystemManager::ApplyImpulseToActor(AActor* Actor, FVector Impulse, FVector Location)
{
    if (!Actor || !IsValid(Actor))
    {
        return;
    }

    // Apply impulse scaled by global multiplier
    FVector ScaledImpulse = Impulse * GlobalPhysicsMultiplier;

    UStaticMeshComponent* StaticMeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
    if (StaticMeshComp && StaticMeshComp->IsSimulatingPhysics())
    {
        StaticMeshComp->AddImpulseAtLocation(ScaledImpulse, Location);
        
        // Broadcast impact event
        OnPhysicsImpact.Broadcast(Actor, ScaledImpulse.Size());
    }

    USkeletalMeshComponent* SkeletalMeshComp = Actor->FindComponentByClass<USkeletalMeshComponent>();
    if (SkeletalMeshComp && SkeletalMeshComp->IsSimulatingPhysics())
    {
        SkeletalMeshComp->AddImpulseAtLocation(ScaledImpulse, Location);
        
        // Broadcast impact event
        OnPhysicsImpact.Broadcast(Actor, ScaledImpulse.Size());
    }
}

void UCore_PhysicsSystemManager::SetupDinosaurPhysics(AActor* DinosaurActor, float Mass)
{
    if (!DinosaurActor || !IsValid(DinosaurActor))
    {
        return;
    }

    // Check if this is a dinosaur by name
    FString ActorName = DinosaurActor->GetName().ToLower();
    bool bIsDinosaur = ActorName.Contains(TEXT("trex")) || 
                      ActorName.Contains(TEXT("raptor")) || 
                      ActorName.Contains(TEXT("brachi")) || 
                      ActorName.Contains(TEXT("tricera")) ||
                      ActorName.Contains(TEXT("dinosaur"));

    if (!bIsDinosaur)
    {
        return;
    }

    // Setup physics properties
    UStaticMeshComponent* MeshComp = DinosaurActor->FindComponentByClass<UStaticMeshComponent>();
    if (MeshComp)
    {
        MeshComp->SetSimulatePhysics(true);
        MeshComp->SetMassOverrideInKg(NAME_None, Mass, true);
        MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        MeshComp->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
        MeshComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
        MeshComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
        
        // Set linear and angular damping for realistic movement
        MeshComp->SetLinearDamping(0.5f);
        MeshComp->SetAngularDamping(0.8f);
    }

    // Setup collision system integration
    if (CollisionSystem)
    {
        CollisionSystem->RegisterActor(DinosaurActor);
    }

    // Track this dinosaur
    TrackedPhysicsActors.AddUnique(DinosaurActor);

    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Dinosaur physics setup for %s (Mass: %.1f kg)"), 
           *DinosaurActor->GetName(), Mass);
}

void UCore_PhysicsSystemManager::TriggerDinosaurRagdoll(AActor* DinosaurActor, FVector ImpactForce)
{
    if (!DinosaurActor || !IsValid(DinosaurActor) || !RagdollSystem)
    {
        return;
    }

    USkeletalMeshComponent* SkeletalMeshComp = DinosaurActor->FindComponentByClass<USkeletalMeshComponent>();
    if (SkeletalMeshComp)
    {
        RagdollSystem->EnableRagdoll(SkeletalMeshComp);
        RagdollSystem->ApplyRagdollImpulse(SkeletalMeshComp, ImpactForce, FVector::ZeroVector);
        
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Ragdoll triggered for %s"), *DinosaurActor->GetName());
    }
}

void UCore_PhysicsSystemManager::DebugShowPhysicsInfo()
{
    if (!GEngine)
    {
        return;
    }

    FString DebugInfo = FString::Printf(TEXT("=== PHYSICS SYSTEM INFO ===\n"));
    DebugInfo += FString::Printf(TEXT("Systems Initialized: %s\n"), bSystemsInitialized ? TEXT("YES") : TEXT("NO"));
    DebugInfo += FString::Printf(TEXT("Global Physics Multiplier: %.2f\n"), GlobalPhysicsMultiplier);
    DebugInfo += FString::Printf(TEXT("Tracked Physics Actors: %d\n"), TrackedPhysicsActors.Num());
    DebugInfo += FString::Printf(TEXT("Collision System: %s\n"), CollisionSystem ? TEXT("ACTIVE") : TEXT("NULL"));
    DebugInfo += FString::Printf(TEXT("Ragdoll System: %s\n"), RagdollSystem ? TEXT("ACTIVE") : TEXT("NULL"));
    DebugInfo += FString::Printf(TEXT("Destruction System: %s\n"), DestructionSystem ? TEXT("ACTIVE") : TEXT("NULL"));

    GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, DebugInfo);
    UE_LOG(LogTemp, Warning, TEXT("%s"), *DebugInfo);
}

void UCore_PhysicsSystemManager::TogglePhysicsDebugDraw(bool bEnabled)
{
    bEnablePhysicsDebug = bEnabled;
    
    if (CollisionSystem)
    {
        CollisionSystem->SetDebugDrawEnabled(bEnabled);
    }
    
    if (RagdollSystem)
    {
        RagdollSystem->SetDebugDrawEnabled(bEnabled);
    }
    
    if (DestructionSystem)
    {
        DestructionSystem->SetDebugDrawEnabled(bEnabled);
    }
}

void UCore_PhysicsSystemManager::RegisterPhysicsEvents()
{
    // Register for actor hit events
    if (UWorld* World = GetWorld())
    {
        // We'll handle this in the collision system
    }
}

void UCore_PhysicsSystemManager::UnregisterPhysicsEvents()
{
    // Cleanup event bindings
    OnPhysicsImpact.Clear();
    OnActorDestroyed.Clear();
}

void UCore_PhysicsSystemManager::HandleActorHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit)
{
    if (!SelfActor || !OtherActor)
    {
        return;
    }

    float ImpactForce = NormalImpulse.Size();
    
    // Broadcast impact event
    OnPhysicsImpact.Broadcast(SelfActor, ImpactForce);
    
    // Handle destruction if force is high enough
    if (DestructionSystem && ImpactForce > 1000.0f)
    {
        DestructionSystem->ProcessImpact(SelfActor, ImpactForce, Hit.ImpactPoint);
    }
    
    // Handle ragdoll activation for dinosaurs
    if (RagdollSystem && ImpactForce > 500.0f)
    {
        FString ActorName = SelfActor->GetName().ToLower();
        bool bIsDinosaur = ActorName.Contains(TEXT("trex")) || 
                          ActorName.Contains(TEXT("raptor")) || 
                          ActorName.Contains(TEXT("brachi")) || 
                          ActorName.Contains(TEXT("tricera"));
        
        if (bIsDinosaur)
        {
            TriggerDinosaurRagdoll(SelfActor, NormalImpulse);
        }
    }
}