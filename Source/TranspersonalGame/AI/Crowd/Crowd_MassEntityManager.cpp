#include "Crowd_MassEntityManager.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassSimulationSubsystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Math/UnrealMathUtility.h"

UCrowd_MassEntityManager::UCrowd_MassEntityManager()
{
    // Initialize default spawn configuration
    SpawnConfig.MaxEntitiesPerBiome = 200;
    SpawnConfig.SpawnRadius = 15000.0f;
    SpawnConfig.MinDistanceBetweenEntities = 500.0f;
    SpawnConfig.bEnableLODSystem = true;
    SpawnConfig.HighDetailDistance = 5000.0f;
    SpawnConfig.MediumDetailDistance = 15000.0f;

    MassEntitySubsystem = nullptr;
    MassSpawnerSubsystem = nullptr;
    MassSimulationSubsystem = nullptr;
}

void UCrowd_MassEntityManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: Initializing crowd simulation subsystem"));
    
    // Get Mass Entity subsystems
    if (UWorld* World = GetWorld())
    {
        MassEntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
        MassSpawnerSubsystem = World->GetSubsystem<UMassSpawnerSubsystem>();
        MassSimulationSubsystem = World->GetSubsystem<UMassSimulationSubsystem>();
        
        if (!MassEntitySubsystem)
        {
            UE_LOG(LogTemp, Error, TEXT("Crowd_MassEntityManager: Failed to get MassEntitySubsystem"));
        }
        if (!MassSpawnerSubsystem)
        {
            UE_LOG(LogTemp, Error, TEXT("Crowd_MassEntityManager: Failed to get MassSpawnerSubsystem"));
        }
        if (!MassSimulationSubsystem)
        {
            UE_LOG(LogTemp, Error, TEXT("Crowd_MassEntityManager: Failed to get MassSimulationSubsystem"));
        }
    }
    
    InitializeBiomeData();
}

void UCrowd_MassEntityManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: Deinitializing crowd simulation"));
    ClearAllCrowdEntities();
    Super::Deinitialize();
}

void UCrowd_MassEntityManager::InitializeCrowdSimulation()
{
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: Initializing crowd simulation system"));
    
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("Crowd_MassEntityManager: MassEntitySubsystem not available"));
        return;
    }
    
    // Clear existing entities
    ClearAllCrowdEntities();
    
    // Initialize biome data
    InitializeBiomeData();
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: Crowd simulation initialized with %d biomes"), BiomeEntityData.Num());
}

