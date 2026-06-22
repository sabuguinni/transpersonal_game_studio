// CrowdSimulationManager.cpp
// Agent #13 — Crowd & Traffic Simulation
// Prehistoric crowd simulation: tribal camps, migration columns, scatter-on-threat

#include "CrowdSimulationManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UCrowdSimulationManager::UCrowdSimulationManager()
{
    MaxCrowdAgents = 50;
    ThreatScatterRadius = 1500.0f;
    CampRadius = 400.0f;
    MigrationSpeed = 120.0f;
    bCrowdActive = true;
}

void UCrowdSimulationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Initialized — MaxAgents:%d"), MaxCrowdAgents);
}

void UCrowdSimulationManager::Deinitialize()
{
    CrowdAgents.Empty();
    Super::Deinitialize();
}

void UCrowdSimulationManager::RegisterCrowdAgent(AActor* Agent, ECrowd_AgentRole Role)
{
    if (!Agent) return;
    FCrowd_AgentData Data;
    Data.AgentActor = Agent;
    Data.Role = Role;
    Data.HomeLocation = Agent->GetActorLocation();
    Data.CurrentState = ECrowd_AgentState::Idle;
    Data.bIsAlive = true;
    CrowdAgents.Add(Data);
    UE_LOG(LogTemp, Verbose, TEXT("[CrowdSim] Registered agent: %s Role:%d"), *Agent->GetName(), (int32)Role);
}

void UCrowdSimulationManager::UnregisterCrowdAgent(AActor* Agent)
{
    if (!Agent) return;
    CrowdAgents.RemoveAll([Agent](const FCrowd_AgentData& D){ return D.AgentActor == Agent; });
}

void UCrowdSimulationManager::TriggerThreatResponse(FVector ThreatLocation, float ThreatRadius)
{
    for (FCrowd_AgentData& Agent : CrowdAgents)
    {
        if (!Agent.AgentActor.IsValid() || !Agent.bIsAlive) continue;
        float Dist = FVector::Dist(Agent.AgentActor->GetActorLocation(), ThreatLocation);
        if (Dist <= ThreatRadius)
        {
            Agent.CurrentState = ECrowd_AgentState::Fleeing;
            // Scatter direction: away from threat
            FVector AwayDir = (Agent.AgentActor->GetActorLocation() - ThreatLocation).GetSafeNormal();
            Agent.FleeTarget = Agent.AgentActor->GetActorLocation() + AwayDir * ThreatScatterRadius;
            UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Agent %s FLEEING from threat at dist %.0f"),
                *Agent.AgentActor->GetName(), Dist);
        }
    }
}

void UCrowdSimulationManager::UpdateCrowdTick(float DeltaTime)
{
    if (!bCrowdActive) return;
    for (FCrowd_AgentData& Agent : CrowdAgents)
    {
        if (!Agent.AgentActor.IsValid() || !Agent.bIsAlive) continue;
        switch (Agent.CurrentState)
        {
            case ECrowd_AgentState::Idle:
                // Idle: minor random drift around home
                break;
            case ECrowd_AgentState::Migrating:
                // Move toward migration waypoint
                if (!Agent.MigrationWaypoints.IsEmpty())
                {
                    FVector Target = Agent.MigrationWaypoints[Agent.CurrentWaypointIndex % Agent.MigrationWaypoints.Num()];
                    FVector Dir = (Target - Agent.AgentActor->GetActorLocation()).GetSafeNormal();
                    FVector NewLoc = Agent.AgentActor->GetActorLocation() + Dir * MigrationSpeed * DeltaTime;
                    Agent.AgentActor->SetActorLocation(NewLoc);
                    if (FVector::Dist(NewLoc, Target) < 50.0f)
                        Agent.CurrentWaypointIndex++;
                }
                break;
            case ECrowd_AgentState::Fleeing:
                {
                    FVector Dir = (Agent.FleeTarget - Agent.AgentActor->GetActorLocation()).GetSafeNormal();
                    FVector NewLoc = Agent.AgentActor->GetActorLocation() + Dir * MigrationSpeed * 2.0f * DeltaTime;
                    Agent.AgentActor->SetActorLocation(NewLoc);
                    if (FVector::Dist(NewLoc, Agent.FleeTarget) < 100.0f)
                        Agent.CurrentState = ECrowd_AgentState::Idle;
                }
                break;
            case ECrowd_AgentState::Working:
                // Gathering/crafting: stay near home, animate in place
                break;
            default:
                break;
        }
    }
}

int32 UCrowdSimulationManager::GetActiveCrowdCount() const
{
    int32 Count = 0;
    for (const FCrowd_AgentData& D : CrowdAgents)
        if (D.AgentActor.IsValid() && D.bIsAlive) Count++;
    return Count;
}

void UCrowdSimulationManager::SetCrowdActive(bool bActive)
{
    bCrowdActive = bActive;
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Crowd active: %s"), bActive ? TEXT("true") : TEXT("false"));
}
