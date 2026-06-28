#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

// ============================================================
UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz — sufficient for NPC logic

    CurrentBehaviorState = ENPC_BehaviorState::Idle;
    AlertLevel           = ENPC_AlertLevel::Calm;
    CurrentDailyPhase    = ENPC_DailyPhase::Morning;
    CurrentAlertValue    = 0.0f;
    LastKnownThreatLocation = FVector::ZeroVector;
    WorldTimeCache       = 0.0f;
}

// ============================================================
void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();
    // Initialise daily phase from world time
    UpdateDailyPhaseFromTime();
    // Set initial behavior from phase
    CurrentBehaviorState = GetPreferredStateForPhase(CurrentDailyPhase);
}

// ============================================================
void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    WorldTimeCache += DeltaTime;

    // Update daily phase every 30 seconds of game time
    if (FMath::Fmod(WorldTimeCache, 30.0f) < DeltaTime)
    {
        UpdateDailyPhaseFromTime();
    }

    // Decay alert over time
    UpdateAlertDecay(DeltaTime);

    // Drive behavior state from alert level
    UpdateBehaviorStateFromAlert();

    // Forget old memories
    if (FMath::Fmod(WorldTimeCache, 60.0f) < DeltaTime)
    {
        ForgetOldMemories(MemoryRetentionTime);
    }
}

// ============================================================
void UNPCBehaviorComponent::RaiseAlert(float ThreatLevel, FVector ThreatLocation, bool bIsPlayer)
{
    CurrentAlertValue = FMath::Clamp(CurrentAlertValue + ThreatLevel, 0.0f, 1.0f);
    LastKnownThreatLocation = ThreatLocation;
    AlertLevel = AlertValueToLevel(CurrentAlertValue);

    // Record in memory
    RecordMemory(ThreatLocation, ThreatLevel, bIsPlayer,
        bIsPlayer ? TEXT("Player detected") : TEXT("Threat detected"));

    // If very alarmed, propagate to nearby NPCs
    if (CurrentAlertValue >= 0.6f)
    {
        PropagateAlertToNearbyNPCs(HearingRange * 1.5f);
    }
}

// ============================================================
void UNPCBehaviorComponent::ClearAlert()
{
    CurrentAlertValue = 0.0f;
    AlertLevel = ENPC_AlertLevel::Calm;
    LastKnownThreatLocation = FVector::ZeroVector;
}

// ============================================================
void UNPCBehaviorComponent::RecordMemory(FVector Location, float ThreatLevel,
    bool bIsPlayer, const FString& Description)
{
    FNPC_MemoryEntry Entry;
    Entry.Location       = Location;
    Entry.Timestamp      = WorldTimeCache;
    Entry.ThreatLevel    = ThreatLevel;
    Entry.bIsPlayerRelated = bIsPlayer;
    Entry.EventDescription = Description;

    MemoryLog.Add(Entry);

    // Cap memory at 20 entries (FIFO)
    if (MemoryLog.Num() > 20)
    {
        MemoryLog.RemoveAt(0);
    }
}

// ============================================================
bool UNPCBehaviorComponent::HasMemoryOfPlayer() const
{
    for (const FNPC_MemoryEntry& Entry : MemoryLog)
    {
        if (Entry.bIsPlayerRelated)
        {
            return true;
        }
    }
    return false;
}

// ============================================================
float UNPCBehaviorComponent::GetPlayerThreatMemory() const
{
    float MaxThreat = 0.0f;
    for (const FNPC_MemoryEntry& Entry : MemoryLog)
    {
        if (Entry.bIsPlayerRelated)
        {
            MaxThreat = FMath::Max(MaxThreat, Entry.ThreatLevel);
        }
    }
    return MaxThreat;
}

// ============================================================
void UNPCBehaviorComponent::ForgetOldMemories(float MaxAge)
{
    MemoryLog.RemoveAll([this, MaxAge](const FNPC_MemoryEntry& Entry)
    {
        return (WorldTimeCache - Entry.Timestamp) > MaxAge;
    });
}

// ============================================================
void UNPCBehaviorComponent::SetPatrolRoute(const FNPC_PatrolRoute& NewRoute)
{
    PatrolRoute = NewRoute;
    PatrolRoute.CurrentWaypointIndex = 0;
}

// ============================================================
FVector UNPCBehaviorComponent::GetNextPatrolWaypoint()
{
    if (PatrolRoute.Waypoints.Num() == 0)
    {
        return GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
    }
    int32 Idx = FMath::Clamp(PatrolRoute.CurrentWaypointIndex,
        0, PatrolRoute.Waypoints.Num() - 1);
    return PatrolRoute.Waypoints[Idx];
}

// ============================================================
void UNPCBehaviorComponent::AdvancePatrolWaypoint()
{
    if (PatrolRoute.Waypoints.Num() == 0) return;

    PatrolRoute.CurrentWaypointIndex++;
    if (PatrolRoute.CurrentWaypointIndex >= PatrolRoute.Waypoints.Num())
    {
        PatrolRoute.CurrentWaypointIndex = PatrolRoute.bLoopRoute ? 0 : PatrolRoute.Waypoints.Num() - 1;
    }
}

