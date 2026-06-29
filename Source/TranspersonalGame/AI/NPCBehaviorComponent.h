// NPCBehaviorComponent.h
// NPC Behavior Agent #11 — PROD_CYCLE_AUTO_20260629_002
// UActorComponent — NPC daily routine, threat memory, behavior state machine

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"
#include "NPCBehaviorComponent.generated.h"

// ─── ENUMS (global scope — RULE 1) ──────────────────────────────────────────

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Forage      UMETA(DisplayName = "Forage"),
    Flee        UMETA(DisplayName = "Flee"),
    Alert       UMETA(DisplayName = "Alert"),
    Socialise   UMETA(DisplayName = "Socialise"),
    Sleep       UMETA(DisplayName = "Sleep"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ENPC_ThreatLevel : uint8
{
    None        UMETA(DisplayName = "None"),
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"),
    High        UMETA(DisplayName = "High"),
    Critical    UMETA(DisplayName = "Critical")
};

// ─── STRUCTS (global scope — RULE 1) ────────────────────────────────────────

USTRUCT(BlueprintType)
struct FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    AActor* ThreatActor = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    float Timestamp = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    ENPC_ThreatLevel ThreatLevel = ENPC_ThreatLevel::None;
};

USTRUCT(BlueprintType)
struct FNPC_DailyRoutineSlot
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Routine")
    float StartHour = 6.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Routine")
    float EndHour = 9.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Routine")
    ENPC_BehaviorState TargetState = ENPC_BehaviorState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Routine")
    FVector RoutineLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Routine")
    bool bMustReachLocation = false;
};

// ─── DELEGATE DECLARATIONS ──────────────────────────────────────────────────

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FNPC_OnBehaviorStateChanged,
    ENPC_BehaviorState, OldState,
    ENPC_BehaviorState, NewState
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FNPC_OnThreatDetected,
    AActor*, ThreatActor,
    ENPC_ThreatLevel, ThreatLevel
);

// ─── COMPONENT CLASS ─────────────────────────────────────────────────────────

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent), BlueprintType)
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ── Delegates ──────────────────────────────────────────────────────────
    UPROPERTY(BlueprintAssignable, Category = "NPC|Events")
    FNPC_OnBehaviorStateChanged OnBehaviorStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "NPC|Events")
    FNPC_OnThreatDetected OnThreatDetected;

    // ── State ───────────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|State")
    ENPC_BehaviorState CurrentBehaviorState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|State")
    ENPC_ThreatLevel CurrentThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|State")
    bool bIsLeader;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|State")
    bool bIsSleeping;

    // ── Configuration ───────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float MemoryRetentionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float PatrolRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float AlertRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float FleeRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float SocialRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    int32 MaxMemoryEntries;

    // ── Daily Routine ───────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Routine")
    TArray<FNPC_DailyRoutineSlot> DailyRoutine;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Routine")
    int32 CurrentRoutineIndex;

    // ── Memory ──────────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Memory")
    TArray<FNPC_MemoryEntry> ThreatMemory;

    // ── Blueprint Callable Interface ────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    ENPC_BehaviorState GetCurrentBehaviorState() const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    ENPC_ThreatLevel GetCurrentThreatLevel() const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void RecordThreat(AActor* ThreatActor, FVector LastKnownLocation, ENPC_ThreatLevel ThreatLvl);

    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void ForgetThreat(AActor* ThreatActor);

    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    bool HasMemoryOfThreat(AActor* ThreatActor) const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    FVector GetLastKnownThreatLocation() const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void ForceBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void SetAsLeader(bool bLeader);

    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    int32 GetMemoryCount() const;

private:
    UPROPERTY()
    APawn* OwnerPawn = nullptr;

    float StateTimeAccumulator = 0.0f;

    FTimerHandle RoutineTimerHandle;
    FTimerHandle MemoryCleanupTimerHandle;

    void TransitionToState(ENPC_BehaviorState NewState);
    void EvaluateThreatFromMemory();
    void ReactToThreat(AActor* ThreatActor, ENPC_ThreatLevel ThreatLvl);
    void BuildDefaultDailyRoutine();
    void AdvanceDailyRoutine();
    void PurgeStaleMemories();

    // Per-state tick helpers
    void TickIdle(float DeltaTime);
    void TickPatrol(float DeltaTime);
    void TickForage(float DeltaTime);
    void TickFlee(float DeltaTime);
    void TickAlert(float DeltaTime);
    void TickSocialise(float DeltaTime);
    void TickSleep(float DeltaTime);
};
