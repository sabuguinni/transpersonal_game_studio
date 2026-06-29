// NPCBehaviorComponent.cpp
// NPC Behavior Agent #11 — PROD_CYCLE_AUTO_20260629_002
// Full implementation of UNPCBehaviorComponent — daily routines, memory, threat response

#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "TimerManager.h"

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz update — sufficient for NPC AI

    CurrentBehaviorState = ENPC_BehaviorState::Idle;
    CurrentThreatLevel   = ENPC_ThreatLevel::None;
    MemoryRetentionTime  = 60.0f;
    PatrolRadius         = 2000.0f;
    AlertRadius          = 1500.0f;
    FleeRadius           = 800.0f;
    SocialRadius         = 600.0f;
    MaxMemoryEntries     = 8;
    bIsLeader            = false;
    bIsSleeping          = false;
    CurrentRoutineIndex  = 0;
    StateTimeAccumulator = 0.0f;
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    // Cache owner pawn
    OwnerPawn = Cast<APawn>(GetOwner());

    // Build default daily routine if none provided
    if (DailyRoutine.Num() == 0)
    {
        BuildDefaultDailyRoutine();
    }

    // Start routine advancement timer — check every 30 seconds game time
    GetWorld()->GetTimerManager().SetTimer(
        RoutineTimerHandle,
        this,
        &UNPCBehaviorComponent::AdvanceDailyRoutine,
        30.0f,
        true
    );

    // Start memory cleanup timer — purge stale entries every 10 seconds
    GetWorld()->GetTimerManager().SetTimer(
        MemoryCleanupTimerHandle,
        this,
        &UNPCBehaviorComponent::PurgeStaleMemories,
        10.0f,
        true
    );
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    StateTimeAccumulator += DeltaTime;

    // Evaluate threats from memory every tick
    EvaluateThreatFromMemory();

    // State machine update
    switch (CurrentBehaviorState)
    {
        case ENPC_BehaviorState::Idle:
            TickIdle(DeltaTime);
            break;
        case ENPC_BehaviorState::Patrol:
            TickPatrol(DeltaTime);
            break;
        case ENPC_BehaviorState::Forage:
            TickForage(DeltaTime);
            break;
        case ENPC_BehaviorState::Flee:
            TickFlee(DeltaTime);
            break;
        case ENPC_BehaviorState::Alert:
            TickAlert(DeltaTime);
            break;
        case ENPC_BehaviorState::Socialise:
            TickSocialise(DeltaTime);
            break;
        case ENPC_BehaviorState::Sleep:
            TickSleep(DeltaTime);
            break;
        case ENPC_BehaviorState::Dead:
            // No tick needed
            break;
    }
}

// ─── STATE TRANSITIONS ──────────────────────────────────────────────────────

void UNPCBehaviorComponent::TransitionToState(ENPC_BehaviorState NewState)
{
    if (CurrentBehaviorState == NewState) return;
    if (CurrentBehaviorState == ENPC_BehaviorState::Dead) return;

    ENPC_BehaviorState OldState = CurrentBehaviorState;
    CurrentBehaviorState = NewState;
    StateTimeAccumulator = 0.0f;

    OnBehaviorStateChanged.Broadcast(OldState, NewState);

    UE_LOG(LogTemp, Verbose, TEXT("[NPC %s] State: %d -> %d"),
        *GetOwner()->GetName(),
        (int32)OldState,
        (int32)NewState);
}

// ─── MEMORY SYSTEM ──────────────────────────────────────────────────────────

