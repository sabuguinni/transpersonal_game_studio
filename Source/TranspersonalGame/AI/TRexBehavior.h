// TRexBehavior.h
// Agent #11 — NPC Behavior Agent
// T-Rex AI Controller: patrol 5000u, chase 3000u, attack 300u

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIStimulus.h"
#include "Navigation/PathFollowingComponent.h"
#include "TRexBehavior.generated.h"

// ============================================================
// T-Rex AI State Enum
// ============================================================
UENUM(BlueprintType)
enum class ENPC_TRexState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Chasing     UMETA(DisplayName = "Chasing"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Returning   UMETA(DisplayName = "Returning")
};

// ============================================================
// ATRexAIController
// ============================================================
UCLASS(ClassGroup = "TranspersonalGame|AI", meta = (DisplayName = "T-Rex AI Controller"))
class TRANSPERSONALGAME_API ATRexAIController : public AAIController
{
    GENERATED_BODY()

public:
    ATRexAIController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // --------------------------------------------------------
    // Behavior Tree asset (assign in BP)
    // --------------------------------------------------------
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|BehaviorTree")
    UBehaviorTree* TRexBehaviorTree;

    // --------------------------------------------------------
    // Tuning parameters
    // --------------------------------------------------------
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    float PatrolRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    float PatrolWaitTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Combat")
    float ChaseRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Combat")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Movement")
    float PatrolSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Movement")
    float ChaseSpeed;

    // --------------------------------------------------------
    // Runtime state
    // --------------------------------------------------------
    UPROPERTY(BlueprintReadOnly, Category = "NPC|State", meta = (AllowPrivateAccess = "true"))
    ENPC_TRexState CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State", meta = (AllowPrivateAccess = "true"))
    AActor* TargetActor;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State", meta = (AllowPrivateAccess = "true"))
    FVector HomeLocation;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State", meta = (AllowPrivateAccess = "true"))
    FVector CurrentPatrolTarget;

protected:
    // --------------------------------------------------------
    // Internal components
    // --------------------------------------------------------
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Components", meta = (AllowPrivateAccess = "true"))
    UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Components", meta = (AllowPrivateAccess = "true"))
    UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Components", meta = (AllowPrivateAccess = "true"))
    UAIPerceptionComponent* PerceptionComponent;

    // --------------------------------------------------------
    // State machine ticks
    // --------------------------------------------------------
    void TickIdle(float DeltaTime);
    void TickPatrol(float DeltaTime);
    void TickChase(float DeltaTime);
    void TickAttack(float DeltaTime);
    void TickReturn(float DeltaTime);

    // --------------------------------------------------------
    // Helpers
    // --------------------------------------------------------
    UFUNCTION(BlueprintCallable, Category = "NPC|State")
    void SetState(ENPC_TRexState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    void PickNewPatrolPoint();

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

private:
    bool bIsWaiting;
    float WaitTimer;
};
