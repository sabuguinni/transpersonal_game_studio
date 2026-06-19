#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz — sufficient for NPC logic

    CurrentState = ENPC_BehaviorState::Idle;
    CurrentThreatLevel = ENPC_ThreatLevel::None;
    TimeInCurrentState = 0.0f;
    CurrentPatrolIndex = 0;
    AlertTimer = 0.0f;
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();
    TransitionToState(ENPC_BehaviorState::Idle);
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    TimeInCurrentState += DeltaTime;

    TickMemoryDecay(DeltaTime);
    TickAlertTimer(DeltaTime);
}

// --- State Machine ---

void UNPCBehaviorComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (NewState != CurrentState)
    {
        TransitionToState(NewState);
    }
}

void UNPCBehaviorComponent::TransitionToState(ENPC_BehaviorState NewState)
{
    CurrentState = NewState;
    TimeInCurrentState = 0.0f;
}

void UNPCBehaviorComponent::OnThreatDetected(AActor* ThreatActor, ENPC_ThreatLevel Level)
{
    if (!ThreatActor) return;

    CurrentThreatLevel = Level;

    // Update memory immediately
    if (ThreatActor)
    {
        UpdateMemory(ThreatActor, ThreatActor->GetActorLocation(), Level);
    }

    // State transitions based on threat level and aggression
    switch (Level)
    {
        case ENPC_ThreatLevel::Low:
            if (CurrentState == ENPC_BehaviorState::Idle || CurrentState == ENPC_BehaviorState::Patrol)
            {
                TransitionToState(ENPC_BehaviorState::Investigate);
            }
            break;

        case ENPC_ThreatLevel::Medium:
            TransitionToState(ENPC_BehaviorState::Alert);
            AlertTimer = AlertDuration;
            break;

        case ENPC_ThreatLevel::High:
        case ENPC_ThreatLevel::Extreme:
            if (bIsAggressive)
            {
                TransitionToState(ENPC_BehaviorState::Attack);
            }
            else
            {
                TransitionToState(ENPC_BehaviorState::Flee);
            }
            AlertTimer = AlertDuration;
            break;

        default:
            break;
    }
}

void UNPCBehaviorComponent::OnThreatLost()
{
    CurrentThreatLevel = ENPC_ThreatLevel::None;

    // Only go back to alert/investigate — don't immediately return to patrol
    if (CurrentState == ENPC_BehaviorState::Attack || CurrentState == ENPC_BehaviorState::Flee)
    {
        TransitionToState(ENPC_BehaviorState::Alert);
        AlertTimer = AlertDuration;
    }
}

// --- Memory System ---

void UNPCBehaviorComponent::UpdateMemory(AActor* Target, FVector Location, ENPC_ThreatLevel Threat)
{
    if (!Target) return;

    TWeakObjectPtr<AActor> WeakTarget(Target);
    FNPC_MemoryEntry& Entry = MemoryMap.FindOrAdd(WeakTarget);
    Entry.LastKnownLocation = Location;
    Entry.TimeSinceLastSeen = 0.0f;
    Entry.ThreatLevel = Threat;
    Entry.bIsPlayerTarget = Target->ActorHasTag(FName("Player"));
}

bool UNPCBehaviorComponent::HasMemoryOfTarget(AActor* Target) const
{
    if (!Target) return false;
    TWeakObjectPtr<AActor> WeakTarget(Target);
    return MemoryMap.Contains(WeakTarget);
}

FVector UNPCBehaviorComponent::GetLastKnownLocation(AActor* Target) const
{
    if (!Target) return FVector::ZeroVector;
    TWeakObjectPtr<AActor> WeakTarget(Target);
    const FNPC_MemoryEntry* Entry = MemoryMap.Find(WeakTarget);
    return Entry ? Entry->LastKnownLocation : FVector::ZeroVector;
}

void UNPCBehaviorComponent::ForgetTarget(AActor* Target)
{
    if (!Target) return;
    TWeakObjectPtr<AActor> WeakTarget(Target);
    MemoryMap.Remove(WeakTarget);
}

void UNPCBehaviorComponent::ForgetAllMemories()
{
    MemoryMap.Empty();
}

