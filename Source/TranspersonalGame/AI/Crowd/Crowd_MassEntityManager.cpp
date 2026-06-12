#include "Crowd_MassEntityManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

ACrowd_MassEntityManager::ACrowd_MassEntityManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize configuration
    MaxCrowdEntities = 1000;
    EntityUpdateDistance = 2000.0f;
    LODDistance1 = 500.0f;
    LODDistance2 = 1000.0f;
    CollisionRadius = 50.0f;
    
    // Initialize state
    bMassSystemInitialized = false;
    ActiveEntityCount = 0;
    GatheringPoint = FVector::ZeroVector;
}

void ACrowd_MassEntityManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeMassSystem();
}

void ACrowd_MassEntityManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bMassSystemInitialized && ActiveEntityCount > 0)
    {
        UpdateEntityBehaviors(DeltaTime);
        ProcessCrowdMovement(DeltaTime);
        HandleCrowdCollisions();
        UpdateCrowdLOD();
    }
}

void ACrowd_MassEntityManager::InitializeMassSystem()
{
    if (bMassSystemInitialized)
    {
        return;
    }
    
    // Clear existing data
    CrowdEntities.Empty();
    SpawnZones.Empty();
    Waypoints.Empty();
    
    // Create default spawn zones
    FCrowd_SpawnZone DefaultZone;
    DefaultZone.Center = FVector(0, 0, 0);
    DefaultZone.Radius = 300.0f;
    DefaultZone.MaxEntities = 50;
    DefaultZone.DefaultBehavior = ECrowd_BehaviorState::Wandering;
    SpawnZones.Add(DefaultZone);
    
    // Create default waypoints in circular pattern
    float WaypointRadius = 800.0f;
    int32 WaypointCount = 8;
    for (int32 i = 0; i < WaypointCount; i++)
    {
        float Angle = (i * 2.0f * PI) / WaypointCount;
        FVector WaypointPos = FVector(
            FMath::Cos(Angle) * WaypointRadius,
            FMath::Sin(Angle) * WaypointRadius,
            100.0f
        );
        Waypoints.Add(WaypointPos);
    }
    
    bMassSystemInitialized = true;
    
    UE_LOG(LogTemp, Warning, TEXT("Mass Entity System Initialized with %d waypoints"), Waypoints.Num());
}

void ACrowd_MassEntityManager::SpawnCrowdEntities(int32 EntityCount, const FCrowd_SpawnZone& SpawnZone)
{
    if (!bMassSystemInitialized)
    {
        InitializeMassSystem();
    }
    
    int32 EntitiesToSpawn = FMath::Min(EntityCount, MaxCrowdEntities - ActiveEntityCount);
    
    for (int32 i = 0; i < EntitiesToSpawn; i++)
    {
        FCrowd_EntityData NewEntity;
        
        // Random position within spawn zone
        float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
        float RandomRadius = FMath::RandRange(0.0f, SpawnZone.Radius);
        
        NewEntity.Position = SpawnZone.Center + FVector(
            FMath::Cos(RandomAngle) * RandomRadius,
            FMath::Sin(RandomAngle) * RandomRadius,
            0.0f
        );
        
        NewEntity.Velocity = FVector::ZeroVector;
        NewEntity.Speed = FMath::RandRange(100.0f, 200.0f);
        NewEntity.BehaviorState = SpawnZone.DefaultBehavior;
        NewEntity.EntityID = CrowdEntities.Num();
        NewEntity.DistanceToTarget = 0.0f;
        
        CrowdEntities.Add(NewEntity);
        ActiveEntityCount++;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Spawned %d crowd entities. Total active: %d"), EntitiesToSpawn, ActiveEntityCount);
}

void ACrowd_MassEntityManager::UpdateEntityBehaviors(float DeltaTime)
{
    for (FCrowd_EntityData& Entity : CrowdEntities)
    {
        ApplyBehaviorLogic(Entity, DeltaTime);
    }
}

void ACrowd_MassEntityManager::ProcessCrowdMovement(float DeltaTime)
{
    for (FCrowd_EntityData& Entity : CrowdEntities)
    {
        UpdateEntityPosition(Entity, DeltaTime);
    }
}

void ACrowd_MassEntityManager::HandleCrowdCollisions()
{
    for (int32 i = 0; i < CrowdEntities.Num(); i++)
    {
        for (int32 j = i + 1; j < CrowdEntities.Num(); j++)
        {
            if (CheckEntityCollision(CrowdEntities[i], CrowdEntities[j]))
            {
                // Simple collision response - push entities apart
                FVector Direction = (CrowdEntities[j].Position - CrowdEntities[i].Position).GetSafeNormal();
                float PushForce = CollisionRadius * 0.5f;
                
                CrowdEntities[i].Position -= Direction * PushForce;
                CrowdEntities[j].Position += Direction * PushForce;
            }
        }
    }
}

void ACrowd_MassEntityManager::UpdateCrowdLOD()
{
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PlayerController || !PlayerController->GetPawn())
    {
        return;
    }
    
    FVector PlayerLocation = PlayerController->GetPawn()->GetActorLocation();
    
    for (FCrowd_EntityData& Entity : CrowdEntities)
    {
        float DistanceToPlayer = FVector::Dist(Entity.Position, PlayerLocation);
        Entity.DistanceToTarget = DistanceToPlayer;
        
        // LOD-based update frequency could be implemented here
        // For now, all entities update at full rate within update distance
        if (DistanceToPlayer > EntityUpdateDistance)
        {
            // Skip detailed updates for distant entities
            continue;
        }
    }
}

