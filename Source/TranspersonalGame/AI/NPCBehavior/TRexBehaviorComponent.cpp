#include "TRexBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

UNPCTRexBehaviorComponent::UNPCTRexBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz tick — sufficient for AI
}

void UNPCTRexBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    // Cache player reference
    CachedPlayer = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);

    // Start patrolling if waypoints exist
    if (PatrolWaypoints.Num() > 0)
    {
        SetState(ENPC_TRexState::Patrolling);
    }
    else
    {
        SetState(ENPC_TRexState::Idle);
    }
}

void UNPCTRexBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Refresh player cache periodically
    if (!CachedPlayer || !IsValid(CachedPlayer))
    {
        CachedPlayer = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    }

    // Update memory
    Memory.TimeSinceLastPlayerSight += DeltaTime;
    TimeSinceLastAttack += DeltaTime;
    StateTimer += DeltaTime;

    UpdateStateMachine(DeltaTime);
}

void UNPCTRexBehaviorComponent::UpdateStateMachine(float DeltaTime)
{
    switch (CurrentState)
    {
        case ENPC_TRexState::Idle:
            UpdateIdle(DeltaTime);
            break;
        case ENPC_TRexState::Patrolling:
            UpdatePatrolling(DeltaTime);
            break;
        case ENPC_TRexState::Investigating:
            UpdateInvestigating(DeltaTime);
            break;
        case ENPC_TRexState::Chasing:
            UpdateChasing(DeltaTime);
            break;
        case ENPC_TRexState::Attacking:
            UpdateAttacking(DeltaTime);
            break;
        case ENPC_TRexState::Feeding:
        case ENPC_TRexState::Resting:
            // Passive states — check for player intrusion
            if (CanSeePlayer())
            {
                SetState(ENPC_TRexState::Investigating);
            }
            break;
    }
}

void UNPCTRexBehaviorComponent::UpdateIdle(float DeltaTime)
{
    // After 5 seconds idle, start patrolling
    if (StateTimer > 5.0f && PatrolWaypoints.Num() > 0)
    {
        SetState(ENPC_TRexState::Patrolling);
        return;
    }

    // React to player if visible
    if (CanSeePlayer())
    {
        float Dist = GetDistanceToPlayer();
        if (Dist < Senses.AttackRadius)
        {
            SetState(ENPC_TRexState::Attacking);
        }
        else
        {
            SetState(ENPC_TRexState::Chasing);
        }
    }
}

void UNPCTRexBehaviorComponent::UpdatePatrolling(float DeltaTime)
{
    // Check for player first — priority over patrol
    if (CanSeePlayer())
    {
        float Dist = GetDistanceToPlayer();
        Memory.LastKnownPlayerLocation = CachedPlayer->GetActorLocation();
        Memory.bHasSeenPlayer = true;
        Memory.TimeSinceLastPlayerSight = 0.0f;
        OnPlayerDetected(CachedPlayer, Dist);

        if (Dist < Senses.AttackRadius)
        {
            SetState(ENPC_TRexState::Attacking);
        }
        else
        {
            SetState(ENPC_TRexState::Chasing);
        }
        return;
    }

    // Waypoint reached logic — check distance to current waypoint
    if (PatrolWaypoints.IsValidIndex(CurrentWaypointIndex))
    {
        AActor* TargetWP = PatrolWaypoints[CurrentWaypointIndex];
        if (!TargetWP || !IsValid(TargetWP)) return;

        AActor* Owner = GetOwner();
        if (!Owner) return;

        float DistToWP = FVector::Dist(Owner->GetActorLocation(), TargetWP->GetActorLocation());

        if (DistToWP < PatrolAcceptanceRadius)
        {
            // Wait at waypoint
            WaypointWaitTimer += DeltaTime;
            if (WaypointWaitTimer >= WaypointWaitTime)
            {
                WaypointWaitTimer = 0.0f;
                CurrentWaypointIndex = (CurrentWaypointIndex + 1) % PatrolWaypoints.Num();
            }
        }
    }
}

