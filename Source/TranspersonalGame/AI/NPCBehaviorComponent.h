// NPCBehaviorComponent.h
// NPC Behavior Agent #11 — Transpersonal Game Studio
// Defines behavior state machine, threat response, memory, and daily routine for primitive human NPCs.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NPCBehaviorComponent.generated.h"

// ─── ENUMS (global scope — UHT requirement) ─────────────────────────────────

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle         UMETA(DisplayName = "Idle"),
    Patrol       UMETA(DisplayName = "Patrol"),
    Gather       UMETA(DisplayName = "Gather"),
    Flee         UMETA(DisplayName = "Flee"),
    Seek         UMETA(DisplayName = "Seek"),
    Investigate  UMETA(DisplayName = "Investigate"),
    Interact     UMETA(DisplayName = "Interact"),
    Dead         UMETA(DisplayName = "Dead")
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
    Dawn    UMETA(DisplayName = "Dawn"),
    Morning UMETA(DisplayName = "Morning"),
    Midday  UMETA(DisplayName = "Midday"),
    Dusk    UMETA(DisplayName = "Dusk"),
    Night   UMETA(DisplayName = "Night")
};

// ─── DELEGATES ──────────────────────────────────────────────────────────────

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNPC_OnBehaviorStateChanged, ENPC_BehaviorState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNPC_OnThreatLevelChanged, ENPC_ThreatLevel, NewThreatLevel);

// ─── COMPONENT CLASS ────────────────────────────────────────────────────────

UCLASS(ClassGroup = (AI), meta = (BlueprintSpawnableComponent), DisplayName = "NPC Behavior Component")
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

    // ── Delegates ────────────────────────────────────────────────────────────
    UPROPERTY(BlueprintAssignable, Category = "NPC|Events")
    FNPC_OnBehaviorStateChanged OnBehaviorStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "NPC|Events")
    FNPC_OnThreatLevelChanged OnThreatLevelChanged;

    // ── State ─────────────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|State")
    ENPC_BehaviorState CurrentBehaviorState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|State")
    ENPC_ThreatLevel CurrentThreatLevel;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|State")
    ENPC_TimeOfDay CurrentTimeOfDay;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|State")
    bool bIsAlive;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|State")
    bool bHasActiveTarget;

    // ── Configuration ─────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float PatrolRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float FleeSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float PatrolSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float DetectionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float MemoryDuration;

    // ── Patrol Waypoints ──────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    TArray<FVector> PatrolWaypoints;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Patrol")
    FVector HomeLocation;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Patrol")
    FVector CurrentPatrolTarget;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Patrol")
    FVector LastKnownThreatLocation;

    // ── Public API ────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void TransitionToState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void RegisterThreat(AActor* ThreatActor, float Distance);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void ClearThreatMemory();

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void SetTimeOfDay(ENPC_TimeOfDay NewTime);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void KillNPC();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    float StateTimer;
    float ThreatMemoryTimer;
    int32 PatrolWaypointIndex;

    FTimerHandle RoutineTimerHandle;

    void TickIdle(float DeltaTime);
    void TickPatrol(float DeltaTime);
    void TickGather(float DeltaTime);
    void TickFlee(float DeltaTime);
    void TickSeek(float DeltaTime);
    void TickInvestigate(float DeltaTime);
    void TickInteract(float DeltaTime);

    void ScanForThreats();
    void AdvancePatrolWaypoint();
    void EvaluateDailyRoutine();
};
