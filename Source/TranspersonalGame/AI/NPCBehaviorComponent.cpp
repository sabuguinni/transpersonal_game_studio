// NPCBehaviorComponent.cpp
// NPC Behavior Agent #11 — Transpersonal Game Studio
// Implements daily routines, threat response, memory, and social relations.

#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

// ============================================================
// Constructor
// ============================================================
UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz — sufficient for NPC logic
}

// ============================================================
// BeginPlay
// ============================================================
void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    // Default schedule: gather at dawn, patrol at morning, idle at midday, gather at dusk, idle at night
    FNPC_ScheduleSlot DawnSlot;
    DawnSlot.TimeOfDay = ENPC_TimeOfDay::Dawn;
    DawnSlot.DesiredState = ENPC_BehaviorState::Gather;
    DawnSlot.Duration = 120.0f;
    DailySchedule.Add(DawnSlot);

    FNPC_ScheduleSlot MorningSlot;
    MorningSlot.TimeOfDay = ENPC_TimeOfDay::Morning;
    MorningSlot.DesiredState = ENPC_BehaviorState::Patrol;
    MorningSlot.Duration = 180.0f;
    DailySchedule.Add(MorningSlot);

    FNPC_ScheduleSlot MiddaySlot;
    MiddaySlot.TimeOfDay = ENPC_TimeOfDay::Midday;
    MiddaySlot.DesiredState = ENPC_BehaviorState::Idle;
    MiddaySlot.Duration = 90.0f;
    DailySchedule.Add(MiddaySlot);

    FNPC_ScheduleSlot DuskSlot;
    DuskSlot.TimeOfDay = ENPC_TimeOfDay::Dusk;
    DuskSlot.DesiredState = ENPC_BehaviorState::Gather;
    DuskSlot.Duration = 120.0f;
    DailySchedule.Add(DuskSlot);

    FNPC_ScheduleSlot NightSlot;
    NightSlot.TimeOfDay = ENPC_TimeOfDay::Night;
    NightSlot.DesiredState = ENPC_BehaviorState::Idle;
    NightSlot.Duration = 240.0f;
    DailySchedule.Add(NightSlot);
}

// ============================================================
// TickComponent
// ============================================================
void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateThreatDecay(DeltaTime);
    UpdateFearDecay(DeltaTime);
}

// ============================================================
// State
// ============================================================
void UNPCBehaviorComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (CurrentState == NewState) return;
    CurrentState = NewState;
}

void UNPCBehaviorComponent::EvaluateThreat(AActor* ThreatSource, float ThreatScore)
{
    if (!ThreatSource) return;

    CurrentThreatScore = FMath::Max(CurrentThreatScore, ThreatScore);
    CurrentThreatLevel = ScoreToThreatLevel(CurrentThreatScore);

    // Add to memory
    AddMemory(ThreatSource, ThreatSource->GetActorLocation(), ThreatScore, ThreatScore > 50.0f);

    // React: high threat → flee, medium → investigate, low → seek
    if (CurrentThreatLevel >= ENPC_ThreatLevel::High)
    {
        SetBehaviorState(ENPC_BehaviorState::Flee);
        ModifyFear(ThreatScore * 0.5f);
    }
    else if (CurrentThreatLevel == ENPC_ThreatLevel::Medium)
    {
        SetBehaviorState(ENPC_BehaviorState::Investigate);
    }
    else if (CurrentThreatLevel == ENPC_ThreatLevel::Low)
    {
        SetBehaviorState(ENPC_BehaviorState::Seek);
    }
}

// ============================================================
// Memory
// ============================================================
void UNPCBehaviorComponent::AddMemory(AActor* Source, FVector Location, float Threat, bool bDangerous)
{
    if (!Source) return;

    float Now = 0.0f;
    if (UWorld* World = GetWorld())
    {
        Now = World->GetTimeSeconds();
    }

    // Update existing memory if source already known
    for (FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        if (Entry.SourceActor == Source)
        {
            Entry.LastKnownLocation = Location;
            Entry.ThreatScore = FMath::Max(Entry.ThreatScore, Threat);
            Entry.TimeStamp = Now;
            Entry.bIsDangerous = bDangerous || Entry.bIsDangerous;
            Entry.ThreatLevel = ScoreToThreatLevel(Entry.ThreatScore);
            return;
        }
    }

    // New memory entry
    FNPC_MemoryEntry NewEntry;
    NewEntry.SourceActor = Source;
    NewEntry.LastKnownLocation = Location;
    NewEntry.ThreatScore = Threat;
    NewEntry.TimeStamp = Now;
    NewEntry.bIsDangerous = bDangerous;
    NewEntry.ThreatLevel = ScoreToThreatLevel(Threat);
    MemoryEntries.Add(NewEntry);
}

