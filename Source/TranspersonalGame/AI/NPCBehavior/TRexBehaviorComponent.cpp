#include "TRexBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"

UTRexBehaviorComponent::UTRexBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    CurrentState = ENPC_TRexState::Patrolling;
    CurrentPatrolTarget = FVector::ZeroVector;
}

void UTRexBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    // Store patrol origin at spawn location
    if (AActor* Owner = GetOwner())
    {
        PatrolOrigin = Owner->GetActorLocation();
        CurrentPatrolTarget = GetRandomPatrolPoint();
    }

    // Cache player reference
    CachedPlayerActor = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
}

void UTRexBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!IsAlive()) return;

    // Update timers
    TimeSinceLastAttack += DeltaTime;

    // Refresh player cache periodically
    if (!CachedPlayerActor)
    {
        CachedPlayerActor = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    }

    // Update sensory detection
    UpdateSensoryDetection();

    // State machine
    switch (CurrentState)
    {
    case ENPC_TRexState::Idle:
    case ENPC_TRexState::Patrolling:
        if (IsPlayerInSightRange())
        {
            SetState(ENPC_TRexState::Chasing);
        }
        else
        {
            UpdatePatrolBehavior(DeltaTime);
        }
        break;

    case ENPC_TRexState::Investigating:
        if (IsPlayerInSightRange())
        {
            SetState(ENPC_TRexState::Chasing);
        }
        else
        {
            // Return to patrol after investigation
            SetState(ENPC_TRexState::Patrolling);
        }
        break;

    case ENPC_TRexState::Chasing:
        if (IsPlayerInAttackRange())
        {
            SetState(ENPC_TRexState::Attacking);
        }
        else if (!IsPlayerInSightRange())
        {
            // Lost sight — investigate last known position
            SetState(ENPC_TRexState::Investigating);
        }
        else
        {
            UpdateChaseBehavior(DeltaTime);
        }
        break;

    case ENPC_TRexState::Attacking:
        if (!IsPlayerInAttackRange())
        {
            SetState(ENPC_TRexState::Chasing);
        }
        else
        {
            UpdateAttackBehavior(DeltaTime);
        }
        break;

    case ENPC_TRexState::Feeding:
    case ENPC_TRexState::Resting:
        // Passive states — still react to nearby player
        if (IsPlayerInSightRange() && GetDistanceToPlayer() < SensoryData.SightRange * 0.5f)
        {
            SetState(ENPC_TRexState::Chasing);
        }
        break;

    default:
        break;
    }
}

void UTRexBehaviorComponent::SetState(ENPC_TRexState NewState)
{
    if (CurrentState == NewState) return;

    CurrentState = NewState;

    // State entry actions
    switch (NewState)
    {
    case ENPC_TRexState::Patrolling:
        CurrentPatrolTarget = GetRandomPatrolPoint();
        bWaitingAtPatrolPoint = false;
        break;
    case ENPC_TRexState::Chasing:
        if (CachedPlayerActor)
        {
            SensoryData.LastKnownPlayerLocation = CachedPlayerActor->GetActorLocation();
        }
        break;
    default:
        break;
    }
}

void UTRexBehaviorComponent::UpdatePatrolBehavior(float DeltaTime)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    if (bWaitingAtPatrolPoint)
    {
        PatrolWaitTimer -= DeltaTime;
        if (PatrolWaitTimer <= 0.0f)
        {
            bWaitingAtPatrolPoint = false;
            CurrentPatrolTarget = GetRandomPatrolPoint();
        }
        return;
    }

    FVector OwnerLoc = Owner->GetActorLocation();
    float DistToTarget = FVector::Dist(OwnerLoc, CurrentPatrolTarget);

    if (DistToTarget < 200.0f)
    {
        // Reached patrol point — wait
        bWaitingAtPatrolPoint = true;
        PatrolWaitTimer = PatrolWaitTime;
    }
    else
    {
        // Move toward patrol target
        FVector Direction = (CurrentPatrolTarget - OwnerLoc).GetSafeNormal();
        FVector NewLocation = OwnerLoc + Direction * CombatStats.PatrolSpeed * DeltaTime;
        Owner->SetActorLocation(NewLocation, true);

        // Face movement direction
        FRotator NewRotation = Direction.Rotation();
        Owner->SetActorRotation(NewRotation);
    }
}

