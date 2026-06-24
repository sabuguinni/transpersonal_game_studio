#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NPCBehaviorComponent.generated.h"

// ============================================================
// NPC Behavior Component — Agent #11
// Drives NPC daily routines, memory, and state transitions
// for the prehistoric survival game.
// ============================================================

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Forage      UMETA(DisplayName = "Forage"),
    Flee        UMETA(DisplayName = "Flee"),
    Investigate UMETA(DisplayName = "Investigate"),
    Chase       UMETA(DisplayName = "Chase"),
    Attack      UMETA(DisplayName = "Attack"),
    Shelter     UMETA(DisplayName = "Shelter"),
    Sleep       UMETA(DisplayName = "Sleep")
};

UENUM(BlueprintType)
enum class ENPC_ThreatLevel : uint8
{
    None    UMETA(DisplayName = "None"),
    Low     UMETA(DisplayName = "Low"),
    Medium  UMETA(DisplayName = "Medium"),
    High    UMETA(DisplayName = "High"),
    Extreme UMETA(DisplayName = "Extreme")
};

USTRUCT(BlueprintType)
struct FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    float Timestamp = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    ENPC_ThreatLevel ThreatLevel = ENPC_ThreatLevel::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    FString EventDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    bool bIsPlayerRelated = false;
};

USTRUCT(BlueprintType)
struct FNPC_PatrolPoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    FVector WorldLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    float WaitDuration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    bool bLookAround = true;
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent), DisplayName = "NPC Behavior Component")
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ── State Machine ──────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintPure, Category = "NPC|Behavior")
    ENPC_BehaviorState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void EvaluateStateTransition();

    // ── Memory System ──────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void RecordMemory(FVector Location, ENPC_ThreatLevel Threat, const FString& Description, bool bPlayerRelated = false);

    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void ForgetOldMemories(float MaxAge = 120.0f);

    UFUNCTION(BlueprintPure, Category = "NPC|Memory")
    bool HasRecentThreatMemory(float WithinSeconds = 30.0f) const;

    UFUNCTION(BlueprintPure, Category = "NPC|Memory")
    FNPC_MemoryEntry GetMostRecentMemory() const;

    // ── Patrol ─────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    void SetPatrolPoints(const TArray<FNPC_PatrolPoint>& Points);

    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    FVector GetNextPatrolTarget();

    // ── Threat Detection ───────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "NPC|Threat")
    void OnThreatDetected(AActor* ThreatActor, ENPC_ThreatLevel Level);

    UFUNCTION(BlueprintCallable, Category = "NPC|Threat")
    void OnThreatLost();

    UFUNCTION(BlueprintPure, Category = "NPC|Threat")
    bool IsThreatActive() const { return CurrentThreatLevel != ENPC_ThreatLevel::None; }

    // ── Daily Routine ──────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "NPC|Routine")
    void UpdateDailyRoutine(float GameHour);

    // ── Configuration ──────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float PatrolRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float ChaseRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float AttackRange = 250.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float FleeHealthThreshold = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float StateEvalInterval = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    int32 MaxMemoryEntries = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    bool bEnableDailyRoutine = true;

    // ── Runtime State (Read-Only) ──────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "NPC|State", meta = (AllowPrivateAccess = "true"))
    ENPC_BehaviorState CurrentState = ENPC_BehaviorState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State", meta = (AllowPrivateAccess = "true"))
    ENPC_ThreatLevel CurrentThreatLevel = ENPC_ThreatLevel::None;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State", meta = (AllowPrivateAccess = "true"))
    TArray<FNPC_MemoryEntry> MemoryLog;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State", meta = (AllowPrivateAccess = "true"))
    AActor* CurrentThreatActor = nullptr;

private:
    TArray<FNPC_PatrolPoint> PatrolPoints;
    int32 CurrentPatrolIndex = 0;
    float StateEvalTimer = 0.0f;
    float CurrentGameHour = 6.0f;
    float CurrentHealth = 1.0f;

    void TickStateLogic(float DeltaTime);
    float GetDistanceToThreat() const;
};
