// CrowdSimulationManager.cpp
// Agent #13 — Crowd & Traffic Simulation
// Prehistoric crowd simulation: tribe members, dinosaur herds, LOD crowd agents

#include "CrowdSimulationManager.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UCrowdSimulationManager::UCrowdSimulationManager()
{
    MaxCrowdAgents = 200;
    TribeSettlementRadius = 1500.0f;
    HerdSpreadRadius = 800.0f;
    LODDistanceNear = 2000.0f;
    LODDistanceFar = 6000.0f;
    bSimulationActive = false;
    TickInterval = 0.5f;
    TimeSinceLastTick = 0.0f;
}

void UCrowdSimulationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    bSimulationActive = true;
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Initialized — MaxAgents=%d, TribeRadius=%.0f"),
        MaxCrowdAgents, TribeSettlementRadius);
}

void UCrowdSimulationManager::Deinitialize()
{
    bSimulationActive = false;
    ActiveAgents.Empty();
    Super::Deinitialize();
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Deinitialized"));
}

void UCrowdSimulationManager::Tick(float DeltaTime)
{
    if (!bSimulationActive) return;

    TimeSinceLastTick += DeltaTime;
    if (TimeSinceLastTick < TickInterval) return;
    TimeSinceLastTick = 0.0f;

    UpdateAgentBehaviors(DeltaTime * TickInterval);
    UpdateHerdFormations(DeltaTime * TickInterval);
    UpdateLODLevels();
}

void UCrowdSimulationManager::RegisterAgent(FCrowd_AgentData& AgentData)
{
    if (ActiveAgents.Num() >= MaxCrowdAgents)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CrowdSim] Max agents reached (%d)"), MaxCrowdAgents);
        return;
    }
    AgentData.AgentID = NextAgentID++;
    AgentData.bIsActive = true;
    ActiveAgents.Add(AgentData);
    UE_LOG(LogTemp, Verbose, TEXT("[CrowdSim] Agent registered: ID=%d, Type=%d"),
        AgentData.AgentID, (int32)AgentData.AgentType);
}

void UCrowdSimulationManager::UnregisterAgent(int32 AgentID)
{
    ActiveAgents.RemoveAll([AgentID](const FCrowd_AgentData& A) {
        return A.AgentID == AgentID;
    });
}

void UCrowdSimulationManager::UpdateAgentBehaviors(float DeltaTime)
{
    for (FCrowd_AgentData& Agent : ActiveAgents)
    {
        if (!Agent.bIsActive) continue;

        switch (Agent.AgentType)
        {
            case ECrowd_AgentType::TribeHunter:
                UpdateHunterBehavior(Agent, DeltaTime);
                break;
            case ECrowd_AgentType::TribeGatherer:
                UpdateGathererBehavior(Agent, DeltaTime);
                break;
            case ECrowd_AgentType::TribeElder:
                // Elders stay near settlement center
                break;
            case ECrowd_AgentType::DinosaurHerd:
                UpdateHerdMemberBehavior(Agent, DeltaTime);
                break;
            default:
                break;
        }
    }
}

void UCrowdSimulationManager::UpdateHunterBehavior(FCrowd_AgentData& Agent, float DeltaTime)
{
    // Hunters patrol outward from settlement, return when stamina low
    Agent.CurrentStamina -= DeltaTime * 2.0f;
    if (Agent.CurrentStamina <= 0.0f)
    {
        Agent.CurrentStamina = Agent.MaxStamina;
        Agent.CurrentState = ECrowd_AgentState::Resting;
    }
    else if (Agent.CurrentState == ECrowd_AgentState::Resting)
    {
        Agent.CurrentState = ECrowd_AgentState::Patrolling;
    }
}

void UCrowdSimulationManager::UpdateGathererBehavior(FCrowd_AgentData& Agent, float DeltaTime)
{
    // Gatherers move toward resource nodes, collect, return
    Agent.CurrentStamina -= DeltaTime * 1.5f;
    if (Agent.CurrentStamina <= 20.0f)
    {
        Agent.CurrentState = ECrowd_AgentState::Returning;
    }
}

void UCrowdSimulationManager::UpdateHerdMemberBehavior(FCrowd_AgentData& Agent, float DeltaTime)
{
    // Herd members follow alpha, maintain formation spacing
    Agent.CurrentState = ECrowd_AgentState::Wandering;
}

void UCrowdSimulationManager::UpdateHerdFormations(float DeltaTime)
{
    // Group agents by herd ID and maintain formation
    TMap<int32, TArray<int32>> HerdGroups;
    for (const FCrowd_AgentData& Agent : ActiveAgents)
    {
        if (Agent.AgentType == ECrowd_AgentType::DinosaurHerd)
        {
            HerdGroups.FindOrAdd(Agent.HerdID).Add(Agent.AgentID);
        }
    }

    for (auto& Pair : HerdGroups)
    {
        // Maintain minimum spacing between herd members
        if (Pair.Value.Num() > 1)
        {
            UE_LOG(LogTemp, Verbose, TEXT("[CrowdSim] Herd %d has %d members"),
                Pair.Key, Pair.Value.Num());
        }
    }
}

void UCrowdSimulationManager::UpdateLODLevels()
{
    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC || !PC->GetPawn()) return;

    FVector PlayerLocation = PC->GetPawn()->GetActorLocation();

    for (FCrowd_AgentData& Agent : ActiveAgents)
    {
        float Distance = FVector::Dist(PlayerLocation, Agent.CurrentLocation);

        if (Distance < LODDistanceNear)
        {
            Agent.CurrentLOD = ECrowd_LODLevel::Full;
        }
        else if (Distance < LODDistanceFar)
        {
            Agent.CurrentLOD = ECrowd_LODLevel::Medium;
        }
        else
        {
            Agent.CurrentLOD = ECrowd_LODLevel::Distant;
        }
    }
}

int32 UCrowdSimulationManager::GetActiveAgentCount() const
{
    return ActiveAgents.Num();
}

TArray<FCrowd_AgentData> UCrowdSimulationManager::GetAgentsByType(ECrowd_AgentType AgentType) const
{
    TArray<FCrowd_AgentData> Result;
    for (const FCrowd_AgentData& Agent : ActiveAgents)
    {
        if (Agent.AgentType == AgentType)
        {
            Result.Add(Agent);
        }
    }
    return Result;
}

void UCrowdSimulationManager::SetSimulationActive(bool bActive)
{
    bSimulationActive = bActive;
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Simulation %s"), bActive ? TEXT("ACTIVE") : TEXT("PAUSED"));
}

bool UCrowdSimulationManager::IsTickable() const
{
    return bSimulationActive;
}

TStatId UCrowdSimulationManager::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UCrowdSimulationManager, STATGROUP_Tickables);
}