void UNPCTRexBehaviorComponent::UpdateInvestigating(float DeltaTime)
{
    // Move toward last known player location
    // If player found → chase; if too long → return to patrol
    if (CanSeePlayer())
    {
        Memory.LastKnownPlayerLocation = CachedPlayer->GetActorLocation();
        Memory.TimeSinceLastPlayerSight = 0.0f;
        SetState(ENPC_TRexState::Chasing);
        return;
    }

    // Give up after 15 seconds of no sighting
    if (StateTimer > 15.0f)
    {
        OnLostPlayer();
        SetState(ENPC_TRexState::Patrolling);
    }
}

void UNPCTRexBehaviorComponent::UpdateChasing(float DeltaTime)
{
    if (!CachedPlayer || !IsValid(CachedPlayer))
    {
        SetState(ENPC_TRexState::Investigating);
        return;
    }

    float Dist = GetDistanceToPlayer();

    // Update last known location
    if (CanSeePlayer())
    {
        Memory.LastKnownPlayerLocation = CachedPlayer->GetActorLocation();
        Memory.TimeSinceLastPlayerSight = 0.0f;
    }

    // Attack range reached
    if (Dist < Senses.AttackRadius)
    {
        SetState(ENPC_TRexState::Attacking);
        return;
    }

    // Player escaped
    if (Dist > Senses.ChaseBreakRadius)
    {
        OnLostPlayer();
        SetState(ENPC_TRexState::Investigating);
        return;
    }

    // Lost sight for too long → investigate last known position
    if (Memory.TimeSinceLastPlayerSight > 8.0f)
    {
        SetState(ENPC_TRexState::Investigating);
    }
}

void UNPCTRexBehaviorComponent::UpdateAttacking(float DeltaTime)
{
    if (!CachedPlayer || !IsValid(CachedPlayer))
    {
        SetState(ENPC_TRexState::Idle);
        return;
    }

    float Dist = GetDistanceToPlayer();

    // Player moved out of attack range → chase
    if (Dist > Senses.AttackRadius * 1.5f)
    {
        SetState(ENPC_TRexState::Chasing);
        return;
    }

    // Execute attack on cooldown
    if (TimeSinceLastAttack >= AttackCooldown)
    {
        TimeSinceLastAttack = 0.0f;
        OnAttackPlayer(CachedPlayer);
    }
}

void UNPCTRexBehaviorComponent::SetState(ENPC_TRexState NewState)
{
    if (NewState == CurrentState) return;

    ENPC_TRexState OldState = CurrentState;
    CurrentState = NewState;
    StateTimer = 0.0f;

    OnStateChanged(NewState, OldState);
}

void UNPCTRexBehaviorComponent::ForceState(ENPC_TRexState NewState)
{
    SetState(NewState);
}

float UNPCTRexBehaviorComponent::GetDistanceToPlayer() const
{
    if (!CachedPlayer || !IsValid(CachedPlayer)) return TNumericLimits<float>::Max();
    AActor* Owner = GetOwner();
    if (!Owner) return TNumericLimits<float>::Max();
    return FVector::Dist(Owner->GetActorLocation(), CachedPlayer->GetActorLocation());
}

bool UNPCTRexBehaviorComponent::CanSeePlayer() const
{
    if (!CachedPlayer || !IsValid(CachedPlayer)) return false;
    AActor* Owner = GetOwner();
    if (!Owner) return false;

    float Dist = GetDistanceToPlayer();
    if (Dist > Senses.SightRadius) return false;

    // Angle check — T-Rex has forward-facing vision cone
    FVector ToPlayer = (CachedPlayer->GetActorLocation() - Owner->GetActorLocation()).GetSafeNormal();
    FVector OwnerForward = Owner->GetActorForwardVector();
    float DotProduct = FVector::DotProduct(OwnerForward, ToPlayer);
    float HalfAngleCos = FMath::Cos(FMath::DegreesToRadians(Senses.SightAngleDegrees * 0.5f));

    return DotProduct >= HalfAngleCos;
}

AActor* UNPCTRexBehaviorComponent::FindPlayer() const
{
    return UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
}
