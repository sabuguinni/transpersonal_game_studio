#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "TRexBehaviorTask.generated.h"

/**
 * T-Rex Patrol Task — moves T-Rex to next waypoint in sequence.
 * Used by the T-Rex Behavior Tree to implement patrol loop.
 */
UCLASS()
class TRANSPERSONALGAME_API UNPC_TRexPatrolTask : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UNPC_TRexPatrolTask();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

	/** Radius around each waypoint considered "reached" */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Patrol")
	float AcceptanceRadius;

	/** Key for the current patrol target location in the Blackboard */
	UPROPERTY(EditAnywhere, Category = "TRex|Patrol")
	FBlackboardKeySelector PatrolTargetKey;
};

/**
 * T-Rex Chase Task — moves T-Rex toward the player at full speed.
 */
UCLASS()
class TRANSPERSONALGAME_API UNPC_TRexChaseTask : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UNPC_TRexChaseTask();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

	/** Key for the player actor reference in the Blackboard */
	UPROPERTY(EditAnywhere, Category = "TRex|Chase")
	FBlackboardKeySelector PlayerActorKey;

	/** Chase speed multiplier applied to CharacterMovement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Chase")
	float ChaseSpeedMultiplier;
};

/**
 * T-Rex Attack Task — executes a stomp/bite attack on the player.
 */
UCLASS()
class TRANSPERSONALGAME_API UNPC_TRexAttackTask : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UNPC_TRexAttackTask();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

	/** Damage dealt per attack */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Attack")
	float AttackDamage;

	/** Attack cooldown in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Attack")
	float AttackCooldown;

	/** Key for the player actor reference in the Blackboard */
	UPROPERTY(EditAnywhere, Category = "TRex|Attack")
	FBlackboardKeySelector PlayerActorKey;
};
