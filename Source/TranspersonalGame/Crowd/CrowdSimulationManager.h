// CrowdSimulationManager.h
// Agent #13 — Crowd & Traffic Simulation
// Prehistoric tribal crowd simulation — up to 50,000 agents via Mass AI foundations

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CrowdSimulationManager.generated.h"

// ============================================================
// ENUMS — must be at global scope (UE5 compilation rule)
// ============================================================

UENUM(BlueprintType)
enum class ECrowd_GroupBehavior : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Migrating   UMETA(DisplayName = "Migrating"),
    Foraging    UMETA(DisplayName = "Foraging"),
    Resting     UMETA(DisplayName = "Resting"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Mourning    UMETA(DisplayName = "Mourning")
};

UENUM(BlueprintType)
enum class ECrowd_LODLevel : uint8
{
    Full        UMETA(DisplayName = "Full — within 50m"),
    Medium      UMETA(DisplayName = "Medium — 50-200m"),
    Low         UMETA(DisplayName = "Low — 200-500m"),
    Culled      UMETA(DisplayName = "Culled — beyond 500m")
};

// ============================================================
// STRUCTS — must be at global scope
// ============================================================

USTRUCT(BlueprintType)
struct FCrowd_Waypoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Waypoint")
    FName WaypointID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Waypoint")
    FVector WorldLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Waypoint")
    float WaitTimeSeconds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Waypoint")
    ECrowd_GroupBehavior BehaviorAtWaypoint;

    FCrowd_Waypoint()
        : WaypointID(NAME_None)
        , WorldLocation(FVector::ZeroVector)
        , WaitTimeSeconds(0.0f)
        , BehaviorAtWaypoint(ECrowd_GroupBehavior::Idle)
    {}
};

USTRUCT(BlueprintType)
struct FCrowd_TribalGroup
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Group")
    FName GroupID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Group")
    int32 MemberCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Group")
    FVector CurrentLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Group")
    ECrowd_GroupBehavior CurrentBehavior;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Group")
    ECrowd_LODLevel LODLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Group")
    int32 CurrentWaypointIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Group")
    float ThreatAwareness;  // 0.0 = calm, 1.0 = maximum alert

    FCrowd_TribalGroup()
        : GroupID(NAME_None)
        , MemberCount(0)
        , CurrentLocation(FVector::ZeroVector)
        , CurrentBehavior(ECrowd_GroupBehavior::Idle)
        , LODLevel(ECrowd_LODLevel::Culled)
        , CurrentWaypointIndex(0)
        , ThreatAwareness(0.0f)
    {}
};

// ============================================================
// SUBSYSTEM CLASS
// ============================================================

UCLASS()
class TRANSPERSONALGAME_API UCrowdSimulationManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCrowdSimulationManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // --- Public API ---

    UFUNCTION(BlueprintCallable, Category = "Crowd|Simulation")
    void RegisterTribalGroup(const FCrowd_TribalGroup& Group);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Simulation")
    void UnregisterTribalGroup(FName GroupID);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Simulation")
    void AddMigrationWaypoint(const FCrowd_Waypoint& Waypoint);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Simulation")
    void TriggerFleeResponse(FVector ThreatLocation, float ThreatRadius = 800.0f);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Simulation")
    void UpdateGroupBehavior(float DeltaTime);

    UFUNCTION(BlueprintPure, Category = "Crowd|Simulation")
    int32 GetActiveAgentCount() const;

    UFUNCTION(BlueprintPure, Category = "Crowd|Simulation")
    TArray<FCrowd_TribalGroup> GetAllGroups() const;

    // --- Config ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float TickInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float MigrationSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float FleeRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float GroupCohesionRadius;

private:
    UPROPERTY()
    TMap<FName, FCrowd_TribalGroup> TribalGroups;

    UPROPERTY()
    TArray<FCrowd_Waypoint> MigrationWaypoints;

    UPROPERTY()
    int32 ActiveAgentCount;

    UPROPERTY()
    bool bSimulationRunning;

    void advance_migration(FCrowd_TribalGroup& Group, float DeltaTime);
    void flee_from_threat(FCrowd_TribalGroup& Group, float DeltaTime);
    void unregister_all_groups();
};
