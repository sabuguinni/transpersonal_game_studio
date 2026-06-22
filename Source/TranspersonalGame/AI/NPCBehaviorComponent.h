// NPCBehaviorComponent.h
// NPC Behavior Agent #11 — PROD_CYCLE_AUTO_20260622_003
// Drives NPC daily routines, memory, patrol, flee, and aggro state via Behavior Tree integration.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "NPCBehaviorComponent.generated.h"

// ─── Enums (global scope, NPC_ prefix) ───────────────────────────────────────

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Investigate UMETA(DisplayName = "Investigate"),
    Flee        UMETA(DisplayName = "Flee"),
    Alert       UMETA(DisplayName = "Alert"),
    Combat      UMETA(DisplayName = "Combat"),
    Rest        UMETA(DisplayName = "Rest"),
    Forage      UMETA(DisplayName = "Forage"),
};

UENUM(BlueprintType)
enum class ENPC_DailyRoutinePhase : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night"),
    DeepNight   UMETA(DisplayName = "DeepNight"),
};

UENUM(BlueprintType)
enum class ENPC_ThreatLevel : uint8
{
    None        UMETA(DisplayName = "None"),
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"),
    High        UMETA(DisplayName = "High"),
    Lethal      UMETA(DisplayName = "Lethal"),
};

// ─── Structs (global scope) ───────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    FVector ThreatLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float ThreatTimestamp = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    ENPC_ThreatLevel ThreatLevel = ENPC_ThreatLevel::None;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    bool bIsActive = false;
};

USTRUCT(BlueprintType)
struct FNPC_PatrolPoint
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Patrol")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Patrol")
    float WaitDuration = 2.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Patrol")
    bool bIsReached = false;
};

// ─── Component Class ──────────────────────────────────────────────────────────

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent), BlueprintType)
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ── State ──────────────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadWrite, Category = "NPC|State")
    ENPC_BehaviorState CurrentState = ENPC_BehaviorState::Idle;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|State")
    ENPC_DailyRoutinePhase RoutinePhase = ENPC_DailyRoutinePhase::Morning;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|State")
    ENPC_ThreatLevel CurrentThreatLevel = ENPC_ThreatLevel::None;

    // ── Detection ─────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Detection")
    float AggroRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Detection")
    float FleeRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Detection")
    float AttackRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Detection")
    float HearingRadius = 2000.0f;

    // ── Patrol ────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    TArray<FNPC_PatrolPoint> PatrolPoints;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Patrol", meta = (AllowPrivateAccess = "true"))
    int32 CurrentPatrolIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    float PatrolRadius = 5000.0f;

    // ── Memory ────────────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    TArray<FNPC_MemoryEntry> MemoryEntries;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    int32 MaxMemoryEntries = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    float MemoryDecayTime = 60.0f;

    // ── Survival Stats (mirrored from SurvivalComponent) ─────────────────
    UPROPERTY(BlueprintReadWrite, Category = "NPC|Survival")
    float HealthNormalized = 1.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Survival")
    float HungerNormalized = 1.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Survival")
    float FearLevel = 0.0f;

    // ── Blackboard Keys (for BT integration) ─────────────────────────────
    UPROPERTY(BlueprintReadWrite, Category = "NPC|BehaviorTree")
    FVector LastKnownThreatLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|BehaviorTree")
    bool bThreatVisible = false;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|BehaviorTree")
    bool bIsFleeing = false;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|BehaviorTree")
    bool bIsAttacking = false;

    // ── Functions ─────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void RegisterThreat(FVector ThreatLocation, ENPC_ThreatLevel Level);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void ClearMemory();

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void AdvancePatrolIndex();

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    ENPC_BehaviorState EvaluateState(float DistanceToPlayer);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void UpdateDailyRoutine(float GameHour);

    UFUNCTION(BlueprintPure, Category = "NPC|Behavior")
    bool IsPlayerInAggroRange() const;

    UFUNCTION(BlueprintPure, Category = "NPC|Behavior")
    bool IsPlayerInAttackRange() const;

    UFUNCTION(BlueprintPure, Category = "NPC|Memory")
    FNPC_MemoryEntry GetMostRecentThreat() const;

private:
    void TickMemoryDecay(float DeltaTime);
    void UpdateBlackboardKeys();

    float ElapsedTime = 0.0f;
    float DistanceToPlayerCached = 99999.0f;
};
