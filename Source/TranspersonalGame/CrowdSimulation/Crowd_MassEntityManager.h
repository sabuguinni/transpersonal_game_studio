#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Crowd_SharedTypes.h"
#include "Crowd_MassEntityManager.generated.h"

class UCrowd_AgentComponent;
class ACrowd_SpawnPoint;

/**
 * Mass Entity Manager for crowd simulation
 * Handles spawning, updating, and managing large numbers of crowd agents
 * Uses UE5 Mass Entity system for performance optimization
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCrowd_MassEntityManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowd_MassEntityManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    /** Initialize the Mass Entity system */
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void InitializeMassSystem();

    /** Spawn crowd agents at specified location */
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnCrowdAgents(FVector SpawnLocation, const FCrowd_SpawnConfig& SpawnConfig);

    /** Update crowd density based on player proximity */
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateCrowdDensity(FVector PlayerLocation, float Radius);

    /** Set global crowd performance settings */
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetPerformanceSettings(const FCrowd_PerformanceSettings& NewSettings);

    /** Get current number of active agents */
    UFUNCTION(BlueprintPure, Category = "Crowd Simulation")
    int32 GetActiveAgentCount() const { return ActiveAgentCount; }

    /** Enable or disable crowd simulation */
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetCrowdSimulationEnabled(bool bEnabled);

    /** Clear all crowd agents */
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void ClearAllAgents();

    /** Update LOD levels based on distance from camera */
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateLODLevels(FVector CameraLocation);

protected:
    /** Performance settings for crowd simulation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FCrowd_PerformanceSettings PerformanceSettings;

    /** Pathfinding configuration */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    FCrowd_PathfindingConfig PathfindingConfig;

    /** Enable debug visualization */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowDebugInfo = false;

    /** Current number of active agents */
    UPROPERTY(BlueprintReadOnly, Category = "Status")
    int32 ActiveAgentCount = 0;

    /** Maximum allowed agents based on performance */
    UPROPERTY(BlueprintReadOnly, Category = "Status")
    int32 MaxAllowedAgents = 50000;

    /** Is crowd simulation currently enabled */
    UPROPERTY(BlueprintReadOnly, Category = "Status")
    bool bSimulationEnabled = true;

private:
    /** Internal agent management */
    TArray<UCrowd_AgentComponent*> ManagedAgents;
    
    /** Spawn point references */
    TArray<ACrowd_SpawnPoint*> SpawnPoints;

    /** Last camera location for LOD updates */
    FVector LastCameraLocation = FVector::ZeroVector;

    /** Time since last LOD update */
    float TimeSinceLastLODUpdate = 0.0f;

    /** LOD update frequency (seconds) */
    float LODUpdateFrequency = 0.1f;

    /** Internal methods */
    void UpdateAgentLOD(UCrowd_AgentComponent* Agent, float DistanceToCamera);
    void OptimizePerformance();
    void UpdateFlowFields();
    ECrowd_LODLevel CalculateLODLevel(float Distance) const;
};