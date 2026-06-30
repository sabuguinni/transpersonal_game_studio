#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

// ============================================================
// Constructor
// ============================================================

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz tick — sufficient for NPC AI

    CurrentAlertLevel   = ENPC_AlertLevel::Unaware;
    CurrentPhase        = ENPC_RoutinePhase::Foraging;
    PlayerRelationship  = ENPC_Relationship::Unknown;
    CurrentTargetLocation = FVector::ZeroVector;
    RelationshipScore   = 0.0f;
    FearLevel           = 0.0f;
    TimeSinceLastThreat = 0.0f;
}

// ============================================================
// Lifecycle
// ============================================================

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    // Build a default daily schedule if none was set externally
    if (DailySchedule.Num() == 0)
    {
        FNPC_DailyScheduleEntry Sleep;
        Sleep.Phase = ENPC_RoutinePhase::Sleeping;
        Sleep.StartHour = 20.0f;
        Sleep.EndHour   = 6.0f;
        Sleep.TargetLocation = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;

        FNPC_DailyScheduleEntry Forage;
        Forage.Phase = ENPC_RoutinePhase::Foraging;
        Forage.StartHour = 6.0f;
        Forage.EndHour   = 12.0f;
        Forage.TargetLocation = GetOwner() ? GetOwner()->GetActorLocation() + FVector(500.0f, 0.0f, 0.0f) : FVector::ZeroVector;

        FNPC_DailyScheduleEntry Rest;
        Rest.Phase = ENPC_RoutinePhase::Resting;
        Rest.StartHour = 12.0f;
        Rest.EndHour   = 15.0f;
        Rest.TargetLocation = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;

        FNPC_DailyScheduleEntry Socialize;
        Socialize.Phase = ENPC_RoutinePhase::Socializing;
        Socialize.StartHour = 15.0f;
        Socialize.EndHour   = 20.0f;
        Socialize.TargetLocation = GetOwner() ? GetOwner()->GetActorLocation() + FVector(-300.0f, 200.0f, 0.0f) : FVector::ZeroVector;

        DailySchedule.Add(Sleep);
        DailySchedule.Add(Forage);
        DailySchedule.Add(Rest);
        DailySchedule.Add(Socialize);
    }
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateAlertDecay(DeltaTime);
    DecayFear(DeltaTime);
    PurgeOldMemories(MemoryRetentionSeconds);
}

// ============================================================
// Alert System
// ============================================================

void UNPCBehaviorComponent::SetAlertLevel(ENPC_AlertLevel NewLevel)
{
    if (NewLevel != CurrentAlertLevel)
    {
        CurrentAlertLevel = NewLevel;
        TimeSinceLastThreat = 0.0f;

        // Panic immediately overrides routine
        if (NewLevel == ENPC_AlertLevel::Panicked)
        {
            SetRoutinePhase(ENPC_RoutinePhase::Fleeing);
        }
        else if (NewLevel == ENPC_AlertLevel::Alarmed)
        {
            SetRoutinePhase(ENPC_RoutinePhase::Investigating);
        }
    }
}

void UNPCBehaviorComponent::RegisterThreatEvent(FVector ThreatLocation, float ThreatMagnitude, bool bPlayerCaused)
{
    TimeSinceLastThreat = 0.0f;

    // Add fear proportional to threat
    AddFear(ThreatMagnitude * 10.0f);

    // Escalate alert level based on magnitude
    if (ThreatMagnitude >= 8.0f)
    {
        SetAlertLevel(ENPC_AlertLevel::Panicked);
    }
    else if (ThreatMagnitude >= 5.0f)
    {
        SetAlertLevel(ENPC_AlertLevel::Alarmed);
    }
    else if (ThreatMagnitude >= 3.0f)
    {
        SetAlertLevel(ENPC_AlertLevel::Suspicious);
    }
    else
    {
        SetAlertLevel(ENPC_AlertLevel::Curious);
    }

    // Log to memory
    FString Desc = bPlayerCaused ? TEXT("Player-caused threat") : TEXT("Environmental threat");
    AddMemoryEntry(ThreatLocation, ThreatMagnitude, bPlayerCaused, Desc);

    // Damage relationship if player-caused
    if (bPlayerCaused)
    {
        ModifyRelationshipScore(-ThreatMagnitude * 5.0f);
    }
}

void UNPCBehaviorComponent::UpdateAlertDecay(float DeltaTime)
{
    if (CurrentAlertLevel == ENPC_AlertLevel::Unaware)
    {
        return;
    }

    TimeSinceLastThreat += DeltaTime;

    if (TimeSinceLastThreat < AlertDecayDelay)
    {
        return;
    }

    // Decay one level every (1/AlertDecayRate) seconds
    static float DecayAccumulator = 0.0f;
    DecayAccumulator += DeltaTime * AlertDecayRate;

    if (DecayAccumulator >= 1.0f)
    {
        DecayAccumulator = 0.0f;
        uint8 CurrentVal = static_cast<uint8>(CurrentAlertLevel);
        if (CurrentVal > 0)
        {
            CurrentAlertLevel = static_cast<ENPC_AlertLevel>(CurrentVal - 1);
        }
    }
}

