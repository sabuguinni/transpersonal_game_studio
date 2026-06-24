// CrowdSimulationManager.cpp
// Agent #13 — Crowd & Traffic Simulation
// Prehistoric tribal crowd simulation using UE5 Mass AI patterns

#include "CrowdSimulationManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "NavigationSystem.h"
#include "AIController.h"
#include "TimerManager.h"

UCrowdSimulationManager::UCrowdSimulationManager()
{
    MaxCrowdAgents = 50;
    AgentUpdateRadius = 3000.0f;
    bCrowdActive = false;
    CampCenterLocation = FVector(900.0f, 100.0f, 50.0f);
    CampRadius = 600.0f;
    PatrolRadius = 1200.0f;
}

void UCrowdSimulationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    bCrowdActive = true;
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Initialized. Max agents: %d"), MaxCrowdAgents);

    // Start crowd tick timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            CrowdTickHandle,
            this,
            &UCrowdSimulationManager::TickCrowdAgents,
            2.0f,  // Update every 2 seconds for performance
            true
        );
    }
}

void UCrowdSimulationManager::Deinitialize()
{
    bCrowdActive = false;
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(CrowdTickHandle);
    }
    Super::Deinitialize();
}

void UCrowdSimulationManager::TickCrowdAgents()
{
    if (!bCrowdActive) return;

    UWorld* World = GetWorld();
    if (!World) return;

    // Update agent states based on time of day and threat level
    UpdateAgentBehaviorStates();
}

void UCrowdSimulationManager::UpdateAgentBehaviorStates()
{
    // Cycle through registered agents and update their behavior
    for (FCrowd_AgentData& Agent : RegisteredAgents)
    {
        switch (Agent.CurrentState)
        {
            case ECrowd_AgentState::Idle:
                // 30% chance to start wandering
                if (FMath::RandRange(0, 100) < 30)
                {
                    Agent.CurrentState = ECrowd_AgentState::Wandering;
                }
                break;

            case ECrowd_AgentState::Wandering:
                // Update wander target
                Agent.WanderTarget = GetRandomCampPosition();
                // 20% chance to return to idle
                if (FMath::RandRange(0, 100) < 20)
                {
                    Agent.CurrentState = ECrowd_AgentState::Idle;
                }
                break;

            case ECrowd_AgentState::Fleeing:
                // Fleeing agents always move away from threat
                // Revert to wandering after flee distance reached
                if (Agent.ThreatDistance > PatrolRadius)
                {
                    Agent.CurrentState = ECrowd_AgentState::Wandering;
                }
                break;

            case ECrowd_AgentState::Gathering:
                // Gathering agents cluster around fire pit
                Agent.WanderTarget = CampCenterLocation;
                break;

            default:
                break;
        }
    }
}

FVector UCrowdSimulationManager::GetRandomCampPosition() const
{
    float Angle = FMath::RandRange(0.0f, 360.0f);
    float Radius = FMath::RandRange(0.0f, CampRadius);
    float X = CampCenterLocation.X + Radius * FMath::Cos(FMath::DegreesToRadians(Angle));
    float Y = CampCenterLocation.Y + Radius * FMath::Sin(FMath::DegreesToRadians(Angle));
    return FVector(X, Y, CampCenterLocation.Z);
}

void UCrowdSimulationManager::RegisterAgent(const FCrowd_AgentData& AgentData)
{
    if (RegisteredAgents.Num() < MaxCrowdAgents)
    {
        RegisteredAgents.Add(AgentData);
        UE_LOG(LogTemp, Verbose, TEXT("CrowdSimulationManager: Registered agent %s"), *AgentData.AgentID.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Max agents reached (%d)"), MaxCrowdAgents);
    }
}

void UCrowdSimulationManager::UnregisterAgent(const FName& AgentID)
{
    RegisteredAgents.RemoveAll([&AgentID](const FCrowd_AgentData& Agent)
    {
        return Agent.AgentID == AgentID;
    });
}

void UCrowdSimulationManager::TriggerFleeResponse(const FVector& ThreatLocation, float ThreatRadius)
{
    for (FCrowd_AgentData& Agent : RegisteredAgents)
    {
        float DistToThreat = FVector::Dist(Agent.CurrentLocation, ThreatLocation);
        if (DistToThreat < ThreatRadius)
        {
            Agent.CurrentState = ECrowd_AgentState::Fleeing;
            Agent.ThreatDistance = DistToThreat;
            UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Agent %s fleeing from threat at %s"),
                *Agent.AgentID.ToString(), *ThreatLocation.ToString());
        }
    }
}

void UCrowdSimulationManager::TriggerGatheringEvent(const FVector& GatherLocation)
{
    for (FCrowd_AgentData& Agent : RegisteredAgents)
    {
        Agent.CurrentState = ECrowd_AgentState::Gathering;
        Agent.WanderTarget = GatherLocation;
    }
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Gathering event triggered at %s"), *GatherLocation.ToString());
}

int32 UCrowdSimulationManager::GetActiveAgentCount() const
{
    return RegisteredAgents.Num();
}

ECrowd_AgentState UCrowdSimulationManager::GetAgentState(const FName& AgentID) const
{
    for (const FCrowd_AgentData& Agent : RegisteredAgents)
    {
        if (Agent.AgentID == AgentID)
        {
            return Agent.CurrentState;
        }
    }
    return ECrowd_AgentState::Idle;
}