void UCrowd_MassEntityManager::InitializeBiomeData()
{
    BiomeEntityData.Empty();
    
    // Define biome centers based on the established coordinates
    TArray<TPair<EBiomeType, FVector>> BiomeDefinitions = {
        {EBiomeType::Savanna, FVector(0, 0, 100)},
        {EBiomeType::Swamp, FVector(-50000, -45000, 100)},
        {EBiomeType::Forest, FVector(-45000, 40000, 100)},
        {EBiomeType::Desert, FVector(55000, 0, 100)},
        {EBiomeType::Mountain, FVector(40000, 50000, 100)}
    };
    
    for (const auto& BiomeDef : BiomeDefinitions)
    {
        FCrowd_BiomeEntityData BiomeData;
        BiomeData.BiomeType = BiomeDef.Key;
        BiomeData.BiomeCenter = BiomeDef.Value;
        BiomeData.CurrentEntityCount = 0;
        
        switch (BiomeDef.Key)
        {
            case EBiomeType::Savanna:
                BiomeData.BiomeName = TEXT("Savanna");
                break;
            case EBiomeType::Swamp:
                BiomeData.BiomeName = TEXT("Swamp");
                break;
            case EBiomeType::Forest:
                BiomeData.BiomeName = TEXT("Forest");
                break;
            case EBiomeType::Desert:
                BiomeData.BiomeName = TEXT("Desert");
                break;
            case EBiomeType::Mountain:
                BiomeData.BiomeName = TEXT("Mountain");
                break;
            default:
                BiomeData.BiomeName = TEXT("Unknown");
                break;
        }
        
        BiomeEntityData.Add(BiomeData);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: Initialized %d biomes for crowd simulation"), BiomeEntityData.Num());
}

void UCrowd_MassEntityManager::SpawnCrowdEntitiesInBiome(EBiomeType BiomeType, int32 EntityCount)
{
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("Crowd_MassEntityManager: Cannot spawn entities - MassEntitySubsystem not available"));
        return;
    }
    
    // Find biome data
    FCrowd_BiomeEntityData* BiomeData = BiomeEntityData.FindByPredicate([BiomeType](const FCrowd_BiomeEntityData& Data)
    {
        return Data.BiomeType == BiomeType;
    });
    
    if (!BiomeData)
    {
        UE_LOG(LogTemp, Error, TEXT("Crowd_MassEntityManager: Biome data not found for biome type %d"), (int32)BiomeType);
        return;
    }
    
    // Clamp entity count to maximum
    int32 ActualEntityCount = FMath::Min(EntityCount, SpawnConfig.MaxEntitiesPerBiome - BiomeData->CurrentEntityCount);
    
    if (ActualEntityCount <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: Biome %s already at maximum capacity"), *BiomeData->BiomeName);
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: Spawning %d crowd entities in %s biome"), ActualEntityCount, *BiomeData->BiomeName);
    
    // Spawn entities at random locations within the biome
    for (int32 i = 0; i < ActualEntityCount; i++)
    {
        FVector SpawnLocation = GetRandomLocationInBiome(BiomeType);
        
        if (IsLocationValid(SpawnLocation, BiomeType))
        {
            // Create mass entity (simplified for now - would use proper Mass Entity configuration in full implementation)
            FMassEntityHandle EntityHandle = MassEntitySubsystem->CreateEntity();
            
            if (EntityHandle.IsValid())
            {
                BiomeData->SpawnedEntities.Add(EntityHandle);
                BiomeData->CurrentEntityCount++;
                
                UE_LOG(LogTemp, Log, TEXT("Crowd_MassEntityManager: Spawned entity at location %s in %s"), 
                       *SpawnLocation.ToString(), *BiomeData->BiomeName);
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: Successfully spawned %d entities in %s biome (Total: %d)"), 
           ActualEntityCount, *BiomeData->BiomeName, BiomeData->CurrentEntityCount);
}

void UCrowd_MassEntityManager::SpawnCrowdEntitiesInAllBiomes()
{
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: Spawning crowd entities in all biomes"));
    
    for (const FCrowd_BiomeEntityData& BiomeData : BiomeEntityData)
    {
        int32 EntitiesToSpawn = SpawnConfig.MaxEntitiesPerBiome - BiomeData.CurrentEntityCount;
        if (EntitiesToSpawn > 0)
        {
            SpawnCrowdEntitiesInBiome(BiomeData.BiomeType, EntitiesToSpawn);
        }
    }
}

void UCrowd_MassEntityManager::ClearAllCrowdEntities()
{
    if (!MassEntitySubsystem)
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: Clearing all crowd entities"));
    
    for (FCrowd_BiomeEntityData& BiomeData : BiomeEntityData)
    {
        for (const FMassEntityHandle& EntityHandle : BiomeData.SpawnedEntities)
        {
            if (EntityHandle.IsValid())
            {
                MassEntitySubsystem->DestroyEntity(EntityHandle);
            }
        }
        
        BiomeData.SpawnedEntities.Empty();
        BiomeData.CurrentEntityCount = 0;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: All crowd entities cleared"));
}

int32 UCrowd_MassEntityManager::GetTotalEntityCount() const
{
    int32 TotalCount = 0;
    for (const FCrowd_BiomeEntityData& BiomeData : BiomeEntityData)
    {
        TotalCount += BiomeData.CurrentEntityCount;
    }
    return TotalCount;
}

int32 UCrowd_MassEntityManager::GetEntityCountInBiome(EBiomeType BiomeType) const
{
    const FCrowd_BiomeEntityData* BiomeData = BiomeEntityData.FindByPredicate([BiomeType](const FCrowd_BiomeEntityData& Data)
    {
        return Data.BiomeType == BiomeType;
    });
    
    return BiomeData ? BiomeData->CurrentEntityCount : 0;
}

void UCrowd_MassEntityManager::UpdateEntityLOD(const FVector& PlayerLocation)
{
    if (!SpawnConfig.bEnableLODSystem)
    {
        return;
    }
    
    // LOD system implementation would go here
    // This is a simplified version that logs the update
    UE_LOG(LogTemp, VeryVerbose, TEXT("Crowd_MassEntityManager: Updating entity LOD based on player location %s"), *PlayerLocation.ToString());
}

void UCrowd_MassEntityManager::SetEntitySpawnConfig(const FCrowd_EntitySpawnConfig& NewConfig)
{
    SpawnConfig = NewConfig;
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: Updated spawn configuration - Max entities per biome: %d"), SpawnConfig.MaxEntitiesPerBiome);
}

FVector UCrowd_MassEntityManager::GetBiomeCenter(EBiomeType BiomeType) const
{
    const FCrowd_BiomeEntityData* BiomeData = BiomeEntityData.FindByPredicate([BiomeType](const FCrowd_BiomeEntityData& Data)
    {
        return Data.BiomeType == BiomeType;
    });
    
    return BiomeData ? BiomeData->BiomeCenter : FVector::ZeroVector;
}

FVector UCrowd_MassEntityManager::GetRandomLocationInBiome(EBiomeType BiomeType) const
{
    FVector BiomeCenter = GetBiomeCenter(BiomeType);
    
    // Generate random location within spawn radius
    float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
    float RandomDistance = FMath::RandRange(0.0f, SpawnConfig.SpawnRadius);
    
    FVector RandomOffset = FVector(
        FMath::Cos(RandomAngle) * RandomDistance,
        FMath::Sin(RandomAngle) * RandomDistance,
        0.0f
    );
    
    return BiomeCenter + RandomOffset;
}

bool UCrowd_MassEntityManager::IsLocationValid(const FVector& Location, EBiomeType BiomeType) const
{
    // Basic validation - check if location is within biome bounds
    FVector BiomeCenter = GetBiomeCenter(BiomeType);
    float DistanceFromCenter = FVector::Dist2D(Location, BiomeCenter);
    
    if (DistanceFromCenter > SpawnConfig.SpawnRadius)
    {
        return false;
    }
    
    // Check minimum distance from other entities (simplified check)
    const FCrowd_BiomeEntityData* BiomeData = BiomeEntityData.FindByPredicate([BiomeType](const FCrowd_BiomeEntityData& Data)
    {
        return Data.BiomeType == BiomeType;
    });
    
    if (BiomeData && BiomeData->SpawnedEntities.Num() > 0)
    {
        // In a full implementation, this would check actual entity positions
        // For now, we'll assume the location is valid if it's within bounds
    }
    
    return true;
}

void UCrowd_MassEntityManager::CleanupInvalidEntities()
{
    if (!MassEntitySubsystem)
    {
        return;
    }
    
    for (FCrowd_BiomeEntityData& BiomeData : BiomeEntityData)
    {
        BiomeData.SpawnedEntities.RemoveAll([this](const FMassEntityHandle& EntityHandle)
        {
            return !EntityHandle.IsValid() || !MassEntitySubsystem->IsEntityValid(EntityHandle);
        });
        
        BiomeData.CurrentEntityCount = BiomeData.SpawnedEntities.Num();
    }
}