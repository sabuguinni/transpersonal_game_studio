#include "TRexBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

// ── Constructor ──────────────────────────────────────────────────────────────
UNPC_TRexBehaviorComponent::UNPC_TRexBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.05f; // 20 Hz — sufficient for AI
}

// ── BeginPlay ────────────────────────────────────────────────────────────────
void UNPC_TRexBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    if (AActor* Owner = GetOwner())
    {
        HomeLocation = Owner->GetActorLocation();
    }

    // Start in patrol state
    SetState(ENPC_TRexState::Patrol);
    PickNewPatrolTarget();
}

// ── TickComponent ─────────────────────────────────────────────────────────────
void UNPC_TRexBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    TimeSinceLastAttack += DeltaTime;

    switch (CurrentState)
    {
        case ENPC_TRexState::Idle:   TickIdle(DeltaTime);   break;
        case ENPC_TRexState::Patrol: TickPatrol(DeltaTime); break;
        case ENPC_TRexState::Alert:  TickAlert(DeltaTime);  break;
        case ENPC_TRexState::Chase:  TickChase(DeltaTime);  break;
        case ENPC_TRexState::Attack: TickAttack(DeltaTime); break;
        case ENPC_TRexState::Return: TickReturn(DeltaTime); break;
        case ENPC_TRexState::Dead:   /* no tick */           break;
        default: break;
    }
}

// ── State Machine ─────────────────────────────────────────────────────────────
void UNPC_TRexBehaviorComponent::TickIdle(float DeltaTime)
{
    // After 3 seconds idle, resume patrol
    static float IdleTimer = 0.f;
    IdleTimer += DeltaTime;
    if (IdleTimer >= 3.f)
    {
        IdleTimer = 0.f;
        SetState(ENPC_TRexState::Patrol);
        PickNewPatrolTarget();
    }

    // Scan for player
    if (IsPlayerInDetectionRange())
    {
        IdleTimer = 0.f;
        SetState(ENPC_TRexState::Alert);
    }
}

void UNPC_TRexBehaviorComponent::TickPatrol(float DeltaTime)
{
    // Check for player first
    if (IsPlayerInDetectionRange())
    {
        SetState(ENPC_TRexState::Alert);
        return;
    }

    AActor* Owner = GetOwner();
    if (!Owner) return;

    FVector OwnerLoc = Owner->GetActorLocation();
    float DistToTarget = FVector::Dist(OwnerLoc, CurrentPatrolTarget);

    if (DistToTarget < 200.f)
    {
        // Reached patrol point — idle briefly
        SetState(ENPC_TRexState::Idle);
        return;
    }

    // Move toward patrol target
    FVector Direction = (CurrentPatrolTarget - OwnerLoc).GetSafeNormal();
    Owner->SetActorLocation(OwnerLoc + Direction * PatrolSpeed * DeltaTime, true);

    // Face movement direction
    FRotator LookAt = UKismetMathLibrary::FindLookAtRotation(OwnerLoc, CurrentPatrolTarget);
    Owner->SetActorRotation(FMath::RInterpTo(Owner->GetActorRotation(), LookAt, DeltaTime, 3.f));
}

void UNPC_TRexBehaviorComponent::TickAlert(float DeltaTime)
{
    // Alert: T-Rex has spotted player — roar and transition to chase
    TriggerRoar();
    SetState(ENPC_TRexState::Chase);
}

void UNPC_TRexBehaviorComponent::TickChase(float DeltaTime)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    // Re-acquire target if lost
    if (!TargetActor)
    {
        TargetActor = FindNearestPlayer();
    }

    if (!TargetActor)
    {
        SetState(ENPC_TRexState::Return);
        return;
    }

    float Dist = GetDistanceToTarget();

    // Lost player — return home
    if (Dist > DetectionRange * 1.5f)
    {
        TargetActor = nullptr;
        SetState(ENPC_TRexState::Return);
        return;
    }

    // Close enough to attack
    if (Dist <= AttackRange)
    {
        SetState(ENPC_TRexState::Attack);
        return;
    }

    // Chase movement
    FVector OwnerLoc = Owner->GetActorLocation();
    FVector TargetLoc = TargetActor->GetActorLocation();
    FVector Direction = (TargetLoc - OwnerLoc).GetSafeNormal();

    Owner->SetActorLocation(OwnerLoc + Direction * ChaseSpeed * DeltaTime, true);

    FRotator LookAt = UKismetMathLibrary::FindLookAtRotation(OwnerLoc, TargetLoc);
    Owner->SetActorRotation(FMath::RInterpTo(Owner->GetActorRotation(), LookAt, DeltaTime, 5.f));
}

