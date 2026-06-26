// NPCBehaviorComponent.cpp
// NPC Behavior Agent #11 — Transpersonal Game Studio
// Implements: state machine, patrol waypoints, threat memory, daily schedule, perception callbacks

#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

// -----------------------------------------------------------------------
// Constructor
// -----------------------------------------------------------------------

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 Hz — sufficient for NPC logic

    // Default daily schedule for a tribal hunter NPC
    // 0-6h: Sleep at camp
    FNPC_ScheduleEntry SleepEntry;
    SleepEntry.StartHour = 0.0f;
    SleepEntry.EndHour = 6.0f;
    SleepEntry.Activity = ENPC_DailyActivity::Sleep;
    DailySchedule.Add(SleepEntry);

    // 6-10h: Forage near camp
    FNPC_ScheduleEntry ForageEntry;
    ForageEntry.StartHour = 6.0f;
    ForageEntry.EndHour = 10.0f;
    ForageEntry.Activity = ENPC_DailyActivity::Forage;
    DailySchedule.Add(ForageEntry);

    // 10-14h: Hunt
    FNPC_ScheduleEntry HuntEntry;
    HuntEntry.StartHour = 10.0f;
    HuntEntry.EndHour = 14.0f;
    HuntEntry.Activity = ENPC_DailyActivity::Hunt;
    DailySchedule.Add(HuntEntry);

    // 14-16h: Craft tools
    FNPC_ScheduleEntry CraftEntry;
    CraftEntry.StartHour = 14.0f;
    CraftEntry.EndHour = 16.0f;
    CraftEntry.Activity = ENPC_DailyActivity::Craft;
    DailySchedule.Add(CraftEntry);

    // 16-20h: Guard camp perimeter
    FNPC_ScheduleEntry GuardEntry;
    GuardEntry.StartHour = 16.0f;
    GuardEntry.EndHour = 20.0f;
    GuardEntry.Activity = ENPC_DailyActivity::Guard;
    DailySchedule.Add(GuardEntry);

    // 20-24h: Socialize around fire
    FNPC_ScheduleEntry SocialEntry;
    SocialEntry.StartHour = 20.0f;
    SocialEntry.EndHour = 24.0f;
    SocialEntry.Activity = ENPC_DailyActivity::Socialize;
    DailySchedule.Add(SocialEntry);
}

// -----------------------------------------------------------------------
// BeginPlay
// -----------------------------------------------------------------------

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    // Start in patrol state if waypoints are assigned, else idle
    if (PatrolWaypoints.Num() > 0)
    {
        CurrentState = ENPC_BehaviorState::Patrol;
    }
    else
    {
        CurrentState = ENPC_BehaviorState::Idle;
    }
}

// -----------------------------------------------------------------------
// TickComponent
// -----------------------------------------------------------------------

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    DecayMemory(DeltaTime);
    UpdateFearLevel(DeltaTime);
    EvaluateThreatLevel();
    UpdateBehaviorState(DeltaTime);
}

// -----------------------------------------------------------------------
// RegisterThreat
// -----------------------------------------------------------------------

void UNPCBehaviorComponent::RegisterThreat(AActor* ThreatActor, float ThreatScore, FVector LastKnownLoc)
{
    if (!ThreatActor)
    {
        return;
    }

    // Check if this actor is already in memory — update if so
    for (FNPC_MemoryEntry& Entry : ThreatMemory)
    {
        if (Entry.SourceActor == ThreatActor)
        {
            Entry.ThreatScore = FMath::Max(Entry.ThreatScore, ThreatScore);
            Entry.LastKnownLocation = LastKnownLoc;
            Entry.TimeStamp = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
            Entry.bIsActive = true;
            return;
        }
    }

    // New threat — add to memory (evict oldest if at capacity)
    if (ThreatMemory.Num() >= MaxMemoryEntries)
    {
        // Find and remove the oldest entry
        int32 OldestIndex = 0;
        float OldestTime = ThreatMemory[0].TimeStamp;
        for (int32 i = 1; i < ThreatMemory.Num(); ++i)
        {
            if (ThreatMemory[i].TimeStamp < OldestTime)
            {
                OldestTime = ThreatMemory[i].TimeStamp;
                OldestIndex = i;
            }
        }
        ThreatMemory.RemoveAt(OldestIndex);
    }

    FNPC_MemoryEntry NewEntry;
    NewEntry.SourceActor = ThreatActor;
    NewEntry.LastKnownLocation = LastKnownLoc;
    NewEntry.ThreatScore = ThreatScore;
    NewEntry.TimeStamp = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    NewEntry.bIsActive = true;
    ThreatMemory.Add(NewEntry);

    // Immediately escalate fear
    FearLevel = FMath::Min(FearLevel + ThreatScore * 0.5f, 100.0f);
}

