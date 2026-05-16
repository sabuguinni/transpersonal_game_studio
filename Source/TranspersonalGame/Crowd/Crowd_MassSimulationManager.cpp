#include "Crowd_MassSimulationManager.h"
#include "Crowd_EntitySpawner.h"
#include "Crowd_PathfindingComponent.h"
#include "Crowd_LODManager.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "AI/NavigationSystemBase.h"

UCrowd_MassSimulationManager::UCrowd_MassSimulationManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz default tick rate
    
    // Initialize default values
    MaxSimulationEntities = 50000;
    TickRate = 10.0f;
    CurrentPerformanceMode = ECrowd_PerformanceMode::Balanced;
    MaxRenderDistance = 10000.0f;
    bIsSimulationActive = false;
    LastUpdateTime = 0.0f;
    NextEntityID = 1;
    LastPlayerLocation = FVector::ZeroVector;
    AverageFrameTime = 0.0f;
    FrameCounter = 0;

    // Initialize LOD distances
    LODDistances.SetNum(4);
    LODDistances[0] = 1000.0f;  // High detail
    LODDistances[1] = 3000.0f;  // Medium detail
    LODDistances[2] = 7000.0f;  // Low detail
    LODDistances[3] = 10000.0f; // Minimal detail

    // Initialize default spawn parameters
    DefaultSpawnParams.MaxEntities = 1000;
    DefaultSpawnParams.SpawnRadius = 5000.0f;
    DefaultSpawnParams.SpawnCenter = FVector::ZeroVector;
    DefaultSpawnParams.MinDistanceBetweenEntities = 200.0f;
    DefaultSpawnParams.EntityType = ECrowd_EntityType::Tribal;
}

void UCrowd_MassSimulationManager::BeginPlay()
{
    Super::BeginPlay();

    // Create sub-components
    PathfindingComponent = CreateDefaultSubobject<UCrowd_PathfindingComponent>(TEXT("PathfindingComponent"));
    EntitySpawner = CreateDefaultSubobject<UCrowd_EntitySpawner>(TEXT("EntitySpawner"));
    LODManager = CreateDefaultSubobject<UCrowd_LODManager>(TEXT("LODManager"));

    // Initialize the mass simulation system
    InitializeMassSimulation();

    UE_LOG(LogTemp, Log, TEXT("Crowd_MassSimulationManager: BeginPlay completed"));
}

void UCrowd_MassSimulationManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bIsSimulationActive)
    {
        return;
    }

    // Performance tracking
    FrameCounter++;
    AverageFrameTime = (AverageFrameTime * (FrameCounter - 1) + DeltaTime) / FrameCounter;

    // Update player location for LOD calculations
    if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        if (APawn* PlayerPawn = PlayerController->GetPawn())
        {
            LastPlayerLocation = PlayerPawn->GetActorLocation();
        }
    }

    // Update simulation systems
    UpdateEntityMovement(DeltaTime);
    UpdateEntityBehaviors(DeltaTime);
    UpdateLODLevels(LastPlayerLocation);
    ProcessEntityCollisions();

    // Optimize entity distribution every 5 seconds
    if (GetWorld()->GetTimeSeconds() - LastUpdateTime > 5.0f)
    {
        OptimizeEntityDistribution();
        LastUpdateTime = GetWorld()->GetTimeSeconds();
    }
}

