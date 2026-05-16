#pragma once

#include "CoreMinimal.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassRepresentationFragments.h"
#include "MassLODFragments.h"
#include "MassActorSubsystem.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Subsystems/WorldSubsystem.h"
#include "TranspersonalGame/SharedTypes.h"
#include "Crowd_MassEntityManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_EntityConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    int32 MaxEntities = 10000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    float SpawnRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    float MovementSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    float LODDistance1 = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    float LODDistance2 = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    float LODDistance3 = 8000.0f;

    FCrowd_EntityConfig()
    {
        MaxEntities = 10000;
        SpawnRadius = 5000.0f;
        MovementSpeed = 300.0f;
        LODDistance1 = 1000.0f;
        LODDistance2 = 3000.0f;
        LODDistance3 = 8000.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_SpawnPoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    float Radius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    int32 EntityCount = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    ECrowd_BiomeType BiomeType = ECrowd_BiomeType::Savanna;

    FCrowd_SpawnPoint()
    {
        Location = FVector::ZeroVector;
        Radius = 500.0f;
        EntityCount = 100;
        BiomeType = ECrowd_BiomeType::Savanna;
    }
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
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Mass Entity Management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void InitializeMassEntitySystem();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnCrowdEntities(const FCrowd_SpawnPoint& SpawnPoint);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnCrowdEntitiesInBiome(ECrowd_BiomeType BiomeType, int32 EntityCount);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateCrowdBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetCrowdLODDistances(float LOD1, float LOD2, float LOD3);

    // Combat Integration
    UFUNCTION(BlueprintCallable, Category = "Combat Integration")
    void NotifyPlayerCombatState(bool bInCombat, FVector CombatLocation);

    UFUNCTION(BlueprintCallable, Category = "Combat Integration")
    void ScatterCrowdFromLocation(FVector DangerLocation, float ScatterRadius);

    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetMaxEntityCount(int32 NewMaxCount);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetCurrentEntityCount() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeCrowdDensity();

    // Pathfinding Integration
    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    void UpdateCrowdNavigation();

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    void SetCrowdDestination(FVector Destination, float AcceptanceRadius);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FCrowd_EntityConfig CrowdConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TArray<FCrowd_SpawnPoint> SpawnPoints;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    class UMassEntitySubsystem* MassEntitySubsystem;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    class UMassSpawnerSubsystem* MassSpawnerSubsystem;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TArray<FMassEntityHandle> ActiveEntities;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    bool bSystemInitialized;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    float LastUpdateTime;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    int32 CurrentEntityCount;

private:
    void CreateMassArchetype();
    void SetupEntityFragments();
    void RegisterProcessors();
    void UpdateEntityLOD();
    void HandleCombatResponse(FVector CombatLocation);
    FVector GetBiomeSpawnLocation(ECrowd_BiomeType BiomeType);
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_MassSpawner : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_MassSpawner();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
    FCrowd_SpawnPoint SpawnConfiguration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
    bool bAutoSpawnOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
    float SpawnInterval;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    UCrowd_MassEntityManager* CrowdManager;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    float LastSpawnTime;

public:
    UFUNCTION(BlueprintCallable, Category = "Spawner")
    void TriggerSpawn();

    UFUNCTION(BlueprintCallable, Category = "Spawner")
    void SetSpawnConfiguration(const FCrowd_SpawnPoint& NewConfig);
};