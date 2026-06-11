#include "Crowd_MassEntitySubsystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "AI/NavigationSystemBase.h"

UCrowd_MassEntitySubsystem::UCrowd_MassEntitySubsystem()
{
    MaxCrowdEntities = 50000;
    UpdateFrequency = 0.1f;
    CrowdDensityPerSquareMeter = 2.0f;
    NextEntityID = 1;
    LastUpdateTime = 0.0f;
}

void UCrowd_MassEntitySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    InitializeMassEntitySystem();
    
    UE_LOG(LogTemp, Log, TEXT("Crowd Mass Entity Subsystem initialized - Max entities: %d"), MaxCrowdEntities);
}

void UCrowd_MassEntitySubsystem::Deinitialize()
{
    CleanupMassEntitySystem();
    Super::Deinitialize();
    
    UE_LOG(LogTemp, Log, TEXT("Crowd Mass Entity Subsystem deinitialized"));
}

void UCrowd_MassEntitySubsystem::InitializeMassEntitySystem()
{
    // Initialize the Mass Entity system for crowd simulation
    ActiveEntities.Empty();
    NextEntityID = 1;
    
    // Set up default LOD configuration
    LODConfiguration.HighDetailDistance = 500.0f;
    LODConfiguration.MediumDetailDistance = 1500.0f;
    LODConfiguration.LowDetailDistance = 3000.0f;
    LODConfiguration.CullingDistance = 5000.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Mass Entity system initialized for crowd simulation"));
}

void UCrowd_MassEntitySubsystem::CleanupMassEntitySystem()
{
    // Clean up all active entities
    ActiveEntities.Empty();
    NextEntityID = 1;
    
    UE_LOG(LogTemp, Log, TEXT("Mass Entity system cleaned up"));
}

int32 UCrowd_MassEntitySubsystem::SpawnCrowdEntity(const FVector& Location, ECrowd_BehaviorState InitialState)
{
    if (ActiveEntities.Num() >= MaxCrowdEntities)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot spawn crowd entity - max limit reached: %d"), MaxCrowdEntities);
        return -1;
    }
    
    int32 EntityID = NextEntityID++;
    
    FCrowd_EntityData NewEntity;
    NewEntity.Location = Location;
    NewEntity.Velocity = FVector::ZeroVector;
    NewEntity.Speed = FMath::RandRange(80.0f, 120.0f);
    NewEntity.BehaviorState = InitialState;
    NewEntity.EntityID = EntityID;
    NewEntity.LODDistance = 1000.0f;
    
    ActiveEntities.Add(EntityID, NewEntity);
    
    UE_LOG(LogTemp, Log, TEXT("Spawned crowd entity %d at location %s"), EntityID, *Location.ToString());
    return EntityID;
}

void UCrowd_MassEntitySubsystem::DespawnCrowdEntity(int32 EntityID)
{
    if (ActiveEntities.Contains(EntityID))
    {
        ActiveEntities.Remove(EntityID);
        UE_LOG(LogTemp, Log, TEXT("Despawned crowd entity %d"), EntityID);
    }
}

void UCrowd_MassEntitySubsystem::UpdateCrowdEntities(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastUpdateTime < UpdateFrequency)
    {
        return;
    }
    
    LastUpdateTime = CurrentTime;
    
    // Update all active entities
    for (auto& EntityPair : ActiveEntities)
    {
        int32 EntityID = EntityPair.Key;
        FCrowd_EntityData& Entity = EntityPair.Value;
        
        // Process behavior for this entity
        ProcessCrowdBehavior(EntityID, DeltaTime);
        
        // Update position based on velocity
        Entity.Location += Entity.Velocity * DeltaTime;
        
        // Handle interactions with other entities
        HandleCrowdInteractions(EntityID);
    }
}

void UCrowd_MassEntitySubsystem::SetEntityBehaviorState(int32 EntityID, ECrowd_BehaviorState NewState)
{
    if (FCrowd_EntityData* Entity = ActiveEntities.Find(EntityID))
    {
        Entity->BehaviorState = NewState;
        UE_LOG(LogTemp, Log, TEXT("Entity %d behavior state changed to %d"), EntityID, (int32)NewState);
    }
}

void UCrowd_MassEntitySubsystem::UpdateEntityLOD(int32 EntityID, const FVector& ViewerLocation)
{
    if (FCrowd_EntityData* Entity = ActiveEntities.Find(EntityID))
    {
        float Distance = FVector::Dist(Entity->Location, ViewerLocation);
        Entity->LODDistance = Distance;
        
        ECrowd_LODLevel LODLevel = GetEntityLODLevel(EntityID, ViewerLocation);
        ApplyLODOptimizations(EntityID, LODLevel);
    }
}

