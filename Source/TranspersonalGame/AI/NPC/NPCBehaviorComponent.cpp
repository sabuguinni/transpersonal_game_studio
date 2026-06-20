#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "Math/UnrealMathUtility.h"

// ============================================================
// NPCBehaviorComponent — Agent #11 NPC Behavior
// Full implementation of state machine, memory, patrol, threat
// ============================================================

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz tick — sufficient for AI
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();
    NPC_CurrentState = ENPC_BehaviorState::Patrol;
    NPC_PreviousState = ENPC_BehaviorState::Idle;
    NPC_StateTimer = 0.0f;
    NPC_Hunger = 1.0f;
    NPC_Health = 1.0f;
    NPC_bHasPlayerInMemory = false;
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateStateTimer(DeltaTime);
    UpdateMemoryDecay(DeltaTime);
    UpdateHunger(DeltaTime);

    // Waypoint wait timer
    if (NPC_bIsWaitingAtWaypoint)
    {
        NPC_WaypointWaitTimer -= DeltaTime;
        if (NPC_WaypointWaitTimer <= 0.0f)
        {
            NPC_bIsWaitingAtWaypoint = false;
            NPC_WaypointWaitTimer = 0.0f;
        }
    }
}

void UNPCBehaviorComponent::NPC_SetState(ENPC_BehaviorState NewState)
{
    if (NPC_CurrentState == NewState) return;
    NPC_PreviousState = NPC_CurrentState;
    NPC_CurrentState = NewState;
    NPC_StateTimer = 0.0f;
}

void UNPCBehaviorComponent::NPC_UpdatePlayerMemory(FVector PlayerLocation, ENPC_ThreatLevel Threat, bool bHostile)
{
    NPC_PlayerMemory.LastKnownLocation = PlayerLocation;
    NPC_PlayerMemory.TimeSinceLastSeen = 0.0f;
    NPC_PlayerMemory.PerceivedThreat = Threat;
    NPC_PlayerMemory.bIsHostile = bHostile;
    NPC_PlayerMemory.EncounterCount++;
    NPC_bHasPlayerInMemory = true;
}

void UNPCBehaviorComponent::NPC_ClearPlayerMemory()
{
    NPC_PlayerMemory = FNPC_MemoryEntry();
    NPC_bHasPlayerInMemory = false;
}

FVector UNPCBehaviorComponent::NPC_GetNextPatrolWaypoint()
{
    if (NPC_PatrolData.WaypointLocations.Num() == 0)
    {
        return GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
    }
    int32 Idx = FMath::Clamp(NPC_PatrolData.CurrentWaypointIndex, 0, NPC_PatrolData.WaypointLocations.Num() - 1);
    return NPC_PatrolData.WaypointLocations[Idx];
}

void UNPCBehaviorComponent::NPC_AdvancePatrolWaypoint()
{
    if (NPC_PatrolData.WaypointLocations.Num() == 0) return;

    NPC_PatrolData.CurrentWaypointIndex++;
    if (NPC_PatrolData.CurrentWaypointIndex >= NPC_PatrolData.WaypointLocations.Num())
    {
        NPC_PatrolData.CurrentWaypointIndex = NPC_PatrolData.bLoopPatrol ? 0 : NPC_PatrolData.WaypointLocations.Num() - 1;
    }

    // Start wait at waypoint
    NPC_bIsWaitingAtWaypoint = true;
    NPC_WaypointWaitTimer = NPC_PatrolData.WaitTimeAtWaypoint;
}

ENPC_ThreatLevel UNPCBehaviorComponent::NPC_EvaluateThreat(float DistanceToPlayer, float PlayerHealthNormalized, bool bPlayerIsExhausted) const
{
    // Predators evaluate player as prey based on distance, health, exhaustion
    if (!NPC_bIsPredator) return ENPC_ThreatLevel::None;

    // Very close = critical threat / attack opportunity
    if (DistanceToPlayer < NPC_AttackRadius)
    {
        return ENPC_ThreatLevel::Critical;
    }

    // Within chase range — assess player vulnerability
    if (DistanceToPlayer < NPC_ChaseRadius)
    {
        float VulnerabilityScore = 0.0f;
        VulnerabilityScore += (1.0f - PlayerHealthNormalized) * 0.5f; // Low health = more vulnerable
        VulnerabilityScore += bPlayerIsExhausted ? 0.3f : 0.0f;       // Exhaustion = easier prey
        VulnerabilityScore += NPC_IsHungry() ? 0.2f : 0.0f;           // Hunger drives aggression
        VulnerabilityScore *= NPC_AggressionLevel;

        if (VulnerabilityScore > 0.7f) return ENPC_ThreatLevel::High;
        if (VulnerabilityScore > 0.4f) return ENPC_ThreatLevel::Medium;
        return ENPC_ThreatLevel::Low;
    }

    return ENPC_ThreatLevel::None;
}

void UNPCBehaviorComponent::UpdateMemoryDecay(float DeltaTime)
{
    if (!NPC_bHasPlayerInMemory) return;

    NPC_PlayerMemory.TimeSinceLastSeen += DeltaTime;

    // Memory fades after NPC_MemoryDecayTime seconds
    if (NPC_PlayerMemory.TimeSinceLastSeen >= NPC_MemoryDecayTime)
    {
        // Reduce threat level over time — NPC forgets
        if (NPC_PlayerMemory.PerceivedThreat > ENPC_ThreatLevel::None)
        {
            uint8 ThreatVal = static_cast<uint8>(NPC_PlayerMemory.PerceivedThreat);
            NPC_PlayerMemory.PerceivedThreat = static_cast<ENPC_ThreatLevel>(ThreatVal - 1);
        }
        else
        {
            NPC_ClearPlayerMemory();
        }
        NPC_PlayerMemory.TimeSinceLastSeen = 0.0f;
    }
}

void UNPCBehaviorComponent::UpdateHunger(float DeltaTime)
{
    NPC_Hunger = FMath::Clamp(NPC_Hunger - (NPC_HungerDecayRate * DeltaTime), 0.0f, 1.0f);
}

void UNPCBehaviorComponent::UpdateStateTimer(float DeltaTime)
{
    NPC_StateTimer += DeltaTime;
}
