// NPCBehaviorComponent.h
// Agent #11 — NPC Behavior Agent
// Cycle: PROD_CYCLE_AUTO_20260628_003
// Attaches to any ACharacter-based NPC. Drives patrol, detection, memory, and daily routine.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "NPCBehaviorComponent.generated.h"

// ─── NPC-specific enums (NPC_ prefix — unique project-wide) ─────────────────

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Investigating UMETA(DisplayName = "Investigating"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ENPC_DailyRoutinePhase : uint8
{
    Dawn        UMETA(DisplayName = "Dawn — foraging"),
    Midday      UMETA(DisplayName = "Midday — shelter"),
    Dusk        UMETA(DisplayName = "Dusk — social"),
    Night       UMETA(DisplayName = "Night — sleep")
};

UENUM(BlueprintType)
enum class ENPC_ThreatTier : uint8
{
    None        UMETA(DisplayName = "No threat"),
    Noise       UMETA(DisplayName = "Noise detected"),
    Sight       UMETA(DisplayName = "Visual contact"),
    Contact     UMETA(DisplayName = "Direct contact")
};

// ─── Memory entry — what this NPC remembers about a stimulus ────────────────

USTRUCT(BlueprintType)
struct FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    AActor* SourceActor = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float TimeStamp = 0.0f;

    // How alarming was this stimulus (0-1)
    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float ThreatWeight = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    ENPC_ThreatTier ThreatTier = ENPC_ThreatTier::None;

    // Memory decays over time — fades to 0 after MemoryDecaySeconds
    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    bool bStillValid = true;
};

// ─── Patrol waypoint ────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FNPC_PatrolWaypoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    FVector WorldLocation = FVector::ZeroVector;

    // How long to wait at this waypoint (seconds)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    float WaitDuration = 2.0f;

    // Optional: look-at direction while waiting
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    FRotator LookAtRotation = FRotator::ZeroRotator;
};

// ─── Main component ─────────────────────────────────────────────────────────

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    // ── State ────────────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
    ENPC_BehaviorState CurrentState = ENPC_BehaviorState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
    ENPC_DailyRoutinePhase CurrentRoutinePhase = ENPC_DailyRoutinePhase::Dawn;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
    float AlertLevel = 0.0f;   // 0 = calm, 1 = maximum alert

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
    float FearLevel = 0.0f;    // fed to AnimInstance for visual state

    // ── Patrol config ────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    TArray<FNPC_PatrolWaypoint> PatrolWaypoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    float PatrolRadius = 1500.0f;   // auto-generate radius if no waypoints set

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    float PatrolMoveSpeed = 300.0f;

    // ── Detection config ─────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Detection")
    float SightRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Detection")
    float SightAngleDegrees = 90.0f;   // half-angle cone

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Detection")
    float HearingRange = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Detection")
    float FleeDistance = 3000.0f;      // flee if threat within this range

    // ── Memory config ────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    float MemoryDecaySeconds = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    int32 MaxMemoryEntries = 8;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    TArray<FNPC_MemoryEntry> MemoryBank;

    // ── Public API ───────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void RegisterStimulus(AActor* Source, ENPC_ThreatTier Tier, float Weight);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void SetDailyPhase(ENPC_DailyRoutinePhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void ForceFlee(FVector ThreatLocation);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    float GetAlertLevel() const { return AlertLevel; }

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    ENPC_BehaviorState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    bool HasMemoryOf(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    FVector GetLastKnownLocationOf(AActor* Actor) const;

private:
    void TickPatrol(float DeltaTime);
    void TickDetection(float DeltaTime);
    void TickMemoryDecay(float DeltaTime);
    void TickAlertDecay(float DeltaTime);
    void TransitionToState(ENPC_BehaviorState NewState);
    bool CanSeeActor(AActor* Target) const;
    void AddMemoryEntry(FNPC_MemoryEntry Entry);

    int32 CurrentWaypointIndex = 0;
    float WaypointWaitTimer = 0.0f;
    bool bWaitingAtWaypoint = false;
    FVector HomeLocation = FVector::ZeroVector;

    // Cached owner as ACharacter
    class ACharacter* OwnerCharacter = nullptr;
};
