#pragma once

#include "CoreMinimal.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassSimulationSubsystem.h"
#include "MassEntityConfigAsset.h"
#include "MassEntityTemplate.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "Crowd_MassEntityManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_EntitySpawnConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawn")
    int32 MaxEntitiesPerBiome = 200;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawn")
    float SpawnRadius = 15000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawn")
    float MinDistanceBetweenEntities = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawn")
    bool bEnableLODSystem = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawn")
    float HighDetailDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawn")
    float MediumDetailDistance = 15000.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_BiomeEntityData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Data")
    FVector BiomeCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Data")
    FString BiomeName = TEXT("Unknown");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Data")
    int32 CurrentEntityCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Data")
    TArray<FMassEntityHandle> SpawnedEntities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Data")
    EBiomeType BiomeType = EBiomeType::Savanna;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCrowd_MassEntityManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCrowd_MassEntityManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void InitializeCrowdSimulation();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnCrowdEntitiesInBiome(EBiomeType BiomeType, int32 EntityCount);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnCrowdEntitiesInAllBiomes();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void ClearAllCrowdEntities();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 GetTotalEntityCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 GetEntityCountInBiome(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateEntityLOD(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetEntitySpawnConfig(const FCrowd_EntitySpawnConfig& NewConfig);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    FCrowd_EntitySpawnConfig GetEntitySpawnConfig() const { return SpawnConfig; }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FCrowd_EntitySpawnConfig SpawnConfig;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Data")
    TArray<FCrowd_BiomeEntityData> BiomeEntityData;

    UPROPERTY(BlueprintReadOnly, Category = "Mass Entity")
    UMassEntitySubsystem* MassEntitySubsystem;

    UPROPERTY(BlueprintReadOnly, Category = "Mass Entity")
    UMassSpawnerSubsystem* MassSpawnerSubsystem;

    UPROPERTY(BlueprintReadOnly, Category = "Mass Entity")
    UMassSimulationSubsystem* MassSimulationSubsystem;

private:
    void InitializeBiomeData();
    FVector GetBiomeCenter(EBiomeType BiomeType) const;
    FVector GetRandomLocationInBiome(EBiomeType BiomeType) const;
    bool IsLocationValid(const FVector& Location, EBiomeType BiomeType) const;
    void CleanupInvalidEntities();
};