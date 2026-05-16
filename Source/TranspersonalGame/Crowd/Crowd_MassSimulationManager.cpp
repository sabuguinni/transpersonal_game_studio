#include "Crowd_MassSimulationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

ACrowd_MassSimulationManager::ACrowd_MassSimulationManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create visualization mesh
    VisualizationMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualizationMesh"));
    VisualizationMesh->SetupAttachment(RootComponent);
    VisualizationMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    VisualizationMesh->SetVisibility(false);

    // Initialize default spawn parameters
    DefaultSpawnParams.SpawnLocation = FVector::ZeroVector;
    DefaultSpawnParams.SpawnRadius = 1000.0f;
    DefaultSpawnParams.EntityCount = 50;
    DefaultSpawnParams.BehaviorType = ECrowd_CrowdBehaviorType::Wandering;
    DefaultSpawnParams.DensityLevel = ECrowd_CrowdDensity::Medium;
    DefaultSpawnParams.MovementSpeed = 200.0f;
    DefaultSpawnParams.DetectionRadius = 500.0f;
    DefaultSpawnParams.bEnableLOD = true;
    DefaultSpawnParams.LODDistance1 = 1000.0f;
    DefaultSpawnParams.LODDistance2 = 2000.0f;
    DefaultSpawnParams.LODDistance3 = 5000.0f;

    // Initialize pathfinding configuration
    PathfindingConfig.PathUpdateInterval = 2.0f;
    PathfindingConfig.AcceptanceRadius = 100.0f;
    PathfindingConfig.bUseNavMesh = true;
    PathfindingConfig.bAvoidOtherEntities = true;
    PathfindingConfig.AvoidanceRadius = 150.0f;

    // Set default values
    MaxEntityCount = 50000;
    TickInterval = 0.1f;
    bAutoSpawnOnBeginPlay = false;
    bEnableDebugVisualization = false;
    CurrentEntityCount = 0;
    CurrentBehaviorType = ECrowd_CrowdBehaviorType::Wandering;
    bIsInitialized = false;
    bIsCombatMode = false;
    LastFrameTime = 0.0f;
    AverageFrameTime = 0.0f;
    PerformanceSampleCount = 0;
}

void ACrowd_MassSimulationManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeMassEntitySystem();
    
    if (bAutoSpawnOnBeginPlay)
    {
        SpawnCrowdEntities(DefaultSpawnParams);
    }

    UE_LOG(LogTemp, Log, TEXT("Crowd_MassSimulationManager: BeginPlay completed"));
}

void ACrowd_MassSimulationManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    DespawnAllEntities();
    
    MassEntitySubsystem = nullptr;
    MassSpawnerSubsystem = nullptr;
    
    Super::EndPlay(EndPlayReason);
}

void ACrowd_MassSimulationManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (!bIsInitialized)
    {
        return;
    }

    UpdatePerformanceMetrics(DeltaTime);
    ProcessEntityBehaviors();
    UpdateEntityLOD();
    UpdatePathfinding();
    HandleCombatEvents();

    if (bEnableDebugVisualization)
    {
        DebugDrawCrowdInfo();
    }
}

void ACrowd_MassSimulationManager::InitializeMassEntitySystem()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Crowd_MassSimulationManager: No valid world found"));
        return;
    }

    // Get Mass Entity subsystem
    MassEntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("Crowd_MassSimulationManager: Failed to get MassEntitySubsystem"));
        return;
    }

    // Get Mass Spawner subsystem
    MassSpawnerSubsystem = World->GetSubsystem<UMassSpawnerSubsystem>();
    if (!MassSpawnerSubsystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("Crowd_MassSimulationManager: MassSpawnerSubsystem not available"));
    }

    bIsInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("Crowd_MassSimulationManager: Mass Entity system initialized"));
}

