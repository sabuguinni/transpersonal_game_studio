#include "TRexBehaviorTree.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "NavigationSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

// ============================================================
// UNPC_BTTask_TRexPatrol
// ============================================================

UNPC_BTTask_TRexPatrol::UNPC_BTTask_TRexPatrol()
{
    NodeName = TEXT("TRex Patrol");
    bNotifyTick = false;
}

EBTNodeResult::Type UNPC_BTTask_TRexPatrol::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

    UWorld* World = ControlledPawn->GetWorld();
    if (!World)
    {
        return EBTNodeResult::Failed;
    }

    // Get home location from blackboard (set at spawn time)
    FVector HomeLocation = Blackboard->GetValueAsVector(HomeLocationKey.SelectedKeyName);
    if (HomeLocation.IsZero())
    {
        HomeLocation = ControlledPawn->GetActorLocation();
        Blackboard->SetValueAsVector(HomeLocationKey.SelectedKeyName, HomeLocation);
    }

    // Find a random reachable point within patrol radius
    UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(World);
    if (!NavSystem)
    {
        return EBTNodeResult::Failed;
    }

    FNavLocation RandomNavLocation;
    bool bFoundLocation = false;

    // Try up to 5 times to find a valid patrol point far enough from current position
    for (int32 Attempt = 0; Attempt < 5; ++Attempt)
    {
        if (NavSystem->GetRandomReachablePointInRadius(HomeLocation, PatrolRadius, RandomNavLocation))
        {
            float DistFromCurrent = FVector::Dist(ControlledPawn->GetActorLocation(), RandomNavLocation.Location);
            if (DistFromCurrent >= MinPatrolStepDistance)
            {
                bFoundLocation = true;
                break;
            }
        }
    }

    if (!bFoundLocation)
    {
        // Fallback: pick a point directly away from current heading
        FVector ForwardDir = ControlledPawn->GetActorForwardVector();
        FVector FallbackPoint = HomeLocation + ForwardDir * (PatrolRadius * 0.5f);
        Blackboard->SetValueAsVector(PatrolDestinationKey.SelectedKeyName, FallbackPoint);
        return EBTNodeResult::Succeeded;
    }

    Blackboard->SetValueAsVector(PatrolDestinationKey.SelectedKeyName, RandomNavLocation.Location);

#if WITH_EDITOR
    // Debug: draw patrol destination in editor
    DrawDebugSphere(World, RandomNavLocation.Location, 120.0f, 8, FColor::Green, false, 3.0f);
#endif

    return EBTNodeResult::Succeeded;
}

FString UNPC_BTTask_TRexPatrol::GetStaticDescription() const
{
    return FString::Printf(TEXT("T-Rex patrols within %.0f units of home.\nMin step: %.0f units."),
        PatrolRadius, MinPatrolStepDistance);
}

// ============================================================
// UNPC_BTTask_TRexChasePlayer
// ============================================================

UNPC_BTTask_TRexChasePlayer::UNPC_BTTask_TRexChasePlayer()
{
    NodeName = TEXT("TRex Chase Player");
    bNotifyTick = false;
}

EBTNodeResult::Type UNPC_BTTask_TRexChasePlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

    // Get target from blackboard
    AActor* TargetActor = Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName));
    if (!TargetActor)
    {
        return EBTNodeResult::Failed;
    }

    APawn* ControlledPawn = AIController->GetPawn();
    if (!ControlledPawn)
    {
        return EBTNodeResult::Failed;
    }

    // Check if target is still within give-up range
    float DistToTarget = FVector::Dist(ControlledPawn->GetActorLocation(), TargetActor->GetActorLocation());
    if (DistToTarget > GiveUpDistance)
    {
        // Lost the target — clear blackboard target and return failed to trigger patrol
        Blackboard->ClearValue(TargetActorKey.SelectedKeyName);
        return EBTNodeResult::Failed;
    }

    // Update last known player location
    Blackboard->SetValueAsVector(LastKnownPlayerLocationKey.SelectedKeyName, TargetActor->GetActorLocation());

    // Boost movement speed for chase
    ACharacter* TRexCharacter = Cast<ACharacter>(ControlledPawn);
    if (TRexCharacter && TRexCharacter->GetCharacterMovement())
    {
        float BaseSpeed = 600.0f; // T-Rex base walk speed
        TRexCharacter->GetCharacterMovement()->MaxWalkSpeed = BaseSpeed * ChaseSpeedMultiplier;
    }

    // Move to target
    EPathFollowingRequestResult::Type MoveResult = AIController->MoveToActor(
        TargetActor,
        /*AcceptanceRadius=*/ 300.0f,
        /*bStopOnOverlap=*/ true,
        /*bUsePathfinding=*/ true,
        /*bCanStrafe=*/ false
    );

    if (MoveResult == EPathFollowingRequestResult::Failed)
    {
        return EBTNodeResult::Failed;
    }

#if WITH_EDITOR
    DrawDebugLine(ControlledPawn->GetWorld(),
        ControlledPawn->GetActorLocation(),
        TargetActor->GetActorLocation(),
        FColor::Red, false, 0.5f, 0, 8.0f);
#endif

    return EBTNodeResult::Succeeded;
}

