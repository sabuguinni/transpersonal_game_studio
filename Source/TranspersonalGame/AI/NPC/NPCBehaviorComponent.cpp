#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────
UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f; // Poll every 0.5s — not every frame
}

// ─────────────────────────────────────────────────────────────────────────────
// BeginPlay — build default routine if none assigned
// ─────────────────────────────────────────────────────────────────────────────
void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    if (DailyRoutine.Num() == 0)
    {
        BuildDefaultRoutine();
    }

    // Start at Dawn
    SetDailyPhase(ENPC_DailyPhase::Dawn);
}

// ─────────────────────────────────────────────────────────────────────────────
// TickComponent — advance time, decay memories, update fear
// ─────────────────────────────────────────────────────────────────────────────
void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    AccumulatedTime += DeltaTime;

    // Decay fear over time
    if (FearLevel > 0.0f)
    {
        FearLevel = FMath::Max(0.0f, FearLevel - DeltaTime * 0.05f);
        if (FearLevel < 0.1f)
        {
            bIsFleeing = false;
            bIsAlerting = false;
        }
    }

    // Decay memories periodically
    if (AccumulatedTime - LastMemoryDecayTime > 30.0f)
    {
        DecayMemories(AccumulatedTime);
        LastMemoryDecayTime = AccumulatedTime;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// OnThreatDetected — react to a threat in the world
// ─────────────────────────────────────────────────────────────────────────────
void UNPCBehaviorComponent::OnThreatDetected(FVector ThreatLocation, float ThreatLevel, const FString& ThreatTag)
{
    if (!GetOwner()) return;

    float DistToThreat = FVector::Dist(GetOwner()->GetActorLocation(), ThreatLocation);

    // Record in memory
    RecordMemory(ThreatLocation, AccumulatedTime, ThreatLevel, ThreatTag);

    // Update fear
    FearLevel = FMath::Clamp(FearLevel + ThreatLevel * 0.5f, 0.0f, 1.0f);

    // Determine reaction
    if (DistToThreat <= FleeRadius && ThreatLevel >= 0.6f)
    {
        bIsFleeing = true;
        bIsAlerting = true;
        CurrentActivity = ENPC_Activity::Flee;
    }
    else if (DistToThreat <= AlertRadius && ThreatLevel >= 0.3f)
    {
        bIsAlerting = true;
        if (!bIsFleeing)
        {
            CurrentActivity = ENPC_Activity::Alert;
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// SetDailyPhase — transition to a new time-of-day phase
// ─────────────────────────────────────────────────────────────────────────────
void UNPCBehaviorComponent::SetDailyPhase(ENPC_DailyPhase NewPhase)
{
    CurrentPhase = NewPhase;

    // Only update activity if not in a threat reaction
    if (!bIsFleeing && !bIsAlerting)
    {
        CurrentActivity = GetActivityForPhase(NewPhase);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// GetActivityForPhase — look up routine for a given phase
// ─────────────────────────────────────────────────────────────────────────────
ENPC_Activity UNPCBehaviorComponent::GetActivityForPhase(ENPC_DailyPhase Phase) const
{
    for (const FNPC_RoutineSlot& Slot : DailyRoutine)
    {
        if (Slot.Phase == Phase)
        {
            return Slot.Activity;
        }
    }
    return ENPC_Activity::Idle;
}

// ─────────────────────────────────────────────────────────────────────────────
// RecordMemory — add a new memory entry, evict oldest if over cap
// ─────────────────────────────────────────────────────────────────────────────
void UNPCBehaviorComponent::RecordMemory(FVector Location, float Time, float Threat, const FString& Tag)
{
    FNPC_MemoryEntry Entry;
    Entry.EventLocation = Location;
    Entry.EventTime = Time;
    Entry.ThreatLevel = Threat;
    Entry.EventTag = Tag;
    Entry.bIsActive = true;

    MemoryLog.Add(Entry);

    // Evict oldest entries beyond cap
    while (MemoryLog.Num() > MaxMemoryEntries)
    {
        MemoryLog.RemoveAt(0);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// DecayMemories — mark old memories as inactive
// ─────────────────────────────────────────────────────────────────────────────
void UNPCBehaviorComponent::DecayMemories(float CurrentTime)
{
    for (FNPC_MemoryEntry& Entry : MemoryLog)
    {
        if (Entry.bIsActive && (CurrentTime - Entry.EventTime) > MemoryDecayTime)
        {
            Entry.bIsActive = false;
        }
    }

    // Remove fully decayed entries
    MemoryLog.RemoveAll([](const FNPC_MemoryEntry& E) { return !E.bIsActive; });
}

// ─────────────────────────────────────────────────────────────────────────────
// HasMemoryOfThreat — check if NPC remembers a specific threat type
// ─────────────────────────────────────────────────────────────────────────────
bool UNPCBehaviorComponent::HasMemoryOfThreat(const FString& ThreatTag) const
{
    for (const FNPC_MemoryEntry& Entry : MemoryLog)
    {
        if (Entry.bIsActive && Entry.EventTag == ThreatTag)
        {
            return true;
        }
    }
    return false;
}

// ─────────────────────────────────────────────────────────────────────────────
// GetHighestThreatInMemory — returns the highest active threat level remembered
// ─────────────────────────────────────────────────────────────────────────────
float UNPCBehaviorComponent::GetHighestThreatInMemory() const
{
    float Highest = 0.0f;
    for (const FNPC_MemoryEntry& Entry : MemoryLog)
    {
        if (Entry.bIsActive && Entry.ThreatLevel > Highest)
        {
            Highest = Entry.ThreatLevel;
        }
    }
    return Highest;
}

// ─────────────────────────────────────────────────────────────────────────────
// BuildDefaultRoutine — assign role-appropriate daily schedule
// ─────────────────────────────────────────────────────────────────────────────
void UNPCBehaviorComponent::BuildDefaultRoutine()
{
    DailyRoutine.Empty();

    auto AddSlot = [this](ENPC_DailyPhase Phase, ENPC_Activity Activity, float Duration)
    {
        FNPC_RoutineSlot Slot;
        Slot.Phase = Phase;
        Slot.Activity = Activity;
        Slot.Duration = Duration;
        DailyRoutine.Add(Slot);
    };

    switch (Role)
    {
    case ENPC_Role::Elder:
        AddSlot(ENPC_DailyPhase::Dawn,   ENPC_Activity::Idle,    120.0f);
        AddSlot(ENPC_DailyPhase::Midday, ENPC_Activity::Converse, 300.0f);
        AddSlot(ENPC_DailyPhase::Dusk,   ENPC_Activity::Watch,   180.0f);
        AddSlot(ENPC_DailyPhase::Night,  ENPC_Activity::Sleep,   480.0f);
        break;

    case ENPC_Role::Scout:
        AddSlot(ENPC_DailyPhase::Dawn,   ENPC_Activity::Patrol,  300.0f);
        AddSlot(ENPC_DailyPhase::Midday, ENPC_Activity::Watch,   180.0f);
        AddSlot(ENPC_DailyPhase::Dusk,   ENPC_Activity::Patrol,  240.0f);
        AddSlot(ENPC_DailyPhase::Night,  ENPC_Activity::Watch,   300.0f);
        break;

    case ENPC_Role::Hunter:
        AddSlot(ENPC_DailyPhase::Dawn,   ENPC_Activity::Hunt,    360.0f);
        AddSlot(ENPC_DailyPhase::Midday, ENPC_Activity::Idle,    120.0f);
        AddSlot(ENPC_DailyPhase::Dusk,   ENPC_Activity::Hunt,    240.0f);
        AddSlot(ENPC_DailyPhase::Night,  ENPC_Activity::Sleep,   480.0f);
        break;

    case ENPC_Role::Crafter:
        AddSlot(ENPC_DailyPhase::Dawn,   ENPC_Activity::Gather,  180.0f);
        AddSlot(ENPC_DailyPhase::Midday, ENPC_Activity::Craft,   360.0f);
        AddSlot(ENPC_DailyPhase::Dusk,   ENPC_Activity::Craft,   180.0f);
        AddSlot(ENPC_DailyPhase::Night,  ENPC_Activity::Sleep,   480.0f);
        break;

    case ENPC_Role::Gatherer:
    default:
        AddSlot(ENPC_DailyPhase::Dawn,   ENPC_Activity::Gather,  300.0f);
        AddSlot(ENPC_DailyPhase::Midday, ENPC_Activity::Idle,    120.0f);
        AddSlot(ENPC_DailyPhase::Dusk,   ENPC_Activity::Gather,  180.0f);
        AddSlot(ENPC_DailyPhase::Night,  ENPC_Activity::Sleep,   480.0f);
        break;

    case ENPC_Role::Guard:
        AddSlot(ENPC_DailyPhase::Dawn,   ENPC_Activity::Watch,   300.0f);
        AddSlot(ENPC_DailyPhase::Midday, ENPC_Activity::Patrol,  240.0f);
        AddSlot(ENPC_DailyPhase::Dusk,   ENPC_Activity::Watch,   300.0f);
        AddSlot(ENPC_DailyPhase::Night,  ENPC_Activity::Patrol,  360.0f);
        break;
    }
}
