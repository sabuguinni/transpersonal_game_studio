#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "NPC_TRexBTTask_Patrol.generated.h"

/**
 * NPC_TRexBTTask_Patrol
 * Behavior Tree Task: T-Rex patrols a 5000-unit radius around its home point.
 * Picks a random point within patrol radius, moves there, waits, repeats.
 * Agent #11 — NPC Behavior Agent
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNPC_TRexBTTask_Patrol : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UNPC_TRexBTTask_Patrol();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

	/** Radius around home point to patrol (units) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
	float PatrolRadius;

	/** Minimum wait time at patrol point (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
	float MinWaitTime;

	/** Maximum wait time at patrol point (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
	float MaxWaitTime;

	/** Blackboard key: home location (set once on spawn) */
	UPROPERTY(EditAnywhere, Category = "NPC|Blackboard")
	FBlackboardKeySelector HomeLocationKey;

	/** Blackboard key: current patrol target */
	UPROPERTY(EditAnywhere, Category = "NPC|Blackboard")
	FBlackboardKeySelector PatrolTargetKey;

private:
	/** Pick a random navigable point within PatrolRadius of HomeLocation */
	bool PickRandomPatrolPoint(AActor* OwnerActor, FVector HomeLocation, FVector& OutPoint) const;
};
