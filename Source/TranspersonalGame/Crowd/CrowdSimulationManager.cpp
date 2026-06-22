// CrowdSimulationManager.cpp
// Agent #13 — Crowd & Traffic Simulation
// Prehistoric tribal crowd simulation using UE5 Mass AI primitives

#include "CrowdSimulationManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "NavigationSystem.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UCrowdSimulationManager::UCrowdSimulationManager()
{
    MaxCrowdAgents = 50;
    CampCenter = FVector(800.f, 400.f, 100.f);
    CampRadius = 300.f;
    PatrolSpeed = 120.f;
    FleeSpeed = 400.f;
    bCrowdInitialized = false;
    ThreatLevel = 0.f;
    DayPhase = ECrowd_DayPhase::Day;
}

void UCrowdSimulationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    InitializeCrowdAgents();
    bCrowdInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Initialized with %d agents"), ActiveAgents.Num());
}

void UCrowdSimulationManager::Deinitialize()
{
    ActiveAgents.Empty();
    PatrolWaypoints.Empty();
    Super::Deinitialize();
}

void UCrowdSimulationManager::InitializeCrowdAgents()
{
    // Define camp tribal agents in circular formation
    const int32 TribeCount = 8;
    for (int32 i = 0; i < TribeCount; ++i)
    {
        FCrowd_AgentData Agent;
        float Angle = (2.f * PI * i) / TribeCount;
        Agent.AgentID = i;
        Agent.Location = CampCenter + FVector(
            CampRadius * FMath::Cos(Angle),
            CampRadius * FMath::Sin(Angle),
            0.f
        );
        Agent.AgentType = ECrowd_AgentType::TribalMember;
        Agent.CurrentState = ECrowd_AgentState::Idle;
        Agent.MoveSpeed = PatrolSpeed;
        Agent.bIsAlive = true;
        Agent.FearLevel = 0.f;
        ActiveAgents.Add(Agent);
    }

    // Define forager agents spread in the world
    TArray<FVector> ForagerPositions = {
        FVector(-300.f, 200.f, 100.f),
        FVector(-500.f, -400.f, 100.f),
        FVector(1500.f, 600.f, 100.f),
        FVector(600.f, 1400.f, 100.f)
    };

    for (int32 i = 0; i < ForagerPositions.Num(); ++i)
    {
        FCrowd_AgentData Agent;
        Agent.AgentID = TribeCount + i;
        Agent.Location = ForagerPositions[i];
        Agent.AgentType = ECrowd_AgentType::Forager;
        Agent.CurrentState = ECrowd_AgentState::Foraging;
        Agent.MoveSpeed = PatrolSpeed * 0.8f;
        Agent.bIsAlive = true;
        Agent.FearLevel = 0.f;
        ActiveAgents.Add(Agent);
    }

    // Define patrol waypoints
    PatrolWaypoints = {
        FVector(1200.f, 400.f, 100.f),
        FVector(1000.f, 900.f, 100.f),
        FVector(400.f, 1000.f, 100.f),
        FVector(100.f, 500.f, 100.f),
        FVector(300.f, 0.f, 100.f),
        FVector(900.f, -100.f, 100.f)
    };

    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] %d tribal + %d forager agents initialized"),
        TribeCount, ForagerPositions.Num());
}

void UCrowdSimulationManager::TickCrowdSimulation(float DeltaTime)
{
    if (!bCrowdInitialized) return;

    for (FCrowd_AgentData& Agent : ActiveAgents)
    {
        if (!Agent.bIsAlive) continue;

        switch (Agent.CurrentState)
        {
            case ECrowd_AgentState::Idle:
                UpdateIdleAgent(Agent, DeltaTime);
                break;
            case ECrowd_AgentState::Patrolling:
                UpdatePatrolAgent(Agent, DeltaTime);
                break;
            case ECrowd_AgentState::Foraging:
                UpdateForagingAgent(Agent, DeltaTime);
                break;
            case ECrowd_AgentState::Fleeing:
                UpdateFleeingAgent(Agent, DeltaTime);
                break;
            case ECrowd_AgentState::Sheltering:
                // Agent stays at camp — no movement
                break;
            default:
                break;
        }

        // Propagate fear: if threat is high, transition to flee
        if (ThreatLevel > 0.7f && Agent.CurrentState != ECrowd_AgentState::Fleeing)
        {
            Agent.CurrentState = ECrowd_AgentState::Fleeing;
            Agent.MoveSpeed = FleeSpeed;
            Agent.FearLevel = ThreatLevel;
        }
    }
}

void UCrowdSimulationManager::UpdateIdleAgent(FCrowd_AgentData& Agent, float DeltaTime)
{
    // Idle agents occasionally transition to patrol
    Agent.IdleTimer += DeltaTime;
    if (Agent.IdleTimer > 5.f)
    {
        Agent.IdleTimer = 0.f;
        if (Agent.AgentType == ECrowd_AgentType::TribalMember)
        {
            Agent.CurrentState = ECrowd_AgentState::Patrolling;
            Agent.CurrentWaypointIndex = FMath::RandRange(0, PatrolWaypoints.Num() - 1);
        }
        else if (Agent.AgentType == ECrowd_AgentType::Forager)
        {
            Agent.CurrentState = ECrowd_AgentState::Foraging;
        }
    }
}

