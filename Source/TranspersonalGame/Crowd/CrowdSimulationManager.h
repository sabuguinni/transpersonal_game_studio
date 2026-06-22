#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CrowdSimulationManager.generated.h"

// ── Enums (global scope — RULE 1) ──────────────────────────────────────────

UENUM(BlueprintType)
enum class ECrowd_WaypointType : uint8
{
    CampFire        UMETA(DisplayName = "Camp Fire"),
    WaterSource     UMETA(DisplayName = "Water Source"),
    HuntingGround   UMETA(DisplayName = "Hunting Ground"),
    ForestEdge      UMETA(DisplayName = "Forest Edge"),
    CaveEntrance    UMETA(DisplayName = "Cave Entrance"),
    FleePoint       UMETA(DisplayName = "Flee Point"),
};

UENUM(BlueprintType)
enum class ECrowd_AgentBehavior : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Wandering   UMETA(DisplayName = "Wandering"),
    Working     UMETA(DisplayName = "Working"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Gathering   UMETA(DisplayName = "Gathering"),
    Sleeping    UMETA(DisplayName = "Sleeping"),
};

UENUM(BlueprintType)
enum class ECrowd_LODLevel : uint8
{
    Full    UMETA(DisplayName = "Full — close range"),
    Medium  UMETA(DisplayName = "Medium — mid range"),
    Minimal UMETA(DisplayName = "Minimal — far range"),
};

// ── Structs (global scope — RULE 1) ────────────────────────────────────────

USTRUCT(BlueprintType)
struct FCrowd_Waypoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Waypoint")
    int32 WaypointID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Waypoint")
    FVector WorldLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Waypoint")
    ECrowd_WaypointType WaypointType = ECrowd_WaypointType::CampFire;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Waypoint")
    int32 MaxOccupants = 10;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Waypoint")
    int32 CurrentOccupants = 0;
};

USTRUCT(BlueprintType)
struct FCrowd_AgentData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    int32 AgentID = -1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    FVector CurrentLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    ECrowd_AgentBehavior CurrentBehavior = ECrowd_AgentBehavior::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    ECrowd_LODLevel LODLevel = ECrowd_LODLevel::Full;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    float MoveSpeed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    int32 AssignedWaypointID = -1;
};

// ── Manager class ───────────────────────────────────────────────────────────

UCLASS(BlueprintType, meta = (DisplayName = "Crowd Simulation Manager"))
class TRANSPERSONALGAME_API UCrowdSimulationManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCrowdSimulationManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Agent management
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void RegisterCrowdAgent(const FCrowd_AgentData& AgentData);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void UnregisterCrowdAgent(int32 AgentID);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    int32 GetActiveAgentCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    TArray<FCrowd_AgentData> GetAgentsInRadius(const FVector& Center, float Radius) const;

    // Waypoint queries
    UFUNCTION(BlueprintCallable, Category = "Crowd|Waypoints")
    FCrowd_Waypoint* FindNearestWaypoint(const FVector& FromLocation, ECrowd_WaypointType WaypointType);

    // LOD
    UFUNCTION(BlueprintCallable, Category = "Crowd|LOD")
    ECrowd_LODLevel GetAgentLOD(const FVector& AgentLocation, const FVector& PlayerLocation) const;

    // Threat response
    UFUNCTION(BlueprintCallable, Category = "Crowd|Behavior")
    void TriggerFleeResponse(const FVector& ThreatLocation, float ThreatRadius);

    // Config
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxCrowdAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float AgentUpdateRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float LODDistanceClose;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float LODDistanceMedium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float TickInterval;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    bool bCrowdSystemActive;

private:
    UPROPERTY()
    TArray<FCrowd_AgentData> ActiveAgents;

    UPROPERTY()
    TArray<FCrowd_Waypoint> RegisteredWaypoints;
};