ECrowd_LODLevel UCrowd_MassEntitySubsystem::GetEntityLODLevel(int32 EntityID, const FVector& ViewerLocation)
{
    if (FCrowd_EntityData* Entity = ActiveEntities.Find(EntityID))
    {
        float Distance = Entity->LODDistance;
        
        if (Distance <= LODConfiguration.HighDetailDistance)
        {
            return ECrowd_LODLevel::High;
        }
        else if (Distance <= LODConfiguration.MediumDetailDistance)
        {
            return ECrowd_LODLevel::Medium;
        }
        else if (Distance <= LODConfiguration.LowDetailDistance)
        {
            return ECrowd_LODLevel::Low;
        }
        else if (Distance <= LODConfiguration.CullingDistance)
        {
            return ECrowd_LODLevel::Culled;
        }
    }
    
    return ECrowd_LODLevel::Culled;
}

FVector UCrowd_MassEntitySubsystem::GetNextPathPoint(int32 EntityID, const FVector& CurrentLocation)
{
    // Simple pathfinding - move towards a random nearby point
    FVector RandomDirection = FVector(
        FMath::RandRange(-1.0f, 1.0f),
        FMath::RandRange(-1.0f, 1.0f),
        0.0f
    ).GetSafeNormal();
    
    return CurrentLocation + RandomDirection * FMath::RandRange(100.0f, 300.0f);
}

void UCrowd_MassEntitySubsystem::SetEntityDestination(int32 EntityID, const FVector& Destination)
{
    if (FCrowd_EntityData* Entity = ActiveEntities.Find(EntityID))
    {
        FVector Direction = (Destination - Entity->Location).GetSafeNormal();
        Entity->Velocity = Direction * Entity->Speed;
    }
}

void UCrowd_MassEntitySubsystem::ProcessCrowdBehavior(int32 EntityID, float DeltaTime)
{
    FCrowd_EntityData* Entity = ActiveEntities.Find(EntityID);
    if (!Entity)
    {
        return;
    }
    
    switch (Entity->BehaviorState)
    {
        case ECrowd_BehaviorState::Idle:
        {
            // Randomly start moving
            if (FMath::RandRange(0.0f, 1.0f) < 0.1f * DeltaTime)
            {
                Entity->BehaviorState = ECrowd_BehaviorState::Walking;
                FVector RandomDestination = GetNextPathPoint(EntityID, Entity->Location);
                SetEntityDestination(EntityID, RandomDestination);
            }
            break;
        }
        
        case ECrowd_BehaviorState::Walking:
        {
            // Apply steering forces
            FVector SteeringForce = CalculateSteeringForce(*Entity);
            Entity->Velocity += SteeringForce * DeltaTime;
            Entity->Velocity = Entity->Velocity.GetClampedToMaxSize(Entity->Speed);
            
            // Randomly change to idle or running
            float RandomValue = FMath::RandRange(0.0f, 1.0f);
            if (RandomValue < 0.05f * DeltaTime)
            {
                Entity->BehaviorState = ECrowd_BehaviorState::Idle;
                Entity->Velocity = FVector::ZeroVector;
            }
            else if (RandomValue < 0.02f * DeltaTime)
            {
                Entity->BehaviorState = ECrowd_BehaviorState::Running;
                Entity->Speed *= 2.0f;
            }
            break;
        }
        
        case ECrowd_BehaviorState::Running:
        {
            // Apply stronger steering forces
            FVector SteeringForce = CalculateSteeringForce(*Entity) * 1.5f;
            Entity->Velocity += SteeringForce * DeltaTime;
            Entity->Velocity = Entity->Velocity.GetClampedToMaxSize(Entity->Speed);
            
            // Randomly slow down to walking
            if (FMath::RandRange(0.0f, 1.0f) < 0.1f * DeltaTime)
            {
                Entity->BehaviorState = ECrowd_BehaviorState::Walking;
                Entity->Speed *= 0.5f;
            }
            break;
        }
        
        case ECrowd_BehaviorState::Gathering:
        {
            // Move towards center of nearby entities
            TArray<FCrowd_EntityData> NearbyEntities = GetNearbyEntities(Entity->Location, 200.0f);
            if (NearbyEntities.Num() > 0)
            {
                FVector CenterPoint = FVector::ZeroVector;
                for (const FCrowd_EntityData& NearbyEntity : NearbyEntities)
                {
                    CenterPoint += NearbyEntity.Location;
                }
                CenterPoint /= NearbyEntities.Num();
                
                FVector Direction = (CenterPoint - Entity->Location).GetSafeNormal();
                Entity->Velocity = Direction * Entity->Speed * 0.5f;
            }
            break;
        }
        
        case ECrowd_BehaviorState::Fleeing:
        {
            // Move away from danger (simplified - move in random direction quickly)
            if (Entity->Velocity.IsNearlyZero())
            {
                FVector FleeDirection = FVector(
                    FMath::RandRange(-1.0f, 1.0f),
                    FMath::RandRange(-1.0f, 1.0f),
                    0.0f
                ).GetSafeNormal();
                
                Entity->Velocity = FleeDirection * Entity->Speed * 1.5f;
            }
            
            // Randomly calm down
            if (FMath::RandRange(0.0f, 1.0f) < 0.05f * DeltaTime)
            {
                Entity->BehaviorState = ECrowd_BehaviorState::Walking;
                Entity->Speed *= 0.67f;
            }
            break;
        }
    }
}

