#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

// ============================================================
// Constructor
// ============================================================
UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz — sufficient for NPC logic
}

// ============================================================
// BeginPlay — build a default schedule if none assigned
// ============================================================
void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    if (DailySchedule.Num() == 0)
    {
        BuildDefaultGathererSchedule();
    }

    // Sync phase to current game hour
    CurrentPhase = GetPhaseForHour(CurrentGameHour);
}

// ============================================================
// TickComponent
// ============================================================
void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    TickSchedule(DeltaTime);
    TickMemoryDecay(DeltaTime);
    TickThreatDecay(DeltaTime);
}

// ============================================================
// TickSchedule — advance game time and update current phase
// ============================================================
void UNPCBehaviorComponent::TickSchedule(float DeltaTime)
{
    GameTimeAccumulator += DeltaTime;

    // Advance game hour
    CurrentGameHour += DeltaTime * GameHoursPerSecond;
    if (CurrentGameHour >= 24.0f)
    {
        CurrentGameHour -= 24.0f;
    }

    ENPC_RoutinePhase NewPhase = GetPhaseForHour(CurrentGameHour);
    if (NewPhase != CurrentPhase)
    {
        CurrentPhase = NewPhase;
    }
}

// ============================================================
// TickMemoryDecay — old memories fade over time
// ============================================================
void UNPCBehaviorComponent::TickMemoryDecay(float DeltaTime)
{
    for (int32 i = EpisodicMemory.Num() - 1; i >= 0; --i)
    {
        EpisodicMemory[i].ThreatScore -= MemoryDecayRate * DeltaTime;
        if (EpisodicMemory[i].ThreatScore <= 0.0f)
        {
            EpisodicMemory.RemoveAt(i);
        }
    }
}

// ============================================================
// TickThreatDecay — threat level recovers when nothing happens
// ============================================================
void UNPCBehaviorComponent::TickThreatDecay(float DeltaTime)
{
    if (ThreatScore > 0.0f)
    {
        ThreatScore = FMath::Max(0.0f, ThreatScore - (0.5f * DeltaTime));
        UpdateThreatLevel(ThreatScore);
    }
}

// ============================================================
// RecordMemory
// ============================================================
void UNPCBehaviorComponent::RecordMemory(FVector Location, float Threat, bool bDangerous, FName Tag)
{
    FNPC_MemoryRecord Record;
    Record.EventLocation = Location;
    Record.EventTime = CurrentGameHour;
    Record.ThreatScore = FMath::Clamp(Threat, 0.0f, 10.0f);
    Record.bWasDangerous = bDangerous;
    Record.EventTag = Tag;

    EpisodicMemory.Add(Record);

    // Trim oldest memories if over capacity
    while (EpisodicMemory.Num() > MaxMemoryRecords)
    {
        EpisodicMemory.RemoveAt(0);
    }

    // Immediately spike threat score
    ThreatScore = FMath::Max(ThreatScore, Threat);
    UpdateThreatLevel(ThreatScore);
}

// ============================================================
// UpdateThreatLevel — map float score to enum
// ============================================================
void UNPCBehaviorComponent::UpdateThreatLevel(float NewThreatScore)
{
    ThreatScore = FMath::Clamp(NewThreatScore, 0.0f, 10.0f);

    if (ThreatScore <= 0.0f)
        CurrentThreatLevel = ENPC_ThreatLevel::None;
    else if (ThreatScore < 2.0f)
        CurrentThreatLevel = ENPC_ThreatLevel::Curious;
    else if (ThreatScore < 4.0f)
        CurrentThreatLevel = ENPC_ThreatLevel::Wary;
    else if (ThreatScore < 6.0f)
        CurrentThreatLevel = ENPC_ThreatLevel::Alarmed;
    else if (ThreatScore < 8.0f)
        CurrentThreatLevel = ENPC_ThreatLevel::Panicking;
    else
        CurrentThreatLevel = ENPC_ThreatLevel::Fleeing;
}

// ============================================================
// GetPhaseForHour — find which schedule entry covers this hour
// ============================================================
ENPC_RoutinePhase UNPCBehaviorComponent::GetPhaseForHour(float Hour) const
{
    // Walk schedule entries; last one that started before Hour wins
    ENPC_RoutinePhase Result = ENPC_RoutinePhase::Resting;
    for (const FNPC_ScheduleEntry& Entry : DailySchedule)
    {
        float End = Entry.StartHour + Entry.DurationHours;
        if (Hour >= Entry.StartHour && Hour < End)
        {
            Result = Entry.Phase;
            break;
        }
    }
    return Result;
}