void UNPCBehaviorComponent::RecordThreat(AActor* ThreatActor, FVector LastKnownLocation, ENPC_ThreatLevel ThreatLvl)
{
    if (!ThreatActor) return;

    float Now = GetWorld()->GetTimeSeconds();

    // Update existing entry if present
    for (FNPC_MemoryEntry& Entry : ThreatMemory)
    {
        if (Entry.ThreatActor == ThreatActor)
        {
            Entry.LastKnownLocation = LastKnownLocation;
            Entry.Timestamp         = Now;
            Entry.ThreatLevel       = ThreatLvl;
            return;
        }
    }

    // Add new entry — evict oldest if at capacity
    if (ThreatMemory.Num() >= MaxMemoryEntries)
    {
        int32 OldestIdx = 0;
        float OldestTime = ThreatMemory[0].Timestamp;
        for (int32 i = 1; i < ThreatMemory.Num(); ++i)
        {
            if (ThreatMemory[i].Timestamp < OldestTime)
            {
                OldestTime = ThreatMemory[i].Timestamp;
                OldestIdx  = i;
            }
        }
        ThreatMemory.RemoveAt(OldestIdx);
    }

    FNPC_MemoryEntry NewEntry;
    NewEntry.ThreatActor        = ThreatActor;
    NewEntry.LastKnownLocation  = LastKnownLocation;
    NewEntry.Timestamp          = Now;
    NewEntry.ThreatLevel        = ThreatLvl;
    ThreatMemory.Add(NewEntry);

    // Immediately react if threat is high
    if (ThreatLvl >= ENPC_ThreatLevel::High)
    {
        ReactToThreat(ThreatActor, ThreatLvl);
    }
}

void UNPCBehaviorComponent::ForgetThreat(AActor* ThreatActor)
{
    ThreatMemory.RemoveAll([ThreatActor](const FNPC_MemoryEntry& E)
    {
        return E.ThreatActor == ThreatActor;
    });
}

void UNPCBehaviorComponent::PurgeStaleMemories()
{
    float Now = GetWorld()->GetTimeSeconds();
    ThreatMemory.RemoveAll([this, Now](const FNPC_MemoryEntry& E)
    {
        return (Now - E.Timestamp) > MemoryRetentionTime;
    });
}

void UNPCBehaviorComponent::EvaluateThreatFromMemory()
{
    if (ThreatMemory.Num() == 0)
    {
        if (CurrentThreatLevel != ENPC_ThreatLevel::None)
        {
            CurrentThreatLevel = ENPC_ThreatLevel::None;
            // Return to routine if we were fleeing/alert
            if (CurrentBehaviorState == ENPC_BehaviorState::Flee ||
                CurrentBehaviorState == ENPC_BehaviorState::Alert)
            {
                TransitionToState(ENPC_BehaviorState::Idle);
            }
        }
        return;
    }

    // Find highest threat in memory
    ENPC_ThreatLevel MaxThreat = ENPC_ThreatLevel::None;
    for (const FNPC_MemoryEntry& Entry : ThreatMemory)
    {
        if (Entry.ThreatLevel > MaxThreat)
        {
            MaxThreat = Entry.ThreatLevel;
        }
    }

    CurrentThreatLevel = MaxThreat;

    // Trigger state transitions based on threat
    if (MaxThreat >= ENPC_ThreatLevel::Critical)
    {
        if (CurrentBehaviorState != ENPC_BehaviorState::Flee)
            TransitionToState(ENPC_BehaviorState::Flee);
    }
    else if (MaxThreat >= ENPC_ThreatLevel::High)
    {
        if (CurrentBehaviorState != ENPC_BehaviorState::Flee &&
            CurrentBehaviorState != ENPC_BehaviorState::Alert)
            TransitionToState(ENPC_BehaviorState::Alert);
    }
    else if (MaxThreat >= ENPC_ThreatLevel::Medium)
    {
        if (CurrentBehaviorState == ENPC_BehaviorState::Idle ||
            CurrentBehaviorState == ENPC_BehaviorState::Forage ||
            CurrentBehaviorState == ENPC_BehaviorState::Socialise)
            TransitionToState(ENPC_BehaviorState::Alert);
    }
}

