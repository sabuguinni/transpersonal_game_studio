// CrowdSimulationManager.h
// Crowd & Traffic Simulation Agent #13
// Herd migration, territory systems, and crowd AI for prehistoric fauna

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CrowdSimulationManager.generated.h"

// ─── Enums ───────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ECrowd_HerdType : uint8
{
    None               UMETA(DisplayName = "None"),
    RaptorPack         UMETA(DisplayName = "Raptor Pack"),
    BrachiosaurusHerd  UMETA(DisplayName = "Brachiosaurus Herd"),
    TRexSolitary       UMETA(DisplayName = "T-Rex Solitary"),
    HerbivoreHerd      UMETA(DisplayName = "Herbivore Herd"),
};

UENUM(BlueprintType)
enum class ECrowd_AgentState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Grazing     UMETA(DisplayName = "Grazing"),
    Resting     UMETA(DisplayName = "Resting"),
};

// ─── Structs ─────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FCrowd_HerdWaypoint
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FString WaypointLabel;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_HerdType AssignedHerdType = ECrowd_HerdType::None;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    bool bIsActive = true;
};

USTRUCT(BlueprintType)
struct FCrowd_HerdAgent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    AActor* AgentActor = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_HerdType HerdType = ECrowd_HerdType::None;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_AgentState CurrentState = ECrowd_AgentState::Idle;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    int32 CurrentWaypointIndex = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    int32 TotalWaypoints = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float TimeSinceLastDecision = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float FleeTimer = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float HuntTimer = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector ThreatLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector TargetLocation = FVector::ZeroVector;
};

// ─── Delegate ────────────────────────────────────────────────────────────────

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCrowd_OnPlayerEnterTerritory,
    ECrowd_HerdType, HerdType,
    FVector, PlayerLocation);

// ─── Component ───────────────────────────────────────────────────────────────

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent), BlueprintType)
class TRANSPERSONALGAME_API UCrowdSimulationManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowdSimulationManager();

    // ── Lifecycle ──────────────────────────────────────────────────────────
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

    // ── Configuration ──────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxActiveAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float HerdUpdateRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float MigrationSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float TerritoryRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Debug")
    bool bEnableCrowdDebug;

    // ── Runtime State ──────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State",
        meta = (AllowPrivateAccess = "true"))
    int32 ActiveAgentCount;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State",
        meta = (AllowPrivateAccess = "true"))
    bool bSystemInitialized = false;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State",
        meta = (AllowPrivateAccess = "true"))
    bool bPlayerInTRexTerritory = false;

    // ── Waypoint Data ──────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Waypoints")
    TArray<FCrowd_HerdWaypoint> RaptorPatrolWaypoints;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Waypoints")
    TArray<FCrowd_HerdWaypoint> BrachMigrationWaypoints;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Waypoints")
    TArray<FCrowd_HerdWaypoint> TRexTerritoryMarkers;

    // ── Agent Registry ─────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Agents")
    TArray<FCrowd_HerdAgent> ActiveAgents;

    // ── Public API ─────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void RegisterHerdAgent(AActor* AgentActor, ECrowd_HerdType HerdType);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void TriggerHerdFlee(ECrowd_HerdType HerdType, FVector ThreatLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void TriggerRaptorHunt(FVector TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    int32 GetActiveAgentCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    bool IsPlayerInDangerZone() const;

    // ── Delegate ───────────────────────────────────────────────────────────
    UPROPERTY(BlueprintAssignable, Category = "Crowd|Events")
    FCrowd_OnPlayerEnterTerritory OnPlayerEnterTerritoryDelegate;

private:
    void InitializeCrowdSystems();
    void UpdateHerdBehaviors(float DeltaTime);
    void UpdateTerritoryStates(float DeltaTime);
    void UpdateSingleAgent(FCrowd_HerdAgent& Agent, float DeltaTime);
};
