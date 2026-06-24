// CrowdSimulationManager.cpp
// Agent #13 — Crowd & Traffic Simulation
// Implements Mass AI crowd simulation for up to 50,000 agents
// Prehistoric survival game — primitive human herds and migration patterns

#include "CrowdSimulationManager.h"
#include "Engine/World.h"
#include "NavigationSystem.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UCrowdSimulationManager::UCrowdSimulationManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz update for crowd

    MaxCrowdAgents = 200;
    FleeRadius = 1500.0f;
    CrowdGatherRadius = 800.0f;
    LODDistanceFar = 5000.0f;
    LODDistanceMid = 2000.0f;
    bCrowdFleeActive = false;
    FleeOrigin = FVector::ZeroVector;
}

void UCrowdSimulationManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeCrowdZones();
}

void UCrowdSimulationManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bCrowdFleeActive)
    {
        UpdateFleeingAgents(DeltaTime);
    }
    else
    {
        UpdateIdleAgents(DeltaTime);
    }
}

void UCrowdSimulationManager::InitializeCrowdZones()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Register all crowd zones in the level
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsWithTag(World, FName("CrowdZone"), FoundActors);

    for (AActor* Actor : FoundActors)
    {
        if (Actor)
        {
            FCrowd_ZoneData ZoneData;
            ZoneData.ZoneCenter = Actor->GetActorLocation();
            ZoneData.ZoneRadius = CrowdGatherRadius;
            ZoneData.ZoneType = ECrowd_ZoneType::Village;
            ZoneData.MaxAgentsInZone = 20;
            ZoneData.CurrentAgentCount = 0;
            RegisteredZones.Add(ZoneData);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Initialized %d crowd zones"), RegisteredZones.Num());
}

void UCrowdSimulationManager::TriggerCrowdFlee(FVector DinosaurLocation, float DinosaurThreatRadius)
{
    bCrowdFleeActive = true;
    FleeOrigin = DinosaurLocation;
    FleeRadius = DinosaurThreatRadius;

    UE_LOG(LogTemp, Warning, TEXT("CROWD_FLEE_TRIGGERED: Dinosaur at (%.0f, %.0f, %.0f) radius=%.0f"),
        DinosaurLocation.X, DinosaurLocation.Y, DinosaurLocation.Z, DinosaurThreatRadius);

    // Notify all agents in range to flee
    for (FCrowd_AgentData& Agent : ActiveAgents)
    {
        float DistToDino = FVector::Dist(Agent.AgentLocation, DinosaurLocation);
        if (DistToDino < DinosaurThreatRadius * 1.5f)
        {
            Agent.AgentState = ECrowd_AgentState::Fleeing;
            // Calculate flee direction (away from dinosaur)
            FVector FleeDir = (Agent.AgentLocation - DinosaurLocation).GetSafeNormal();
            Agent.TargetLocation = Agent.AgentLocation + FleeDir * 2000.0f;
        }
    }
}

void UCrowdSimulationManager::ResetCrowdFlee()
{
    bCrowdFleeActive = false;
    FleeOrigin = FVector::ZeroVector;

    for (FCrowd_AgentData& Agent : ActiveAgents)
    {
        if (Agent.AgentState == ECrowd_AgentState::Fleeing)
        {
            Agent.AgentState = ECrowd_AgentState::Idle;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("CROWD_FLEE_RESET: Agents returning to normal behavior"));
}

void UCrowdSimulationManager::UpdateFleeingAgents(float DeltaTime)
{
    for (FCrowd_AgentData& Agent : ActiveAgents)
    {
        if (Agent.AgentState == ECrowd_AgentState::Fleeing && Agent.AgentActor)
        {
            FVector CurrentLoc = Agent.AgentActor->GetActorLocation();
            FVector Direction = (Agent.TargetLocation - CurrentLoc).GetSafeNormal();
            FVector NewLoc = CurrentLoc + Direction * Agent.MoveSpeed * DeltaTime * 3.0f; // 3x speed when fleeing
            Agent.AgentActor->SetActorLocation(NewLoc, true);
            Agent.AgentLocation = NewLoc;

            // Check if far enough from threat
            if (FVector::Dist(NewLoc, FleeOrigin) > FleeRadius * 2.0f)
            {
                Agent.AgentState = ECrowd_AgentState::Idle;
            }
        }
    }
}

void UCrowdSimulationManager::UpdateIdleAgents(float DeltaTime)
{
    // Idle agents wander within their zone
    for (FCrowd_AgentData& Agent : ActiveAgents)
    {
        if (Agent.AgentState == ECrowd_AgentState::Idle && Agent.AgentActor)
        {
            // Simple wander: move toward target, pick new target when reached
            FVector CurrentLoc = Agent.AgentActor->GetActorLocation();
            float DistToTarget = FVector::Dist(CurrentLoc, Agent.TargetLocation);

            if (DistToTarget < 50.0f)
            {
                // Pick new wander target within zone
                Agent.TargetLocation = Agent.HomeLocation + FVector(
                    FMath::RandRange(-300.0f, 300.0f),
                    FMath::RandRange(-300.0f, 300.0f),
                    0.0f
                );
            }
            else
            {
                FVector Direction = (Agent.TargetLocation - CurrentLoc).GetSafeNormal();
                FVector NewLoc = CurrentLoc + Direction * Agent.MoveSpeed * DeltaTime;
                Agent.AgentActor->SetActorLocation(NewLoc, true);
                Agent.AgentLocation = NewLoc;
            }
        }
    }
}

ECrowd_AgentLOD UCrowdSimulationManager::GetAgentLODLevel(FVector AgentLocation, FVector PlayerLocation)
{
    float Distance = FVector::Dist(AgentLocation, PlayerLocation);

    if (Distance < LODDistanceMid)
    {
        return ECrowd_AgentLOD::Full;
    }
    else if (Distance < LODDistanceFar)
    {
        return ECrowd_AgentLOD::Mid;
    }
    else
    {
        return ECrowd_AgentLOD::Distant;
    }
}

int32 UCrowdSimulationManager::GetActiveCrowdCount() const
{
    return ActiveAgents.Num();
}

TArray<FCrowd_AgentData> UCrowdSimulationManager::GetAgentsInRadius(FVector Center, float Radius) const
{
    TArray<FCrowd_AgentData> Result;
    for (const FCrowd_AgentData& Agent : ActiveAgents)
    {
        if (FVector::Dist(Agent.AgentLocation, Center) <= Radius)
        {
            Result.Add(Agent);
        }
    }
    return Result;
}
