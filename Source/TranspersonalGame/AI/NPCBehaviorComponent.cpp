// NPCBehaviorComponent.cpp
// NPC Behavior Agent #11 — Transpersonal Game Studio
// Full implementation of prehistoric NPC behavior state machine

#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 Hz update — performance-friendly
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    // Record patrol origin at spawn location
    if (GetOwner())
    {
        PatrolOrigin = GetOwner()->GetActorLocation();
    }

    // Start in Idle
    CurrentState = ENPC_BehaviorState::Idle;
    StateTimer = 0.0f;
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    StateTimer += DeltaTime;
    DecayMemory(DeltaTime);

    switch (CurrentState)
    {
        case ENPC_BehaviorState::Idle:
            UpdateIdleBehavior(DeltaTime);
            break;
        case ENPC_BehaviorState::Patrol:
            UpdatePatrolBehavior(DeltaTime);
            break;
        case ENPC_BehaviorState::Flee:
            UpdateFleeBehavior(DeltaTime);
            break;
        default:
            break;
    }
}

void UNPCBehaviorComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (CurrentState == NewState) return;

    CurrentState = NewState;
    StateTimer = 0.0f;

    UE_LOG(LogTemp, Verbose, TEXT("NPC [%s] state -> %d"),
        GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"),
        (int32)NewState);
}

void UNPCBehaviorComponent::UpdateThreatLevel(ENPC_ThreatLevel NewThreat, FVector ThreatLocation)
{
    CurrentThreatLevel = NewThreat;
    Memory.LastKnownThreatLocation = ThreatLocation;
    Memory.TimeSinceLastThreat = 0.0f;

    // Escalate fear
    switch (NewThreat)
    {
        case ENPC_ThreatLevel::None:     Memory.FearLevel = FMath::Max(0.0f, Memory.FearLevel - 0.2f); break;
        case ENPC_ThreatLevel::Low:      Memory.FearLevel = FMath::Min(1.0f, Memory.FearLevel + 0.1f); break;
        case ENPC_ThreatLevel::Medium:   Memory.FearLevel = FMath::Min(1.0f, Memory.FearLevel + 0.3f); break;
        case ENPC_ThreatLevel::High:     Memory.FearLevel = FMath::Min(1.0f, Memory.FearLevel + 0.5f); break;
        case ENPC_ThreatLevel::Critical: Memory.FearLevel = 1.0f; break;
    }

    // Trigger flee if fear is high enough
    if (Memory.FearLevel >= 0.6f && CurrentState != ENPC_BehaviorState::Dead)
    {
        SetBehaviorState(ENPC_BehaviorState::Flee);
    }
    else if (NewThreat == ENPC_ThreatLevel::None && CurrentState == ENPC_BehaviorState::Flee)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrol);
    }
}

void UNPCBehaviorComponent::OnTimeOfDayChanged(ENPC_TimeOfDay NewTimeOfDay)
{
    // Adjust behavior based on time of day
    switch (NewTimeOfDay)
    {
        case ENPC_TimeOfDay::Dawn:
        case ENPC_TimeOfDay::Morning:
            // Active gathering time
            if (CurrentState == ENPC_BehaviorState::Idle)
                SetBehaviorState(ENPC_BehaviorState::Gather);
            break;

        case ENPC_TimeOfDay::Midday:
            // Rest during hottest part of day
            if (CurrentState == ENPC_BehaviorState::Patrol)
                SetBehaviorState(ENPC_BehaviorState::Idle);
            break;

        case ENPC_TimeOfDay::Dusk:
            // Return to camp / patrol perimeter
            SetBehaviorState(ENPC_BehaviorState::Patrol);
            break;

        case ENPC_TimeOfDay::Night:
            // Shelter — high threat awareness
            SetBehaviorState(ENPC_BehaviorState::Idle);
            break;
    }
}

void UNPCBehaviorComponent::UpdateIdleBehavior(float DeltaTime)
{
    // After 5-10 seconds idle, transition to patrol
    if (StateTimer > FMath::RandRange(5.0f, 10.0f))
    {
        SetBehaviorState(ENPC_BehaviorState::Patrol);
    }
}

void UNPCBehaviorComponent::UpdatePatrolBehavior(float DeltaTime)
{
    // Patrol for 20-40 seconds then rest
    if (StateTimer > FMath::RandRange(20.0f, 40.0f))
    {
        SetBehaviorState(ENPC_BehaviorState::Idle);
    }
}

void UNPCBehaviorComponent::UpdateFleeBehavior(float DeltaTime)
{
    // Flee for 8 seconds then investigate
    if (StateTimer > 8.0f && Memory.FearLevel < 0.4f)
    {
        SetBehaviorState(ENPC_BehaviorState::Investigate);
    }
}

void UNPCBehaviorComponent::DecayMemory(float DeltaTime)
{
    Memory.TimeSinceLastThreat += DeltaTime;
    Memory.FearLevel = FMath::Max(0.0f, Memory.FearLevel - MemoryDecayRate * DeltaTime);

    // Clear threat memory after 30 seconds
    if (Memory.TimeSinceLastThreat > 30.0f)
    {
        CurrentThreatLevel = ENPC_ThreatLevel::None;
        Memory.bHasSeenPlayer = false;
    }
}
