// CrowdSimulationManager.cpp
// Agent #13 — Crowd & Traffic Simulation
// Prehistoric tribe crowd simulation: hunters, gatherers, scouts, elders, children
// Uses UE5 Mass AI concepts adapted for prehistoric survival context

#include "CrowdSimulationManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "NavMesh/NavMeshBoundsVolume.h"

// ─────────────────────────────────────────────────────────────────────────────
// UCrowdSimulationManager — UObject lifecycle
// ─────────────────────────────────────────────────────────────────────────────

UCrowdSimulationManager::UCrowdSimulationManager()
{
    MaxCrowdAgents = 500;
    TickInterval = 0.1f;
    CampCenterLocation = FVector(0.f, 0.f, 100.f);
    DangerRadius = 2000.f;
    bCrowdInDangerState = false;
    CurrentThreatLevel = ECrowd_ThreatLevel::Safe;
    AccumulatedTime = 0.f;
}

void UCrowdSimulationManager::Initialize(UWorld* InWorld)
{
    World = InWorld;
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: World is null on Initialize"));
        return;
    }

    // Register default waypoints
    RegisterDefaultWaypoints();

    // Spawn initial tribe population
    SpawnInitialTribe();

    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Initialized with %d agents"), ActiveAgents.Num());
}

void UCrowdSimulationManager::Tick(float DeltaTime)
{
    if (!World) return;

    AccumulatedTime += DeltaTime;
    if (AccumulatedTime < TickInterval) return;
    AccumulatedTime = 0.f;

    // Update each agent
    for (FCrowd_AgentData& Agent : ActiveAgents)
    {
        UpdateAgentBehavior(Agent, TickInterval);
    }

    // Update threat assessment
    UpdateThreatLevel();
}

// ─────────────────────────────────────────────────────────────────────────────
// Agent Management
// ─────────────────────────────────────────────────────────────────────────────

