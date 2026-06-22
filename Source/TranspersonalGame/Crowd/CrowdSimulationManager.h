// CrowdSimulationManager.h
// Agent #13 — Crowd & Traffic Simulation
// Prehistoric tribal crowd simulation — up to 50 agents with patrol, foraging, flee, day-phase behavior

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CrowdSimulationManager.generated.h"

// ─── Enums (global scope — RULE 1) ───────────────────────────────────────────

UENUM(BlueprintType)
enum class ECrowd_AgentType : uint8
{
    TribalMember    UMETA(DisplayName = "Tribal Member"),
    Forager         UMETA(DisplayName = "Forager"),
    Scout           UMETA(DisplayName = "Scout"),
    Elder           UMETA(DisplayName = "Elder")
};

UENUM(BlueprintType)
enum class ECrowd_AgentState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Patrolling      UMETA(DisplayName = "Patrolling"),
    Foraging        UMETA(DisplayName = "Foraging"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Sheltering      UMETA(DisplayName = "Sheltering"),
    Interacting     UMETA(DisplayName = "Interacting")
};

UENUM(BlueprintType)
enum class ECrowd_DayPhase : uint8
{
    Dawn            UMETA(DisplayName = "Dawn"),
    Day             UMETA(DisplayName = "Day"),
    Dusk            UMETA(DisplayName = "Dusk"),
    Night           UMETA(DisplayName = "Night")
};

// ─── Structs (global scope — RULE 1) ─────────────────────────────────────────

USTRUCT(BlueprintType)
struct FCrowd_AgentData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    int32 AgentID = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_AgentType AgentType = ECrowd_AgentType::TribalMember;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_AgentState CurrentState = ECrowd_AgentState::Idle;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float MoveSpeed = 120.f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float FearLevel = 0.f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    bool bIsAlive = true;

    // Internal navigation state
    int32 CurrentWaypointIndex = 0;
    float IdleTimer = 0.f;
    float ForageTimer = 0.f;
    FVector ForageTarget = FVector::ZeroVector;
};

// ─── Class ───────────────────────────────────────────────────────────────────

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCrowdSimulationManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCrowdSimulationManager();

    // UWorldSubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ─── Configuration ────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxCrowdAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    FVector CampCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float CampRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float PatrolSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float FleeSpeed;

    // ─── Runtime State ────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    TArray<FCrowd_AgentData> ActiveAgents;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    TArray<FVector> PatrolWaypoints;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    float ThreatLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    ECrowd_DayPhase DayPhase;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    bool bCrowdInitialized;

    // ─── Public API ───────────────────────────────────────────────────────

    /** Tick all crowd agents — call from GameMode or a Tick-enabled actor */
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void TickCrowdSimulation(float DeltaTime);

    /** Notify all nearby agents of a threat (dinosaur attack, fire, etc.) */
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void NotifyThreat(FVector ThreatLocation, float Intensity);

    /** Change the time-of-day phase, adjusting crowd behavior */
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void SetDayPhase(ECrowd_DayPhase NewPhase);

    /** Returns number of living agents */
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    int32 GetActiveAgentCount() const;

    /** Returns all agents of a given type */
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    TArray<FCrowd_AgentData> GetAgentsByType(ECrowd_AgentType Type) const;

private:
    void InitializeCrowdAgents();
    void UpdateIdleAgent(FCrowd_AgentData& Agent, float DeltaTime);
    void UpdatePatrolAgent(FCrowd_AgentData& Agent, float DeltaTime);
    void UpdateForagingAgent(FCrowd_AgentData& Agent, float DeltaTime);
    void UpdateFleeingAgent(FCrowd_AgentData& Agent, float DeltaTime);
};
