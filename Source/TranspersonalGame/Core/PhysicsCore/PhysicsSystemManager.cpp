// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "PhysicsSystemManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Chaos/ChaosEngineInterface.h"
#include "Chaos/ChaosSolverActor.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/BodySetup.h"
#include "Engine/StaticMesh.h"

DEFINE_LOG_CATEGORY(LogTranspersonalPhysics);

void UPhysicsSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTranspersonalPhysics, Log, TEXT("Physics System Manager initializing..."));
    
    // Initialize physics actor tracking
    PhysicsActorsByType.Empty();
    PhysicsActorsByType.Add(TEXT("Dinosaur"), TArray<TObjectPtr<AActor>>());
    PhysicsActorsByType.Add(TEXT("Environment"), TArray<TObjectPtr<AActor>>());
    PhysicsActorsByType.Add(TEXT("Destructible"), TArray<TObjectPtr<AActor>>());
    PhysicsActorsByType.Add(TEXT("Ragdoll"), TArray<TObjectPtr<AActor>>());
    
    // Setup physics LOD distances
    PhysicsLODDistances.Add(TEXT("Dinosaur"), 2000.0f);
    PhysicsLODDistances.Add(TEXT("Environment"), 5000.0f);
    PhysicsLODDistances.Add(TEXT("Destructible"), 1500.0f);
    PhysicsLODDistances.Add(TEXT("Ragdoll"), 1000.0f);
    
    bPhysicsSimulationEnabled = true;
}

void UPhysicsSystemManager::Deinitialize()
{
    UE_LOG(LogTranspersonalPhysics, Log, TEXT("Physics System Manager deinitializing..."));
    
    // Clear timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(PhysicsUpdateTimer);
        World->GetTimerManager().ClearTimer(PhysicsLODTimer);
    }
    
    // Clear tracked actors
    PhysicsActorsByType.Empty();
    PhysicsLODDistances.Empty();
    
    Super::Deinitialize();
}

void UPhysicsSystemManager::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    
    UE_LOG(LogTranspersonalPhysics, Log, TEXT("Physics System Manager starting world play..."));
    
    // Initialize Chaos Physics for the world
    InitializeChaosPhysics();
    
    // Configure physics for dinosaur simulation
    ConfigureDinosaurPhysics();
    
    // Setup performance monitoring
    InWorld.GetTimerManager().SetTimer(PhysicsUpdateTimer, this, &UPhysicsSystemManager::UpdatePhysicsMetrics, 0.1f, true);
    InWorld.GetTimerManager().SetTimer(PhysicsLODTimer, this, &UPhysicsSystemManager::ApplyPhysicsLOD, 0.5f, true);
}

UPhysicsSystemManager* UPhysicsSystemManager::Get(const UObject* WorldContext)
{
    if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull))
    {
        return World->GetSubsystem<UPhysicsSystemManager>();
    }
    return nullptr;
}

void UPhysicsSystemManager::InitializeChaosPhysics()
{
    UE_LOG(LogTranspersonalPhysics, Log, TEXT("Initializing Chaos Physics for large-scale simulation..."));
    
    // Get physics settings
    UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get();
    if (!PhysicsSettings)
    {
        UE_LOG(LogTranspersonalPhysics, Error, TEXT("Failed to get Physics Settings!"));
        return;
    }
    
    // Configure Chaos for large world simulation
    PhysicsSettings->bTickPhysicsAsync = true;
    PhysicsSettings->bSubstepping = true;
    PhysicsSettings->MaxSubstepDeltaTime = 0.016f; // 60 FPS substeps
    PhysicsSettings->MaxSubsteps = 4;
    
    // Chaos-specific settings for performance
    PhysicsSettings->ChaosSolverConfiguration.Iterations = 4;
    PhysicsSettings->ChaosSolverConfiguration.CollisionPairIterations = 2;
    PhysicsSettings->ChaosSolverConfiguration.PushOutIterations = 2;
    PhysicsSettings->ChaosSolverConfiguration.CollisionPushOutPairIterations = 1;
    
    // Collision settings for large creatures
    PhysicsSettings->ChaosSolverConfiguration.CollisionMarginFraction = 0.1f;
    PhysicsSettings->ChaosSolverConfiguration.CollisionMarginMax = 50.0f;
    PhysicsSettings->ChaosSolverConfiguration.CollisionCullDistance = 100.0f;
    
    UE_LOG(LogTranspersonalPhysics, Log, TEXT("Chaos Physics initialized successfully"));
}