// -----------------------------------------------------------------------
// ClearThreat
// -----------------------------------------------------------------------

void UNPCBehaviorComponent::ClearThreat(AActor* ThreatActor)
{
    ThreatMemory.RemoveAll([ThreatActor](const FNPC_MemoryEntry& Entry)
    {
        return Entry.SourceActor == ThreatActor;
    });
}

// -----------------------------------------------------------------------
// UpdateBehaviorState
// -----------------------------------------------------------------------

void UNPCBehaviorComponent::UpdateBehaviorState(float DeltaTime)
{
    ENPC_BehaviorState DesiredState = DetermineStateFromContext();

    if (DesiredState != CurrentState)
    {
        SetBehaviorState(DesiredState);
    }

    // Handle patrol wait timer
    if (CurrentState == ENPC_BehaviorState::Patrol && bIsWaitingAtWaypoint)
    {
        PatrolWaitTimer -= DeltaTime;
        if (PatrolWaitTimer <= 0.0f)
        {
            bIsWaitingAtWaypoint = false;
            AdvancePatrolWaypoint();
        }
    }
}

// -----------------------------------------------------------------------
// DetermineStateFromContext
// -----------------------------------------------------------------------

ENPC_BehaviorState UNPCBehaviorComponent::DetermineStateFromContext() const
{
    // Extreme fear → flee immediately
    if (FearLevel >= FleeThresholdFear)
    {
        return ENPC_BehaviorState::Flee;
    }

    // Active high threat → alert/investigate
    if (HasActiveThreat())
    {
        if (CurrentThreatLevel >= ENPC_ThreatLevel::High)
        {
            return ENPC_BehaviorState::Alert;
        }
        return ENPC_BehaviorState::Investigate;
    }

    // Schedule-driven: if hunting activity → hunt state
    if (CurrentActivity == ENPC_DailyActivity::Hunt)
    {
        return ENPC_BehaviorState::Hunt;
    }

    // Default to patrol if waypoints exist
    if (PatrolWaypoints.Num() > 0)
    {
        return ENPC_BehaviorState::Patrol;
    }

    return ENPC_BehaviorState::Idle;
}

// -----------------------------------------------------------------------
// SetBehaviorState
// -----------------------------------------------------------------------

void UNPCBehaviorComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (CurrentState == NewState)
    {
        return;
    }

    CurrentState = NewState;

    // Reset patrol wait when re-entering patrol
    if (NewState == ENPC_BehaviorState::Patrol)
    {
        bIsWaitingAtWaypoint = false;
        PatrolWaitTimer = 0.0f;
    }
}

// -----------------------------------------------------------------------
// AdvancePatrolWaypoint
// -----------------------------------------------------------------------

void UNPCBehaviorComponent::AdvancePatrolWaypoint()
{
    if (PatrolWaypoints.Num() == 0)
    {
        return;
    }

    CurrentWaypointIndex = (CurrentWaypointIndex + 1) % PatrolWaypoints.Num();

    // Start wait timer at new waypoint
    bIsWaitingAtWaypoint = true;
    PatrolWaitTimer = PatrolWaitTime;
}

// -----------------------------------------------------------------------
// GetNextPatrolLocation
// -----------------------------------------------------------------------

FVector UNPCBehaviorComponent::GetNextPatrolLocation() const
{
    if (PatrolWaypoints.Num() == 0 || !PatrolWaypoints.IsValidIndex(CurrentWaypointIndex))
    {
        return GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
    }

    AActor* Waypoint = PatrolWaypoints[CurrentWaypointIndex];
    return Waypoint ? Waypoint->GetActorLocation() : FVector::ZeroVector;
}

// -----------------------------------------------------------------------
// HasActiveThreat
// -----------------------------------------------------------------------

bool UNPCBehaviorComponent::HasActiveThreat() const
{
    for (const FNPC_MemoryEntry& Entry : ThreatMemory)
    {
        if (Entry.bIsActive && Entry.ThreatScore > 10.0f)
        {
            return true;
        }
    }
    return false;
}

// -----------------------------------------------------------------------
// GetHighestThreatActor
// -----------------------------------------------------------------------

AActor* UNPCBehaviorComponent::GetHighestThreatActor() const
{
    AActor* HighestThreat = nullptr;
    float HighestScore = 0.0f;

    for (const FNPC_MemoryEntry& Entry : ThreatMemory)
    {
        if (Entry.bIsActive && Entry.ThreatScore > HighestScore)
        {
            HighestScore = Entry.ThreatScore;
            HighestThreat = Entry.SourceActor;
        }
    }

    return HighestThreat;
}

