#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "Engine/World.h"
#include "CrowdSimulationSubsystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCrowdSimulation, Log, All);

/**
 * Crowd Simulation Subsystem for managing massive dinosaur herds and ecosystem simulation
 * Handles up to 50,000 simultaneous agents using UE5 Mass AI framework
 */
UCLASS()
class TRANSPERSONALGAME_API UCrowdSimulationSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Core simulation management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void StartEcosystemSimulation();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void StopEcosystemSimulation();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void PauseEcosystemSimulation(bool bPause);

    // Herd management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnDinosaurHerd(const FVector& Location, int32 HerdSize, const FString& SpeciesType);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void DespawnHerd(int32 HerdID);

    // Player interaction
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void NotifyPlayerPresence(const FVector& PlayerLocation, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void TriggerPanicResponse(const FVector& ThreatLocation, float PanicRadius);

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 GetActiveAgentCount() const { return ActiveAgentCount; }

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    float GetCurrentPerformanceLoad() const { return CurrentPerformanceLoad; }

protected:
    // Mass Entity references
    UPROPERTY()
    class UMassEntitySubsystem* MassEntitySubsystem;

    UPROPERTY()
    class UMassSpawnerSubsystem* MassSpawnerSubsystem;

    // Simulation state
    UPROPERTY(BlueprintReadOnly, Category = "Crowd Simulation")
    bool bIsSimulationActive;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd Simulation")
    int32 ActiveAgentCount;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd Simulation")
    float CurrentPerformanceLoad;

    // Configuration
    UPROPERTY(EditDefaultsOnly, Category = "Performance")
    int32 MaxSimultaneousAgents = 50000;

    UPROPERTY(EditDefaultsOnly, Category = "Performance")
    float PerformanceThreshold = 0.85f;

    UPROPERTY(EditDefaultsOnly, Category = "Simulation")
    float EcosystemUpdateFrequency = 1.0f;

    // Internal management
    TArray<int32> ActiveHerdIDs;
    FTimerHandle EcosystemUpdateTimer;

    void UpdateEcosystemState();
    void ManagePerformance();
    void UpdateHerdBehaviors();
};