#include "NPCBehaviorComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

    CurrentState      = ENPC_BehaviorState::Idle;
    PersonalityType   = ENPC_PersonalityType::Scout;

    PatrolSpeed       = 200.f;
    RunSpeed          = 500.f;

    SightRadius       = 2000.f;
    HearingRadius     = 800.f;
    ThreatFleeRadius  = 3000.f;

    MemoryDecayRate   = 0.1f;   // seconds subtracted per tick
    MaxMemoryEntries  = 8;
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    // Scouts start patrolling immediately; Elders begin idle
    if (PersonalityType == ENPC_PersonalityType::Scout)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrolling);
    }
    else
    {
        SetBehaviorState(ENPC_BehaviorState::Idle);
    }
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    TickMemoryDecay(DeltaTime);
    EvaluateStateTransitions();
}

// ---------------------------------------------------------------------------
// State management
// ---------------------------------------------------------------------------

void UNPCBehaviorComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (CurrentState == NewState)
    {
        return;
    }

    CurrentState = NewState;

    // Adjust movement speed based on state
    ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
    if (OwnerChar && OwnerChar->GetCharacterMovement())
    {
        UCharacterMovementComponent* MoveComp = OwnerChar->GetCharacterMovement();
        switch (CurrentState)
        {
            case ENPC_BehaviorState::Patrolling:
            case ENPC_BehaviorState::Idle:
            case ENPC_BehaviorState::Investigating:
                MoveComp->MaxWalkSpeed = PatrolSpeed;
                break;
            case ENPC_BehaviorState::Fleeing:
            case ENPC_BehaviorState::Alerted:
                MoveComp->MaxWalkSpeed = RunSpeed;
                break;
            default:
                MoveComp->MaxWalkSpeed = PatrolSpeed;
                break;
        }
    }
}

// ---------------------------------------------------------------------------
// Patrol
// ---------------------------------------------------------------------------

void UNPCBehaviorComponent::AdvancePatrolWaypoint()
{
    if (PatrolRoute.Waypoints.Num() == 0)
    {
        return;
    }

    PatrolRoute.CurrentWaypointIndex =
        (PatrolRoute.CurrentWaypointIndex + 1) % PatrolRoute.Waypoints.Num();
}

FVector UNPCBehaviorComponent::GetCurrentPatrolTarget() const
{
    if (PatrolRoute.Waypoints.Num() == 0)
    {
        return GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
    }

    int32 Idx = FMath::Clamp(PatrolRoute.CurrentWaypointIndex, 0, PatrolRoute.Waypoints.Num() - 1);
    return PatrolRoute.Waypoints[Idx];
}

// ---------------------------------------------------------------------------
// Memory
// ---------------------------------------------------------------------------

void UNPCBehaviorComponent::RegisterThreat(FVector ThreatLocation, FString ThreatTag)
{
    // Update existing entry if same tag
    for (FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        if (Entry.ThreatTag == ThreatTag)
        {
            Entry.LastKnownLocation = ThreatLocation;
            Entry.TimeSinceObserved = 0.f;
            Entry.bIsThreat = true;
            SetBehaviorState(ENPC_BehaviorState::Alerted);
            return;
        }
    }

    // Add new entry (cap at MaxMemoryEntries)
    if (MemoryEntries.Num() >= MaxMemoryEntries)
    {
        MemoryEntries.RemoveAt(0); // drop oldest
    }

    FNPC_MemoryEntry NewEntry;
    NewEntry.LastKnownLocation = ThreatLocation;
    NewEntry.TimeSinceObserved = 0.f;
    NewEntry.bIsThreat = true;
    NewEntry.ThreatTag = ThreatTag;
    MemoryEntries.Add(NewEntry);

    SetBehaviorState(ENPC_BehaviorState::Alerted);
}

bool UNPCBehaviorComponent::HasActiveThreatMemory() const
{
    for (const FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        if (Entry.bIsThreat && Entry.TimeSinceObserved < 30.f)
        {
            return true;
        }
    }
    return false;
}

FVector UNPCBehaviorComponent::GetLastKnownThreatLocation() const
{
    float MostRecent = FLT_MAX;
    FVector Result = FVector::ZeroVector;

    for (const FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        if (Entry.bIsThreat && Entry.TimeSinceObserved < MostRecent)
        {
            MostRecent = Entry.TimeSinceObserved;
            Result = Entry.LastKnownLocation;
        }
    }

    return Result;
}

// ---------------------------------------------------------------------------
// Internal tick helpers
// ---------------------------------------------------------------------------

void UNPCBehaviorComponent::TickMemoryDecay(float DeltaTime)
{
    for (FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        Entry.TimeSinceObserved += DeltaTime;
    }

    // Remove entries older than 60 seconds
    MemoryEntries.RemoveAll([](const FNPC_MemoryEntry& E)
    {
        return E.TimeSinceObserved > 60.f;
    });
}

void UNPCBehaviorComponent::EvaluateStateTransitions()
{
    // If we had an active threat but memory has decayed, return to patrol/idle
    if (CurrentState == ENPC_BehaviorState::Alerted && !HasActiveThreatMemory())
    {
        if (PersonalityType == ENPC_PersonalityType::Scout)
        {
            SetBehaviorState(ENPC_BehaviorState::Patrolling);
        }
        else
        {
            SetBehaviorState(ENPC_BehaviorState::Idle);
        }
    }

    if (CurrentState == ENPC_BehaviorState::Fleeing && !HasActiveThreatMemory())
    {
        SetBehaviorState(ENPC_BehaviorState::Investigating);
    }
}
