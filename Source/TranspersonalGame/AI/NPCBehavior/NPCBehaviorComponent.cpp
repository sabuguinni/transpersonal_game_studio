#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz tick — sufficient for NPC AI
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();
    CurrentState = ENPC_BehaviorState::Idle;
    PreviousState = ENPC_BehaviorState::Idle;
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update memory decay
    UpdateMemory(DeltaTime);

    // Evaluate state transitions based on current conditions
    EvaluateStateTransition();
}

// ============================================================
// STATE MACHINE
// ============================================================

void UNPCBehaviorComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (NewState == CurrentState)
    {
        return;
    }

    OnStateExit(CurrentState);
    PreviousState = CurrentState;
    CurrentState = NewState;
    OnStateEnter(NewState);
}

void UNPCBehaviorComponent::OnStateEnter(ENPC_BehaviorState NewState)
{
    // Reset fear accumulation when entering non-flee states
    if (NewState == ENPC_BehaviorState::Idle || NewState == ENPC_BehaviorState::Patrol)
    {
        FearLevel = FMath::Max(0.0f, FearLevel - 0.3f);
    }
    // Spike fear when entering flee state
    else if (NewState == ENPC_BehaviorState::Flee)
    {
        FearLevel = FMath::Min(1.0f, FearLevel + 0.5f);
    }
}

void UNPCBehaviorComponent::OnStateExit(ENPC_BehaviorState OldState)
{
    // Nothing required on exit for base implementation
    // Subclasses or Behavior Tree tasks can override logic here
}

void UNPCBehaviorComponent::EvaluateStateTransition()
{
    // Dead NPCs stay dead
    if (CurrentState == ENPC_BehaviorState::Dead)
    {
        return;
    }

    // Low health → flee
    if (HealthRatio < FleeThreshold && CurrentState != ENPC_BehaviorState::Flee)
    {
        SetBehaviorState(ENPC_BehaviorState::Flee);
        return;
    }

    float HighestThreat = GetHighestThreatLevel();

    // No threats → return to patrol or idle
    if (HighestThreat <= 0.0f)
    {
        if (PatrolPoints.Num() > 0 && CurrentState != ENPC_BehaviorState::Patrol)
        {
            SetBehaviorState(ENPC_BehaviorState::Patrol);
        }
        else if (PatrolPoints.Num() == 0 && CurrentState != ENPC_BehaviorState::Idle)
        {
            SetBehaviorState(ENPC_BehaviorState::Idle);
        }
        return;
    }

    // Threat detected — escalate state based on threat level
    if (HighestThreat >= 0.8f)
    {
        // High threat → attack or chase
        AActor* Threat = GetHighestThreat();
        if (Threat)
        {
            float DistToThreat = FVector::Dist(GetOwner()->GetActorLocation(), Threat->GetActorLocation());
            if (DistToThreat <= AttackRange)
            {
                SetBehaviorState(ENPC_BehaviorState::Attack);
            }
            else
            {
                SetBehaviorState(ENPC_BehaviorState::Chase);
            }
        }
    }
    else if (HighestThreat >= 0.4f)
    {
        // Medium threat → alert or investigate
        if (CurrentState == ENPC_BehaviorState::Idle || CurrentState == ENPC_BehaviorState::Patrol)
        {
            SetBehaviorState(ENPC_BehaviorState::Alert);
        }
    }
    else
    {
        // Low threat → investigate
        if (CurrentState == ENPC_BehaviorState::Idle || CurrentState == ENPC_BehaviorState::Patrol)
        {
            SetBehaviorState(ENPC_BehaviorState::Investigate);
        }
    }
}

bool UNPCBehaviorComponent::IsHostile() const
{
    return CurrentState == ENPC_BehaviorState::Chase ||
           CurrentState == ENPC_BehaviorState::Attack;
}

// ============================================================
// THREAT DETECTION
// ============================================================

