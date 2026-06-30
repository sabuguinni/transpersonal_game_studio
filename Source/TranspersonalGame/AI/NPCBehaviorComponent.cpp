// NPCBehaviorComponent.cpp
// NPC Behavior Agent #11 — Transpersonal Game Studio
// Implements: alert escalation, daily routine scheduling, memory system

#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz — sufficient for NPC logic
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    // Record home location at spawn
    if (AActor* Owner = GetOwner())
    {
        HomeLocation = Owner->GetActorLocation();
    }

    // Set initial routine based on role
    switch (TribeRole)
    {
        case ENPC_TribeRole::Guard:
            SetRoutinePhase(ENPC_RoutinePhase::Patrolling);
            break;
        case ENPC_TribeRole::Scout:
            SetRoutinePhase(ENPC_RoutinePhase::Investigating);
            break;
        default:
            SetRoutinePhase(ENPC_RoutinePhase::Foraging);
            break;
    }
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Decay threat over time
    DecayThreat(DeltaTime);

    // Forget memories older than 120 seconds
    ForgetOldMemories(120.0f);
}

// === Alert System ===

void UNPCBehaviorComponent::SetAlertLevel(ENPC_AlertLevel NewLevel)
{
    if (CurrentAlertLevel == NewLevel) return;

    CurrentAlertLevel = NewLevel;

    // Transition routine phase based on alert
    switch (NewLevel)
    {
        case ENPC_AlertLevel::Panicked:
        case ENPC_AlertLevel::Alarmed:
            SetRoutinePhase(ENPC_RoutinePhase::Fleeing);
            break;
        case ENPC_AlertLevel::Suspicious:
            SetRoutinePhase(ENPC_RoutinePhase::Investigating);
            break;
        case ENPC_AlertLevel::Unaware:
            // Return to role-appropriate routine
            if (TribeRole == ENPC_TribeRole::Guard)
                SetRoutinePhase(ENPC_RoutinePhase::Patrolling);
            else
                SetRoutinePhase(ENPC_RoutinePhase::Foraging);
            break;
        default:
            break;
    }
}

void UNPCBehaviorComponent::RaiseThreat(float ThreatAmount, FVector ThreatLocation)
{
    CurrentThreatScore = FMath::Clamp(CurrentThreatScore + ThreatAmount, 0.0f, 100.0f);

    // Record this threat in memory
    RecordMemory(ThreatLocation, ThreatAmount, false);

    // Escalate alert level based on score
    if (CurrentThreatScore >= AlertThresholdPanicked)
        SetAlertLevel(ENPC_AlertLevel::Panicked);
    else if (CurrentThreatScore >= AlertThresholdAlarmed)
        SetAlertLevel(ENPC_AlertLevel::Alarmed);
    else if (CurrentThreatScore >= AlertThresholdSuspicious)
        SetAlertLevel(ENPC_AlertLevel::Suspicious);
    else if (CurrentThreatScore >= AlertThresholdCurious)
        SetAlertLevel(ENPC_AlertLevel::Curious);
}

void UNPCBehaviorComponent::DecayThreat(float DeltaTime)
{
    if (CurrentThreatScore <= 0.0f) return;

    CurrentThreatScore = FMath::Max(0.0f, CurrentThreatScore - ThreatDecayRate * DeltaTime);

    // De-escalate alert level as threat decays
    if (CurrentThreatScore < AlertThresholdCurious && CurrentAlertLevel != ENPC_AlertLevel::Unaware)
    {
        SetAlertLevel(ENPC_AlertLevel::Unaware);
    }
    else if (CurrentThreatScore < AlertThresholdSuspicious && CurrentAlertLevel == ENPC_AlertLevel::Alarmed)
    {
        SetAlertLevel(ENPC_AlertLevel::Suspicious);
    }
    else if (CurrentThreatScore < AlertThresholdAlarmed && CurrentAlertLevel == ENPC_AlertLevel::Panicked)
    {
        SetAlertLevel(ENPC_AlertLevel::Alarmed);
    }
}

// === Routine System ===

void UNPCBehaviorComponent::SetRoutinePhase(ENPC_RoutinePhase NewPhase)
{
    CurrentRoutinePhase = NewPhase;
}

void UNPCBehaviorComponent::UpdateDailyRoutine(float WorldTimeHours)
{
    // Only update routine if not in threat response
    if (CurrentAlertLevel >= ENPC_AlertLevel::Suspicious) return;

    // Hour-based routine schedule (0-24)
    if (WorldTimeHours >= 22.0f || WorldTimeHours < 5.0f)
    {
        SetRoutinePhase(ENPC_RoutinePhase::Sleeping);
    }
    else if (WorldTimeHours >= 5.0f && WorldTimeHours < 7.0f)
    {
        SetRoutinePhase(ENPC_RoutinePhase::Waking);
    }
    else if (WorldTimeHours >= 7.0f && WorldTimeHours < 12.0f)
    {
        // Morning — role-specific activity
        switch (TribeRole)
        {
            case ENPC_TribeRole::Hunter:
                SetRoutinePhase(ENPC_RoutinePhase::Patrolling);
                break;
            case ENPC_TribeRole::Scout:
                SetRoutinePhase(ENPC_RoutinePhase::Investigating);
                break;
            default:
                SetRoutinePhase(ENPC_RoutinePhase::Foraging);
                break;
        }
    }
    else if (WorldTimeHours >= 12.0f && WorldTimeHours < 14.0f)
    {
        SetRoutinePhase(ENPC_RoutinePhase::Resting);
    }
    else if (WorldTimeHours >= 14.0f && WorldTimeHours < 18.0f)
    {
        SetRoutinePhase(ENPC_RoutinePhase::Foraging);
    }
    else if (WorldTimeHours >= 18.0f && WorldTimeHours < 22.0f)
    {
        SetRoutinePhase(ENPC_RoutinePhase::Socializing);
    }
}

// === Memory System ===

void UNPCBehaviorComponent::RecordMemory(FVector Location, float ThreatLevel, bool bPlayerRelated)
{
    if (!GetWorld()) return;

    FNPC_MemoryEntry NewEntry;
    NewEntry.Location = Location;
    NewEntry.ThreatLevel = ThreatLevel;
    NewEntry.TimeStamp = GetWorld()->GetTimeSeconds();
    NewEntry.bIsPlayerRelated = bPlayerRelated;

    MemoryEntries.Add(NewEntry);

    // Cap memory at 20 entries
    if (MemoryEntries.Num() > 20)
    {
        MemoryEntries.RemoveAt(0);
    }
}

void UNPCBehaviorComponent::ForgetOldMemories(float MaxAge)
{
    if (!GetWorld()) return;

    float CurrentTime = GetWorld()->GetTimeSeconds();
    MemoryEntries.RemoveAll([CurrentTime, MaxAge](const FNPC_MemoryEntry& Entry)
    {
        return (CurrentTime - Entry.TimeStamp) > MaxAge;
    });
}

bool UNPCBehaviorComponent::HasMemoryOfThreat() const
{
    for (const FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        if (Entry.ThreatLevel > 10.0f)
        {
            return true;
        }
    }
    return false;
}
