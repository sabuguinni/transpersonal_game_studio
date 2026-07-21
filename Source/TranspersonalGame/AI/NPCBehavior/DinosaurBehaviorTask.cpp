#include "DinosaurBehaviorTask.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ─────────────────────────────────────────────────────────────────────────────
// UNPC_BTTask_DinoPatrol
// ─────────────────────────────────────────────────────────────────────────────

UNPC_BTTask_DinoPatrol::UNPC_BTTask_DinoPatrol()
{
    NodeName = TEXT("Dino Patrol");
    bNotifyTick = false;
}

EBTNodeResult::Type UNPC_BTTask_DinoPatrol::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIC = OwnerComp.GetAIOwner();
    if (!AIC) return EBTNodeResult::Failed;

    APawn* Pawn = AIC->GetPawn();
    if (!Pawn) return EBTNodeResult::Failed;

    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(Pawn->GetWorld());
    if (!NavSys) return EBTNodeResult::Failed;

    // Pick a random reachable point within PatrolRadius of current location
    FNavLocation RandomPoint;
    const FVector Origin = Pawn->GetActorLocation();
    bool bFound = NavSys->GetRandomReachablePointInRadius(Origin, PatrolRadius, RandomPoint);

    if (!bFound)
    {
        // Fallback: pick a random point in a sphere and try to move there
        FVector Fallback = Origin + FMath::VRand() * PatrolRadius * 0.5f;
        RandomPoint.Location = Fallback;
    }

    // Write to blackboard
    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (BB)
    {
        BB->SetValueAsVector(PatrolTargetKey.SelectedKeyName, RandomPoint.Location);
    }

    // Move to the point
    EPathFollowingRequestResult::Type MoveResult = AIC->MoveToLocation(RandomPoint.Location, 50.f);

    if (MoveResult == EPathFollowingRequestResult::RequestSuccessful ||
        MoveResult == EPathFollowingRequestResult::AlreadyAtGoal)
    {
        return EBTNodeResult::Succeeded;
    }

    return EBTNodeResult::Failed;
}

FString UNPC_BTTask_DinoPatrol::GetStaticDescription() const
{
    return FString::Printf(TEXT("Patrol within %.0f cm radius"), PatrolRadius);
}

// ─────────────────────────────────────────────────────────────────────────────
// UNPC_BTTask_DinoChase
// ─────────────────────────────────────────────────────────────────────────────

UNPC_BTTask_DinoChase::UNPC_BTTask_DinoChase()
{
    NodeName = TEXT("Dino Chase");
    bNotifyTick = false;
}

EBTNodeResult::Type UNPC_BTTask_DinoChase::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIC = OwnerComp.GetAIOwner();
    if (!AIC) return EBTNodeResult::Failed;

    APawn* Pawn = AIC->GetPawn();
    if (!Pawn) return EBTNodeResult::Failed;

    // Get chase target from blackboard
    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!BB) return EBTNodeResult::Failed;

    AActor* Target = Cast<AActor>(BB->GetValueAsObject(ChaseTargetKey.SelectedKeyName));
    if (!Target) return EBTNodeResult::Failed;

    // Boost movement speed for chase
    ACharacter* DinoChar = Cast<ACharacter>(Pawn);
    if (DinoChar && DinoChar->GetCharacterMovement())
    {
        UCharacterMovementComponent* MoveComp = DinoChar->GetCharacterMovement();
        const float BaseSpeed = MoveComp->MaxWalkSpeed;
        MoveComp->MaxWalkSpeed = BaseSpeed * ChaseSpeedMultiplier;
    }

    // Move toward target
    EPathFollowingRequestResult::Type MoveResult = AIC->MoveToActor(Target, 50.f);

    if (MoveResult == EPathFollowingRequestResult::RequestSuccessful ||
        MoveResult == EPathFollowingRequestResult::AlreadyAtGoal)
    {
        return EBTNodeResult::Succeeded;
    }

    return EBTNodeResult::Failed;
}

FString UNPC_BTTask_DinoChase::GetStaticDescription() const
{
    return FString::Printf(TEXT("Chase target at %.1fx speed"), ChaseSpeedMultiplier);
}