FString UNPC_BTTask_TRexChasePlayer::GetStaticDescription() const
{
    return FString::Printf(TEXT("T-Rex chases player at %.1fx speed.\nGives up at %.0f units."),
        ChaseSpeedMultiplier, GiveUpDistance);
}

// ============================================================
// UNPC_BTTask_TRexAttack
// ============================================================

UNPC_BTTask_TRexAttack::UNPC_BTTask_TRexAttack()
{
    NodeName = TEXT("TRex Attack");
    bNotifyTick = false;
}

EBTNodeResult::Type UNPC_BTTask_TRexAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

    APawn* ControlledPawn = AIController->GetPawn();
    if (!ControlledPawn)
    {
        return EBTNodeResult::Failed;
    }

    AActor* TargetActor = Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName));
    if (!TargetActor)
    {
        return EBTNodeResult::Failed;
    }

    // Check attack range
    float DistToTarget = FVector::Dist(ControlledPawn->GetActorLocation(), TargetActor->GetActorLocation());
    if (DistToTarget > AttackRange)
    {
        // Not close enough — return failed so BT switches to chase
        return EBTNodeResult::Failed;
    }

    // Check attack cooldown
    UWorld* World = ControlledPawn->GetWorld();
    if (!World)
    {
        return EBTNodeResult::Failed;
    }

    float CurrentTime = World->GetTimeSeconds();
    if (CurrentTime - LastAttackTime < AttackCooldown)
    {
        // Still on cooldown — succeed without dealing damage (animation continues)
        return EBTNodeResult::Succeeded;
    }

    LastAttackTime = CurrentTime;

    // Apply damage to target
    UGameplayStatics::ApplyDamage(
        TargetActor,
        BiteDamage,
        AIController,
        ControlledPawn,
        UDamageType::StaticClass()
    );

#if WITH_EDITOR
    DrawDebugSphere(World, TargetActor->GetActorLocation(), 80.0f, 8, FColor::Red, false, 1.0f);
    UE_LOG(LogTemp, Warning, TEXT("TRex BITE: %.0f damage to %s"), BiteDamage, *TargetActor->GetName());
#endif

    return EBTNodeResult::Succeeded;
}

FString UNPC_BTTask_TRexAttack::GetStaticDescription() const
{
    return FString::Printf(TEXT("T-Rex bites for %.0f damage.\nRange: %.0f units. Cooldown: %.1fs."),
        BiteDamage, AttackRange, AttackCooldown);
}

// ============================================================
// UNPC_BTDecorator_TRexSensePlayer
// ============================================================

UNPC_BTDecorator_TRexSensePlayer::UNPC_BTDecorator_TRexSensePlayer()
{
    NodeName = TEXT("TRex Sense Player");
    bNotifyBecomeRelevant = true;
    bNotifyCeaseRelevant = false;
}

bool UNPC_BTDecorator_TRexSensePlayer::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
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

    // Get the player character
    ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(World, 0);
    if (!PlayerCharacter)
    {
        return false;
    }

    FVector TRexLocation = ControlledPawn->GetActorLocation();
    FVector PlayerLocation = PlayerCharacter->GetActorLocation();
    float DistToPlayer = FVector::Dist(TRexLocation, PlayerLocation);

    // Sound detection — player within sound radius (movement-based)
    if (DistToPlayer <= SoundRadius)
    {
        UCharacterMovementComponent* PlayerMovement = PlayerCharacter->GetCharacterMovement();
        if (PlayerMovement && PlayerMovement->Velocity.SizeSquared() > 10000.0f) // Moving faster than ~100 cm/s
        {
            // Write target to blackboard
            UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
            if (Blackboard)
            {
                Blackboard->SetValueAsObject(TargetActorKey.SelectedKeyName, PlayerCharacter);
            }
            return true;
        }
    }

    // Sight detection — player within sight radius AND in vision cone
    if (DistToPlayer <= SightRadius)
    {
        FVector TRexForward = ControlledPawn->GetActorForwardVector();
        FVector ToPlayer = (PlayerLocation - TRexLocation).GetSafeNormal();
        float DotProduct = FVector::DotProduct(TRexForward, ToPlayer);
        float AngleToPlayer = FMath::RadiansToDegrees(FMath::Acos(DotProduct));

        if (AngleToPlayer <= VisionConeHalfAngle)
        {
            // Line-of-sight check
            FHitResult HitResult;
            FCollisionQueryParams QueryParams;
            QueryParams.AddIgnoredActor(ControlledPawn);

            bool bBlocked = World->LineTraceSingleByChannel(
                HitResult,
                TRexLocation + FVector(0, 0, 100), // Eye height
                PlayerLocation,
                ECC_Visibility,
                QueryParams
            );

            if (!bBlocked || HitResult.GetActor() == PlayerCharacter)
            {
                // Clear line of sight — T-Rex sees the player
                UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
                if (Blackboard)
                {
                    Blackboard->SetValueAsObject(TargetActorKey.SelectedKeyName, PlayerCharacter);
                }
                return true;
            }
        }
    }

    return false;
}

FString UNPC_BTDecorator_TRexSensePlayer::GetStaticDescription() const
{
    return FString::Printf(TEXT("T-Rex detects player.\nSight: %.0f units / %.0f° cone.\nSound: %.0f units."),
        SightRadius, VisionConeHalfAngle, SoundRadius);
}
