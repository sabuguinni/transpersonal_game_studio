#include "TRexBehaviorController.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

ATRexBehaviorController::ATRexBehaviorController()
{
    PrimaryActorTick.bCanEverTick = true;
    CurrentState = ENPC_TRexState::Idle;
    CurrentTarget = nullptr;
    CurrentWaypointIndex = 0;
    LastAttackTime = -999.0f;
    StateTimer = 0.0f;
    PatrolOrigin = FVector::ZeroVector;
}

void ATRexBehaviorController::BeginPlay()
{
    Super::BeginPlay();
    PatrolOrigin = GetPatrolOrigin();
    SetBehaviorState(ENPC_TRexState::Patrolling);
}

void ATRexBehaviorController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    StateTimer += DeltaTime;

    // Scan for player every 0.5 seconds
    if (FMath::Fmod(StateTimer, 0.5f) < DeltaTime)
    {
        ScanForPlayer();
    }

    switch (CurrentState)
    {
        case ENPC_TRexState::Patrolling:
            UpdatePatrolBehavior(DeltaTime);
            break;
        case ENPC_TRexState::Chasing:
            UpdateChaseBehavior(DeltaTime);
            break;
        case ENPC_TRexState::Attacking:
            UpdateAttackBehavior(DeltaTime);
            break;
        default:
            break;
    }
}

void ATRexBehaviorController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    if (InPawn)
    {
        PatrolOrigin = InPawn->GetActorLocation();
        UE_LOG(LogTemp, Log, TEXT("TRexBehaviorController: Possessing %s at origin (%.0f, %.0f, %.0f)"),
            *InPawn->GetName(), PatrolOrigin.X, PatrolOrigin.Y, PatrolOrigin.Z);
    }
}

void ATRexBehaviorController::SetBehaviorState(ENPC_TRexState NewState)
{
    if (CurrentState == NewState) return;

    UE_LOG(LogTemp, Log, TEXT("TRex state: %d -> %d"), (int32)CurrentState, (int32)NewState);
    CurrentState = NewState;
    StateTimer = 0.0f;

    // State entry actions
    switch (NewState)
    {
        case ENPC_TRexState::Patrolling:
            CurrentTarget = nullptr;
            if (PatrolWaypoints.Num() > 0)
            {
                MoveToActor(PatrolWaypoints[CurrentWaypointIndex], 100.0f);
            }
            break;
        case ENPC_TRexState::Chasing:
            if (CurrentTarget)
            {
                MoveToActor(CurrentTarget, Senses.AttackRadius * 0.8f);
            }
            break;
        case ENPC_TRexState::Attacking:
            StopMovement();
            break;
        default:
            break;
    }
}

void ATRexBehaviorController::SetPatrolTarget(AActor* NewTarget)
{
    CurrentTarget = NewTarget;
}

bool ATRexBehaviorController::CanSeePlayer() const
{
    APawn* Player = GetPlayerPawn();
    if (!Player || !GetPawn()) return false;

    float Dist = FVector::Dist(GetPawn()->GetActorLocation(), Player->GetActorLocation());
    if (Dist > Senses.SightRadius) return false;

    // Check sight angle
    FVector ToPlayer = (Player->GetActorLocation() - GetPawn()->GetActorLocation()).GetSafeNormal();
    FVector Forward = GetPawn()->GetActorForwardVector();
    float DotProduct = FVector::DotProduct(Forward, ToPlayer);
    float AngleDeg = FMath::RadiansToDegrees(FMath::Acos(DotProduct));

    return AngleDeg <= (Senses.SightAngleDegrees * 0.5f);
}

float ATRexBehaviorController::GetDistanceToPlayer() const
{
    APawn* Player = GetPlayerPawn();
    if (!Player || !GetPawn()) return TNumericLimits<float>::Max();
    return FVector::Dist(GetPawn()->GetActorLocation(), Player->GetActorLocation());
}

void ATRexBehaviorController::UpdatePatrolBehavior(float DeltaTime)
{
    if (PatrolWaypoints.Num() == 0) return;

    // Check if reached waypoint
    EPathFollowingStatus::Type MoveStatus = GetMoveStatus();
    if (MoveStatus == EPathFollowingStatus::Idle || MoveStatus == EPathFollowingStatus::Waiting)
    {
        // Advance to next waypoint
        CurrentWaypointIndex = (CurrentWaypointIndex + 1) % PatrolWaypoints.Num();
        AActor* NextWP = PatrolWaypoints[CurrentWaypointIndex];
        if (NextWP)
        {
            MoveToActor(NextWP, 100.0f);
        }
    }

    // Check if player is visible — switch to chase
    if (CanSeePlayer())
    {
        APawn* Player = GetPlayerPawn();
        if (Player)
        {
            CurrentTarget = Player;
            SetBehaviorState(ENPC_TRexState::Chasing);
        }
    }
}

void ATRexBehaviorController::UpdateChaseBehavior(float DeltaTime)
{
    if (!CurrentTarget) 
    {
        SetBehaviorState(ENPC_TRexState::Patrolling);
        return;
    }

    float DistToTarget = GetDistanceToPlayer();

    // Within attack range
    if (DistToTarget <= Senses.AttackRadius)
    {
        SetBehaviorState(ENPC_TRexState::Attacking);
        return;
    }

    // Lost sight — return to patrol
    if (DistToTarget > Senses.SightRadius * 1.5f)
    {
        SetBehaviorState(ENPC_TRexState::Patrolling);
        return;
    }

    // Keep chasing
    MoveToActor(CurrentTarget, Senses.AttackRadius * 0.8f);
}

void ATRexBehaviorController::UpdateAttackBehavior(float DeltaTime)
{
    if (!CurrentTarget)
    {
        SetBehaviorState(ENPC_TRexState::Patrolling);
        return;
    }

    float DistToTarget = GetDistanceToPlayer();

    // Target escaped attack range — chase again
    if (DistToTarget > Senses.AttackRadius * 1.5f)
    {
        SetBehaviorState(ENPC_TRexState::Chasing);
        return;
    }

    // Perform attack on cooldown
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    if (CurrentTime - LastAttackTime >= AttackCooldown)
    {
        LastAttackTime = CurrentTime;
        UE_LOG(LogTemp, Log, TEXT("TRex ATTACKS! Damage: %.0f"), AttackDamage);
        // Damage application handled by Combat Agent (#12)
    }
}

void ATRexBehaviorController::ScanForPlayer()
{
    APawn* Player = GetPlayerPawn();
    if (!Player || !GetPawn()) return;

    float Dist = FVector::Dist(GetPawn()->GetActorLocation(), Player->GetActorLocation());

    // Hearing detection (no line of sight required)
    if (Dist <= Senses.HearingRadius && CurrentState == ENPC_TRexState::Patrolling)
    {
        CurrentTarget = Player;
        SetBehaviorState(ENPC_TRexState::Chasing);
        return;
    }

    // Sight detection
    if (CanSeePlayer() && CurrentState == ENPC_TRexState::Patrolling)
    {
        CurrentTarget = Player;
        SetBehaviorState(ENPC_TRexState::Chasing);
    }
}

APawn* ATRexBehaviorController::GetPlayerPawn() const
{
    UWorld* World = GetWorld();
    if (!World) return nullptr;
    APlayerController* PC = World->GetFirstPlayerController();
    return PC ? PC->GetPawn() : nullptr;
}

FVector ATRexBehaviorController::GetPatrolOrigin() const
{
    if (GetPawn())
    {
        return GetPawn()->GetActorLocation();
    }
    return FVector::ZeroVector;
}