void UNPCBehaviorComponent::TickMemoryDecay(float DeltaTime)
{
    TArray<TWeakObjectPtr<AActor>> ToRemove;

    for (auto& Pair : MemoryMap)
    {
        if (!Pair.Key.IsValid())
        {
            ToRemove.Add(Pair.Key);
            continue;
        }

        Pair.Value.TimeSinceLastSeen += DeltaTime;

        // Decay threat level over time
        // Each threat level decays after a threshold of seconds
        float DecayThreshold = 30.0f; // 30 seconds before threat level drops
        if (Pair.Value.TimeSinceLastSeen > DecayThreshold)
        {
            uint8 ThreatInt = (uint8)Pair.Value.ThreatLevel;
            if (ThreatInt > 0)
            {
                Pair.Value.ThreatLevel = (ENPC_ThreatLevel)(ThreatInt - 1);
                Pair.Value.TimeSinceLastSeen = 0.0f; // reset timer for next decay step
            }
            else
            {
                // Fully forgotten
                ToRemove.Add(Pair.Key);
            }
        }
    }

    for (auto& Key : ToRemove)
    {
        MemoryMap.Remove(Key);
    }
}

void UNPCBehaviorComponent::TickAlertTimer(float DeltaTime)
{
    if (CurrentState == ENPC_BehaviorState::Alert && AlertTimer > 0.0f)
    {
        AlertTimer -= DeltaTime;
        if (AlertTimer <= 0.0f)
        {
            AlertTimer = 0.0f;
            // Return to patrol if we have a route, otherwise idle
            if (HasPatrolRoute())
            {
                TransitionToState(ENPC_BehaviorState::Patrol);
            }
            else
            {
                TransitionToState(ENPC_BehaviorState::Idle);
            }
        }
    }
}

// --- Daily Routine ---

void UNPCBehaviorComponent::AddRoutineSlot(float TimeOfDay, FVector Location, ENPC_BehaviorState State, float Duration)
{
    FNPC_DailyRoutineSlot Slot;
    Slot.TimeOfDay = TimeOfDay;
    Slot.TargetLocation = Location;
    Slot.RoutineState = State;
    Slot.Duration = Duration;
    DailyRoutine.Add(Slot);

    // Keep sorted by time of day
    DailyRoutine.Sort([](const FNPC_DailyRoutineSlot& A, const FNPC_DailyRoutineSlot& B)
    {
        return A.TimeOfDay < B.TimeOfDay;
    });
}

void UNPCBehaviorComponent::EvaluateRoutine(float CurrentTimeOfDay)
{
    if (!bUseDailyRoutine || DailyRoutine.Num() == 0) return;

    // Only evaluate routine if not in a threat state
    if (CurrentState == ENPC_BehaviorState::Attack ||
        CurrentState == ENPC_BehaviorState::Flee ||
        CurrentState == ENPC_BehaviorState::Alert)
    {
        return;
    }

    // Find the most recent routine slot for the current time
    FNPC_DailyRoutineSlot* ActiveSlot = nullptr;
    for (int32 i = DailyRoutine.Num() - 1; i >= 0; --i)
    {
        if (DailyRoutine[i].TimeOfDay <= CurrentTimeOfDay)
        {
            ActiveSlot = &DailyRoutine[i];
            break;
        }
    }

    // Wrap around: if no slot found before current time, use last slot of previous day
    if (!ActiveSlot && DailyRoutine.Num() > 0)
    {
        ActiveSlot = &DailyRoutine.Last();
    }

    if (ActiveSlot)
    {
        SetBehaviorState(ActiveSlot->RoutineState);
    }
}

// --- Patrol ---

void UNPCBehaviorComponent::SetPatrolWaypoints(const TArray<FVector>& Waypoints)
{
    PatrolWaypoints = Waypoints;
    CurrentPatrolIndex = 0;

    if (PatrolWaypoints.Num() > 0)
    {
        TransitionToState(ENPC_BehaviorState::Patrol);
    }
}

FVector UNPCBehaviorComponent::GetNextPatrolPoint()
{
    if (PatrolWaypoints.Num() == 0)
    {
        return GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
    }

    FVector NextPoint = PatrolWaypoints[CurrentPatrolIndex];
    CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolWaypoints.Num();
    return NextPoint;
}
