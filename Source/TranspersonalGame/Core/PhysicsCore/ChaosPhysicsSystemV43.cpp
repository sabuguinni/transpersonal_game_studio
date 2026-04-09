#include "ChaosPhysicsSystemV43.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Chaos/ChaosEngineInterface.h"
#include "Field/FieldSystemComponent.h"
#include "Field/FieldSystemActor.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY(LogChaosPhysicsV43);

void UChaosPhysicsSystemV43::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogChaosPhysicsV43, Log, TEXT("Initializing Chaos Physics System V43 for Jurassic Survival Game"));
    
    // Initialize LOD distances for different quality levels
    PhysicsLODDistances.Add(EPhysicsQualityLevel::Low, 5000.0f);      // 50m
    PhysicsLODDistances.Add(EPhysicsQualityLevel::Medium, 2000.0f);   // 20m  
    PhysicsLODDistances.Add(EPhysicsQualityLevel::High, 1000.0f);     // 10m
    PhysicsLODDistances.Add(EPhysicsQualityLevel::Cinematic, 500.0f); // 5m
    
    // Cache original physics settings
    if (UPhysicsSettings* PhysicsSettings = GetMutableDefault<UPhysicsSettings>())
    {
        OriginalGravityZ = PhysicsSettings->DefaultGravityZ;
        OriginalMaxDeltaTime = PhysicsSettings->MaxPhysicsDeltaTime;
        OriginalSubstepping = PhysicsSettings->bSubstepping;
    }
}

void UChaosPhysicsSystemV43::Deinitialize()
{
    UE_LOG(LogChaosPhysicsV43, Log, TEXT("Deinitializing Chaos Physics System V43"));
    
    // Clear all timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(PhysicsUpdateTimer);
        World->GetTimerManager().ClearTimer(PhysicsLODTimer);
        World->GetTimerManager().ClearTimer(PhysicsOptimizationTimer);
    }
    
    // Clear all registered actors
    PhysicsActorsByQuality.Empty();
    PhysicsActorsByType.Empty();
    ActivePhysicsFields.Empty();
    
    Super::Deinitialize();
}

void UChaosPhysicsSystemV43::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    
    UE_LOG(LogChaosPhysicsV43, Log, TEXT("World Begin Play - Setting up physics systems"));
    
    // Initialize all physics subsystems
    InitializeAdvancedChaosPhysics();
    InitializeCollisionSystem();
    InitializeDestructionSystem();
    InitializeRagdollSystem();
    InitializeFluidPhysics();
    SetupAdaptivePhysicsLOD();
    
    // Start performance monitoring
    InWorld.GetTimerManager().SetTimer(PhysicsUpdateTimer, this, 
        &UChaosPhysicsSystemV43::UpdatePhysicsMetrics, 0.1f, true);
    
    InWorld.GetTimerManager().SetTimer(PhysicsLODTimer, this, 
        &UChaosPhysicsSystemV43::ApplyAdaptivePhysicsLOD, 0.5f, true);
        
    InWorld.GetTimerManager().SetTimer(PhysicsOptimizationTimer, this, 
        &UChaosPhysicsSystemV43::OptimizePhysicsPerformance, 2.0f, true);
}

void UChaosPhysicsSystemV43::Tick(float DeltaTime)
{
    LastFrameTime = DeltaTime;
    
    // Monitor performance and adjust quality if needed
    if (DeltaTime > 0.033f) // 30fps threshold
    {
        LowPerformanceFrameCount++;
        if (LowPerformanceFrameCount > 10 && !bInLowPerformanceMode)
        {
            HandleLowPerformanceMode();
        }
    }
    else
    {
        LowPerformanceFrameCount = FMath::Max(0, LowPerformanceFrameCount - 1);
        if (LowPerformanceFrameCount == 0 && bInLowPerformanceMode)
        {
            bInLowPerformanceMode = false;
            UE_LOG(LogChaosPhysicsV43, Log, TEXT("Exiting low performance mode"));
        }
    }
    
    ProcessPhysicsEvents();
}

UChaosPhysicsSystemV43* UChaosPhysicsSystemV43::Get(const UObject* WorldContext)
{
    if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull))
    {
        return World->GetSubsystem<UChaosPhysicsSystemV43>();
    }
    return nullptr;
}