void UCrowd_MassEntitySubsystem::HandleCrowdInteractions(int32 EntityID)
{
    FCrowd_EntityData* Entity = ActiveEntities.Find(EntityID);
    if (!Entity)
    {
        return;
    }
    
    // Simple collision avoidance with nearby entities
    TArray<FCrowd_EntityData> NearbyEntities = GetNearbyEntities(Entity->Location, 50.0f);
    
    for (const FCrowd_EntityData& NearbyEntity : NearbyEntities)
    {
        if (NearbyEntity.EntityID != EntityID)
        {
            FVector AvoidanceDirection = (Entity->Location - NearbyEntity.Location).GetSafeNormal();
            Entity->Velocity += AvoidanceDirection * 50.0f;
        }
    }
}

FVector UCrowd_MassEntitySubsystem::CalculateSteeringForce(const FCrowd_EntityData& Entity)
{
    // Simple steering behavior - add some randomness and avoid boundaries
    FVector SteeringForce = FVector::ZeroVector;
    
    // Add random wandering force
    FVector RandomForce = FVector(
        FMath::RandRange(-1.0f, 1.0f),
        FMath::RandRange(-1.0f, 1.0f),
        0.0f
    ) * 20.0f;
    
    SteeringForce += RandomForce;
    
    // Boundary avoidance (simple version)
    float BoundaryDistance = 2000.0f;
    if (FMath::Abs(Entity.Location.X) > BoundaryDistance)
    {
        SteeringForce.X += (Entity.Location.X > 0) ? -100.0f : 100.0f;
    }
    if (FMath::Abs(Entity.Location.Y) > BoundaryDistance)
    {
        SteeringForce.Y += (Entity.Location.Y > 0) ? -100.0f : 100.0f;
    }
    
    return SteeringForce;
}

void UCrowd_MassEntitySubsystem::ApplyLODOptimizations(int32 EntityID, ECrowd_LODLevel LODLevel)
{
    // Apply different optimizations based on LOD level
    switch (LODLevel)
    {
        case ECrowd_LODLevel::High:
            // Full detail - no optimizations
            break;
            
        case ECrowd_LODLevel::Medium:
            // Reduce update frequency
            break;
            
        case ECrowd_LODLevel::Low:
            // Minimal updates, simplified behavior
            break;
            
        case ECrowd_LODLevel::Culled:
            // No visual updates, minimal simulation
            break;
    }
}

void UCrowd_MassEntitySubsystem::SetMaxCrowdEntities(int32 MaxEntities)
{
    MaxCrowdEntities = FMath::Max(1, MaxEntities);
    UE_LOG(LogTemp, Log, TEXT("Max crowd entities set to: %d"), MaxCrowdEntities);
}

void UCrowd_MassEntitySubsystem::SetLODConfiguration(const FCrowd_LODConfig& NewLODConfig)
{
    LODConfiguration = NewLODConfig;
    UE_LOG(LogTemp, Log, TEXT("LOD configuration updated"));
}

int32 UCrowd_MassEntitySubsystem::GetActiveCrowdEntityCount() const
{
    return ActiveEntities.Num();
}

TArray<FCrowd_EntityData> UCrowd_MassEntitySubsystem::GetNearbyEntities(const FVector& Location, float Radius)
{
    TArray<FCrowd_EntityData> NearbyEntities;
    
    for (const auto& EntityPair : ActiveEntities)
    {
        const FCrowd_EntityData& Entity = EntityPair.Value;
        float Distance = FVector::Dist(Entity.Location, Location);
        
        if (Distance <= Radius)
        {
            NearbyEntities.Add(Entity);
        }
    }
    
    return NearbyEntities;
}