#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz — sufficient for NPC logic

    // Default daily schedule: prehistoric human routine
    // 0-6: Sleep, 6-8: Wake/prepare, 8-18: Forage/patrol, 18-20: Socialize, 20-22: Rest, 22-24: Sleep
    FNPC_DailyScheduleEntry Sleep1;
    Sleep1.Phase = ENPC_RoutinePhase::Sleeping;
    Sleep1.StartHour = 0.0f;
    Sleep1.EndHour = 6.0f;

    FNPC_DailyScheduleEntry Wake;
    Wake.Phase = ENPC_RoutinePhase::Waking;
    Wake.StartHour = 6.0f;
    Wake.EndHour = 8.0f;

    FNPC_DailyScheduleEntry Forage;
    Forage.Phase = ENPC_RoutinePhase::Foraging;
    Forage.StartHour = 8.0f;
    Forage.EndHour = 12.0f;

    FNPC_DailyScheduleEntry Rest;
    Rest.Phase = ENPC_RoutinePhase::Resting;
    Rest.StartHour = 12.0f;
    Rest.EndHour = 14.0f;

    FNPC_DailyScheduleEntry Patrol;
    Patrol.Phase = ENPC_RoutinePhase::Patrolling;
    Patrol.StartHour = 14.0f;
    Patrol.EndHour = 18.0f;

    FNPC_DailyScheduleEntry Social;
    Social.Phase = ENPC_RoutinePhase::Socializing;
    Social.StartHour = 18.0f;
    Social.EndHour = 21.0f;

    FNPC_DailyScheduleEntry Sleep2;
    Sleep2.Phase = ENPC_RoutinePhase::Sleeping;
    Sleep2.StartHour = 21.0f;
    Sleep2.EndHour = 24.0f;

    DailySchedule.Add(Sleep1);
    DailySchedule.Add(Wake);
    DailySchedule.Add(Forage);
    DailySchedule.Add(Rest);
    DailySchedule.Add(Patrol);
    DailySchedule.Add(Social);
    DailySchedule.Add(Sleep2);
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();
    // Initialize routine phase based on starting game hour
    CurrentRoutinePhase = GetScheduledPhaseForHour(CurrentGameHour).Phase;
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    ElapsedTime += DeltaTime;

    UpdateAlertLevel(DeltaTime);
    UpdateRoutinePhase(DeltaTime);
    DecayThreatMemories(DeltaTime);
    UpdateSurvivalStats(DeltaTime);
}

// ============================================================
// ALERT SYSTEM
// ============================================================

void UNPCBehaviorComponent::AddThreat(AActor* ThreatSource, float ThreatAmount)
{
    if (!ThreatSource) return;

    CurrentAlertScore = FMath::Clamp(CurrentAlertScore + ThreatAmount, 0.0f, 100.0f);
    Fear = FMath::Clamp(Fear + ThreatAmount * 0.5f, 0.0f, 100.0f);

    // Record in memory
    RecordThreatSighting(ThreatSource, ThreatSource->GetActorLocation(), ThreatAmount);
}

void UNPCBehaviorComponent::ClearThreat(AActor* ThreatSource)
{
    if (!ThreatSource) return;

    ThreatMemories.RemoveAll([ThreatSource](const FNPC_ThreatMemory& Mem)
    {
        return Mem.ThreatActor == ThreatSource;
    });
}

