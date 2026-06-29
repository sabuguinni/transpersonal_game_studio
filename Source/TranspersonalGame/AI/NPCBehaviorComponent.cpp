// NPCBehaviorComponent.cpp
// NPC Behavior Agent #11 — Transpersonal Game Studio
// Full implementation: state machine, memory decay, threat assessment,
// daily routine evaluation, patrol navigation, survival needs.

#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

// ─────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────
UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz — sufficient for NPC AI
}

// ─────────────────────────────────────────────────────────────
// BeginPlay
// ─────────────────────────────────────────────────────────────
void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    // Start in Idle, evaluate after first tick
    CurrentState = ENPC_BehaviorState::Idle;
    CurrentThreatLevel = ENPC_ThreatLevel::None;
    CurrentPatrolIndex = 0;
    StateTimer = 0.0f;
}

// ─────────────────────────────────────────────────────────────
// TickComponent
// ─────────────────────────────────────────────────────────────
void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    TickSurvivalNeeds(DeltaTime);
    TickStateTimer(DeltaTime);
    DecayMemories(DeltaTime);
    UpdateThreatLevel();
    EvaluateStateTransition();
}

// ─────────────────────────────────────────────────────────────
// State Machine
// ─────────────────────────────────────────────────────────────
void UNPCBehaviorComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (CurrentState == NewState) return;
    if (CurrentState == ENPC_BehaviorState::Dead) return; // Dead is terminal

    CurrentState = NewState;
    StateTimer = 0.0f;
}

void UNPCBehaviorComponent::EvaluateStateTransition()
{
    // Dead is terminal — no transitions out
    if (CurrentState == ENPC_BehaviorState::Dead) return;

    const float ThreatScore = GetAggregatedThreatScore();

    // Critical threat → always flee
    if (ThreatScore >= FleeThreshold && CurrentState != ENPC_BehaviorState::Flee)
    {
        SetBehaviorState(ENPC_BehaviorState::Flee);
        return;
    }

    // Moderate threat → investigate
    if (ThreatScore >= InvestigateThreshold && ThreatScore < FleeThreshold)
    {
        if (CurrentState == ENPC_BehaviorState::Idle || CurrentState == ENPC_BehaviorState::Patrol)
        {
            SetBehaviorState(ENPC_BehaviorState::Investigate);
            return;
        }
    }

    // Threat gone → return to patrol or idle
    if (ThreatScore < InvestigateThreshold)
    {
        if (CurrentState == ENPC_BehaviorState::Flee || CurrentState == ENPC_BehaviorState::Investigate)
        {
            if (PatrolPoints.Num() > 0)
            {
                SetBehaviorState(ENPC_BehaviorState::Patrol);
            }
            else
            {
                SetBehaviorState(ENPC_BehaviorState::Idle);
            }
            return;
        }
    }

    // Survival needs override patrol
    if (CurrentState == ENPC_BehaviorState::Patrol || CurrentState == ENPC_BehaviorState::Idle)
    {
        if (Hunger < 0.2f || Thirst < 0.2f)
        {
            SetBehaviorState(ENPC_BehaviorState::Seek);
            return;
        }
        if (Fatigue > 0.85f)
        {
            SetBehaviorState(ENPC_BehaviorState::Idle);
            return;
        }
    }

    // Default: if idle and patrol points exist, start patrolling
    if (CurrentState == ENPC_BehaviorState::Idle && PatrolPoints.Num() > 0 && StateTimer > 5.0f)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrol);
    }
}

// ─────────────────────────────────────────────────────────────
// Memory System
// ─────────────────────────────────────────────────────────────
void UNPCBehaviorComponent::AddMemoryEntry(AActor* Source, FVector Location, float ThreatWeight)
{
    FNPC_MemoryEntry Entry;
    Entry.SourceActor = Source;
    Entry.Location = Location;
    Entry.ThreatWeight = FMath::Clamp(ThreatWeight, 0.0f, 1.0f);
    Entry.Timestamp = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    Entry.bIsActive = true;

    // Check for existing entry from same source — update instead of duplicate
    for (FNPC_MemoryEntry& Existing : MemoryEntries)
    {
        if (Existing.SourceActor == Source && Existing.bIsActive)
        {
            Existing.Location = Location;
            Existing.ThreatWeight = FMath::Max(Existing.ThreatWeight, ThreatWeight);
            Existing.Timestamp = Entry.Timestamp;
            return;
        }
    }

    MemoryEntries.Add(Entry);

    // Cap memory at 16 entries — remove oldest
    if (MemoryEntries.Num() > 16)
    {
        MemoryEntries.RemoveAt(0);
    }
}

void UNPCBehaviorComponent::DecayMemories(float DeltaTime)
{
    MemoryDecayAccumulator += DeltaTime;
    if (MemoryDecayAccumulator < 1.0f) return; // Decay once per second
    MemoryDecayAccumulator = 0.0f;

    const float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

    for (FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        if (!Entry.bIsActive) continue;

        const float Age = CurrentTime - Entry.Timestamp;
        if (Age > MaxMemoryAge)
        {
            Entry.bIsActive = false;
            Entry.ThreatWeight = 0.0f;
            continue;
        }

        // Linear decay of threat weight over memory lifetime
        const float DecayFactor = 1.0f - (Age / MaxMemoryAge);
        Entry.ThreatWeight *= DecayFactor;

        if (Entry.ThreatWeight < 0.01f)
        {
            Entry.bIsActive = false;
        }
    }

    // Remove inactive entries periodically
    MemoryEntries.RemoveAll([](const FNPC_MemoryEntry& E) { return !E.bIsActive; });
}

