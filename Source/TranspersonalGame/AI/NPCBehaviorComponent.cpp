#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz — sufficient for NPC AI

    CurrentState = ENPC_BehaviorState::Idle;
    PreviousState = ENPC_BehaviorState::Idle;
    CurrentPatrolIndex = 0;
    StateTimer = 0.0f;
    MemoryCleanupTimer = 0.0f;
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    // Start in idle, let routine evaluation set the correct initial state
    SetBehaviorState(ENPC_BehaviorState::Idle);
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    StateTimer += DeltaTime;
    MemoryCleanupTimer += DeltaTime;

    // Clean expired memories every 5 seconds
    if (MemoryCleanupTimer >= 5.0f)
    {
        ClearExpiredMemories();
        MemoryCleanupTimer = 0.0f;
    }

    UpdateBehavior(DeltaTime);
}

void UNPCBehaviorComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (CurrentState == NewState) return;

    PreviousState = CurrentState;
    CurrentState = NewState;
    StateTimer = 0.0f;

    UE_LOG(LogTemp, Verbose, TEXT("NPC [%s] state: %d -> %d"),
        GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"),
        (int32)PreviousState, (int32)CurrentState);
}

void UNPCBehaviorComponent::RegisterThreat(AActor* ThreatActor, FVector Location,
    ENPC_ThreatLevel Level)
{
    if (!ThreatActor) return;

    // Update existing entry if this actor is already tracked
    for (FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        if (Entry.ThreatActor == ThreatActor)
        {
            Entry.LastKnownLocation = Location;
            Entry.TimeStamp = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
            Entry.ThreatLevel = Level;
            Entry.bIsActive = true;
            return;
        }
    }

    // Add new entry, evict oldest if at capacity
    if (MemoryEntries.Num() >= MaxMemoryEntries)
    {
        // Find oldest entry
        int32 OldestIdx = 0;
        float OldestTime = MemoryEntries[0].TimeStamp;
        for (int32 i = 1; i < MemoryEntries.Num(); i++)
        {
            if (MemoryEntries[i].TimeStamp < OldestTime)
            {
                OldestTime = MemoryEntries[i].TimeStamp;
                OldestIdx = i;
            }
        }
        MemoryEntries.RemoveAt(OldestIdx);
    }

    FNPC_MemoryEntry NewEntry;
    NewEntry.ThreatActor = ThreatActor;
    NewEntry.LastKnownLocation = Location;
    NewEntry.TimeStamp = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    NewEntry.ThreatLevel = Level;
    NewEntry.bIsActive = true;
    MemoryEntries.Add(NewEntry);

    // React to threat level
    if (Level >= ENPC_ThreatLevel::High)
    {
        SetBehaviorState(ENPC_BehaviorState::Flee);
    }
    else if (Level >= ENPC_ThreatLevel::Medium)
    {
        SetBehaviorState(ENPC_BehaviorState::Alert);
    }
}

void UNPCBehaviorComponent::ClearExpiredMemories()
{
    if (!GetWorld()) return;

    float CurrentTime = GetWorld()->GetTimeSeconds();
    MemoryEntries.RemoveAll([&](const FNPC_MemoryEntry& Entry)
    {
        return (CurrentTime - Entry.TimeStamp) > MemoryDuration;
    });
}

bool UNPCBehaviorComponent::HasActiveThreat() const
{
    for (const FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        if (Entry.bIsActive && Entry.ThreatLevel > ENPC_ThreatLevel::None)
        {
            return true;
        }
    }
    return false;
}

ENPC_ThreatLevel UNPCBehaviorComponent::GetHighestThreatLevel() const
{
    ENPC_ThreatLevel Highest = ENPC_ThreatLevel::None;
    for (const FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        if (Entry.bIsActive && Entry.ThreatLevel > Highest)
        {
            Highest = Entry.ThreatLevel;
        }
    }
    return Highest;
}

void UNPCBehaviorComponent::AddPatrolPoint(FVector Point)
{
    PatrolPoints.Add(Point);
}

FVector UNPCBehaviorComponent::GetNextPatrolPoint()
{
    if (PatrolPoints.Num() == 0)
    {
        return GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
    }

    FVector NextPoint = PatrolPoints[CurrentPatrolIndex];
    CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
    return NextPoint;
}

