// CrowdSimulationManager.cpp
// Agent #13 — Crowd & Traffic Simulation
// Prehistoric tribe crowd simulation for Transpersonal Game Studio

#include "CrowdSimulationManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "NavigationSystem.h"
#include "NavMesh/RecastNavMesh.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

UCrowdSimulationManager::UCrowdSimulationManager()
{
    MaxCrowdAgents = 50;
    AgentUpdateRadius = 2000.0f;
    bCrowdSimulationActive = false;
    LODDistanceClose = 500.0f;
    LODDistanceMedium = 1500.0f;
    LODDistanceFar = 3000.0f;
}

void UCrowdSimulationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    bCrowdSimulationActive = true;
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] CrowdSimulationManager initialized. Max agents: %d"), MaxCrowdAgents);
}

void UCrowdSimulationManager::Deinitialize()
{
    bCrowdSimulationActive = false;
    ActiveAgents.Empty();
    Super::Deinitialize();
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] CrowdSimulationManager deinitialized."));
}

void UCrowdSimulationManager::RegisterCrowdAgent(AActor* Agent, ECrowd_AgentRole Role)
{
    if (!Agent) return;
    if (ActiveAgents.Num() >= MaxCrowdAgents)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CrowdSim] Max crowd agents (%d) reached. Cannot register: %s"), MaxCrowdAgents, *Agent->GetName());
        return;
    }

    FCrowd_AgentData NewAgent;
    NewAgent.AgentActor = Agent;
    NewAgent.Role = Role;
    NewAgent.CurrentState = ECrowd_AgentState::Idle;
    NewAgent.LODLevel = 0;
    NewAgent.HomeLocation = Agent->GetActorLocation();
    NewAgent.WanderRadius = 400.0f;
    ActiveAgents.Add(NewAgent);

    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Registered agent: %s Role=%d Total=%d"), *Agent->GetName(), (int32)Role, ActiveAgents.Num());
}

void UCrowdSimulationManager::UnregisterCrowdAgent(AActor* Agent)
{
    if (!Agent) return;
    ActiveAgents.RemoveAll([Agent](const FCrowd_AgentData& Data) { return Data.AgentActor == Agent; });
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Unregistered agent: %s Remaining=%d"), *Agent->GetName(), ActiveAgents.Num());
}

void UCrowdSimulationManager::UpdateCrowdLOD(const FVector& PlayerLocation)
{
    for (FCrowd_AgentData& Agent : ActiveAgents)
    {
        if (!Agent.AgentActor.IsValid()) continue;

        float Distance = FVector::Dist(PlayerLocation, Agent.AgentActor->GetActorLocation());

        int32 NewLOD = 0;
        if (Distance < LODDistanceClose)
            NewLOD = 0; // Full simulation
        else if (Distance < LODDistanceMedium)
            NewLOD = 1; // Reduced update rate
        else if (Distance < LODDistanceFar)
            NewLOD = 2; // Minimal simulation
        else
            NewLOD = 3; // Culled

        Agent.LODLevel = NewLOD;
    }
}

int32 UCrowdSimulationManager::GetActiveCrowdCount() const
{
    return ActiveAgents.Num();
}

TArray<FCrowd_AgentData> UCrowdSimulationManager::GetAgentsByRole(ECrowd_AgentRole Role) const
{
    TArray<FCrowd_AgentData> Result;
    for (const FCrowd_AgentData& Agent : ActiveAgents)
    {
        if (Agent.Role == Role)
            Result.Add(Agent);
    }
    return Result;
}

void UCrowdSimulationManager::TriggerCrowdFlee(const FVector& ThreatLocation, float FleeRadius)
{
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] FLEE triggered at (%.0f,%.0f,%.0f) radius=%.0f"), ThreatLocation.X, ThreatLocation.Y, ThreatLocation.Z, FleeRadius);

    for (FCrowd_AgentData& Agent : ActiveAgents)
    {
        if (!Agent.AgentActor.IsValid()) continue;

        float Distance = FVector::Dist(ThreatLocation, Agent.AgentActor->GetActorLocation());
        if (Distance <= FleeRadius)
        {
            Agent.CurrentState = ECrowd_AgentState::Fleeing;
            UE_LOG(LogTemp, Verbose, TEXT("[CrowdSim] Agent %s is FLEEING"), *Agent.AgentActor->GetName());
        }
    }
}

void UCrowdSimulationManager::TriggerCrowdAlert(const FVector& AlertLocation, float AlertRadius)
{
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] ALERT triggered at (%.0f,%.0f,%.0f) radius=%.0f"), AlertLocation.X, AlertLocation.Y, AlertLocation.Z, AlertRadius);

    for (FCrowd_AgentData& Agent : ActiveAgents)
    {
        if (!Agent.AgentActor.IsValid()) continue;

        float Distance = FVector::Dist(AlertLocation, Agent.AgentActor->GetActorLocation());
        if (Distance <= AlertRadius && Agent.CurrentState == ECrowd_AgentState::Idle)
        {
            Agent.CurrentState = ECrowd_AgentState::Alerted;
        }
    }
}