// -----------------------------------------------------------------------
// UpdateDailyActivity
// -----------------------------------------------------------------------

void UNPCBehaviorComponent::UpdateDailyActivity(float CurrentGameHour)
{
    for (const FNPC_ScheduleEntry& Entry : DailySchedule)
    {
        if (CurrentGameHour >= Entry.StartHour && CurrentGameHour < Entry.EndHour)
        {
            CurrentActivity = Entry.Activity;
            return;
        }
    }

    // Default to idle if no schedule entry matches
    CurrentActivity = ENPC_DailyActivity::Guard;
}

// -----------------------------------------------------------------------
// OnHearNoise
// -----------------------------------------------------------------------

void UNPCBehaviorComponent::OnHearNoise(AActor* NoiseSource, float Loudness)
{
    if (!NoiseSource)
    {
        return;
    }

    // Scale threat by loudness (0-1 range expected)
    float ThreatScore = Loudness * 40.0f;
    RegisterThreat(NoiseSource, ThreatScore, NoiseSource->GetActorLocation());

    // Bump alertness
    Alertness = FMath::Min(Alertness + Loudness * 30.0f, 100.0f);
}

// -----------------------------------------------------------------------
// OnSeeActor
// -----------------------------------------------------------------------

void UNPCBehaviorComponent::OnSeeActor(AActor* SeenActor)
{
    if (!SeenActor)
    {
        return;
    }

    // Determine threat score based on actor type name (simple heuristic)
    float ThreatScore = 20.0f;
    FString ActorName = SeenActor->GetName().ToLower();

    if (ActorName.Contains(TEXT("trex")) || ActorName.Contains(TEXT("rex")))
    {
        ThreatScore = 90.0f;
    }
    else if (ActorName.Contains(TEXT("raptor")) || ActorName.Contains(TEXT("velociraptor")))
    {
        ThreatScore = 70.0f;
    }
    else if (ActorName.Contains(TEXT("trike")) || ActorName.Contains(TEXT("triceratops")))
    {
        ThreatScore = 50.0f;
    }
    else if (ActorName.Contains(TEXT("brachio")) || ActorName.Contains(TEXT("brachiosaurus")))
    {
        ThreatScore = 30.0f; // Large but herbivore — less threatening
    }

    RegisterThreat(SeenActor, ThreatScore, SeenActor->GetActorLocation());
}

// -----------------------------------------------------------------------
// DecayMemory (private)
// -----------------------------------------------------------------------

void UNPCBehaviorComponent::DecayMemory(float DeltaTime)
{
    for (FNPC_MemoryEntry& Entry : ThreatMemory)
    {
        Entry.ThreatScore -= MemoryDecayRate * DeltaTime;
        if (Entry.ThreatScore <= 0.0f)
        {
            Entry.bIsActive = false;
        }
    }

    // Remove fully decayed entries
    ThreatMemory.RemoveAll([](const FNPC_MemoryEntry& Entry)
    {
        return !Entry.bIsActive;
    });
}

// -----------------------------------------------------------------------
// UpdateFearLevel (private)
// -----------------------------------------------------------------------

void UNPCBehaviorComponent::UpdateFearLevel(float DeltaTime)
{
    // Fear decays over time when no active threats
    if (!HasActiveThreat())
    {
        FearLevel = FMath::Max(FearLevel - 10.0f * DeltaTime, 0.0f);
        Alertness = FMath::Max(Alertness - 5.0f * DeltaTime, 0.0f);
    }
}

// -----------------------------------------------------------------------
// EvaluateThreatLevel (private)
// -----------------------------------------------------------------------

void UNPCBehaviorComponent::EvaluateThreatLevel()
{
    float MaxThreat = 0.0f;
    for (const FNPC_MemoryEntry& Entry : ThreatMemory)
    {
        if (Entry.bIsActive)
        {
            MaxThreat = FMath::Max(MaxThreat, Entry.ThreatScore);
        }
    }

    if (MaxThreat <= 0.0f)
    {
        CurrentThreatLevel = ENPC_ThreatLevel::None;
    }
    else if (MaxThreat < 25.0f)
    {
        CurrentThreatLevel = ENPC_ThreatLevel::Low;
    }
    else if (MaxThreat < 50.0f)
    {
        CurrentThreatLevel = ENPC_ThreatLevel::Medium;
    }
    else if (MaxThreat < 75.0f)
    {
        CurrentThreatLevel = ENPC_ThreatLevel::High;
    }
    else
    {
        CurrentThreatLevel = ENPC_ThreatLevel::Extreme;
    }
}
