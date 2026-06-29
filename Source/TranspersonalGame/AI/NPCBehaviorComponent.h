#pragma once

// NPCBehaviorComponent.h
// NPC Behavior Agent #11 — PROD_CYCLE_AUTO_20260629_013
// UActorComponent subclass that drives all NPC daily routines, threat response,
// memory decay, and state transitions in the prehistoric survival world.

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NPCBehaviorComponent.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// Enums — GLOBAL SCOPE (UHT requirement)
// ─────────────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Gather      UMETA(DisplayName = "Gather"),
    Flee        UMETA(DisplayName = "Flee"),
    Seek        UMETA(DisplayName = "Seek"),
    Investigate UMETA(DisplayName = "Investigate"),
    Interact    UMETA(DisplayName = "Interact"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ENPC_ThreatLevel : uint8
{
    None     UMETA(DisplayName = "None"),
    Low      UMETA(DisplayName = "Low"),
    Medium   UMETA(DisplayName = "Medium"),
    High     UMETA(DisplayName = "High"),
    Critical UMETA(DisplayName = "Critical")
};

UENUM(BlueprintType)
enum class ENPC_TimeOfDay : uint8
{
    Dawn      UMETA(DisplayName = "Dawn"),
    Morning   UMETA(DisplayName = "Morning"),
    Midday    UMETA(DisplayName = "Midday"),
    Afternoon UMETA(DisplayName = "Afternoon"),
    Night     UMETA(DisplayName = "Night")
};

// ─────────────────────────────────────────────────────────────────────────────
// Delegates
// ─────────────────────────────────────────────────────────────────────────────

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FNPC_OnStateChanged,
    ENPC_BehaviorState, OldState,
    ENPC_BehaviorState, NewState);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNPC_OnThreatLevelChanged,
    ENPC_ThreatLevel, NewThreatLevel);

// ─────────────────────────────────────────────────────────────────────────────
// UNPCBehaviorComponent
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent),
       DisplayName = "NPC Behavior Component")
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:

    UNPCBehaviorComponent();

    // ── Lifecycle ────────────────────────────────────────────────────────────

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    // ── State Control ────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void RegisterThreat(AActor* ThreatActor, float ThreatDistance);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void ForgetThreat();

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void KillNPC();

    // ── Daily Routine ────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "NPC|Routine")
    void SetTimeOfDay(ENPC_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "NPC|Routine")
    void ReturnToHome();

    // ── Patrol ───────────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    void SetPatrolPoints(const TArray<FVector>& Points);

    // ── Accessors ────────────────────────────────────────────────────────────

    UFUNCTION(BlueprintPure, Category = "NPC|Behavior")
    ENPC_BehaviorState GetCurrentState() const;

    UFUNCTION(BlueprintPure, Category = "NPC|Behavior")
    ENPC_ThreatLevel GetCurrentThreatLevel() const;

    // ── Delegates ────────────────────────────────────────────────────────────

    UPROPERTY(BlueprintAssignable, Category = "NPC|Events")
    FNPC_OnStateChanged OnStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "NPC|Events")
    FNPC_OnThreatLevelChanged OnThreatLevelChanged;

    // ── Configuration ────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Movement")
    float PatrolRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Movement")
    float PatrolSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Movement")
    float FleeSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Movement")
    float ChaseSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Movement")
    float InteractRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Threat")
    float ThreatDetectRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    float MemoryDuration;

    // ── Runtime State (read-only in BP) ──────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Behavior",
              meta = (AllowPrivateAccess = "true"))
    ENPC_BehaviorState CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Behavior",
              meta = (AllowPrivateAccess = "true"))
    ENPC_BehaviorState PreviousState;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Threat",
              meta = (AllowPrivateAccess = "true"))
    ENPC_ThreatLevel CurrentThreatLevel;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Routine",
              meta = (AllowPrivateAccess = "true"))
    ENPC_TimeOfDay CurrentTimeOfDay;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory",
              meta = (AllowPrivateAccess = "true"))
    FVector LastKnownThreatLocation;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory",
              meta = (AllowPrivateAccess = "true"))
    float LastKnownThreatAge;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Behavior",
              meta = (AllowPrivateAccess = "true"))
    bool bIsAlerted;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Patrol",
              meta = (AllowPrivateAccess = "true"))
    FVector HomeLocation;

private:

    // ── Patrol ───────────────────────────────────────────────────────────────

    TArray<FVector> PatrolPoints;
    bool            bHasPatrolRoute;
    int32           PatrolIndex;

    // ── Timers ───────────────────────────────────────────────────────────────

    FTimerHandle DailyRoutineTimer;
    FTimerHandle ThreatScanTimer;

    // ── Internal Ticks ───────────────────────────────────────────────────────

    void TickPatrol    (float DeltaTime);
    void TickFlee      (float DeltaTime);
    void TickSeek      (float DeltaTime);
    void TickInvestigate(float DeltaTime);
    void TickGather    (float DeltaTime);

    // ── Internal Routines ────────────────────────────────────────────────────

    void EvaluateDailyRoutine();
    void ScanForThreats();
};