FNPC_MemoryEntry UNPCBehaviorComponent::GetMostRecentThreat() const
{
    FNPC_MemoryEntry Best;
    float BestWeight = -1.0f;

    for (const FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        if (Entry.bIsActive && Entry.ThreatWeight > BestWeight)
        {
            BestWeight = Entry.ThreatWeight;
            Best = Entry;
        }
    }

    return Best;
}

bool UNPCBehaviorComponent::HasActiveMemories() const
{
    for (const FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        if (Entry.bIsActive && Entry.ThreatWeight > 0.01f) return true;
    }
    return false;
}

// ─────────────────────────────────────────────────────────────
// Threat Assessment
// ─────────────────────────────────────────────────────────────
void UNPCBehaviorComponent::UpdateThreatLevel()
{
    const float Score = GetAggregatedThreatScore();

    if (Score <= 0.0f)
        CurrentThreatLevel = ENPC_ThreatLevel::None;
    else if (Score < 0.25f)
        CurrentThreatLevel = ENPC_ThreatLevel::Low;
    else if (Score < 0.5f)
        CurrentThreatLevel = ENPC_ThreatLevel::Medium;
    else if (Score < 0.75f)
        CurrentThreatLevel = ENPC_ThreatLevel::High;
    else
        CurrentThreatLevel = ENPC_ThreatLevel::Critical;
}

float UNPCBehaviorComponent::GetAggregatedThreatScore() const
{
    float Total = 0.0f;
    for (const FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        if (Entry.bIsActive)
        {
            Total += Entry.ThreatWeight;
        }
    }
    // Normalize: diminishing returns above 1.0
    return FMath::Clamp(Total, 0.0f, 1.0f);
}

// ─────────────────────────────────────────────────────────────
// Daily Routine
// ─────────────────────────────────────────────────────────────
void UNPCBehaviorComponent::AddRoutineSlot(FNPC_DailyRoutineSlot Slot)
{
    DailyRoutine.Add(Slot);
}

void UNPCBehaviorComponent::EvaluateDailyRoutine(ENPC_TimeOfDay CurrentTime)
{
    // Only apply routine if no active threat
    if (CurrentThreatLevel >= ENPC_ThreatLevel::Medium) return;
    if (CurrentState == ENPC_BehaviorState::Dead) return;

    for (const FNPC_DailyRoutineSlot& Slot : DailyRoutine)
    {
        if (Slot.TimeSlot == CurrentTime)
        {
            SetBehaviorState(Slot.TargetBehavior);

            // If routine specifies a target location, set as patrol point
            if (!Slot.TargetLocation.IsZero())
            {
                PatrolPoints.Empty();
                PatrolPoints.Add(Slot.TargetLocation);
                CurrentPatrolIndex = 0;
            }
            return;
        }
    }
}

// ─────────────────────────────────────────────────────────────
// Patrol
// ─────────────────────────────────────────────────────────────
void UNPCBehaviorComponent::SetPatrolPoints(const TArray<FVector>& Points)
{
    PatrolPoints = Points;
    CurrentPatrolIndex = 0;
}

FVector UNPCBehaviorComponent::GetNextPatrolPoint()
{
    if (PatrolPoints.Num() == 0) return FVector::ZeroVector;

    if (HasReachedCurrentPatrolPoint())
    {
        CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
    }

    return PatrolPoints[CurrentPatrolIndex];
}

bool UNPCBehaviorComponent::HasReachedCurrentPatrolPoint() const
{
    if (PatrolPoints.Num() == 0) return false;
    if (!GetOwner()) return false;

    const FVector OwnerLoc = GetOwner()->GetActorLocation();
    const FVector TargetLoc = PatrolPoints[CurrentPatrolIndex];
    const float DistSq = FVector::DistSquaredXY(OwnerLoc, TargetLoc);

    return DistSq <= (PatrolAcceptanceRadius * PatrolAcceptanceRadius);
}

// ─────────────────────────────────────────────────────────────
// Private helpers
// ─────────────────────────────────────────────────────────────
void UNPCBehaviorComponent::TickSurvivalNeeds(float DeltaTime)
{
    Hunger  = FMath::Clamp(Hunger  - SurvivalNeedDecayRate * DeltaTime, 0.0f, 1.0f);
    Thirst  = FMath::Clamp(Thirst  - SurvivalNeedDecayRate * DeltaTime, 0.0f, 1.0f);
    Fatigue = FMath::Clamp(Fatigue + SurvivalNeedDecayRate * 0.5f * DeltaTime, 0.0f, 1.0f);
    Fear    = FMath::Clamp(Fear    - SurvivalNeedDecayRate * 2.0f * DeltaTime, 0.0f, 1.0f);
}

void UNPCBehaviorComponent::TickStateTimer(float DeltaTime)
{
    StateTimer += DeltaTime;
}
