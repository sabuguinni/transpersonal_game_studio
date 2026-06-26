#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TimerManager.h"
#include "CrowdSimulationManager.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// Enums — must be at global scope (UE5 RULE 1)
// ─────────────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ECrowd_GroupType : uint8
{
    HerbivoreHerd   UMETA(DisplayName = "Herbivore Herd"),
    PredatorPack    UMETA(DisplayName = "Predator Pack"),
    MigrationGroup  UMETA(DisplayName = "Migration Group"),
    ScavengerFlock  UMETA(DisplayName = "Scavenger Flock"),
    SolitaryRoamer  UMETA(DisplayName = "Solitary Roamer")
};

UENUM(BlueprintType)
enum class ECrowd_LODLevel : uint8
{
    LOD_Full    UMETA(DisplayName = "Full Simulation"),
    LOD_Medium  UMETA(DisplayName = "Medium LOD"),
    LOD_Low     UMETA(DisplayName = "Low LOD"),
    LOD_Culled  UMETA(DisplayName = "Culled")
};

UENUM(BlueprintType)
enum class ECrowd_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Wandering   UMETA(DisplayName = "Wandering"),
    Grazing     UMETA(DisplayName = "Grazing"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Migrating   UMETA(DisplayName = "Migrating")
};

// ─────────────────────────────────────────────────────────────────────────────
// Structs — must be at global scope (UE5 RULE 1)
// ─────────────────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FCrowd_GroupData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FName GroupID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_GroupType GroupType = ECrowd_GroupType::HerbivoreHerd;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_BehaviorState BehaviorState = ECrowd_BehaviorState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_LODLevel CurrentLOD = ECrowd_LODLevel::LOD_Full;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 AgentCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector HomeLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float WanderRadius = 1000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float MovementSpeed = 300.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float FleeThreshold = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float AlertLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    bool bIsAlerting = false;
};

// ─────────────────────────────────────────────────────────────────────────────
// UCrowdSimulationManager — World Subsystem
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCrowdSimulationManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCrowdSimulationManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ── Group Registration ──────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void RegisterCrowdGroup(const FCrowd_GroupData& GroupData);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void RegisterDefaultGroups();

    // ── Alert System ────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void AlertGroupsInRadius(FVector Origin, float Radius, float AlertStrength);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void ResetAlerts();

    // ── LOD Management ──────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void UpdateCrowdLOD(FVector PlayerLocation);

    // ── Pathfinding ─────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    FVector GetFleeDestination(FName GroupID, FVector ThreatLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    FVector GetWanderTarget(FName GroupID);

    // ── Simulation Control ──────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void StartSimulationTick();

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void StopSimulationTick();

    // ── Query Interface ─────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crowd")
    int32 GetGroupCount() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crowd")
    bool IsGroupAlerting(FName GroupID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crowd")
    float GetGroupAlertLevel(FName GroupID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crowd")
    FCrowd_GroupData GetGroupData(FName GroupID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crowd")
    TArray<FName> GetAlertingGroups() const;

    // ── State ───────────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    bool bSimulationActive;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    float GlobalAlertLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    int32 CurrentAgentCount;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    int32 MaxActiveAgents;

private:
    UPROPERTY()
    TArray<FCrowd_GroupData> CrowdGroups;

    float SimulationTickRate;
    FTimerHandle SimulationTickHandle;

    void OnSimulationTick();
    void TickGroupBehavior(FCrowd_GroupData& Group);
};
