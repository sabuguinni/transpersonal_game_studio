#include "Crowd_MassEntityManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassRepresentationFragments.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UCrowd_MassEntityManager::UCrowd_MassEntityManager()
{
    MaxEntityCount = 50000;
    CurrentBehavior = ECrowd_EntityBehavior::Wandering;
    EntitySpeed = 300.0f;
    EntityDetectionRadius = 500.0f;
}

void UCrowd_MassEntityManager::InitializeMassSystem(UWorld* World)
{
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("UCrowd_MassEntityManager: Invalid World"));
        return;
    }

    CachedWorld = World;
    MassSubsystem = World->GetSubsystem<UMassEntitySubsystem>();
    
    if (!MassSubsystem.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("UCrowd_MassEntityManager: Failed to get Mass Entity Subsystem"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("UCrowd_MassEntityManager: Mass system initialized successfully"));
}

void UCrowd_MassEntityManager::SpawnEntityCluster(const FCrowd_EntitySpawnData& SpawnData)
{
    if (!MassSubsystem.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("UCrowd_MassEntityManager: Mass subsystem not initialized"));
        return;
    }

    if (SpawnedEntities.Num() + SpawnData.EntityCount > MaxEntityCount)
    {
        UE_LOG(LogTemp, Warning, TEXT("UCrowd_MassEntityManager: Entity count would exceed maximum (%d)"), MaxEntityCount);
        return;
    }

    // Create entities in a cluster pattern
    for (int32 i = 0; i < SpawnData.EntityCount; i++)
    {
        FMassEntityHandle EntityHandle = MassSubsystem->CreateEntity();
        if (EntityHandle.IsValid())
        {
            // Calculate spawn position within radius
            float Angle = (2.0f * PI * i) / SpawnData.EntityCount;
            float Distance = FMath::RandRange(0.0f, SpawnData.SpawnRadius);
            FVector SpawnLocation = SpawnData.Location + FVector(
                Distance * FMath::Cos(Angle),
                Distance * FMath::Sin(Angle),
                0.0f
            );

            // Add basic fragments
            FMassEntityManager& EntityManager = MassSubsystem->GetMutableEntityManager();
            
            // Transform fragment
            FTransformFragment TransformFragment;
            TransformFragment.GetMutableTransform().SetLocation(SpawnLocation);
            TransformFragment.GetMutableTransform().SetRotation(SpawnData.Rotation.Quaternion());
            EntityManager.AddFragmentToEntity(EntityHandle, TransformFragment);

            // Velocity fragment for movement
            FMassVelocityFragment VelocityFragment;
            VelocityFragment.Value = FVector::ZeroVector;
            EntityManager.AddFragmentToEntity(EntityHandle, VelocityFragment);

            SpawnedEntities.Add(EntityHandle);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("UCrowd_MassEntityManager: Spawned %d entities at %s"), 
           SpawnData.EntityCount, *SpawnData.Location.ToString());
}

