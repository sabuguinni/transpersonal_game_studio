#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "AI/NavigationSystemBase.h"
#include "NavigationSystem.h"
#include "../SharedTypes.h"
#include "Crowd_SimulationManager.generated.h"

class ACrowd_NPCActor;
class UCrowd_BehaviorComponent;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_AgentData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Agent")
    FVector Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Agent")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Agent")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Agent")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Agent")
    float PersonalSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Agent")
    ECrowd_BehaviorState BehaviorState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Agent")
    float PanicLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Agent")
    int32 GroupID;

    FCrowd_AgentData()
    {
        Position = FVector::ZeroVector;
        Velocity = FVector::ZeroVector;
        TargetLocation = FVector::ZeroVector;
        Speed = 150.0f;
        PersonalSpace = 100.0f;
        BehaviorState = ECrowd_BehaviorState::Idle;
        PanicLevel = 0.0f;
        GroupID = -1;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_GroupData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Group")
    TArray<int32> AgentIndices;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Group")
    FVector GroupCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Group")
    FVector GroupTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Group")
    ECrowd_BehaviorState GroupBehavior;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Group")
    float GroupCohesion;

    FCrowd_GroupData()
    {
        GroupCenter = FVector::ZeroVector;
        GroupTarget = FVector::ZeroVector;
        GroupBehavior = ECrowd_BehaviorState::Idle;
        GroupCohesion = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCrowd_SimulationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UCrowd_SimulationManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Crowd management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 RegisterAgent(const FVector& StartPosition, int32 GroupID = -1);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UnregisterAgent(int32 AgentIndex);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateAgentTarget(int32 AgentIndex, const FVector& NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetAgentBehavior(int32 AgentIndex, ECrowd_BehaviorState NewBehavior);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void TriggerPanicResponse(const FVector& ThreatLocation, float ThreatRadius);

    // Group management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 CreateGroup(const TArray<int32>& AgentIndices);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetGroupTarget(int32 GroupID, const FVector& Target);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetGroupBehavior(int32 GroupID, ECrowd_BehaviorState Behavior);

    // Simulation control
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void StartSimulation();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void StopSimulation();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void PauseSimulation(bool bPause);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetSimulationSpeed(float Speed);

    // LOD system
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetLODDistance(float Distance);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateLODForAgent(int32 AgentIndex, float DistanceToPlayer);

    // Query functions
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    TArray<FCrowd_AgentData> GetAgentsInRadius(const FVector& Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    FCrowd_AgentData GetAgentData(int32 AgentIndex);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 GetActiveAgentCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    float GetAverageAgentDensity() const;

protected:
    // Core simulation data
    UPROPERTY()
    TArray<FCrowd_AgentData> Agents;

    UPROPERTY()
    TArray<FCrowd_GroupData> Groups;

    UPROPERTY()
    TArray<int32> FreeAgentIndices;

    UPROPERTY()
    TArray<int32> FreeGroupIndices;

    // Simulation parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation Settings")
    float TickInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation Settings")
    float SimulationSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation Settings")
    int32 MaxAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation Settings")
    float LODDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation Settings")
    bool bSimulationActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation Settings")
    bool bSimulationPaused;

    // Flocking parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float SeparationWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float AlignmentWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float CohesionWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float AvoidanceWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float SeekWeight;

    // Simulation timer
    FTimerHandle SimulationTimerHandle;

private:
    // Core simulation functions
    void TickSimulation();
    void UpdateAgent(int32 AgentIndex, float DeltaTime);
    void UpdateGroup(int32 GroupIndex, float DeltaTime);

    // Flocking behaviors
    FVector CalculateSeparation(int32 AgentIndex);
    FVector CalculateAlignment(int32 AgentIndex);
    FVector CalculateCohesion(int32 AgentIndex);
    FVector CalculateAvoidance(int32 AgentIndex);
    FVector CalculateSeek(int32 AgentIndex, const FVector& Target);

    // Utility functions
    TArray<int32> GetNearbyAgents(int32 AgentIndex, float Radius);
    bool IsValidAgentIndex(int32 Index) const;
    bool IsValidGroupIndex(int32 Index) const;
    int32 GetNextAvailableAgentIndex();
    int32 GetNextAvailableGroupIndex();
};