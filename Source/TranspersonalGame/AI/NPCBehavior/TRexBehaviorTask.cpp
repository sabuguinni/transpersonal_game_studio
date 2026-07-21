// TRexBehaviorTask.cpp
// NPC Behavior Agent #11 — T-Rex Behavior Tree Task Implementations
// Patrol (5000u radius) → Detect (3000u cone) → Chase (1.4× speed) → Attack (300u, 85 dmg)

#include "TRexBehaviorTask.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "NavigationSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

// ─── UNPC_TRexPatrolTask ─────────────────────────────────────────────────────

UNPC_TRexPatrolTask::UNPC_TRexPatrolTask()
{
    NodeName = TEXT("TRex Patrol");
    bNotifyTick = true;
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

    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(ControlledPawn->GetWorld());
    if (!NavSys)
    {
        return EBTNodeResult::Failed;
    }

    // Get home location from blackboard or use current pawn location
    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    FVector HomeLocation = ControlledPawn->GetActorLocation();

    // Pick a random patrol point within radius
    FVector PatrolPoint = GetRandomPatrolPoint(HomeLocation, PatrolRadius, NavSys, ControlledPawn);

    // Write patrol destination to blackboard
    if (BB)
    {
        BB->SetValueAsVector(PatrolDestinationKey.SelectedKeyName, PatrolPoint);
    }

    // Move to patrol point
    AIController->MoveToLocation(PatrolPoint, 100.0f, true, true, false, false);

    return EBTNodeResult::InProgress;
}

void UNPC_TRexPatrolTask::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    // Check if we've reached the patrol point
    EPathFollowingStatus::Type MoveStatus = AIController->GetMoveStatus();
    if (MoveStatus == EPathFollowingStatus::Idle)
    {
        // Reached destination — task complete, BT will loop
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
    }
}

FVector UNPC_TRexPatrolTask::GetRandomPatrolPoint(const FVector& HomeLocation, float Radius, UNavigationSystemV1* NavSys, AActor* Owner) const
{
    FNavLocation NavLoc;
    if (NavSys && NavSys->GetRandomReachablePointInRadius(HomeLocation, Radius, NavLoc))
    {
        return NavLoc.Location;
    }
    // Fallback: random offset from home
    FVector RandomOffset = FVector(
        FMath::RandRange(-Radius, Radius),
        FMath::RandRange(-Radius, Radius),
        0.0f
    );
    return HomeLocation + RandomOffset;
}

// ─── UNPC_TRexChaseTask ──────────────────────────────────────────────────────

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

    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!BB)
    {
        return EBTNodeResult::Failed;
    }

    // Get target from blackboard
    AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetActorKey.SelectedKeyName));
    if (!Target)
    {
        return EBTNodeResult::Failed;
    }

    // Boost movement speed for chase
    APawn* ControlledPawn = AIController->GetPawn();
    ACharacter* DinoChar = Cast<ACharacter>(ControlledPawn);
    if (DinoChar && DinoChar->GetCharacterMovement())
    {
        float BaseSpeed = DinoChar->GetCharacterMovement()->MaxWalkSpeed;
        DinoChar->GetCharacterMovement()->MaxWalkSpeed = BaseSpeed * ChaseSpeedMultiplier;
    }

    // Move to target with tight acceptance radius
    AIController->MoveToActor(Target, 25000.0f, true, true, false); // 250 unit acceptance

    return EBTNodeResult::Succeeded;
}

// ─── UNPC_TRexAttackTask ─────────────────────────────────────────────────────

UNPC_TRexAttackTask::UNPC_TRexAttackTask()
{
    NodeName = TEXT("TRex Bite Attack");
    bNotifyTick = false;
}

EBTNodeResult::Type UNPC_TRexAttackTask::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController)
    {
        return EBTNodeResult::Failed;
    }

    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!BB)
    {
        return EBTNodeResult::Failed;
    }

    AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetActorKey.SelectedKeyName));
    if (!Target)
    {
        return EBTNodeResult::Failed;
    }

    APawn* ControlledPawn = AIController->GetPawn();
    if (!ControlledPawn)
    {
        return EBTNodeResult::Failed;
    }

    // Check cooldown
    float CurrentTime = ControlledPawn->GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastAttackTime < AttackCooldown)
    {
        return EBTNodeResult::Failed;
    }

    // Check range
    float DistToTarget = FVector::Dist(ControlledPawn->GetActorLocation(), Target->GetActorLocation());
    if (DistToTarget > AttackRange)
    {
        return EBTNodeResult::Failed;
    }

    // Apply damage
    UGameplayStatics::ApplyDamage(
        Target,
        BiteDamage,
        AIController,
        ControlledPawn,
        UDamageType::StaticClass()
    );

    LastAttackTime = CurrentTime;

    UE_LOG(LogTemp, Log, TEXT("TRex BITE: %.0f damage to %s"), BiteDamage, *Target->GetName());

    return EBTNodeResult::Succeeded;
}

