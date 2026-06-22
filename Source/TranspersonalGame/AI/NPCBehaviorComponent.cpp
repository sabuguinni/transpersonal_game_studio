#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz tick for performance
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    if (AActor* Owner = GetOwner())
    {
        PatrolOrigin = Owner->GetActorLocation();
    }

    CurrentState = ENPC_BehaviorState::Patrol;
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateMemoryDecay(DeltaTime);

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
}

void UNPCBehaviorComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (CurrentState == NewState) return;

    CurrentState = NewState;

    if (NewState == ENPC_BehaviorState::Alert)
    {
        AlertTimer = AlertDuration;
    }
}

void UNPCBehaviorComponent::RegisterThreat(AActor* ThreatActor, FVector LastKnownLoc)
{
    if (!ThreatActor) return;

    FNPC_MemoryEntry Entry;
    Entry.LastKnownLocation = LastKnownLoc;
    Entry.TimeSinceLastSeen = 0.0f;
    Entry.bIsThreat = true;
    Entry.ThreatActorName = ThreatActor->GetName();

    // Update existing entry if present
    for (FNPC_MemoryEntry& Existing : MemoryEntries)
    {
        if (Existing.ThreatActorName == Entry.ThreatActorName)
        {
            Existing = Entry;
            SetBehaviorState(ENPC_BehaviorState::Alert);
            return;
        }
    }

    MemoryEntries.Add(Entry);
    SetBehaviorState(ENPC_BehaviorState::Alert);
}

void UNPCBehaviorComponent::ClearMemory()
{
    MemoryEntries.Empty();
    SetBehaviorState(ENPC_BehaviorState::Patrol);
}

bool UNPCBehaviorComponent::HasActiveThreat() const
{
    for (const FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        if (Entry.bIsThreat && Entry.TimeSinceLastSeen < AlertDuration)
        {
            return true;
        }
    }
    return false;
}

void UNPCBehaviorComponent::TickIdle(float DeltaTime)
{
    // After 3 seconds of idle, resume patrol
    static float IdleAccum = 0.0f;
    IdleAccum += DeltaTime;
    if (IdleAccum >= 3.0f)
    {
        IdleAccum = 0.0f;
        SetBehaviorState(ENPC_BehaviorState::Patrol);
    }
}

void UNPCBehaviorComponent::TickPatrol(float DeltaTime)
{
    // Patrol logic is handled by the AI Controller / Behavior Tree
    // This component provides state and memory — movement is delegated
    // If a threat is detected in memory, escalate to Alert
    if (HasActiveThreat())
    {
        SetBehaviorState(ENPC_BehaviorState::Alert);
    }
}

void UNPCBehaviorComponent::TickAlert(float DeltaTime)
{
    AlertTimer -= DeltaTime;

    if (AlertTimer <= 0.0f)
    {
        // Threat window expired — clear threats and return to patrol
        for (FNPC_MemoryEntry& Entry : MemoryEntries)
        {
            Entry.bIsThreat = false;
        }
        SetBehaviorState(ENPC_BehaviorState::Patrol);
    }
}

void UNPCBehaviorComponent::TickFlee(float DeltaTime)
{
    // Flee logic: movement handled by AI Controller
    // After fleeing far enough from PatrolOrigin, transition back to Alert
    AActor* Owner = GetOwner();
    if (!Owner) return;

    float DistFromOrigin = FVector::Dist(Owner->GetActorLocation(), PatrolOrigin);
    if (DistFromOrigin >= FleeRange)
    {
        SetBehaviorState(ENPC_BehaviorState::Alert);
    }
}

void UNPCBehaviorComponent::UpdateMemoryDecay(float DeltaTime)
{
    for (FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        Entry.TimeSinceLastSeen += DeltaTime;
    }

    // Remove very old memories (>60 seconds)
    MemoryEntries.RemoveAll([](const FNPC_MemoryEntry& E) {
        return E.TimeSinceLastSeen > 60.0f;
    });
}
