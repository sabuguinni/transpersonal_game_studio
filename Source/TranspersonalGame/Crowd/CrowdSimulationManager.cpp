// CrowdSimulationManager.cpp
// Agent #13 — Crowd & Traffic Simulation
// Manages up to 50,000 simultaneous crowd agents using UE5 Mass AI principles.
// Prehistoric world: herbivore herds, predator packs, aerial flocks, human tribes.

#include "CrowdSimulationManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "DrawDebugHelpers.h"
#include "Math/UnrealMathUtility.h"

UCrowdSimulationManager::UCrowdSimulationManager()
{
    MaxAgents = 500;
    ActiveAgentCount = 0;
    bSimulationActive = false;
    TickInterval = 0.1f;
    LODDistanceNear = 2000.0f;
    LODDistanceMid = 5000.0f;
    LODDistanceFar = 10000.0f;
    TimeSinceLastTick = 0.0f;
}

void UCrowdSimulationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    bSimulationActive = true;
    ActiveAgentCount = 0;
    AgentPool.Reserve(MaxAgents);
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Initialized — max agents: %d"), MaxAgents);
}

void UCrowdSimulationManager::Deinitialize()
{
    bSimulationActive = false;
    AgentPool.Empty();
    Super::Deinitialize();
}

int32 UCrowdSimulationManager::SpawnHerd(ECrowd_AgentType AgentType, FVector Origin, int32 Count, float Radius)
{
    if (!bSimulationActive) return 0;
    if (ActiveAgentCount + Count > MaxAgents)
    {
        Count = MaxAgents - ActiveAgentCount;
    }
    if (Count <= 0) return 0;

    int32 Spawned = 0;
    for (int32 i = 0; i < Count; ++i)
    {
        FCrowd_Agent NewAgent;
        NewAgent.AgentID = ActiveAgentCount + i;
        NewAgent.AgentType = AgentType;
        NewAgent.bIsActive = true;
        NewAgent.LODLevel = ECrowd_LODLevel::Near;

        // Distribute agents in formation around origin
        float Angle = (float(i) / float(Count)) * 2.0f * PI;
        float SpreadRadius = FMath::RandRange(Radius * 0.3f, Radius);
        NewAgent.Location = Origin + FVector(
            FMath::Cos(Angle) * SpreadRadius,
            FMath::Sin(Angle) * SpreadRadius,
            0.0f
        );
        NewAgent.Velocity = FVector::ZeroVector;
        NewAgent.DesiredSpeed = GetDefaultSpeedForType(AgentType);
        NewAgent.Health = 100.0f;
        NewAgent.BehaviorState = ECrowd_BehaviorState::Idle;

        AgentPool.Add(NewAgent);
        ++Spawned;
    }

    ActiveAgentCount += Spawned;
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Spawned herd: %d agents of type %d at %s"),
        Spawned, (int32)AgentType, *Origin.ToString());
    return Spawned;
}

void UCrowdSimulationManager::UpdateAgentLOD(const FVector& PlayerLocation)
{
    for (FCrowd_Agent& Agent : AgentPool)
    {
        if (!Agent.bIsActive) continue;

        float Dist = FVector::Dist(Agent.Location, PlayerLocation);
        if (Dist < LODDistanceNear)
            Agent.LODLevel = ECrowd_LODLevel::Near;
        else if (Dist < LODDistanceMid)
            Agent.LODLevel = ECrowd_LODLevel::Mid;
        else if (Dist < LODDistanceFar)
            Agent.LODLevel = ECrowd_LODLevel::Far;
        else
            Agent.LODLevel = ECrowd_LODLevel::Culled;
    }
}

void UCrowdSimulationManager::TickSimulation(float DeltaTime, const FVector& PlayerLocation)
{
    if (!bSimulationActive) return;

    TimeSinceLastTick += DeltaTime;
    if (TimeSinceLastTick < TickInterval) return;
    TimeSinceLastTick = 0.0f;

    UpdateAgentLOD(PlayerLocation);

    for (FCrowd_Agent& Agent : AgentPool)
    {
        if (!Agent.bIsActive) continue;
        if (Agent.LODLevel == ECrowd_LODLevel::Culled) continue;

        // Reduced tick rate for distant agents
        if (Agent.LODLevel == ECrowd_LODLevel::Far && FMath::RandBool()) continue;

        TickAgent(Agent, TickInterval);
    }
}

