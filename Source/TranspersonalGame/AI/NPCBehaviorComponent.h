// NPCBehaviorComponent.h
// NPC Behavior Agent #11 — Transpersonal Game Studio
// Universal NPC behavior component: patrol, investigate, flee, social routines
// Attach to any ACharacter/APawn to give it autonomous daily-life behavior

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "NPCBehaviorComponent.generated.h"

// ─── Enums (global scope, NPC_ prefix) ───────────────────────────────────────

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Investigate UMETA(DisplayName = "Investigate"),
    Alert       UMETA(DisplayName = "Alert"),
    Flee        UMETA(DisplayName = "Flee"),
    Interact    UMETA(DisplayName = "Interact"),
    Rest        UMETA(DisplayName = "Rest"),
    Feed        UMETA(DisplayName = "Feed"),
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

UENUM(BlueprintType)
enum class ENPC_DailyPhase : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

// ─── Structs (global scope) ──────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FNPC_ThreatMemory
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    AActor* ThreatActor = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float ThreatIntensity = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float TimeLastSeen = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    ENPC_ThreatLevel ThreatLevel = ENPC_ThreatLevel::None;
};

USTRUCT(BlueprintType)
struct FNPC_PatrolConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    TArray<FVector> WaypointLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    float WaypointAcceptRadius = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    float WaypointWaitTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    bool bLoopPatrol = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    bool bRandomizeOrder = false;
};

USTRUCT(BlueprintType)
struct FNPC_SensoryConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Senses")
    float SightRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Senses")
    float SightAngleDegrees = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Senses")
    float HearingRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Senses")
    float SmellRange = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Senses")
    float AlertDecayRate = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Senses")
    float MemoryDuration = 30.0f;
};

// ─── Component ───────────────────────────────────────────────────────────────

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

    // ── State ─────────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|State")
    ENPC_BehaviorState CurrentState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|State")
    ENPC_ThreatLevel CurrentThreatLevel;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|State")
    ENPC_DailyPhase CurrentDailyPhase;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|State")
    float AlertLevel;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|State")
    bool bIsAwareOfPlayer;

    // ── Config ────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    FNPC_PatrolConfig PatrolConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    FNPC_SensoryConfig SensoryConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float FleeSpeedMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float AlertThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float FleeThreshold;

    // ── Memory ────────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Memory")
    TArray<FNPC_ThreatMemory> ThreatMemories;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Memory")
    FVector LastKnownPlayerLocation;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Memory")
    float TimeSincePlayerSeen;

    // ── Public API ────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void RegisterThreat(AActor* ThreatActor, float Intensity, ENPC_ThreatLevel Level);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void ClearThreat(AActor* ThreatActor);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void UpdateDailyPhase(ENPC_DailyPhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    bool CanSeeActor(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    bool CanHearActor(AActor* Target, float NoiseLevel = 1.0f) const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    ENPC_ThreatLevel EvaluateThreatLevel() const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    FVector GetNextPatrolWaypoint();

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void AdvancePatrolWaypoint();

    UFUNCTION(BlueprintPure, Category = "NPC|State")
    bool IsInState(ENPC_BehaviorState State) const { return CurrentState == State; }

    UFUNCTION(BlueprintPure, Category = "NPC|State")
    float GetAlertLevel() const { return AlertLevel; }

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    int32 CurrentWaypointIndex;
    float StateTimer;
    float ThreatDecayTimer;

    void TickAlertDecay(float DeltaTime);
    void TickThreatMemories(float DeltaTime);
    void EvaluateStateTransition();
    void OnStateEnter(ENPC_BehaviorState NewState);
    void OnStateExit(ENPC_BehaviorState OldState);
};