void UNPCBehaviorComponent::ForgetOldMemories(float MaxAge)
{
    float Now = 0.0f;
    if (UWorld* World = GetWorld())
    {
        Now = World->GetTimeSeconds();
    }

    MemoryEntries.RemoveAll([Now, MaxAge](const FNPC_MemoryEntry& Entry)
    {
        return (Now - Entry.TimeStamp) > MaxAge;
    });
}

bool UNPCBehaviorComponent::HasMemoryOf(AActor* Source) const
{
    if (!Source) return false;
    for (const FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        if (Entry.SourceActor == Source) return true;
    }
    return false;
}

FNPC_MemoryEntry UNPCBehaviorComponent::GetMostDangerousMemory() const
{
    FNPC_MemoryEntry Best;
    float BestScore = -1.0f;
    for (const FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        if (Entry.ThreatScore > BestScore)
        {
            BestScore = Entry.ThreatScore;
            Best = Entry;
        }
    }
    return Best;
}

// ============================================================
// Schedule
// ============================================================
void UNPCBehaviorComponent::AddScheduleSlot(FNPC_ScheduleSlot Slot)
{
    DailySchedule.Add(Slot);
}

void UNPCBehaviorComponent::EvaluateSchedule(ENPC_TimeOfDay CurrentTimeOfDay)
{
    // Only apply schedule if not currently threatened
    if (CurrentThreatLevel >= ENPC_ThreatLevel::Medium) return;

    for (const FNPC_ScheduleSlot& Slot : DailySchedule)
    {
        if (Slot.TimeOfDay == CurrentTimeOfDay)
        {
            SetBehaviorState(Slot.DesiredState);
            return;
        }
    }
}

// ============================================================
// Social
// ============================================================
void UNPCBehaviorComponent::SetRelationToPlayer(ENPC_SocialRelation Relation)
{
    RelationToPlayer = Relation;
}

void UNPCBehaviorComponent::ModifyFear(float Delta)
{
    FearLevel = FMath::Clamp(FearLevel + Delta, 0.0f, 100.0f);
}

// ============================================================
// Patrol
// ============================================================
void UNPCBehaviorComponent::SetPatrolPoints(const TArray<FVector>& Points)
{
    PatrolPoints = Points;
    CurrentPatrolIndex = 0;
}

FVector UNPCBehaviorComponent::GetNextPatrolPoint()
{
    if (PatrolPoints.Num() == 0) return FVector::ZeroVector;
    return PatrolPoints[CurrentPatrolIndex % PatrolPoints.Num()];
}

void UNPCBehaviorComponent::AdvancePatrolIndex()
{
    if (PatrolPoints.Num() == 0) return;
    CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
}

// ============================================================
// Internal helpers
// ============================================================
void UNPCBehaviorComponent::UpdateThreatDecay(float DeltaTime)
{
    if (CurrentThreatScore > 0.0f)
    {
        CurrentThreatScore = FMath::Max(0.0f, CurrentThreatScore - ThreatDecayRate * DeltaTime);
        CurrentThreatLevel = ScoreToThreatLevel(CurrentThreatScore);

        // Return to schedule-driven state when threat clears
        if (CurrentThreatScore <= 0.0f && CurrentState == ENPC_BehaviorState::Flee)
        {
            SetBehaviorState(ENPC_BehaviorState::Idle);
        }
    }

    // Periodically forget old memories
    if (UWorld* World = GetWorld())
    {
        static float LastForgetTime = 0.0f;
        float Now = World->GetTimeSeconds();
        if (Now - LastForgetTime > 30.0f)
        {
            ForgetOldMemories(MemoryMaxAge);
            LastForgetTime = Now;
        }
    }
}

void UNPCBehaviorComponent::UpdateFearDecay(float DeltaTime)
{
    if (FearLevel > 0.0f && CurrentThreatLevel == ENPC_ThreatLevel::None)
    {
        FearLevel = FMath::Max(0.0f, FearLevel - FearDecayRate * DeltaTime);
    }
}

ENPC_ThreatLevel UNPCBehaviorComponent::ScoreToThreatLevel(float Score) const
{
    if (Score <= 0.0f)  return ENPC_ThreatLevel::None;
    if (Score < 25.0f)  return ENPC_ThreatLevel::Low;
    if (Score < 50.0f)  return ENPC_ThreatLevel::Medium;
    if (Score < 75.0f)  return ENPC_ThreatLevel::High;
    return ENPC_ThreatLevel::Critical;
}
