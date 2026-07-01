#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "TRexBehavior.generated.h"

// T-Rex behavioral states — drives all decision-making
UENUM(BlueprintType)
enum class ENPC_TRexState : uint8
{
    Patrolling    UMETA(DisplayName = "Patrolling"),
    Investigating UMETA(DisplayName = "Investigating"),
    Chasing       UMETA(DisplayName = "Chasing"),
    Attacking     UMETA(DisplayName = "Attacking"),
    Roaring       UMETA(DisplayName = "Roaring"),
    Resting       UMETA(DisplayName = "Resting")
};

// Tunable parameters for T-Rex behavior — exposed to Blueprint
USTRUCT(BlueprintType)
struct FNPC_TRexConfig
{
    GENERATED_BODY()

    // Patrol radius around spawn point (Unreal units)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Patrol")
    float PatrolRadius = 5000.0f;

    // Distance at which T-Rex detects and begins chasing the player
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Chase")
    float ChaseRange = 3000.0f;

    // Distance at which T-Rex transitions from chasing to attacking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Attack")
    float AttackRange = 300.0f;

    // Distance at which T-Rex gives up chase and returns to patrol
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Chase")
    float GiveUpRange = 6000.0f;

    // Movement speeds (cm/s)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Movement")
    float PatrolSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Movement")
    float ChaseSpeed = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Movement")
    float AttackSpeed = 1000.0f;

    // Damage per attack hit
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float AttackDamage = 75.0f;

    // Cooldown between attacks (seconds)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float AttackCooldown = 2.5f;

    // How long T-Rex investigates a sound/sight before returning to patrol
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Investigate")
    float InvestigateDuration = 8.0f;

    // How long T-Rex rests before resuming patrol
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Rest")
    float RestDuration = 30.0f;

    // Roar duration before initiating chase
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float RoarDuration = 2.0f;

    // Sight configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Perception")
    float SightRadius = 3500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Perception")
    float LoseSightRadius = 4500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Perception")
    float PeripheralVisionAngle = 60.0f;

    // Hearing radius
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Perception")
    float HearingRange = 2000.0f;
};

/**
 * ATRexAIController — AI Controller for the T-Rex apex predator.
 *
 * State machine:
 *   Resting → Patrolling → Investigating → Chasing → Roaring → Attacking
 *                                                  ↑___________________________↓ (loop)
 *
 * The T-Rex patrols a 5000-unit radius area around its spawn point.
 * When the player enters ChaseRange, it roars then chases.
 * When within AttackRange, it attacks every AttackCooldown seconds.
 * If the player escapes beyond GiveUpRange, it returns to patrol.
 */
UCLASS(ClassGroup = "TranspersonalGame|AI", meta = (DisplayName = "TRex AI Controller"))
class TRANSPERSONALGAME_API ATRexAIController : public AAIController
{
    GENERATED_BODY()

public:
    ATRexAIController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

    // --- State Access ---
    UFUNCTION(BlueprintCallable, Category = "TRex|State")
    ENPC_TRexState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "TRex|State")
    void ForceState(ENPC_TRexState NewState);

    // --- Config ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Config")
    FNPC_TRexConfig Config;

    // --- Perception events ---
    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

protected:
    // State machine transitions
    void TransitionTo(ENPC_TRexState NewState);
    void UpdateState(float DeltaTime);

    // Per-state tick logic
    void TickPatrolling(float DeltaTime);
    void TickInvestigating(float DeltaTime);
    void TickChasing(float DeltaTime);
    void TickAttacking(float DeltaTime);
    void TickRoaring(float DeltaTime);
    void TickResting(float DeltaTime);

    // Utility
    float GetDistanceToPlayer() const;
    bool HasLineOfSightToPlayer() const;
    FVector GetRandomPatrolPoint() const;
    void MoveToPatrolPoint();
    void ExecuteAttack();

    // Patrol waypoint management
    void SelectNextPatrolWaypoint();

private:
    UPROPERTY(VisibleAnywhere, Category = "TRex|State")
    ENPC_TRexState CurrentState;

    UPROPERTY(VisibleAnywhere, Category = "TRex|State")
    ENPC_TRexState PreviousState;

    // Perception component
    UPROPERTY(VisibleAnywhere, Category = "TRex|Perception")
    UAIPerceptionComponent* PerceptionComponent;

    UPROPERTY(VisibleAnywhere, Category = "TRex|Perception")
    UAISenseConfig_Sight* SightConfig;

    UPROPERTY(VisibleAnywhere, Category = "TRex|Perception")
    UAISenseConfig_Hearing* HearingConfig;

    // Target tracking
    UPROPERTY(VisibleAnywhere, Category = "TRex|Combat")
    AActor* CurrentTarget;

    // Spawn point — patrol origin
    FVector SpawnLocation;

    // Current patrol destination
    FVector CurrentPatrolDestination;

    // State timers
    float StateTimer;
    float AttackCooldownTimer;
    float InvestigateTimer;

    // Investigation point (last known player location or sound source)
    FVector InvestigationPoint;

    // Whether we've roared before this chase
    bool bHasRoaredThisChase;

    // Patrol waypoints (generated at BeginPlay)
    TArray<FVector> PatrolWaypoints;
    int32 CurrentWaypointIndex;
};
