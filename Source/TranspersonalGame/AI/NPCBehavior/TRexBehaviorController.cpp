#include "TRexBehaviorController.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"
#include "Engine/World.h"

ATRexBehaviorController::ATRexBehaviorController()
{
    PrimaryActorTick.bCanEverTick = true;
    CurrentState = ENPC_TRexState::Patrolling;
    CurrentWaypointIndex = 0;
    TimeSinceLastAttack = 0.0f;
}

void ATRexBehaviorController::BeginPlay()
{
    Super::BeginPlay();
    StartPatrol();
}

void ATRexBehaviorController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    StartPatrol();
}

void ATRexBehaviorController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    TimeSinceLastAttack += DeltaTime;
    UpdateBehavior(DeltaTime);
}

void ATRexBehaviorController::StartPatrol()
{
    CurrentState = ENPC_TRexState::Patrolling;
    CurrentTarget = nullptr;
    MoveToNextWaypoint();
}

void ATRexBehaviorController::ChaseTarget(AActor* Target)
{
    if (!Target) return;
    CurrentTarget = Target;
    CurrentState = ENPC_TRexState::Chasing;

    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) return;

    ACharacter* TRexChar = Cast<ACharacter>(ControlledPawn);
    if (TRexChar && TRexChar->GetCharacterMovement())
    {
        TRexChar->GetCharacterMovement()->MaxWalkSpeed = ChaseSpeed;
    }

    MoveToActor(Target, AttackRange * 0.8f);
}

void ATRexBehaviorController::AttackTarget(AActor* Target)
{
    if (!Target) return;
    if (TimeSinceLastAttack < AttackCooldown) return;

    CurrentState = ENPC_TRexState::Attacking;
    TimeSinceLastAttack = 0.0f;

    // Stop movement during attack
    StopMovement();

    // Attack logic: apply damage via gameplay damage system
    UGameplayStatics::ApplyDamage(
        Target,
        150.0f,         // T-Rex bite damage
        this,
        GetPawn(),
        nullptr
    );
}

void ATRexBehaviorController::UpdateBehavior(float DeltaTime)
{
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) return;

    switch (CurrentState)
    {
        case ENPC_TRexState::Patrolling:
        {
            ScanForPlayer();

            // Check if reached current waypoint
            if (PatrolWaypoints.IsValidIndex(CurrentWaypointIndex))
            {
                AActor* WP = PatrolWaypoints[CurrentWaypointIndex];
                if (WP)
                {
                    float DistToWP = FVector::Dist(ControlledPawn->GetActorLocation(), WP->GetActorLocation());
                    if (DistToWP < 200.0f)
                    {
                        CurrentWaypointIndex = (CurrentWaypointIndex + 1) % PatrolWaypoints.Num();
                        MoveToNextWaypoint();
                    }
                }
            }
            break;
        }

        case ENPC_TRexState::Chasing:
        {
            if (!CurrentTarget)
            {
                StartPatrol();
                break;
            }

            float DistToTarget = FVector::Dist(ControlledPawn->GetActorLocation(), CurrentTarget->GetActorLocation());

            if (DistToTarget <= AttackRange)
            {
                AttackTarget(CurrentTarget);
            }
            else if (DistToTarget > DetectionRange * 1.5f)
            {
                // Lost the target
                StartPatrol();
            }
            else
            {
                MoveToActor(CurrentTarget, AttackRange * 0.8f);
            }
            break;
        }

        case ENPC_TRexState::Attacking:
        {
            if (!CurrentTarget)
            {
                StartPatrol();
                break;
            }

            float DistToTarget = FVector::Dist(ControlledPawn->GetActorLocation(), CurrentTarget->GetActorLocation());
            if (DistToTarget > AttackRange * 1.2f)
            {
                CurrentState = ENPC_TRexState::Chasing;
            }
            else if (TimeSinceLastAttack >= AttackCooldown)
            {
                AttackTarget(CurrentTarget);
            }
            break;
        }

        case ENPC_TRexState::Resting:
        {
            // After rest period, resume patrol
            static float RestTimer = 0.0f;
            RestTimer += DeltaTime;
            if (RestTimer > 10.0f)
            {
                RestTimer = 0.0f;
                StartPatrol();
            }
            break;
        }

        default:
            break;
    }
}

void ATRexBehaviorController::ScanForPlayer()
{
    UWorld* World = GetWorld();
    if (!World) return;

    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) return;

    ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(World, 0);
    if (!PlayerChar) return;

    float DistToPlayer = FVector::Dist(ControlledPawn->GetActorLocation(), PlayerChar->GetActorLocation());
    if (DistToPlayer <= DetectionRange)
    {
        ChaseTarget(PlayerChar);
    }
}

void ATRexBehaviorController::MoveToNextWaypoint()
{
    if (PatrolWaypoints.Num() == 0)
    {
        // No waypoints — random nav point within patrol radius
        APawn* ControlledPawn = GetPawn();
        if (!ControlledPawn) return;

        UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
        if (!NavSys) return;

        FNavLocation RandomPoint;
        bool bFound = NavSys->GetRandomReachablePointInRadius(
            ControlledPawn->GetActorLocation(),
            PatrolRadius,
            RandomPoint
        );

        if (bFound)
        {
            MoveToLocation(RandomPoint.Location);
        }
        return;
    }

    if (PatrolWaypoints.IsValidIndex(CurrentWaypointIndex))
    {
        AActor* WP = PatrolWaypoints[CurrentWaypointIndex];
        if (WP)
        {
            MoveToActor(WP, 150.0f);
        }
    }
}

bool ATRexBehaviorController::IsPlayerInRange(float Range) const
{
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) return false;

    UWorld* World = GetWorld();
    if (!World) return false;

    ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(World, 0);
    if (!PlayerChar) return false;

    return FVector::Dist(ControlledPawn->GetActorLocation(), PlayerChar->GetActorLocation()) <= Range;
}
