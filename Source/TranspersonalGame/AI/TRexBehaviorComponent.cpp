// TRexBehaviorComponent.cpp
// Agent #11 — NPC Behavior Agent | PROD_CYCLE_AUTO_20260629_007
// T-Rex patrol, chase, and attack behavior implementation

#include "TRexBehaviorComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "TimerManager.h"

UNPC_TRexBehaviorComponent::UNPC_TRexBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

    // Default behavioral parameters
    PatrolRadius = 5000.0f;
    ChaseDetectionRange = 3000.0f;
    AttackRange = 300.0f;
    PatrolSpeed = 300.0f;
    ChaseSpeed = 700.0f;
    AttackCooldown = 2.5f;
    RoarRadius = 4000.0f;

    CurrentState = ENPC_TRexState::Idle;
    bCanAttack = true;
    PatrolOrigin = FVector::ZeroVector;
    CurrentPatrolTarget = FVector::ZeroVector;
    TargetPawn = nullptr;
}

void UNPC_TRexBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    // Record patrol origin at spawn location
    if (AActor* Owner = GetOwner())
    {
        PatrolOrigin = Owner->GetActorLocation();
        CurrentPatrolTarget = PatrolOrigin;
    }

    // Start idle → patrol transition after short delay
    GetWorld()->GetTimerManager().SetTimer(
        PatrolTimerHandle,
        this,
        &UNPC_TRexBehaviorComponent::PickNewPatrolTarget,
        3.0f,
        false
    );
}

void UNPC_TRexBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (CurrentState == ENPC_TRexState::Dead)
    {
        return;
    }

    ScanForPlayer();
    UpdateBehaviorState(DeltaTime);
}

void UNPC_TRexBehaviorComponent::ScanForPlayer()
{
    if (CurrentState == ENPC_TRexState::Dead)
    {
        return;
    }

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    // Get player pawn
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }

    float DistToPlayer = FVector::Dist(Owner->GetActorLocation(), PlayerPawn->GetActorLocation());

    // Chase trigger
    if (DistToPlayer <= ChaseDetectionRange)
    {
        if (CurrentState != ENPC_TRexState::Chase && CurrentState != ENPC_TRexState::Attack)
        {
            TargetPawn = PlayerPawn;
            TransitionToState(ENPC_TRexState::Alert);

            // Roar on first detection
            OnTRexRoar.Broadcast(Owner->GetActorLocation(), RoarRadius);

            // Short alert pause before chase
            GetWorld()->GetTimerManager().SetTimer(
                AlertTimerHandle,
                this,
                &UNPC_TRexBehaviorComponent::BeginChase,
                1.2f,
                false
            );
        }
    }
    else if (DistToPlayer > ChaseDetectionRange * 1.5f)
    {
        // Lost player — return to patrol
        if (CurrentState == ENPC_TRexState::Chase || CurrentState == ENPC_TRexState::Alert)
        {
            TargetPawn = nullptr;
            TransitionToState(ENPC_TRexState::Return);
        }
    }
}

void UNPC_TRexBehaviorComponent::UpdateBehaviorState(float DeltaTime)
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    switch (CurrentState)
    {
        case ENPC_TRexState::Patrol:
            ExecutePatrol(DeltaTime);
            break;

        case ENPC_TRexState::Chase:
            ExecuteChase(DeltaTime);
            break;

        case ENPC_TRexState::Attack:
            ExecuteAttack();
            break;

        case ENPC_TRexState::Return:
            ExecuteReturn(DeltaTime);
            break;

        default:
            break;
    }
}

void UNPC_TRexBehaviorComponent::ExecutePatrol(float DeltaTime)
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    FVector OwnerLoc = Owner->GetActorLocation();
    float DistToTarget = FVector::Dist2D(OwnerLoc, CurrentPatrolTarget);

    if (DistToTarget < 200.0f)
    {
        // Reached patrol point — pick a new one after a pause
        TransitionToState(ENPC_TRexState::Idle);
        GetWorld()->GetTimerManager().SetTimer(
            PatrolTimerHandle,
            this,
            &UNPC_TRexBehaviorComponent::PickNewPatrolTarget,
            FMath::RandRange(4.0f, 8.0f),
            false
        );
    }
    else
    {
        // Move toward patrol target
        FVector Direction = (CurrentPatrolTarget - OwnerLoc).GetSafeNormal2D();
        Owner->AddActorWorldOffset(Direction * PatrolSpeed * DeltaTime, true);

        // Face movement direction
        FRotator TargetRot = Direction.Rotation();
        Owner->SetActorRotation(FMath::RInterpTo(Owner->GetActorRotation(), TargetRot, DeltaTime, 2.0f));
    }
}