void UChaosPhysicsSystemV43::InitializeAdvancedChaosPhysics()
{
    UE_LOG(LogChaosPhysicsV43, Log, TEXT("Initializing Advanced Chaos Physics for large-scale simulation"));
    
    UPhysicsSettings* PhysicsSettings = GetMutableDefault<UPhysicsSettings>();
    if (!PhysicsSettings)
    {
        UE_LOG(LogChaosPhysicsV43, Error, TEXT("Failed to get Physics Settings"));
        return;
    }
    
    // Optimize for large-scale dinosaur simulation
    PhysicsSettings->DefaultGravityZ = -980.0f; // Realistic gravity
    PhysicsSettings->MaxPhysicsDeltaTime = 0.033f; // 30fps minimum
    PhysicsSettings->bSubstepping = true; // Enable substepping for accuracy
    PhysicsSettings->MaxSubstepDeltaTime = 0.016f; // 60fps substeps
    PhysicsSettings->MaxSubsteps = 4; // Limit substeps for performance
    
    // Chaos-specific settings
    PhysicsSettings->ChaosSettings.DefaultThreadingModel = EChaosThreadingMode::TaskGraph;
    PhysicsSettings->ChaosSettings.DedicatedThreadTickMode = EChaosSolverTickMode::Variable;
    PhysicsSettings->ChaosSettings.Iterations = 4; // Balance accuracy vs performance
    PhysicsSettings->ChaosSettings.PushOutIterations = 2;
    PhysicsSettings->ChaosSettings.CollisionMarginFraction = 0.1f;
    PhysicsSettings->ChaosSettings.CollisionMarginMax = 5.0f; // 5cm max margin
    PhysicsSettings->ChaosSettings.CollisionCullDistance = 100.0f; // 1m cull distance
    
    // Enable advanced features for destruction and effects
    PhysicsSettings->ChaosSettings.bGenerateCollisionData = true;
    PhysicsSettings->ChaosSettings.bGenerateBreakData = true;
    PhysicsSettings->ChaosSettings.bGenerateTrailingData = true;
    
    UE_LOG(LogChaosPhysicsV43, Log, TEXT("Advanced Chaos Physics initialized successfully"));
}

void UChaosPhysicsSystemV43::ConfigureDinosaurPhysicsSettings()
{
    UE_LOG(LogChaosPhysicsV43, Log, TEXT("Configuring physics settings for dinosaur creatures"));
    
    // Create specialized physics materials for dinosaurs
    // This would typically be done through asset creation, but we can set defaults
    
    // Large dinosaur settings (T-Rex, Triceratops)
    PhysicsActorsByType.FindOrAdd(TEXT("LargeDinosaur"));
    
    // Medium dinosaur settings (Raptors, Parasaurolophus)
    PhysicsActorsByType.FindOrAdd(TEXT("MediumDinosaur"));
    
    // Small dinosaur settings (Compsognathus, small herbivores)
    PhysicsActorsByType.FindOrAdd(TEXT("SmallDinosaur"));
    
    // Flying dinosaur settings (Pteranodon)
    PhysicsActorsByType.FindOrAdd(TEXT("FlyingDinosaur"));
    
    UE_LOG(LogChaosPhysicsV43, Log, TEXT("Dinosaur physics categories configured"));
}

void UChaosPhysicsSystemV43::InitializeDestructionSystem()
{
    UE_LOG(LogChaosPhysicsV43, Log, TEXT("Initializing Chaos Destruction System"));
    
    // Configure destruction settings for environmental interaction
    UPhysicsSettings* PhysicsSettings = GetMutableDefault<UPhysicsSettings>();
    if (PhysicsSettings)
    {
        // Enable destruction data generation
        PhysicsSettings->ChaosSettings.bGenerateBreakData = true;
        PhysicsSettings->ChaosSettings.BreakingFilterEnabled = true;
        PhysicsSettings->ChaosSettings.MinVolumeThreshold = 10.0f; // Minimum volume for break events
        
        UE_LOG(LogChaosPhysicsV43, Log, TEXT("Destruction system configured for environmental interaction"));
    }
}

void UChaosPhysicsSystemV43::InitializeRagdollSystem()
{
    UE_LOG(LogChaosPhysicsV43, Log, TEXT("Initializing Ragdoll Physics System"));
    
    // Ragdoll system will be configured per-character
    // This sets up the global settings for ragdoll physics
    
    PhysicsActorsByType.FindOrAdd(TEXT("RagdollCharacter"));
    
    UE_LOG(LogChaosPhysicsV43, Log, TEXT("Ragdoll system initialized"));
}