// ============================================================
// SetGameHour — external time sync (e.g. from day/night cycle)
// ============================================================
void UNPCBehaviorComponent::SetGameHour(float Hour)
{
    CurrentGameHour = FMath::Fmod(FMath::Abs(Hour), 24.0f);
    CurrentPhase = GetPhaseForHour(CurrentGameHour);
}

// ============================================================
// IsInDanger
// ============================================================
bool UNPCBehaviorComponent::IsInDanger() const
{
    return CurrentThreatLevel >= ENPC_ThreatLevel::Alarmed;
}

// ============================================================
// GetMostDangerousMemoryLocation
// ============================================================
FVector UNPCBehaviorComponent::GetMostDangerousMemoryLocation() const
{
    if (EpisodicMemory.Num() == 0)
    {
        return FVector::ZeroVector;
    }

    const FNPC_MemoryRecord* Worst = nullptr;
    for (const FNPC_MemoryRecord& Rec : EpisodicMemory)
    {
        if (!Worst || Rec.ThreatScore > Worst->ThreatScore)
        {
            Worst = &Rec;
        }
    }
    return Worst ? Worst->EventLocation : FVector::ZeroVector;
}

// ============================================================
// BuildDefaultHunterSchedule — active hunter NPC routine
// ============================================================
void UNPCBehaviorComponent::BuildDefaultHunterSchedule()
{
    DailySchedule.Empty();

    auto Add = [&](ENPC_RoutinePhase Phase, float Start, float Duration)
    {
        FNPC_ScheduleEntry E;
        E.Phase = Phase;
        E.StartHour = Start;
        E.DurationHours = Duration;
        DailySchedule.Add(E);
    };

    Add(ENPC_RoutinePhase::Sleeping,    0.0f,  5.0f);  // 00:00 - 05:00
    Add(ENPC_RoutinePhase::Waking,      5.0f,  1.0f);  // 05:00 - 06:00
    Add(ENPC_RoutinePhase::Hunting,     6.0f,  4.0f);  // 06:00 - 10:00
    Add(ENPC_RoutinePhase::Resting,    10.0f,  2.0f);  // 10:00 - 12:00
    Add(ENPC_RoutinePhase::Patrolling, 12.0f,  3.0f);  // 12:00 - 15:00
    Add(ENPC_RoutinePhase::Hunting,    15.0f,  3.0f);  // 15:00 - 18:00
    Add(ENPC_RoutinePhase::Socializing,18.0f,  2.0f);  // 18:00 - 20:00
    Add(ENPC_RoutinePhase::Resting,    20.0f,  2.0f);  // 20:00 - 22:00
    Add(ENPC_RoutinePhase::Sleeping,   22.0f,  2.0f);  // 22:00 - 24:00
}

// ============================================================
// BuildDefaultGathererSchedule — forager NPC routine
// ============================================================
void UNPCBehaviorComponent::BuildDefaultGathererSchedule()
{
    DailySchedule.Empty();

    auto Add = [&](ENPC_RoutinePhase Phase, float Start, float Duration)
    {
        FNPC_ScheduleEntry E;
        E.Phase = Phase;
        E.StartHour = Start;
        E.DurationHours = Duration;
        DailySchedule.Add(E);
    };

    Add(ENPC_RoutinePhase::Sleeping,    0.0f,  6.0f);  // 00:00 - 06:00
    Add(ENPC_RoutinePhase::Waking,      6.0f,  1.0f);  // 06:00 - 07:00
    Add(ENPC_RoutinePhase::Foraging,    7.0f,  3.0f);  // 07:00 - 10:00
    Add(ENPC_RoutinePhase::Crafting,   10.0f,  2.0f);  // 10:00 - 12:00
    Add(ENPC_RoutinePhase::Resting,    12.0f,  1.0f);  // 12:00 - 13:00
    Add(ENPC_RoutinePhase::Foraging,   13.0f,  3.0f);  // 13:00 - 16:00
    Add(ENPC_RoutinePhase::Socializing,16.0f,  2.0f);  // 16:00 - 18:00
    Add(ENPC_RoutinePhase::Hiding,     18.0f,  2.0f);  // 18:00 - 20:00 (dusk = danger)
    Add(ENPC_RoutinePhase::Resting,    20.0f,  2.0f);  // 20:00 - 22:00
    Add(ENPC_RoutinePhase::Sleeping,   22.0f,  2.0f);  // 22:00 - 24:00
}
