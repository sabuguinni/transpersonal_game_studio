#include "DinosaurBehaviorTask.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

// ============================================================
// UNPC_BTTask_DinoPatrol
// ============================================================

UNPC_BTTask_DinoPatrol::UNPC_BTTask_DinoPatrol()
{
    NodeName = TEXT("NPC Dino Patrol");
    bNotifyTick = false;
}

EBTNodeResult::Type UNPC_BTTask_DinoPatrol::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

    // Get patrol origin from blackboard (default to current pawn location)
    FVector Origin = ControlledPawn->GetActorLocation();
    float Radius = 5000.0f;

    // Try to read from blackboard keys if set
    if (PatrolOriginKey.SelectedKeyName != NAME_None)
    {
        FVector BBOrigin = Blackboard->GetValueAsVector(PatrolOriginKey.SelectedKeyName);
        if (!BBOrigin.IsZero())
        {
            Origin = BBOrigin;
        }
    }
    if (PatrolRadiusKey.SelectedKeyName != NAME_None)
    {
        float BBRadius = Blackboard->GetValueAsFloat(PatrolRadiusKey.SelectedKeyName);
        if (BBRadius > 0.0f)
        {
            Radius = BBRadius;
        }
    }

    // Find a random reachable point within patrol radius
    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(ControlledPawn->GetWorld());
    if (!NavSys)
    {
        return EBTNodeResult::Failed;
    }

    FNavLocation RandomPoint;
    bool bFound = false;
    int32 Attempts = 0;

    while (!bFound && Attempts < 10)
    {
        bFound = NavSys->GetRandomReachablePointInRadius(Origin, Radius, RandomPoint);
        // Ensure minimum distance from current position
        if (bFound && FVector::Dist(RandomPoint.Location, ControlledPawn->GetActorLocation()) < MinPatrolDistance)
        {
            bFound = false;
        }
        Attempts++;
    }

    if (!bFound)
    {
        return EBTNodeResult::Failed;
    }

    // Move to the random patrol point
    EPathFollowingRequestResult::Type MoveResult = AIController->MoveToLocation(
        RandomPoint.Location,
        50.0f,   // acceptance radius
        true,    // stop on overlap
        true,    // use pathfinding
        false,   // project destination to navigation
        true,    // can strafe
        nullptr, // filter class
        false    // allow partial path
    );

    if (MoveResult == EPathFollowingRequestResult::Failed)
    {
        return EBTNodeResult::Failed;
    }

    return EBTNodeResult::Succeeded;
}

FString UNPC_BTTask_DinoPatrol::GetStaticDescription() const
{
    return TEXT("Move dinosaur to random patrol point within radius.\nUses NavMesh for pathfinding.");
}

// ============================================================
// UNPC_BTTask_DinoChase
// ============================================================

UNPC_BTTask_DinoChase::UNPC_BTTask_DinoChase()
{
    NodeName = TEXT("NPC Dino Chase");
    bNotifyTick = false;
}

EBTNodeResult::Type UNPC_BTTask_DinoChase::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

    // Get target actor from blackboard
    AActor* Target = Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName));
    if (!Target)
    {
        return EBTNodeResult::Failed;
    }

    // Move to target with acceptance radius just outside attack range
    EPathFollowingRequestResult::Type MoveResult = AIController->MoveToActor(
        Target,
        AcceptanceRadius,
        true,    // stop on overlap
        true,    // use pathfinding
        false,   // project destination
        true,    // can strafe
        nullptr, // filter class
        false    // allow partial path
    );

    if (MoveResult == EPathFollowingRequestResult::Failed)
    {
        return EBTNodeResult::Failed;
    }

    return EBTNodeResult::Succeeded;
}

FString UNPC_BTTask_DinoChase::GetStaticDescription() const
{
    return FString::Printf(TEXT("Chase target actor.\nAcceptance radius: %.0f units"), AcceptanceRadius);
}

// ============================================================
// UNPC_BTTask_DinoAttack
// ============================================================

