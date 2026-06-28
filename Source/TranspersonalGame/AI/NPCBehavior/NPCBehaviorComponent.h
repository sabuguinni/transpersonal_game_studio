// NPCBehaviorComponent.h
// NPC Behavior Agent #11 — PROD_CYCLE_AUTO_20260628_011
// Full NPC AI state machine: daily routines, alert levels, memory, reactions
// Prefix: ENPC_ for enums, FNPC_ for structs, UNPC_ for classes

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TimerManager.h"
#include "NPCBehaviorComponent.generated.h"

// Forward declarations
class ACharacter;
class AAIController;
class UBlackboardComponent;
class UNavigationSystemV1;

// ─── ENUMS ────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ENPC_DailyRoutinePhase : uint8
{
    Sleeping     UMETA(DisplayName = "Sleeping"),
    Waking       UMETA(DisplayName = "Waking"),
    Foraging     UMETA(DisplayName = "Foraging"),
    Resting      UMETA(DisplayName = "Resting"),
    Patrolling   UMETA(DisplayName = "Patrolling"),
    Investigating UMETA(DisplayName = "Investigating"),
    Fleeing      UMETA(DisplayName = "Fleeing"),
    Attacking    UMETA(DisplayName = "Attacking"),
    Dead         UMETA(DisplayName = "Dead"),
    Idle         UMETA(DisplayName = "Idle")
};

UENUM(BlueprintType)
enum class ENPC_AlertLevel : uint8
{
    Unaware    UMETA(DisplayName = "Unaware"),
    Curious    UMETA(DisplayName = "Curious"),
    Suspicious UMETA(DisplayName = "Suspicious"),
    Alert      UMETA(DisplayName = "Alert"),
    Combat     UMETA(DisplayName = "Combat"),
    Fleeing    UMETA(DisplayName = "Fleeing")
};

UENUM(BlueprintType)
enum class ENPC_ThreatType : uint8
{
    Sight    UMETA(DisplayName = "Sight"),
    Sound    UMETA(DisplayName = "Sound"),
    Damage   UMETA(DisplayName = "Damage"),
    Smell    UMETA(DisplayName = "Smell"),
    Vibration UMETA(DisplayName = "Vibration")
};

// ─── STRUCTS ──────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FNPC_ThreatMemory
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    AActor* ThreatActor = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    float ThreatMagnitude = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    ENPC_ThreatType ThreatType = ENPC_ThreatType::Sight;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    float TimeStamp = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    bool bIsActive = false;
};

// ─── DELEGATES ────────────────────────────────────────────────────────────────

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FNPC_OnAlertLevelChanged,
    ENPC_AlertLevel, OldLevel, ENPC_AlertLevel, NewLevel);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FNPC_OnRoutinePhaseChanged,
    ENPC_DailyRoutinePhase, OldPhase, ENPC_DailyRoutinePhase, NewPhase);

// ─── COMPONENT ────────────────────────────────────────────────────────────────

UCLASS(ClassGroup = (AI), meta = (BlueprintSpawnableComponent), DisplayName = "NPC Behavior Component")
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

    // ── Lifecycle ──────────────────────────────────────────────────────────────
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

    // ── Configuration ─────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    float PatrolRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Alert")
    float AlertDecayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    float MemoryRetentionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Perception")
    float HearingRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Perception")
    float SightRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Perception")
    float SightAngle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Combat")
    float FleeSpeedMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Combat")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    float InvestigateRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Debug")
    bool bDebugDraw;

    // ── State (read-only from Blueprint) ──────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "NPC|State",
        meta = (AllowPrivateAccess = "true"))
    ENPC_DailyRoutinePhase CurrentRoutinePhase;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State",
        meta = (AllowPrivateAccess = "true"))
    ENPC_AlertLevel CurrentAlertLevel;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State",
        meta = (AllowPrivateAccess = "true"))
    float AlertValue;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State",
        meta = (AllowPrivateAccess = "true"))
    bool bIsAlive;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory",
        meta = (AllowPrivateAccess = "true"))
    bool bHasThreatMemory;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory",
        meta = (AllowPrivateAccess = "true"))
    FVector LastKnownThreatLocation;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory",
        meta = (AllowPrivateAccess = "true"))
    TArray<FNPC_ThreatMemory> ThreatMemories;

    // ── Delegates ─────────────────────────────────────────────────────────────
    UPROPERTY(BlueprintAssignable, Category = "NPC|Events")
    FNPC_OnAlertLevelChanged OnAlertLevelChanged;

    UPROPERTY(BlueprintAssignable, Category = "NPC|Events")
    FNPC_OnRoutinePhaseChanged OnRoutinePhaseChanged;

    // ── Public API (callable from BT / Blueprint) ──────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "NPC|Perception")
    void OnHearNoise(AActor* NoiseInstigator, float Volume);

    UFUNCTION(BlueprintCallable, Category = "NPC|Perception")
    void OnSeeActor(AActor* SeenActor, float Confidence);

    UFUNCTION(BlueprintCallable, Category = "NPC|Combat")
    void OnTakeDamage(float DamageAmount, AActor* DamageInstigator);

    UFUNCTION(BlueprintCallable, Category = "NPC|State")
    void SetDead();

    UFUNCTION(BlueprintCallable, Category = "NPC|State")
    void SetRoutinePhase(ENPC_DailyRoutinePhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "NPC|State")
    void SetAlertLevel(ENPC_AlertLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "NPC|Threat")
    void RegisterThreat(AActor* ThreatActor, float ThreatMagnitude, ENPC_ThreatType ThreatType);

    UFUNCTION(BlueprintCallable, Category = "NPC|Threat")
    void ClearThreatMemory();

    UFUNCTION(BlueprintPure, Category = "NPC|State")
    bool IsInCombat() const;

    UFUNCTION(BlueprintPure, Category = "NPC|State")
    bool IsFleeing() const;

    UFUNCTION(BlueprintPure, Category = "NPC|State")
    float GetAlertValue() const;

    UFUNCTION(BlueprintPure, Category = "NPC|Navigation")
    FVector GetNextPatrolPoint() const;

    UFUNCTION(BlueprintPure, Category = "NPC|Navigation")
    FVector GetFleeDestination() const;

private:
    // Cached references
    ACharacter* OwnerCharacter;
    AAIController* OwnerController;
    UBlackboardComponent* OwnerBlackboard;

    // Home / spawn location
    FVector HomeLocation;

    // Threat memory
    float ThreatMemoryAge;

    // Timers
    FTimerHandle RoutineTimerHandle;

    // Internal methods
    void EvaluateDailyRoutine();
    void UpdateAlertLevel(float DeltaTime);
    ENPC_AlertLevel ComputeAlertLevel() const;
    void SyncToBlackboard();
    void DrawDebugState();
};
