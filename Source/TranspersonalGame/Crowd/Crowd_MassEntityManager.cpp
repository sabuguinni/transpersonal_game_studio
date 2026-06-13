#include "Crowd_MassEntityManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

UCrowd_MassEntityManager::UCrowd_MassEntityManager()
{
    // Set default spawn configuration
    SpawnConfig.MaxEntities = 1000;
    SpawnConfig.SpawnRadius = 2000.0f;
    SpawnConfig.LODDistance_High = 500.0f;
    SpawnConfig.LODDistance_Medium = 1500.0f;
    SpawnConfig.LODDistance_Low = 3000.0f;
    SpawnConfig.MovementSpeed = 150.0f;
    SpawnConfig.bEnablePathfinding = true;
    SpawnConfig.bEnableLODSystem = true;
}

void UCrowd_MassEntityManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Crowd_MassEntityManager: Initializing crowd simulation system"));
    
    // Initialize crowd system
    InitializeCrowdSystem();
}

void UCrowd_MassEntityManager::Deinitialize()
{
    ClearAllCrowdEntities();
    bIsInitialized = false;
    
    Super::Deinitialize();
}

bool UCrowd_MassEntityManager::ShouldCreateSubsystem(UObject* Outer) const
{
    return true;
}

void UCrowd_MassEntityManager::InitializeCrowdSystem()
{
    if (bIsInitialized)
    {
        return;
    }

    // Clear existing data
    CrowdEntities.Empty();
    PathfindingWaypoints.Empty();
    
    // Generate default waypoints for pathfinding
    const int32 WaypointCount = 20;
    for (int32 i = 0; i < WaypointCount; i++)
    {
        FVector Waypoint;
        Waypoint.X = FMath::RandRange(-SpawnConfig.SpawnRadius, SpawnConfig.SpawnRadius);
        Waypoint.Y = FMath::RandRange(-SpawnConfig.SpawnRadius, SpawnConfig.SpawnRadius);
        Waypoint.Z = FMath::RandRange(50.0f, 300.0f);
        
        PathfindingWaypoints.Add(Waypoint);
    }
    
    bIsInitialized = true;
    CurrentBehaviorMode = ECrowd_BehaviorMode::Wandering;
    
    UE_LOG(LogTemp, Log, TEXT("Crowd_MassEntityManager: System initialized with %d waypoints"), PathfindingWaypoints.Num());
}

