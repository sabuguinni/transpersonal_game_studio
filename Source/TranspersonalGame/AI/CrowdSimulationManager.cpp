// CrowdSimulationManager.cpp
// Agent #13 — Crowd & Traffic Simulation
// Prehistoric crowd simulation: tribe members + dinosaur herds

#include "CrowdSimulationManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "NavigationSystem.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UCrowdSimulationManager::UCrowdSimulationManager()
{
    MaxTribeAgents = 50;
    MaxHerdAgents = 20;
    MaxRaptorPackAgents = 8;
    AgentUpdateInterval = 0.5f;
    FlockingRadius = 300.0f;
    SeparationRadius = 80.0f;
    bSimulationActive = false;
}

void UCrowdSimulationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    bSimulationActive = true;
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Initialized — Max agents: Tribe=%d, Herd=%d, Pack=%d"),
        MaxTribeAgents, MaxHerdAgents, MaxRaptorPackAgents);
}

void UCrowdSimulationManager::Deinitialize()
{
    bSimulationActive = false;
    TribeAgents.Empty();
    HerdAgents.Empty();
    RaptorPackAgents.Empty();
    Super::Deinitialize();
}

void UCrowdSimulationManager::RegisterTribeAgent(AActor* Agent)
{
    if (!Agent) return;
    if (TribeAgents.Num() >= MaxTribeAgents)
    {
        UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Tribe agent cap reached (%d)"), MaxTribeAgents);
        return;
    }
    TribeAgents.AddUnique(Agent);
    UE_LOG(LogTemp, Verbose, TEXT("CrowdSimulationManager: Registered tribe agent %s (total: %d)"),
        *Agent->GetName(), TribeAgents.Num());
}

void UCrowdSimulationManager::RegisterHerdAgent(AActor* Agent)
{
    if (!Agent) return;
    if (HerdAgents.Num() >= MaxHerdAgents)
    {
        UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Herd agent cap reached (%d)"), MaxHerdAgents);
        return;
    }
    HerdAgents.AddUnique(Agent);
}

void UCrowdSimulationManager::RegisterRaptorPackAgent(AActor* Agent)
{
    if (!Agent) return;
    if (RaptorPackAgents.Num() >= MaxRaptorPackAgents)
    {
        UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Raptor pack cap reached (%d)"), MaxRaptorPackAgents);
        return;
    }
    RaptorPackAgents.AddUnique(Agent);
}

FVector UCrowdSimulationManager::ComputeFlockingVector(AActor* Agent, const TArray<AActor*>& Group) const
{
    if (!Agent || Group.Num() <= 1) return FVector::ZeroVector;

    FVector Separation = FVector::ZeroVector;
    FVector Alignment = FVector::ZeroVector;
    FVector Cohesion = FVector::ZeroVector;
    int32 NeighborCount = 0;

    FVector AgentLoc = Agent->GetActorLocation();

    for (AActor* Other : Group)
    {
        if (!Other || Other == Agent) continue;

        FVector Delta = AgentLoc - Other->GetActorLocation();
        float Dist = Delta.Size();

        if (Dist < FlockingRadius)
        {
            // Separation: push away from close neighbors
            if (Dist < SeparationRadius && Dist > 0.0f)
            {
                Separation += Delta.GetSafeNormal() * (SeparationRadius / FMath::Max(Dist, 1.0f));
            }

            // Cohesion: move toward group center
            Cohesion += Other->GetActorLocation();

            // Alignment: match velocity direction
            Alignment += Other->GetActorForwardVector();

            NeighborCount++;
        }
    }

    if (NeighborCount == 0) return FVector::ZeroVector;

    Cohesion = (Cohesion / NeighborCount) - AgentLoc;
    Cohesion.Normalize();
    Alignment /= NeighborCount;
    Alignment.Normalize();

    // Weighted combination: separation has highest priority
    FVector Result = (Separation * 2.0f) + (Alignment * 1.0f) + (Cohesion * 0.5f);
    Result.Z = 0.0f; // Keep movement on ground plane
    return Result.GetSafeNormal();
}

int32 UCrowdSimulationManager::GetTotalActiveAgents() const
{
    return TribeAgents.Num() + HerdAgents.Num() + RaptorPackAgents.Num();
}

FCrowd_SimulationStats UCrowdSimulationManager::GetSimulationStats() const
{
    FCrowd_SimulationStats Stats;
    Stats.TribeAgentCount = TribeAgents.Num();
    Stats.HerdAgentCount = HerdAgents.Num();
    Stats.RaptorPackCount = RaptorPackAgents.Num();
    Stats.TotalAgents = GetTotalActiveAgents();
    Stats.bIsActive = bSimulationActive;
    Stats.MaxCapacity = MaxTribeAgents + MaxHerdAgents + MaxRaptorPackAgents;
    return Stats;
}
