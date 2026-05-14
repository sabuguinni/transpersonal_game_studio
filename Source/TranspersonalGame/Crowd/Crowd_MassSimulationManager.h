#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "AI/NavigationSystemBase.h"
#include "Engine/StaticMeshActor.h"
#include "../SharedTypes.h"
#include "Crowd_MassSimulationManager.generated.h"

// Forward declarations
class ACrowd_CrowdAgent;
class UCrowd_FlockingBehavior;

/**
 * Crowd Agent Data for mass simulation
 * Contains minimal data needed for each agent in the crowd
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_AgentData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FVector Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    float MaxSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    float SeparationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    int32 GroupID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    ECrowd_BehaviorState CurrentState;

    FCrowd_AgentData()
    {
        Position = FVector::ZeroVector;
        Velocity = FVector::ZeroVector;
        TargetLocation = FVector::ZeroVector;
        MaxSpeed = 300.0f;
        SeparationRadius = 100.0f;
        GroupID = 0;
        bIsActive = true;
        CurrentState = ECrowd_BehaviorState::Wandering;
    }
};

/**
 * Mass Simulation Configuration
 * Settings for large-scale crowd simulation performance
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_MassSimulationConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "100", ClampMax = "50000"))
    int32 MaxAgentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "10", ClampMax = "1000"))
    int32 AgentsPerBatch;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "0.016", ClampMax = "0.1"))
    float UpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD", meta = (ClampMin = "500", ClampMax = "10000"))
    float HighDetailDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD", meta = (ClampMin = "1000", ClampMax = "20000"))
    float MediumDetailDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD", meta = (ClampMin = "2000", ClampMax = "50000"))
    float LowDetailDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial")
    float SpatialGridSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial")
    int32 MaxAgentsPerCell;

    FCrowd_MassSimulationConfig()
    {
        MaxAgentCount = 10000;
        AgentsPerBatch = 100;
        UpdateInterval = 0.033f; // 30 FPS
        HighDetailDistance = 1000.0f;
        MediumDetailDistance = 3000.0f;
        LowDetailDistance = 8000.0f;
        SpatialGridSize = 500.0f;
        MaxAgentsPerCell = 50;
    }
};

/**
 * Spatial Grid Cell for efficient neighbor queries
 */
USTRUCT()
struct TRANSPERSONALGAME_API FCrowd_SpatialCell
{
    GENERATED_BODY()

    UPROPERTY()
    TArray<int32> AgentIndices;

    UPROPERTY()
    FVector CellCenter;

    UPROPERTY()
    bool bIsDirty;

    FCrowd_SpatialCell()
    {
        CellCenter = FVector::ZeroVector;
        bIsDirty = true;
    }
};

/**
 * Mass Simulation Manager
 * Handles up to 50,000 crowd agents using optimized batch processing
 * Implements spatial partitioning, LOD system, and performance management
 */
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
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Simulation")
    FCrowd_MassSimulationConfig SimulationConfig;

    // Agent Management
    UPROPERTY(BlueprintReadOnly, Category = "Agents")
    TArray<FCrowd_AgentData> AgentPool;

    UPROPERTY(BlueprintReadOnly, Category = "Agents")
    int32 ActiveAgentCount;

    // Spatial Partitioning
    UPROPERTY()
    TMap<FIntVector, FCrowd_SpatialCell> SpatialGrid;

    UPROPERTY()
    FVector WorldBounds;

    // Performance Tracking
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float LastUpdateTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 AgentsProcessedThisFrame;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFrameTime;

    // Batch Processing
    UPROPERTY()
    int32 CurrentBatchIndex;

    UPROPERTY()
    float BatchTimer;

    // LOD Management
    UPROPERTY()
    TArray<int32> HighDetailAgents;

    UPROPERTY()
    TArray<int32> MediumDetailAgents;

    UPROPERTY()
    TArray<int32> LowDetailAgents;

    UPROPERTY()
    TArray<int32> CulledAgents;

    // Player reference for LOD calculations
    UPROPERTY()
    APawn* PlayerPawn;

public:
    // Agent Management Functions
    UFUNCTION(BlueprintCallable, Category = "Mass Simulation")
    int32 SpawnAgent(const FVector& Location, int32 GroupID = 0);

    UFUNCTION(BlueprintCallable, Category = "Mass Simulation")
    void RemoveAgent(int32 AgentIndex);

    UFUNCTION(BlueprintCallable, Category = "Mass Simulation")
    void SetAgentTarget(int32 AgentIndex, const FVector& TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Mass Simulation")
    void SetAgentState(int32 AgentIndex, ECrowd_BehaviorState NewState);

    // Group Management
    UFUNCTION(BlueprintCallable, Category = "Mass Simulation")
    void SpawnAgentGroup(const FVector& CenterLocation, int32 GroupSize, float SpawnRadius, int32 GroupID);

    UFUNCTION(BlueprintCallable, Category = "Mass Simulation")
    void SetGroupTarget(int32 GroupID, const FVector& TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Mass Simulation")
    void SetGroupBehavior(int32 GroupID, ECrowd_BehaviorState NewState);

    // Spatial Partitioning
    UFUNCTION(BlueprintCallable, Category = "Spatial")
    void UpdateSpatialGrid();

    UFUNCTION(BlueprintCallable, Category = "Spatial")
    TArray<int32> GetNearbyAgents(const FVector& Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Spatial")
    FIntVector GetGridCoordinate(const FVector& WorldLocation);

    // LOD Management
    UFUNCTION(BlueprintCallable, Category = "LOD")
    void UpdateLODLevels();

    UFUNCTION(BlueprintCallable, Category = "LOD")
    ECrowd_LODLevel GetAgentLODLevel(int32 AgentIndex);

    // Batch Processing
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ProcessAgentBatch(int32 StartIndex, int32 BatchSize, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateAgentMovement(int32 AgentIndex, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FVector CalculateSteeringForce(int32 AgentIndex);

    // Flocking Behaviors
    UFUNCTION(BlueprintCallable, Category = "Flocking")
    FVector CalculateSeparation(int32 AgentIndex, const TArray<int32>& NearbyAgents);

    UFUNCTION(BlueprintCallable, Category = "Flocking")
    FVector CalculateAlignment(int32 AgentIndex, const TArray<int32>& NearbyAgents);

    UFUNCTION(BlueprintCallable, Category = "Flocking")
    FVector CalculateCohesion(int32 AgentIndex, const TArray<int32>& NearbyAgents);

    UFUNCTION(BlueprintCallable, Category = "Flocking")
    FVector CalculateObstacleAvoidance(int32 AgentIndex);

    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizePerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void AdjustSimulationQuality(float TargetFrameTime);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentPerformanceMetric();

    // Debug and Visualization
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DrawDebugInfo();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void ToggleDebugVisualization();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowDebugInfo;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowSpatialGrid;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowAgentVelocities;

    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAgentSpawned, int32, AgentIndex, FVector, Location);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAgentRemoved, int32, AgentIndex);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPerformanceWarning, float, CurrentFrameTime, int32, ActiveAgents);

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnAgentSpawned OnAgentSpawned;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnAgentRemoved OnAgentRemoved;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPerformanceWarning OnPerformanceWarning;

private:
    // Internal helper functions
    void InitializeSpatialGrid();
    void UpdatePerformanceMetrics(float DeltaTime);
    bool IsAgentValid(int32 AgentIndex) const;
    void CleanupInvalidAgents();
    FVector GetRandomLocationInRadius(const FVector& Center, float Radius);
};