void UNPCBehaviorComponent::ReactToThreat(AActor* ThreatActor, ENPC_ThreatLevel ThreatLvl)
{
    if (!ThreatActor) return;

    // Broadcast so Behavior Tree can pick up the event
    OnThreatDetected.Broadcast(ThreatActor, ThreatLvl);

    if (ThreatLvl >= ENPC_ThreatLevel::Critical)
    {
        TransitionToState(ENPC_BehaviorState::Flee);
    }
    else if (ThreatLvl >= ENPC_ThreatLevel::High)
    {
        TransitionToState(ENPC_BehaviorState::Alert);
    }
}

// ─── DAILY ROUTINE ──────────────────────────────────────────────────────────

void UNPCBehaviorComponent::BuildDefaultDailyRoutine()
{
    // Dawn — forage near spawn
    FNPC_DailyRoutineSlot Dawn;
    Dawn.StartHour       = 6.0f;
    Dawn.EndHour         = 9.0f;
    Dawn.TargetState     = ENPC_BehaviorState::Forage;
    Dawn.RoutineLocation = GetOwner()->GetActorLocation() + FVector(500.0f, 0.0f, 0.0f);
    Dawn.bMustReachLocation = false;
    DailyRoutine.Add(Dawn);

    // Mid-morning — patrol territory
    FNPC_DailyRoutineSlot MidMorning;
    MidMorning.StartHour       = 9.0f;
    MidMorning.EndHour         = 12.0f;
    MidMorning.TargetState     = ENPC_BehaviorState::Patrol;
    MidMorning.RoutineLocation = GetOwner()->GetActorLocation();
    MidMorning.bMustReachLocation = false;
    DailyRoutine.Add(MidMorning);

    // Midday — rest/socialise near water
    FNPC_DailyRoutineSlot Midday;
    Midday.StartHour       = 12.0f;
    Midday.EndHour         = 15.0f;
    Midday.TargetState     = ENPC_BehaviorState::Socialise;
    Midday.RoutineLocation = GetOwner()->GetActorLocation() + FVector(0.0f, 800.0f, 0.0f);
    Midday.bMustReachLocation = true;
    DailyRoutine.Add(Midday);

    // Afternoon — forage again
    FNPC_DailyRoutineSlot Afternoon;
    Afternoon.StartHour       = 15.0f;
    Afternoon.EndHour         = 18.0f;
    Afternoon.TargetState     = ENPC_BehaviorState::Forage;
    Afternoon.RoutineLocation = GetOwner()->GetActorLocation() + FVector(-400.0f, 400.0f, 0.0f);
    Afternoon.bMustReachLocation = false;
    DailyRoutine.Add(Afternoon);

    // Dusk — return to shelter, idle
    FNPC_DailyRoutineSlot Dusk;
    Dusk.StartHour       = 18.0f;
    Dusk.EndHour         = 21.0f;
    Dusk.TargetState     = ENPC_BehaviorState::Idle;
    Dusk.RoutineLocation = GetOwner()->GetActorLocation();
    Dusk.bMustReachLocation = true;
    DailyRoutine.Add(Dusk);

    // Night — sleep
    FNPC_DailyRoutineSlot Night;
    Night.StartHour       = 21.0f;
    Night.EndHour         = 6.0f;
    Night.TargetState     = ENPC_BehaviorState::Sleep;
    Night.RoutineLocation = GetOwner()->GetActorLocation();
    Night.bMustReachLocation = true;
    DailyRoutine.Add(Night);
}

void UNPCBehaviorComponent::AdvanceDailyRoutine()
{
    // Only advance routine if not in a threat state
    if (CurrentThreatLevel >= ENPC_ThreatLevel::Medium) return;
    if (CurrentBehaviorState == ENPC_BehaviorState::Dead) return;

    if (DailyRoutine.Num() == 0) return;

    CurrentRoutineIndex = (CurrentRoutineIndex + 1) % DailyRoutine.Num();
    const FNPC_DailyRoutineSlot& Slot = DailyRoutine[CurrentRoutineIndex];

    TransitionToState(Slot.TargetState);

    if (Slot.TargetState == ENPC_BehaviorState::Sleep)
    {
        bIsSleeping = true;
    }
    else
    {
        bIsSleeping = false;
    }
}