void UChaosPhysicsSystemV43::InitializeCollisionSystem()
{
    UE_LOG(LogChaosPhysicsV43, Log, TEXT("Initializing Advanced Collision System"));
    
    UPhysicsSettings* PhysicsSettings = GetMutableDefault<UPhysicsSettings>();
    if (PhysicsSettings)
    {
        // Optimize collision detection for large world
        PhysicsSettings->ChaosSettings.CollisionCullDistance = 200.0f; // 2m cull distance
        PhysicsSettings->ChaosSettings.bGenerateCollisionData = true;
        PhysicsSettings->ChaosSettings.CollisionFilterEnabled = true;
        
        // Set collision thresholds for different event types
        PhysicsSettings->ChaosSettings.MinSpeedThreshold = 50.0f; // 50 cm/s minimum for events
        PhysicsSettings->ChaosSettings.MinImpulseThreshold = 100.0f; // Minimum impulse for events
        
        UE_LOG(LogChaosPhysicsV43, Log, TEXT("Advanced collision system configured"));
    }
}

void UChaosPhysicsSystemV43::InitializeFluidPhysics()
{
    UE_LOG(LogChaosPhysicsV43, Log, TEXT("Initializing Fluid Physics System"));
    
    // Fluid physics for water interaction and blood effects
    PhysicsActorsByType.FindOrAdd(TEXT("FluidActor"));
    
    UE_LOG(LogChaosPhysicsV43, Log, TEXT("Fluid physics system initialized"));
}

void UChaosPhysicsSystemV43::RegisterPhysicsActor(AActor* Actor, EPhysicsQualityLevel QualityLevel, const FString& PhysicsType)
{
    if (!Actor)
    {
        UE_LOG(LogChaosPhysicsV43, Warning, TEXT("Attempted to register null actor"));
        return;
    }
    
    // Register by quality level
    PhysicsActorsByQuality.FindOrAdd(QualityLevel).AddUnique(Actor);
    
    // Register by type
    PhysicsActorsByType.FindOrAdd(PhysicsType).AddUnique(Actor);
    
    UE_LOG(LogChaosPhysicsV43, Log, TEXT("Registered actor %s with quality %d and type %s"), 
        *Actor->GetName(), (int32)QualityLevel, *PhysicsType);
}

void UChaosPhysicsSystemV43::UnregisterPhysicsActor(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }
    
    // Remove from all quality levels
    for (auto& QualityPair : PhysicsActorsByQuality)
    {
        QualityPair.Value.Remove(Actor);
    }
    
    // Remove from all types
    for (auto& TypePair : PhysicsActorsByType)
    {
        TypePair.Value.Remove(Actor);
    }
    
    UE_LOG(LogChaosPhysicsV43, Log, TEXT("Unregistered actor %s"), *Actor->GetName());
}

void UChaosPhysicsSystemV43::SetActorPhysicsQuality(AActor* Actor, EPhysicsQualityLevel QualityLevel)
{
    if (!Actor)
    {
        return;
    }
    
    // Remove from current quality level
    for (auto& QualityPair : PhysicsActorsByQuality)
    {
        QualityPair.Value.Remove(Actor);
    }
    
    // Add to new quality level
    PhysicsActorsByQuality.FindOrAdd(QualityLevel).AddUnique(Actor);
    
    // Apply quality settings to actor
    if (UPrimitiveComponent* PrimComp = Actor->GetRootComponent() ? 
        Cast<UPrimitiveComponent>(Actor->GetRootComponent()) : nullptr)
    {
        switch (QualityLevel)
        {
        case EPhysicsQualityLevel::Low:
            PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            break;
        case EPhysicsQualityLevel::Medium:
            PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            PrimComp->SetSimulatePhysics(false);
            break;
        case EPhysicsQualityLevel::High:
        case EPhysicsQualityLevel::Cinematic:
            PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            PrimComp->SetSimulatePhysics(true);
            break;
        }
    }
}

