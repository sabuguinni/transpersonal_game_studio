#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "TRexBehavior.generated.h"

/** T-Rex AI state enum — drives behavior tree selection */
UENUM(BlueprintType)
enum class ENPC_TRexState : uint8
{
    Patrol     UMETA(DisplayName = "Patrol"),
    Alert      UMETA(DisplayName = "Alert"),
    Chase      UMETA(DisplayName = "Chase"),
    Attack     UMETA(DisplayName = "Attack"),
    Roar       UMETA(DisplayName = "Roar"),
    Rest       UMETA(DisplayName = "Rest")
};

/** Patrol waypoint data */
USTRUCT(BlueprintType)
struct FNPC_TRexPatrolPoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Patrol")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Patrol")
    float WaitTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Patrol")
    bool bRoarOnArrival = false;
};

/**
 * ATRexBehaviorController — AI Controller for the T-Rex
 *
 * Behavior logic:
 *   PATROL  : Moves between waypoints in a 5000-unit radius area
 *   ALERT   : Stops, looks toward detected stimulus
 *   CHASE   : Pursues player when within 3000 units (sight range)
 *   ATTACK  : Melee attack when within 300 units
 *   ROAR    : Intimidation roar on first detection or kill
 *   REST    : Idle at rest point (night cycle / post-feed)
 */
UCLASS(ClassGroup = "TranspersonalGame|AI", meta = (DisplayName = "TRex Behavior Controller"))
class TRANSPERSONALGAME_API ATRexBehaviorController : public AAIController
{
    GENERATED_BODY()

public:
    ATRexBehaviorController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

    // ── Behavior State ──────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "TRex|AI")
    ENPC_TRexState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "TRex|AI")
    void SetState(ENPC_TRexState NewState);

    // ── Patrol ──────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Patrol")
    TArray<FNPC_TRexPatrolPoint> PatrolWaypoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Patrol")
    float PatrolRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Patrol")
    float PatrolMoveSpeed = 300.0f;

    // ── Detection ───────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Detection")
    float SightRange = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Detection")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Detection")
    float SightAngleDegrees = 120.0f;

    // ── Combat ──────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float AttackDamage = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float AttackCooldown = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float ChaseMoveSpeed = 700.0f;

    // ── Blackboard Keys ─────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Blackboard")
    FName BB_TargetActor = FName("TargetActor");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Blackboard")
    FName BB_PatrolLocation = FName("PatrolLocation");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Blackboard")
    FName BB_AIState = FName("AIState");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Blackboard")
    FName BB_DistanceToTarget = FName("DistanceToTarget");

protected:
    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    void UpdatePatrol(float DeltaTime);
    void UpdateChase(float DeltaTime);
    void UpdateAttack(float DeltaTime);
    void TryDetectPlayer();
    bool IsPlayerInSightCone(AActor* Player) const;
    void AdvancePatrolWaypoint();

private:
    UPROPERTY()
    UAIPerceptionComponent* PerceptionComponent = nullptr;

    UPROPERTY()
    UAISenseConfig_Sight* SightConfig = nullptr;

    ENPC_TRexState CurrentState = ENPC_TRexState::Patrol;

    int32 CurrentWaypointIndex = 0;
    float AttackCooldownTimer = 0.0f;
    float WaypointWaitTimer = 0.0f;
    bool bWaitingAtWaypoint = false;

    UPROPERTY()
    AActor* CurrentTarget = nullptr;
};
