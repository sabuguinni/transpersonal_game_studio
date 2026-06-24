#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    CurrentHealth = MaxHealth;
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();
    CurrentHealth = MaxHealth;
    SetBehaviorState(ENPC_BehaviorState::Idle);
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (CurrentState == ENPC_BehaviorState::Dead)
    {
        return;
    }

    // Update memory timers
    if (Memory.bHasSeenPlayer)
    {
        Memory.TimeSinceLastPlayerSighting += DeltaTime;
        // Forget player after 15 seconds of no sighting
        if (Memory.TimeSinceLastPlayerSighting > 15.0f)
        {
            OnPlayerLost();
        }
    }

    // Decay threat level over time
    if (Memory.ThreatLevel > 0.0f)
    {
        Memory.ThreatLevel = FMath::Max(0.0f, Memory.ThreatLevel - DeltaTime * 0.1f);
    }

    // Tick current state
    switch (CurrentState)
    {
        case ENPC_BehaviorState::Idle:   TickIdle(DeltaTime);   break;
        case ENPC_BehaviorState::Patrol: TickPatrol(DeltaTime); break;
        case ENPC_BehaviorState::Alert:  TickAlert(DeltaTime);  break;
        case ENPC_BehaviorState::Chase:  TickChase(DeltaTime);  break;
        case ENPC_BehaviorState::Attack: TickAttack(DeltaTime); break;
        case ENPC_BehaviorState::Flee:   TickFlee(DeltaTime);   break;
        default: break;
    }

    EvaluateStateTransitions();
}

void UNPCBehaviorComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (CurrentState == NewState) return;
    CurrentState = NewState;
}

void UNPCBehaviorComponent::OnPlayerDetected(const FVector& PlayerLocation)
{
    Memory.LastKnownPlayerLocation = PlayerLocation;
    Memory.TimeSinceLastPlayerSighting = 0.0f;
    Memory.bHasSeenPlayer = true;
    Memory.bIsAlerted = true;

    // Personality-based reaction
    switch (Personality)
    {
        case ENPC_DinoPersonality::TerritorialPatrol:
            SetBehaviorState(ENPC_BehaviorState::Chase);
            break;
        case ENPC_DinoPersonality::PackHunter:
            SetBehaviorState(ENPC_BehaviorState::Alert);
            break;
        case ENPC_DinoPersonality::GrazerDefensive:
            SetBehaviorState(ENPC_BehaviorState::Alert);
            break;
        case ENPC_DinoPersonality::PassiveHerd:
            SetBehaviorState(ENPC_BehaviorState::Flee);
            break;
        case ENPC_DinoPersonality::AlertFlee:
            SetBehaviorState(ENPC_BehaviorState::Flee);
            break;
    }
}

void UNPCBehaviorComponent::OnPlayerLost()
{
    Memory.bHasSeenPlayer = false;
    Memory.bIsAlerted = false;
    Memory.TimeSinceLastPlayerSighting = 0.0f;

    // Return to patrol or idle
    if (PatrolData.WaypointLocations.Num() > 0)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrol);
    }
    else
    {
        SetBehaviorState(ENPC_BehaviorState::Idle);
    }
}

void UNPCBehaviorComponent::OnTakeDamage(float DamageAmount)
{
    CurrentHealth = FMath::Max(0.0f, CurrentHealth - DamageAmount);
    UpdateThreatLevel(DamageAmount * 0.1f);

    if (CurrentHealth <= 0.0f)
    {
        SetBehaviorState(ENPC_BehaviorState::Dead);
        return;
    }

    // Flee if health drops below threshold
    float HealthRatio = CurrentHealth / MaxHealth;
    if (HealthRatio <= FleeHealthThreshold)
    {
        if (Personality != ENPC_DinoPersonality::TerritorialPatrol)
        {
            SetBehaviorState(ENPC_BehaviorState::Flee);
        }
    }
    else if (CurrentState == ENPC_BehaviorState::Patrol || CurrentState == ENPC_BehaviorState::Idle)
    {
        SetBehaviorState(ENPC_BehaviorState::Alert);
    }
}

void UNPCBehaviorComponent::SetPatrolWaypoints(const TArray<FVector>& Waypoints)
{
    PatrolData.WaypointLocations = Waypoints;
    PatrolData.CurrentWaypointIndex = 0;
    if (Waypoints.Num() > 0)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrol);
    }
}

FVector UNPCBehaviorComponent::GetNextWaypointLocation() const
{
    if (PatrolData.WaypointLocations.Num() == 0)
    {
        return GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
    }
    int32 Idx = PatrolData.CurrentWaypointIndex % PatrolData.WaypointLocations.Num();
    return PatrolData.WaypointLocations[Idx];
}