void UNPCBehaviorComponent::UpdateAlertLevel(float DeltaTime)
{
    // Decay alert score over time (only when no active visible threats)
    bool bHasVisibleThreat = false;
    for (const FNPC_ThreatMemory& Mem : ThreatMemories)
    {
        if (Mem.bIsVisible) { bHasVisibleThreat = true; break; }
    }

    if (!bHasVisibleThreat)
    {
        CurrentAlertScore = FMath::Max(0.0f, CurrentAlertScore - AlertDecayRate * DeltaTime);
        Fear = FMath::Max(0.0f, Fear - AlertDecayRate * 0.3f * DeltaTime);
    }

    // Update alert level enum from score
    if (CurrentAlertScore >= AlertThresholdPanicked)
        CurrentAlertLevel = ENPC_AlertLevel::Panicked;
    else if (CurrentAlertScore >= AlertThresholdAlarmed)
        CurrentAlertLevel = ENPC_AlertLevel::Alarmed;
    else if (CurrentAlertScore >= AlertThresholdSuspicious)
        CurrentAlertLevel = ENPC_AlertLevel::Suspicious;
    else if (CurrentAlertScore >= AlertThresholdCurious)
        CurrentAlertLevel = ENPC_AlertLevel::Curious;
    else
        CurrentAlertLevel = ENPC_AlertLevel::Unaware;

    // Override routine phase if panicked or alarmed
    if (CurrentAlertLevel == ENPC_AlertLevel::Panicked)
        CurrentRoutinePhase = ENPC_RoutinePhase::Fleeing;
    else if (CurrentAlertLevel == ENPC_AlertLevel::Alarmed)
        CurrentRoutinePhase = ENPC_RoutinePhase::Investigating;
}

// ============================================================
// ROUTINE SYSTEM
// ============================================================

void UNPCBehaviorComponent::SetRoutinePhase(ENPC_RoutinePhase NewPhase)
{
    CurrentRoutinePhase = NewPhase;
}

FNPC_DailyScheduleEntry UNPCBehaviorComponent::GetScheduledPhaseForHour(float Hour) const
{
    for (const FNPC_DailyScheduleEntry& Entry : DailySchedule)
    {
        if (Hour >= Entry.StartHour && Hour < Entry.EndHour)
            return Entry;
    }
    // Default: resting
    FNPC_DailyScheduleEntry Default;
    Default.Phase = ENPC_RoutinePhase::Resting;
    return Default;
}

void UNPCBehaviorComponent::UpdateRoutinePhase(float DeltaTime)
{
    // Advance game hour (1 real second = 1 game minute → 24 game hours in 24 real minutes)
    CurrentGameHour += DeltaTime / 60.0f;
    if (CurrentGameHour >= 24.0f) CurrentGameHour -= 24.0f;

    // Only update routine if not in alert override
    if (CurrentAlertLevel == ENPC_AlertLevel::Unaware ||
        CurrentAlertLevel == ENPC_AlertLevel::Curious)
    {
        FNPC_DailyScheduleEntry Scheduled = GetScheduledPhaseForHour(CurrentGameHour);
        CurrentRoutinePhase = Scheduled.Phase;
    }
}

// ============================================================
// MEMORY SYSTEM
// ============================================================

void UNPCBehaviorComponent::RecordThreatSighting(AActor* Threat, FVector Location, float ThreatLevel)
{
    if (!Threat) return;

    // Update existing memory if present
    for (FNPC_ThreatMemory& Mem : ThreatMemories)
    {
        if (Mem.ThreatActor == Threat)
        {
            Mem.LastKnownLocation = Location;
            Mem.ThreatLevel = FMath::Max(Mem.ThreatLevel, ThreatLevel);
            Mem.TimeLastSeen = ElapsedTime;
            Mem.bIsVisible = true;
            return;
        }
    }

    // Add new memory (enforce max)
    if (ThreatMemories.Num() >= MaxThreatMemories)
    {
        // Remove oldest memory
        float OldestTime = ThreatMemories[0].TimeLastSeen;
        int32 OldestIdx = 0;
        for (int32 i = 1; i < ThreatMemories.Num(); ++i)
        {
            if (ThreatMemories[i].TimeLastSeen < OldestTime)
            {
                OldestTime = ThreatMemories[i].TimeLastSeen;
                OldestIdx = i;
            }
        }
        ThreatMemories.RemoveAt(OldestIdx);
    }

    FNPC_ThreatMemory NewMem;
    NewMem.ThreatActor = Threat;
    NewMem.LastKnownLocation = Location;
    NewMem.ThreatLevel = ThreatLevel;
    NewMem.TimeLastSeen = ElapsedTime;
    NewMem.bIsVisible = true;
    ThreatMemories.Add(NewMem);
}

bool UNPCBehaviorComponent::HasActiveThreatMemory() const
{
    return ThreatMemories.Num() > 0;
}

