// CrowdSimulationManager.cpp
// Agent #13 — Crowd & Traffic Simulation
// Prehistoric survival game — dinosaur herd and predator crowd AI

#include "CrowdSimulationManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "DrawDebugHelpers.h"

UCrowdSimulationManager::UCrowdSimulationManager()
{
    MaxHerbivoreAgents = 200;
    MaxPredatorAgents = 20;
    HerdCohesionRadius = 500.0f;
    PredatorDetectionRadius = 800.0f;
    bCrowdSystemActive = true;
}

void UCrowdSimulationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Initialized — max herbivores=%d, max predators=%d"),
        MaxHerbivoreAgents, MaxPredatorAgents);
}

void UCrowdSimulationManager::Deinitialize()
{
    HerdZones.Empty();
    PatrolNodes.Empty();
    Super::Deinitialize();
}

void UCrowdSimulationManager::RegisterHerdZone(const FCrowd_HerdZone& Zone)
{
    HerdZones.Add(Zone);
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Registered herd zone '%s' at (%.0f, %.0f, %.0f)"),
        *Zone.ZoneName.ToString(), Zone.Location.X, Zone.Location.Y, Zone.Location.Z);
}

void UCrowdSimulationManager::RegisterPatrolNode(const FCrowd_PatrolNode& Node)
{
    PatrolNodes.Add(Node);
}

FCrowd_HerdZone UCrowdSimulationManager::GetNearestHerdZone(const FVector& WorldLocation) const
{
    FCrowd_HerdZone Nearest;
    float BestDist = MAX_FLT;

    for (const FCrowd_HerdZone& Zone : HerdZones)
    {
        float Dist = FVector::Dist(WorldLocation, Zone.Location);
        if (Dist < BestDist)
        {
            BestDist = Dist;
            Nearest = Zone;
        }
    }
    return Nearest;
}

TArray<FCrowd_PatrolNode> UCrowdSimulationManager::GetPatrolRouteForSpecies(ECrowd_DinoSpecies Species) const
{
    TArray<FCrowd_PatrolNode> Route;
    for (const FCrowd_PatrolNode& Node : PatrolNodes)
    {
        if (Node.OwnerSpecies == Species)
        {
            Route.Add(Node);
        }
    }
    return Route;
}

int32 UCrowdSimulationManager::GetActiveAgentCount(ECrowd_AgentType AgentType) const
{
    int32 Count = 0;
    for (const FCrowd_AgentState& State : ActiveAgents)
    {
        if (State.AgentType == AgentType)
        {
            Count++;
        }
    }
    return Count;
}

void UCrowdSimulationManager::UpdateCrowdTick(float DeltaTime)
{
    if (!bCrowdSystemActive) return;

    // Update each active agent state
    for (FCrowd_AgentState& Agent : ActiveAgents)
    {
        UpdateAgentBehavior(Agent, DeltaTime);
    }
}

void UCrowdSimulationManager::UpdateAgentBehavior(FCrowd_AgentState& Agent, float DeltaTime)
{
    switch (Agent.CurrentBehavior)
    {
    case ECrowd_AgentBehavior::Grazing:
        // Herbivores slowly drift within herd zone
        Agent.Velocity = FVector::ZeroVector;
        Agent.BehaviorTimer += DeltaTime;
        if (Agent.BehaviorTimer > 5.0f)
        {
            Agent.CurrentBehavior = ECrowd_AgentBehavior::Wandering;
            Agent.BehaviorTimer = 0.0f;
        }
        break;

    case ECrowd_AgentBehavior::Wandering:
        Agent.BehaviorTimer += DeltaTime;
        if (Agent.BehaviorTimer > 8.0f)
        {
            Agent.CurrentBehavior = ECrowd_AgentBehavior::Grazing;
            Agent.BehaviorTimer = 0.0f;
        }
        break;

    case ECrowd_AgentBehavior::Fleeing:
        // Move away from threat at high speed
        Agent.Velocity = Agent.FleeDirection * 600.0f;
        Agent.Location += Agent.Velocity * DeltaTime;
        Agent.BehaviorTimer += DeltaTime;
        if (Agent.BehaviorTimer > 10.0f)
        {
            Agent.CurrentBehavior = ECrowd_AgentBehavior::Grazing;
            Agent.BehaviorTimer = 0.0f;
            Agent.FleeDirection = FVector::ZeroVector;
        }
        break;

    case ECrowd_AgentBehavior::Hunting:
        // Predator moves toward prey
        if (!Agent.TargetLocation.IsZero())
        {
            FVector Dir = (Agent.TargetLocation - Agent.Location).GetSafeNormal();
            Agent.Velocity = Dir * 500.0f;
            Agent.Location += Agent.Velocity * DeltaTime;
        }
        break;

    case ECrowd_AgentBehavior::Patrolling:
        // Move toward next patrol node
        Agent.BehaviorTimer += DeltaTime;
        break;

    default:
        break;
    }
}

void UCrowdSimulationManager::TriggerFleeResponse(const FVector& ThreatLocation, float ThreatRadius)
{
    for (FCrowd_AgentState& Agent : ActiveAgents)
    {
        if (Agent.AgentType != ECrowd_AgentType::Herbivore) continue;

        float Dist = FVector::Dist(Agent.Location, ThreatLocation);
        if (Dist < ThreatRadius)
        {
            Agent.CurrentBehavior = ECrowd_AgentBehavior::Fleeing;
            Agent.FleeDirection = (Agent.Location - ThreatLocation).GetSafeNormal();
            Agent.BehaviorTimer = 0.0f;
        }
    }
}

void UCrowdSimulationManager::SpawnHerdAtZone(const FCrowd_HerdZone& Zone, int32 Count)
{
    int32 CurrentHerbivores = GetActiveAgentCount(ECrowd_AgentType::Herbivore);
    int32 ToSpawn = FMath::Min(Count, MaxHerbivoreAgents - CurrentHerbivores);

    for (int32 i = 0; i < ToSpawn; i++)
    {
        FCrowd_AgentState NewAgent;
        NewAgent.AgentType = ECrowd_AgentType::Herbivore;
        NewAgent.CurrentBehavior = ECrowd_AgentBehavior::Grazing;
        NewAgent.Location = Zone.Location + FVector(
            FMath::RandRange(-Zone.Radius, Zone.Radius),
            FMath::RandRange(-Zone.Radius, Zone.Radius),
            0.0f
        );
        NewAgent.BehaviorTimer = FMath::RandRange(0.0f, 5.0f);
        ActiveAgents.Add(NewAgent);
    }

    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Spawned %d herbivores at zone '%s'"),
        ToSpawn, *Zone.ZoneName.ToString());
}
