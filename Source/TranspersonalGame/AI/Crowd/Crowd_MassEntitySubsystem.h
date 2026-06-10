#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "Crowd_MassEntitySubsystem.generated.h"

class UCrowd_MassProcessor;
class UCrowd_BehaviorTree;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_EntitySpawnData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    ECrowd_BehaviorType BehaviorType = ECrowd_BehaviorType::Wandering;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    float MovementSpeed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    int32 GroupID = 0;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_SimulationStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    int32 ActiveEntities = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    int32 MaxEntities = 10000;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    float AverageFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    int32 EntitiesInHighDensityZones = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    int32 EntitiesInLowDensityZones = 0;
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCrowd_MassEntitySubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Mass Entity Management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnCrowdEntities(const TArray<FCrowd_EntitySpawnData>& SpawnData);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void DespawnAllEntities();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetMaxEntityCount(int32 NewMaxCount);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    FCrowd_SimulationStats GetSimulationStats() const;

    // Behavior Management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetGlobalBehaviorState(ECrowd_BehaviorType NewBehavior);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void AddBehaviorZone(const FVector& Center, float Radius, ECrowd_BehaviorType ZoneBehavior);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void RemoveBehaviorZone(const FVector& Center);

    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetLODDistances(float HighLOD, float MediumLOD, float LowLOD);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void EnableDynamicLOD(bool bEnable);

    // Debug and Testing
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugSpawnTestCrowd();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugClearAllEntities();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugPrintStats();

protected:
    // Core system references
    UPROPERTY()
    TObjectPtr<UCrowd_MassProcessor> MassProcessor;

    UPROPERTY()
    TObjectPtr<UCrowd_BehaviorTree> BehaviorTree;

    // Simulation parameters
    UPROPERTY(EditAnywhere, Category = "Simulation")
    int32 MaxEntityCount = 10000;

    UPROPERTY(EditAnywhere, Category = "Simulation")
    float TickRate = 30.0f;

    UPROPERTY(EditAnywhere, Category = "Simulation")
    bool bEnableLODSystem = true;

    // LOD distances
    UPROPERTY(EditAnywhere, Category = "Performance")
    float HighLODDistance = 1000.0f;

    UPROPERTY(EditAnywhere, Category = "Performance")
    float MediumLODDistance = 2500.0f;

    UPROPERTY(EditAnywhere, Category = "Performance")
    float LowLODDistance = 5000.0f;

    // Runtime data
    UPROPERTY()
    TArray<FCrowd_EntitySpawnData> ActiveEntities;

    UPROPERTY()
    FCrowd_SimulationStats CurrentStats;

    // Behavior zones
    UPROPERTY()
    TMap<FVector, ECrowd_BehaviorType> BehaviorZones;

    // Internal methods
    void InitializeMassEntity();
    void UpdateSimulationStats();
    void ProcessLODSystem();
    void HandleBehaviorZones();
};