FNPC_ThreatMemory UNPCBehaviorComponent::GetMostDangerousThreat() const
{
    FNPC_ThreatMemory Best;
    float BestLevel = -1.0f;
    for (const FNPC_ThreatMemory& Mem : ThreatMemories)
    {
        if (Mem.ThreatLevel > BestLevel)
        {
            BestLevel = Mem.ThreatLevel;
            Best = Mem;
        }
    }
    return Best;
}

void UNPCBehaviorComponent::DecayThreatMemories(float DeltaTime)
{
    float CurrentTime = ElapsedTime;
    ThreatMemories.RemoveAll([this, CurrentTime](FNPC_ThreatMemory& Mem) -> bool
    {
        float Age = CurrentTime - Mem.TimeLastSeen;
        if (Age > MemoryDecayTime)
            return true; // Remove stale memory

        // Mark as not visible if old enough
        if (Age > 5.0f)
            Mem.bIsVisible = false;

        // Decay threat level
        Mem.ThreatLevel = FMath::Max(0.0f, Mem.ThreatLevel - 1.0f * DeltaTime);
        return false;
    });
}

// ============================================================
// SOCIAL SYSTEM
// ============================================================

void UNPCBehaviorComponent::UpdateSocialRelation(AActor* OtherActor, ENPC_SocialRelation NewRelation, float TrustDelta)
{
    if (!OtherActor) return;

    for (FNPC_SocialRecord& Record : SocialRecords)
    {
        if (Record.OtherActor == OtherActor)
        {
            Record.Relation = NewRelation;
            Record.TrustScore = FMath::Clamp(Record.TrustScore + TrustDelta, -100.0f, 100.0f);
            Record.LastInteractionTime = ElapsedTime;
            return;
        }
    }

    FNPC_SocialRecord NewRecord;
    NewRecord.OtherActor = OtherActor;
    NewRecord.Relation = NewRelation;
    NewRecord.TrustScore = TrustDelta;
    NewRecord.LastInteractionTime = ElapsedTime;
    SocialRecords.Add(NewRecord);
}

ENPC_SocialRelation UNPCBehaviorComponent::GetRelationTo(AActor* OtherActor) const
{
    if (!OtherActor) return ENPC_SocialRelation::Stranger;

    for (const FNPC_SocialRecord& Record : SocialRecords)
    {
        if (Record.OtherActor == OtherActor)
            return Record.Relation;
    }
    return ENPC_SocialRelation::Stranger;
}

// ============================================================
// SURVIVAL STATS
// ============================================================

void UNPCBehaviorComponent::UpdateSurvivalStats(float DeltaTime)
{
    // Hunger and thirst decay slowly over time
    float DecayRate = 0.5f * DeltaTime;

    if (CurrentRoutinePhase == ENPC_RoutinePhase::Foraging)
        DecayRate *= 2.0f; // Foraging burns more energy

    Hunger = FMath::Max(0.0f, Hunger - DecayRate);
    Thirst = FMath::Max(0.0f, Thirst - DecayRate * 1.2f);

    // Stamina recovers when resting or sleeping
    if (CurrentRoutinePhase == ENPC_RoutinePhase::Resting ||
        CurrentRoutinePhase == ENPC_RoutinePhase::Sleeping)
    {
        Stamina = FMath::Min(100.0f, Stamina + 2.0f * DeltaTime);
    }
    else if (CurrentRoutinePhase == ENPC_RoutinePhase::Fleeing)
    {
        Stamina = FMath::Max(0.0f, Stamina - 5.0f * DeltaTime);
    }

    // Trigger foraging if hungry/thirsty and not in danger
    if (NeedsFoodOrWater() && CurrentAlertLevel == ENPC_AlertLevel::Unaware)
    {
        CurrentRoutinePhase = ENPC_RoutinePhase::Foraging;
    }
}

bool UNPCBehaviorComponent::IsInDanger() const
{
    return CurrentAlertLevel >= ENPC_AlertLevel::Alarmed;
}

bool UNPCBehaviorComponent::NeedsFoodOrWater() const
{
    return Hunger < 30.0f || Thirst < 30.0f;
}
