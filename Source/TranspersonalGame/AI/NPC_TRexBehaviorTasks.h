// NPC_TRexBehaviorTasks.h
// T-Rex Behavior Tree task nodes: Patrol, Chase, Attack
// Agent #11 — NPC Behavior Agent | PROD_CYCLE_AUTO_20260625_012
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "NPC_TRexBehaviorTasks.generated.h"

// ── Patrol Task ──────────────────────────────────────────────────────────────
UCLASS()
class TRANSPERSONALGAME_API UNPC_TRexPatrolTask : public UBTTaskNode
{
    GENERATED_BODY()
public:
    UNPC_TRexPatrolTask();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual FString GetStaticDescription() const override;

    /** Patrol radius around home location (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Patrol")
    float PatrolRadius = 500000.0f; // 5000 units

    /** Minimum time to wait at each patrol point (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Patrol")
    float MinWaitTime = 3.0f;

    /** Maximum time to wait at each patrol point (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Patrol")
    float MaxWaitTime = 8.0f;

    /** Blackboard key for the patrol destination */
    UPROPERTY(EditAnywhere, Category = "TRex|Patrol")
    FBlackboardKeySelector PatrolLocationKey;

    /** Blackboard key for the T-Rex home location */
    UPROPERTY(EditAnywhere, Category = "TRex|Patrol")
    FBlackboardKeySelector HomeLocationKey;
};

// ── Chase Task ───────────────────────────────────────────────────────────────
UCLASS()
class TRANSPERSONALGAME_API UNPC_TRexChaseTask : public UBTTaskNode
{
    GENERATED_BODY()
public:
    UNPC_TRexChaseTask();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual FString GetStaticDescription() const override;

    /** Chase speed multiplier over base movement speed */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Chase")
    float ChaseSpeedMultiplier = 1.8f;

    /** Distance at which T-Rex gives up chase (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Chase")
    float AbandonChaseDistance = 800000.0f; // 8000 units

    /** Blackboard key for the target actor (player) */
    UPROPERTY(EditAnywhere, Category = "TRex|Chase")
    FBlackboardKeySelector TargetActorKey;

    /** Blackboard key for last known player location */
    UPROPERTY(EditAnywhere, Category = "TRex|Chase")
    FBlackboardKeySelector LastKnownLocationKey;
};

// ── Attack Task ──────────────────────────────────────────────────────────────
UCLASS()
class TRANSPERSONALGAME_API UNPC_TRexAttackTask : public UBTTaskNode
{
    GENERATED_BODY()
public:
    UNPC_TRexAttackTask();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual FString GetStaticDescription() const override;

    /** Melee attack range (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Attack")
    float AttackRange = 30000.0f; // 300 units

    /** Base damage dealt per bite */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Attack")
    float BiteDamage = 85.0f;

    /** Cooldown between attacks (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Attack")
    float AttackCooldown = 2.5f;

    /** Blackboard key for the target actor */
    UPROPERTY(EditAnywhere, Category = "TRex|Attack")
    FBlackboardKeySelector TargetActorKey;
};
