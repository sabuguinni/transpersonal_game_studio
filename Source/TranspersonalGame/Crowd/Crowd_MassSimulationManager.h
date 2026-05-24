#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "MassEntityTypes.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerTypes.h"
#include "MassCommonTypes.h"
#include "../SharedTypes.h"
#include "Crowd_MassSimulationManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_BiomeSpawnConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    EBiomeType BiomeType = EBiomeType::Savana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    FVector SpawnCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    float SpawnRadius = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    int32 MaxEntities = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    float EntityDensity = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    TArray<ECrowdEntityType> AllowedEntityTypes;

    FCrowd_BiomeSpawnConfig()
    {
        AllowedEntityTypes.Add(ECrowdEntityType::Herbivore);
        AllowedEntityTypes.Add(ECrowdEntityType::SmallCarnivore);
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_EntityArchetype
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    ECrowdEntityType EntityType = ECrowdEntityType::Herbivore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    float MovementSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    float DetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    float FlockingRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    float SeparationDistance = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    bool bCanAttack = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    float AttackDamage = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    float AttackRange = 150.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_SimulationStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Crowd Stats")
    int32 TotalActiveEntities = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd Stats")
    int32 EntitiesInSavana = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd Stats")
    int32 EntitiesInForest = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd Stats")
    int32 EntitiesInDesert = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd Stats")
    int32 EntitiesInSwamp = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd Stats")
    int32 EntitiesInMountain = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd Stats")
    float AverageFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd Stats")
    float SimulationLoad = 0.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCrowd_MassSimulationManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCrowd_MassSimulationManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Mass Entity Management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void InitializeMassSimulation();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void StartSimulation();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void StopSimulation();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void PauseSimulation();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void ResumeSimulation();

    // Biome Configuration
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void ConfigureBiomeSpawning(EBiomeType BiomeType, const FCrowd_BiomeSpawnConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnEntitiesInBiome(EBiomeType BiomeType, int32 EntityCount);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void ClearEntitiesInBiome(EBiomeType BiomeType);

    // Entity Archetypes
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void RegisterEntityArchetype(ECrowdEntityType EntityType, const FCrowd_EntityArchetype& Archetype);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    FCrowd_EntityArchetype GetEntityArchetype(ECrowdEntityType EntityType) const;

    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetMaxEntityCount(int32 MaxCount);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetSimulationQuality(float QualityLevel);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void EnableLODSystem(bool bEnabled);

    // Statistics and Monitoring
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    FCrowd_SimulationStats GetSimulationStats() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 GetEntityCountInBiome(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    float GetCurrentFrameTime() const;

    // Debug and Visualization
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void EnableDebugVisualization(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void DrawDebugInfo();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Crowd Simulation")
    void TestSpawnEntities();

protected:
    // Mass Entity System Integration
    UPROPERTY()
    class UMassEntitySubsystem* MassEntitySubsystem;

    // Configuration Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Configuration")
    TMap<EBiomeType, FCrowd_BiomeSpawnConfig> BiomeConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Configuration")
    TMap<ECrowdEntityType, FCrowd_EntityArchetype> EntityArchetypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Configuration")
    int32 MaxTotalEntities = 5000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Configuration")
    float SimulationTickRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Configuration")
    bool bUseAdaptiveLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Configuration")
    float LODDistance1 = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Configuration")
    float LODDistance2 = 15000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Configuration")
    float LODDistance3 = 30000.0f;

    // Runtime State
    UPROPERTY()
    bool bSimulationActive = false;

    UPROPERTY()
    bool bSimulationPaused = false;

    UPROPERTY()
    bool bDebugVisualization = false;

    UPROPERTY()
    FCrowd_SimulationStats CurrentStats;

    // Entity Management
    UPROPERTY()
    TArray<FMassEntityHandle> ActiveEntities;

    UPROPERTY()
    TMap<EBiomeType, TArray<FMassEntityHandle>> EntitiesByBiome;

private:
    void InitializeDefaultConfigurations();
    void SetupMassEntityProcessors();
    void UpdateSimulationStats();
    void ProcessEntityLOD();
    void HandleEntityCulling();
    
    FVector GetRandomSpawnLocationInBiome(EBiomeType BiomeType) const;
    bool IsLocationValidForSpawn(const FVector& Location, EBiomeType BiomeType) const;
    
    float LastStatsUpdateTime = 0.0f;
    const float StatsUpdateInterval = 1.0f;
};