void UCrowd_MassEntityManager::UpdateEntityBehavior(ECrowd_EntityBehavior NewBehavior)
{
    CurrentBehavior = NewBehavior;
    
    if (!MassSubsystem.IsValid())
    {
        return;
    }

    FMassEntityManager& EntityManager = MassSubsystem->GetMutableEntityManager();
    
    for (const FMassEntityHandle& EntityHandle : SpawnedEntities)
    {
        if (EntityHandle.IsValid())
        {
            // Update entity behavior based on new behavior type
            FMassVelocityFragment* VelocityFragment = EntityManager.GetFragmentDataPtr<FMassVelocityFragment>(EntityHandle);
            if (VelocityFragment)
            {
                switch (CurrentBehavior)
                {
                case ECrowd_EntityBehavior::Wandering:
                    VelocityFragment->Value = FVector(
                        FMath::RandRange(-EntitySpeed, EntitySpeed),
                        FMath::RandRange(-EntitySpeed, EntitySpeed),
                        0.0f
                    );
                    break;
                case ECrowd_EntityBehavior::Fleeing:
                    VelocityFragment->Value = FVector(EntitySpeed, 0.0f, 0.0f);
                    break;
                case ECrowd_EntityBehavior::Resting:
                    VelocityFragment->Value = FVector::ZeroVector;
                    break;
                default:
                    VelocityFragment->Value = FVector(
                        FMath::RandRange(-EntitySpeed * 0.5f, EntitySpeed * 0.5f),
                        FMath::RandRange(-EntitySpeed * 0.5f, EntitySpeed * 0.5f),
                        0.0f
                    );
                    break;
                }
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("UCrowd_MassEntityManager: Updated behavior to %d for %d entities"), 
           (int32)CurrentBehavior, SpawnedEntities.Num());
}

void UCrowd_MassEntityManager::SetEntityCount(int32 NewCount)
{
    if (NewCount > MaxEntityCount)
    {
        UE_LOG(LogTemp, Warning, TEXT("UCrowd_MassEntityManager: Requested count exceeds maximum"));
        return;
    }

    int32 CurrentCount = SpawnedEntities.Num();
    
    if (NewCount > CurrentCount)
    {
        // Spawn more entities
        FCrowd_EntitySpawnData SpawnData;
        SpawnData.EntityCount = NewCount - CurrentCount;
        SpawnData.Location = FVector::ZeroVector;
        SpawnData.SpawnRadius = 2000.0f;
        SpawnEntityCluster(SpawnData);
    }
    else if (NewCount < CurrentCount)
    {
        // Remove excess entities
        if (MassSubsystem.IsValid())
        {
            FMassEntityManager& EntityManager = MassSubsystem->GetMutableEntityManager();
            for (int32 i = CurrentCount - 1; i >= NewCount; i--)
            {
                if (SpawnedEntities.IsValidIndex(i) && SpawnedEntities[i].IsValid())
                {
                    EntityManager.DestroyEntity(SpawnedEntities[i]);
                    SpawnedEntities.RemoveAt(i);
                }
            }
        }
    }
}

int32 UCrowd_MassEntityManager::GetActiveEntityCount() const
{
    return SpawnedEntities.Num();
}

void UCrowd_MassEntityManager::DestroyAllEntities()
{
    if (!MassSubsystem.IsValid())
    {
        return;
    }

    FMassEntityManager& EntityManager = MassSubsystem->GetMutableEntityManager();
    
    for (const FMassEntityHandle& EntityHandle : SpawnedEntities)
    {
        if (EntityHandle.IsValid())
        {
            EntityManager.DestroyEntity(EntityHandle);
        }
    }
    
    SpawnedEntities.Empty();
    UE_LOG(LogTemp, Log, TEXT("UCrowd_MassEntityManager: Destroyed all entities"));
}

void UCrowd_MassEntityManager::TestSpawnEntities()
{
    if (!CachedWorld.IsValid())
    {
        if (UWorld* World = GEngine->GetWorldFromContextObjectChecked(this))
        {
            InitializeMassSystem(World);
        }
    }

    FCrowd_EntitySpawnData TestSpawnData;
    TestSpawnData.Location = FVector(0.0f, 0.0f, 100.0f);
    TestSpawnData.EntityCount = 500;
    TestSpawnData.SpawnRadius = 1500.0f;
    
    SpawnEntityCluster(TestSpawnData);
    UpdateEntityBehavior(ECrowd_EntityBehavior::Wandering);
}

void UCrowd_MassEntityManager::ProcessEntityMovement()
{
    if (!MassSubsystem.IsValid())
    {
        return;
    }

    FMassEntityManager& EntityManager = MassSubsystem->GetMutableEntityManager();
    
    for (const FMassEntityHandle& EntityHandle : SpawnedEntities)
    {
        if (EntityHandle.IsValid())
        {
            FTransformFragment* TransformFragment = EntityManager.GetFragmentDataPtr<FTransformFragment>(EntityHandle);
            FMassVelocityFragment* VelocityFragment = EntityManager.GetFragmentDataPtr<FMassVelocityFragment>(EntityHandle);
            
            if (TransformFragment && VelocityFragment)
            {
                FVector CurrentLocation = TransformFragment->GetTransform().GetLocation();
                FVector NewLocation = CurrentLocation + (VelocityFragment->Value * GetWorld()->GetDeltaSeconds());
                TransformFragment->GetMutableTransform().SetLocation(NewLocation);
            }
        }
    }
}

void UCrowd_MassEntityManager::UpdateEntityLOD()
{
    // LOD system for performance optimization
    if (!CachedWorld.IsValid())
    {
        return;
    }

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(CachedWorld.Get(), 0);
    if (!PlayerPawn)
    {
        return;
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    for (const FMassEntityHandle& EntityHandle : SpawnedEntities)
    {
        if (EntityHandle.IsValid() && MassSubsystem.IsValid())
        {
            FMassEntityManager& EntityManager = MassSubsystem->GetMutableEntityManager();
            FTransformFragment* TransformFragment = EntityManager.GetFragmentDataPtr<FTransformFragment>(EntityHandle);
            
            if (TransformFragment)
            {
                float Distance = FVector::Dist(PlayerLocation, TransformFragment->GetTransform().GetLocation());
                
                // Adjust update frequency based on distance
                if (Distance > 5000.0f)
                {
                    // Very low LOD - minimal updates
                    continue;
                }
                else if (Distance > 2000.0f)
                {
                    // Medium LOD - reduced updates
                    if (FMath::RandRange(0, 3) != 0)
                    {
                        continue;
                    }
                }
                // High LOD - full updates for close entities
            }
        }
    }
}

void UCrowd_MassEntityManager::HandleEntityCollisions()
{
    if (!MassSubsystem.IsValid())
    {
        return;
    }

    FMassEntityManager& EntityManager = MassSubsystem->GetMutableEntityManager();
    
    // Simple collision avoidance between entities
    for (int32 i = 0; i < SpawnedEntities.Num(); i++)
    {
        if (!SpawnedEntities[i].IsValid())
        {
            continue;
        }

        FTransformFragment* TransformA = EntityManager.GetFragmentDataPtr<FTransformFragment>(SpawnedEntities[i]);
        FMassVelocityFragment* VelocityA = EntityManager.GetFragmentDataPtr<FMassVelocityFragment>(SpawnedEntities[i]);
        
        if (!TransformA || !VelocityA)
        {
            continue;
        }

        FVector LocationA = TransformA->GetTransform().GetLocation();
        
        for (int32 j = i + 1; j < SpawnedEntities.Num(); j++)
        {
            if (!SpawnedEntities[j].IsValid())
            {
                continue;
            }

            FTransformFragment* TransformB = EntityManager.GetFragmentDataPtr<FTransformFragment>(SpawnedEntities[j]);
            FMassVelocityFragment* VelocityB = EntityManager.GetFragmentDataPtr<FMassVelocityFragment>(SpawnedEntities[j]);
            
            if (!TransformB || !VelocityB)
            {
                continue;
            }

            FVector LocationB = TransformB->GetTransform().GetLocation();
            float Distance = FVector::Dist(LocationA, LocationB);
            
            if (Distance < EntityDetectionRadius)
            {
                // Apply separation force
                FVector SeparationVector = (LocationA - LocationB).GetSafeNormal();
                float SeparationForce = (EntityDetectionRadius - Distance) / EntityDetectionRadius;
                
                VelocityA->Value += SeparationVector * SeparationForce * EntitySpeed * 0.1f;
                VelocityB->Value -= SeparationVector * SeparationForce * EntitySpeed * 0.1f;
            }
        }
    }
}