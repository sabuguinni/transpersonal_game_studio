#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz tick for performance
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    // Store home location for patrol radius calculations
    if (AActor* Owner = GetOwner())
    {
        HomeLocation = Owner->GetActorLocation();
    }

    // Default to foraging at game start
    CurrentRoutinePhase = ENPC_RoutinePhase::Foraging;
    CurrentAlertLevel = ENPC_AlertLevel::Unaware;
    CurrentThreatScore = 0.0f;
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Continuously decay threat score toward calm
    DecayThreat(DeltaTime);
}

void UNPCBehaviorComponent::RaiseThreat(float ThreatAmount, FVector ThreatLocation)
{
    CurrentThreatScore = FMath::Clamp(CurrentThreatScore + ThreatAmount, 0.0f, 100.0f);
    UpdateAlertLevelFromScore();

    // Record this threat in memory
    RecordMemory(ThreatLocation, ThreatAmount);

    // If alarmed or panicked, switch to flee routine
    if (CurrentAlertLevel == ENPC_AlertLevel::Alarmed || CurrentAlertLevel == ENPC_AlertLevel::Panicked)
    {
        SetRoutinePhase(ENPC_RoutinePhase::Fleeing);
    }
    else if (CurrentAlertLevel == ENPC_AlertLevel::Suspicious)
    {
        SetRoutinePhase(ENPC_RoutinePhase::Investigating);
    }
}

void UNPCBehaviorComponent::DecayThreat(float DeltaTime)
{
    if (CurrentThreatScore <= 0.0f)
        return;

    // Decay faster when not fleeing
    float DecayMultiplier = (CurrentRoutinePhase == ENPC_RoutinePhase::Fleeing) ? 0.3f : 1.0f;
    CurrentThreatScore = FMath::Max(0.0f, CurrentThreatScore - (ThreatDecayRate * DecayMultiplier * DeltaTime));
    UpdateAlertLevelFromScore();

    // Return to normal routine when calm
    if (CurrentThreatScore <= 0.0f &&
        (CurrentRoutinePhase == ENPC_RoutinePhase::Fleeing || CurrentRoutinePhase == ENPC_RoutinePhase::Investigating))
    {
        SetRoutinePhase(ENPC_RoutinePhase::Patrolling);
    }
}

void UNPCBehaviorComponent::UpdateAlertLevelFromScore()
{
    ENPC_AlertLevel NewLevel = ENPC_AlertLevel::Unaware;

    if (CurrentThreatScore >= AlertThresholdPanicked)
        NewLevel = ENPC_AlertLevel::Panicked;
    else if (CurrentThreatScore >= AlertThresholdAlarmed)
        NewLevel = ENPC_AlertLevel::Alarmed;
    else if (CurrentThreatScore >= AlertThresholdSuspicious)
        NewLevel = ENPC_AlertLevel::Suspicious;
    else if (CurrentThreatScore >= AlertThresholdCurious)
        NewLevel = ENPC_AlertLevel::Curious;

    CurrentAlertLevel = NewLevel;
}

void UNPCBehaviorComponent::SetRoutinePhase(ENPC_RoutinePhase NewPhase)
{
    if (CurrentRoutinePhase != NewPhase)
    {
        CurrentRoutinePhase = NewPhase;
    }
}

void UNPCBehaviorComponent::UpdateDailyRoutine(float GameTimeHours)
{
    // Only update routine if NPC is calm (not fleeing/investigating)
    if (CurrentAlertLevel >= ENPC_AlertLevel::Suspicious)
        return;

    // Prehistoric human daily schedule
    if (GameTimeHours >= 22.0f || GameTimeHours < 5.0f)
    {
        SetRoutinePhase(ENPC_RoutinePhase::Sleeping);
    }
    else if (GameTimeHours >= 5.0f && GameTimeHours < 7.0f)
    {
        SetRoutinePhase(ENPC_RoutinePhase::Waking);
    }
    else if (GameTimeHours >= 7.0f && GameTimeHours < 11.0f)
    {
        SetRoutinePhase(ENPC_RoutinePhase::Foraging);
    }
    else if (GameTimeHours >= 11.0f && GameTimeHours < 13.0f)
    {
        SetRoutinePhase(ENPC_RoutinePhase::Resting);
    }
    else if (GameTimeHours >= 13.0f && GameTimeHours < 17.0f)
    {
        SetRoutinePhase(ENPC_RoutinePhase::Patrolling);
    }
    else if (GameTimeHours >= 17.0f && GameTimeHours < 20.0f)
    {
        SetRoutinePhase(ENPC_RoutinePhase::Socializing);
    }
    else
    {
        SetRoutinePhase(ENPC_RoutinePhase::Resting);
    }
}

void UNPCBehaviorComponent::RecordMemory(FVector Location, float ThreatLevel)
{
    // Find an inactive slot or the oldest entry
    int32 TargetIndex = -1;
    float OldestTime = FLT_MAX;

    for (int32 i = 0; i < MemoryEntries.Num(); ++i)
    {
        if (!MemoryEntries[i].bIsActive)
        {
            TargetIndex = i;
            break;
        }
        if (MemoryEntries[i].Timestamp < OldestTime)
        {
            OldestTime = MemoryEntries[i].Timestamp;
            TargetIndex = i;
        }
    }

    // Add new entry
    FNPC_MemoryEntry NewEntry;
    NewEntry.Location = Location;
    NewEntry.ThreatLevel = ThreatLevel;
    NewEntry.bIsActive = true;

    UWorld* World = GetWorld();
    NewEntry.Timestamp = World ? World->GetTimeSeconds() : 0.0f;

    if (TargetIndex >= 0 && TargetIndex < MemoryEntries.Num())
    {
        MemoryEntries[TargetIndex] = NewEntry;
    }
    else if (MemoryEntries.Num() < MaxMemoryEntries)
    {
        MemoryEntries.Add(NewEntry);
    }
}

bool UNPCBehaviorComponent::HasActiveMemory() const
{
    for (const FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        if (Entry.bIsActive)
            return true;
    }
    return false;
}

FNPC_MemoryEntry UNPCBehaviorComponent::GetMostRecentMemory() const
{
    FNPC_MemoryEntry Best;
    float LatestTime = -1.0f;

    for (const FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        if (Entry.bIsActive && Entry.Timestamp > LatestTime)
        {
            LatestTime = Entry.Timestamp;
            Best = Entry;
        }
    }
    return Best;
}

FVector UNPCBehaviorComponent::GetNextPatrolPoint() const
{
    // Generate a random point within patrol radius from home
    float Angle = FMath::FRandRange(0.0f, 360.0f);
    float Distance = FMath::FRandRange(PatrolRadius * 0.3f, PatrolRadius);
    float RadAngle = FMath::DegreesToRadians(Angle);

    return HomeLocation + FVector(
        FMath::Cos(RadAngle) * Distance,
        FMath::Sin(RadAngle) * Distance,
        0.0f
    );
}
