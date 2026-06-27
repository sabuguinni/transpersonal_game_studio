// DinosaurCombatAIController.h
// Agent #12 — Combat & Enemy AI Agent
// Base combat AI controller for all dinosaur species.
// Provides shared combat logic: damage delivery, attack cooldowns, threat tracking, flee behaviour.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "DinosaurCombatAIController.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// Enums — prefixed Combat_ to avoid project-wide name collisions
// ─────────────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ECombat_DinoAIState : uint8
{
    Idle          UMETA(DisplayName = "Idle"),
    Patrolling    UMETA(DisplayName = "Patrolling"),
    Investigating UMETA(DisplayName = "Investigating"),
    Chasing       UMETA(DisplayName = "Chasing"),
    Attacking     UMETA(DisplayName = "Attacking"),
    Cooldown      UMETA(DisplayName = "Cooldown"),
    Fleeing       UMETA(DisplayName = "Fleeing"),
    Returning     UMETA(DisplayName = "Returning"),
    Dead          UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ECombat_DinoAggression : uint8
{
    Passive     UMETA(DisplayName = "Passive"),       // Brachiosaurus — never attacks
    Defensive   UMETA(DisplayName = "Defensive"),     // Triceratops — attacks if threatened
    Territorial UMETA(DisplayName = "Territorial"),   // T-Rex — attacks in patrol zone
    Aggressive  UMETA(DisplayName = "Aggressive"),    // Raptor — actively hunts
    PackHunter  UMETA(DisplayName = "PackHunter")     // Raptor pack — coordinates with others
};

// ─────────────────────────────────────────────────────────────────────────────
// Structs
// ─────────────────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FCombat_AttackProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float BaseDamage = 40.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float AttackRange = 250.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float AttackCooldownSeconds = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float KnockbackForce = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    bool bCanInterruptPlayer = true;

    FCombat_AttackProfile()
        : BaseDamage(40.0f), AttackRange(250.0f), AttackCooldownSeconds(2.0f),
          KnockbackForce(800.0f), bCanInterruptPlayer(true) {}
};

USTRUCT(BlueprintType)
struct FCombat_PerceptionConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Perception")
    float SightRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Perception")
    float LoseSightRadius = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Perception")
    float PeripheralVisionAngle = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Perception")
    float HearingRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Perception")
    float ChaseRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Perception")
    float PatrolRadius = 3000.0f;

    FCombat_PerceptionConfig()
        : SightRadius(2000.0f), LoseSightRadius(2500.0f), PeripheralVisionAngle(60.0f),
          HearingRadius(1500.0f), ChaseRange(2000.0f), PatrolRadius(3000.0f) {}
};

// ─────────────────────────────────────────────────────────────────────────────
// ADinosaurCombatAIController — Base class for all dinosaur combat AI
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(ClassGroup = "CombatAI", meta = (DisplayName = "Dinosaur Combat AI Controller"))
class TRANSPERSONALGAME_API ADinosaurCombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurCombatAIController();

    // ── Lifecycle ──────────────────────────────────────────────────────────
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

    // ── Combat State ───────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat|State")
    void SetCombatState(ECombat_DinoAIState NewState);

    UFUNCTION(BlueprintPure, Category = "Combat|State")
    ECombat_DinoAIState GetCombatState() const { return CurrentState; }

    UFUNCTION(BlueprintPure, Category = "Combat|State")
    bool IsInCombat() const;

    // ── Attack Logic ───────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat|Attack")
    void TryExecuteAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat|Attack")
    void DeliverDamage(AActor* Target);

    UFUNCTION(BlueprintPure, Category = "Combat|Attack")
    bool IsAttackOnCooldown() const;

    UFUNCTION(BlueprintPure, Category = "Combat|Attack")
    float GetAttackCooldownRemaining() const;

    // ── Threat & Target ────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat|Threat")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintPure, Category = "Combat|Threat")
    AActor* GetCurrentTarget() const { return CurrentTarget; }

    UFUNCTION(BlueprintCallable, Category = "Combat|Threat")
    void ClearTarget();

    UFUNCTION(BlueprintPure, Category = "Combat|Threat")
    float GetDistanceToTarget() const;

    // ── Flee Logic ─────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat|Flee")
    void EvaluateFlee(float CurrentHealthPercent);

    // ── Configuration ──────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    FCombat_AttackProfile AttackProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    FCombat_PerceptionConfig PerceptionConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    ECombat_DinoAggression AggressionType = ECombat_DinoAggression::Territorial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    float FleeHealthThresholdPercent = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    float ChaseAbandonMultiplier = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|BehaviorTree")
    UBehaviorTree* BehaviorTreeAsset = nullptr;

protected:
    // ── Internal State ─────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Combat|State", meta = (AllowPrivateAccess = "true"))
    ECombat_DinoAIState CurrentState = ECombat_DinoAIState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State", meta = (AllowPrivateAccess = "true"))
    AActor* CurrentTarget = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State", meta = (AllowPrivateAccess = "true"))
    FVector HomeLocation = FVector::ZeroVector;

    float LastAttackTime = 0.0f;
    float StateEnteredTime = 0.0f;

    // ── Perception ─────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Perception", meta = (AllowPrivateAccess = "true"))
    UAIPerceptionComponent* PerceptionComponent = nullptr;

    UFUNCTION()
    virtual void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // ── State Tick Methods ─────────────────────────────────────────────────
    virtual void TickIdle(float DeltaTime);
    virtual void TickChase(float DeltaTime);
    virtual void TickAttack(float DeltaTime);
    virtual void TickCooldown(float DeltaTime);
    virtual void TickFlee(float DeltaTime);
    virtual void TickReturn(float DeltaTime);

    // ── Blackboard Keys ────────────────────────────────────────────────────
    static const FName BB_TargetActor;
    static const FName BB_AIState;
    static const FName BB_HomeLocation;
    static const FName BB_IsInCombat;
};
