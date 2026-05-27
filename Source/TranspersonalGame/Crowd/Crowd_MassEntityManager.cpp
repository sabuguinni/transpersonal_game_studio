#include "Crowd_MassEntityManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassRepresentationFragments.h"
#include "Engine/StaticMesh.h"

ACrowd_MassEntityManager::ACrowd_MassEntityManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create manager mesh component
    ManagerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ManagerMesh"));
    RootComponent = ManagerMesh;

    // Load a simple cube mesh for visualization
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        ManagerMesh->SetStaticMesh(CubeMeshAsset.Object);
        ManagerMesh->SetWorldScale3D(FVector(0.5f, 0.5f, 0.5f));
    }

    // Initialize default values
    MaxEntities = 1000;
    CrowdDensity = 1.0f;
    DefaultBehavior = ECrowd_BehaviorType::Wandering;
    UpdateFrequency = 0.1f;
    LastUpdateTime = 0.0f;
}

void ACrowd_MassEntityManager::BeginPlay()
{
    Super::BeginPlay();

    InitializeMassEntity();

    // Auto-spawn some entities for testing
    SpawnCrowdEntities(50, GetActorLocation(), 2000.0f);

    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: BeginPlay - Spawned initial crowd"));
}

void ACrowd_MassEntityManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    LastUpdateTime += DeltaTime;
    if (LastUpdateTime >= UpdateFrequency)
    {
        UpdateEntityBehaviors();
        CleanupInvalidEntities();
        LastUpdateTime = 0.0f;
    }
}

void ACrowd_MassEntityManager::InitializeMassEntity()
{
    if (UWorld* World = GetWorld())
    {
        MassEntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
        if (!MassEntitySubsystem)
        {
            UE_LOG(LogTemp, Error, TEXT("Crowd_MassEntityManager: Failed to get MassEntitySubsystem"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: MassEntitySubsystem initialized"));
        }
    }
}

void ACrowd_MassEntityManager::SpawnCrowdEntities(int32 EntityCount, const FVector& CenterLocation, float SpawnRadius)
{
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("Crowd_MassEntityManager: MassEntitySubsystem not available"));
        return;
    }

    // Clamp entity count to max
    EntityCount = FMath::Min(EntityCount, MaxEntities - SpawnedEntities.Num());

    for (int32 i = 0; i < EntityCount; i++)
    {
        FCrowd_EntitySpawnData SpawnData;
        
        // Random position within spawn radius
        FVector RandomOffset = FVector(
            FMath::RandRange(-SpawnRadius, SpawnRadius),
            FMath::RandRange(-SpawnRadius, SpawnRadius),
            0.0f
        );
        SpawnData.SpawnLocation = CenterLocation + RandomOffset;
        SpawnData.SpawnRotation = FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f);
        SpawnData.BehaviorType = DefaultBehavior;
        SpawnData.MovementSpeed = FMath::RandRange(100.0f, 200.0f);
        SpawnData.DetectionRadius = FMath::RandRange(300.0f, 700.0f);

        FMassEntityHandle EntityHandle = CreateCrowdEntity(SpawnData);
        if (EntityHandle.IsValid())
        {
            SpawnedEntities.Add(SpawnData);
            ActiveEntities.Add(EntityHandle);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: Spawned %d entities, Total: %d"), 
           EntityCount, SpawnedEntities.Num());
}

void ACrowd_MassEntityManager::DespawnAllEntities()
{
    if (!MassEntitySubsystem)
    {
        return;
    }

    for (FMassEntityHandle EntityHandle : ActiveEntities)
    {
        DestroyCrowdEntity(EntityHandle);
    }

    ActiveEntities.Empty();
    SpawnedEntities.Empty();

    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: All entities despawned"));
}

int32 ACrowd_MassEntityManager::GetActiveEntityCount() const
{
    return ActiveEntities.Num();
}

void ACrowd_MassEntityManager::SetCrowdDensity(float NewDensity)
{
    CrowdDensity = FMath::Clamp(NewDensity, 0.1f, 2.0f);
    
    // Adjust entity count based on density
    int32 TargetCount = FMath::RoundToInt(MaxEntities * CrowdDensity);
    int32 CurrentCount = GetActiveEntityCount();

    if (TargetCount > CurrentCount)
    {
        SpawnCrowdEntities(TargetCount - CurrentCount, GetActorLocation(), 2000.0f);
    }
    else if (TargetCount < CurrentCount)
    {
        // Remove excess entities
        int32 ToRemove = CurrentCount - TargetCount;
        for (int32 i = 0; i < ToRemove && ActiveEntities.Num() > 0; i++)
        {
            FMassEntityHandle EntityToRemove = ActiveEntities.Last();
            DestroyCrowdEntity(EntityToRemove);
            ActiveEntities.RemoveAt(ActiveEntities.Num() - 1);
            SpawnedEntities.RemoveAt(SpawnedEntities.Num() - 1);
        }
    }
}

void ACrowd_MassEntityManager::UpdateCrowdBehavior(ECrowd_BehaviorType NewBehavior)
{
    DefaultBehavior = NewBehavior;
    
    // Update all existing entities
    for (FCrowd_EntitySpawnData& SpawnData : SpawnedEntities)
    {
        SpawnData.BehaviorType = NewBehavior;
    }
}

void ACrowd_MassEntityManager::UpdateEntityBehaviors()
{
    // Update entity behaviors based on current state
    // This would integrate with Mass Entity behavior processors
    
    if (ActiveEntities.Num() > 0)
    {
        // Simple behavior update - could be expanded with actual Mass Entity fragments
        UE_LOG(LogTemp, VeryVerbose, TEXT("Crowd_MassEntityManager: Updating %d entity behaviors"), 
               ActiveEntities.Num());
    }
}

void ACrowd_MassEntityManager::CleanupInvalidEntities()
{
    if (!MassEntitySubsystem)
    {
        return;
    }

    // Remove invalid entity handles
    for (int32 i = ActiveEntities.Num() - 1; i >= 0; i--)
    {
        if (!ActiveEntities[i].IsValid())
        {
            ActiveEntities.RemoveAt(i);
            if (i < SpawnedEntities.Num())
            {
                SpawnedEntities.RemoveAt(i);
            }
        }
    }
}

FMassEntityHandle ACrowd_MassEntityManager::CreateCrowdEntity(const FCrowd_EntitySpawnData& SpawnData)
{
    if (!MassEntitySubsystem)
    {
        return FMassEntityHandle();
    }

    // Create a basic Mass Entity
    // In a full implementation, this would use Mass Entity archetype and fragments
    FMassEntityHandle EntityHandle = MassEntitySubsystem->CreateEntity();
    
    if (EntityHandle.IsValid())
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("Crowd_MassEntityManager: Created entity at %s"), 
               *SpawnData.SpawnLocation.ToString());
    }

    return EntityHandle;
}

void ACrowd_MassEntityManager::DestroyCrowdEntity(FMassEntityHandle EntityHandle)
{
    if (!MassEntitySubsystem || !EntityHandle.IsValid())
    {
        return;
    }

    MassEntitySubsystem->DestroyEntity(EntityHandle);
}