#include "Crowd_MassEntityManager.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Components/SphereComponent.h"

ACrowd_MassEntityManager::ACrowd_MassEntityManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create influence radius component
    InfluenceRadius = CreateDefaultSubobject<USphereComponent>(TEXT("InfluenceRadius"));
    RootComponent = InfluenceRadius;
    InfluenceRadius->SetSphereRadius(1000.0f);

    // Default values
    MaxCrowdEntities = 50;
    SpawnRadius = 800.0f;
    UpdateFrequency = 0.1f;

    // LOD distances
    HighLODDistance = 500.0f;
    MediumLODDistance = 1000.0f;
    LowLODDistance = 2000.0f;
    CullDistance = 3000.0f;

    // Pathfinding
    WaypointRadius = 200.0f;

    // Behavior
    FleeDistance = 300.0f;
    GatherDistance = 150.0f;
    MovementSpeed = 200.0f;

    // Performance
    EntitiesPerFrame = 10;
    bEnablePerformanceThrottling = true;

    // Internal
    LastUpdateTime = 0.0f;
    CurrentUpdateIndex = 0;
    PlayerPawn = nullptr;
}

void ACrowd_MassEntityManager::BeginPlay()
{
    Super::BeginPlay();

    // Get player reference
    PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);

    // Initialize crowd entities
    SpawnCrowdEntities(MaxCrowdEntities);

    // Find waypoints in the level
    TArray<AActor*> FoundWaypoints;
    UGameplayStatics::GetAllActorsOfClass(this, ATargetPoint::StaticClass(), FoundWaypoints);
    
    for (AActor* Actor : FoundWaypoints)
    {
        if (ATargetPoint* Waypoint = Cast<ATargetPoint>(Actor))
        {
            if (Waypoint->GetActorLabel().Contains(TEXT("CrowdWaypoint")))
            {
                WaypointNetwork.Add(Waypoint);
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Crowd Manager initialized with %d entities and %d waypoints"), 
           CrowdEntities.Num(), WaypointNetwork.Num());
}

void ACrowd_MassEntityManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Throttled updates for performance
    if (GetWorld()->GetTimeSeconds() - LastUpdateTime >= UpdateFrequency)
    {
        UpdateCrowdLOD();
        UpdateCrowdBehavior(DeltaTime);
        
        if (bEnablePerformanceThrottling)
        {
            OptimizePerformance();
        }

        LastUpdateTime = GetWorld()->GetTimeSeconds();
    }
}

void ACrowd_MassEntityManager::SpawnCrowdEntities(int32 Count)
{
    CrowdEntities.Empty();
    
    for (int32 i = 0; i < Count; i++)
    {
        FCrowd_EntityData NewEntity;
        
        // Random position within spawn radius
        float Angle = FMath::RandRange(0.0f, 2.0f * PI);
        float Distance = FMath::RandRange(100.0f, SpawnRadius);
        
        NewEntity.Position = GetActorLocation() + FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.0f
        );
        
        NewEntity.Velocity = FVector::ZeroVector;
        NewEntity.LODLevel = ECrowd_LODLevel::High;
        NewEntity.BehaviorMode = ECrowd_BehaviorMode::Wandering;
        NewEntity.EntityID = i;
        
        CrowdEntities.Add(NewEntity);
    }
}

