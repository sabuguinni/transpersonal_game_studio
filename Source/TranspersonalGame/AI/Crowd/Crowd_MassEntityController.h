#pragma once

#include "CoreMinimal.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassMovementFragments.h"
#include "MassCommonFragments.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Crowd_MassEntityController.generated.h"

UENUM(BlueprintType)
enum class ECrowd_BehaviorState : uint8
{
    Idle,
    Gathering,
    Hunting,
    Trading,
    Fleeing,
    Following,
    Patrolling
};

USTRUCT(BlueprintType)
struct FCrowd_AgentData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECrowd_BehaviorState CurrentBehavior = ECrowd_BehaviorState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MovementSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DetectionRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 GroupID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StressLevel = 0.0f;

    FCrowd_AgentData()
    {
        CurrentBehavior = ECrowd_BehaviorState::Idle;
        MovementSpeed = 300.0f;
        DetectionRadius = 500.0f;
        GroupID = 0;
        TargetLocation = FVector::ZeroVector;
        StressLevel = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FCrowd_ZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector ZoneCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ZoneRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxAgents = 10000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECrowd_BehaviorState DefaultBehavior = ECrowd_BehaviorState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> WaypointNetwork;

    FCrowd_ZoneConfig()
    {
        ZoneCenter = FVector::ZeroVector;
        ZoneRadius = 1000.0f;
        MaxAgents = 10000;
        DefaultBehavior = ECrowd_BehaviorState::Idle;
    }
};

UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCrowd_MassEntityController : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowd_MassEntityController();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Mass Entity Management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void InitializeMassEntitySystem();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnCrowdAgents(int32 AgentCount, const FCrowd_ZoneConfig& ZoneConfig);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetCrowdBehavior(int32 GroupID, ECrowd_BehaviorState NewBehavior);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateCrowdDensity(float DensityMultiplier);

    // LOD System
    UFUNCTION(BlueprintCallable, Category = "Crowd LOD")
    void ConfigureLODSystem(float HighDetailDistance, float MediumDetailDistance, float LowDetailDistance);

    UFUNCTION(BlueprintCallable, Category = "Crowd LOD")
    void UpdateAgentLOD(const FVector& ViewerLocation);

    // Pathfinding
    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    void SetupFlowField(const TArray<FVector>& WaypointNetwork);

    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    void UpdatePathfinding(float DeltaTime);

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Crowd Performance")
    int32 GetActiveAgentCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Performance")
    float GetCrowdPerformanceMetric() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Performance")
    void OptimizeCrowdPerformance();

protected:
    // Mass Entity System
    UPROPERTY()
    class UMassEntitySubsystem* MassEntitySubsystem;

    UPROPERTY()
    class UMassSpawnerSubsystem* MassSpawnerSubsystem;

    // Crowd Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    TArray<FCrowd_ZoneConfig> CrowdZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    int32 MaxTotalAgents = 50000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    float UpdateFrequency = 30.0f;

    // LOD Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd LOD")
    float HighDetailDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd LOD")
    float MediumDetailDistance = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd LOD")
    float LowDetailDistance = 5000.0f;

    // Performance Metrics
    UPROPERTY(BlueprintReadOnly, Category = "Crowd Performance")
    int32 CurrentActiveAgents = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd Performance")
    float CurrentFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd Performance")
    float CrowdMemoryUsage = 0.0f;

private:
    // Internal Systems
    void ProcessCrowdBehaviors(float DeltaTime);
    void UpdateAgentStates(float DeltaTime);
    void HandleCrowdInteractions();
    void OptimizeMemoryUsage();
    
    // Pathfinding
    TArray<FVector> FlowFieldGrid;
    TArray<FVector> WaypointNetwork;
    
    // Performance Tracking
    float LastUpdateTime = 0.0f;
    int32 FrameCounter = 0;
    float AccumulatedFrameTime = 0.0f;
};