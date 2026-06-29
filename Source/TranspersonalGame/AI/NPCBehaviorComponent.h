#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NPCBehaviorComponent.generated.h"

// ============================================================
// NPC Behavior Component — Agent #11
// Provides behavioral state machine, memory, and daily routine
// for all non-player characters in the prehistoric survival world.
// ============================================================

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
    Dawn    UMETA(DisplayName = "Dawn"),
    Morning UMETA(DisplayName = "Morning"),
    Midday  UMETA(DisplayName = "Midday"),
    Evening UMETA(DisplayName = "Evening"),
    Night   UMETA(DisplayName = "Night")
};

USTRUCT(BlueprintType)
struct FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    AActor* SourceActor = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float Timestamp = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float ThreatWeight = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    bool bIsActive = true;
};

USTRUCT(BlueprintType)
struct FNPC_DailyRoutineSlot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Routine")
    ENPC_TimeOfDay TimeSlot = ENPC_TimeOfDay::Morning;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Routine")
    ENPC_BehaviorState TargetBehavior = ENPC_BehaviorState::Patrol;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Routine")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Routine")
    float Duration = 60.0f;
};

UCLASS(ClassGroup = (AI), meta = (BlueprintSpawnableComponent))
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
    void AddMemoryEntry(AActor* Source, FVector Location, float ThreatWeight);

    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void DecayMemories(float DeltaTime);

    UFUNCTION(BlueprintPure, Category = "NPC|Memory")
    FNPC_MemoryEntry GetMostRecentThreat() const;

    UFUNCTION(BlueprintPure, Category = "NPC|Memory")
    bool HasActiveMemories() const;

    // ── Threat Assessment ──────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "NPC|Threat")
    void UpdateThreatLevel();

    UFUNCTION(BlueprintPure, Category = "NPC|Threat")
    ENPC_ThreatLevel GetThreatLevel() const { return CurrentThreatLevel; }

    UFUNCTION(BlueprintPure, Category = "NPC|Threat")
    float GetAggregatedThreatScore() const;

    // ── Daily Routine ──────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "NPC|Routine")
    void AddRoutineSlot(FNPC_DailyRoutineSlot Slot);

    UFUNCTION(BlueprintCallable, Category = "NPC|Routine")
    void EvaluateDailyRoutine(ENPC_TimeOfDay CurrentTime);

    // ── Patrol ─────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    void SetPatrolPoints(const TArray<FVector>& Points);

    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    FVector GetNextPatrolPoint();

    UFUNCTION(BlueprintPure, Category = "NPC|Patrol")
    bool HasReachedCurrentPatrolPoint() const;

    // ── Survival Stats ─────────────────────────────────────────
    UPROPERTY(BlueprintReadWrite, Category = "NPC|Survival")
    float Hunger = 1.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Survival")
    float Thirst = 1.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Survival")
    float Fatigue = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Survival")
    float Fear = 0.0f;

    // ── Config ─────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float MemoryDecayRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float MaxMemoryAge = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float FleeThreshold = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float InvestigateThreshold = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float PatrolAcceptanceRadius = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float SurvivalNeedDecayRate = 0.005f;

private:
    UPROPERTY()
    ENPC_BehaviorState CurrentState = ENPC_BehaviorState::Idle;

    UPROPERTY()
    ENPC_ThreatLevel CurrentThreatLevel = ENPC_ThreatLevel::None;

    UPROPERTY()
    TArray<FNPC_MemoryEntry> MemoryEntries;

    UPROPERTY()
    TArray<FNPC_DailyRoutineSlot> DailyRoutine;

    UPROPERTY()
    TArray<FVector> PatrolPoints;

    int32 CurrentPatrolIndex = 0;
    float StateTimer = 0.0f;
    float MemoryDecayAccumulator = 0.0f;

    void TickSurvivalNeeds(float DeltaTime);
    void TickStateTimer(float DeltaTime);
};