void UNPC_TRexBehaviorComponent::ExecuteChase(float DeltaTime)
{
    if (!TargetPawn)
    {
        TransitionToState(ENPC_TRexState::Return);
        return;
    }

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    FVector OwnerLoc = Owner->GetActorLocation();
    FVector PlayerLoc = TargetPawn->GetActorLocation();
    float DistToPlayer = FVector::Dist(OwnerLoc, PlayerLoc);

    if (DistToPlayer <= AttackRange)
    {
        TransitionToState(ENPC_TRexState::Attack);
        return;
    }

    // Move toward player
    FVector Direction = (PlayerLoc - OwnerLoc).GetSafeNormal2D();
    Owner->AddActorWorldOffset(Direction * ChaseSpeed * DeltaTime, true);

    // Face player
    FRotator TargetRot = Direction.Rotation();
    Owner->SetActorRotation(FMath::RInterpTo(Owner->GetActorRotation(), TargetRot, DeltaTime, 5.0f));

    OnTRexAnimStateChanged.Broadcast(ENPC_TRexState::Chase, ChaseSpeed, 0.0f, 0.0f, false);
}

void UNPC_TRexBehaviorComponent::ExecuteAttack()
{
    if (!bCanAttack || !TargetPawn)
    {
        return;
    }

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    float DistToPlayer = FVector::Dist(Owner->GetActorLocation(), TargetPawn->GetActorLocation());
    if (DistToPlayer > AttackRange * 1.5f)
    {
        TransitionToState(ENPC_TRexState::Chase);
        return;
    }

    // Trigger attack
    bCanAttack = false;
    int32 AttackIndex = FMath::RandRange(0, 2); // 3 attack animations
    OnTRexAttackMontage.Broadcast(AttackIndex);

    // Apply damage to player
    if (AActor* PlayerActor = Cast<AActor>(TargetPawn))
    {
        UGameplayStatics::ApplyDamage(PlayerActor, 35.0f, nullptr, Owner, nullptr);
    }

    // Reset attack cooldown
    GetWorld()->GetTimerManager().SetTimer(
        AttackCooldownHandle,
        this,
        &UNPC_TRexBehaviorComponent::ResetAttackCooldown,
        AttackCooldown,
        false
    );
}

void UNPC_TRexBehaviorComponent::ExecuteReturn(float DeltaTime)
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    FVector OwnerLoc = Owner->GetActorLocation();
    float DistToOrigin = FVector::Dist2D(OwnerLoc, PatrolOrigin);

    if (DistToOrigin < 300.0f)
    {
        // Back at origin — resume patrol
        PickNewPatrolTarget();
        return;
    }

    // Move back to patrol origin
    FVector Direction = (PatrolOrigin - OwnerLoc).GetSafeNormal2D();
    Owner->AddActorWorldOffset(Direction * PatrolSpeed * DeltaTime, true);

    FRotator TargetRot = Direction.Rotation();
    Owner->SetActorRotation(FMath::RInterpTo(Owner->GetActorRotation(), TargetRot, DeltaTime, 3.0f));
}

void UNPC_TRexBehaviorComponent::TransitionToState(ENPC_TRexState NewState)
{
    if (CurrentState == NewState)
    {
        return;
    }

    CurrentState = NewState;
    OnTRexAnimStateChanged.Broadcast(NewState, 0.0f, 0.0f, 0.0f, false);
}

void UNPC_TRexBehaviorComponent::PickNewPatrolTarget()
{
    // Pick random point within patrol radius
    float Angle = FMath::RandRange(0.0f, 360.0f);
    float Distance = FMath::RandRange(PatrolRadius * 0.3f, PatrolRadius);
    float RadAngle = FMath::DegreesToRadians(Angle);

    CurrentPatrolTarget = PatrolOrigin + FVector(
        FMath::Cos(RadAngle) * Distance,
        FMath::Sin(RadAngle) * Distance,
        0.0f
    );

    TransitionToState(ENPC_TRexState::Patrol);
}

void UNPC_TRexBehaviorComponent::BeginChase()
{
    if (TargetPawn)
    {
        TransitionToState(ENPC_TRexState::Chase);
    }
}

void UNPC_TRexBehaviorComponent::ResetAttackCooldown()
{
    bCanAttack = true;
}

void UNPC_TRexBehaviorComponent::OnTRexDeath()
{
    TransitionToState(ENPC_TRexState::Dead);
    GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
    SetComponentTickEnabled(false);
}