void UChaosPhysicsSystemV43::GetDetailedPhysicsMetrics(int32& ActiveBodies, float& SimulationTime, 
    int32& CollisionPairs, float& MemoryUsage, int32& DestructionEvents) const
{
    ActiveBodies = ActivePhysicsBodies;
    SimulationTime = LastSimulationTime;
    CollisionPairs = LastCollisionPairs;
    MemoryUsage = PhysicsMemoryUsage;
    DestructionEvents = DestructionEventCount;
}

void UChaosPhysicsSystemV43::SetPhysicsSimulationEnabled(bool bEnabled, bool bGracefulTransition)
{
    if (bPhysicsSimulationEnabled == bEnabled)
    {
        return;
    }
    
    bPhysicsSimulationEnabled = bEnabled;
    
    UE_LOG(LogChaosPhysicsV43, Log, TEXT("Physics simulation %s"), 
        bEnabled ? TEXT("enabled") : TEXT("disabled"));
    
    if (bGracefulTransition)
    {
        // Gradually transition physics state
        SetPhysicsTimeDilation(bEnabled ? 1.0f : 0.1f);
    }
    else
    {
        // Immediate transition
        if (UWorld* World = GetWorld())
        {
            World->GetPhysicsScene()->SetIsStaticLoading(!bEnabled);
        }
    }
}

void UChaosPhysicsSystemV43::SetupAdaptivePhysicsLOD()
{
    UE_LOG(LogChaosPhysicsV43, Log, TEXT("Setting up Adaptive Physics LOD system"));
    
    // LOD system will automatically adjust physics quality based on:
    // 1. Distance from player
    // 2. Actor importance
    // 3. Performance budget
    // 4. Visibility
    
    CurrentGlobalQuality = EPhysicsQualityLevel::High;
}

void UChaosPhysicsSystemV43::ForcePhysicsUpdate()
{
    UpdatePhysicsMetrics();
    ApplyAdaptivePhysicsLOD();
    OptimizePhysicsPerformance();
}

void UChaosPhysicsSystemV43::SetPhysicsTimeDilation(float TimeDilation)
{
    PhysicsTimeDilation = FMath::Clamp(TimeDilation, 0.0f, 2.0f);
    
    if (UWorld* World = GetWorld())
    {
        // Apply time dilation to physics simulation
        World->GetWorldSettings()->SetTimeDilation(PhysicsTimeDilation);
    }
    
    UE_LOG(LogChaosPhysicsV43, Log, TEXT("Physics time dilation set to %f"), PhysicsTimeDilation);
}

void UChaosPhysicsSystemV43::CreatePhysicsField(FVector Location, float Radius, float Strength, float Duration)
{
    if (UWorld* World = GetWorld())
    {
        // Create a physics field actor for environmental effects
        AFieldSystemActor* FieldActor = World->SpawnActor<AFieldSystemActor>();
        if (FieldActor)
        {
            FieldActor->SetActorLocation(Location);
            ActivePhysicsFields.Add(FieldActor);
            
            UE_LOG(LogChaosPhysicsV43, Log, TEXT("Created physics field at %s with radius %f"), 
                *Location.ToString(), Radius);
            
            // Schedule cleanup
            FTimerHandle CleanupTimer;
            World->GetTimerManager().SetTimer(CleanupTimer, [this, FieldActor]()
            {
                if (FieldActor && IsValid(FieldActor))
                {
                    ActivePhysicsFields.Remove(FieldActor);
                    FieldActor->Destroy();
                }
            }, Duration, false);
        }
    }
}

void UChaosPhysicsSystemV43::UpdatePhysicsMetrics()
{
    // Update performance metrics
    ActivePhysicsBodies = 0;
    LastCollisionPairs = 0;
    
    // Count active physics bodies
    for (const auto& QualityPair : PhysicsActorsByQuality)
    {
        for (const TObjectPtr<AActor>& Actor : QualityPair.Value)
        {
            if (IsValid(Actor.Get()))
            {
                if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
                {
                    if (PrimComp->IsSimulatingPhysics())
                    {
                        ActivePhysicsBodies++;
                    }
                }
            }
        }
    }
    
    // Estimate memory usage (simplified)
    PhysicsMemoryUsage = ActivePhysicsBodies * 0.5f; // Rough estimate in MB
    
    LastSimulationTime = LastFrameTime;
}

