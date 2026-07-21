#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AI/Crowd/CrowdBehaviorTypes.h"
#include "CrowdSimulationController.generated.h"

// ============================================================
// CrowdSimulationController — Agent #13 Crowd & Traffic Simulation
// Manages up to 50,000 lightweight crowd agents using flocking
// algorithms (separation, alignment, cohesion) + stampede events.
// Cycle: PROD_CYCLE_AUTO_20260629_010
// ============================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCrowd_OnStampedeStarted, int32, GroupID, FVector, Direction);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCrowd_OnStampedeEnded, int32, GroupID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCrowd_OnThreatDetected, int32, GroupID, ECrowd_ThreatLevel, ThreatLevel);

UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Crowd Simulation Controller"))
class TRANSPERSONALGAME_API ACrowdSimulationController : public AActor
{
    GENERATED_BODY()

public:
    ACrowdSimulationController();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

    // ─── Configuration ──────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxAgents = 500;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxHerdGroups = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float SimulationTickRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float LODDistanceClose = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float LODDistanceMedium = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float LODDistanceFar = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    bool bEnableStampedes = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    bool bEnableFlocking = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float FlockingWeight_Separation = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float FlockingWeight_Alignment = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float FlockingWeight_Cohesion = 0.8f;

    // ─── Runtime State ──────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    int32 ActiveAgentCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    int32 ActiveHerdCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    int32 ActiveStampedeCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    float LastSimulationTimeMs = 0.0f;

    // ─── Events ─────────────────────────────────────────────────

    UPROPERTY(BlueprintAssignable, Category = "Crowd|Events")
    FCrowd_OnStampedeStarted OnStampedeStarted;

    UPROPERTY(BlueprintAssignable, Category = "Crowd|Events")
    FCrowd_OnStampedeEnded OnStampedeEnded;

    UPROPERTY(BlueprintAssignable, Category = "Crowd|Events")
    FCrowd_OnThreatDetected OnThreatDetected;

    // ─── Public API ─────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Crowd|Management")
    int32 SpawnHerdGroup(ECrowd_HerdType HerdType, FVector SpawnCenter, int32 AgentCount, float SpawnRadius = 500.0f);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Management")
    void DisbandHerdGroup(int32 GroupID);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Management")
    void TriggerStampede(int32 GroupID, FVector ThreatLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Management")
    void TriggerGlobalThreat(FVector ThreatLocation, float ThreatRadius, ECrowd_ThreatLevel ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Query")
    FCrowd_AgentData GetAgentData(int32 AgentID) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Query")
    FCrowd_HerdGroup GetHerdGroupData(int32 GroupID) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Query")
    TArray<int32> GetAgentsInRadius(FVector Center, float Radius) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Query")
    int32 GetNearestAgentToLocation(FVector Location) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Debug")
    void SetAgentState(int32 AgentID, ECrowd_AgentState NewState);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Crowd|Debug")
    void SpawnTestHerd();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Crowd|Debug")
    void ClearAllAgents();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Crowd|Debug")
    void PrintCrowdStats();

private:
    // Agent pool
    TArray<FCrowd_AgentData> AgentPool;
    TArray<FCrowd_HerdGroup> HerdGroups;
    TArray<FCrowd_StampedeEvent> ActiveStampedes;

    int32 NextAgentID = 0;
    int32 NextGroupID = 0;

    float SimAccumulator = 0.0f;

    // Internal simulation steps
    void SimulateAgentBatch(int32 StartIdx, int32 EndIdx, float DeltaTime);
    void UpdateFlocking(FCrowd_AgentData& Agent, float DeltaTime);
    void UpdateStampedes(float DeltaTime);
    void UpdateHerdCentroids();
    void UpdateAgentLOD(FCrowd_AgentData& Agent, const FVector& PlayerLocation);

    FVector ComputeSeparation(const FCrowd_AgentData& Agent) const;
    FVector ComputeAlignment(const FCrowd_AgentData& Agent) const;
    FVector ComputeCohesion(const FCrowd_AgentData& Agent) const;

    int32 AllocateAgent();
    void FreeAgent(int32 AgentID);
};