void ACrowd_MassEntityManager::SetEntityBehaviorState(int32 EntityID, ECrowd_BehaviorState NewState)
{
    if (EntityID >= 0 && EntityID < CrowdEntities.Num())
    {
        CrowdEntities[EntityID].BehaviorState = NewState;
    }
}

FVector ACrowd_MassEntityManager::FindNearestWaypoint(const FVector& EntityPosition)
{
    if (Waypoints.Num() == 0)
    {
        return EntityPosition;
    }
    
    FVector NearestWaypoint = Waypoints[0];
    float NearestDistance = FVector::Dist(EntityPosition, Waypoints[0]);
    
    for (const FVector& Waypoint : Waypoints)
    {
        float Distance = FVector::Dist(EntityPosition, Waypoint);
        if (Distance < NearestDistance)
        {
            NearestDistance = Distance;
            NearestWaypoint = Waypoint;
        }
    }
    
    return NearestWaypoint;
}

void ACrowd_MassEntityManager::AddWaypoint(const FVector& WaypointLocation)
{
    Waypoints.Add(WaypointLocation);
}

void ACrowd_MassEntityManager::ClearWaypoints()
{
    Waypoints.Empty();
}

void ACrowd_MassEntityManager::TriggerFleeResponse(const FVector& ThreatLocation, float ThreatRadius)
{
    for (FCrowd_EntityData& Entity : CrowdEntities)
    {
        float DistanceToThreat = FVector::Dist(Entity.Position, ThreatLocation);
        if (DistanceToThreat <= ThreatRadius)
        {
            Entity.BehaviorState = ECrowd_BehaviorState::Fleeing;
            Entity.Speed = FMath::Min(Entity.Speed * 1.5f, 300.0f); // Increase speed when fleeing
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Triggered flee response at location: %s"), *ThreatLocation.ToString());
}

void ACrowd_MassEntityManager::SetGatheringPoint(const FVector& GatherLocation)
{
    GatheringPoint = GatherLocation;
    
    // Set nearby entities to gathering behavior
    for (FCrowd_EntityData& Entity : CrowdEntities)
    {
        float DistanceToGather = FVector::Dist(Entity.Position, GatherLocation);
        if (DistanceToGather <= 500.0f)
        {
            Entity.BehaviorState = ECrowd_BehaviorState::Gathering;
        }
    }
}

void ACrowd_MassEntityManager::StartPatrolBehavior(const TArray<FVector>& PatrolPoints)
{
    PatrolRoute = PatrolPoints;
    
    // Set entities to patrol behavior
    for (FCrowd_EntityData& Entity : CrowdEntities)
    {
        Entity.BehaviorState = ECrowd_BehaviorState::Patrolling;
    }
}

void ACrowd_MassEntityManager::UpdateEntityPosition(FCrowd_EntityData& Entity, float DeltaTime)
{
    if (Entity.Velocity.SizeSquared() > 0.0f)
    {
        Entity.Position += Entity.Velocity * DeltaTime;
        
        // Simple ground constraint
        Entity.Position.Z = FMath::Max(Entity.Position.Z, 50.0f);
    }
}

void ACrowd_MassEntityManager::ApplyBehaviorLogic(FCrowd_EntityData& Entity, float DeltaTime)
{
    FVector TargetDirection = FVector::ZeroVector;
    
    switch (Entity.BehaviorState)
    {
        case ECrowd_BehaviorState::Wandering:
        {
            // Random wandering with waypoint attraction
            FVector NearestWaypoint = FindNearestWaypoint(Entity.Position);
            FVector WaypointDirection = (NearestWaypoint - Entity.Position).GetSafeNormal();
            FVector RandomDirection = FVector(
                FMath::RandRange(-1.0f, 1.0f),
                FMath::RandRange(-1.0f, 1.0f),
                0.0f
            ).GetSafeNormal();
            
            TargetDirection = (WaypointDirection * 0.3f + RandomDirection * 0.7f).GetSafeNormal();
            break;
        }
        
        case ECrowd_BehaviorState::Fleeing:
        {
            TargetDirection = CalculateFleeDirection(Entity, GatheringPoint);
            break;
        }
        
        case ECrowd_BehaviorState::Gathering:
        {
            TargetDirection = CalculateGatherDirection(Entity);
            break;
        }
        
        case ECrowd_BehaviorState::Patrolling:
        {
            TargetDirection = CalculatePatrolDirection(Entity);
            break;
        }
    }
    
    // Apply movement
    Entity.Velocity = TargetDirection * Entity.Speed;
}

FVector ACrowd_MassEntityManager::CalculateFleeDirection(const FCrowd_EntityData& Entity, const FVector& ThreatLocation)
{
    FVector FleeDirection = (Entity.Position - ThreatLocation).GetSafeNormal();
    if (FleeDirection.SizeSquared() < 0.1f)
    {
        // If too close to threat, pick random direction
        FleeDirection = FVector(FMath::RandRange(-1.0f, 1.0f), FMath::RandRange(-1.0f, 1.0f), 0.0f).GetSafeNormal();
    }
    return FleeDirection;
}

FVector ACrowd_MassEntityManager::CalculateGatherDirection(const FCrowd_EntityData& Entity)
{
    return (GatheringPoint - Entity.Position).GetSafeNormal();
}

FVector ACrowd_MassEntityManager::CalculatePatrolDirection(const FCrowd_EntityData& Entity)
{
    if (PatrolRoute.Num() == 0)
    {
        return FVector::ZeroVector;
    }
    
    // Find nearest patrol point
    FVector NearestPatrolPoint = PatrolRoute[0];
    float NearestDistance = FVector::Dist(Entity.Position, PatrolRoute[0]);
    
    for (const FVector& PatrolPoint : PatrolRoute)
    {
        float Distance = FVector::Dist(Entity.Position, PatrolPoint);
        if (Distance < NearestDistance)
        {
            NearestDistance = Distance;
            NearestPatrolPoint = PatrolPoint;
        }
    }
    
    return (NearestPatrolPoint - Entity.Position).GetSafeNormal();
}

int32 ACrowd_MassEntityManager::GetEntityLODLevel(const FCrowd_EntityData& Entity)
{
    if (Entity.DistanceToTarget < LODDistance1)
    {
        return 0; // High detail
    }
    else if (Entity.DistanceToTarget < LODDistance2)
    {
        return 1; // Medium detail
    }
    else
    {
        return 2; // Low detail
    }
}

bool ACrowd_MassEntityManager::CheckEntityCollision(const FCrowd_EntityData& EntityA, const FCrowd_EntityData& EntityB)
{
    float Distance = FVector::Dist(EntityA.Position, EntityB.Position);
    return Distance < (CollisionRadius * 2.0f);
}