void UChaosPhysicsSystemV43::ApplyAdaptivePhysicsLOD()
{
    if (!bPhysicsSimulationEnabled)
    {
        return;
    }
    
    // Get player location for distance calculations
    FVector PlayerLocation = FVector::ZeroVector;
    if (UWorld* World = GetWorld())
    {
        if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0))
        {
            PlayerLocation = PlayerPawn->GetActorLocation();
        }
    }
    
    // Apply LOD to all registered actors
    for (auto& QualityPair : PhysicsActorsByQuality)
    {
        EPhysicsQualityLevel TargetQuality = QualityPair.Key;
        
        for (const TObjectPtr<AActor>& Actor : QualityPair.Value)
        {
            if (IsValid(Actor.Get()))
            {
                float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerLocation);
                
                // Determine appropriate quality level based on distance
                EPhysicsQualityLevel NewQuality = EPhysicsQualityLevel::Low;
                if (Distance < PhysicsLODDistances[EPhysicsQualityLevel::Cinematic])
                {
                    NewQuality = EPhysicsQualityLevel::Cinematic;
                }
                else if (Distance < PhysicsLODDistances[EPhysicsQualityLevel::High])
                {
                    NewQuality = EPhysicsQualityLevel::High;
                }
                else if (Distance < PhysicsLODDistances[EPhysicsQualityLevel::Medium])
                {
                    NewQuality = EPhysicsQualityLevel::Medium;
                }
                
                // Apply quality if it has changed
                if (NewQuality != TargetQuality)
                {
                    SetActorPhysicsQuality(Actor.Get(), NewQuality);
                }
            }
        }
    }
}

void UChaosPhysicsSystemV43::ProcessPhysicsEvents()
{
    // Process and broadcast physics events
    // This would typically be called from physics callbacks
    
    // Example: Broadcast dinosaur collision events
    // OnPhysicsEvent.Broadcast(EPhysicsEventType::DinosaurCollision, SomeActor, SomeLocation);
}

void UChaosPhysicsSystemV43::OptimizePhysicsPerformance()
{
    // Clean up inactive physics objects
    CleanupInactivePhysicsObjects();
    
    // Adjust global physics settings based on performance
    if (bInLowPerformanceMode)
    {
        // Reduce physics quality globally
        if (UPhysicsSettings* PhysicsSettings = GetMutableDefault<UPhysicsSettings>())
        {
            PhysicsSettings->ChaosSettings.Iterations = 2; // Reduce iterations
            PhysicsSettings->MaxSubsteps = 2; // Reduce substeps
        }
    }
    else
    {
        // Restore normal physics quality
        if (UPhysicsSettings* PhysicsSettings = GetMutableDefault<UPhysicsSettings>())
        {
            PhysicsSettings->ChaosSettings.Iterations = 4; // Normal iterations
            PhysicsSettings->MaxSubsteps = 4; // Normal substeps
        }
    }
}

void UChaosPhysicsSystemV43::HandleLowPerformanceMode()
{
    bInLowPerformanceMode = true;
    
    UE_LOG(LogChaosPhysicsV43, Warning, TEXT("Entering low performance mode - reducing physics quality"));
    
    // Reduce physics quality for distant objects
    for (auto& QualityPair : PhysicsActorsByQuality)
    {
        if (QualityPair.Key == EPhysicsQualityLevel::High || QualityPair.Key == EPhysicsQualityLevel::Cinematic)
        {
            for (const TObjectPtr<AActor>& Actor : QualityPair.Value)
            {
                if (IsValid(Actor.Get()))
                {
                    SetActorPhysicsQuality(Actor.Get(), EPhysicsQualityLevel::Medium);
                }
            }
        }
    }
}

void UChaosPhysicsSystemV43::CleanupInactivePhysicsObjects()
{
    // Remove invalid actors from all collections
    for (auto& QualityPair : PhysicsActorsByQuality)
    {
        QualityPair.Value.RemoveAll([](const TObjectPtr<AActor>& Actor)
        {
            return !IsValid(Actor.Get());
        });
    }
    
    for (auto& TypePair : PhysicsActorsByType)
    {
        TypePair.Value.RemoveAll([](const TObjectPtr<AActor>& Actor)
        {
            return !IsValid(Actor.Get());
        });
    }
    
    // Clean up destroyed physics fields
    ActivePhysicsFields.RemoveAll([](const TObjectPtr<AFieldSystemActor>& Field)
    {
        return !IsValid(Field.Get());
    });
}