void UCrowd_MassEntityManager::SpawnCrowdEntities(int32 Count, FVector CenterLocation, float Radius)
{
    if (!bIsInitialized)
    {
        InitializeCrowdSystem();
    }

    // Limit spawn count to prevent performance issues
    const int32 ActualCount = FMath::Min(Count, SpawnConfig.MaxEntities - CrowdEntities.Num());
    
    for (int32 i = 0; i < ActualCount; i++)
    {
        FCrowd_EntityData NewEntity;
        NewEntity.EntityID = CrowdEntities.Num() + i;
        
        // Random spawn location within radius
        const float Angle = FMath::RandRange(0.0f, 2.0f * PI);
        const float Distance = FMath::RandRange(0.0f, Radius);
        
        NewEntity.CurrentLocation.X = CenterLocation.X + Distance * FMath::Cos(Angle);
        NewEntity.CurrentLocation.Y = CenterLocation.Y + Distance * FMath::Sin(Angle);
        NewEntity.CurrentLocation.Z = CenterLocation.Z + FMath::RandRange(0.0f, 100.0f);
        
        // Set initial target
        NewEntity.TargetLocation = GetRandomWaypoint();
        NewEntity.bIsActive = true;
        NewEntity.LODLevel = ECrowd_LODLevel::High;
        NewEntity.LastUpdateTime = GetWorld()->GetTimeSeconds();
        
        CrowdEntities.Add(NewEntity);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Crowd_MassEntityManager: Spawned %d entities (Total: %d)"), ActualCount, CrowdEntities.Num());
}

void UCrowd_MassEntityManager::UpdateCrowdLOD(FVector PlayerLocation)
{
    if (!SpawnConfig.bEnableLODSystem)
    {
        return;
    }

    const float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Throttle LOD updates for performance
    if (CurrentTime - LastLODUpdateTime < 0.1f) // Update every 100ms
    {
        return;
    }
    
    LastLODUpdateTime = CurrentTime;
    
    for (FCrowd_EntityData& Entity : CrowdEntities)
    {
        if (!Entity.bIsActive)
        {
            continue;
        }
        
        Entity.DistanceToPlayer = FVector::Dist(Entity.CurrentLocation, PlayerLocation);
        UpdateEntityLOD(Entity, Entity.DistanceToPlayer);
    }
}

void UCrowd_MassEntityManager::UpdateEntityLOD(FCrowd_EntityData& Entity, float DistanceToPlayer)
{
    ECrowd_LODLevel NewLODLevel;
    
    if (DistanceToPlayer <= SpawnConfig.LODDistance_High)
    {
        NewLODLevel = ECrowd_LODLevel::High;
    }
    else if (DistanceToPlayer <= SpawnConfig.LODDistance_Medium)
    {
        NewLODLevel = ECrowd_LODLevel::Medium;
    }
    else if (DistanceToPlayer <= SpawnConfig.LODDistance_Low)
    {
        NewLODLevel = ECrowd_LODLevel::Low;
    }
    else
    {
        NewLODLevel = ECrowd_LODLevel::Culled;
    }
    
    Entity.LODLevel = NewLODLevel;
}

void UCrowd_MassEntityManager::SetCrowdBehaviorMode(ECrowd_BehaviorMode NewMode)
{
    CurrentBehaviorMode = NewMode;
    UE_LOG(LogTemp, Log, TEXT("Crowd_MassEntityManager: Behavior mode changed to %d"), (int32)NewMode);
}

void UCrowd_MassEntityManager::UpdateCrowdMovement(float DeltaTime)
{
    if (!bIsInitialized || !SpawnConfig.bEnablePathfinding)
    {
        return;
    }

    const float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (FCrowd_EntityData& Entity : CrowdEntities)
    {
        if (!Entity.bIsActive || Entity.LODLevel == ECrowd_LODLevel::Culled)
        {
            continue;
        }
        
        // Update frequency based on LOD level
        float UpdateInterval = 0.1f; // High LOD
        if (Entity.LODLevel == ECrowd_LODLevel::Medium)
        {
            UpdateInterval = 0.2f;
        }
        else if (Entity.LODLevel == ECrowd_LODLevel::Low)
        {
            UpdateInterval = 0.5f;
        }
        
        if (CurrentTime - Entity.LastUpdateTime >= UpdateInterval)
        {
            ApplyMovementBehavior(Entity, DeltaTime);
            Entity.LastUpdateTime = CurrentTime;
        }
    }
}

void UCrowd_MassEntityManager::ApplyMovementBehavior(FCrowd_EntityData& Entity, float DeltaTime)
{
    const float DistanceToTarget = FVector::Dist(Entity.CurrentLocation, Entity.TargetLocation);
    
    // If close to target, pick new waypoint
    if (DistanceToTarget < 100.0f)
    {
        Entity.TargetLocation = GetRandomWaypoint();
    }
    
    // Move towards target
    const FVector Direction = (Entity.TargetLocation - Entity.CurrentLocation).GetSafeNormal();
    float MovementSpeed = SpawnConfig.MovementSpeed;
    
    // Adjust speed based on LOD level
    switch (Entity.LODLevel)
    {
        case ECrowd_LODLevel::High:
            MovementSpeed *= 1.0f;
            break;
        case ECrowd_LODLevel::Medium:
            MovementSpeed *= 0.7f;
            break;
        case ECrowd_LODLevel::Low:
            MovementSpeed *= 0.3f;
            break;
        default:
            return;
    }
    
    // Apply behavior-specific modifiers
    switch (CurrentBehaviorMode)
    {
        case ECrowd_BehaviorMode::Wandering:
            // Add some randomness to movement
            MovementSpeed *= FMath::RandRange(0.8f, 1.2f);
            break;
        case ECrowd_BehaviorMode::Fleeing:
            MovementSpeed *= 2.0f;
            break;
        case ECrowd_BehaviorMode::Gathering:
            MovementSpeed *= 0.5f;
            break;
    }
    
    Entity.CurrentLocation += Direction * MovementSpeed * DeltaTime;
}

FVector UCrowd_MassEntityManager::GetRandomWaypoint() const
{
    if (PathfindingWaypoints.Num() == 0)
    {
        return FVector::ZeroVector;
    }
    
    const int32 RandomIndex = FMath::RandRange(0, PathfindingWaypoints.Num() - 1);
    return PathfindingWaypoints[RandomIndex];
}

void UCrowd_MassEntityManager::CreatePathfindingZone(FVector Center, float Radius, FString ZoneName)
{
    // Generate waypoints in circular pattern around center
    const int32 WaypointsPerZone = 8;
    
    for (int32 i = 0; i < WaypointsPerZone; i++)
    {
        const float Angle = (2.0f * PI * i) / WaypointsPerZone;
        const float Distance = FMath::RandRange(Radius * 0.3f, Radius * 0.8f);
        
        FVector Waypoint;
        Waypoint.X = Center.X + Distance * FMath::Cos(Angle);
        Waypoint.Y = Center.Y + Distance * FMath::Sin(Angle);
        Waypoint.Z = Center.Z + FMath::RandRange(-50.0f, 50.0f);
        
        PathfindingWaypoints.Add(Waypoint);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Crowd_MassEntityManager: Created pathfinding zone '%s' with %d waypoints"), *ZoneName, WaypointsPerZone);
}

void UCrowd_MassEntityManager::ClearAllCrowdEntities()
{
    CrowdEntities.Empty();
    UE_LOG(LogTemp, Log, TEXT("Crowd_MassEntityManager: Cleared all crowd entities"));
}

void UCrowd_MassEntityManager::SetSpawnConfiguration(const FCrowd_EntitySpawnConfig& NewConfig)
{
    SpawnConfig = NewConfig;
    UE_LOG(LogTemp, Log, TEXT("Crowd_MassEntityManager: Updated spawn configuration"));
}

int32 UCrowd_MassEntityManager::GetActiveCrowdCount() const
{
    int32 ActiveCount = 0;
    for (const FCrowd_EntityData& Entity : CrowdEntities)
    {
        if (Entity.bIsActive)
        {
            ActiveCount++;
        }
    }
    return ActiveCount;
}

int32 UCrowd_MassEntityManager::GetHighLODCount() const
{
    int32 Count = 0;
    for (const FCrowd_EntityData& Entity : CrowdEntities)
    {
        if (Entity.bIsActive && Entity.LODLevel == ECrowd_LODLevel::High)
        {
            Count++;
        }
    }
    return Count;
}

int32 UCrowd_MassEntityManager::GetMediumLODCount() const
{
    int32 Count = 0;
    for (const FCrowd_EntityData& Entity : CrowdEntities)
    {
        if (Entity.bIsActive && Entity.LODLevel == ECrowd_LODLevel::Medium)
        {
            Count++;
        }
    }
    return Count;
}

int32 UCrowd_MassEntityManager::GetLowLODCount() const
{
    int32 Count = 0;
    for (const FCrowd_EntityData& Entity : CrowdEntities)
    {
        if (Entity.bIsActive && Entity.LODLevel == ECrowd_LODLevel::Low)
        {
            Count++;
        }
    }
    return Count;
}

void UCrowd_MassEntityManager::OptimizePerformance()
{
    // Remove inactive entities periodically
    CrowdEntities.RemoveAll([](const FCrowd_EntityData& Entity)
    {
        return !Entity.bIsActive;
    });
}