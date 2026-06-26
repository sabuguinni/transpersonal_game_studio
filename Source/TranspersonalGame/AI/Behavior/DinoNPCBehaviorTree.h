#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "DinoNPCBehaviorTree.generated.h"

// ─── Blackboard Key Names ────────────────────────────────────────────────────
// "TargetActor"     — AActor* — the current chase/attack target
// "PatrolOrigin"    — FVector — home location for patrol radius
// "LastKnownPos"    — FVector — last seen position of target
// "AlertLevel"      — float   — 0=calm, 1=curious, 2=alert, 3=aggressive
// "bCanSeeTarget"   — bool    — perception result
// "bIsAttacking"    — bool    — currently in attack window
// "bIsDead"         — bool    — dead state guard
// "PatrolRadius"    — float   — radius around PatrolOrigin
// "AttackRange"     — float   — melee strike distance
// "ChaseRange"      — float   — max pursuit distance before giving up

UENUM(BlueprintType)
enum class ENPC_DinoAlertState : uint8
{
    Calm        UMETA(DisplayName = "Calm"),
    Curious     UMETA(DisplayName = "Curious"),
    Alert       UMETA(DisplayName = "Alert"),
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ENPC_DinoBehaviorMode : uint8
{
    Patrol      UMETA(DisplayName = "Patrol"),
    Idle        UMETA(DisplayName = "Idle"),
    Investigate UMETA(DisplayName = "Investigate"),
    Chase       UMETA(DisplayName = "Chase"),
    Attack      UMETA(DisplayName = "Attack"),
    Flee        UMETA(DisplayName = "Flee"),
    Feed        UMETA(DisplayName = "Feed")
};

USTRUCT(BlueprintType)
struct FNPC_DinoPerceptionData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Perception")
    float SightRadius = 3000.f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Perception")
    float LoseSightRadius = 4000.f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Perception")
    float PeripheralVisionAngle = 60.f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Perception")
    float HearingRange = 1500.f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Perception")
    float AlertDecayRate = 0.5f;   // alert units per second when no stimulus

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Perception")
    float AlertThresholdChase = 2.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Perception")
    float AlertThresholdAttack = 3.0f;
};

USTRUCT(BlueprintType)
struct FNPC_DinoCombatData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Combat")
    float AttackRange = 300.f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Combat")
    float AttackDamage = 40.f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Combat")
    float AttackCooldown = 2.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Combat")
    float ChaseSpeed = 800.f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Combat")
    float PatrolSpeed = 250.f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Combat")
    float MaxChaseDistance = 8000.f;
};

/**
 * ADinoNPCBehaviorTree
 * AI Controller for dinosaur NPCs.
 * Manages perception (sight + hearing), blackboard state, and drives
 * the Behavior Tree through alert-level transitions.
 *
 * Behavior flow:
 *   Calm → patrol radius around PatrolOrigin
 *   Curious → move to LastKnownPos, scan
 *   Alert → track target, raise alert
 *   Aggressive → chase + attack until target escapes MaxChaseDistance
 *   Dead → stop all behavior
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinoNPCBehaviorTree : public AAIController
{
    GENERATED_BODY()

public:
    ADinoNPCBehaviorTree();

    // ── Behavior Tree Asset ────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|BehaviorTree")
    UBehaviorTree* BehaviorTreeAsset;

    // ── Perception Config ──────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Perception")
    FNPC_DinoPerceptionData PerceptionData;

    // ── Combat Config ──────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Combat")
    FNPC_DinoCombatData CombatData;

    // ── Runtime State ─────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "NPC|State",
              meta = (AllowPrivateAccess = "true"))
    ENPC_DinoAlertState CurrentAlertState;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State",
              meta = (AllowPrivateAccess = "true"))
    ENPC_DinoBehaviorMode CurrentBehaviorMode;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State",
              meta = (AllowPrivateAccess = "true"))
    float CurrentAlertLevel;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State",
              meta = (AllowPrivateAccess = "true"))
    AActor* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State",
              meta = (AllowPrivateAccess = "true"))
    FVector PatrolOrigin;

    // ── Blackboard Keys ───────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Blackboard")
    FName BB_TargetActor = "TargetActor";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Blackboard")
    FName BB_PatrolOrigin = "PatrolOrigin";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Blackboard")
    FName BB_LastKnownPos = "LastKnownPos";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Blackboard")
    FName BB_AlertLevel = "AlertLevel";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Blackboard")
    FName BB_bCanSeeTarget = "bCanSeeTarget";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Blackboard")
    FName BB_bIsAttacking = "bIsAttacking";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Blackboard")
    FName BB_bIsDead = "bIsDead";

    // ── AAIController Overrides ────────────────────────────────────────────
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;
    virtual void Tick(float DeltaTime) override;

    // ── Perception Callbacks ───────────────────────────────────────────────
    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // ── Behavior API ──────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void SetAlertLevel(float NewLevel);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void ClearTarget();

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void TriggerDeath();

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void TriggerFlee(FVector FleeDirection);

    UFUNCTION(BlueprintPure, Category = "NPC|Behavior")
    bool IsTargetInAttackRange() const;

    UFUNCTION(BlueprintPure, Category = "NPC|Behavior")
    bool IsTargetInChaseRange() const;

    UFUNCTION(BlueprintPure, Category = "NPC|Behavior")
    ENPC_DinoAlertState GetAlertState() const { return CurrentAlertState; }

    UFUNCTION(BlueprintPure, Category = "NPC|Behavior")
    ENPC_DinoBehaviorMode GetBehaviorMode() const { return CurrentBehaviorMode; }

protected:
    // ── Internal ──────────────────────────────────────────────────────────
    UPROPERTY()
    UBehaviorTreeComponent* BehaviorTreeComp;

    UPROPERTY()
    UBlackboardComponent* BlackboardComp;

    UPROPERTY()
    UAIPerceptionComponent* PerceptionComp;

    UPROPERTY()
    UAISenseConfig_Sight* SightConfig;

    UPROPERTY()
    UAISenseConfig_Hearing* HearingConfig;

    void UpdateAlertState();
    void UpdateBlackboard();
    void DecayAlertLevel(float DeltaTime);
    void SetupPerception();
    ENPC_DinoAlertState AlertLevelToState(float Level) const;
};
