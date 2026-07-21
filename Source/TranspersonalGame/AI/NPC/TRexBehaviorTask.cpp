#include "TRexBehaviorTask.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "Kismet/GameplayStatics.h"

// ============================================================
// UNPC_TRexPatrolTask
// ============================================================

UNPC_TRexPatrolTask::UNPC_TRexPatrolTask()
{
	NodeName = TEXT("TRex Patrol To Waypoint");
	AcceptanceRadius = 300.0f;
}

EBTNodeResult::Type UNPC_TRexPatrolTask::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard)
	{
		return EBTNodeResult::Failed;
	}

	// Get patrol target location from blackboard
	FVector PatrolTarget = Blackboard->GetValueAsVector(PatrolTargetKey.SelectedKeyName);

	// Move to patrol target
	EPathFollowingRequestResult::Type MoveResult = AIController->MoveToLocation(
		PatrolTarget,
		AcceptanceRadius,
		true,   // bStopOnOverlap
		true,   // bUsePathfinding
		false,  // bProjectDestinationToNavigation
		false,  // bCanStrafe
		nullptr,
		false   // bAllowPartialPath
	);

	if (MoveResult == EPathFollowingRequestResult::RequestSuccessful)
	{
		return EBTNodeResult::InProgress;
	}
	else if (MoveResult == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}

FString UNPC_TRexPatrolTask::GetStaticDescription() const
{
	return FString::Printf(TEXT("TRex Patrol\nAcceptance Radius: %.0f\nTarget Key: %s"),
		AcceptanceRadius, *PatrolTargetKey.SelectedKeyName.ToString());
}

// ============================================================
// UNPC_TRexChaseTask
// ============================================================

UNPC_TRexChaseTask::UNPC_TRexChaseTask()
{
	NodeName = TEXT("TRex Chase Player");
	ChaseSpeedMultiplier = 1.8f;
}

EBTNodeResult::Type UNPC_TRexChaseTask::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard)
	{
		return EBTNodeResult::Failed;
	}

	// Get player actor from blackboard
	AActor* PlayerActor = Cast<AActor>(Blackboard->GetValueAsObject(PlayerActorKey.SelectedKeyName));
	if (!PlayerActor)
	{
		// Fallback: get player pawn directly
		PlayerActor = UGameplayStatics::GetPlayerPawn(AIController->GetWorld(), 0);
		if (!PlayerActor)
		{
			return EBTNodeResult::Failed;
		}
	}

	// Apply chase speed boost to the controlled pawn
	APawn* ControlledPawn = AIController->GetPawn();
	if (ControlledPawn)
	{
		ACharacter* TRexCharacter = Cast<ACharacter>(ControlledPawn);
		if (TRexCharacter && TRexCharacter->GetCharacterMovement())
		{
			// Boost speed during chase
			TRexCharacter->GetCharacterMovement()->MaxWalkSpeed = 800.0f * ChaseSpeedMultiplier;
		}
	}

	// Move toward player
	EPathFollowingRequestResult::Type MoveResult = AIController->MoveToActor(
		PlayerActor,
		200.0f,  // AcceptanceRadius — stop when within 200 units (attack range)
		true,
		true,
		false,
		nullptr,
		false
	);

	if (MoveResult == EPathFollowingRequestResult::RequestSuccessful)
	{
		return EBTNodeResult::InProgress;
	}
	else if (MoveResult == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}

FString UNPC_TRexChaseTask::GetStaticDescription() const
{
	return FString::Printf(TEXT("TRex Chase Player\nSpeed Multiplier: %.1fx\nPlayer Key: %s"),
		ChaseSpeedMultiplier, *PlayerActorKey.SelectedKeyName.ToString());
}

// ============================================================
// UNPC_TRexAttackTask
// ============================================================

UNPC_TRexAttackTask::UNPC_TRexAttackTask()
{
	NodeName = TEXT("TRex Attack Player");
	AttackDamage = 75.0f;
	AttackCooldown = 2.5f;
}

EBTNodeResult::Type UNPC_TRexAttackTask::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard)
	{
		return EBTNodeResult::Failed;
	}

	// Get player actor
	AActor* PlayerActor = Cast<AActor>(Blackboard->GetValueAsObject(PlayerActorKey.SelectedKeyName));
	if (!PlayerActor)
	{
		PlayerActor = UGameplayStatics::GetPlayerPawn(AIController->GetWorld(), 0);
	}

	if (!PlayerActor)
	{
		return EBTNodeResult::Failed;
	}

	APawn* ControlledPawn = AIController->GetPawn();
	if (!ControlledPawn)
	{
		return EBTNodeResult::Failed;
	}

	// Distance check — only attack if within range
	float DistToPlayer = FVector::Dist(ControlledPawn->GetActorLocation(), PlayerActor->GetActorLocation());
	if (DistToPlayer > 400.0f)
	{
		// Player moved out of range — abort attack
		return EBTNodeResult::Failed;
	}

	// Apply damage to player
	UGameplayStatics::ApplyDamage(
		PlayerActor,
		AttackDamage,
		AIController,
		ControlledPawn,
		UDamageType::StaticClass()
	);

	// Face the player during attack
	FVector DirectionToPlayer = (PlayerActor->GetActorLocation() - ControlledPawn->GetActorLocation()).GetSafeNormal();
	FRotator LookAtRotation = DirectionToPlayer.Rotation();
	ControlledPawn->SetActorRotation(LookAtRotation);

	// Reset speed after attack
	ACharacter* TRexCharacter = Cast<ACharacter>(ControlledPawn);
	if (TRexCharacter && TRexCharacter->GetCharacterMovement())
	{
		TRexCharacter->GetCharacterMovement()->MaxWalkSpeed = 600.0f;
	}

	return EBTNodeResult::Succeeded;
}

FString UNPC_TRexAttackTask::GetStaticDescription() const
{
	return FString::Printf(TEXT("TRex Attack\nDamage: %.0f\nCooldown: %.1fs\nPlayer Key: %s"),
		AttackDamage, AttackCooldown, *PlayerActorKey.SelectedKeyName.ToString());
}
