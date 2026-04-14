#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/World.h"
#include "MassEntityTypes.h"
#include "MassProcessor.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "../SharedTypes.h"
#include "CrowdSimulationSubsystem.generated.h"

class UMassEntitySubsystem;
class UMassSpawnerSubsystem;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_EntityConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    int32 MaxEntities = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    float SpawnRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    float MovementSpeed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    float AvoidanceRadius = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    bool bEnableLOD = true;

    FCrowd_EntityConfig()
    {
        MaxEntities = 1000;
        SpawnRadius = 5000.0f;
        MovementSpeed = 150.0f;
        AvoidanceRadius = 100.0f;
        bEnableLOD = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_SpawnZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Zone")
    FVector Center = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Zone")
    FVector Extents = FVector(1000.0f, 1000.0f, 100.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Zone")
    int32 TargetPopulation = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Zone")
    ECrowd_BehaviorType BehaviorType = ECrowd_BehaviorType::Wandering;

    FCrowd_SpawnZone()
    {
        Center = FVector::ZeroVector;
        Extents = FVector(1000.0f, 1000.0f, 100.0f);
        TargetPopulation = 100;
        BehaviorType = ECrowd_BehaviorType::Wandering;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCrowdSimulationSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCrowdSimulationSubsystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Crowd Management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void InitializeCrowdSystem();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnCrowdEntities(const FCrowd_SpawnZone& SpawnZone);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void DespawnAllCrowdEntities();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetCrowdDensity(float DensityMultiplier);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 GetActiveCrowdEntityCount() const;

    // LOD Management
    UFUNCTION(BlueprintCallable, Category = "Crowd LOD")
    void UpdateCrowdLOD(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd LOD")
    void SetLODDistances(float HighDetailDistance, float MediumDetailDistance, float LowDetailDistance);

    // Behavior Control
    UFUNCTION(BlueprintCallable, Category = "Crowd Behavior")
    void SetGlobalCrowdBehavior(ECrowd_BehaviorType NewBehavior);

    UFUNCTION(BlueprintCallable, Category = "Crowd Behavior")
    void TriggerCrowdEvent(ECrowd_EventType EventType, const FVector& EventLocation, float EventRadius);

    // Debug and Visualization
    UFUNCTION(BlueprintCallable, Category = "Crowd Debug", CallInEditor)
    void DebugDrawCrowdInfo();

    UFUNCTION(BlueprintCallable, Category = "Crowd Debug")
    void ToggleCrowdDebugVisualization();

protected:
    // Core configuration
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crowd Config", meta = (AllowPrivateAccess = "true"))
    FCrowd_EntityConfig EntityConfig;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crowd Config", meta = (AllowPrivateAccess = "true"))
    TArray<FCrowd_SpawnZone> SpawnZones;

    // LOD settings
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crowd LOD", meta = (AllowPrivateAccess = "true"))
    float HighDetailDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crowd LOD", meta = (AllowPrivateAccess = "true"))
    float MediumDetailDistance = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crowd LOD", meta = (AllowPrivateAccess = "true"))
    float LowDetailDistance = 5000.0f;

    // Runtime state
    UPROPERTY(BlueprintReadOnly, Category = "Crowd State", meta = (AllowPrivateAccess = "true"))
    int32 ActiveEntityCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd State", meta = (AllowPrivateAccess = "true"))
    bool bCrowdSystemInitialized = false;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd State", meta = (AllowPrivateAccess = "true"))
    bool bDebugVisualizationEnabled = false;

private:
    // Mass Entity references
    UMassEntitySubsystem* MassEntitySubsystem = nullptr;
    UMassSpawnerSubsystem* MassSpawnerSubsystem = nullptr;

    // Internal methods
    void SetupMassEntityReferences();
    void CreateCrowdArchetype();
    void RegisterCrowdProcessors();
    void UpdateEntityLOD(const FVector& PlayerLocation);
    void HandleCrowdEvent(ECrowd_EventType EventType, const FVector& Location, float Radius);
};