void UPhysicsSystemManager::ConfigureDinosaurPhysics()
{
    UE_LOG(LogTranspersonalPhysics, Log, TEXT("Configuring physics for dinosaur simulation..."));
    
    UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get();
    if (!PhysicsSettings)
    {
        return;
    }
    
    // Configure gravity for realistic dinosaur movement
    PhysicsSettings->DefaultGravityZ = -980.0f; // Standard Earth gravity
    
    // Set bounce threshold for realistic collisions
    PhysicsSettings->BounceThresholdVelocity = 200.0f;
    
    // Configure friction and restitution combining
    PhysicsSettings->FrictionCombineMode = EFrictionCombineMode::Average;
    PhysicsSettings->RestitutionCombineMode = ERestitutionCombineMode::Average;
    
    // Set maximum velocities for stability
    PhysicsSettings->MaxAngularVelocity = 3600.0f; // Degrees per second
    PhysicsSettings->MaxDepenetrationVelocity = 1000.0f;
    
    UE_LOG(LogTranspersonalPhysics, Log, TEXT("Dinosaur physics configuration complete"));
}

void UPhysicsSystemManager::EnableDestructionSystem()
{
    UE_LOG(LogTranspersonalPhysics, Log, TEXT("Enabling destruction system..."));
    
    // Enable geometry collection support
    UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get();
    if (PhysicsSettings)
    {
        PhysicsSettings->ChaosSolverConfiguration.bGenerateCollisionData = true;
        PhysicsSettings->ChaosSolverConfiguration.bGenerateBreakData = true;
        PhysicsSettings->ChaosSolverConfiguration.bGenerateTrailingData = true;
        PhysicsSettings->ChaosSolverConfiguration.bGenerateContactGraph = true;
        
        // Set thresholds for break events
        PhysicsSettings->ChaosSolverConfiguration.MinMassThreshold = 0.1f;
        PhysicsSettings->ChaosSolverConfiguration.MinSpeedThreshold = 50.0f;
        PhysicsSettings->ChaosSolverConfiguration.MinImpulseThreshold = 100.0f;
    }
    
    UE_LOG(LogTranspersonalPhysics, Log, TEXT("Destruction system enabled"));
}

void UPhysicsSystemManager::SetupRagdollSystem()
{
    UE_LOG(LogTranspersonalPhysics, Log, TEXT("Setting up ragdoll system..."));
    
    // Configure physics settings for ragdoll simulation
    UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get();
    if (PhysicsSettings)
    {
        // Enable contact modification for better ragdoll stability
        PhysicsSettings->bEnablePCM = true;
        PhysicsSettings->bEnableStabilization = true;
        
        // Set appropriate simulation settings
        PhysicsSettings->bSimulateSkeletalMeshOnDedicatedServer = true;
    }
    
    UE_LOG(LogTranspersonalPhysics, Log, TEXT("Ragdoll system setup complete"));
}

void UPhysicsSystemManager::ConfigureCollisionSystem()
{
    UE_LOG(LogTranspersonalPhysics, Log, TEXT("Configuring collision system for large world..."));
    
    UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get();
    if (PhysicsSettings)
    {
        // Optimize collision detection
        PhysicsSettings->bDisableActiveActors = false;
        PhysicsSettings->bDisableKinematicStaticPairs = true;
        PhysicsSettings->bDisableKinematicKinematicPairs = true;
        
        // Enable shape sharing for performance
        PhysicsSettings->bEnableShapeSharing = true;
        
        // Configure contact generation
        PhysicsSettings->ContactOffsetMultiplier = 0.02f;
        PhysicsSettings->MinContactOffset = 2.0f;
        PhysicsSettings->MaxContactOffset = 8.0f;
    }
    
    UE_LOG(LogTranspersonalPhysics, Log, TEXT("Collision system configuration complete"));
}

