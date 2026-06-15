#include "Crowd_MassEntityManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ACrowd_MassEntityManager::ACrowd_MassEntityManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    MaxEntities = 1000;
    UpdateFrequency = 0.1f;
    bEnableLODSystem = true;
    bEnablePathfinding = true;
    LastUpdateTime = 0.0f;
    
    // Initialize LOD settings
    LODSettings = FCrowd_LODSettings();
}

void ACrowd_MassEntityManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeMassSystem();
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd Mass Entity Manager initialized with max %d entities"), MaxEntities);
}

void ACrowd_MassEntityManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    LastUpdateTime += DeltaTime;
    
    if (LastUpdateTime >= UpdateFrequency)
    {
        if (bEnableLODSystem)
        {
            UpdateLODSystem();
        }
        
        // Update all entities
        for (int32 i = 0; i < Entities.Num(); i++)
        {
            UpdateEntityBehavior(i, LastUpdateTime);
            ProcessEntityMovement(i, LastUpdateTime);
        }
        
        OptimizeEntityCount();
        LastUpdateTime = 0.0f;
    }
}

void ACrowd_MassEntityManager::SpawnEntity(FVector Location, ECrowd_BehaviorState InitialState)
{
    if (Entities.Num() >= MaxEntities)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot spawn entity - max limit reached: %d"), MaxEntities);
        return;
    }
    
    FCrowd_EntityData NewEntity;
    NewEntity.Position = Location;
    NewEntity.Velocity = FVector::ZeroVector;
    NewEntity.Speed = FMath::RandRange(80.0f, 120.0f);
    NewEntity.BehaviorState = InitialState;
    NewEntity.GroupID = FMath::RandRange(0, 10);
    NewEntity.LODDistance = 0.0f;
    
    Entities.Add(NewEntity);
    
    UE_LOG(LogTemp, Log, TEXT("Spawned crowd entity at location: %s"), *Location.ToString());
}

void ACrowd_MassEntityManager::RemoveEntity(int32 EntityIndex)
{
    if (Entities.IsValidIndex(EntityIndex))
    {
        Entities.RemoveAt(EntityIndex);
        
        // Update LOD arrays
        HighDetailEntities.Remove(EntityIndex);
        MediumDetailEntities.Remove(EntityIndex);
        LowDetailEntities.Remove(EntityIndex);
    }
}

void ACrowd_MassEntityManager::UpdateEntityLOD(int32 EntityIndex, float DistanceToPlayer)
{
    if (!Entities.IsValidIndex(EntityIndex))
        return;
    
    Entities[EntityIndex].LODDistance = DistanceToPlayer;
    
    // Remove from all LOD arrays first
    HighDetailEntities.Remove(EntityIndex);
    MediumDetailEntities.Remove(EntityIndex);
    LowDetailEntities.Remove(EntityIndex);
    
    // Add to appropriate LOD array
    if (DistanceToPlayer <= LODSettings.HighDetailDistance)
    {
        if (HighDetailEntities.Num() < LODSettings.MaxHighDetailEntities)
        {
            HighDetailEntities.Add(EntityIndex);
        }
    }
    else if (DistanceToPlayer <= LODSettings.MediumDetailDistance)
    {
        if (MediumDetailEntities.Num() < LODSettings.MaxMediumDetailEntities)
        {
            MediumDetailEntities.Add(EntityIndex);
        }
    }
    else if (DistanceToPlayer <= LODSettings.LowDetailDistance)
    {
        if (LowDetailEntities.Num() < LODSettings.MaxLowDetailEntities)
        {
            LowDetailEntities.Add(EntityIndex);
        }
    }
}

void ACrowd_MassEntityManager::UpdateEntityBehavior(int32 EntityIndex, float DeltaTime)
{
    if (!Entities.IsValidIndex(EntityIndex))
        return;
    
    FCrowd_EntityData& Entity = Entities[EntityIndex];
    
    switch (Entity.BehaviorState)
    {
        case ECrowd_BehaviorState::Idle:
            // Random chance to start wandering
            if (FMath::RandRange(0.0f, 1.0f) < 0.1f * DeltaTime)
            {
                Entity.BehaviorState = ECrowd_BehaviorState::Wandering;
                Entity.Velocity = FVector(FMath::RandRange(-1.0f, 1.0f), FMath::RandRange(-1.0f, 1.0f), 0.0f).GetSafeNormal();
            }
            break;
            
        case ECrowd_BehaviorState::Wandering:
            // Random chance to stop and idle
            if (FMath::RandRange(0.0f, 1.0f) < 0.05f * DeltaTime)
            {
                Entity.BehaviorState = ECrowd_BehaviorState::Idle;
                Entity.Velocity = FVector::ZeroVector;
            }
            break;
            
        case ECrowd_BehaviorState::Working:
            // Work for a while then rest
            if (FMath::RandRange(0.0f, 1.0f) < 0.02f * DeltaTime)
            {
                Entity.BehaviorState = ECrowd_BehaviorState::Idle;
            }
            break;
            
        case ECrowd_BehaviorState::Socializing:
            // Social interaction duration
            if (FMath::RandRange(0.0f, 1.0f) < 0.03f * DeltaTime)
            {
                Entity.BehaviorState = ECrowd_BehaviorState::Wandering;
            }
            break;
    }
}

