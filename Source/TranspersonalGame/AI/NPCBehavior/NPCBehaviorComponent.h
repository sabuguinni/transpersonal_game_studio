#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "NPCBehaviorComponent.generated.h"

// ============================================================
// NPC_DailyRoutinePhase — phases of a prehistoric NPC's day
// ============================================================
UENUM(BlueprintType)
enum class ENPC_DailyRoutinePhase : uint8
{
    Sleeping        UMETA(DisplayName = "Sleeping"),
    Waking          UMETA(DisplayName = "Waking"),
    Foraging        UMETA(DisplayName = "Foraging"),
    Resting         UMETA(DisplayName = "Resting"),
    Patrolling      UMETA(DisplayName = "Patrolling"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Investigating   UMETA(DisplayName = "Investigating"),
    Attacking       UMETA(DisplayName = "Attacking"),
    Dead            UMETA(DisplayName = "Dead")
};

// ============================================================
// NPC_AlertLevel — how aware the NPC is of threats
// ============================================================
UENUM(BlueprintType)
enum class ENPC_AlertLevel : uint8
{
    Unaware     UMETA(DisplayName = "Unaware"),
    Curious     UMETA(DisplayName = "Curious"),
    Suspicious  UMETA(DisplayName = "Suspicious"),
    Alert       UMETA(DisplayName = "Alert"),
    Combat      UMETA(DisplayName = "Combat"),
    Fleeing     UMETA(DisplayName = "Fleeing")
};

// ============================================================
// NPC_MemoryEntry — a single remembered event
// ============================================================
USTRUCT(BlueprintType)
struct FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    float TimeStamp = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    float ThreatLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    bool bIsPlayerRelated = false;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    FString EventDescription;
};

// ============================================================
// NPC_PatrolPoint — a waypoint in a patrol route
// ============================================================
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

// ============================================================
// UNPCBehaviorComponent — core NPC AI state machine
// ============================================================
UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent), DisplayName = "NPC Behavior Component")
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ── Routine ──────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Routine")
    ENPC_DailyRoutinePhase CurrentPhase = ENPC_DailyRoutinePhase::Patrolling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Routine")
    TArray<FNPC_PatrolPoint> PatrolRoute;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Routine")
    float PatrolRadius = 2000.0f;

    // ── Alert ─────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "NPC|Alert")
    ENPC_AlertLevel AlertLevel = ENPC_AlertLevel::Unaware;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Alert")
    float DetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Alert")
    float FleeRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Alert")
    float AlertDecayRate = 0.5f;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Alert")
    float CurrentAlertValue = 0.0f;

    // ── Memory ────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    TArray<FNPC_MemoryEntry> MemoryLog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    int32 MaxMemoryEntries = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    float MemoryDecayTime = 60.0f;

    // ── Survival Stats ────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Survival")
    float Hunger = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Survival")
    float Thirst = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Survival")
    float Fear = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Survival")
    float HungerDrainRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Survival")
    float ThirstDrainRate = 1.5f;

    // ── Debug ─────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Debug")
    bool bDrawDebug = false;

    // ── Public API ────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "NPC|Alert")
    void ReceiveThreat(FVector ThreatLocation, float ThreatLevel, bool bIsPlayer);

    UFUNCTION(BlueprintCallable, Category = "NPC|Routine")
    void SetPhase(ENPC_DailyRoutinePhase NewPhase);

    UFUNCTION(BlueprintPure, Category = "NPC|Alert")
    ENPC_AlertLevel GetAlertLevel() const { return AlertLevel; }

    UFUNCTION(BlueprintPure, Category = "NPC|Routine")
    ENPC_DailyRoutinePhase GetCurrentPhase() const { return CurrentPhase; }

    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void AddMemoryEntry(FVector Location, float ThreatLevel, bool bIsPlayer, const FString& Description);

    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void ClearOldMemories();

    UFUNCTION(BlueprintPure, Category = "NPC|Survival")
    bool IsHungry() const { return Hunger < 30.0f; }

    UFUNCTION(BlueprintPure, Category = "NPC|Survival")
    bool IsThirsty() const { return Thirst < 30.0f; }

    UFUNCTION(BlueprintPure, Category = "NPC|Survival")
    bool IsFleeing() const { return CurrentPhase == ENPC_DailyRoutinePhase::Fleeing; }

private:
    void UpdateAlertLevel(float DeltaTime);
    void UpdateSurvivalStats(float DeltaTime);
    void UpdateRoutineFromStats();
    void EvaluateThreatFromMemory();

    int32 CurrentPatrolIndex = 0;
    float PhaseTimer = 0.0f;
    float WorldTime = 0.0f;

    UPROPERTY()
    AActor* OwnerActor = nullptr;
};
