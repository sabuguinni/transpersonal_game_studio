#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 Hz — sufficient for NPC logic

    // Default daily schedule: forage dawn, rest midday, patrol dusk, sleep night
    FNPC_ScheduleEntry Dawn;
    Dawn.StartHour = 5.0f;
    Dawn.EndHour = 10.0f;
    Dawn.Activity = ENPC_RoutineActivity::Forage;
    DailySchedule.Add(Dawn);

    FNPC_ScheduleEntry Midday;
    Midday.StartHour = 10.0f;
    Midday.EndHour = 14.0f;
    Midday.Activity = ENPC_RoutineActivity::Rest;
    DailySchedule.Add(Midday);

    FNPC_ScheduleEntry Afternoon;
    Afternoon.StartHour = 14.0f;
    Afternoon.EndHour = 19.0f;
    Afternoon.Activity = ENPC_RoutineActivity::Patrol;
    DailySchedule.Add(Afternoon);

    FNPC_ScheduleEntry Night;
    Night.StartHour = 19.0f;
    Night.EndHour = 5.0f;
    Night.Activity = ENPC_RoutineActivity::SeekShelter;
    DailySchedule.Add(Night);
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    // Initialise current activity from schedule
    UpdateSchedule(CurrentGameHour);
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Decay threat over time
    UpdateThreatDecay(DeltaTime);

    // Re-evaluate threat level from score
    EvaluateThreatLevel();

    // Override activity if under threat
    SyncActivityWithThreat();

    // Periodically prune expired memories
    TimeSinceLastMemoryPrune += DeltaTime;
    if (TimeSinceLastMemoryPrune >= MemoryPruneInterval)
    {
        float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
        PruneExpiredMemories(CurrentTime);
        TimeSinceLastMemoryPrune = 0.0f;
    }
}

// ── Routine System ────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::UpdateSchedule(float GameHour)
{
    CurrentGameHour = GameHour;
    ENPC_RoutineActivity Scheduled = GetScheduledActivityForHour(GameHour);

    // Only update if not overridden by threat
    if (ThreatLevel == ENPC_ThreatLevel::Calm || ThreatLevel == ENPC_ThreatLevel::Wary)
    {
        CurrentActivity = Scheduled;
    }
}

ENPC_RoutineActivity UNPCBehaviorComponent::GetScheduledActivityForHour(float Hour) const
{
    for (const FNPC_ScheduleEntry& Entry : DailySchedule)
    {
        // Handle overnight schedules (e.g. 19:00 → 05:00)
        if (Entry.StartHour > Entry.EndHour)
        {
            if (Hour >= Entry.StartHour || Hour < Entry.EndHour)
                return Entry.Activity;
        }
        else
        {
            if (Hour >= Entry.StartHour && Hour < Entry.EndHour)
                return Entry.Activity;
        }
    }
    return ENPC_RoutineActivity::Idle;
}

// ── Threat & Emotion System ───────────────────────────────────────────────────

void UNPCBehaviorComponent::RegisterThreat(FVector ThreatLocation, float ThreatMagnitude, bool bIsPlayer, FString StimulusTag)
{
    CurrentThreatScore = FMath::Min(CurrentThreatScore + ThreatMagnitude, 100.0f);

    // Create memory entry
    FNPC_MemoryEntry Entry;
    Entry.Location = ThreatLocation;
    Entry.Timestamp = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    Entry.ThreatScore = ThreatMagnitude;
    Entry.bIsPlayerRelated = bIsPlayer;
    Entry.StimulusTag = StimulusTag;

    AddMemoryEntry(Entry);

    // Broadcast alert to nearby NPCs if configured
    if (bCanAlertOthers && ThreatMagnitude >= 30.0f)
    {
        BroadcastAlert(ThreatLocation, ThreatMagnitude * 0.7f); // Attenuated alert
    }

    EvaluateThreatLevel();
    SyncActivityWithThreat();
}

void UNPCBehaviorComponent::ClearThreat()
{
    CurrentThreatScore = 0.0f;
    ThreatLevel = ENPC_ThreatLevel::Calm;
    UpdateSchedule(CurrentGameHour);
}

bool UNPCBehaviorComponent::IsUnderThreat() const
{
    return ThreatLevel >= ENPC_ThreatLevel::Alarmed;
}

void UNPCBehaviorComponent::UpdateThreatDecay(float DeltaTime)
{
    if (CurrentThreatScore > 0.0f)
    {
        CurrentThreatScore = FMath::Max(0.0f, CurrentThreatScore - ThreatDecayRate * DeltaTime);
    }
}

void UNPCBehaviorComponent::EvaluateThreatLevel()
{
    if (CurrentThreatScore <= 0.0f)
        ThreatLevel = ENPC_ThreatLevel::Calm;
    else if (CurrentThreatScore < 25.0f)
        ThreatLevel = ENPC_ThreatLevel::Wary;
    else if (CurrentThreatScore < 50.0f)
        ThreatLevel = ENPC_ThreatLevel::Alarmed;
    else if (CurrentThreatScore < 75.0f)
        ThreatLevel = ENPC_ThreatLevel::Panicked;
    else
        ThreatLevel = ENPC_ThreatLevel::Fleeing;
}

