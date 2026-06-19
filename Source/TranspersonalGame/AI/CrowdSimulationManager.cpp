// CrowdSimulationManager.cpp
// Agent #13 — Crowd & Traffic Simulation
// Prehistoric crowd simulation: tribe members, dinosaur herds, migration paths

#include "CrowdSimulationManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

UCrowdSimulationManager::UCrowdSimulationManager()
{
    MaxCrowdAgents = 200;
    bSimulationActive = false;
    HerdMigrationSpeed = 150.0f;
    TribeMemberWanderRadius = 500.0f;
    RaptorPackSize = 3;
}

void UCrowdSimulationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    bSimulationActive = true;
    unreal::log("CrowdSimulationManager: Initialized — prehistoric crowd simulation active");
}

void UCrowdSimulationManager::Deinitialize()
{
    bSimulationActive = false;
    CrowdAgents.Empty();
    Super::Deinitialize();
}

void UCrowdSimulationManager::RegisterCrowdAgent(AActor* Agent, ECrowd_AgentType AgentType)
{
    if (!Agent) return;

    FCrowd_AgentData Data;
    Data.AgentActor = Agent;
    Data.AgentType = AgentType;
    Data.CurrentLocation = Agent->GetActorLocation();
    Data.TargetLocation = Agent->GetActorLocation();
    Data.bIsActive = true;
    Data.MoveSpeed = (AgentType == ECrowd_AgentType::TribeMember) ? 120.0f : 200.0f;

    CrowdAgents.Add(Data);
}

void UCrowdSimulationManager::UnregisterCrowdAgent(AActor* Agent)
{
    if (!Agent) return;
    CrowdAgents.RemoveAll([Agent](const FCrowd_AgentData& D) {
        return D.AgentActor == Agent;
    });
}

void UCrowdSimulationManager::SetHerdMigrationTarget(const FVector& Destination)
{
    for (FCrowd_AgentData& Agent : CrowdAgents)
    {
        if (Agent.AgentType == ECrowd_AgentType::HerbivoreHerd)
        {
            // Offset each herd member slightly for natural formation
            FVector Offset = FVector(
                FMath::RandRange(-200.0f, 200.0f),
                FMath::RandRange(-200.0f, 200.0f),
                0.0f
            );
            Agent.TargetLocation = Destination + Offset;
        }
    }
}

void UCrowdSimulationManager::ActivateRaptorPackHunt(const FVector& PreyLocation)
{
    int32 PackCount = 0;
    for (FCrowd_AgentData& Agent : CrowdAgents)
    {
        if (Agent.AgentType == ECrowd_AgentType::RaptorPack && PackCount < RaptorPackSize)
        {
            // Flanking positions — raptors surround prey
            float Angle = (360.0f / RaptorPackSize) * PackCount;
            float Rad = FMath::DegreesToRadians(Angle);
            FVector FlankOffset = FVector(
                FMath::Cos(Rad) * 300.0f,
                FMath::Sin(Rad) * 300.0f,
                0.0f
            );
            Agent.TargetLocation = PreyLocation + FlankOffset;
            Agent.bIsActive = true;
            PackCount++;
        }
    }
}

int32 UCrowdSimulationManager::GetActiveCrowdCount() const
{
    int32 Count = 0;
    for (const FCrowd_AgentData& Agent : CrowdAgents)
    {
        if (Agent.bIsActive) Count++;
    }
    return Count;
}

TArray<FCrowd_AgentData> UCrowdSimulationManager::GetAgentsByType(ECrowd_AgentType AgentType) const
{
    TArray<FCrowd_AgentData> Result;
    for (const FCrowd_AgentData& Agent : CrowdAgents)
    {
        if (Agent.AgentType == AgentType)
        {
            Result.Add(Agent);
        }
    }
    return Result;
}
