#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "NavigationSystem.h"
#include "AI/NavigationSystemBase.h"
#include "CrowdSimulationTypes.h"
#include "CrowdSimulationManager.generated.h"

class UCrowdAgentComponent;
class ACrowdWaypoint;

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowdSimulationManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowdSimulationManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Crowd Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    int32 MaxCrowdAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float SpawnRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float UpdateFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    FCrowd_LODSettings LODSettings;

    // Agent Management
    UPROPERTY(BlueprintReadOnly, Category = "Crowd State")
    TArray<FCrowd_AgentData> ActiveAgents;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd State")
    TArray<AActor*> CrowdActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Behavior")
    ECrowd_BehaviorType DefaultBehavior;

    // Pathfinding
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    TArray<ACrowdWaypoint*> Waypoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float PathfindingUpdateRate;

    // Performance
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableLODSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODUpdateRate;

private:
    // Internal state
    float LastUpdateTime;
    float LastLODUpdateTime;
    int32 NextAgentID;

    // Navigation system reference
    UNavigationSystemV1* NavSystem;

public:
    // Core Functions
    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void InitializeCrowdSystem();

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    int32 SpawnCrowdAgent(FVector Location, ECrowd_BehaviorType Behavior = ECrowd_BehaviorType::Wandering);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void RemoveCrowdAgent(int32 AgentID);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void SetAgentBehavior(int32 AgentID, ECrowd_BehaviorType NewBehavior);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void SetAgentTarget(int32 AgentID, FVector TargetLocation);

    // LOD System
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateLODSystem();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    ECrowd_LODLevel CalculateAgentLOD(const FCrowd_AgentData& Agent, FVector PlayerLocation);

    // Pathfinding
    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    bool FindPathForAgent(int32 AgentID, FVector TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    void UpdateAgentMovement(float DeltaTime);

    // Waypoint System
    UFUNCTION(BlueprintCallable, Category = "Waypoints")
    void RegisterWaypoint(ACrowdWaypoint* Waypoint);

    UFUNCTION(BlueprintCallable, Category = "Waypoints")
    ACrowdWaypoint* GetNearestWaypoint(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Waypoints")
    ACrowdWaypoint* GetRandomWaypoint();

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Crowd Info")
    int32 GetActiveCrowdCount() const { return ActiveAgents.Num(); }

    UFUNCTION(BlueprintCallable, Category = "Crowd Info")
    FCrowd_AgentData GetAgentData(int32 AgentID);

    UFUNCTION(BlueprintCallable, Category = "Crowd Info")
    TArray<FCrowd_AgentData> GetAgentsInRadius(FVector Center, float Radius);

    // Debug Functions
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugSpawnTestCrowd();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugClearAllAgents();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DrawDebugInfo();

protected:
    // Internal helper functions
    void UpdateAgentStates(float DeltaTime);
    void ProcessAgentBehavior(FCrowd_AgentData& Agent, float DeltaTime);
    void HandleWanderingBehavior(FCrowd_AgentData& Agent, float DeltaTime);
    void HandlePatrollingBehavior(FCrowd_AgentData& Agent, float DeltaTime);
    void HandleGatheringBehavior(FCrowd_AgentData& Agent, float DeltaTime);
    void HandleFleeingBehavior(FCrowd_AgentData& Agent, float DeltaTime);
    void HandleFollowingBehavior(FCrowd_AgentData& Agent, float DeltaTime);

    // Collision avoidance
    FVector CalculateSteeringForce(const FCrowd_AgentData& Agent);
    FVector CalculateSeparationForce(const FCrowd_AgentData& Agent);
    FVector CalculateAlignmentForce(const FCrowd_AgentData& Agent);
    FVector CalculateCohesionForce(const FCrowd_AgentData& Agent);

    // Utility
    bool IsValidAgentID(int32 AgentID) const;
    int32 FindAgentIndex(int32 AgentID) const;
    FVector GetPlayerLocation() const;
};

#include "CrowdSimulationManager.generated.h"