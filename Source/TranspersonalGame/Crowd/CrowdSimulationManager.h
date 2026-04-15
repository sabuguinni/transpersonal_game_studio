#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "../SharedTypes.h"
#include "CrowdSimulationManager.generated.h"

class UCrowd_AgentComponent;
class UCrowd_BehaviorZoneComponent;
class UCrowd_PathfindingComponent;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_AgentData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Agent")
    FVector Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Agent")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Agent")
    ECrowd_BehaviorState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Agent")
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Agent")
    int32 AgentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Agent")
    ECrowd_TribalRole TribalRole;

    FCrowd_AgentData()
    {
        Position = FVector::ZeroVector;
        Velocity = FVector::ZeroVector;
        CurrentState = ECrowd_BehaviorState::Idle;
        MovementSpeed = 150.0f;
        AgentID = -1;
        TribalRole = ECrowd_TribalRole::Gatherer;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_PerformanceStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveAgents;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 VisibleAgents;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 PathfindingQueries;

    FCrowd_PerformanceStats()
    {
        ActiveAgents = 0;
        VisibleAgents = 0;
        AverageFrameTime = 0.0f;
        PathfindingQueries = 0;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowdSimulationManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowdSimulationManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core crowd simulation functions
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void InitializeCrowdSimulation();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnCrowdAgents(int32 AgentCount, FVector SpawnCenter, float SpawnRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateCrowdBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetCrowdDensity(float NewDensity);

    // Mass Entity integration
    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void InitializeMassEntitySystem();

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void RegisterMassEntityFragments();

    // Performance optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeCrowdLOD();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FCrowd_PerformanceStats GetPerformanceStats() const;

    // Behavior zones
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void CreateBehaviorZone(FVector Location, float Radius, ECrowd_BehaviorState ZoneState);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void UpdateAgentBehaviors();

    // Pathfinding
    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    void UpdatePathfinding();

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    TArray<FVector> GetPathToTarget(FVector StartLocation, FVector TargetLocation);

protected:
    // Core components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    // Crowd simulation data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    int32 MaxCrowdAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float CrowdDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float SimulationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    bool bUseMassEntitySystem;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxVisibleAgents;

    // Runtime data
    UPROPERTY()
    TArray<FCrowd_AgentData> CrowdAgents;

    UPROPERTY()
    TArray<AActor*> BehaviorZones;

    UPROPERTY()
    TArray<FVector> PathfindingWaypoints;

    UPROPERTY()
    FCrowd_PerformanceStats CurrentStats;

private:
    // Internal simulation functions
    void UpdateAgentMovement(FCrowd_AgentData& Agent, float DeltaTime);
    void UpdateAgentState(FCrowd_AgentData& Agent);
    void HandleAgentCollisions();
    void CullDistantAgents();
    void UpdatePerformanceStats();

    // Timers
    FTimerHandle BehaviorUpdateTimer;
    FTimerHandle PerformanceUpdateTimer;
    FTimerHandle PathfindingUpdateTimer;

    // Performance tracking
    float LastFrameTime;
    int32 FrameCounter;
    float AccumulatedFrameTime;
};