// ─── STATE TICK IMPLEMENTATIONS ─────────────────────────────────────────────

void UNPCBehaviorComponent::TickIdle(float DeltaTime)
{
    // After 5 seconds idle, transition to patrol
    if (StateTimeAccumulator > 5.0f && CurrentThreatLevel == ENPC_ThreatLevel::None)
    {
        TransitionToState(ENPC_BehaviorState::Patrol);
    }
}

void UNPCBehaviorComponent::TickPatrol(float DeltaTime)
{
    // Patrol is driven by Behavior Tree — we just maintain state here
    // After 30s of patrol with no threat, consider foraging
    if (StateTimeAccumulator > 30.0f)
    {
        TransitionToState(ENPC_BehaviorState::Forage);
    }
}

void UNPCBehaviorComponent::TickForage(float DeltaTime)
{
    // Foraging — driven by BT, we maintain state
    // After 20s, return to patrol
    if (StateTimeAccumulator > 20.0f && CurrentThreatLevel == ENPC_ThreatLevel::None)
    {
        TransitionToState(ENPC_BehaviorState::Patrol);
    }
}

void UNPCBehaviorComponent::TickFlee(float DeltaTime)
{
    // Flee until memory clears (handled by EvaluateThreatFromMemory)
    // Safety: if fleeing for >60s without threat update, return to alert
    if (StateTimeAccumulator > 60.0f && CurrentThreatLevel < ENPC_ThreatLevel::High)
    {
        TransitionToState(ENPC_BehaviorState::Alert);
    }
}

void UNPCBehaviorComponent::TickAlert(float DeltaTime)
{
    // Alert — scan for threats, driven by BT perception
    // After 15s of alert with no new threat, return to idle
    if (StateTimeAccumulator > 15.0f && CurrentThreatLevel <= ENPC_ThreatLevel::Low)
    {
        TransitionToState(ENPC_BehaviorState::Idle);
    }
}

void UNPCBehaviorComponent::TickSocialise(float DeltaTime)
{
    // Socialise — driven by BT proximity checks
    // After 10s, return to patrol
    if (StateTimeAccumulator > 10.0f)
    {
        TransitionToState(ENPC_BehaviorState::Patrol);
    }
}

void UNPCBehaviorComponent::TickSleep(float DeltaTime)
{
    // Sleep — interrupted only by Critical threat
    // bIsSleeping flag drives animation
}

// ─── BLUEPRINT CALLABLE INTERFACE ───────────────────────────────────────────

ENPC_BehaviorState UNPCBehaviorComponent::GetCurrentBehaviorState() const
{
    return CurrentBehaviorState;
}

ENPC_ThreatLevel UNPCBehaviorComponent::GetCurrentThreatLevel() const
{
    return CurrentThreatLevel;
}

bool UNPCBehaviorComponent::HasMemoryOfThreat(AActor* ThreatActor) const
{
    for (const FNPC_MemoryEntry& Entry : ThreatMemory)
    {
        if (Entry.ThreatActor == ThreatActor) return true;
    }
    return false;
}

FVector UNPCBehaviorComponent::GetLastKnownThreatLocation() const
{
    if (ThreatMemory.Num() == 0) return FVector::ZeroVector;

    // Return location of highest-priority threat
    const FNPC_MemoryEntry* Best = nullptr;
    for (const FNPC_MemoryEntry& Entry : ThreatMemory)
    {
        if (!Best || Entry.ThreatLevel > Best->ThreatLevel)
        {
            Best = &Entry;
        }
    }
    return Best ? Best->LastKnownLocation : FVector::ZeroVector;
}

void UNPCBehaviorComponent::ForceBehaviorState(ENPC_BehaviorState NewState)
{
    TransitionToState(NewState);
}

void UNPCBehaviorComponent::SetAsLeader(bool bLeader)
{
    bIsLeader = bLeader;
}

int32 UNPCBehaviorComponent::GetMemoryCount() const
{
    return ThreatMemory.Num();
}