void UCrowdSimulationManager::UpdatePatrolAgent(FCrowd_AgentData& Agent, float DeltaTime)
{
    if (PatrolWaypoints.Num() == 0) return;

    FVector Target = PatrolWaypoints[Agent.CurrentWaypointIndex % PatrolWaypoints.Num()];
    FVector Direction = (Target - Agent.Location).GetSafeNormal();
    float DistToTarget = FVector::Dist(Agent.Location, Target);

    if (DistToTarget < 50.f)
    {
        // Reached waypoint — advance to next
        Agent.CurrentWaypointIndex = (Agent.CurrentWaypointIndex + 1) % PatrolWaypoints.Num();
        Agent.CurrentState = ECrowd_AgentState::Idle;
        Agent.IdleTimer = 0.f;
    }
    else
    {
        Agent.Location += Direction * Agent.MoveSpeed * DeltaTime;
    }
}

void UCrowdSimulationManager::UpdateForagingAgent(FCrowd_AgentData& Agent, float DeltaTime)
{
    // Foragers wander in a small radius around their start position
    Agent.ForageTimer += DeltaTime;
    if (Agent.ForageTimer > 3.f)
    {
        Agent.ForageTimer = 0.f;
        // Pick a random nearby point
        FVector RandomOffset = FVector(
            FMath::RandRange(-200.f, 200.f),
            FMath::RandRange(-200.f, 200.f),
            0.f
        );
        Agent.ForageTarget = Agent.Location + RandomOffset;
    }

    FVector Direction = (Agent.ForageTarget - Agent.Location).GetSafeNormal();
    Agent.Location += Direction * Agent.MoveSpeed * DeltaTime;
}

void UCrowdSimulationManager::UpdateFleeingAgent(FCrowd_AgentData& Agent, float DeltaTime)
{
    // Flee toward camp center
    FVector Direction = (CampCenter - Agent.Location).GetSafeNormal();
    float DistToCamp = FVector::Dist(Agent.Location, CampCenter);

    if (DistToCamp < 100.f)
    {
        // Reached safety — shelter
        Agent.CurrentState = ECrowd_AgentState::Sheltering;
        Agent.FearLevel = FMath::Max(0.f, Agent.FearLevel - 0.1f);
    }
    else
    {
        Agent.Location += Direction * Agent.MoveSpeed * DeltaTime;
    }
}

void UCrowdSimulationManager::NotifyThreat(FVector ThreatLocation, float Intensity)
{
    ThreatLevel = FMath::Clamp(Intensity, 0.f, 1.f);
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Threat notified at (%.0f,%.0f,%.0f) intensity=%.2f"),
        ThreatLocation.X, ThreatLocation.Y, ThreatLocation.Z, Intensity);

    // Propagate fear to nearby agents
    for (FCrowd_AgentData& Agent : ActiveAgents)
    {
        float Dist = FVector::Dist(Agent.Location, ThreatLocation);
        if (Dist < 1000.f)
        {
            float FearFromThreat = Intensity * (1.f - Dist / 1000.f);
            Agent.FearLevel = FMath::Min(1.f, Agent.FearLevel + FearFromThreat);
        }
    }
}

void UCrowdSimulationManager::SetDayPhase(ECrowd_DayPhase NewPhase)
{
    DayPhase = NewPhase;

    // Adjust crowd behavior based on time of day
    for (FCrowd_AgentData& Agent : ActiveAgents)
    {
        switch (DayPhase)
        {
            case ECrowd_DayPhase::Night:
                // At night, all agents shelter at camp
                Agent.CurrentState = ECrowd_AgentState::Sheltering;
                break;
            case ECrowd_DayPhase::Dawn:
            case ECrowd_DayPhase::Day:
                // During day, resume normal activities
                if (Agent.CurrentState == ECrowd_AgentState::Sheltering)
                {
                    Agent.CurrentState = ECrowd_AgentState::Idle;
                }
                break;
            case ECrowd_DayPhase::Dusk:
                // At dusk, foragers return to camp
                if (Agent.AgentType == ECrowd_AgentType::Forager)
                {
                    Agent.CurrentState = ECrowd_AgentState::Fleeing; // reuse flee-to-camp logic
                }
                break;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Day phase changed to %d"), (int32)DayPhase);
}

int32 UCrowdSimulationManager::GetActiveAgentCount() const
{
    int32 Count = 0;
    for (const FCrowd_AgentData& Agent : ActiveAgents)
    {
        if (Agent.bIsAlive) Count++;
    }
    return Count;
}

TArray<FCrowd_AgentData> UCrowdSimulationManager::GetAgentsByType(ECrowd_AgentType Type) const
{
    TArray<FCrowd_AgentData> Result;
    for (const FCrowd_AgentData& Agent : ActiveAgents)
    {
        if (Agent.AgentType == Type && Agent.bIsAlive)
        {
            Result.Add(Agent);
        }
    }
    return Result;
}
