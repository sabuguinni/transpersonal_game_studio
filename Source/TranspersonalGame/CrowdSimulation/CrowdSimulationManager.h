#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/TriggerBox.h"
#include "NavigationSystem.h"
#include "CrowdSimulationManager.generated.h"

// Forward declarations
class UCrowd_AgentComponent;
class UCrowd_SpawnPointComponent;
class ACrowd_MovementZone;

UENUM(BlueprintType)
enum class ECrowd_SimulationState : uint8
{
    Inactive        UMETA(DisplayName = "Inactive"),
    Initializing    UMETA(DisplayName = "Initializing"),
    Active          UMETA(DisplayName = "Active"),
    Paused          UMETA(DisplayName = "Paused"),
    Cleanup         UMETA(DisplayName = "Cleanup")
};

UENUM(BlueprintType)
enum class ECrowd_DensityLevel : uint8
{
    Sparse          UMETA(DisplayName = "Sparse (1-10 agents)"),
    Light           UMETA(DisplayName = "Light (11-50 agents)"),
    Medium          UMETA(DisplayName = "Medium (51-200 agents)"),
    Dense           UMETA(DisplayName = "Dense (201-1000 agents)"),
    VeryDense       UMETA(DisplayName = "Very Dense (1000+ agents)")
};

USTRUCT(BlueprintType)
struct FCrowd_SimulationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    int32 MaxAgents = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float SpawnRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float UpdateFrequency = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    bool bEnableLODSystem = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float LODDistanceHigh = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float LODDistanceMedium = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float LODDistanceLow = 3000.0f;

    FCrowd_SimulationSettings()
    {
        MaxAgents = 1000;
        SpawnRadius = 2000.0f;
        UpdateFrequency = 30.0f;
        bEnableLODSystem = true;
        LODDistanceHigh = 500.0f;
        LODDistanceMedium = 1500.0f;
        LODDistanceLow = 3000.0f;
    }
};

USTRUCT(BlueprintType)
struct FCrowd_PerformanceStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveAgents = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CrowdUpdateTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 HighLODAgents = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 MediumLODAgents = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 LowLODAgents = 0;

    FCrowd_PerformanceStats()
    {
        ActiveAgents = 0;
        FrameTime = 0.0f;
        CrowdUpdateTime = 0.0f;
        HighLODAgents = 0;
        MediumLODAgents = 0;
        LowLODAgents = 0;
    }
};

/**
 * Main crowd simulation manager for prehistoric tribal communities
 * Handles mass entity spawning, pathfinding, and LOD management
 * Supports up to 50,000 simultaneous agents using UE5 Mass Entity system
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowdSimulationManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowdSimulationManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaTime) override;

public:
    // Core simulation control
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void StartSimulation();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void StopSimulation();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void PauseSimulation();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void ResumeSimulation();

    // Agent management
    UFUNCTION(BlueprintCallable, Category = "Crowd Agents")
    void SpawnCrowdAgents(int32 Count, FVector SpawnCenter, float SpawnRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd Agents")
    void DespawnAllAgents();

    UFUNCTION(BlueprintCallable, Category = "Crowd Agents")
    void SetCrowdDensity(ECrowd_DensityLevel DensityLevel);

    // Performance and monitoring
    UFUNCTION(BlueprintCallable, Category = "Crowd Performance")
    FCrowd_PerformanceStats GetPerformanceStats() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Performance")
    void UpdateLODSystem();

    // Zone management
    UFUNCTION(BlueprintCallable, Category = "Crowd Zones")
    void RegisterMovementZone(ACrowd_MovementZone* Zone);

    UFUNCTION(BlueprintCallable, Category = "Crowd Zones")
    void UnregisterMovementZone(ACrowd_MovementZone* Zone);

    // Settings
    UFUNCTION(BlueprintCallable, Category = "Crowd Settings")
    void SetSimulationSettings(const FCrowd_SimulationSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Crowd Settings")
    FCrowd_SimulationSettings GetSimulationSettings() const { return SimulationSettings; }

    // Debug and visualization
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugSpawnTestCrowd();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugClearAllCrowds();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void SetDebugVisualization(bool bEnabled);

protected:
    // Core components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* VisualizationMesh;

    // Simulation state
    UPROPERTY(BlueprintReadOnly, Category = "Simulation State")
    ECrowd_SimulationState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation Settings")
    FCrowd_SimulationSettings SimulationSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FCrowd_PerformanceStats PerformanceStats;

    // Agent tracking
    UPROPERTY()
    TArray<UCrowd_AgentComponent*> ActiveAgents;

    UPROPERTY()
    TArray<ACrowd_MovementZone*> MovementZones;

    // Timing
    float LastUpdateTime;
    float AccumulatedDeltaTime;

private:
    void InitializeSimulation();
    void UpdateSimulation(float DeltaTime);
    void CleanupSimulation();
    void UpdatePerformanceStats();
    void ProcessAgentLOD();
    FVector GetRandomSpawnLocation(FVector Center, float Radius);
    bool IsValidSpawnLocation(FVector Location);
};