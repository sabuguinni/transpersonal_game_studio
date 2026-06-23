#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

    NPCRole = ENPC_Role::Scout;
    PatrolRadius = 1200.0f;
    AlertRadius = 2500.0f;
    FleeThreshold = 0.75f;
    CurrentState = ENPC_BehaviorState::Idle;
    StateTimer = 0.0f;
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();
    CurrentState = ENPC_BehaviorState::Patrol;
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    StateTimer += DeltaTime;

    switch (CurrentState)
    {
        case ENPC_BehaviorState::Idle:
            TickIdle(DeltaTime);
            break;
        case ENPC_BehaviorState::Patrol:
            TickPatrol(DeltaTime);
            break;
        case ENPC_BehaviorState::Alert:
            TickAlert(DeltaTime);
            break;
        case ENPC_BehaviorState::Flee:
            TickFlee(DeltaTime);
            break;
        default:
            break;
    }

    // Decay threat over time
    if (Memory.ThreatLevel > 0.0f)
    {
        Memory.ThreatLevel = FMath::Max(0.0f, Memory.ThreatLevel - DeltaTime * 0.05f);
    }
}

void UNPCBehaviorComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (CurrentState != NewState)
    {
        CurrentState = NewState;
        StateTimer = 0.0f;
    }
}

void UNPCBehaviorComponent::RegisterThreat(FVector ThreatLocation, float Intensity)
{
    Memory.LastKnownThreatLocation = ThreatLocation;
    Memory.ThreatLevel = FMath::Clamp(Memory.ThreatLevel + Intensity, 0.0f, 1.0f);
    Memory.TimeLastSeen = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

    if (Memory.ThreatLevel >= FleeThreshold)
    {
        SetBehaviorState(ENPC_BehaviorState::Flee);
    }
    else if (Memory.ThreatLevel > 0.2f)
    {
        SetBehaviorState(ENPC_BehaviorState::Alert);
    }
}

void UNPCBehaviorComponent::ClearMemory()
{
    Memory.ThreatLevel = 0.0f;
    Memory.LastKnownThreatLocation = FVector::ZeroVector;
    Memory.TimeLastSeen = 0.0f;
    SetBehaviorState(ENPC_BehaviorState::Patrol);
}

void UNPCBehaviorComponent::TickIdle(float DeltaTime)
{
    // After 5 seconds idle, resume patrol
    if (StateTimer > 5.0f)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrol);
    }
}

void UNPCBehaviorComponent::TickPatrol(float DeltaTime)
{
    // Watcher role: stay at post, scan
    // Scout role: move along patrol radius
    // Elder role: remain near camp center
    // Actual movement handled by AIController / BT
    // This component tracks state and memory only
}

void UNPCBehaviorComponent::TickAlert(float DeltaTime)
{
    // Alert state: face threat direction, warn nearby NPCs
    // After 10 seconds without new threat, return to patrol
    if (Memory.ThreatLevel < 0.1f && StateTimer > 10.0f)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrol);
    }
}

void UNPCBehaviorComponent::TickFlee(float DeltaTime)
{
    // Flee state: move away from LastKnownThreatLocation
    // After 20 seconds, check if threat is gone
    if (Memory.ThreatLevel < 0.2f && StateTimer > 20.0f)
    {
        SetBehaviorState(ENPC_BehaviorState::Alert);
    }
}
