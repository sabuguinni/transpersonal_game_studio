#include "NPCBehaviorComponent.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/World.h"

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz for performance
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    // Set default camp location to owner's start position
    if (AActor* Owner = GetOwner())
    {
        CampLocation = Owner->GetActorLocation();
        PatrolCenter = CampLocation;
    }

    // Build default daily routine for a tribal survivor
    // Dawn: wake, patrol perimeter
    AddRoutineSlot(6.0f,  ENPC_BehaviorState::Patrol,    CampLocation + FVector(400, 0, 0),   120.0f);
    // Morning: forage for food
    AddRoutineSlot(8.0f,  ENPC_BehaviorState::Forage,    CampLocation + FVector(800, 600, 0),  180.0f);
    // Midday: rest at camp
    AddRoutineSlot(12.0f, ENPC_BehaviorState::Rest,       CampLocation,                         90.0f);
    // Afternoon: forage again
    AddRoutineSlot(14.0f, ENPC_BehaviorState::Forage,    CampLocation + FVector(-600, 400, 0), 180.0f);
    // Evening: socialize at camp
    AddRoutineSlot(17.0f, ENPC_BehaviorState::Socialize,  CampLocation,                         120.0f);
    // Night: rest/sleep
    AddRoutineSlot(20.0f, ENPC_BehaviorState::Rest,       CampLocation,                         600.0f);
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    TickSurvivalStats(DeltaTime);
    TickStateLogic(DeltaTime);
    TickMemoryDecay(DeltaTime);

    StateTimer += DeltaTime;
}

void UNPCBehaviorComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (CurrentState == NewState) return;

    CurrentState = NewState;
    StateTimer = 0.0f;
}

void UNPCBehaviorComponent::RegisterThreat(FVector ThreatLocation, ENPC_DangerLevel Danger, const FString& ThreatTag)
{
    CurrentDangerLevel = Danger;

    // Add to memory
    AddMemoryEntry(ThreatLocation, Danger, ThreatTag);

    // React based on danger level
    switch (Danger)
    {
        case ENPC_DangerLevel::Critical:
        case ENPC_DangerLevel::High:
            Fear = FMath::Min(Fear + 40.0f, 100.0f);
            SetBehaviorState(ENPC_BehaviorState::Flee);
            break;
        case ENPC_DangerLevel::Medium:
            Fear = FMath::Min(Fear + 20.0f, 100.0f);
            SetBehaviorState(ENPC_BehaviorState::Alert);
            break;
        case ENPC_DangerLevel::Low:
            Fear = FMath::Min(Fear + 5.0f, 100.0f);
            SetBehaviorState(ENPC_BehaviorState::Alert);
            break;
        default:
            break;
    }
}

void UNPCBehaviorComponent::ClearThreat()
{
    CurrentDangerLevel = ENPC_DangerLevel::None;
    Fear = FMath::Max(Fear - 10.0f, 0.0f);

    if (CurrentState == ENPC_BehaviorState::Flee || CurrentState == ENPC_BehaviorState::Alert)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrol);
    }
}

bool UNPCBehaviorComponent::HasActiveThreat() const
{
    return CurrentDangerLevel != ENPC_DangerLevel::None;
}

void UNPCBehaviorComponent::AddMemoryEntry(FVector Location, ENPC_DangerLevel Danger, const FString& Tag)
{
    FNPC_MemoryEntry Entry;
    Entry.Location = Location;
    Entry.DangerLevel = Danger;
    Entry.ThreatTag = Tag;
    Entry.bIsActive = true;

    if (GetWorld())
    {
        Entry.Timestamp = GetWorld()->GetTimeSeconds();
    }

    MemoryLog.Add(Entry);

    // Cap memory at 32 entries
    if (MemoryLog.Num() > 32)
    {
        MemoryLog.RemoveAt(0);
    }
}

TArray<FNPC_MemoryEntry> UNPCBehaviorComponent::GetRecentMemories(float WithinSeconds) const
{
    TArray<FNPC_MemoryEntry> Recent;
    float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

    for (const FNPC_MemoryEntry& Entry : MemoryLog)
    {
        if (Entry.bIsActive && (Now - Entry.Timestamp) <= WithinSeconds)
        {
            Recent.Add(Entry);
        }
    }
    return Recent;
}

