#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "TranspersonalGame/SharedTypes.h"
#include "Crowd_FlockingBehavior.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_FlockingParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float SeparationRadius = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float AlignmentRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float CohesionRadius = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float SeparationWeight = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float AlignmentWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float CohesionWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float MaxSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float MaxForce = 300.0f;

    FCrowd_FlockingParams()
    {
        SeparationRadius = 150.0f;
        AlignmentRadius = 300.0f;
        CohesionRadius = 400.0f;
        SeparationWeight = 2.0f;
        AlignmentWeight = 1.0f;
        CohesionWeight = 1.0f;
        MaxSpeed = 600.0f;
        MaxForce = 300.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_FlockingAgent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FVector Position = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FVector Acceleration = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    float Mass = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    bool bIsActive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    int32 FlockID = 0;

    FCrowd_FlockingAgent()
    {
        Position = FVector::ZeroVector;
        Velocity = FVector::ZeroVector;
        Acceleration = FVector::ZeroVector;
        Mass = 1.0f;
        bIsActive = true;
        FlockID = 0;
    }
};

/**
 * Advanced flocking behavior system for large-scale crowd simulation
 * Implements Reynolds' boids algorithm with performance optimizations
 * Supports up to 10,000 agents with spatial partitioning
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCrowd_FlockingBehavior : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowd_FlockingBehavior();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core flocking system
    UFUNCTION(BlueprintCallable, Category = "Flocking")
    void InitializeFlockingSystem(int32 MaxAgents = 5000);

    UFUNCTION(BlueprintCallable, Category = "Flocking")
    void AddFlockingAgent(const FVector& Position, const FVector& InitialVelocity, int32 FlockID = 0);

    UFUNCTION(BlueprintCallable, Category = "Flocking")
    void RemoveFlockingAgent(int32 AgentIndex);

    UFUNCTION(BlueprintCallable, Category = "Flocking")
    void UpdateFlockingBehavior(float DeltaTime);

    // Flocking forces calculation
    UFUNCTION(BlueprintCallable, Category = "Flocking")
    FVector CalculateSeparation(int32 AgentIndex, const TArray<int32>& Neighbors);

    UFUNCTION(BlueprintCallable, Category = "Flocking")
    FVector CalculateAlignment(int32 AgentIndex, const TArray<int32>& Neighbors);

    UFUNCTION(BlueprintCallable, Category = "Flocking")
    FVector CalculateCohesion(int32 AgentIndex, const TArray<int32>& Neighbors);

    // Spatial optimization
    UFUNCTION(BlueprintCallable, Category = "Flocking")
    TArray<int32> GetNeighbors(int32 AgentIndex, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Flocking")
    void UpdateSpatialGrid();

    // Obstacle avoidance
    UFUNCTION(BlueprintCallable, Category = "Flocking")
    FVector CalculateObstacleAvoidance(int32 AgentIndex);

    UFUNCTION(BlueprintCallable, Category = "Flocking")
    void AddObstacle(const FVector& Position, float Radius);

    // Performance management
    UFUNCTION(BlueprintCallable, Category = "Flocking")
    void SetLODLevel(int32 Level);

    UFUNCTION(BlueprintCallable, Category = "Flocking")
    void EnablePerformanceMode(bool bEnabled);

    // Debug and visualization
    UFUNCTION(BlueprintCallable, Category = "Flocking")
    void DrawFlockingDebug(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Flocking")
    int32 GetActiveAgentCount() const;

    UFUNCTION(BlueprintCallable, Category = "Flocking")
    float GetAverageSpeed() const;

protected:
    // Flocking parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking Parameters")
    FCrowd_FlockingParams FlockingParams;

    // Agent storage
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Agents")
    TArray<FCrowd_FlockingAgent> FlockingAgents;

    // Spatial partitioning
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float GridCellSize = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    int32 GridWidth = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    int32 GridHeight = 100;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxAgentsPerFrame = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateFrequency = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUsePerformanceMode = false;

    // LOD system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    int32 CurrentLODLevel = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    TArray<float> LODDistances;

    // Obstacles
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Obstacles")
    TArray<FVector> ObstaclePositions;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Obstacles")
    TArray<float> ObstacleRadii;

    // Debug
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bDrawDebug = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowVelocityVectors = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowNeighborConnections = false;

private:
    // Internal state
    TMap<int32, TArray<int32>> SpatialGrid;
    float LastUpdateTime = 0.0f;
    int32 CurrentUpdateIndex = 0;

    // Helper functions
    FVector LimitVector(const FVector& Vector, float MaxMagnitude);
    int32 GetGridIndex(const FVector& Position);
    void UpdateAgentPosition(int32 AgentIndex, float DeltaTime);
    bool IsValidAgentIndex(int32 Index) const;
};