void UPhysicsSystemManager::RegisterPhysicsActor(AActor* Actor, const FString& PhysicsType)
{
    if (!Actor)
    {
        UE_LOG(LogTranspersonalPhysics, Warning, TEXT("Attempted to register null actor"));
        return;
    }
    
    if (TArray<TObjectPtr<AActor>>* ActorArray = PhysicsActorsByType.Find(PhysicsType))
    {
        ActorArray->AddUnique(Actor);
        UE_LOG(LogTranspersonalPhysics, VeryVerbose, TEXT("Registered actor %s as %s"), *Actor->GetName(), *PhysicsType);
    }
    else
    {
        UE_LOG(LogTranspersonalPhysics, Warning, TEXT("Unknown physics type: %s"), *PhysicsType);
    }
}

void UPhysicsSystemManager::UnregisterPhysicsActor(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }
    
    for (auto& ActorTypePair : PhysicsActorsByType)
    {
        ActorTypePair.Value.Remove(Actor);
    }
    
    UE_LOG(LogTranspersonalPhysics, VeryVerbose, TEXT("Unregistered actor %s"), *Actor->GetName());
}

void UPhysicsSystemManager::GetPhysicsMetrics(int32& ActiveBodies, float& SimulationTime, int32& CollisionPairs) const
{
    ActiveBodies = ActivePhysicsBodies;
    SimulationTime = LastSimulationTime;
    CollisionPairs = LastCollisionPairs;
}

void UPhysicsSystemManager::SetPhysicsSimulationEnabled(bool bEnabled)
{
    bPhysicsSimulationEnabled = bEnabled;
    
    UE_LOG(LogTranspersonalPhysics, Log, TEXT("Physics simulation %s"), 
           bEnabled ? TEXT("enabled") : TEXT("disabled"));
    
    // Apply to all registered actors
    for (const auto& ActorTypePair : PhysicsActorsByType)
    {
        for (AActor* Actor : ActorTypePair.Value)
        {
            if (Actor && IsValid(Actor))
            {
                if (UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>())
                {
                    PrimComp->SetSimulatePhysics(bEnabled);
                }
            }
        }
    }
}

void UPhysicsSystemManager::SetupPhysicsLOD()
{
    UE_LOG(LogTranspersonalPhysics, Log, TEXT("Setting up Physics LOD system..."));
    
    // Physics LOD will be applied via timer in ApplyPhysicsLOD()
    UE_LOG(LogTranspersonalPhysics, Log, TEXT("Physics LOD system ready"));
}

void UPhysicsSystemManager::UpdatePhysicsMetrics()
{
    if (!bPhysicsSimulationEnabled)
    {
        return;
    }
    
    // Count active physics bodies
    ActivePhysicsBodies = 0;
    LastCollisionPairs = 0;
    
    for (const auto& ActorTypePair : PhysicsActorsByType)
    {
        for (AActor* Actor : ActorTypePair.Value)
        {
            if (Actor && IsValid(Actor))
            {
                if (UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>())
                {
                    if (PrimComp->IsSimulatingPhysics())
                    {
                        ActivePhysicsBodies++;
                    }
                }
            }
        }
    }
    
    // Update simulation time (simplified - would need actual physics solver access)
    LastSimulationTime = GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.0f;
}