void UNPCBehaviorComponent::AdvanceToNextWaypoint()
{
    if (PatrolData.WaypointLocations.Num() == 0) return;
    PatrolData.CurrentWaypointIndex = (PatrolData.CurrentWaypointIndex + 1) % PatrolData.WaypointLocations.Num();
    bWaitingAtWaypoint = false;
    WaypointWaitTimer = 0.0f;
}

void UNPCBehaviorComponent::UpdateThreatLevel(float Delta)
{
    Memory.ThreatLevel = FMath::Clamp(Memory.ThreatLevel + Delta, 0.0f, 10.0f);
}

// === Private tick methods ===

void UNPCBehaviorComponent::TickIdle(float DeltaTime)
{
    // Idle: stand still, occasionally look around
    // Transition to patrol if waypoints exist
    if (PatrolData.WaypointLocations.Num() > 0)
    {
        WaypointWaitTimer += DeltaTime;
        if (WaypointWaitTimer >= PatrolData.WaitTimeAtWaypoint)
        {
            WaypointWaitTimer = 0.0f;
            SetBehaviorState(ENPC_BehaviorState::Patrol);
        }
    }
}

void UNPCBehaviorComponent::TickPatrol(float DeltaTime)
{
    if (PatrolData.WaypointLocations.Num() == 0) return;

    if (bWaitingAtWaypoint)
    {
        WaypointWaitTimer += DeltaTime;
        if (WaypointWaitTimer >= PatrolData.WaitTimeAtWaypoint)
        {
            AdvanceToNextWaypoint();
        }
        return;
    }

    // Check if we reached the current waypoint
    AActor* Owner = GetOwner();
    if (!Owner) return;

    FVector CurrentWP = GetNextWaypointLocation();
    float DistToWP = FVector::Dist2D(Owner->GetActorLocation(), CurrentWP);

    if (DistToWP <= PatrolData.WaypointAcceptanceRadius)
    {
        bWaitingAtWaypoint = true;
        WaypointWaitTimer = 0.0f;
    }
}

void UNPCBehaviorComponent::TickAlert(float DeltaTime)
{
    // Alert: scan for player, decide whether to chase or flee
    float DistToPlayer = GetDistanceToPlayer();

    if (DistToPlayer <= AttackRadius)
    {
        SetBehaviorState(ENPC_BehaviorState::Attack);
    }
    else if (DistToPlayer <= DetectionRadius)
    {
        // Aggressive personalities chase, passive ones flee
        if (Personality == ENPC_DinoPersonality::TerritorialPatrol ||
            Personality == ENPC_DinoPersonality::PackHunter ||
            Personality == ENPC_DinoPersonality::GrazerDefensive)
        {
            SetBehaviorState(ENPC_BehaviorState::Chase);
        }
        else
        {
            SetBehaviorState(ENPC_BehaviorState::Flee);
        }
    }
}

void UNPCBehaviorComponent::TickChase(float DeltaTime)
{
    float DistToPlayer = GetDistanceToPlayer();

    if (DistToPlayer <= AttackRadius)
    {
        SetBehaviorState(ENPC_BehaviorState::Attack);
    }
    else if (DistToPlayer > DetectionRadius * 1.5f)
    {
        // Lost the player
        OnPlayerLost();
    }
}

void UNPCBehaviorComponent::TickAttack(float DeltaTime)
{
    float DistToPlayer = GetDistanceToPlayer();

    if (DistToPlayer > AttackRadius * 1.5f)
    {
        // Player escaped attack range — chase again
        SetBehaviorState(ENPC_BehaviorState::Chase);
    }
}

void UNPCBehaviorComponent::TickFlee(float DeltaTime)
{
    float DistToPlayer = GetDistanceToPlayer();

    // Stop fleeing when far enough
    if (DistToPlayer > DetectionRadius * 2.0f)
    {
        Memory.bIsAlerted = false;
        SetBehaviorState(ENPC_BehaviorState::Idle);
    }
}

void UNPCBehaviorComponent::EvaluateStateTransitions()
{
    if (CurrentState == ENPC_BehaviorState::Dead) return;

    // Passive herds never attack
    if (Personality == ENPC_DinoPersonality::PassiveHerd &&
        CurrentState == ENPC_BehaviorState::Attack)
    {
        SetBehaviorState(ENPC_BehaviorState::Flee);
    }
}

float UNPCBehaviorComponent::GetDistanceToPlayer() const
{
    AActor* Owner = GetOwner();
    if (!Owner) return TNumericLimits<float>::Max();

    UWorld* World = Owner->GetWorld();
    if (!World) return TNumericLimits<float>::Max();

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn) return TNumericLimits<float>::Max();

    return FVector::Dist(Owner->GetActorLocation(), PlayerPawn->GetActorLocation());
}