void UNPCBehaviorComponent::SyncActivityWithThreat()
{
    switch (ThreatLevel)
    {
    case ENPC_ThreatLevel::Wary:
        if (CurrentActivity != ENPC_RoutineActivity::Flee)
            CurrentActivity = ENPC_RoutineActivity::Investigate;
        break;
    case ENPC_ThreatLevel::Alarmed:
        CurrentActivity = ENPC_RoutineActivity::Alert;
        break;
    case ENPC_ThreatLevel::Panicked:
    case ENPC_ThreatLevel::Fleeing:
        CurrentActivity = ENPC_RoutineActivity::Flee;
        break;
    default:
        break; // Calm — schedule drives activity
    }
}

// ── Memory System ─────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::AddMemoryEntry(FNPC_MemoryEntry Entry)
{
    // Add to short-term memory
    ShortTermMemory.Add(Entry);

    // Cap short-term memory at 8 entries (remove oldest)
    while (ShortTermMemory.Num() > 8)
        ShortTermMemory.RemoveAt(0);

    // Commit to long-term memory if threat is high enough
    if (Entry.ThreatScore >= LongTermMemoryThreshold)
    {
        LongTermMemory.Add(Entry);
        while (LongTermMemory.Num() > 4)
            LongTermMemory.RemoveAt(0);
    }
}

void UNPCBehaviorComponent::PruneExpiredMemories(float CurrentTime)
{
    ShortTermMemory.RemoveAll([this, CurrentTime](const FNPC_MemoryEntry& E)
    {
        return (CurrentTime - E.Timestamp) > ShortTermMemoryDuration;
    });
    // Long-term memories never expire
}

bool UNPCBehaviorComponent::HasMemoryOfLocation(FVector Location, float Radius) const
{
    for (const FNPC_MemoryEntry& E : ShortTermMemory)
    {
        if (FVector::Dist(E.Location, Location) <= Radius)
            return true;
    }
    for (const FNPC_MemoryEntry& E : LongTermMemory)
    {
        if (FVector::Dist(E.Location, Location) <= Radius)
            return true;
    }
    return false;
}

FNPC_MemoryEntry UNPCBehaviorComponent::GetMostRecentThreatMemory() const
{
    if (ShortTermMemory.Num() > 0)
        return ShortTermMemory.Last();
    if (LongTermMemory.Num() > 0)
        return LongTermMemory.Last();
    return FNPC_MemoryEntry();
}

// ── Social Behavior ───────────────────────────────────────────────────────────

void UNPCBehaviorComponent::BroadcastAlert(FVector ThreatLocation, float ThreatMagnitude)
{
    if (!GetWorld()) return;

    AActor* Owner = GetOwner();
    if (!Owner) return;

    // Sphere overlap to find nearby actors with NPCBehaviorComponent
    TArray<AActor*> OverlapActors;
    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(Owner);

    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), OverlapActors);

    for (AActor* OtherActor : OverlapActors)
    {
        if (!OtherActor || OtherActor == Owner) continue;

        float Distance = FVector::Dist(Owner->GetActorLocation(), OtherActor->GetActorLocation());
        if (Distance > AlertBroadcastRadius) continue;

        UNPCBehaviorComponent* OtherBehavior = OtherActor->FindComponentByClass<UNPCBehaviorComponent>();
        if (OtherBehavior)
        {
            // Attenuate by distance
            float Attenuation = 1.0f - (Distance / AlertBroadcastRadius);
            OtherBehavior->ReceiveAlert(ThreatLocation, ThreatMagnitude * Attenuation);
        }
    }
}

void UNPCBehaviorComponent::ReceiveAlert(FVector ThreatLocation, float ThreatMagnitude)
{
    // Received alert is treated as a lower-priority threat (heard, not seen)
    RegisterThreat(ThreatLocation, ThreatMagnitude * 0.5f, false, TEXT("HeardAlert"));
}

// ── Patrol System ─────────────────────────────────────────────────────────────

FVector UNPCBehaviorComponent::GetNextPatrolWaypoint()
{
    if (PatrolWaypoints.Num() == 0)
    {
        // No waypoints — return current location
        return GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
    }
    return PatrolWaypoints[CurrentPatrolIndex % PatrolWaypoints.Num()];
}

void UNPCBehaviorComponent::AdvancePatrolIndex()
{
    if (PatrolWaypoints.Num() > 0)
    {
        CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolWaypoints.Num();
    }
}

void UNPCBehaviorComponent::GeneratePatrolWaypointsAround(FVector Center, float Radius, int32 Count)
{
    PatrolWaypoints.Empty();
    CurrentPatrolIndex = 0;

    for (int32 i = 0; i < Count; ++i)
    {
        float Angle = (float(i) / float(Count)) * 2.0f * PI;
        // Vary radius slightly for organic feel
        float VaryRadius = Radius * FMath::RandRange(0.7f, 1.0f);
        FVector WP = Center + FVector(FMath::Cos(Angle) * VaryRadius, FMath::Sin(Angle) * VaryRadius, 0.0f);
        PatrolWaypoints.Add(WP);
    }
}