UNPC_BTTask_DinoAttack::UNPC_BTTask_DinoAttack()
{
    NodeName = TEXT("NPC Dino Attack");
    bNotifyTick = false;
}

EBTNodeResult::Type UNPC_BTTask_DinoAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

    AActor* Target = Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName));
    if (!Target)
    {
        return EBTNodeResult::Failed;
    }

    APawn* ControlledPawn = AIController->GetPawn();
    if (!ControlledPawn)
    {
        return EBTNodeResult::Failed;
    }

    // Cooldown check
    float CurrentTime = ControlledPawn->GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastAttackTime < AttackCooldown)
    {
        return EBTNodeResult::Failed;
    }

    // Distance check — must be within attack range
    float Distance = FVector::Dist(ControlledPawn->GetActorLocation(), Target->GetActorLocation());
    if (Distance > AttackRange + 100.0f)
    {
        return EBTNodeResult::Failed;
    }

    // Apply damage using UE5 damage system
    UGameplayStatics::ApplyDamage(
        Target,
        BaseDamage,
        AIController,
        ControlledPawn,
        UDamageType::StaticClass()
    );

    LastAttackTime = CurrentTime;

    UE_LOG(LogTemp, Log, TEXT("[NPC_DinoAttack] %s attacked %s for %.0f damage"),
        *ControlledPawn->GetName(), *Target->GetName(), BaseDamage);

    return EBTNodeResult::Succeeded;
}

FString UNPC_BTTask_DinoAttack::GetStaticDescription() const
{
    return FString::Printf(TEXT("Attack target.\nDamage: %.0f | Cooldown: %.1fs | Range: %.0f"),
        BaseDamage, AttackCooldown, AttackRange);
}

// ============================================================
// UNPC_BTTask_DinoRoar
// ============================================================

UNPC_BTTask_DinoRoar::UNPC_BTTask_DinoRoar()
{
    NodeName = TEXT("NPC Dino Roar");
    bNotifyTick = false;
}

EBTNodeResult::Type UNPC_BTTask_DinoRoar::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

    UWorld* World = ControlledPawn->GetWorld();
    if (!World)
    {
        return EBTNodeResult::Failed;
    }

    FVector RoarOrigin = ControlledPawn->GetActorLocation();

    // Find all characters within roar radius and apply fear
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(World, ACharacter::StaticClass(), NearbyActors);

    int32 AffectedCount = 0;
    for (AActor* NearbyActor : NearbyActors)
    {
        if (!NearbyActor || NearbyActor == ControlledPawn)
        {
            continue;
        }

        float Distance = FVector::Dist(RoarOrigin, NearbyActor->GetActorLocation());
        if (Distance <= RoarRadius)
        {
            // Apply fear damage (non-lethal — fear type)
            // Scale fear by proximity: closer = more fear
            float FearScale = 1.0f - (Distance / RoarRadius);
            float FearAmount = FearInflicted * FearScale;

            UGameplayStatics::ApplyDamage(
                NearbyActor,
                FearAmount,
                AIController,
                ControlledPawn,
                UDamageType::StaticClass()
            );

            AffectedCount++;

            UE_LOG(LogTemp, Log, TEXT("[NPC_DinoRoar] Roar affected %s — fear: %.1f (dist: %.0f)"),
                *NearbyActor->GetName(), FearAmount, Distance);
        }
    }

    // Debug sphere in editor
#if WITH_EDITOR
    DrawDebugSphere(World, RoarOrigin, RoarRadius, 16, FColor::Orange, false, 3.0f);
#endif

    UE_LOG(LogTemp, Log, TEXT("[NPC_DinoRoar] %s roared — affected %d actors within %.0f units"),
        *ControlledPawn->GetName(), AffectedCount, RoarRadius);

    return EBTNodeResult::Succeeded;
}

FString UNPC_BTTask_DinoRoar::GetStaticDescription() const
{
    return FString::Printf(TEXT("Roar intimidation.\nRadius: %.0f | Fear: %.0f"),
        RoarRadius, FearInflicted);
}
