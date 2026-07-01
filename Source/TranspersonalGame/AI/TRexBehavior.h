#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "TRexBehavior.generated.h"

// ─── Enums ────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ENPC_TRexState : uint8
{
    Patrolling   UMETA(DisplayName = "Patrolling"),
    Investigating UMETA(DisplayName = "Investigating"),
    Chasing      UMETA(DisplayName = "Chasing"),
    Attacking    UMETA(DisplayName = "Attacking"),
    Roaring      UMETA(DisplayName = "Roaring"),
    Resting      UMETA(DisplayName = "Resting")
};

// ─── Structs ──────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FNPC_TRexConfig
{
    GENERATED_BODY()

    /** Radius of the patrol area around the home point */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Patrol")
    float PatrolRadius = 5000.0f;

    /** Distance at which T-Rex detects and starts chasing the player */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Detection")
    float ChaseDetectionRange = 3000.0f;

    /** Distance at which T-Rex performs a melee attack */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float AttackRange = 300.0f;

    /** Movement speed while patrolling */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Movement")
    float PatrolSpeed = 300.0f;

    /** Movement speed while chasing */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Movement")
    float ChaseSpeed = 900.0f;

    /** Damage dealt per attack hit */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float AttackDamage = 75.0f;

    /** Cooldown between attacks in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float AttackCooldown = 2.5f;

    /** Time before T-Rex gives up the chase and returns to patrol */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|AI")
    float ChaseGiveUpTime = 12.0f;

    /** Hearing radius — footsteps and loud noises */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Detection")
    float HearingRange = 1500.0f;
};

// ─── ATRexAIController ────────────────────────────────────────────────────────

UCLASS(ClassGroup = "TranspersonalGame|AI", meta = (DisplayName = "T-Rex AI Controller"))
class TRANSPERSONALGAME_API ATRexAIController : public AAIController
{
    GENERATED_BODY()

public:
    ATRexAIController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

    // ── State Machine ──────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "TRex|AI")
    ENPC_TRexState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "TRex|AI")
    void SetState(ENPC_TRexState NewState);

    UFUNCTION(BlueprintCallable, Category = "TRex|AI")
    void ForceRoar();

    // ── Config ─────────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Config")
    FNPC_TRexConfig Config;

    // ── Blackboard Keys ────────────────────────────────────────────────────

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TRex|Blackboard")
    FName BB_TargetActor = FName("TargetActor");

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TRex|Blackboard")
    FName BB_HomeLocation = FName("HomeLocation");

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TRex|Blackboard")
    FName BB_PatrolTarget = FName("PatrolTarget");

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TRex|Blackboard")
    FName BB_CurrentState = FName("CurrentState");

protected:
    // ── Perception ─────────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TRex|Perception",
              meta = (AllowPrivateAccess = "true"))
    UAIPerceptionComponent* PerceptionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TRex|Perception",
              meta = (AllowPrivateAccess = "true"))
    UAISenseConfig_Sight* SightConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TRex|Perception",
              meta = (AllowPrivateAccess = "true"))
    UAISenseConfig_Hearing* HearingConfig;

    UFUNCTION()
    void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // ── Internal State ─────────────────────────────────────────────────────

    ENPC_TRexState CurrentState;
    FVector HomeLocation;
    AActor* TargetActor;

    float TimeSinceLastAttack;
    float TimeSinceTargetSeen;
    float RoarTimer;

    // ── State Handlers ─────────────────────────────────────────────────────

    void TickPatrol(float DeltaTime);
    void TickChase(float DeltaTime);
    void TickAttack(float DeltaTime);
    void TickRoar(float DeltaTime);
    void TickResting(float DeltaTime);

    void PickNewPatrolPoint();
    bool CanSeeTarget() const;
    float DistanceToTarget() const;
    void ApplyMoveSpeed(float Speed);
};