// ============================================================
// Routine System
// ============================================================

void UNPCBehaviorComponent::AdvanceRoutinePhase()
{
    uint8 Next = (static_cast<uint8>(CurrentPhase) + 1) % 8;
    CurrentPhase = static_cast<ENPC_RoutinePhase>(Next);
}

void UNPCBehaviorComponent::SetRoutinePhase(ENPC_RoutinePhase NewPhase)
{
    CurrentPhase = NewPhase;
}

void UNPCBehaviorComponent::EvaluateScheduleForHour(float CurrentHour)
{
    // Don't override emergency states
    if (CurrentAlertLevel >= ENPC_AlertLevel::Alarmed)
    {
        return;
    }

    for (const FNPC_DailyScheduleEntry& Entry : DailySchedule)
    {
        bool bInRange = false;

        if (Entry.StartHour < Entry.EndHour)
        {
            bInRange = (CurrentHour >= Entry.StartHour && CurrentHour < Entry.EndHour);
        }
        else
        {
            // Wraps midnight (e.g., 20:00 → 06:00)
            bInRange = (CurrentHour >= Entry.StartHour || CurrentHour < Entry.EndHour);
        }

        if (bInRange)
        {
            SetRoutinePhase(Entry.Phase);
            CurrentTargetLocation = Entry.TargetLocation;
            return;
        }
    }
}

void UNPCBehaviorComponent::SetDailySchedule(const TArray<FNPC_DailyScheduleEntry>& NewSchedule)
{
    DailySchedule = NewSchedule;
}

// ============================================================
// Memory System
// ============================================================

void UNPCBehaviorComponent::AddMemoryEntry(FVector Location, float ThreatLevel, bool bPlayerInvolved, const FString& Description)
{
    FNPC_MemoryEntry Entry;
    Entry.EventLocation     = Location;
    Entry.ThreatLevel       = ThreatLevel;
    Entry.bWasPlayerInvolved = bPlayerInvolved;
    Entry.EventDescription  = Description;

    UWorld* World = GetWorld();
    Entry.TimeStamp = World ? World->GetTimeSeconds() : 0.0f;

    MemoryLog.Add(Entry);

    // Cap memory size
    while (MemoryLog.Num() > MaxMemoryEntries)
    {
        MemoryLog.RemoveAt(0);
    }
}

bool UNPCBehaviorComponent::HasRecentThreatMemory(float WithinSeconds) const
{
    UWorld* World = GetWorld();
    if (!World) return false;

    float Now = World->GetTimeSeconds();

    for (const FNPC_MemoryEntry& Entry : MemoryLog)
    {
        if ((Now - Entry.TimeStamp) <= WithinSeconds && Entry.ThreatLevel > 0.0f)
        {
            return true;
        }
    }
    return false;
}

FNPC_MemoryEntry UNPCBehaviorComponent::GetMostRecentMemory() const
{
    if (MemoryLog.Num() == 0)
    {
        return FNPC_MemoryEntry();
    }
    return MemoryLog.Last();
}

void UNPCBehaviorComponent::PurgeOldMemories(float OlderThanSeconds)
{
    UWorld* World = GetWorld();
    if (!World) return;

    float Now = World->GetTimeSeconds();

    MemoryLog.RemoveAll([Now, OlderThanSeconds](const FNPC_MemoryEntry& Entry)
    {
        return (Now - Entry.TimeStamp) > OlderThanSeconds;
    });
}

// ============================================================
// Relationship System
// ============================================================

void UNPCBehaviorComponent::SetRelationshipToPlayer(ENPC_Relationship NewRelationship)
{
    PlayerRelationship = NewRelationship;
}

void UNPCBehaviorComponent::ModifyRelationshipScore(float Delta)
{
    RelationshipScore = FMath::Clamp(RelationshipScore + Delta, RelationshipScoreMin, RelationshipScoreMax);
    PlayerRelationship = ScoreToRelationship(RelationshipScore);
}

ENPC_Relationship UNPCBehaviorComponent::ScoreToRelationship(float Score) const
{
    if (Score >= 60.0f)  return ENPC_Relationship::Trusted;
    if (Score >= 20.0f)  return ENPC_Relationship::Friendly;
    if (Score >= -20.0f) return ENPC_Relationship::Neutral;
    if (Score >= -60.0f) return ENPC_Relationship::Hostile;
    return ENPC_Relationship::Feared;
}

float UNPCBehaviorComponent::GetRelationshipScore() const
{
    return RelationshipScore;
}

// ============================================================
// Fear System
// ============================================================

void UNPCBehaviorComponent::AddFear(float Amount)
{
    FearLevel = FMath::Clamp(FearLevel + Amount, 0.0f, 100.0f);

    if (FearLevel >= FearPanicThreshold && CurrentAlertLevel < ENPC_AlertLevel::Panicked)
    {
        SetAlertLevel(ENPC_AlertLevel::Panicked);
    }
}

void UNPCBehaviorComponent::DecayFear(float DeltaTime)
{
    if (FearLevel > 0.0f)
    {
        FearLevel = FMath::Max(0.0f, FearLevel - FearDecayRate * DeltaTime);
    }
}