void UCrowdSimulationManager::SpawnInitialTribe()
{
    if (!World) return;

    // Tribe composition: realistic prehistoric group
    // ~40% hunters, ~35% gatherers, ~10% scouts, ~10% children, ~5% elders
    TArray<TPair<ECrowd_AgentRole, int32>> Composition = {
        { ECrowd_AgentRole::Hunter,   8 },
        { ECrowd_AgentRole::Gatherer, 7 },
        { ECrowd_AgentRole::Scout,    2 },
        { ECrowd_AgentRole::Child,    2 },
        { ECrowd_AgentRole::Elder,    1 },
    };

    int32 AgentID = 0;
    for (auto& Pair : Composition)
    {
        for (int32 i = 0; i < Pair.Value; i++)
        {
            FCrowd_AgentData NewAgent;
            NewAgent.AgentID = AgentID++;
            NewAgent.Role = Pair.Key;
            NewAgent.State = ECrowd_AgentState::Idle;
            NewAgent.Health = 100.f;
            NewAgent.Stamina = 100.f;
            NewAgent.FearLevel = 0.f;

            // Scatter around camp center
            float Angle = FMath::RandRange(0.f, 360.f);
            float Radius = FMath::RandRange(100.f, 600.f);
            NewAgent.CurrentLocation = CampCenterLocation + FVector(
                FMath::Cos(FMath::DegreesToRadians(Angle)) * Radius,
                FMath::Sin(FMath::DegreesToRadians(Angle)) * Radius,
                0.f
            );
            NewAgent.TargetLocation = NewAgent.CurrentLocation;
            NewAgent.HomeLocation = CampCenterLocation;

            // Assign movement speed by role
            switch (Pair.Key)
            {
                case ECrowd_AgentRole::Hunter:   NewAgent.MoveSpeed = 350.f; break;
                case ECrowd_AgentRole::Gatherer: NewAgent.MoveSpeed = 250.f; break;
                case ECrowd_AgentRole::Scout:    NewAgent.MoveSpeed = 450.f; break;
                case ECrowd_AgentRole::Child:    NewAgent.MoveSpeed = 300.f; break;
                case ECrowd_AgentRole::Elder:    NewAgent.MoveSpeed = 180.f; break;
                default:                         NewAgent.MoveSpeed = 280.f; break;
            }

            ActiveAgents.Add(NewAgent);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Spawned %d tribe members"), ActiveAgents.Num());
}

void UCrowdSimulationManager::RegisterDefaultWaypoints()
{
    // Camp center
    FCrowd_Waypoint Camp;
    Camp.WaypointID = 0;
    Camp.Location = CampCenterLocation;
    Camp.WaypointType = ECrowd_WaypointType::Camp;
    Camp.bIsDangerous = false;
    Waypoints.Add(Camp);

    // Hunt zones (north and east)
    FCrowd_Waypoint HuntNorth;
    HuntNorth.WaypointID = 1;
    HuntNorth.Location = CampCenterLocation + FVector(0.f, 1500.f, 0.f);
    HuntNorth.WaypointType = ECrowd_WaypointType::HuntingGrounds;
    HuntNorth.bIsDangerous = true;
    Waypoints.Add(HuntNorth);

    FCrowd_Waypoint HuntEast;
    HuntEast.WaypointID = 2;
    HuntEast.Location = CampCenterLocation + FVector(1500.f, 0.f, 0.f);
    HuntEast.WaypointType = ECrowd_WaypointType::HuntingGrounds;
    HuntEast.bIsDangerous = true;
    Waypoints.Add(HuntEast);

    // Gather zones (west and south)
    FCrowd_Waypoint GatherWest;
    GatherWest.WaypointID = 3;
    GatherWest.Location = CampCenterLocation + FVector(-1500.f, 0.f, 0.f);
    GatherWest.WaypointType = ECrowd_WaypointType::GatheringArea;
    GatherWest.bIsDangerous = false;
    Waypoints.Add(GatherWest);

    FCrowd_Waypoint GatherSouth;
    GatherSouth.WaypointID = 4;
    GatherSouth.Location = CampCenterLocation + FVector(0.f, -1500.f, 0.f);
    GatherSouth.WaypointType = ECrowd_WaypointType::GatheringArea;
    GatherSouth.bIsDangerous = false;
    Waypoints.Add(GatherSouth);

    // Water source
    FCrowd_Waypoint Water;
    Water.WaypointID = 5;
    Water.Location = CampCenterLocation + FVector(-800.f, 800.f, 0.f);
    Water.WaypointType = ECrowd_WaypointType::WaterSource;
    Water.bIsDangerous = false;
    Waypoints.Add(Water);

    // Danger zone (near dinosaur territory)
    FCrowd_Waypoint Danger;
    Danger.WaypointID = 6;
    Danger.Location = FVector(2000.f, 2000.f, 100.f);
    Danger.WaypointType = ECrowd_WaypointType::DangerZone;
    Danger.bIsDangerous = true;
    Waypoints.Add(Danger);

    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Registered %d waypoints"), Waypoints.Num());
}

// ─────────────────────────────────────────────────────────────────────────────
// Agent Behavior Update
// ─────────────────────────────────────────────────────────────────────────────

void UCrowdSimulationManager::UpdateAgentBehavior(FCrowd_AgentData& Agent, float DeltaTime)
{
    // Fear propagation: if in danger state, all agents flee
    if (bCrowdInDangerState)
    {
        Agent.FearLevel = FMath::Min(Agent.FearLevel + DeltaTime * 2.f, 10.f);
        Agent.State = ECrowd_AgentState::Fleeing;
        Agent.TargetLocation = CampCenterLocation;
        return;
    }

    // Stamina recovery when idle
    if (Agent.State == ECrowd_AgentState::Idle)
    {
        Agent.Stamina = FMath::Min(Agent.Stamina + DeltaTime * 5.f, 100.f);
        Agent.FearLevel = FMath::Max(Agent.FearLevel - DeltaTime * 1.f, 0.f);
    }

    // Role-based behavior state machine
    switch (Agent.Role)
    {
        case ECrowd_AgentRole::Hunter:
            UpdateHunterBehavior(Agent, DeltaTime);
            break;
        case ECrowd_AgentRole::Gatherer:
            UpdateGathererBehavior(Agent, DeltaTime);
            break;
        case ECrowd_AgentRole::Scout:
            UpdateScoutBehavior(Agent, DeltaTime);
            break;
        case ECrowd_AgentRole::Child:
            UpdateChildBehavior(Agent, DeltaTime);
            break;
        case ECrowd_AgentRole::Elder:
            UpdateElderBehavior(Agent, DeltaTime);
            break;
        default:
            break;
    }

    // Move agent toward target
    FVector Direction = Agent.TargetLocation - Agent.CurrentLocation;
    float Distance = Direction.Size();
    if (Distance > 50.f)
    {
        Direction.Normalize();
        Agent.CurrentLocation += Direction * Agent.MoveSpeed * DeltaTime;
    }
    else
    {
        // Reached target — go idle
        Agent.State = ECrowd_AgentState::Idle;
    }
}

void UCrowdSimulationManager::UpdateHunterBehavior(FCrowd_AgentData& Agent, float DeltaTime)
{
    if (Agent.State == ECrowd_AgentState::Idle && Agent.Stamina > 50.f)
    {
        // Pick a hunting waypoint
        FCrowd_Waypoint* HuntWP = FindWaypointByType(ECrowd_WaypointType::HuntingGrounds);
        if (HuntWP)
        {
            Agent.TargetLocation = HuntWP->Location + FVector(
                FMath::RandRange(-200.f, 200.f),
                FMath::RandRange(-200.f, 200.f),
                0.f
            );
            Agent.State = ECrowd_AgentState::Hunting;
        }
    }
    else if (Agent.Stamina < 20.f)
    {
        // Return to camp to rest
        Agent.TargetLocation = CampCenterLocation + FVector(
            FMath::RandRange(-150.f, 150.f),
            FMath::RandRange(-150.f, 150.f),
            0.f
        );
        Agent.State = ECrowd_AgentState::Resting;
    }
}

void UCrowdSimulationManager::UpdateGathererBehavior(FCrowd_AgentData& Agent, float DeltaTime)
{
    if (Agent.State == ECrowd_AgentState::Idle && Agent.Stamina > 30.f)
    {
        FCrowd_Waypoint* GatherWP = FindWaypointByType(ECrowd_WaypointType::GatheringArea);
        if (GatherWP)
        {
            Agent.TargetLocation = GatherWP->Location + FVector(
                FMath::RandRange(-300.f, 300.f),
                FMath::RandRange(-300.f, 300.f),
                0.f
            );
            Agent.State = ECrowd_AgentState::Gathering;
        }
    }
}

void UCrowdSimulationManager::UpdateScoutBehavior(FCrowd_AgentData& Agent, float DeltaTime)
{
    // Scouts patrol between waypoints
    if (Agent.State == ECrowd_AgentState::Idle)
    {
        int32 RandomWP = FMath::RandRange(0, Waypoints.Num() - 1);
        if (Waypoints.IsValidIndex(RandomWP) && !Waypoints[RandomWP].bIsDangerous)
        {
            Agent.TargetLocation = Waypoints[RandomWP].Location;
            Agent.State = ECrowd_AgentState::Patrolling;
        }
    }
}

void UCrowdSimulationManager::UpdateChildBehavior(FCrowd_AgentData& Agent, float DeltaTime)
{
    // Children stay near camp, play randomly
    if (Agent.State == ECrowd_AgentState::Idle)
    {
        Agent.TargetLocation = CampCenterLocation + FVector(
            FMath::RandRange(-300.f, 300.f),
            FMath::RandRange(-300.f, 300.f),
            0.f
        );
        Agent.State = ECrowd_AgentState::Playing;
    }
}

void UCrowdSimulationManager::UpdateElderBehavior(FCrowd_AgentData& Agent, float DeltaTime)
{
    // Elders stay near campfire, move slowly
    if (Agent.State == ECrowd_AgentState::Idle)
    {
        Agent.TargetLocation = CampCenterLocation + FVector(
            FMath::RandRange(-80.f, 80.f),
            FMath::RandRange(-80.f, 80.f),
            0.f
        );
        Agent.State = ECrowd_AgentState::Resting;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Threat System
// ─────────────────────────────────────────────────────────────────────────────

void UCrowdSimulationManager::UpdateThreatLevel()
{
    if (!World) return;

    // Check if any dinosaur actor is within danger radius of camp
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);

    bool bDinoNearby = false;
    for (AActor* Actor : AllActors)
    {
        if (!Actor) continue;
        FString Label = Actor->GetActorLabel();
        bool bIsDino = Label.Contains(TEXT("TRex")) || Label.Contains(TEXT("Raptor")) ||
                       Label.Contains(TEXT("Trike")) || Label.Contains(TEXT("Brachio")) ||
                       Label.Contains(TEXT("Dino"));
        if (bIsDino)
        {
            float Dist = FVector::Dist(Actor->GetActorLocation(), CampCenterLocation);
            if (Dist < DangerRadius)
            {
                bDinoNearby = true;
                break;
            }
        }
    }

    if (bDinoNearby)
    {
        CurrentThreatLevel = ECrowd_ThreatLevel::Critical;
        bCrowdInDangerState = true;
    }
    else
    {
        CurrentThreatLevel = ECrowd_ThreatLevel::Safe;
        bCrowdInDangerState = false;
    }
}

void UCrowdSimulationManager::TriggerDangerAlert(FVector ThreatLocation)
{
    bCrowdInDangerState = true;
    CurrentThreatLevel = ECrowd_ThreatLevel::Critical;

    // All agents flee toward camp
    for (FCrowd_AgentData& Agent : ActiveAgents)
    {
        Agent.State = ECrowd_AgentState::Fleeing;
        Agent.FearLevel = 10.f;
        Agent.TargetLocation = CampCenterLocation + FVector(
            FMath::RandRange(-200.f, 200.f),
            FMath::RandRange(-200.f, 200.f),
            0.f
        );
    }

    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: DANGER ALERT at %s — %d agents fleeing"),
        *ThreatLocation.ToString(), ActiveAgents.Num());
}

void UCrowdSimulationManager::ClearDangerAlert()
{
    bCrowdInDangerState = false;
    CurrentThreatLevel = ECrowd_ThreatLevel::Safe;

    for (FCrowd_AgentData& Agent : ActiveAgents)
    {
        Agent.FearLevel = FMath::Max(Agent.FearLevel - 3.f, 0.f);
        Agent.State = ECrowd_AgentState::Idle;
    }

    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Danger cleared — crowd returning to normal"));
}

// ─────────────────────────────────────────────────────────────────────────────
// Utility
// ─────────────────────────────────────────────────────────────────────────────

FCrowd_Waypoint* UCrowdSimulationManager::FindWaypointByType(ECrowd_WaypointType Type)
{
    TArray<FCrowd_Waypoint*> Matching;
    for (FCrowd_Waypoint& WP : Waypoints)
    {
        if (WP.WaypointType == Type && !WP.bIsDangerous)
        {
            Matching.Add(&WP);
        }
    }
    if (Matching.Num() == 0) return nullptr;
    return Matching[FMath::RandRange(0, Matching.Num() - 1)];
}

int32 UCrowdSimulationManager::GetAgentCountByRole(ECrowd_AgentRole Role) const
{
    int32 Count = 0;
    for (const FCrowd_AgentData& Agent : ActiveAgents)
    {
        if (Agent.Role == Role) Count++;
    }
    return Count;
}

FCrowd_TribeStats UCrowdSimulationManager::GetTribeStats() const
{
    FCrowd_TribeStats Stats;
    Stats.TotalAgents = ActiveAgents.Num();
    Stats.HunterCount = GetAgentCountByRole(ECrowd_AgentRole::Hunter);
    Stats.GathererCount = GetAgentCountByRole(ECrowd_AgentRole::Gatherer);
    Stats.ScoutCount = GetAgentCountByRole(ECrowd_AgentRole::Scout);
    Stats.ChildCount = GetAgentCountByRole(ECrowd_AgentRole::Child);
    Stats.ElderCount = GetAgentCountByRole(ECrowd_AgentRole::Elder);
    Stats.bInDanger = bCrowdInDangerState;
    Stats.ThreatLevel = CurrentThreatLevel;

    float TotalFear = 0.f;
    for (const FCrowd_AgentData& Agent : ActiveAgents)
    {
        TotalFear += Agent.FearLevel;
    }
    Stats.AverageFearLevel = (ActiveAgents.Num() > 0) ? (TotalFear / ActiveAgents.Num()) : 0.f;

    return Stats;
}