// ============================================================
void UNPCBehaviorComponent::SetDailyPhase(ENPC_DailyPhase NewPhase)
{
    CurrentDailyPhase = NewPhase;
    // If calm, shift behavior to phase preference
    if (AlertLevel == ENPC_AlertLevel::Calm)
    {
        CurrentBehaviorState = GetPreferredStateForPhase(NewPhase);
    }
}

// ============================================================
ENPC_BehaviorState UNPCBehaviorComponent::GetPreferredStateForPhase(ENPC_DailyPhase Phase) const
{
    switch (Phase)
    {
    case ENPC_DailyPhase::Dawn:      return ENPC_BehaviorState::Patrol;   // Scout at dawn
    case ENPC_DailyPhase::Morning:   return ENPC_BehaviorState::Forage;   // Gather food
    case ENPC_DailyPhase::Midday:    return ENPC_BehaviorState::Forage;   // Continue foraging
    case ENPC_DailyPhase::Afternoon: return ENPC_BehaviorState::Socialise;// Social time
    case ENPC_DailyPhase::Dusk:      return ENPC_BehaviorState::Patrol;   // Return to camp
    case ENPC_DailyPhase::Night:     return ENPC_BehaviorState::Rest;     // Sleep/guard
    case ENPC_DailyPhase::DeepNight: return ENPC_BehaviorState::Rest;     // Deep sleep
    default:                         return ENPC_BehaviorState::Idle;
    }
}

// ============================================================
void UNPCBehaviorComponent::PropagateAlertToNearbyNPCs(float Radius)
{
    if (!GetOwner()) return;

    FVector MyLocation = GetOwner()->GetActorLocation();
    UWorld* World = GetWorld();
    if (!World) return;

    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(World, APawn::StaticClass(), NearbyActors);

    for (AActor* Actor : NearbyActors)
    {
        if (Actor == GetOwner()) continue;

        float Dist = FVector::Dist(MyLocation, Actor->GetActorLocation());
        if (Dist <= Radius)
        {
            UNPCBehaviorComponent* OtherComp =
                Actor->FindComponentByClass<UNPCBehaviorComponent>();
            if (OtherComp)
            {
                // Propagate reduced alert (distance falloff)
                float PropagatedThreat = CurrentAlertValue * (1.0f - (Dist / Radius)) * 0.6f;
                OtherComp->RaiseAlert(PropagatedThreat, LastKnownThreatLocation, false);
            }
        }
    }
}

// ============================================================
// Private helpers
// ============================================================
void UNPCBehaviorComponent::UpdateAlertDecay(float DeltaTime)
{
    if (CurrentAlertValue > 0.0f)
    {
        CurrentAlertValue = FMath::Clamp(
            CurrentAlertValue - (AlertDecayRate * DeltaTime), 0.0f, 1.0f);
        AlertLevel = AlertValueToLevel(CurrentAlertValue);
    }
}

// ============================================================
void UNPCBehaviorComponent::UpdateBehaviorStateFromAlert()
{
    if (CurrentBehaviorState == ENPC_BehaviorState::Dead) return;

    if (CurrentAlertValue >= FightThreshold)
    {
        CurrentBehaviorState = ENPC_BehaviorState::Combat;
    }
    else if (CurrentAlertValue >= FleeThreshold)
    {
        CurrentBehaviorState = ENPC_BehaviorState::Flee;
    }
    else if (CurrentAlertValue >= 0.3f)
    {
        CurrentBehaviorState = ENPC_BehaviorState::Seek;
    }
    else
    {
        // Return to daily routine when calm
        CurrentBehaviorState = GetPreferredStateForPhase(CurrentDailyPhase);
    }
}

// ============================================================
void UNPCBehaviorComponent::UpdateDailyPhaseFromTime()
{
    // Map WorldTimeCache (seconds) to a 24-hour cycle (1 real minute = 1 game hour)
    float GameHour = FMath::Fmod(WorldTimeCache / 60.0f, 24.0f);

    if      (GameHour >= 5.0f  && GameHour < 7.0f)  CurrentDailyPhase = ENPC_DailyPhase::Dawn;
    else if (GameHour >= 7.0f  && GameHour < 12.0f) CurrentDailyPhase = ENPC_DailyPhase::Morning;
    else if (GameHour >= 12.0f && GameHour < 15.0f) CurrentDailyPhase = ENPC_DailyPhase::Midday;
    else if (GameHour >= 15.0f && GameHour < 18.0f) CurrentDailyPhase = ENPC_DailyPhase::Afternoon;
    else if (GameHour >= 18.0f && GameHour < 20.0f) CurrentDailyPhase = ENPC_DailyPhase::Dusk;
    else if (GameHour >= 20.0f && GameHour < 23.0f) CurrentDailyPhase = ENPC_DailyPhase::Night;
    else                                              CurrentDailyPhase = ENPC_DailyPhase::DeepNight;
}

// ============================================================
ENPC_AlertLevel UNPCBehaviorComponent::AlertValueToLevel(float Value) const
{
    if      (Value >= 0.9f) return ENPC_AlertLevel::Fighting;
    else if (Value >= 0.7f) return ENPC_AlertLevel::Fleeing;
    else if (Value >= 0.5f) return ENPC_AlertLevel::Alarmed;
    else if (Value >= 0.3f) return ENPC_AlertLevel::Suspicious;
    else if (Value >= 0.1f) return ENPC_AlertLevel::Curious;
    else                    return ENPC_AlertLevel::Calm;
}
