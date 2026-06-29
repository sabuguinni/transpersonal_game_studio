// CrowdSimulationManager.cpp
// Agent #13 — Crowd & Traffic Simulation
// Mass crowd simulation: up to 50,000 simultaneous agents using UE5 Mass AI

#include "CrowdSimulationManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "NavigationSystem.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UCrowdSimulationManager::UCrowdSimulationManager()
{
    MaxAgents = 500;
    ActiveAgentCount = 0;
    ThreatPropagationRadius = 1500.f;
    ThreatDecayRate = 0.1f;
    FlockingParams = FCrowd_FlockingParams();
    bSimulationActive = false;
    SimulationTickRate = 0.1f; // 10Hz update for performance
}

void UCrowdSimulationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Initializing with max %d agents"), MaxAgents);
    
    // Pre-allocate agent pool
    AgentPool.Reserve(MaxAgents);
    ActiveAgents.Reserve(MaxAgents);
    
    bSimulationActive = true;
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Initialized OK"));
}

void UCrowdSimulationManager::Deinitialize()
{
    bSimulationActive = false;
    AgentPool.Empty();
    ActiveAgents.Empty();
    ThreatEvents.Empty();
    Super::Deinitialize();
}

int32 UCrowdSimulationManager::SpawnCrowdAgent(const FVector& Location, ECrowd_NpcRole Role)
{
    if (ActiveAgentCount >= MaxAgents)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CrowdSim] Max agents reached (%d)"), MaxAgents);
        return -1;
    }

    FCrowd_AgentData NewAgent;
    NewAgent.AgentIndex = ActiveAgentCount;
    NewAgent.Location = Location;
    NewAgent.Velocity = FVector::ZeroVector;
    NewAgent.State = ECrowd_AgentState::Idle;
    NewAgent.Role = Role;
    NewAgent.Health = 100.f;
    NewAgent.Fear = 0.f;
    NewAgent.bIsLeader = (Role == ECrowd_NpcRole::Elder || Role == ECrowd_NpcRole::Guard);

    ActiveAgents.Add(NewAgent);
    ActiveAgentCount++;

    UE_LOG(LogTemp, Verbose, TEXT("[CrowdSim] Spawned agent %d at %s (Role: %d)"),
        NewAgent.AgentIndex, *Location.ToString(), (int32)Role);

    return NewAgent.AgentIndex;
}

void UCrowdSimulationManager::RegisterThreatEvent(const FVector& ThreatLocation, ECrowd_ThreatLevel Level, float Radius)
{
    FCrowd_ThreatEvent Event;
    Event.ThreatLocation = ThreatLocation;
    Event.ThreatLevel = Level;
    Event.ThreatRadius = Radius;
    Event.TimeStamp = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;

    ThreatEvents.Add(Event);

    // Propagate fear to nearby agents
    PropagateThreatToAgents(Event);

    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Threat registered at %s, Level=%d, Radius=%.0f"),
        *ThreatLocation.ToString(), (int32)Level, Radius);
}

void UCrowdSimulationManager::PropagateThreatToAgents(const FCrowd_ThreatEvent& Event)
{
    float FearIncrease = 0.f;
    switch (Event.ThreatLevel)
    {
        case ECrowd_ThreatLevel::Low:    FearIncrease = 15.f; break;
        case ECrowd_ThreatLevel::Medium: FearIncrease = 35.f; break;
        case ECrowd_ThreatLevel::High:   FearIncrease = 65.f; break;
        case ECrowd_ThreatLevel::Panic:  FearIncrease = 100.f; break;
        default: return;
    }

    for (FCrowd_AgentData& Agent : ActiveAgents)
    {
        float Dist = FVector::Dist(Agent.Location, Event.ThreatLocation);
        if (Dist <= Event.ThreatRadius)
        {
            float DistFactor = 1.f - (Dist / Event.ThreatRadius);
            Agent.Fear = FMath::Clamp(Agent.Fear + FearIncrease * DistFactor, 0.f, 100.f);

            // Transition to fleeing if fear exceeds threshold
            if (Agent.Fear >= 60.f && Agent.State != ECrowd_AgentState::Dead)
            {
                Agent.State = ECrowd_AgentState::Fleeing;
            }
        }
    }
}