void UPhysicsSystemManager::ApplyPhysicsLOD()
{
    if (!bPhysicsSimulationEnabled)
    {
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Get player location for distance calculations
    FVector PlayerLocation = FVector::ZeroVector;
    if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0))
    {
        PlayerLocation = PlayerPawn->GetActorLocation();
    }
    
    // Apply LOD to each physics type
    for (const auto& ActorTypePair : PhysicsActorsByType)
    {
        const FString& PhysicsType = ActorTypePair.Key;
        const float* LODDistance = PhysicsLODDistances.Find(PhysicsType);
        
        if (!LODDistance)
        {
            continue;
        }
        
        for (AActor* Actor : ActorTypePair.Value)
        {
            if (!Actor || !IsValid(Actor))
            {
                continue;
            }
            
            float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
            bool bShouldSimulate = Distance <= *LODDistance;
            
            if (UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>())
            {
                if (PrimComp->IsSimulatingPhysics() != bShouldSimulate)
                {
                    PrimComp->SetSimulatePhysics(bShouldSimulate);
                }
            }
        }
    }
}

// UDestructionSystemComponent Implementation

UDestructionSystemComponent::UDestructionSystemComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    DestructionThreshold = 1000.0f;
    MaxFragments = 50;
    bCanBeDestroyed = true;
}

void UDestructionSystemComponent::BeginPlay()
{
    Super::BeginPlay();
    RegisterWithPhysicsSystem();
}

void UDestructionSystemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UnregisterFromPhysicsSystem();
    Super::EndPlay(EndPlayReason);
}

void UDestructionSystemComponent::SetupDestruction(float NewDestructionThreshold, int32 NewMaxFragments)
{
    DestructionThreshold = NewDestructionThreshold;
    MaxFragments = FMath::Clamp(NewMaxFragments, 1, 1000);
    
    UE_LOG(LogTranspersonalPhysics, Log, TEXT("Destruction setup: Threshold=%.1f, MaxFragments=%d"), 
           DestructionThreshold, MaxFragments);
}

void UDestructionSystemComponent::TriggerDestruction(const FVector& ImpactLocation, float ImpactForce)
{
    if (!bCanBeDestroyed || ImpactForce < DestructionThreshold)
    {
        return;
    }
    
    UE_LOG(LogTranspersonalPhysics, Log, TEXT("Triggering destruction at %s with force %.1f"), 
           *ImpactLocation.ToString(), ImpactForce);
    
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }
    
    // Find geometry collection component
    if (UGeometryCollectionComponent* GeomComp = Owner->FindComponentByClass<UGeometryCollectionComponent>())
    {
        // Apply impulse at impact location
        GeomComp->ApplyPhysicsField(true, EFieldPhysicsType::Field_LinearVelocity, nullptr, nullptr);
    }
    
    // Spawn destruction effects
    if (DestructionEffect)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), DestructionEffect, ImpactLocation);
    }
    
    if (DestructionSound)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), DestructionSound, ImpactLocation);
    }
}

void UDestructionSystemComponent::RegisterWithPhysicsSystem()
{
    if (UPhysicsSystemManager* PhysicsManager = UPhysicsSystemManager::Get(this))
    {
        PhysicsManager->RegisterPhysicsActor(GetOwner(), TEXT("Destructible"));
    }
}

void UDestructionSystemComponent::UnregisterFromPhysicsSystem()
{
    if (UPhysicsSystemManager* PhysicsManager = UPhysicsSystemManager::Get(this))
    {
        PhysicsManager->UnregisterPhysicsActor(GetOwner());
    }
}

// URagdollSystemComponent Implementation

URagdollSystemComponent::URagdollSystemComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    bRagdollActive = false;
    RagdollMass = 1.0f;
    LinearDamping = 0.1f;
    AngularDamping = 0.1f;
    AutoDeactivateTime = 10.0f;
}

void URagdollSystemComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache skeletal mesh component
    SkeletalMeshComponent = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
    
    RegisterWithPhysicsSystem();
}

void URagdollSystemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(AutoDeactivateTimer);
    }
    
    UnregisterFromPhysicsSystem();
    Super::EndPlay(EndPlayReason);
}

