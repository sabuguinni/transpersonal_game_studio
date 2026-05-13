#include "Crowd_MassEntityManager.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"

ACrowd_MassEntityManager::ACrowd_MassEntityManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize default values
    CurrentEntityCount = 0;
    AverageFrameTime = 16.67f; // 60 FPS target
    LastUpdateTime = 0.0f;
    UpdateFrequency = 0.1f; // Update 10 times per second
    LODDistance = 5000.0f;
    bEnableCollisions = true;
    bEnableLOD = true;
    NextGroupID = 0;
    
    // Set default spawn config
    DefaultSpawnConfig.MaxEntities = 1000;
    DefaultSpawnConfig.SpawnRadius = 5000.0f;
    DefaultSpawnConfig.SpawnCenter = FVector::ZeroVector;
    DefaultSpawnConfig.EntityType = ECrowd_EntityType::Human;
    DefaultSpawnConfig.MinDistance = 100.0f;
    DefaultSpawnConfig.MaxDistance = 200.0f;
}

void ACrowd_MassEntityManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeMassEntitySystem();
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: BeginPlay - System initialized"));
}

void ACrowd_MassEntityManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    DestroyAllEntities();
    Super::EndPlay(EndPlayReason);
}

void ACrowd_MassEntityManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    LastUpdateTime += DeltaTime;
    
    if (LastUpdateTime >= UpdateFrequency)
    {
        UpdateEntityBehaviors(DeltaTime);
        UpdateEntityPositions(DeltaTime);
        
        if (bEnableCollisions)
        {
            ProcessEntityCollisions();
        }
        
        if (bEnableLOD)
        {
            UpdateEntityLOD();
        }
        
        UpdatePerformanceMetrics(DeltaTime);
        LastUpdateTime = 0.0f;
    }
}

void ACrowd_MassEntityManager::InitializeMassEntitySystem()
{
    if (UWorld* World = GetWorld())
    {
        MassEntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
        
        if (MassEntitySubsystem)
        {
            UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: Mass Entity Subsystem initialized successfully"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Crowd_MassEntityManager: Failed to get Mass Entity Subsystem"));
        }
    }
}

void ACrowd_MassEntityManager::SpawnMassEntities(const FCrowd_SpawnConfig& Config)
{
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("Crowd_MassEntityManager: Mass Entity Subsystem not available"));
        return;
    }
    
    // Clear existing entities
    DestroyAllEntities();
    
    // Spawn new entities in a circle pattern
    for (int32 i = 0; i < Config.MaxEntities; ++i)
    {
        float Angle = (2.0f * PI * i) / Config.MaxEntities;
        float Distance = FMath::RandRange(Config.MinDistance, Config.MaxDistance);
        
        FVector SpawnPosition = Config.SpawnCenter + FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.0f
        );
        
        // Add some random height variation
        SpawnPosition.Z += FMath::RandRange(-50.0f, 50.0f);
        
        FMassEntityHandle EntityHandle = CreateEntity(SpawnPosition, Config.EntityType);
        
        if (EntityHandle.IsValid())
        {
            CurrentEntityCount++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: Spawned %d entities"), CurrentEntityCount);
}

FMassEntityHandle ACrowd_MassEntityManager::CreateEntity(const FVector& Position, ECrowd_EntityType Type)
{
    if (!MassEntitySubsystem)
    {
        return FMassEntityHandle();
    }
    
    // Create entity data
    FCrowd_EntityData EntityData;
    EntityData.Position = Position;
    EntityData.Velocity = FVector::ZeroVector;
    EntityData.BehaviorState = ECrowd_BehaviorState::Idle;
    EntityData.Health = 100.0f;
    EntityData.Fear = 0.0f;
    EntityData.GroupID = -1;
    
    // Create the entity handle (simplified for now)
    FMassEntityHandle EntityHandle;
    
    // Store entity data
    ActiveEntities.Add(EntityData);
    
    return EntityHandle;
}

void ACrowd_MassEntityManager::DestroyEntity(FMassEntityHandle EntityHandle)
{
    // Remove from active entities
    ActiveEntities.RemoveAll([EntityHandle](const FCrowd_EntityData& Data)
    {
        return Data.EntityHandle == EntityHandle;
    });
    
    CurrentEntityCount = ActiveEntities.Num();
}