void UCrowd_MassSimulationManager::InitializeMassSimulation()
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("Crowd_MassSimulationManager: No valid world found"));
        return;
    }

    // Clear existing entities
    EntityDatabase.Empty();
    NextEntityID = 1;

    // Initialize navigation system integration
    if (UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld()))
    {
        UE_LOG(LogTemp, Log, TEXT("Crowd_MassSimulationManager: Navigation system found"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Crowd_MassSimulationManager: No navigation system found"));
    }

    bIsSimulationActive = true;
    UE_LOG(LogTemp, Log, TEXT("Crowd_MassSimulationManager: Mass simulation initialized"));
}

void UCrowd_MassSimulationManager::SpawnEntityGroup(const FCrowd_SpawnParameters& SpawnParams)
{
    if (!bIsSimulationActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Crowd_MassSimulationManager: Cannot spawn entities - simulation not active"));
        return;
    }

    if (EntityDatabase.Num() + SpawnParams.MaxEntities > MaxSimulationEntities)
    {
        UE_LOG(LogTemp, Warning, TEXT("Crowd_MassSimulationManager: Cannot spawn %d entities - would exceed limit of %d"), 
               SpawnParams.MaxEntities, MaxSimulationEntities);
        return;
    }

    int32 SpawnedCount = 0;
    for (int32 i = 0; i < SpawnParams.MaxEntities; ++i)
    {
        FCrowd_EntityData NewEntity;
        
        // Generate random position within spawn radius
        FVector RandomOffset = FVector(
            FMath::RandRange(-SpawnParams.SpawnRadius, SpawnParams.SpawnRadius),
            FMath::RandRange(-SpawnParams.SpawnRadius, SpawnParams.SpawnRadius),
            0.0f
        );
        
        NewEntity.Position = SpawnParams.SpawnCenter + RandomOffset;
        NewEntity.Rotation = FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f);
        NewEntity.MovementSpeed = FMath::RandRange(200.0f, 400.0f);
        NewEntity.TargetLocation = NewEntity.Position;
        NewEntity.EntityID = NextEntityID++;
        NewEntity.BehaviorState = ECrowd_EntityBehavior::Wandering;

        // Check minimum distance to existing entities
        bool bValidPosition = true;
        for (const FCrowd_EntityData& ExistingEntity : EntityDatabase)
        {
            if (FVector::Dist(NewEntity.Position, ExistingEntity.Position) < SpawnParams.MinDistanceBetweenEntities)
            {
                bValidPosition = false;
                break;
            }
        }

        if (bValidPosition)
        {
            EntityDatabase.Add(NewEntity);
            SpawnedCount++;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Crowd_MassSimulationManager: Spawned %d entities at %s"), 
           SpawnedCount, *SpawnParams.SpawnCenter.ToString());
}

void UCrowd_MassSimulationManager::DespawnAllEntities()
{
    EntityDatabase.Empty();
    NextEntityID = 1;
    UE_LOG(LogTemp, Log, TEXT("Crowd_MassSimulationManager: All entities despawned"));
}

int32 UCrowd_MassSimulationManager::GetActiveEntityCount() const
{
    return EntityDatabase.Num();
}

void UCrowd_MassSimulationManager::SetGroupDestination(const FVector& Destination, float Radius)
{
    for (FCrowd_EntityData& Entity : EntityDatabase)
    {
        // Add some randomness to prevent all entities going to exact same spot
        FVector RandomOffset = FVector(
            FMath::RandRange(-Radius, Radius),
            FMath::RandRange(-Radius, Radius),
            0.0f
        );
        
        Entity.TargetLocation = Destination + RandomOffset;
        Entity.BehaviorState = ECrowd_EntityBehavior::MovingToTarget;
    }

    UE_LOG(LogTemp, Log, TEXT("Crowd_MassSimulationManager: Set group destination to %s"), *Destination.ToString());
}

void UCrowd_MassSimulationManager::UpdateEntityMovement(float DeltaTime)
{
    for (FCrowd_EntityData& Entity : EntityDatabase)
    {
        if (Entity.BehaviorState == ECrowd_EntityBehavior::Idle)
        {
            continue;
        }

        FVector DirectionToTarget = (Entity.TargetLocation - Entity.Position).GetSafeNormal();
        float DistanceToTarget = FVector::Dist(Entity.Position, Entity.TargetLocation);

        if (DistanceToTarget > 50.0f) // Close enough threshold
        {
            // Apply flocking and avoidance forces
            FVector FlockingForce = CalculateFlockingForce(Entity);
            FVector AvoidanceForce = CalculateAvoidanceForce(Entity);
            
            FVector FinalDirection = (DirectionToTarget + FlockingForce * 0.3f + AvoidanceForce * 0.5f).GetSafeNormal();
            
            Entity.Position += FinalDirection * Entity.MovementSpeed * DeltaTime;
            Entity.Rotation = FinalDirection.Rotation();
        }
        else
        {
            // Reached target, switch to wandering or idle
            if (Entity.BehaviorState == ECrowd_EntityBehavior::MovingToTarget)
            {
                Entity.BehaviorState = ECrowd_EntityBehavior::Wandering;
                
                // Set new random target for wandering
                FVector RandomDirection = FVector(
                    FMath::RandRange(-1.0f, 1.0f),
                    FMath::RandRange(-1.0f, 1.0f),
                    0.0f
                ).GetSafeNormal();
                
                Entity.TargetLocation = Entity.Position + RandomDirection * FMath::RandRange(500.0f, 1500.0f);
            }
        }
    }
}

bool UCrowd_MassSimulationManager::FindPathToLocation(const FVector& StartLocation, const FVector& EndLocation, TArray<FVector>& OutPath)
{
    // Simple pathfinding implementation - in a full system this would use NavMesh
    OutPath.Empty();
    OutPath.Add(StartLocation);
    OutPath.Add(EndLocation);
    return true;
}

void UCrowd_MassSimulationManager::UpdateLODLevels(const FVector& ViewerLocation)
{
    if (!LODManager)
    {
        return;
    }

    for (FCrowd_EntityData& Entity : EntityDatabase)
    {
        float DistanceToViewer = FVector::Dist(Entity.Position, ViewerLocation);
        
        // Determine LOD level based on distance
        int32 LODLevel = 3; // Default to lowest LOD
        for (int32 i = 0; i < LODDistances.Num(); ++i)
        {
            if (DistanceToViewer <= LODDistances[i])
            {
                LODLevel = i;
                break;
            }
        }

        // Apply LOD-specific optimizations
        if (LODLevel >= 2)
        {
            // Reduce update frequency for distant entities
            if (FrameCounter % (LODLevel * 2) != 0)
            {
                continue;
            }
        }
    }
}

void UCrowd_MassSimulationManager::SetLODDistance(int32 LODLevel, float Distance)
{
    if (LODLevel >= 0 && LODLevel < LODDistances.Num())
    {
        LODDistances[LODLevel] = Distance;
    }
}

void UCrowd_MassSimulationManager::SetEntityBehavior(int32 EntityID, ECrowd_EntityBehavior NewBehavior)
{
    for (FCrowd_EntityData& Entity : EntityDatabase)
    {
        if (Entity.EntityID == EntityID)
        {
            Entity.BehaviorState = NewBehavior;
            break;
        }
    }
}

void UCrowd_MassSimulationManager::TriggerFleeResponse(const FVector& ThreatLocation, float ThreatRadius)
{
    for (FCrowd_EntityData& Entity : EntityDatabase)
    {
        float DistanceToThreat = FVector::Dist(Entity.Position, ThreatLocation);
        
        if (DistanceToThreat <= ThreatRadius)
        {
            Entity.BehaviorState = ECrowd_EntityBehavior::Fleeing;
            
            // Set flee target away from threat
            FVector FleeDirection = (Entity.Position - ThreatLocation).GetSafeNormal();
            Entity.TargetLocation = Entity.Position + FleeDirection * 2000.0f;
            Entity.MovementSpeed *= 1.5f; // Increase speed when fleeing
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Crowd_MassSimulationManager: Triggered flee response from %s"), *ThreatLocation.ToString());
}

void UCrowd_MassSimulationManager::SetGatheringMode(bool bEnabled, const FVector& GatherLocation)
{
    if (bEnabled)
    {
        SetGroupDestination(GatherLocation, 500.0f);
        
        for (FCrowd_EntityData& Entity : EntityDatabase)
        {
            Entity.BehaviorState = ECrowd_EntityBehavior::Gathering;
        }
    }
    else
    {
        for (FCrowd_EntityData& Entity : EntityDatabase)
        {
            Entity.BehaviorState = ECrowd_EntityBehavior::Wandering;
        }
    }
}

float UCrowd_MassSimulationManager::GetSimulationPerformance() const
{
    // Return performance metric (lower is better)
    return AverageFrameTime * EntityDatabase.Num();
}

void UCrowd_MassSimulationManager::SetPerformanceMode(ECrowd_PerformanceMode Mode)
{
    CurrentPerformanceMode = Mode;
    
    switch (Mode)
    {
        case ECrowd_PerformanceMode::HighQuality:
            TickRate = 30.0f;
            MaxSimulationEntities = 20000;
            break;
            
        case ECrowd_PerformanceMode::Balanced:
            TickRate = 10.0f;
            MaxSimulationEntities = 50000;
            break;
            
        case ECrowd_PerformanceMode::Performance:
            TickRate = 5.0f;
            MaxSimulationEntities = 100000;
            break;
    }
    
    PrimaryComponentTick.TickInterval = 1.0f / TickRate;
}

void UCrowd_MassSimulationManager::UpdateEntityBehaviors(float DeltaTime)
{
    // Update behavior-specific logic for each entity
    for (FCrowd_EntityData& Entity : EntityDatabase)
    {
        switch (Entity.BehaviorState)
        {
            case ECrowd_EntityBehavior::Idle:
                // Occasionally switch to wandering
                if (FMath::RandRange(0.0f, 1.0f) < 0.01f)
                {
                    Entity.BehaviorState = ECrowd_EntityBehavior::Wandering;
                }
                break;
                
            case ECrowd_EntityBehavior::Wandering:
                // Continue wandering behavior
                break;
                
            case ECrowd_EntityBehavior::Fleeing:
                // Gradually return to normal behavior after fleeing
                if (FMath::RandRange(0.0f, 1.0f) < 0.005f)
                {
                    Entity.BehaviorState = ECrowd_EntityBehavior::Wandering;
                    Entity.MovementSpeed /= 1.5f; // Restore normal speed
                }
                break;
                
            case ECrowd_EntityBehavior::Gathering:
                // Gathering behavior handled in movement update
                break;
                
            default:
                break;
        }
    }
}

void UCrowd_MassSimulationManager::ProcessEntityCollisions()
{
    // Simple collision avoidance between entities
    for (int32 i = 0; i < EntityDatabase.Num(); ++i)
    {
        for (int32 j = i + 1; j < EntityDatabase.Num(); ++j)
        {
            FCrowd_EntityData& EntityA = EntityDatabase[i];
            FCrowd_EntityData& EntityB = EntityDatabase[j];
            
            float Distance = FVector::Dist(EntityA.Position, EntityB.Position);
            const float MinDistance = 100.0f; // Minimum distance between entities
            
            if (Distance < MinDistance && Distance > 0.0f)
            {
                FVector SeparationVector = (EntityA.Position - EntityB.Position).GetSafeNormal();
                float SeparationForce = (MinDistance - Distance) * 0.5f;
                
                EntityA.Position += SeparationVector * SeparationForce;
                EntityB.Position -= SeparationVector * SeparationForce;
            }
        }
    }
}

void UCrowd_MassSimulationManager::OptimizeEntityDistribution()
{
    // Remove entities that are too far from player to optimize performance
    if (LastPlayerLocation != FVector::ZeroVector)
    {
        EntityDatabase.RemoveAll([this](const FCrowd_EntityData& Entity)
        {
            return FVector::Dist(Entity.Position, LastPlayerLocation) > MaxRenderDistance * 1.5f;
        });
    }
}

FVector UCrowd_MassSimulationManager::CalculateFlockingForce(const FCrowd_EntityData& Entity) const
{
    FVector FlockingForce = FVector::ZeroVector;
    int32 NeighborCount = 0;
    const float FlockingRadius = 500.0f;
    
    for (const FCrowd_EntityData& OtherEntity : EntityDatabase)
    {
        if (OtherEntity.EntityID == Entity.EntityID)
        {
            continue;
        }
        
        float Distance = FVector::Dist(Entity.Position, OtherEntity.Position);
        if (Distance <= FlockingRadius)
        {
            FlockingForce += (OtherEntity.Position - Entity.Position);
            NeighborCount++;
        }
    }
    
    if (NeighborCount > 0)
    {
        FlockingForce /= NeighborCount;
        FlockingForce = FlockingForce.GetSafeNormal();
    }
    
    return FlockingForce;
}

FVector UCrowd_MassSimulationManager::CalculateAvoidanceForce(const FCrowd_EntityData& Entity) const
{
    FVector AvoidanceForce = FVector::ZeroVector;
    const float AvoidanceRadius = 200.0f;
    
    for (const FCrowd_EntityData& OtherEntity : EntityDatabase)
    {
        if (OtherEntity.EntityID == Entity.EntityID)
        {
            continue;
        }
        
        float Distance = FVector::Dist(Entity.Position, OtherEntity.Position);
        if (Distance <= AvoidanceRadius && Distance > 0.0f)
        {
            FVector AvoidanceVector = (Entity.Position - OtherEntity.Position).GetSafeNormal();
            float AvoidanceStrength = (AvoidanceRadius - Distance) / AvoidanceRadius;
            AvoidanceForce += AvoidanceVector * AvoidanceStrength;
        }
    }
    
    return AvoidanceForce.GetSafeNormal();
}

bool UCrowd_MassSimulationManager::IsEntityInLODRange(const FCrowd_EntityData& Entity, const FVector& ViewerLocation) const
{
    float Distance = FVector::Dist(Entity.Position, ViewerLocation);
    return Distance <= MaxRenderDistance;
}