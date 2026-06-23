// CrowdSimulationManager.cpp
// Crowd & Traffic Simulation Agent #13
// Implements herd migration, territory systems, and crowd AI for prehistoric fauna

#include "CrowdSimulationManager.h"
#include "Engine/World.h"
#include "Engine/TargetPoint.h"
#include "NavigationSystem.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UCrowdSimulationManager::UCrowdSimulationManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f; // Update every 500ms for performance

    MaxActiveAgents = 50;
    HerdUpdateRadius = 5000.0f;
    MigrationSpeed = 200.0f;
    TerritoryRadius = 3000.0f;
    bEnableCrowdDebug = false;
}

void UCrowdSimulationManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeCrowdSystems();
}

void UCrowdSimulationManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    UpdateHerdBehaviors(DeltaTime);
    UpdateTerritoryStates(DeltaTime);
}

void UCrowdSimulationManager::InitializeCrowdSystems()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Gather all waypoints tagged for herd migration
    TArray<AActor*> WaypointActors;
    UGameplayStatics::GetAllActorsOfClass(World, ATargetPoint::StaticClass(), WaypointActors);

    for (AActor* WP : WaypointActors)
    {
        if (!WP) continue;
        FString Label = WP->GetActorLabel();

        FCrowd_HerdWaypoint NewWP;
        NewWP.Location = WP->GetActorLocation();
        NewWP.WaypointLabel = Label;
        NewWP.bIsActive = true;

        if (Label.Contains(TEXT("Raptor")))
        {
            NewWP.AssignedHerdType = ECrowd_HerdType::RaptorPack;
            RaptorPatrolWaypoints.Add(NewWP);
        }
        else if (Label.Contains(TEXT("Brach")))
        {
            NewWP.AssignedHerdType = ECrowd_HerdType::BrachiosaurusHerd;
            BrachMigrationWaypoints.Add(NewWP);
        }
        else if (Label.Contains(TEXT("TRex")) || Label.Contains(TEXT("Territory")))
        {
            NewWP.AssignedHerdType = ECrowd_HerdType::TRexSolitary;
            TRexTerritoryMarkers.Add(NewWP);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("CrowdSimulation: Initialized — Raptor WPs: %d, Brach WPs: %d, TRex Markers: %d"),
        RaptorPatrolWaypoints.Num(), BrachMigrationWaypoints.Num(), TRexTerritoryMarkers.Num());

    ActiveAgentCount = 0;
    bSystemInitialized = true;
}

void UCrowdSimulationManager::UpdateHerdBehaviors(float DeltaTime)
{
    if (!bSystemInitialized) return;

    // Tick herd state machines
    for (FCrowd_HerdAgent& Agent : ActiveAgents)
    {
        UpdateSingleAgent(Agent, DeltaTime);
    }
}

void UCrowdSimulationManager::UpdateSingleAgent(FCrowd_HerdAgent& Agent, float DeltaTime)
{
    Agent.TimeSinceLastDecision += DeltaTime;

    // Decision interval: raptors decide every 2s, brachiosaurs every 5s
    float DecisionInterval = (Agent.HerdType == ECrowd_HerdType::RaptorPack) ? 2.0f : 5.0f;

    if (Agent.TimeSinceLastDecision >= DecisionInterval)
    {
        Agent.TimeSinceLastDecision = 0.0f;

        switch (Agent.CurrentState)
        {
            case ECrowd_AgentState::Idle:
                // 70% chance to start patrolling
                if (FMath::RandRange(0, 100) < 70)
                    Agent.CurrentState = ECrowd_AgentState::Patrolling;
                break;

            case ECrowd_AgentState::Patrolling:
                // Advance to next waypoint
                Agent.CurrentWaypointIndex = (Agent.CurrentWaypointIndex + 1) % FMath::Max(1, Agent.TotalWaypoints);
                break;

            case ECrowd_AgentState::Fleeing:
                // After 10s of fleeing, return to patrol
                if (Agent.FleeTimer >= 10.0f)
                {
                    Agent.CurrentState = ECrowd_AgentState::Patrolling;
                    Agent.FleeTimer = 0.0f;
                }
                Agent.FleeTimer += DeltaTime;
                break;

            case ECrowd_AgentState::Hunting:
                // Raptors hunt for 8s then reassess
                if (Agent.HuntTimer >= 8.0f)
                {
                    Agent.CurrentState = ECrowd_AgentState::Patrolling;
                    Agent.HuntTimer = 0.0f;
                }
                Agent.HuntTimer += DeltaTime;
                break;

            default:
                break;
        }
    }
}

