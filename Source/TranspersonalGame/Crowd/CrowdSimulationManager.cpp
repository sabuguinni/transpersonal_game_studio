// CrowdSimulationManager.cpp
// Agent #13 — Crowd & Traffic Simulation
// Prehistoric survival game — tribal crowd and dinosaur herd simulation

#include "CrowdSimulationManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "AIController.h"

UCrowdSimulationManager::UCrowdSimulationManager()
{
    MaxAgents = 200;
    LOD0Radius = 500.0f;
    LOD1Radius = 1500.0f;
    LOD2Radius = 3000.0f;
    TickInterval = 0.1f;
    bStampedeActive = false;
    StampedeDirection = FVector(1.0f, 0.0f, 0.0f);
    StampedeSpeed = 800.0f;
}

void UCrowdSimulationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Initialized — MaxAgents: %d, LOD radii: %.0f/%.0f/%.0f"),
        MaxAgents, LOD0Radius, LOD1Radius, LOD2Radius);
}

void UCrowdSimulationManager::Deinitialize()
{
    ActiveAgents.Empty();
    HerdGroups.Empty();
    Super::Deinitialize();
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Deinitialized"));
}

void UCrowdSimulationManager::RegisterCrowdAgent(AActor* Agent, ECrowd_AgentType AgentType)
{
    if (!Agent) return;

    FCrowd_AgentData NewAgent;
    NewAgent.AgentActor = Agent;
    NewAgent.AgentType = AgentType;
    NewAgent.CurrentLOD = ECrowd_LODLevel::LOD0_FullDetail;
    NewAgent.bIsInStampede = false;
    NewAgent.CurrentSpeed = (AgentType == ECrowd_AgentType::Herbivore_Herd) ? 300.0f : 150.0f;
    NewAgent.HomeLocation = Agent->GetActorLocation();
    NewAgent.TargetLocation = Agent->GetActorLocation();

    ActiveAgents.Add(NewAgent);
    UE_LOG(LogTemp, Verbose, TEXT("[CrowdSim] Registered agent: %s (Type: %d)"),
        *Agent->GetName(), (int32)AgentType);
}

void UCrowdSimulationManager::UnregisterCrowdAgent(AActor* Agent)
{
    if (!Agent) return;
    ActiveAgents.RemoveAll([Agent](const FCrowd_AgentData& Data) {
        return Data.AgentActor == Agent;
    });
}

void UCrowdSimulationManager::UpdateAgentLOD(FCrowd_AgentData& AgentData, const FVector& PlayerLocation)
{
    if (!AgentData.AgentActor.IsValid()) return;

    float Distance = FVector::Dist(AgentData.AgentActor->GetActorLocation(), PlayerLocation);

    ECrowd_LODLevel NewLOD;
    if (Distance <= LOD0Radius)
        NewLOD = ECrowd_LODLevel::LOD0_FullDetail;
    else if (Distance <= LOD1Radius)
        NewLOD = ECrowd_LODLevel::LOD1_Medium;
    else if (Distance <= LOD2Radius)
        NewLOD = ECrowd_LODLevel::LOD2_Low;
    else
        NewLOD = ECrowd_LODLevel::LOD3_Culled;

    if (NewLOD != AgentData.CurrentLOD)
    {
        AgentData.CurrentLOD = NewLOD;
        // Adjust tick rate based on LOD
        switch (NewLOD)
        {
        case ECrowd_LODLevel::LOD0_FullDetail:
            AgentData.CurrentSpeed *= 1.0f; // Full simulation
            break;
        case ECrowd_LODLevel::LOD1_Medium:
            AgentData.CurrentSpeed *= 0.8f; // Reduced fidelity
            break;
        case ECrowd_LODLevel::LOD2_Low:
            AgentData.CurrentSpeed *= 0.5f; // Minimal simulation
            break;
        case ECrowd_LODLevel::LOD3_Culled:
            // Agent invisible — skip simulation entirely
            break;
        }
    }
}

void UCrowdSimulationManager::TriggerStampede(const FVector& Origin, const FVector& Direction, float Radius)
{
    if (bStampedeActive) return;

    bStampedeActive = true;
    StampedeDirection = Direction.GetSafeNormal();
    StampedeOrigin = Origin;

    int32 AffectedCount = 0;
    for (FCrowd_AgentData& Agent : ActiveAgents)
    {
        if (!Agent.AgentActor.IsValid()) continue;
        float DistToOrigin = FVector::Dist(Agent.AgentActor->GetActorLocation(), Origin);
        if (DistToOrigin <= Radius && Agent.AgentType == ECrowd_AgentType::Herbivore_Herd)
        {
            Agent.bIsInStampede = true;
            Agent.TargetLocation = Agent.AgentActor->GetActorLocation() + Direction * 5000.0f;
            AffectedCount++;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("[CrowdSim] STAMPEDE TRIGGERED — Origin: %s, Direction: %s, Affected: %d"),
        *Origin.ToString(), *Direction.ToString(), AffectedCount);
}

void UCrowdSimulationManager::StopStampede()
{
    bStampedeActive = false;
    for (FCrowd_AgentData& Agent : ActiveAgents)
    {
        Agent.bIsInStampede = false;
    }
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Stampede stopped"));
}

void UCrowdSimulationManager::RegisterHerdGroup(const FCrowd_HerdGroup& HerdGroup)
{
    HerdGroups.Add(HerdGroup);
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Herd registered: %s (%d members)"),
        *HerdGroup.HerdName.ToString(), HerdGroup.Members.Num());
}

int32 UCrowdSimulationManager::GetActiveAgentCount() const
{
    return ActiveAgents.Num();
}

int32 UCrowdSimulationManager::GetAgentCountByType(ECrowd_AgentType AgentType) const
{
    int32 Count = 0;
    for (const FCrowd_AgentData& Agent : ActiveAgents)
    {
        if (Agent.AgentType == AgentType) Count++;
    }
    return Count;
}

void UCrowdSimulationManager::TickCrowdSimulation(float DeltaTime)
{
    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    if (!PC || !PC->GetPawn()) return;

    FVector PlayerLocation = PC->GetPawn()->GetActorLocation();

    for (FCrowd_AgentData& Agent : ActiveAgents)
    {
        if (!Agent.AgentActor.IsValid()) continue;

        // Update LOD based on distance to player
        UpdateAgentLOD(Agent, PlayerLocation);

        // Skip culled agents
        if (Agent.CurrentLOD == ECrowd_LODLevel::LOD3_Culled) continue;

        // Stampede movement
        if (Agent.bIsInStampede)
        {
            FVector CurrentLoc = Agent.AgentActor->GetActorLocation();
            FVector MoveDir = (Agent.TargetLocation - CurrentLoc).GetSafeNormal();
            FVector NewLoc = CurrentLoc + MoveDir * StampedeSpeed * DeltaTime;
            Agent.AgentActor->SetActorLocation(NewLoc, true);
        }
    }
}