void URagdollSystemComponent::ActivateRagdoll(const FVector& ImpulseLocation, float ImpulseStrength)
{
    if (bRagdollActive || !SkeletalMeshComponent)
    {
        return;
    }
    
    UE_LOG(LogTranspersonalPhysics, Log, TEXT("Activating ragdoll for %s"), *GetOwner()->GetName());
    
    // Enable physics simulation
    SkeletalMeshComponent->SetSimulatePhysics(true);
    SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    
    // Apply impulse if specified
    if (ImpulseStrength > 0.0f && ImpulseLocation != FVector::ZeroVector)
    {
        SkeletalMeshComponent->AddImpulseAtLocation(
            (ImpulseLocation - SkeletalMeshComponent->GetComponentLocation()).GetSafeNormal() * ImpulseStrength,
            ImpulseLocation
        );
    }
    
    bRagdollActive = true;
    
    // Set auto-deactivate timer
    if (AutoDeactivateTime > 0.0f)
    {
        GetWorld()->GetTimerManager().SetTimer(AutoDeactivateTimer, this, 
                                               &URagdollSystemComponent::AutoDeactivateRagdoll, 
                                               AutoDeactivateTime, false);
    }
}

void URagdollSystemComponent::DeactivateRagdoll()
{
    if (!bRagdollActive || !SkeletalMeshComponent)
    {
        return;
    }
    
    UE_LOG(LogTranspersonalPhysics, Log, TEXT("Deactivating ragdoll for %s"), *GetOwner()->GetName());
    
    // Disable physics simulation
    SkeletalMeshComponent->SetSimulatePhysics(false);
    SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    
    bRagdollActive = false;
    
    // Clear timer
    GetWorld()->GetTimerManager().ClearTimer(AutoDeactivateTimer);
}

void URagdollSystemComponent::ConfigureRagdoll(float Mass, float NewLinearDamping, float NewAngularDamping)
{
    RagdollMass = Mass;
    LinearDamping = NewLinearDamping;
    AngularDamping = NewAngularDamping;
    
    if (SkeletalMeshComponent && bRagdollActive)
    {
        // Apply settings to all bodies
        SkeletalMeshComponent->SetAllBodiesBelowLinearDamping(NAME_None, LinearDamping, true);
        SkeletalMeshComponent->SetAllBodiesBelowAngularDamping(NAME_None, AngularDamping, true);
    }
}

void URagdollSystemComponent::RegisterWithPhysicsSystem()
{
    if (UPhysicsSystemManager* PhysicsManager = UPhysicsSystemManager::Get(this))
    {
        PhysicsManager->RegisterPhysicsActor(GetOwner(), TEXT("Ragdoll"));
    }
}

void URagdollSystemComponent::UnregisterFromPhysicsSystem()
{
    if (UPhysicsSystemManager* PhysicsManager = UPhysicsSystemManager::Get(this))
    {
        PhysicsManager->UnregisterPhysicsActor(GetOwner());
    }
}

void URagdollSystemComponent::AutoDeactivateRagdoll()
{
    UE_LOG(LogTranspersonalPhysics, Log, TEXT("Auto-deactivating ragdoll for %s"), *GetOwner()->GetName());
    DeactivateRagdoll();
}

// UCollisionSystemComponent Implementation

UCollisionSystemComponent::UCollisionSystemComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    CollisionRadius = 100.0f;
    CollisionHeight = 200.0f;
    NearLODDistance = 1000.0f;
    FarLODDistance = 5000.0f;
    CurrentLODLevel = 0;
}

void UCollisionSystemComponent::BeginPlay()
{
    Super::BeginPlay();
    RegisterWithPhysicsSystem();
    
    // Start LOD update timer
    GetWorld()->GetTimerManager().SetTimer(LODUpdateTimer, this, 
                                           &UCollisionSystemComponent::UpdateCollisionLOD, 
                                           1.0f, true);
}

void UCollisionSystemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    GetWorld()->GetTimerManager().ClearTimer(LODUpdateTimer);
    UnregisterFromPhysicsSystem();
    Super::EndPlay(EndPlayReason);
}