// ─────────────────────────────────────────────────────────────────────────────
// UNPC_BTTask_DinoAttack
// ─────────────────────────────────────────────────────────────────────────────

UNPC_BTTask_DinoAttack::UNPC_BTTask_DinoAttack()
{
    NodeName = TEXT("Dino Attack");
    bNotifyTick = false;
}

EBTNodeResult::Type UNPC_BTTask_DinoAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIC = OwnerComp.GetAIOwner();
    if (!AIC) return EBTNodeResult::Failed;

    APawn* Pawn = AIC->GetPawn();
    if (!Pawn) return EBTNodeResult::Failed;

    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!BB) return EBTNodeResult::Failed;

    AActor* Target = Cast<AActor>(BB->GetValueAsObject(AttackTargetKey.SelectedKeyName));
    if (!Target) return EBTNodeResult::Failed;

    // Check range
    const float DistToTarget = FVector::Dist(Pawn->GetActorLocation(), Target->GetActorLocation());
    if (DistToTarget > AttackRange)
    {
        return EBTNodeResult::Failed;
    }

    // Check cooldown
    const float CurrentTime = Pawn->GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastAttackTime < AttackCooldown)
    {
        return EBTNodeResult::Failed;
    }

    LastAttackTime = CurrentTime;

    // Apply damage via UE5 damage system
    UGameplayStatics::ApplyDamage(
        Target,
        BaseDamage,
        AIC,
        Pawn,
        UDamageType::StaticClass()
    );

    // Face target during attack
    const FVector ToTarget = (Target->GetActorLocation() - Pawn->GetActorLocation()).GetSafeNormal();
    const FRotator FaceRot = ToTarget.Rotation();
    Pawn->SetActorRotation(FaceRot);

    return EBTNodeResult::Succeeded;
}

FString UNPC_BTTask_DinoAttack::GetStaticDescription() const
{
    return FString::Printf(TEXT("Attack within %.0f cm, %.0f dmg, %.1fs cooldown"),
        AttackRange, BaseDamage, AttackCooldown);
}

// ─────────────────────────────────────────────────────────────────────────────
// UNPC_BTTask_DinoGraze
// ─────────────────────────────────────────────────────────────────────────────

UNPC_BTTask_DinoGraze::UNPC_BTTask_DinoGraze()
{
    NodeName = TEXT("Dino Graze");
    bNotifyTick = false;
    bCreateNodeInstance = true; // needed for timer handle per-instance
}

EBTNodeResult::Type UNPC_BTTask_DinoGraze::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIC = OwnerComp.GetAIOwner();
    if (!AIC) return EBTNodeResult::Failed;

    APawn* Pawn = AIC->GetPawn();
    if (!Pawn) return EBTNodeResult::Failed;

    // Stop movement while grazing
    AIC->StopMovement();

    // Slow the dino down to a grazing shuffle
    ACharacter* DinoChar = Cast<ACharacter>(Pawn);
    if (DinoChar && DinoChar->GetCharacterMovement())
    {
        DinoChar->GetCharacterMovement()->MaxWalkSpeed = 80.f;
    }

    // Set a timer to finish grazing
    UBehaviorTreeComponent* BTComp = &OwnerComp;
    Pawn->GetWorld()->GetTimerManager().SetTimer(
        GrazeTimerHandle,
        [BTComp]()
        {
            if (BTComp)
            {
                // Signal task completion
                BTComp->OnTaskFinished(nullptr, EBTNodeResult::Succeeded);
            }
        },
        GrazeDuration,
        false
    );

    return EBTNodeResult::InProgress;
}

EBTNodeResult::Type UNPC_BTTask_DinoGraze::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIC = OwnerComp.GetAIOwner();
    if (AIC && AIC->GetPawn())
    {
        AIC->GetPawn()->GetWorld()->GetTimerManager().ClearTimer(GrazeTimerHandle);
    }
    return EBTNodeResult::Aborted;
}

FString UNPC_BTTask_DinoGraze::GetStaticDescription() const
{
    return FString::Printf(TEXT("Graze for %.1f seconds"), GrazeDuration);
}