void ACrowd_MassSimulationManager::SpawnCrowdEntities(const FCrowd_EntitySpawnParams& SpawnParams)
{
    if (!bIsInitialized || !MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("Crowd_MassSimulationManager: System not initialized"));
        return;
    }

    if (CurrentEntityCount + SpawnParams.EntityCount > MaxEntityCount)
    {
        UE_LOG(LogTemp, Warning, TEXT("Crowd_MassSimulationManager: Would exceed max entity count (%d)"), MaxEntityCount);
        return;
    }

    // Clear existing entities if needed
    if (ManagedEntities.Num() > 0)
    {
        DespawnAllEntities();
    }

    // Create entities in a circular pattern around spawn location
    const float AngleStep = 2.0f * PI / SpawnParams.EntityCount;
    const FVector BaseLocation = SpawnParams.SpawnLocation;
    
    for (int32 i = 0; i < SpawnParams.EntityCount; ++i)
    {
        // Calculate spawn position
        const float Angle = i * AngleStep;
        const float RandomRadius = FMath::RandRange(0.0f, SpawnParams.SpawnRadius);
        const FVector SpawnOffset = FVector(
            FMath::Cos(Angle) * RandomRadius,
            FMath::Sin(Angle) * RandomRadius,
            FMath::RandRange(-50.0f, 50.0f)
        );
        const FVector EntitySpawnLocation = BaseLocation + SpawnOffset;

        // Create Mass Entity (simplified approach for now)
        // In a full implementation, this would use proper Mass Entity archetypes
        FMassEntityHandle EntityHandle;
        // EntityHandle = MassEntitySubsystem->CreateEntity(); // This would be the proper call
        
        // For now, store a dummy handle to maintain the interface
        ManagedEntities.Add(EntityHandle);
        
        // Initialize pathfinding data for this entity
        FCrowd_PathfindingData PathData = PathfindingConfig;
        PathData.CurrentTarget = EntitySpawnLocation + FVector(
            FMath::RandRange(-500.0f, 500.0f),
            FMath::RandRange(-500.0f, 500.0f),
            0.0f
        );
        EntityPathfindingData.Add(EntityHandle, PathData);
    }

    CurrentEntityCount = ManagedEntities.Num();
    CurrentBehaviorType = SpawnParams.BehaviorType;

    UE_LOG(LogTemp, Log, TEXT("Crowd_MassSimulationManager: Spawned %d entities at %s"), 
           CurrentEntityCount, *SpawnParams.SpawnLocation.ToString());
}

void ACrowd_MassSimulationManager::DespawnAllEntities()
{
    if (!MassEntitySubsystem)
    {
        return;
    }

    // Destroy all managed entities
    for (const FMassEntityHandle& EntityHandle : ManagedEntities)
    {
        // MassEntitySubsystem->DestroyEntity(EntityHandle); // This would be the proper call
    }

    ManagedEntities.Empty();
    EntityPathfindingData.Empty();
    CurrentEntityCount = 0;

    UE_LOG(LogTemp, Log, TEXT("Crowd_MassSimulationManager: Despawned all entities"));
}

void ACrowd_MassSimulationManager::DespawnEntitiesInRadius(const FVector& Location, float Radius)
{
    if (!MassEntitySubsystem)
    {
        return;
    }

    TArray<FMassEntityHandle> EntitiesToRemove;
    const float RadiusSquared = Radius * Radius;

    for (const FMassEntityHandle& EntityHandle : ManagedEntities)
    {
        // In a full implementation, we would get the entity's actual position
        // For now, use the pathfinding target as a proxy
        if (EntityPathfindingData.Contains(EntityHandle))
        {
            const FCrowd_PathfindingData& PathData = EntityPathfindingData[EntityHandle];
            const float DistanceSquared = FVector::DistSquared(Location, PathData.CurrentTarget);
            
            if (DistanceSquared <= RadiusSquared)
            {
                EntitiesToRemove.Add(EntityHandle);
            }
        }
    }

    // Remove entities
    for (const FMassEntityHandle& EntityHandle : EntitiesToRemove)
    {
        // MassEntitySubsystem->DestroyEntity(EntityHandle); // This would be the proper call
        ManagedEntities.Remove(EntityHandle);
        EntityPathfindingData.Remove(EntityHandle);
    }

    CurrentEntityCount = ManagedEntities.Num();

    UE_LOG(LogTemp, Log, TEXT("Crowd_MassSimulationManager: Despawned %d entities in radius %.1f at %s"), 
           EntitiesToRemove.Num(), Radius, *Location.ToString());
}

