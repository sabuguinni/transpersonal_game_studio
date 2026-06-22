// CrowdSimulationManager.h
// Agent #13 — Crowd & Traffic Simulation
// Prehistoric crowd simulation: tribe members, migrating herds, LOD crowd clusters

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "CrowdSimulationManager.generated.h"

// ─── Enums ───────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ECrowd_AgentState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Wandering   UMETA(DisplayName = "Wandering"),
    Working     UMETA(DisplayName = "Working"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Gathering   UMETA(DisplayName = "Gathering"),
    Sleeping    UMETA(DisplayName = "Sleeping")
};

UENUM(BlueprintType)
enum class ECrowd_GroupType : uint8
{
    TribeMember     UMETA(DisplayName = "Tribe Member"),
    HerdAnimal      UMETA(DisplayName = "Herd Animal"),
    NomadGroup      UMETA(DisplayName = "Nomad Group"),
    ScavengerPack   UMETA(DisplayName = "Scavenger Pack"),
    LODCluster      UMETA(DisplayName = "LOD Cluster")
};

// ─── Structs ─────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FCrowd_AgentData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_AgentState State = ECrowd_AgentState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_GroupType GroupType = ECrowd_GroupType::TribeMember;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float MoveSpeed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float FleeRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 GroupID = 0;
};

USTRUCT(BlueprintType)
struct FCrowd_WaypointNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector WorldPosition = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FName NodeLabel = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    TArray<int32> ConnectedNodeIndices;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float TrafficWeight = 1.0f;
};

USTRUCT(BlueprintType)
struct FCrowd_LODCluster
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    int32 AgentCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    float ClusterRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    ECrowd_GroupType ClusterType = ECrowd_GroupType::LODCluster;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    bool bIsVisible = true;
};

// ─── Main Class ──────────────────────────────────────────────────────────────

UCLASS(ClassGroup = (TranspersonalGame), meta = (DisplayName = "Crowd Simulation Manager"))
class TRANSPERSONALGAME_API ACrowdSimulationManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowdSimulationManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ── Configuration ──────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxActiveAgents = 50000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxFullDetailAgents = 200;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float LODSwitchDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float FleeResponseRadius = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float TickInterval = 0.1f;

    // ── Runtime Data ───────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|Runtime")
    TArray<FCrowd_AgentData> ActiveAgents;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|Runtime")
    TArray<FCrowd_WaypointNode> WaypointGraph;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|Runtime")
    TArray<FCrowd_LODCluster> LODClusters;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|Runtime")
    int32 CurrentActiveCount = 0;

    // ── Public API ─────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void SpawnTribeGroup(FVector CampCenter, int32 MemberCount, int32 GroupID);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void TriggerFleeEvent(FVector ThreatLocation, float ThreatRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void RegisterWaypoint(FVector WorldPos, FName Label);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void UpdateLODClusters(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    int32 GetActiveAgentCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void SetAgentState(int32 AgentIndex, ECrowd_AgentState NewState);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    FCrowd_AgentData GetAgentData(int32 AgentIndex) const;

private:
    float AccumulatedTime = 0.0f;

    void TickAgents(float DeltaTime);
    void UpdateAgentMovement(FCrowd_AgentData& Agent, float DeltaTime);
    FVector GetNextWaypointTarget(const FCrowd_AgentData& Agent) const;
    bool IsAgentInLODRange(const FCrowd_AgentData& Agent, const FVector& PlayerLoc) const;
};