void UNPCBehaviorComponent::RegisterThreat(AActor* ThreatActor, float ThreatLevel, FVector LastKnownLoc)
{
    if (!ThreatActor)
    {
        return;
    }

    // Check if we already have a memory entry for this actor
    for (FNPC_MemoryEntry& Entry : ThreatMemory)
    {
        if (Entry.SourceActor == ThreatActor)
        {
            Entry.ThreatLevel = FMath::Max(Entry.ThreatLevel, ThreatLevel);
            Entry.LastKnownLocation = LastKnownLoc;
            Entry.TimeSinceLastSeen = 0.0f;
            Entry.bIsVisible = true;
            return;
        }
    }

    // New threat — add to memory
    FNPC_MemoryEntry NewEntry;
    NewEntry.SourceActor = ThreatActor;
    NewEntry.ThreatLevel = ThreatLevel;
    NewEntry.LastKnownLocation = LastKnownLoc;
    NewEntry.TimeSinceLastSeen = 0.0f;
    NewEntry.bIsVisible = true;
    ThreatMemory.Add(NewEntry);
}

void UNPCBehaviorComponent::ClearThreat(AActor* ThreatActor)
{
    ThreatMemory.RemoveAll([ThreatActor](const FNPC_MemoryEntry& Entry)
    {
        return Entry.SourceActor == ThreatActor;
    });
}

float UNPCBehaviorComponent::GetHighestThreatLevel() const
{
    float Highest = 0.0f;
    for (const FNPC_MemoryEntry& Entry : ThreatMemory)
    {
        Highest = FMath::Max(Highest, Entry.ThreatLevel);
    }
    return Highest;
}

AActor* UNPCBehaviorComponent::GetHighestThreat() const
{
    AActor* BestActor = nullptr;
    float BestThreat = 0.0f;

    for (const FNPC_MemoryEntry& Entry : ThreatMemory)
    {
        if (Entry.ThreatLevel > BestThreat && Entry.SourceActor != nullptr)
        {
            BestThreat = Entry.ThreatLevel;
            BestActor = Entry.SourceActor;
        }
    }
    return BestActor;
}

// ============================================================
// PATROL
// ============================================================

void UNPCBehaviorComponent::AddPatrolPoint(FVector Location, float WaitTime, bool bLookAround)
{
    FNPC_PatrolPoint Point;
    Point.Location = Location;
    Point.WaitTime = WaitTime;
    Point.bLookAround = bLookAround;
    PatrolPoints.Add(Point);
}

FVector UNPCBehaviorComponent::GetNextPatrolPoint()
{
    if (PatrolPoints.Num() == 0)
    {
        return GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
    }
    return PatrolPoints[CurrentPatrolIndex].Location;
}

void UNPCBehaviorComponent::AdvancePatrolIndex()
{
    if (PatrolPoints.Num() == 0)
    {
        return;
    }

    CurrentPatrolIndex++;
    if (CurrentPatrolIndex >= PatrolPoints.Num())
    {
        CurrentPatrolIndex = bLoopPatrol ? 0 : PatrolPoints.Num() - 1;
    }
}

// ============================================================
// MEMORY
// ============================================================

void UNPCBehaviorComponent::UpdateMemory(float DeltaTime)
{
    for (int32 i = ThreatMemory.Num() - 1; i >= 0; i--)
    {
        FNPC_MemoryEntry& Entry = ThreatMemory[i];

        // Increment time since last seen
        Entry.TimeSinceLastSeen += DeltaTime;

        // Decay threat level for non-visible threats
        if (!Entry.bIsVisible)
        {
            Entry.ThreatLevel -= MemoryDecayRate * DeltaTime;
        }

        // Mark as not visible each tick — must be re-confirmed by perception system
        Entry.bIsVisible = false;

        // Remove forgotten memories
        if (Entry.ThreatLevel <= 0.0f || Entry.TimeSinceLastSeen >= MemoryForgetTime)
        {
            ThreatMemory.RemoveAt(i);
        }
    }
}

bool UNPCBehaviorComponent::HasMemoryOf(AActor* Actor) const
{
    for (const FNPC_MemoryEntry& Entry : ThreatMemory)
    {
        if (Entry.SourceActor == Actor)
        {
            return true;
        }
    }
    return false;
}
