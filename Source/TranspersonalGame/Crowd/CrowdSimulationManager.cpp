// CrowdSimulationManager.cpp
// Agent #13 — Crowd & Traffic Simulation
// Prehistoric tribe crowd simulation using UE5 Mass AI primitives

#include "CrowdSimulationManager.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

UCrowdSimulationManager::UCrowdSimulationManager()
{
    MaxActiveAgents = 50;
    AgentUpdateRadius = 5000.0f;
    LODDistanceClose = 1500.0f;
    LODDistanceMid = 4000.0f;
    bSimulationActive = false;
    TickInterval = 0.1f;
    CurrentTickAccumulator = 0.0f;
}

void UCrowdSimulationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    bSimulationActive = true;
    ActiveAgents.Reserve(MaxActiveAgents);
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Initialized — max %d agents"), MaxActiveAgents);
}

void UCrowdSimulationManager::Deinitialize()
{
    bSimulationActive = false;
    ActiveAgents.Empty();
    Super::Deinitialize();
}

void UCrowdSimulationManager::RegisterAgent(FCrowd_AgentData& AgentData)
{
    if (ActiveAgents.Num() >= MaxActiveAgents)
    {
        UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Agent cap reached (%d)"), MaxActiveAgents);
        return;
    }
    AgentData.AgentID = NextAgentID++;
    AgentData.bIsActive = true;
    AgentData.CurrentLOD = ECrowd_AgentLOD::Full;
    ActiveAgents.Add(AgentData);
    UE_LOG(LogTemp, Verbose, TEXT("CrowdSimulationManager: Registered agent ID=%d type=%d"), AgentData.AgentID, (int32)AgentData.AgentType);
}

void UCrowdSimulationManager::UnregisterAgent(int32 AgentID)
{
    ActiveAgents.RemoveAll([AgentID](const FCrowd_AgentData& A) { return A.AgentID == AgentID; });
}

void UCrowdSimulationManager::Tick(float DeltaTime)
{
    if (!bSimulationActive) return;

    CurrentTickAccumulator += DeltaTime;
    if (CurrentTickAccumulator < TickInterval) return;
    CurrentTickAccumulator = 0.0f;

    UWorld* World = GetWorld();
    if (!World) return;

    // Get player location for LOD calculation
    APlayerController* PC = World->GetFirstPlayerController();
    FVector PlayerLoc = FVector::ZeroVector;
    if (PC && PC->GetPawn())
    {
        PlayerLoc = PC->GetPawn()->GetActorLocation();
    }

    // Update each agent
    for (FCrowd_AgentData& Agent : ActiveAgents)
    {
        if (!Agent.bIsActive) continue;
        UpdateAgentLOD(Agent, PlayerLoc);
        UpdateAgentBehavior(Agent, DeltaTime * (TickInterval / DeltaTime));
    }
}

void UCrowdSimulationManager::UpdateAgentLOD(FCrowd_AgentData& Agent, const FVector& PlayerLocation)
{
    float Distance = FVector::Dist(Agent.CurrentLocation, PlayerLocation);

    if (Distance < LODDistanceClose)
    {
        Agent.CurrentLOD = ECrowd_AgentLOD::Full;
    }
    else if (Distance < LODDistanceMid)
    {
        Agent.CurrentLOD = ECrowd_AgentLOD::Reduced;
    }
    else
    {
        Agent.CurrentLOD = ECrowd_AgentLOD::Minimal;
    }
}

void UCrowdSimulationManager::UpdateAgentBehavior(FCrowd_AgentData& Agent, float DeltaTime)
{
    // Skip full simulation for distant agents
    if (Agent.CurrentLOD == ECrowd_AgentLOD::Minimal) return;

    switch (Agent.AgentType)
    {
        case ECrowd_AgentType::Gatherer:
            SimulateGathererBehavior(Agent, DeltaTime);
            break;
        case ECrowd_AgentType::Hunter:
            SimulateHunterBehavior(Agent, DeltaTime);
            break;
        case ECrowd_AgentType::Sentinel:
            SimulateSentinelBehavior(Agent, DeltaTime);
            break;
        default:
            break;
    }
}

void UCrowdSimulationManager::SimulateGathererBehavior(FCrowd_AgentData& Agent, float DeltaTime)
{
    // Gatherers move slowly between resource nodes
    // Simplified: wander within home radius
    if (FVector::Dist(Agent.CurrentLocation, Agent.HomeLocation) > Agent.WanderRadius)
    {
        // Move back toward home
        FVector Direction = (Agent.HomeLocation - Agent.CurrentLocation).GetSafeNormal();
        Agent.CurrentLocation += Direction * Agent.MoveSpeed * DeltaTime;
    }
    else
    {
        // Random wander
        FVector RandomOffset = FVector(
            FMath::RandRange(-50.0f, 50.0f),
            FMath::RandRange(-50.0f, 50.0f),
            0.0f
        );
        Agent.CurrentLocation += RandomOffset * DeltaTime;
    }
}

void UCrowdSimulationManager::SimulateHunterBehavior(FCrowd_AgentData& Agent, float DeltaTime)
{
    // Hunters patrol a wider radius, alert to threats
    if (Agent.bAlerted)
    {
        // Move toward threat location
        FVector Direction = (Agent.TargetLocation - Agent.CurrentLocation).GetSafeNormal();
        Agent.CurrentLocation += Direction * Agent.MoveSpeed * 1.5f * DeltaTime;
    }
    else
    {
        // Standard patrol
        SimulateGathererBehavior(Agent, DeltaTime);
    }
}

void UCrowdSimulationManager::SimulateSentinelBehavior(FCrowd_AgentData& Agent, float DeltaTime)
{
    // Sentinels stay near home, rotate to scan
    // Minimal movement — just update facing direction
    Agent.FacingAngle += 15.0f * DeltaTime;
    if (Agent.FacingAngle > 360.0f) Agent.FacingAngle -= 360.0f;
}

void UCrowdSimulationManager::AlertNearbyAgents(const FVector& ThreatLocation, float AlertRadius, ECrowd_ThreatType ThreatType)
{
    int32 AlertedCount = 0;
    for (FCrowd_AgentData& Agent : ActiveAgents)
    {
        if (!Agent.bIsActive) continue;
        float Distance = FVector::Dist(Agent.CurrentLocation, ThreatLocation);
        if (Distance <= AlertRadius)
        {
            Agent.bAlerted = true;
            Agent.TargetLocation = ThreatLocation;
            Agent.CurrentThreat = ThreatType;
            AlertedCount++;
        }
    }
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Alerted %d agents — threat type %d at (%.0f,%.0f,%.0f)"),
        AlertedCount, (int32)ThreatType, ThreatLocation.X, ThreatLocation.Y, ThreatLocation.Z);
}

int32 UCrowdSimulationManager::GetActiveAgentCount() const
{
    return ActiveAgents.Num();
}

TArray<FCrowd_AgentData> UCrowdSimulationManager::GetAgentsInRadius(const FVector& Center, float Radius) const
{
    TArray<FCrowd_AgentData> Result;
    for (const FCrowd_AgentData& Agent : ActiveAgents)
    {
        if (FVector::Dist(Agent.CurrentLocation, Center) <= Radius)
        {
            Result.Add(Agent);
        }
    }
    return Result;
}