void ACrowd_MassEntityManager::UpdateEntityBehaviors(float DeltaTime)
{
    for (FCrowd_EntityData& EntityData : ActiveEntities)
    {
        switch (EntityData.BehaviorState)
        {
            case ECrowd_BehaviorState::Idle:
                // Random movement
                if (FMath::RandRange(0.0f, 1.0f) < 0.1f) // 10% chance to start moving
                {
                    EntityData.BehaviorState = ECrowd_BehaviorState::Moving;
                    EntityData.Velocity = FVector(
                        FMath::RandRange(-100.0f, 100.0f),
                        FMath::RandRange(-100.0f, 100.0f),
                        0.0f
                    );
                }
                break;
                
            case ECrowd_BehaviorState::Moving:
                // Continue moving or stop
                if (FMath::RandRange(0.0f, 1.0f) < 0.05f) // 5% chance to stop
                {
                    EntityData.BehaviorState = ECrowd_BehaviorState::Idle;
                    EntityData.Velocity = FVector::ZeroVector;
                }
                break;
                
            case ECrowd_BehaviorState::Fleeing:
                // Increase speed when fleeing
                EntityData.Velocity *= 1.5f;
                EntityData.Fear += DeltaTime * 10.0f;
                
                // Stop fleeing after some time
                if (EntityData.Fear > 50.0f)
                {
                    EntityData.BehaviorState = ECrowd_BehaviorState::Idle;
                    EntityData.Fear = FMath::Max(0.0f, EntityData.Fear - DeltaTime * 20.0f);
                }
                break;
                
            case ECrowd_BehaviorState::Following:
                // Group following behavior
                if (EntityData.GroupID >= 0)
                {
                    // Follow group leader (simplified)
                    EntityData.Velocity = FVector(50.0f, 0.0f, 0.0f);
                }
                break;
        }
    }
}

void ACrowd_MassEntityManager::UpdateEntityPositions(float DeltaTime)
{
    for (FCrowd_EntityData& EntityData : ActiveEntities)
    {
        EntityData.Position += EntityData.Velocity * DeltaTime;
        
        // Simple boundary checking
        float MaxDistance = 10000.0f;
        if (EntityData.Position.Size() > MaxDistance)
        {
            EntityData.Position = EntityData.Position.GetSafeNormal() * MaxDistance;
            EntityData.Velocity = -EntityData.Velocity * 0.5f; // Bounce back
        }
    }
}

void ACrowd_MassEntityManager::ProcessEntityCollisions()
{
    float CollisionDistance = 50.0f;
    
    for (int32 i = 0; i < ActiveEntities.Num(); ++i)
    {
        for (int32 j = i + 1; j < ActiveEntities.Num(); ++j)
        {
            FCrowd_EntityData& EntityA = ActiveEntities[i];
            FCrowd_EntityData& EntityB = ActiveEntities[j];
            
            float Distance = FVector::Dist(EntityA.Position, EntityB.Position);
            
            if (Distance < CollisionDistance)
            {
                // Simple collision response
                FVector Direction = (EntityA.Position - EntityB.Position).GetSafeNormal();
                float PushForce = (CollisionDistance - Distance) * 0.5f;
                
                EntityA.Position += Direction * PushForce;
                EntityB.Position -= Direction * PushForce;
                
                // Adjust velocities
                EntityA.Velocity += Direction * 25.0f;
                EntityB.Velocity -= Direction * 25.0f;
            }
        }
    }
}

void ACrowd_MassEntityManager::ProcessEntityInteractions()
{
    // Process interactions between entities
    for (FCrowd_EntityData& EntityData : ActiveEntities)
    {
        // Check for nearby entities and react
        for (const FCrowd_EntityData& OtherEntity : ActiveEntities)
        {
            if (&EntityData == &OtherEntity) continue;
            
            float Distance = FVector::Dist(EntityData.Position, OtherEntity.Position);
            
            if (Distance < 200.0f) // Interaction range
            {
                // Social behaviors based on entity states
                if (OtherEntity.BehaviorState == ECrowd_BehaviorState::Fleeing)
                {
                    EntityData.Fear += 5.0f;
                    if (EntityData.Fear > 30.0f)
                    {
                        EntityData.BehaviorState = ECrowd_BehaviorState::Fleeing;
                    }
                }
            }
        }
    }
}

void ACrowd_MassEntityManager::UpdateEntityLOD()
{
    if (!GetWorld() || !GetWorld()->GetFirstPlayerController())
    {
        return;
    }
    
    FVector PlayerLocation = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();
    
    for (FCrowd_EntityData& EntityData : ActiveEntities)
    {
        float DistanceToPlayer = FVector::Dist(EntityData.Position, PlayerLocation);
        
        // Simple LOD system - entities far from player are less active
        if (DistanceToPlayer > LODDistance)
        {
            // Reduce update frequency for distant entities
            if (FMath::RandRange(0.0f, 1.0f) < 0.1f) // Only update 10% of distant entities
            {
                EntityData.BehaviorState = ECrowd_BehaviorState::Idle;
                EntityData.Velocity *= 0.5f;
            }
        }
    }
}

