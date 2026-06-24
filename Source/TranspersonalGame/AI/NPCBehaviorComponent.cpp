#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz — sufficient for NPC logic
}

// ─────────────────────────────────────────────────────────────────────────────
// BeginPlay
// ─────────────────────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    // Default to patrol if patrol points exist, otherwise idle
    if (PatrolPoints.Num() > 0)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrol);
    }
    else
    {
        SetBehaviorState(ENPC_BehaviorState::Idle);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// TickComponent
// ─────────────────────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Decay threat memory over time
    UpdateMemoryDecay(DeltaTime);

    // Evaluate state transitions based on current memory
    EvaluateStateTransitions();

    // Per-state tick logic
    switch (CurrentState)
    {
    case ENPC_BehaviorState::Patrol:
        TickPatrol(DeltaTime);
        break;
    case ENPC_BehaviorState::Investigate:
        TickInvestigate(DeltaTime);
        break;
    default:
        break;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// State machine
// ─────────────────────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (CurrentState == NewState) return;

    // Reset timers on state entry
    if (NewState == ENPC_BehaviorState::Investigate)
    {
        InvestigateTimer = 0.0f;
    }
    if (NewState == ENPC_BehaviorState::Patrol)
    {
        PatrolWaitTimer = 0.0f;
    }

    CurrentState = NewState;
}

// ─────────────────────────────────────────────────────────────────────────────
// Threat perception
// ─────────────────────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::RegisterThreat(AActor* ThreatActor, ENPC_ThreatLevel Level, FVector Location)
{
    if (!ThreatActor) return;

    TrackedThreatActor = ThreatActor;
    ThreatMemory.LastKnownLocation = Location;
    ThreatMemory.TimeSinceLastSeen = 0.0f;
    ThreatMemory.ThreatLevel = Level;
    ThreatMemory.bIsActive = true;

    // Immediate state response based on threat level
    switch (Level)
    {
    case ENPC_ThreatLevel::Extreme:
    case ENPC_ThreatLevel::High:
        SetBehaviorState(ENPC_BehaviorState::Flee);
        break;
    case ENPC_ThreatLevel::Medium:
        SetBehaviorState(ENPC_BehaviorState::Alert);
        break;
    case ENPC_ThreatLevel::Low:
        SetBehaviorState(ENPC_BehaviorState::Investigate);
        break;
    default:
        break;
    }
}

void UNPCBehaviorComponent::ClearThreat()
{
    ThreatMemory.bIsActive = false;
    ThreatMemory.ThreatLevel = ENPC_ThreatLevel::None;
    TrackedThreatActor.Reset();

    // Return to patrol or idle
    if (PatrolPoints.Num() > 0)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrol);
    }
    else
    {
        SetBehaviorState(ENPC_BehaviorState::Idle);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Patrol
// ─────────────────────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::SetPatrolPoints(const TArray<FVector>& Points)
{
    PatrolPoints = Points;
    CurrentPatrolIndex = 0;

    if (Points.Num() > 0 && CurrentState == ENPC_BehaviorState::Idle)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrol);
    }
}

FVector UNPCBehaviorComponent::GetNextPatrolPoint() const
{
    if (PatrolPoints.Num() == 0) return FVector::ZeroVector;
    return PatrolPoints[CurrentPatrolIndex % PatrolPoints.Num()];
}

void UNPCBehaviorComponent::AdvancePatrolPoint()
{
    if (PatrolPoints.Num() == 0) return;
    CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
    PatrolWaitTimer = 0.0f;
}

// ─────────────────────────────────────────────────────────────────────────────
// Flee direction
// ─────────────────────────────────────────────────────────────────────────────

FVector UNPCBehaviorComponent::ComputeFleeDirection() const
{
    AActor* Owner = GetOwner();
    if (!Owner) return FVector::ForwardVector;

    FVector ThreatDir = Owner->GetActorLocation() - ThreatMemory.LastKnownLocation;
    ThreatDir.Z = 0.0f;

    if (ThreatDir.IsNearlyZero())
    {
        // Flee in the direction the NPC is facing if threat is on top
        return Owner->GetActorForwardVector();
    }

    return ThreatDir.GetSafeNormal();
}

// ─────────────────────────────────────────────────────────────────────────────
// Internal: memory decay
// ─────────────────────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::UpdateMemoryDecay(float DeltaTime)
{
    if (!ThreatMemory.bIsActive) return;

    ThreatMemory.TimeSinceLastSeen += DeltaTime;

    // Decay threat level over time
    const float DecayThreshold = 1.0f / FMath::Max(MemoryDecayRate, 0.001f);
    if (ThreatMemory.TimeSinceLastSeen > DecayThreshold * 3.0f)
    {
        ClearThreat();
    }
    else if (ThreatMemory.TimeSinceLastSeen > DecayThreshold * 2.0f)
    {
        if (ThreatMemory.ThreatLevel > ENPC_ThreatLevel::Low)
        {
            ThreatMemory.ThreatLevel = ENPC_ThreatLevel::Low;
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Internal: state transition evaluation
// ─────────────────────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::EvaluateStateTransitions()
{
    // If threat cleared naturally, return to base state
    if (!ThreatMemory.bIsActive)
    {
        if (CurrentState == ENPC_BehaviorState::Alert ||
            CurrentState == ENPC_BehaviorState::Flee  ||
            CurrentState == ENPC_BehaviorState::Hide)
        {
            if (PatrolPoints.Num() > 0)
                SetBehaviorState(ENPC_BehaviorState::Patrol);
            else
                SetBehaviorState(ENPC_BehaviorState::Idle);
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Internal: patrol tick
// ─────────────────────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::TickPatrol(float DeltaTime)
{
    if (PatrolPoints.Num() == 0) return;

    AActor* Owner = GetOwner();
    if (!Owner) return;

    FVector Target = GetNextPatrolPoint();
    float DistToTarget = FVector::Dist2D(Owner->GetActorLocation(), Target);

    // Arrived at patrol point — wait then advance
    if (DistToTarget < 150.0f)
    {
        PatrolWaitTimer += DeltaTime;
        if (PatrolWaitTimer >= PatrolWaitTime)
        {
            AdvancePatrolPoint();
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Internal: investigate tick
// ─────────────────────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::TickInvestigate(float DeltaTime)
{
    InvestigateTimer += DeltaTime;

    // Give up investigating after timeout
    if (InvestigateTimer >= InvestigateTimeout)
    {
        ClearThreat();
    }
}
