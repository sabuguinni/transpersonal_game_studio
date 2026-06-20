#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NPCBehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Alert       UMETA(DisplayName = "Alert"),
    Flee        UMETA(DisplayName = "Flee"),
    Attack      UMETA(DisplayName = "Attack"),
    Graze       UMETA(DisplayName = "Graze"),
    Rest        UMETA(DisplayName = "Rest"),
    Investigate UMETA(DisplayName = "Investigate")
};

UENUM(BlueprintType)
enum class ENPC_DinoSpecies : uint8
{
    TRex         UMETA(DisplayName = "T-Rex"),
    Raptor       UMETA(DisplayName = "Raptor"),
    Brachiosaurus UMETA(DisplayName = "Brachiosaurus"),
    Triceratops  UMETA(DisplayName = "Triceratops"),
    Pterodactyl  UMETA(DisplayName = "Pterodactyl")
};

USTRUCT(BlueprintType)
struct FNPC_StimulusEvent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Stimulus")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Stimulus")
    float Intensity = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Stimulus")
    float Timestamp = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Stimulus")
    bool bIsVisual = false;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Stimulus")
    bool bIsAuditory = false;
};

USTRUCT(BlueprintType)
struct FNPC_MemoryRecord
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    FVector LastKnownPlayerLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float TimeSinceLastSighting = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float ThreatLevel = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    bool bPlayerKnown = false;
};

/**
 * NPCBehaviorComponent
 * Drives NPC state machine: patrol, alert, flee, attack, graze, rest.
 * Designed for dinosaur NPCs in the prehistoric survival world.
 * Integrates with UE5 AI Perception and Behavior Trees.
 */
UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent), DisplayName = "NPC Behavior Component")
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ── State Machine ──────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintPure, Category = "NPC|Behavior")
    ENPC_BehaviorState GetBehaviorState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void OnStimulusReceived(const FNPC_StimulusEvent& Stimulus);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void OnPlayerDetected(FVector PlayerLocation, float Distance);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void OnPlayerLost();

    // ── Memory ─────────────────────────────────────────────────────────────────

    UFUNCTION(BlueprintPure, Category = "NPC|Memory")
    FNPC_MemoryRecord GetMemoryRecord() const { return Memory; }

    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void ForgetPlayer();

    // ── Species Config ─────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Species")
    ENPC_DinoSpecies Species = ENPC_DinoSpecies::Raptor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Species")
    float DetectionRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Species")
    float AttackRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Species")
    float FleeHealthThreshold = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Species")
    float PatrolRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Species")
    float MemoryDecayRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Species")
    bool bIsHerbivore = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Species")
    bool bIsPack = false;

    // ── Runtime State ──────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State", meta = (AllowPrivateAccess = "true"))
    ENPC_BehaviorState CurrentState = ENPC_BehaviorState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
    float CurrentThreatLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
    float CurrentHealth = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
    FVector PatrolOrigin = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
    FVector CurrentPatrolTarget = FVector::ZeroVector;

private:
    FNPC_MemoryRecord Memory;

    float StateTimer = 0.0f;
    float PatrolWaitTimer = 0.0f;
    bool bPatrolWaiting = false;

    void UpdateIdle(float DeltaTime);
    void UpdatePatrol(float DeltaTime);
    void UpdateAlert(float DeltaTime);
    void UpdateFlee(float DeltaTime);
    void UpdateAttack(float DeltaTime);
    void UpdateGraze(float DeltaTime);
    void UpdateRest(float DeltaTime);
    void UpdateMemoryDecay(float DeltaTime);
    void ChooseNewPatrolTarget();
    bool IsPlayerInAttackRange() const;
    bool ShouldFlee() const;
    void ApplySpeciesDefaults();
};