void ACrowd_MassEntityManager::SetEntityBehavior(FMassEntityHandle EntityHandle, ECrowd_BehaviorState NewState)
{
    for (FCrowd_EntityData& EntityData : ActiveEntities)
    {
        if (EntityData.EntityHandle == EntityHandle)
        {
            EntityData.BehaviorState = NewState;
            break;
        }
    }
}

FCrowd_EntityData ACrowd_MassEntityManager::GetEntityData(FMassEntityHandle EntityHandle)
{
    for (const FCrowd_EntityData& EntityData : ActiveEntities)
    {
        if (EntityData.EntityHandle == EntityHandle)
        {
            return EntityData;
        }
    }
    
    return FCrowd_EntityData(); // Return default if not found
}

int32 ACrowd_MassEntityManager::CreateGroup(const TArray<FMassEntityHandle>& Entities)
{
    int32 GroupID = NextGroupID++;
    Groups.Add(GroupID, Entities);
    
    // Assign group ID to entities
    for (FMassEntityHandle EntityHandle : Entities)
    {
        for (FCrowd_EntityData& EntityData : ActiveEntities)
        {
            if (EntityData.EntityHandle == EntityHandle)
            {
                EntityData.GroupID = GroupID;
                break;
            }
        }
    }
    
    return GroupID;
}

void ACrowd_MassEntityManager::SetGroupBehavior(int32 GroupID, ECrowd_BehaviorState NewState)
{
    if (TArray<FMassEntityHandle>* GroupEntities = Groups.Find(GroupID))
    {
        for (FMassEntityHandle EntityHandle : *GroupEntities)
        {
            SetEntityBehavior(EntityHandle, NewState);
        }
    }
}

void ACrowd_MassEntityManager::MoveGroup(int32 GroupID, const FVector& TargetLocation)
{
    if (TArray<FMassEntityHandle>* GroupEntities = Groups.Find(GroupID))
    {
        for (FMassEntityHandle EntityHandle : *GroupEntities)
        {
            for (FCrowd_EntityData& EntityData : ActiveEntities)
            {
                if (EntityData.EntityHandle == EntityHandle)
                {
                    FVector Direction = (TargetLocation - EntityData.Position).GetSafeNormal();
                    EntityData.Velocity = Direction * 150.0f; // Group movement speed
                    EntityData.BehaviorState = ECrowd_BehaviorState::Following;
                    break;
                }
            }
        }
    }
}

void ACrowd_MassEntityManager::DestroyAllEntities()
{
    ActiveEntities.Empty();
    Groups.Empty();
    CurrentEntityCount = 0;
    NextGroupID = 0;
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: All entities destroyed"));
}

int32 ACrowd_MassEntityManager::GetActiveEntityCount() const
{
    return CurrentEntityCount;
}

float ACrowd_MassEntityManager::GetAverageFrameTime() const
{
    return AverageFrameTime;
}

void ACrowd_MassEntityManager::SetLODDistance(float Distance)
{
    LODDistance = Distance;
}

void ACrowd_MassEntityManager::UpdatePerformanceMetrics(float DeltaTime)
{
    // Simple moving average for frame time
    AverageFrameTime = (AverageFrameTime * 0.9f) + (DeltaTime * 1000.0f * 0.1f);
    
    // Optimize entity count if performance is poor
    if (AverageFrameTime > 33.0f) // Worse than 30 FPS
    {
        OptimizeEntityCount();
    }
}

void ACrowd_MassEntityManager::OptimizeEntityCount()
{
    if (ActiveEntities.Num() > 500)
    {
        // Remove some entities to improve performance
        int32 EntitiesToRemove = ActiveEntities.Num() * 0.1f; // Remove 10%
        
        for (int32 i = 0; i < EntitiesToRemove; ++i)
        {
            if (ActiveEntities.Num() > 0)
            {
                ActiveEntities.RemoveAt(ActiveEntities.Num() - 1);
                CurrentEntityCount--;
            }
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: Optimized entity count to %d"), CurrentEntityCount);
    }
}

bool ACrowd_MassEntityManager::IsEntityInLODRange(const FVector& EntityPosition) const
{
    if (!GetWorld() || !GetWorld()->GetFirstPlayerController())
    {
        return true; // Default to in range if no player
    }
    
    FVector PlayerLocation = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();
    float Distance = FVector::Dist(EntityPosition, PlayerLocation);
    
    return Distance <= LODDistance;
}