void UNPCBehaviorComponent::AddRoutineSlot(FNPC_DailyRoutineSlot Slot)
{
    DailyRoutine.Add(Slot);
}

void UNPCBehaviorComponent::EvaluateRoutine(float CurrentTimeOfDay)
{
    // Don't override threat-reactive states
    if (CurrentState == ENPC_BehaviorState::Flee ||
        CurrentState == ENPC_BehaviorState::Alert ||
        CurrentState == ENPC_BehaviorState::Dead)
    {
        return;
    }

    for (const FNPC_DailyRoutineSlot& Slot : DailyRoutine)
    {
        if (CurrentTimeOfDay >= Slot.TimeOfDayStart &&
            CurrentTimeOfDay < Slot.TimeOfDayEnd)
        {
            SetBehaviorState(Slot.DesiredState);
            return;
        }
    }
}

void UNPCBehaviorComponent::OnSightStimulus(AActor* SeenActor, float Strength)
{
    if (!SeenActor) return;

    // Classify threat based on actor type and strength
    ENPC_ThreatLevel Level = ENPC_ThreatLevel::None;

    if (Strength > 0.8f)
    {
        Level = ENPC_ThreatLevel::High;
    }
    else if (Strength > 0.5f)
    {
        Level = ENPC_ThreatLevel::Medium;
    }
    else if (Strength > 0.2f)
    {
        Level = ENPC_ThreatLevel::Low;
    }

    if (Level > ENPC_ThreatLevel::None)
    {
        RegisterThreat(SeenActor, SeenActor->GetActorLocation(), Level);
    }
}

void UNPCBehaviorComponent::OnHearingStimulus(FVector SoundLocation, float Loudness)
{
    // Loud sounds trigger alert state without a specific actor reference
    if (Loudness > 0.7f && CurrentState == ENPC_BehaviorState::Idle)
    {
        SetBehaviorState(ENPC_BehaviorState::Alert);
    }
    else if (Loudness > 0.9f)
    {
        SetBehaviorState(ENPC_BehaviorState::Flee);
    }
}

void UNPCBehaviorComponent::UpdateBehavior(float DeltaTime)
{
    switch (CurrentState)
    {
    case ENPC_BehaviorState::Idle:
        HandleIdleState(DeltaTime);
        break;
    case ENPC_BehaviorState::Patrol:
        HandlePatrolState(DeltaTime);
        break;
    case ENPC_BehaviorState::Flee:
        HandleFleeState(DeltaTime);
        break;
    case ENPC_BehaviorState::Alert:
        HandleAlertState(DeltaTime);
        break;
    default:
        break;
    }
}

void UNPCBehaviorComponent::HandleIdleState(float DeltaTime)
{
    // After 10 seconds idle, transition to patrol if patrol points exist
    if (StateTimer > 10.0f && PatrolPoints.Num() > 0)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrol);
    }
}

void UNPCBehaviorComponent::HandlePatrolState(float DeltaTime)
{
    // If a threat appears during patrol, react immediately
    if (HasActiveThreat())
    {
        ENPC_ThreatLevel Threat = GetHighestThreatLevel();
        if (Threat >= ENPC_ThreatLevel::High)
        {
            SetBehaviorState(ENPC_BehaviorState::Flee);
        }
        else
        {
            SetBehaviorState(ENPC_BehaviorState::Alert);
        }
    }
}

void UNPCBehaviorComponent::HandleFleeState(float DeltaTime)
{
    // After 20 seconds of fleeing with no active threat, calm down to alert
    if (StateTimer > 20.0f && !HasActiveThreat())
    {
        SetBehaviorState(ENPC_BehaviorState::Alert);
    }
}

void UNPCBehaviorComponent::HandleAlertState(float DeltaTime)
{
    // After 15 seconds of alert with no threat, return to previous state
    if (StateTimer > 15.0f && !HasActiveThreat())
    {
        if (PatrolPoints.Num() > 0)
        {
            SetBehaviorState(ENPC_BehaviorState::Patrol);
        }
        else
        {
            SetBehaviorState(ENPC_BehaviorState::Idle);
        }
    }
}