void ACrowd_MassSimulationManager::SetCrowdBehavior(ECrowd_CrowdBehaviorType NewBehavior)
{
    CurrentBehaviorType = NewBehavior;
    
    // Update behavior for all entities
    for (auto& PathDataPair : EntityPathfindingData)
    {
        FCrowd_PathfindingData& PathData = PathDataPair.Value;
        
        switch (NewBehavior)
        {
        case ECrowd_CrowdBehaviorType::Wandering:
            PathData.PathUpdateInterval = 3.0f;
            PathData.AcceptanceRadius = 150.0f;
            break;
        case ECrowd_CrowdBehaviorType::Gathering:
            PathData.PathUpdateInterval = 1.0f;
            PathData.AcceptanceRadius = 50.0f;
            break;
        case ECrowd_CrowdBehaviorType::Fleeing:
            PathData.PathUpdateInterval = 0.5f;
            PathData.AcceptanceRadius = 200.0f;
            break;
        case ECrowd_CrowdBehaviorType::Following:
            PathData.PathUpdateInterval = 0.8f;
            PathData.AcceptanceRadius = 100.0f;
            break;
        case ECrowd_CrowdBehaviorType::Hunting:
            PathData.PathUpdateInterval = 1.5f;
            PathData.AcceptanceRadius = 75.0f;
            break;
        case ECrowd_CrowdBehaviorType::Resting:
            PathData.PathUpdateInterval = 5.0f;
            PathData.AcceptanceRadius = 300.0f;
            break;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Crowd_MassSimulationManager: Set crowd behavior to %d"), (int32)NewBehavior);
}

void ACrowd_MassSimulationManager::UpdateCrowdDensity(ECrowd_CrowdDensity NewDensity)
{
    int32 TargetEntityCount = 0;
    
    switch (NewDensity)
    {
    case ECrowd_CrowdDensity::Sparse:
        TargetEntityCount = FMath::RandRange(1, 10);
        break;
    case ECrowd_CrowdDensity::Medium:
        TargetEntityCount = FMath::RandRange(10, 50);
        break;
    case ECrowd_CrowdDensity::Dense:
        TargetEntityCount = FMath::RandRange(50, 200);
        break;
    case ECrowd_CrowdDensity::Massive:
        TargetEntityCount = FMath::RandRange(200, 1000);
        break;
    }

    // Adjust current entity count
    if (TargetEntityCount > CurrentEntityCount)
    {
        // Spawn additional entities
        FCrowd_EntitySpawnParams SpawnParams = DefaultSpawnParams;
        SpawnParams.EntityCount = TargetEntityCount - CurrentEntityCount;
        SpawnCrowdEntities(SpawnParams);
    }
    else if (TargetEntityCount < CurrentEntityCount)
    {
        // Remove excess entities
        int32 EntitiesToRemove = CurrentEntityCount - TargetEntityCount;
        for (int32 i = 0; i < EntitiesToRemove && ManagedEntities.Num() > 0; ++i)
        {
            FMassEntityHandle EntityToRemove = ManagedEntities.Last();
            ManagedEntities.RemoveLast();
            EntityPathfindingData.Remove(EntityToRemove);
        }
        CurrentEntityCount = ManagedEntities.Num();
    }

    UE_LOG(LogTemp, Log, TEXT("Crowd_MassSimulationManager: Updated density to %d (target: %d entities)"), 
           (int32)NewDensity, TargetEntityCount);
}

void ACrowd_MassSimulationManager::SetCrowdTarget(const FVector& TargetLocation)
{
    // Update all entities to move toward the target
    for (auto& PathDataPair : EntityPathfindingData)
    {
        FCrowd_PathfindingData& PathData = PathDataPair.Value;
        PathData.CurrentTarget = TargetLocation + FVector(
            FMath::RandRange(-200.0f, 200.0f),
            FMath::RandRange(-200.0f, 200.0f),
            0.0f
        );
    }

    UE_LOG(LogTemp, Log, TEXT("Crowd_MassSimulationManager: Set crowd target to %s"), *TargetLocation.ToString());
}

void ACrowd_MassSimulationManager::UpdatePathfinding()
{
    PathfindingUpdateTimer += GetWorld()->GetDeltaSeconds();
    
    if (PathfindingUpdateTimer >= PathfindingConfig.PathUpdateInterval)
    {
        PathfindingUpdateTimer = 0.0f;
        
        // Update pathfinding for all entities
        for (auto& PathDataPair : EntityPathfindingData)
        {
            FCrowd_PathfindingData& PathData = PathDataPair.Value;
            
            // Simple pathfinding update - in a full implementation this would use proper navigation
            if (CurrentBehaviorType == ECrowd_CrowdBehaviorType::Wandering)
            {
                // Generate new random target
                PathData.CurrentTarget = GetActorLocation() + FVector(
                    FMath::RandRange(-1000.0f, 1000.0f),
                    FMath::RandRange(-1000.0f, 1000.0f),
                    0.0f
                );
            }
        }
    }
}

void ACrowd_MassSimulationManager::SetWaypointPath(const TArray<FVector>& Waypoints)
{
    PathfindingConfig.WaypointPath = Waypoints;
    
    // Update all entities to follow the waypoint path
    for (auto& PathDataPair : EntityPathfindingData)
    {
        FCrowd_PathfindingData& PathData = PathDataPair.Value;
        PathData.WaypointPath = Waypoints;
        if (Waypoints.Num() > 0)
        {
            PathData.CurrentTarget = Waypoints[0];
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Crowd_MassSimulationManager: Set waypoint path with %d waypoints"), Waypoints.Num());
}

void ACrowd_MassSimulationManager::EnableNavMeshPathfinding(bool bEnable)
{
    PathfindingConfig.bUseNavMesh = bEnable;
    
    for (auto& PathDataPair : EntityPathfindingData)
    {
        FCrowd_PathfindingData& PathData = PathDataPair.Value;
        PathData.bUseNavMesh = bEnable;
    }

    UE_LOG(LogTemp, Log, TEXT("Crowd_MassSimulationManager: NavMesh pathfinding %s"), 
           bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void ACrowd_MassSimulationManager::UpdateLODSystem()
{
    LODUpdateTimer += GetWorld()->GetDeltaSeconds();
    
    if (LODUpdateTimer >= 1.0f) // Update LOD every second
    {
        LODUpdateTimer = 0.0f;
        UpdateEntityLOD();
    }
}

void ACrowd_MassSimulationManager::SetLODDistances(float Distance1, float Distance2, float Distance3)
{
    DefaultSpawnParams.LODDistance1 = Distance1;
    DefaultSpawnParams.LODDistance2 = Distance2;
    DefaultSpawnParams.LODDistance3 = Distance3;

    UE_LOG(LogTemp, Log, TEXT("Crowd_MassSimulationManager: Set LOD distances: %.1f, %.1f, %.1f"), 
           Distance1, Distance2, Distance3);
}

int32 ACrowd_MassSimulationManager::GetActiveEntityCount() const
{
    return CurrentEntityCount;
}

float ACrowd_MassSimulationManager::GetCurrentPerformanceMetric() const
{
    return AverageFrameTime;
}

void ACrowd_MassSimulationManager::EnablePerformanceOptimization(bool bEnable)
{
    if (bEnable)
    {
        // Reduce tick frequency for better performance
        PrimaryActorTick.TickInterval = 0.2f;
    }
    else
    {
        // Restore normal tick frequency
        PrimaryActorTick.TickInterval = 0.1f;
    }

    UE_LOG(LogTemp, Log, TEXT("Crowd_MassSimulationManager: Performance optimization %s"), 
           bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void ACrowd_MassSimulationManager::TriggerCrowdFleeResponse(const FVector& ThreatLocation, float ThreatRadius)
{
    SetCrowdBehavior(ECrowd_CrowdBehaviorType::Fleeing);
    
    // Make entities flee away from the threat
    for (auto& PathDataPair : EntityPathfindingData)
    {
        FCrowd_PathfindingData& PathData = PathDataPair.Value;
        
        // Calculate flee direction
        FVector FleeDirection = PathData.CurrentTarget - ThreatLocation;
        FleeDirection.Normalize();
        
        // Set new target away from threat
        PathData.CurrentTarget = ThreatLocation + FleeDirection * (ThreatRadius + 1000.0f);
    }

    UE_LOG(LogTemp, Log, TEXT("Crowd_MassSimulationManager: Triggered flee response from threat at %s"), 
           *ThreatLocation.ToString());
}

void ACrowd_MassSimulationManager::SetCrowdCombatMode(bool bCombatMode)
{
    bIsCombatMode = bCombatMode;
    
    if (bCombatMode)
    {
        // Switch to more aggressive behavior
        SetCrowdBehavior(ECrowd_CrowdBehaviorType::Hunting);
    }
    else
    {
        // Return to peaceful behavior
        SetCrowdBehavior(ECrowd_CrowdBehaviorType::Wandering);
    }

    UE_LOG(LogTemp, Log, TEXT("Crowd_MassSimulationManager: Combat mode %s"), 
           bCombatMode ? TEXT("enabled") : TEXT("disabled"));
}

void ACrowd_MassSimulationManager::DebugDrawCrowdInfo()
{
    if (!GetWorld())
    {
        return;
    }

    const FVector ActorLocation = GetActorLocation();
    
    // Draw manager location
    DrawDebugSphere(GetWorld(), ActorLocation, 100.0f, 12, FColor::Blue, false, -1.0f, 0, 5.0f);
    
    // Draw entity count info
    const FString InfoText = FString::Printf(TEXT("Entities: %d/%d\nBehavior: %d\nFPS: %.1f"), 
                                           CurrentEntityCount, MaxEntityCount, 
                                           (int32)CurrentBehaviorType, 1.0f / AverageFrameTime);
    
    DrawDebugString(GetWorld(), ActorLocation + FVector(0, 0, 200), InfoText, nullptr, FColor::White, 0.0f);
    
    // Draw pathfinding targets
    int32 DebugCount = 0;
    for (const auto& PathDataPair : EntityPathfindingData)
    {
        if (DebugCount >= 20) break; // Limit debug draws for performance
        
        const FCrowd_PathfindingData& PathData = PathDataPair.Value;
        DrawDebugSphere(GetWorld(), PathData.CurrentTarget, 50.0f, 8, FColor::Green, false, -1.0f, 0, 2.0f);
        DebugCount++;
    }
}

void ACrowd_MassSimulationManager::ToggleDebugVisualization()
{
    bEnableDebugVisualization = !bEnableDebugVisualization;
    VisualizationMesh->SetVisibility(bEnableDebugVisualization);

    UE_LOG(LogTemp, Log, TEXT("Crowd_MassSimulationManager: Debug visualization %s"), 
           bEnableDebugVisualization ? TEXT("enabled") : TEXT("disabled"));
}

void ACrowd_MassSimulationManager::UpdatePerformanceMetrics(float DeltaTime)
{
    LastFrameTime = DeltaTime;
    
    // Calculate rolling average
    PerformanceSampleCount++;
    const float Alpha = 1.0f / FMath::Min(PerformanceSampleCount, 60); // 60-frame average
    AverageFrameTime = AverageFrameTime * (1.0f - Alpha) + DeltaTime * Alpha;
    
    // Auto-optimize if performance is poor
    if (AverageFrameTime > 0.033f && CurrentEntityCount > 100) // Worse than 30 FPS
    {
        EnablePerformanceOptimization(true);
    }
}

void ACrowd_MassSimulationManager::ProcessEntityBehaviors()
{
    BehaviorUpdateTimer += GetWorld()->GetDeltaSeconds();
    
    if (BehaviorUpdateTimer >= 0.5f) // Update behaviors twice per second
    {
        BehaviorUpdateTimer = 0.0f;
        
        // Process behavior-specific logic
        switch (CurrentBehaviorType)
        {
        case ECrowd_CrowdBehaviorType::Gathering:
            // Entities move toward a central gathering point
            SetCrowdTarget(GetActorLocation());
            break;
            
        case ECrowd_CrowdBehaviorType::Fleeing:
            // Continue fleeing behavior - targets are set by TriggerCrowdFleeResponse
            break;
            
        case ECrowd_CrowdBehaviorType::Following:
            // Follow the manager's position
            SetCrowdTarget(GetActorLocation() + FVector(200, 0, 0));
            break;
            
        case ECrowd_CrowdBehaviorType::Hunting:
            // Aggressive movement patterns
            for (auto& PathDataPair : EntityPathfindingData)
            {
                FCrowd_PathfindingData& PathData = PathDataPair.Value;
                PathData.PathUpdateInterval = 1.0f; // More frequent updates
            }
            break;
            
        case ECrowd_CrowdBehaviorType::Resting:
            // Minimal movement
            for (auto& PathDataPair : EntityPathfindingData)
            {
                FCrowd_PathfindingData& PathData = PathDataPair.Value;
                PathData.PathUpdateInterval = 10.0f; // Very slow updates
            }
            break;
            
        case ECrowd_CrowdBehaviorType::Wandering:
        default:
            // Random wandering - handled in UpdatePathfinding
            break;
        }
    }
}

void ACrowd_MassSimulationManager::UpdateEntityLOD()
{
    // In a full implementation, this would update entity LOD based on distance to camera
    // For now, just track the concept
    
    APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
    if (!PlayerPawn)
    {
        return;
    }
    
    const FVector PlayerLocation = PlayerPawn->GetActorLocation();
    const FVector ManagerLocation = GetActorLocation();
    const float DistanceToPlayer = FVector::Dist(PlayerLocation, ManagerLocation);
    
    // Adjust entity detail based on distance
    if (DistanceToPlayer > DefaultSpawnParams.LODDistance3)
    {
        // Very far - minimal detail
        EnablePerformanceOptimization(true);
    }
    else if (DistanceToPlayer > DefaultSpawnParams.LODDistance2)
    {
        // Far - reduced detail
        PrimaryActorTick.TickInterval = 0.15f;
    }
    else if (DistanceToPlayer > DefaultSpawnParams.LODDistance1)
    {
        // Medium distance - normal detail
        PrimaryActorTick.TickInterval = 0.1f;
    }
    else
    {
        // Close - full detail
        PrimaryActorTick.TickInterval = 0.05f;
        EnablePerformanceOptimization(false);
    }
}

void ACrowd_MassSimulationManager::HandleCombatEvents()
{
    if (!bIsCombatMode)
    {
        return;
    }
    
    // In a full implementation, this would integrate with the combat system
    // For now, just maintain combat behavior state
    
    static float CombatTimer = 0.0f;
    CombatTimer += GetWorld()->GetDeltaSeconds();
    
    if (CombatTimer >= 5.0f) // Combat mode lasts 5 seconds
    {
        CombatTimer = 0.0f;
        SetCrowdCombatMode(false); // Return to normal behavior
    }
}