// ─── UNPC_TRexRoarTask ───────────────────────────────────────────────────────

UNPC_TRexRoarTask::UNPC_TRexRoarTask()
{
    NodeName = TEXT("TRex Roar");
    bNotifyTick = false;
}

EBTNodeResult::Type UNPC_TRexRoarTask::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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
    FVector RoarOrigin = ControlledPawn->GetActorLocation();

    // Find all actors within roar radius and apply fear
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(World, APawn::StaticClass(), NearbyActors);

    int32 AffectedCount = 0;
    for (AActor* Actor : NearbyActors)
    {
        if (!Actor || Actor == ControlledPawn) continue;

        float Dist = FVector::Dist(RoarOrigin, Actor->GetActorLocation());
        if (Dist <= RoarRadius)
        {
            // Apply fear damage (non-lethal) — triggers fear response in SurvivalComponent
            UGameplayStatics::ApplyDamage(
                Actor,
                FearImpact,
                AIController,
                ControlledPawn,
                UDamageType::StaticClass()
            );
            AffectedCount++;
        }
    }

    // Debug sphere in editor
#if WITH_EDITOR
    DrawDebugSphere(World, RoarOrigin, RoarRadius, 16, FColor::Orange, false, 3.0f);
#endif

    UE_LOG(LogTemp, Log, TEXT("TRex ROAR: affected %d actors within %.0f cm"), AffectedCount, RoarRadius);

    return EBTNodeResult::Succeeded;
}

// ─── UNPC_TRexDetectDecorator ────────────────────────────────────────────────

UNPC_TRexDetectDecorator::UNPC_TRexDetectDecorator()
{
    NodeName = TEXT("TRex Can Detect Player");
    bNotifyBecomeRelevant = true;
    bNotifyCeaseRelevant = false;
}

bool UNPC_TRexDetectDecorator::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController)
    {
        return false;
    }

    APawn* ControlledPawn = AIController->GetPawn();
    if (!ControlledPawn)
    {
        return false;
    }

    UWorld* World = ControlledPawn->GetWorld();
    if (!World)
    {
        return false;
    }

    // Get player pawn
    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    if (!PC || !PC->GetPawn())
    {
        return false;
    }

    APawn* PlayerPawn = PC->GetPawn();
    FVector TRexLocation = ControlledPawn->GetActorLocation();
    FVector PlayerLocation = PlayerPawn->GetActorLocation();

    // Distance check
    float Distance = FVector::Dist(TRexLocation, PlayerLocation);
    if (Distance > DetectionRange)
    {
        return false;
    }

    // Vision cone check — T-Rex has poor lateral vision
    FVector TRexForward = ControlledPawn->GetActorForwardVector();
    FVector DirToPlayer = (PlayerLocation - TRexLocation).GetSafeNormal();
    float DotProduct = FVector::DotProduct(TRexForward, DirToPlayer);
    float AngleToPlayer = FMath::RadiansToDegrees(FMath::Acos(DotProduct));

    if (AngleToPlayer > VisionConeAngle)
    {
        return false;
    }

    // Movement-based detection: stationary players are harder to detect
    if (bMovementBasedDetection)
    {
        ACharacter* PlayerChar = Cast<ACharacter>(PlayerPawn);
        if (PlayerChar && PlayerChar->GetCharacterMovement())
        {
            float PlayerSpeed = PlayerChar->GetCharacterMovement()->Velocity.Size();
            // If player is nearly still and distance > 1500 units, don't detect
            if (PlayerSpeed < 50.0f && Distance > 150000.0f)
            {
                return false;
            }
        }
    }

    // Line of sight check
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(ControlledPawn);
    QueryParams.AddIgnoredActor(PlayerPawn);

    bool bHit = World->LineTraceSingleByChannel(
        HitResult,
        TRexLocation + FVector(0, 0, 100.0f), // Eye height
        PlayerLocation,
        ECC_Visibility,
        QueryParams
    );

    if (bHit)
    {
        // Something blocking LOS
        return false;
    }

    // Player detected — write to blackboard
    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (BB)
    {
        BB->SetValueAsObject(TargetActorKey.SelectedKeyName, PlayerPawn);
    }

    UE_LOG(LogTemp, Log, TEXT("TRex DETECTED player at dist=%.0f angle=%.1f"), Distance, AngleToPlayer);

    return true;
}