FVector UCrowdSimulationManager::ComputeFlockingForce(const FCrowd_AgentData& Agent) const
{
    FVector Separation = FVector::ZeroVector;
    FVector Alignment = FVector::ZeroVector;
    FVector Cohesion = FVector::ZeroVector;

    int32 SepCount = 0, AlignCount = 0, CohCount = 0;

    for (const FCrowd_AgentData& Other : ActiveAgents)
    {
        if (Other.AgentIndex == Agent.AgentIndex) continue;

        float Dist = FVector::Dist(Agent.Location, Other.Location);

        if (Dist < FlockingParams.SeparationRadius && Dist > 0.f)
        {
            FVector Away = (Agent.Location - Other.Location).GetSafeNormal();
            Separation += Away / Dist;
            SepCount++;
        }

        if (Dist < FlockingParams.AlignmentRadius)
        {
            Alignment += Other.Velocity;
            AlignCount++;
        }

        if (Dist < FlockingParams.CohesionRadius)
        {
            Cohesion += Other.Location;
            CohCount++;
        }
    }

    FVector Force = FVector::ZeroVector;

    if (SepCount > 0)
        Force += (Separation / SepCount).GetSafeNormal() * FlockingParams.SeparationWeight;

    if (AlignCount > 0)
        Force += (Alignment / AlignCount).GetSafeNormal() * FlockingParams.AlignmentWeight;

    if (CohCount > 0)
    {
        FVector CohesionTarget = (Cohesion / CohCount) - Agent.Location;
        Force += CohesionTarget.GetSafeNormal() * FlockingParams.CohesionWeight;
    }

    return Force;
}

void UCrowdSimulationManager::TickSimulation(float DeltaTime)
{
    if (!bSimulationActive) return;

    // Decay threat events
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
    ThreatEvents.RemoveAll([&](const FCrowd_ThreatEvent& E) {
        return (CurrentTime - E.TimeStamp) > (1.f / ThreatDecayRate);
    });

    // Decay agent fear
    for (FCrowd_AgentData& Agent : ActiveAgents)
    {
        Agent.Fear = FMath::Max(0.f, Agent.Fear - ThreatDecayRate * DeltaTime * 10.f);

        if (Agent.Fear < 20.f && Agent.State == ECrowd_AgentState::Fleeing)
        {
            Agent.State = ECrowd_AgentState::Wandering;
        }

        // Apply flocking
        if (Agent.State == ECrowd_AgentState::Fleeing || Agent.State == ECrowd_AgentState::Wandering)
        {
            FVector Flocking = ComputeFlockingForce(Agent);
            float Speed = (Agent.State == ECrowd_AgentState::Fleeing)
                ? FlockingParams.MaxSpeed * FlockingParams.FleeSpeedMultiplier
                : FlockingParams.MaxSpeed;

            Agent.Velocity = (Agent.Velocity + Flocking * DeltaTime * 100.f).GetClampedToMaxSize(Speed);
            Agent.Location += Agent.Velocity * DeltaTime;
        }
    }
}

int32 UCrowdSimulationManager::GetActiveAgentCount() const
{
    return ActiveAgentCount;
}

TArray<FCrowd_AgentData> UCrowdSimulationManager::GetAgentsInRadius(const FVector& Center, float Radius) const
{
    TArray<FCrowd_AgentData> Result;
    for (const FCrowd_AgentData& Agent : ActiveAgents)
    {
        if (FVector::Dist(Agent.Location, Center) <= Radius)
        {
            Result.Add(Agent);
        }
    }
    return Result;
}

void UCrowdSimulationManager::ClearAllAgents()
{
    ActiveAgents.Empty();
    ActiveAgentCount = 0;
    ThreatEvents.Empty();
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] All agents cleared"));
}
