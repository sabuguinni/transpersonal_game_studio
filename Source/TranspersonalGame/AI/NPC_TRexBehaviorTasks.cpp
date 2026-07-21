// NPC_TRexBehaviorTasks.cpp
// T-Rex Behavior Tree task implementations: Patrol, Chase, Attack
// Agent #11 — NPC Behavior Agent | PROD_CYCLE_AUTO_20260625_012

#include "NPC_TRexBehaviorTasks.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

// ─────────────────────────────────────────────────────────────────────────────
// PATROL TASK
// ─────────────────────────────────────────────────────────────────────────────

UNPC_TRexPatrolTask::UNPC_TRexPatrolTask()
{
    NodeName = TEXT("TRex Patrol");
    bNotifyTick = false;
}

EBTNodeResult::Type UNPC_TRexPatrolTask::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

    // Get home location from blackboard (set once when T-Rex spawns)
    FVector HomeLocation = Blackboard->GetValueAsVector(HomeLocationKey.SelectedKeyName);
    if (HomeLocation.IsZero())
    {
        // First time: record current position as home
        HomeLocation = ControlledPawn->GetActorLocation();
        Blackboard->SetValueAsVector(HomeLocationKey.SelectedKeyName, HomeLocation);
    }

    // Find a random reachable point within patrol radius
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(ControlledPawn->GetWorld());
    if (!NavSys)
    {
        return EBTNodeResult::Failed;
    }

    FNavLocation RandomPoint;
    const bool bFound = NavSys->GetRandomReachablePointInRadius(HomeLocation, PatrolRadius, RandomPoint);
    if (!bFound)
    {
        return EBTNodeResult::Failed;
    }

    // Store destination in blackboard
    Blackboard->SetValueAsVector(PatrolLocationKey.SelectedKeyName, RandomPoint.Location);

    // Move to patrol point
    const EPathFollowingRequestResult::Type MoveResult = AIController->MoveToLocation(
        RandomPoint.Location,
        /*AcceptanceRadius=*/ 200.0f,
        /*bStopOnOverlap=*/ true,
        /*bUsePathfinding=*/ true,
        /*bProjectDestinationToNavigation=*/ true,
        /*bCanStrafe=*/ false
    );

    if (MoveResult == EPathFollowingRequestResult::Failed)
    {
        return EBTNodeResult::Failed;
    }

    return EBTNodeResult::Succeeded;
}

FString UNPC_TRexPatrolTask::GetStaticDescription() const
{
    return FString::Printf(TEXT("T-Rex Patrol\nRadius: %.0f cm\nWait: %.1f-%.1f s"),
        PatrolRadius, MinWaitTime, MaxWaitTime);
}

// ─────────────────────────────────────────────────────────────────────────────
// CHASE TASK
// ─────────────────────────────────────────────────────────────────────────────

UNPC_TRexChaseTask::UNPC_TRexChaseTask()
{
    NodeName = TEXT("TRex Chase");
    bNotifyTick = false;
}

EBTNodeResult::Type UNPC_TRexChaseTask::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

    // Get target actor from blackboard
    AActor* TargetActor = Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName));
    if (!TargetActor)
    {
        return EBTNodeResult::Failed;
    }

    // Check if target is within abandon distance
    const float DistanceToTarget = FVector::Dist(ControlledPawn->GetActorLocation(), TargetActor->GetActorLocation());
    if (DistanceToTarget > AbandonChaseDistance)
    {
        // Lost target — clear blackboard key and return failed to trigger patrol
        Blackboard->ClearValue(TargetActorKey.SelectedKeyName);
        return EBTNodeResult::Failed;
    }

    // Boost movement speed during chase
    ACharacter* TRexCharacter = Cast<ACharacter>(ControlledPawn);
    if (TRexCharacter && TRexCharacter->GetCharacterMovement())
    {
        const float BaseSpeed = 600.0f; // T-Rex base walk speed (cm/s)
        TRexCharacter->GetCharacterMovement()->MaxWalkSpeed = BaseSpeed * ChaseSpeedMultiplier;
    }

    // Update last known location
    Blackboard->SetValueAsVector(LastKnownLocationKey.SelectedKeyName, TargetActor->GetActorLocation());

    // Move toward target
    const EPathFollowingRequestResult::Type MoveResult = AIController->MoveToActor(
        TargetActor,
        /*AcceptanceRadius=*/ 30000.0f, // 300 units — transition to attack range
        /*bStopOnOverlap=*/ true,
        /*bUsePathfinding=*/ true,
        /*bCanStrafe=*/ false
    );

    if (MoveResult == EPathFollowingRequestResult::Failed)
    {
        // Pathfinding failed — try moving to last known location
        const FVector LastKnown = Blackboard->GetValueAsVector(LastKnownLocationKey.SelectedKeyName);
        if (!LastKnown.IsZero())
        {
            AIController->MoveToLocation(LastKnown, 200.0f);
        }
        return EBTNodeResult::Failed;
    }

    return EBTNodeResult::Succeeded;
}

FString UNPC_TRexChaseTask::GetStaticDescription() const
{
    return FString::Printf(TEXT("T-Rex Chase\nSpeed Mult: %.1fx\nAbandon: %.0f cm"),
        ChaseSpeedMultiplier, AbandonChaseDistance);
}

// ─────────────────────────────────────────────────────────────────────────────
// ATTACK TASK
// ─────────────────────────────────────────────────────────────────────────────

UNPC_TRexAttackTask::UNPC_TRexAttackTask()
{
    NodeName = TEXT("TRex Attack");
    bNotifyTick = false;
}

EBTNodeResult::Type UNPC_TRexAttackTask::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

    // Get target
    AActor* TargetActor = Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName));
    if (!TargetActor)
    {
        return EBTNodeResult::Failed;
    }

    // Verify target is within attack range
    const float DistanceToTarget = FVector::Dist(ControlledPawn->GetActorLocation(), TargetActor->GetActorLocation());
    if (DistanceToTarget > AttackRange)
    {
        // Target moved out of range — return failed to re-enter chase
        return EBTNodeResult::Failed;
    }

    // Face the target
    const FVector DirectionToTarget = (TargetActor->GetActorLocation() - ControlledPawn->GetActorLocation()).GetSafeNormal();
    const FRotator LookAtRotation = DirectionToTarget.Rotation();
    ControlledPawn->SetActorRotation(FRotator(0.0f, LookAtRotation.Yaw, 0.0f));

    // Apply damage via UE5 damage system
    UGameplayStatics::ApplyDamage(
        TargetActor,
        BiteDamage,
        AIController,
        ControlledPawn,
        UDamageType::StaticClass()
    );

    // Debug: draw attack sphere in editor
#if WITH_EDITOR
    DrawDebugSphere(
        ControlledPawn->GetWorld(),
        ControlledPawn->GetActorLocation() + ControlledPawn->GetActorForwardVector() * 20000.0f,
        AttackRange,
        12,
        FColor::Red,
        false,
        1.5f
    );
#endif

    return EBTNodeResult::Succeeded;
}

FString UNPC_TRexAttackTask::GetStaticDescription() const
{
    return FString::Printf(TEXT("T-Rex Attack\nDamage: %.0f\nRange: %.0f cm\nCooldown: %.1f s"),
        BiteDamage, AttackRange, AttackCooldown);
}
