#include "NPC_TRexBTTask_Patrol.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "GameFramework/Actor.h"
#include "Math/UnrealMathUtility.h"

UNPC_TRexBTTask_Patrol::UNPC_TRexBTTask_Patrol()
{
	NodeName = TEXT("TRex Patrol");
	PatrolRadius = 5000.0f;
	MinWaitTime  = 3.0f;
	MaxWaitTime  = 8.0f;
}

EBTNodeResult::Type UNPC_TRexBTTask_Patrol::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	APawn* ControlledPawn = AIController->GetPawn();
	if (!ControlledPawn)
	{
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard)
	{
		return EBTNodeResult::Failed;
	}

	// Get home location from blackboard (set on spawn by AIController)
	FVector HomeLocation = Blackboard->GetValueAsVector(HomeLocationKey.SelectedKeyName);

	// If home location is zero-vector, use current pawn location as home
	if (HomeLocation.IsZero())
	{
		HomeLocation = ControlledPawn->GetActorLocation();
		Blackboard->SetValueAsVector(HomeLocationKey.SelectedKeyName, HomeLocation);
	}

	// Pick a random navigable patrol point
	FVector PatrolTarget;
	if (!PickRandomPatrolPoint(ControlledPawn, HomeLocation, PatrolTarget))
	{
		// Fallback: stay near home
		PatrolTarget = HomeLocation + FVector(
			FMath::RandRange(-500.0f, 500.0f),
			FMath::RandRange(-500.0f, 500.0f),
			0.0f
		);
	}

	// Store patrol target in blackboard
	Blackboard->SetValueAsVector(PatrolTargetKey.SelectedKeyName, PatrolTarget);

	// Move to patrol target
	EPathFollowingRequestResult::Type MoveResult = AIController->MoveToLocation(PatrolTarget, 200.0f, true, true, false, true);

	if (MoveResult == EPathFollowingRequestResult::RequestSuccessful ||
		MoveResult == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}

FString UNPC_TRexBTTask_Patrol::GetStaticDescription() const
{
	return FString::Printf(TEXT("TRex Patrol\nRadius: %.0f units\nWait: %.1f-%.1fs"),
		PatrolRadius, MinWaitTime, MaxWaitTime);
}

bool UNPC_TRexBTTask_Patrol::PickRandomPatrolPoint(AActor* OwnerActor, FVector HomeLocation, FVector& OutPoint) const
{
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(OwnerActor->GetWorld());
	if (!NavSys)
	{
		return false;
	}

	FNavLocation NavLocation;
	bool bFound = NavSys->GetRandomReachablePointInRadius(HomeLocation, PatrolRadius, NavLocation);
	if (bFound)
	{
		OutPoint = NavLocation.Location;
		return true;
	}
	return false;
}
