#include "TRexBehaviorController.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

ATRexBehaviorController::ATRexBehaviorController()
{
    PrimaryActorTick.bCanEverTick = true;
    CurrentState = ENPC_TRexState::Patrol;
    CurrentTarget = nullptr;
    TimeSinceLastAttack = 0.0f;
    PatrolCenter = FVector::ZeroVector;
    CurrentPatrolDestination = FVector::ZeroVector;
}

void ATRexBehaviorController::BeginPlay()
{
    Super::BeginPlay();
    PatrolCenter = GetPawn() ? GetPawn()->GetActorLocation() : FVector::ZeroVector;
    CurrentPatrolDestination = GetRandomPatrolPoint();
}

void ATRexBehaviorController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    if (InPawn)
    {
        PatrolCenter = InPawn->GetActorLocation();
        CurrentPatrolDestination = GetRandomPatrolPoint();
        SetBehaviorState(ENPC_TRexState::Patrol);
    }
}

void ATRexBehaviorController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    TimeSinceLastAttack += DeltaTime;

    switch (CurrentState)
    {
        case ENPC_TRexState::Patrol:
            ScanForPlayer();
            UpdatePatrol(DeltaTime);
            break;
        case ENPC_TRexState::Alert:
            ScanForPlayer();
            break;
        case ENPC_TRexState::Chase:
            UpdateChase(DeltaTime);
            break;
        case ENPC_TRexState::Attack:
            if (TimeSinceLastAttack >= PatrolData.AttackCooldown)
            {
                ExecuteAttack();
            }
            break;
        case ENPC_TRexState::Rest:
            // Idle — do nothing
            break;
    }
}

void ATRexBehaviorController::SetBehaviorState(ENPC_TRexState NewState)
{
    if (CurrentState == NewState) return;
    CurrentState = NewState;

    APawn* MyPawn = GetPawn();
    if (!MyPawn) return;

    ACharacter* MyChar = Cast<ACharacter>(MyPawn);
    if (MyChar && MyChar->GetCharacterMovement())
    {
        switch (NewState)
        {
            case ENPC_TRexState::Patrol:
                MyChar->GetCharacterMovement()->MaxWalkSpeed = PatrolData.PatrolSpeed;
                break;
            case ENPC_TRexState::Chase:
                MyChar->GetCharacterMovement()->MaxWalkSpeed = PatrolData.ChaseSpeed;
                break;
            case ENPC_TRexState::Attack:
                MyChar->GetCharacterMovement()->MaxWalkSpeed = 0.0f;
                break;
            default:
                break;
        }
    }
}

void ATRexBehaviorController::StartPatrol()
{
    SetBehaviorState(ENPC_TRexState::Patrol);
    CurrentPatrolDestination = GetRandomPatrolPoint();
    MoveToLocation(CurrentPatrolDestination, 100.0f);
}

void ATRexBehaviorController::ChaseTarget(AActor* Target)
{
    if (!Target) return;
    CurrentTarget = Target;
    SetBehaviorState(ENPC_TRexState::Chase);
    MoveToActor(Target, PatrolData.AttackRange * 0.8f);
}

void ATRexBehaviorController::ExecuteAttack()
{
    if (!CurrentTarget) return;

    APawn* MyPawn = GetPawn();
    if (!MyPawn) return;

    float DistToTarget = FVector::Dist(MyPawn->GetActorLocation(), CurrentTarget->GetActorLocation());
    if (DistToTarget <= PatrolData.AttackRange)
    {
        // Apply damage to target
        UGameplayStatics::ApplyDamage(
            CurrentTarget,
            PatrolData.AttackDamage,
            this,
            MyPawn,
            UDamageType::StaticClass()
        );
        TimeSinceLastAttack = 0.0f;
    }
    else
    {
        // Target moved out of range — resume chase
        SetBehaviorState(ENPC_TRexState::Chase);
        MoveToActor(CurrentTarget, PatrolData.AttackRange * 0.8f);
    }
}

void ATRexBehaviorController::UpdatePatrol(float DeltaTime)
{
    APawn* MyPawn = GetPawn();
    if (!MyPawn) return;

    float DistToDest = FVector::Dist2D(MyPawn->GetActorLocation(), CurrentPatrolDestination);
    if (DistToDest < 200.0f)
    {
        // Reached patrol point — pick new one
        CurrentPatrolDestination = GetRandomPatrolPoint();
        MoveToLocation(CurrentPatrolDestination, 100.0f);
    }
}

void ATRexBehaviorController::UpdateChase(float DeltaTime)
{
    if (!CurrentTarget) 
    {
        SetBehaviorState(ENPC_TRexState::Patrol);
        return;
    }

    APawn* MyPawn = GetPawn();
    if (!MyPawn) return;

    float DistToTarget = FVector::Dist(MyPawn->GetActorLocation(), CurrentTarget->GetActorLocation());

    if (DistToTarget <= PatrolData.AttackRange)
    {
        SetBehaviorState(ENPC_TRexState::Attack);
    }
    else if (DistToTarget > PatrolData.ChaseDetectionRange * 1.5f)
    {
        // Lost the target — return to patrol
        CurrentTarget = nullptr;
        SetBehaviorState(ENPC_TRexState::Patrol);
        StartPatrol();
    }
    else
    {
        MoveToActor(CurrentTarget, PatrolData.AttackRange * 0.8f);
    }
}

void ATRexBehaviorController::ScanForPlayer()
{
    UWorld* World = GetWorld();
    if (!World) return;

    APawn* MyPawn = GetPawn();
    if (!MyPawn) return;

    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    if (!PC) return;

    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn) return;

    float DistToPlayer = FVector::Dist(MyPawn->GetActorLocation(), PlayerPawn->GetActorLocation());

    if (DistToPlayer <= PatrolData.ChaseDetectionRange)
    {
        ChaseTarget(PlayerPawn);
    }
}

FVector ATRexBehaviorController::GetRandomPatrolPoint() const
{
    float Angle = FMath::RandRange(0.0f, 360.0f);
    float Radius = FMath::RandRange(PatrolData.PatrolRadius * 0.3f, PatrolData.PatrolRadius);
    float X = PatrolCenter.X + Radius * FMath::Cos(FMath::DegreesToRadians(Angle));
    float Y = PatrolCenter.Y + Radius * FMath::Sin(FMath::DegreesToRadians(Angle));
    return FVector(X, Y, PatrolCenter.Z);
}