void UCrowdSimulationManager::UpdateTerritoryStates(float DeltaTime)
{
    // Territory overlap detection — if player enters TRex territory, escalate threat
    UWorld* World = GetWorld();
    if (!World) return;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn) return;

    FVector PlayerLoc = PlayerPawn->GetActorLocation();

    for (const FCrowd_HerdWaypoint& Marker : TRexTerritoryMarkers)
    {
        float Dist = FVector::Dist(PlayerLoc, Marker.Location);
        if (Dist < TerritoryRadius)
        {
            // Player is inside TRex territory
            if (!bPlayerInTRexTerritory)
            {
                bPlayerInTRexTerritory = true;
                OnPlayerEnterTerritoryDelegate.Broadcast(ECrowd_HerdType::TRexSolitary, PlayerLoc);
                UE_LOG(LogTemp, Warning, TEXT("CrowdSim: Player entered TRex territory at %.0f,%.0f"),
                    PlayerLoc.X, PlayerLoc.Y);
            }
            return;
        }
    }
    bPlayerInTRexTerritory = false;
}

void UCrowdSimulationManager::RegisterHerdAgent(AActor* AgentActor, ECrowd_HerdType HerdType)
{
    if (!AgentActor) return;

    FCrowd_HerdAgent NewAgent;
    NewAgent.AgentActor = AgentActor;
    NewAgent.HerdType = HerdType;
    NewAgent.CurrentState = ECrowd_AgentState::Idle;
    NewAgent.CurrentWaypointIndex = 0;
    NewAgent.TimeSinceLastDecision = FMath::RandRange(0.0f, 2.0f); // Stagger decisions
    NewAgent.FleeTimer = 0.0f;
    NewAgent.HuntTimer = 0.0f;

    // Assign waypoint count based on herd type
    if (HerdType == ECrowd_HerdType::RaptorPack)
        NewAgent.TotalWaypoints = RaptorPatrolWaypoints.Num();
    else if (HerdType == ECrowd_HerdType::BrachiosaurusHerd)
        NewAgent.TotalWaypoints = BrachMigrationWaypoints.Num();
    else
        NewAgent.TotalWaypoints = TRexTerritoryMarkers.Num();

    ActiveAgents.Add(NewAgent);
    ActiveAgentCount = ActiveAgents.Num();

    UE_LOG(LogTemp, Log, TEXT("CrowdSim: Registered agent %s as HerdType %d. Total: %d"),
        *AgentActor->GetActorLabel(), (int32)HerdType, ActiveAgentCount);
}

void UCrowdSimulationManager::TriggerHerdFlee(ECrowd_HerdType HerdType, FVector ThreatLocation)
{
    for (FCrowd_HerdAgent& Agent : ActiveAgents)
    {
        if (Agent.HerdType == HerdType)
        {
            Agent.CurrentState = ECrowd_AgentState::Fleeing;
            Agent.FleeTimer = 0.0f;
            Agent.ThreatLocation = ThreatLocation;
        }
    }
    UE_LOG(LogTemp, Log, TEXT("CrowdSim: Triggered flee for HerdType %d from threat at %.0f,%.0f"),
        (int32)HerdType, ThreatLocation.X, ThreatLocation.Y);
}

void UCrowdSimulationManager::TriggerRaptorHunt(FVector TargetLocation)
{
    for (FCrowd_HerdAgent& Agent : ActiveAgents)
    {
        if (Agent.HerdType == ECrowd_HerdType::RaptorPack)
        {
            Agent.CurrentState = ECrowd_AgentState::Hunting;
            Agent.HuntTimer = 0.0f;
            Agent.TargetLocation = TargetLocation;
        }
    }
    UE_LOG(LogTemp, Log, TEXT("CrowdSim: Raptor pack hunting target at %.0f,%.0f"),
        TargetLocation.X, TargetLocation.Y);
}

int32 UCrowdSimulationManager::GetActiveAgentCount() const
{
    return ActiveAgentCount;
}

bool UCrowdSimulationManager::IsPlayerInDangerZone() const
{
    return bPlayerInTRexTerritory;
}