void UCollisionSystemComponent::SetupDinosaurCollision(float NewCollisionRadius, float NewCollisionHeight)
{
    CollisionRadius = NewCollisionRadius;
    CollisionHeight = NewCollisionHeight;
    
    UE_LOG(LogTranspersonalPhysics, Log, TEXT("Setup dinosaur collision: Radius=%.1f, Height=%.1f"), 
           CollisionRadius, CollisionHeight);
    
    // Apply to owner's collision components
    if (AActor* Owner = GetOwner())
    {
        if (UPrimitiveComponent* PrimComp = Owner->FindComponentByClass<UPrimitiveComponent>())
        {
            // Set collision response for dinosaur interactions
            PrimComp->SetCollisionResponseToAllChannels(ECR_Block);
            PrimComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
            PrimComp->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Block);
        }
    }
}

void UCollisionSystemComponent::SetupEnvironmentCollision()
{
    UE_LOG(LogTranspersonalPhysics, Log, TEXT("Setup environment collision for %s"), *GetOwner()->GetName());
    
    if (AActor* Owner = GetOwner())
    {
        if (UPrimitiveComponent* PrimComp = Owner->FindComponentByClass<UPrimitiveComponent>())
        {
            // Set collision response for environment
            PrimComp->SetCollisionResponseToAllChannels(ECR_Block);
            PrimComp->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
            PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        }
    }
}

void UCollisionSystemComponent::SetCollisionEnabled(bool bEnabled)
{
    if (AActor* Owner = GetOwner())
    {
        if (UPrimitiveComponent* PrimComp = Owner->FindComponentByClass<UPrimitiveComponent>())
        {
            PrimComp->SetCollisionEnabled(bEnabled ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
        }
    }
}

void UCollisionSystemComponent::SetupCollisionLOD(float NewNearDistance, float NewFarDistance)
{
    NearLODDistance = NewNearDistance;
    FarLODDistance = NewFarDistance;
    
    UE_LOG(LogTranspersonalPhysics, Log, TEXT("Collision LOD setup: Near=%.1f, Far=%.1f"), 
           NearLODDistance, FarLODDistance);
}

FVector UCollisionSystemComponent::GetCollisionBounds() const
{
    return FVector(CollisionRadius * 2.0f, CollisionRadius * 2.0f, CollisionHeight);
}

void UCollisionSystemComponent::RegisterWithPhysicsSystem()
{
    if (UPhysicsSystemManager* PhysicsManager = UPhysicsSystemManager::Get(this))
    {
        PhysicsManager->RegisterPhysicsActor(GetOwner(), TEXT("Environment"));
    }
}

void UCollisionSystemComponent::UnregisterFromPhysicsSystem()
{
    if (UPhysicsSystemManager* PhysicsManager = UPhysicsSystemManager::Get(this))
    {
        PhysicsManager->UnregisterPhysicsActor(GetOwner());
    }
}

void UCollisionSystemComponent::UpdateCollisionLOD()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Get player location
    FVector PlayerLocation = FVector::ZeroVector;
    if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0))
    {
        PlayerLocation = PlayerPawn->GetActorLocation();
    }
    
    // Calculate distance to player
    float Distance = FVector::Dist(PlayerLocation, GetOwner()->GetActorLocation());
    
    // Determine LOD level
    int32 NewLODLevel = 0;
    if (Distance > FarLODDistance)
    {
        NewLODLevel = 2; // Lowest detail
    }
    else if (Distance > NearLODDistance)
    {
        NewLODLevel = 1; // Medium detail
    }
    else
    {
        NewLODLevel = 0; // Highest detail
    }
    
    // Apply LOD if changed
    if (NewLODLevel != CurrentLODLevel)
    {
        CurrentLODLevel = NewLODLevel;
        
        if (AActor* Owner = GetOwner())
        {
            if (UPrimitiveComponent* PrimComp = Owner->FindComponentByClass<UPrimitiveComponent>())
            {
                switch (CurrentLODLevel)
                {
                case 0: // High detail
                    PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                    break;
                case 1: // Medium detail
                    PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
                    break;
                case 2: // Low detail
                    PrimComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                    break;
                }
            }
        }
    }
}