void UTRexBehaviorComponent::UpdateChaseBehavior(float DeltaTime)
{
    AActor* Owner = GetOwner();
    if (!Owner || !CachedPlayerActor) return;

    FVector OwnerLoc = Owner->GetActorLocation();
    FVector PlayerLoc = CachedPlayerActor->GetActorLocation();

    // Update last known position
    SensoryData.LastKnownPlayerLocation = PlayerLoc;

    // Move toward player
    FVector Direction = (PlayerLoc - OwnerLoc).GetSafeNormal();
    FVector NewLocation = OwnerLoc + Direction * CombatStats.ChaseSpeed * DeltaTime;
    Owner->SetActorLocation(NewLocation, true);

    // Face player
    FRotator NewRotation = Direction.Rotation();
    Owner->SetActorRotation(NewRotation);
}

void UTRexBehaviorComponent::UpdateAttackBehavior(float DeltaTime)
{
    if (TimeSinceLastAttack < CombatStats.AttackCooldown) return;
    if (!CachedPlayerActor) return;

    // Apply damage to player
    ACharacter* PlayerChar = Cast<ACharacter>(CachedPlayerActor);
    if (PlayerChar)
    {
        UGameplayStatics::ApplyDamage(
            PlayerChar,
            CombatStats.AttackDamage,
            nullptr,
            GetOwner(),
            nullptr
        );
        TimeSinceLastAttack = 0.0f;
    }
}

void UTRexBehaviorComponent::UpdateSensoryDetection()
{
    if (!CachedPlayerActor)
    {
        SensoryData.bPlayerDetected = false;
        return;
    }

    float Dist = GetDistanceToPlayer();
    SensoryData.bPlayerDetected = (Dist <= SensoryData.SightRange);
}

bool UTRexBehaviorComponent::IsPlayerInSightRange() const
{
    return SensoryData.bPlayerDetected && GetDistanceToPlayer() <= SensoryData.SightRange;
}

bool UTRexBehaviorComponent::IsPlayerInAttackRange() const
{
    return GetDistanceToPlayer() <= CombatStats.AttackRange;
}

float UTRexBehaviorComponent::GetDistanceToPlayer() const
{
    AActor* Owner = GetOwner();
    if (!Owner || !CachedPlayerActor) return TNumericLimits<float>::Max();
    return FVector::Dist(Owner->GetActorLocation(), CachedPlayerActor->GetActorLocation());
}

void UTRexBehaviorComponent::TakeDamage_TRex(float DamageAmount)
{
    CombatStats.CurrentHealth = FMath::Max(0.0f, CombatStats.CurrentHealth - DamageAmount);

    if (!IsAlive())
    {
        SetState(ENPC_TRexState::Idle);
    }
    else if (CurrentState == ENPC_TRexState::Patrolling || CurrentState == ENPC_TRexState::Resting)
    {
        // Aggro on damage
        SetState(ENPC_TRexState::Chasing);
    }
}

FVector UTRexBehaviorComponent::GetRandomPatrolPoint() const
{
    float Angle = FMath::RandRange(0.0f, 360.0f);
    float Radius = FMath::RandRange(PatrolRadius * 0.3f, PatrolRadius);
    float X = PatrolOrigin.X + Radius * FMath::Cos(FMath::DegreesToRadians(Angle));
    float Y = PatrolOrigin.Y + Radius * FMath::Sin(FMath::DegreesToRadians(Angle));
    return FVector(X, Y, PatrolOrigin.Z);
}

FString UTRexBehaviorComponent::GetStateAsString() const
{
    switch (CurrentState)
    {
    case ENPC_TRexState::Idle:          return TEXT("Idle");
    case ENPC_TRexState::Patrolling:    return TEXT("Patrolling");
    case ENPC_TRexState::Investigating: return TEXT("Investigating");
    case ENPC_TRexState::Chasing:       return TEXT("Chasing");
    case ENPC_TRexState::Attacking:     return TEXT("Attacking");
    case ENPC_TRexState::Feeding:       return TEXT("Feeding");
    case ENPC_TRexState::Resting:       return TEXT("Resting");
    default:                            return TEXT("Unknown");
    }
}