void ACrowd_MassEntityManager::UpdateCrowdLOD()
{
    if (!PlayerPawn)
    {
        PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
        if (!PlayerPawn) return;
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();

    for (FCrowd_EntityData& Entity : CrowdEntities)
    {
        Entity.DistanceToPlayer = FVector::Dist(Entity.Position, PlayerLocation);
        UpdateEntityLOD(Entity);
    }
}

void ACrowd_MassEntityManager::UpdateEntityLOD(FCrowd_EntityData& Entity)
{
    if (Entity.DistanceToPlayer <= HighLODDistance)
    {
        Entity.LODLevel = ECrowd_LODLevel::High;
    }
    else if (Entity.DistanceToPlayer <= MediumLODDistance)
    {
        Entity.LODLevel = ECrowd_LODLevel::Medium;
    }
    else if (Entity.DistanceToPlayer <= LowLODDistance)
    {
        Entity.LODLevel = ECrowd_LODLevel::Low;
    }
    else if (Entity.DistanceToPlayer <= CullDistance)
    {
        Entity.LODLevel = ECrowd_LODLevel::Culled;
    }
}

void ACrowd_MassEntityManager::UpdateCrowdBehavior(float DeltaTime)
{
    int32 EntitiesProcessed = 0;
    
    for (int32 i = CurrentUpdateIndex; i < CrowdEntities.Num() && EntitiesProcessed < EntitiesPerFrame; i++)
    {
        UpdateEntityBehavior(CrowdEntities[i], DeltaTime);
        EntitiesProcessed++;
    }
    
    CurrentUpdateIndex += EntitiesProcessed;
    if (CurrentUpdateIndex >= CrowdEntities.Num())
    {
        CurrentUpdateIndex = 0;
    }
}

void ACrowd_MassEntityManager::UpdateEntityBehavior(FCrowd_EntityData& Entity, float DeltaTime)
{
    // Skip culled entities
    if (Entity.LODLevel == ECrowd_LODLevel::Culled)
    {
        return;
    }

    FVector NewVelocity = FVector::ZeroVector;

    switch (Entity.BehaviorMode)
    {
        case ECrowd_BehaviorMode::Wandering:
            NewVelocity = CalculateWanderDirection(Entity);
            break;
            
        case ECrowd_BehaviorMode::Fleeing:
            if (PlayerPawn)
            {
                NewVelocity = CalculateFleeDirection(Entity.Position, PlayerPawn->GetActorLocation());
            }
            break;
            
        case ECrowd_BehaviorMode::Gathering:
            if (ATargetPoint* NearestWaypoint = GetNearestWaypoint(Entity.Position))
            {
                FVector Direction = (NearestWaypoint->GetActorLocation() - Entity.Position).GetSafeNormal();
                NewVelocity = Direction * MovementSpeed * 0.5f;
            }
            break;
            
        case ECrowd_BehaviorMode::Following:
            if (PlayerPawn && Entity.DistanceToPlayer > GatherDistance)
            {
                FVector Direction = (PlayerPawn->GetActorLocation() - Entity.Position).GetSafeNormal();
                NewVelocity = Direction * MovementSpeed * 0.8f;
            }
            break;
    }

    // Apply velocity based on LOD
    float LODMultiplier = 1.0f;
    switch (Entity.LODLevel)
    {
        case ECrowd_LODLevel::High: LODMultiplier = 1.0f; break;
        case ECrowd_LODLevel::Medium: LODMultiplier = 0.7f; break;
        case ECrowd_LODLevel::Low: LODMultiplier = 0.3f; break;
        default: LODMultiplier = 0.0f; break;
    }

    Entity.Velocity = NewVelocity * LODMultiplier;
    Entity.Position += Entity.Velocity * DeltaTime;
}

FVector ACrowd_MassEntityManager::CalculateFleeDirection(const FVector& EntityPos, const FVector& ThreatPos)
{
    FVector FleeDirection = (EntityPos - ThreatPos).GetSafeNormal();
    return FleeDirection * MovementSpeed * 1.5f; // Flee faster
}

FVector ACrowd_MassEntityManager::CalculateWanderDirection(const FCrowd_EntityData& Entity)
{
    // Simple wander behavior - move towards random waypoint
    if (WaypointNetwork.Num() > 0)
    {
        ATargetPoint* RandomWaypoint = WaypointNetwork[FMath::RandRange(0, WaypointNetwork.Num() - 1)];
        if (RandomWaypoint)
        {
            FVector Direction = (RandomWaypoint->GetActorLocation() - Entity.Position).GetSafeNormal();
            return Direction * MovementSpeed * FMath::RandRange(0.3f, 0.8f);
        }
    }
    
    // Fallback random direction
    FVector RandomDirection = FVector(
        FMath::RandRange(-1.0f, 1.0f),
        FMath::RandRange(-1.0f, 1.0f),
        0.0f
    ).GetSafeNormal();
    
    return RandomDirection * MovementSpeed * 0.5f;
}

ATargetPoint* ACrowd_MassEntityManager::GetNearestWaypoint(const FVector& Position)
{
    ATargetPoint* NearestWaypoint = nullptr;
    float NearestDistance = FLT_MAX;

    for (ATargetPoint* Waypoint : WaypointNetwork)
    {
        if (Waypoint)
        {
            float Distance = FVector::Dist(Position, Waypoint->GetActorLocation());
            if (Distance < NearestDistance)
            {
                NearestDistance = Distance;
                NearestWaypoint = Waypoint;
            }
        }
    }

    return NearestWaypoint;
}

FVector ACrowd_MassEntityManager::GetRandomWaypointPosition()
{
    if (WaypointNetwork.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, WaypointNetwork.Num() - 1);
        if (WaypointNetwork[RandomIndex])
        {
            return WaypointNetwork[RandomIndex]->GetActorLocation();
        }
    }
    
    return GetActorLocation();
}

void ACrowd_MassEntityManager::TriggerFleeResponse(const FVector& ThreatLocation)
{
    for (FCrowd_EntityData& Entity : CrowdEntities)
    {
        float DistanceToThreat = FVector::Dist(Entity.Position, ThreatLocation);
        if (DistanceToThreat <= FleeDistance)
        {
            Entity.BehaviorMode = ECrowd_BehaviorMode::Fleeing;
        }
    }
}

void ACrowd_MassEntityManager::TriggerGatherResponse(const FVector& GatherLocation)
{
    for (FCrowd_EntityData& Entity : CrowdEntities)
    {
        float DistanceToGather = FVector::Dist(Entity.Position, GatherLocation);
        if (DistanceToGather <= GatherDistance * 2.0f)
        {
            Entity.BehaviorMode = ECrowd_BehaviorMode::Gathering;
        }
    }
}

int32 ACrowd_MassEntityManager::GetActiveEntityCount() const
{
    int32 ActiveCount = 0;
    for (const FCrowd_EntityData& Entity : CrowdEntities)
    {
        if (Entity.LODLevel != ECrowd_LODLevel::Culled)
        {
            ActiveCount++;
        }
    }
    return ActiveCount;
}

float ACrowd_MassEntityManager::GetPerformanceMetric() const
{
    if (CrowdEntities.Num() == 0) return 0.0f;
    
    return (float)GetActiveEntityCount() / (float)CrowdEntities.Num();
}

void ACrowd_MassEntityManager::OptimizePerformance()
{
    // Reduce update frequency if too many entities are active
    int32 ActiveCount = GetActiveEntityCount();
    
    if (ActiveCount > MaxCrowdEntities * 0.8f)
    {
        EntitiesPerFrame = FMath::Max(5, EntitiesPerFrame - 1);
    }
    else if (ActiveCount < MaxCrowdEntities * 0.4f)
    {
        EntitiesPerFrame = FMath::Min(20, EntitiesPerFrame + 1);
    }
}