void ACrowd_MassEntityManager::ProcessEntityMovement(int32 EntityIndex, float DeltaTime)
{
    if (!Entities.IsValidIndex(EntityIndex))
        return;
    
    FCrowd_EntityData& Entity = Entities[EntityIndex];
    
    if (Entity.BehaviorState == ECrowd_BehaviorState::Wandering)
    {
        FVector Movement = CalculateEntityMovement(Entity, DeltaTime);
        Entity.Position += Movement;
        
        // Keep entities within reasonable bounds
        Entity.Position.X = FMath::Clamp(Entity.Position.X, -10000.0f, 10000.0f);
        Entity.Position.Y = FMath::Clamp(Entity.Position.Y, -10000.0f, 10000.0f);
        Entity.Position.Z = FMath::Max(Entity.Position.Z, 100.0f);
    }
}

int32 ACrowd_MassEntityManager::GetActiveEntityCount() const
{
    return Entities.Num();
}

TArray<FCrowd_EntityData> ACrowd_MassEntityManager::GetEntitiesInRadius(FVector Center, float Radius) const
{
    TArray<FCrowd_EntityData> EntitiesInRadius;
    
    for (const FCrowd_EntityData& Entity : Entities)
    {
        float Distance = FVector::Dist(Entity.Position, Center);
        if (Distance <= Radius)
        {
            EntitiesInRadius.Add(Entity);
        }
    }
    
    return EntitiesInRadius;
}

void ACrowd_MassEntityManager::InitializeMassSystem()
{
    // Clear existing entities
    Entities.Empty();
    HighDetailEntities.Empty();
    MediumDetailEntities.Empty();
    LowDetailEntities.Empty();
    
    // Spawn initial entities in various locations
    TArray<FVector> SpawnLocations = {
        FVector(2800, -1800, 150),  // Gathering area
        FVector(3200, -2200, 150),  // Crafting area
        FVector(2900, -2100, 150),  // Food preparation
        FVector(3100, -1900, 150),  // Tool making
        FVector(3000, -1700, 150),  // Lookout point
        FVector(1000, 1000, 150),   // Random area 1
        FVector(-1000, 2000, 150),  // Random area 2
        FVector(5000, 0, 150)       // Random area 3
    };
    
    for (int32 i = 0; i < FMath::Min(50, MaxEntities); i++)
    {
        FVector SpawnLoc = SpawnLocations[i % SpawnLocations.Num()];
        SpawnLoc += FVector(FMath::RandRange(-200.0f, 200.0f), FMath::RandRange(-200.0f, 200.0f), 0.0f);
        
        ECrowd_BehaviorState InitialState = static_cast<ECrowd_BehaviorState>(FMath::RandRange(0, 3));
        SpawnEntity(SpawnLoc, InitialState);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Mass Entity System initialized with %d entities"), Entities.Num());
}

void ACrowd_MassEntityManager::UpdateLODSystem()
{
    // Get player location for distance calculations
    APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!Player)
        return;
    
    FVector PlayerLocation = Player->GetActorLocation();
    
    // Update LOD for all entities
    for (int32 i = 0; i < Entities.Num(); i++)
    {
        float Distance = FVector::Dist(Entities[i].Position, PlayerLocation);
        UpdateEntityLOD(i, Distance);
    }
}

void ACrowd_MassEntityManager::OptimizeEntityCount()
{
    // Remove excess entities if we're over the limit
    while (Entities.Num() > MaxEntities)
    {
        // Remove the entity with highest LOD distance (furthest from player)
        int32 FurthestIndex = 0;
        float MaxDistance = 0.0f;
        
        for (int32 i = 0; i < Entities.Num(); i++)
        {
            if (Entities[i].LODDistance > MaxDistance)
            {
                MaxDistance = Entities[i].LODDistance;
                FurthestIndex = i;
            }
        }
        
        RemoveEntity(FurthestIndex);
    }
}

FVector ACrowd_MassEntityManager::CalculateEntityMovement(const FCrowd_EntityData& Entity, float DeltaTime)
{
    FVector Movement = Entity.Velocity * Entity.Speed * DeltaTime;
    
    // Add some randomness to movement
    FVector RandomOffset = FVector(
        FMath::RandRange(-50.0f, 50.0f),
        FMath::RandRange(-50.0f, 50.0f),
        0.0f
    ) * DeltaTime;
    
    return Movement + RandomOffset;
}