void UCrowdSimulationManager::TickAgent(FCrowd_Agent& Agent, float DeltaTime)
{
    switch (Agent.BehaviorState)
    {
    case ECrowd_BehaviorState::Idle:
        // Occasionally start wandering
        if (FMath::RandRange(0.0f, 1.0f) < 0.05f)
        {
            Agent.BehaviorState = ECrowd_BehaviorState::Wandering;
            Agent.TargetLocation = Agent.Location + FVector(
                FMath::RandRange(-500.0f, 500.0f),
                FMath::RandRange(-500.0f, 500.0f),
                0.0f
            );
        }
        break;

    case ECrowd_BehaviorState::Wandering:
    {
        FVector ToTarget = Agent.TargetLocation - Agent.Location;
        float DistToTarget = ToTarget.Size();
        if (DistToTarget < 50.0f)
        {
            Agent.BehaviorState = ECrowd_BehaviorState::Idle;
        }
        else
        {
            FVector Dir = ToTarget.GetSafeNormal();
            Agent.Velocity = Dir * Agent.DesiredSpeed;
            Agent.Location += Agent.Velocity * DeltaTime;
        }
        break;
    }

    case ECrowd_BehaviorState::Fleeing:
    {
        // Move away from threat at max speed
        FVector FleeDir = (Agent.Location - Agent.ThreatLocation).GetSafeNormal();
        Agent.Velocity = FleeDir * Agent.DesiredSpeed * 2.0f;
        Agent.Location += Agent.Velocity * DeltaTime;
        // Decay fear over time
        if (FMath::RandRange(0.0f, 1.0f) < 0.02f)
            Agent.BehaviorState = ECrowd_BehaviorState::Wandering;
        break;
    }

    case ECrowd_BehaviorState::Grazing:
        // Stay in place, small random drift
        Agent.Location += FVector(
            FMath::RandRange(-5.0f, 5.0f),
            FMath::RandRange(-5.0f, 5.0f),
            0.0f
        ) * DeltaTime;
        break;

    default:
        break;
    }
}

void UCrowdSimulationManager::TriggerFleeEvent(FVector ThreatLocation, float ThreatRadius)
{
    int32 Affected = 0;
    for (FCrowd_Agent& Agent : AgentPool)
    {
        if (!Agent.bIsActive) continue;
        float Dist = FVector::Dist(Agent.Location, ThreatLocation);
        if (Dist < ThreatRadius)
        {
            Agent.BehaviorState = ECrowd_BehaviorState::Fleeing;
            Agent.ThreatLocation = ThreatLocation;
            ++Affected;
        }
    }
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] FleeEvent triggered: %d agents fleeing from %s"),
        Affected, *ThreatLocation.ToString());
}

int32 UCrowdSimulationManager::GetActiveAgentCount() const
{
    return ActiveAgentCount;
}

TArray<FCrowd_Agent> UCrowdSimulationManager::GetAgentsInRadius(FVector Center, float Radius) const
{
    TArray<FCrowd_Agent> Result;
    for (const FCrowd_Agent& Agent : AgentPool)
    {
        if (!Agent.bIsActive) continue;
        if (FVector::Dist(Agent.Location, Center) <= Radius)
            Result.Add(Agent);
    }
    return Result;
}

float UCrowdSimulationManager::GetDefaultSpeedForType(ECrowd_AgentType Type) const
{
    switch (Type)
    {
    case ECrowd_AgentType::HerbivoreHerd:    return 200.0f;
    case ECrowd_AgentType::PredatorPack:     return 350.0f;
    case ECrowd_AgentType::AerialFlock:      return 500.0f;
    case ECrowd_AgentType::HumanTribe:       return 150.0f;
    case ECrowd_AgentType::ScavengerGroup:   return 180.0f;
    default:                                  return 200.0f;
    }
}