void UNPCBehaviorComponent::PurgeOldMemories(float OlderThanSeconds)
{
    float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

    for (FNPC_MemoryEntry& Entry : MemoryLog)
    {
        if ((Now - Entry.Timestamp) > OlderThanSeconds)
        {
            Entry.bIsActive = false;
        }
    }

    MemoryLog.RemoveAll([](const FNPC_MemoryEntry& E) { return !E.bIsActive; });
}

void UNPCBehaviorComponent::AddRoutineSlot(float TimeOfDay, ENPC_BehaviorState State, FVector Location, float Duration)
{
    FNPC_DailyRoutineSlot Slot;
    Slot.TimeOfDay = TimeOfDay;
    Slot.TargetState = State;
    Slot.TargetLocation = Location;
    Slot.Duration = Duration;
    DailyRoutine.Add(Slot);

    // Keep sorted by time
    DailyRoutine.Sort([](const FNPC_DailyRoutineSlot& A, const FNPC_DailyRoutineSlot& B)
    {
        return A.TimeOfDay < B.TimeOfDay;
    });
}

void UNPCBehaviorComponent::EvaluateRoutine(float CurrentTimeOfDay)
{
    // Don't override threat responses
    if (CurrentState == ENPC_BehaviorState::Flee || CurrentState == ENPC_BehaviorState::Alert)
    {
        return;
    }

    // Find the appropriate routine slot for current time
    for (int32 i = DailyRoutine.Num() - 1; i >= 0; --i)
    {
        if (CurrentTimeOfDay >= DailyRoutine[i].TimeOfDay)
        {
            SetBehaviorState(DailyRoutine[i].TargetState);
            return;
        }
    }

    // Default to last slot (night/rest) if before first slot
    if (DailyRoutine.Num() > 0)
    {
        SetBehaviorState(DailyRoutine.Last().TargetState);
    }
}

void UNPCBehaviorComponent::SetPatrolCenter(FVector Center, float Radius)
{
    PatrolCenter = Center;
    PatrolRadius = Radius;
    CurrentPatrolIndex = 0;
}

FVector UNPCBehaviorComponent::GetNextPatrolPoint() const
{
    // Generate 8 patrol waypoints around the center
    const int32 NumPoints = 8;
    float Angle = (CurrentPatrolIndex % NumPoints) * (360.0f / NumPoints);
    float Rad = FMath::DegreesToRadians(Angle);

    return PatrolCenter + FVector(
        FMath::Cos(Rad) * PatrolRadius,
        FMath::Sin(Rad) * PatrolRadius,
        0.0f
    );
}

void UNPCBehaviorComponent::TickSurvivalStats(float DeltaTime)
{
    // Hunger decreases over time
    Hunger = FMath::Max(Hunger - DeltaTime * 0.5f, 0.0f);

    // Energy decreases when active, recovers when resting
    if (CurrentState == ENPC_BehaviorState::Rest)
    {
        Energy = FMath::Min(Energy + DeltaTime * 2.0f, 100.0f);
    }
    else
    {
        Energy = FMath::Max(Energy - DeltaTime * 0.3f, 0.0f);
    }

    // Fear naturally decays
    Fear = FMath::Max(Fear - DeltaTime * 0.2f, 0.0f);

    // Low energy forces rest
    if (Energy < 10.0f && CurrentState != ENPC_BehaviorState::Flee)
    {
        SetBehaviorState(ENPC_BehaviorState::Rest);
    }

    // Hunger drives foraging
    if (Hunger < 20.0f && CurrentState == ENPC_BehaviorState::Idle)
    {
        SetBehaviorState(ENPC_BehaviorState::Forage);
    }
}

void UNPCBehaviorComponent::TickStateLogic(float DeltaTime)
{
    // State timeout — prevent getting stuck
    const float MaxStateTime = 300.0f; // 5 minutes max in any state
    if (StateTimer > MaxStateTime)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrol);
    }
}

void UNPCBehaviorComponent::TickMemoryDecay(float DeltaTime)
{
    MemoryDecayTimer += DeltaTime;
    if (MemoryDecayTimer >= 60.0f) // Purge old memories every 60 seconds
    {
        PurgeOldMemories(300.0f); // Remove memories older than 5 minutes
        MemoryDecayTimer = 0.0f;
    }
}