void UNPC_TRexBehaviorComponent::TickAttack(float DeltaTime)
{
    if (!TargetActor)
    {
        SetState(ENPC_TRexState::Chase);
        return;
    }

    float Dist = GetDistanceToTarget();

    // Target escaped attack range — chase again
    if (Dist > AttackRange * 1.5f)
    {
        SetState(ENPC_TRexState::Chase);
        return;
    }

    // Cooldown-gated attack
    if (TimeSinceLastAttack >= AttackCooldown)
    {
        TimeSinceLastAttack = 0.f;
        int32 AttackIdx = FMath::RandRange(0, 2); // 3 attack animations
        TriggerAttack(AttackIdx);

        // Apply damage to target
        if (ACharacter* TargetChar = Cast<ACharacter>(TargetActor))
        {
            UGameplayStatics::ApplyDamage(TargetActor, AttackDamage,
                                          nullptr, GetOwner(), nullptr);
        }
    }
}

void UNPC_TRexBehaviorComponent::TickReturn(float DeltaTime)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    FVector OwnerLoc = Owner->GetActorLocation();
    float DistHome = FVector::Dist(OwnerLoc, HomeLocation);

    if (DistHome < 200.f)
    {
        SetState(ENPC_TRexState::Idle);
        return;
    }

    FVector Direction = (HomeLocation - OwnerLoc).GetSafeNormal();
    Owner->SetActorLocation(OwnerLoc + Direction * PatrolSpeed * DeltaTime, true);

    FRotator LookAt = UKismetMathLibrary::FindLookAtRotation(OwnerLoc, HomeLocation);
    Owner->SetActorRotation(FMath::RInterpTo(Owner->GetActorRotation(), LookAt, DeltaTime, 3.f));

    // If player re-enters range during return, re-engage
    if (IsPlayerInDetectionRange())
    {
        SetState(ENPC_TRexState::Alert);
    }
}

// ── Public API ────────────────────────────────────────────────────────────────
void UNPC_TRexBehaviorComponent::SetState(ENPC_TRexState NewState)
{
    if (NewState == CurrentState) return;

    ENPC_TRexState OldState = CurrentState;
    CurrentState = NewState;

    float Speed = (NewState == ENPC_TRexState::Chase) ? ChaseSpeed : PatrolSpeed;
    bool bAttacking = (NewState == ENPC_TRexState::Attack);
    bool bRoaring   = (NewState == ENPC_TRexState::Alert);

    OnStateChanged.Broadcast(NewState, OldState, Speed, bAttacking, bRoaring);
}

void UNPC_TRexBehaviorComponent::SetTarget(AActor* NewTarget)
{
    TargetActor = NewTarget;
    if (NewTarget && CurrentState == ENPC_TRexState::Patrol)
    {
        SetState(ENPC_TRexState::Alert);
    }
}

void UNPC_TRexBehaviorComponent::TriggerRoar()
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    FVector Origin = Owner->GetActorLocation();
    OnRoar.Broadcast(Origin, RoarRadius);
}

void UNPC_TRexBehaviorComponent::TriggerAttack(int32 AttackIndex)
{
    OnAttackMontage.Broadcast(AttackIndex);
}

bool UNPC_TRexBehaviorComponent::IsPlayerInDetectionRange() const
{
    AActor* Player = FindNearestPlayer();
    if (!Player) return false;
    return GetDistanceToTarget() <= DetectionRange;
}

bool UNPC_TRexBehaviorComponent::IsPlayerInAttackRange() const
{
    if (!TargetActor) return false;
    return GetDistanceToTarget() <= AttackRange;
}

float UNPC_TRexBehaviorComponent::GetDistanceToTarget() const
{
    AActor* Owner = GetOwner();
    if (!Owner) return TNumericLimits<float>::Max();

    AActor* Target = TargetActor ? TargetActor : FindNearestPlayer();
    if (!Target) return TNumericLimits<float>::Max();

    return FVector::Dist(Owner->GetActorLocation(), Target->GetActorLocation());
}

// ── Private Helpers ───────────────────────────────────────────────────────────
void UNPC_TRexBehaviorComponent::PickNewPatrolTarget()
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    FVector RandomOffset = FVector(
        FMath::RandRange(-PatrolRadius, PatrolRadius),
        FMath::RandRange(-PatrolRadius, PatrolRadius),
        0.f
    );

    CurrentPatrolTarget = HomeLocation + RandomOffset;
    bPatrolTargetReached = false;
}

AActor* UNPC_TRexBehaviorComponent::FindNearestPlayer() const
{
    AActor* Owner = GetOwner();
    if (!Owner) return nullptr;

    UWorld* World = Owner->GetWorld();
    if (!World) return nullptr;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return nullptr;

    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn) return nullptr;

    // Cache target for next detection check
    const_cast<UNPC_TRexBehaviorComponent*>(this)->TargetActor = PlayerPawn;
    return PlayerPawn;
}
