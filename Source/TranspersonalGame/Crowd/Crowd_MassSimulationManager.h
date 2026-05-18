#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassSimulationSubsystem.h"
#include "MassEntityTypes.h"
#include "MassEntityTemplate.h"
#include "MassProcessor.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassRepresentationFragments.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "TranspersonalGame/TranspersonalGame.h"
#include "Crowd_MassSimulationManager.generated.h"

// Mass Entity fragments for crowd simulation
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_MovementFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MovementSpeed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float WanderRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float FlockingStrength = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float SeparationDistance = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsWandering = true;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_BehaviorFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float AggressionLevel = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float FearLevel = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float CuriosityLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float SocialRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    int32 GroupID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bIsGroupLeader = false;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_SpeciesFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    FString SpeciesName = TEXT("Generic");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    float BodySize = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    bool bIsCarnivore = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    bool bIsHerbivore = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    float CurrentHealth = 100.0f;
};

// Mass Entity processor for crowd movement
UCLASS()
class TRANSPERSONALGAME_API UCrowd_MovementProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UCrowd_MovementProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery EntityQuery;
};

// Mass Entity processor for crowd behavior
UCLASS()
class TRANSPERSONALGAME_API UCrowd_BehaviorProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UCrowd_BehaviorProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery EntityQuery;
    float LastBehaviorUpdate = 0.0f;
};

// Mass Entity spawning configuration
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_SpawnConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    FVector SpawnCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    float SpawnRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    int32 MaxEntities = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    FString BiomeName = TEXT("Savana");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    TArray<FString> SpeciesTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    float LODDistance1 = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    float LODDistance2 = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    float LODDistance3 = 10000.0f;
};

// Main crowd simulation manager
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_MassSimulationManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_MassSimulationManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    FCrowd_SpawnConfig SavanaConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    FCrowd_SpawnConfig PantanoConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    FCrowd_SpawnConfig FlorestaConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    FCrowd_SpawnConfig DesertoConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    FCrowd_SpawnConfig MontanhaConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableLODSystem = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateFrequency = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxSimulationEntities = 50000;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    int32 CurrentEntityCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    float LastUpdateTime = 0.0f;

    // Core simulation methods
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void InitializeCrowdSimulation();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnCrowdInBiome(const FCrowd_SpawnConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateCrowdLOD(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetCrowdDensity(const FString& BiomeName, float DensityMultiplier);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeCrowdPerformance();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugCrowdStats();

private:
    UPROPERTY()
    class UMassEntitySubsystem* MassEntitySubsystem;

    UPROPERTY()
    class UMassSpawnerSubsystem* MassSpawnerSubsystem;

    UPROPERTY()
    class UMassSimulationSubsystem* MassSimulationSubsystem;

    TArray<FMassEntityHandle> SpawnedEntities;
    TMap<FString, int32> BiomeEntityCounts;
    
    void SetupBiomeConfigs();
    void CreateEntityTemplate(const FString& SpeciesName);
    void ProcessCrowdMovement(float DeltaTime);
    void ProcessCrowdBehavior(float DeltaTime);
    void UpdatePerformanceMetrics();
};