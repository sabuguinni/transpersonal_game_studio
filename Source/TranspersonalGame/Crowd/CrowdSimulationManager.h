#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassSimulationSubsystem.h"
#include "CrowdSimulationManager.generated.h"

/**
 * Central manager for crowd simulation in the prehistoric world
 * Handles herds, flocks, and pack behaviors using Mass AI
 */
UCLASS()
class TRANSPERSONALGAME_API ACrowdSimulationManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowdSimulationManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime);

    // Core simulation parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    int32 MaxSimultaneousEntities = 50000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    float SimulationRadius = 10000.0f; // 10km radius around player

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    float LODUpdateFrequency = 0.5f; // Update LOD twice per second

    // Herd behavior settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Behavior")
    float HerdCohesionRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Behavior")
    float HerdSeparationRadius = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Behavior")
    float HerdAlignmentRadius = 300.0f;

    // Migration patterns
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    TArray<FVector> MigrationWaypoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    float MigrationSpeed = 200.0f; // cm/s

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    float SeasonalCycleLength = 1800.0f; // 30 minutes = 1 season

private:
    // Mass Entity components
    UPROPERTY()
    class UMassEntitySubsystem* MassEntitySubsystem;

    UPROPERTY()
    class UMassSpawnerSubsystem* MassSpawnerSubsystem;

    UPROPERTY()
    class UMassSimulationSubsystem* MassSimulationSubsystem;

    // Current simulation state
    float CurrentSeasonTime;
    int32 ActiveEntityCount;
    
    // Performance monitoring
    float LastPerformanceCheck;
    float AverageFrameTime;

public:
    // Public interface for other systems
// [UHT-FIX]     UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnHerd(FVector Location, int32 HerdSize, TSubclassOf<class ADinosaur> DinosaurClass);

// [UHT-FIX]     UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnFlock(FVector Location, int32 FlockSize, TSubclassOf<class AFlyingDinosaur> FlyingDinosaurClass);

// [UHT-FIX]     UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnPack(FVector Location, int32 PackSize, TSubclassOf<class APredatorDinosaur> PredatorClass);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void TriggerStampede(FVector ThreatLocation, float ThreatRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetMigrationActive(bool bActive);

    UFUNCTION(BlueprintPure, Category = "Crowd Simulation")
    int32 GetActiveEntityCount() const { return ActiveEntityCount; }

    UFUNCTION(BlueprintPure, Category = "Crowd Simulation")
    float GetCurrentPerformanceMetric() const { return AverageFrameTime; }
};