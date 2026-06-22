// CrowdSimulationManager.cpp
// Agent #13 — Crowd & Traffic Simulation
// Implementation of prehistoric crowd simulation with tribe groups, flee events, LOD clusters

#include "CrowdSimulationManager.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"

ACrowdSimulationManager::ACrowdSimulationManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = TickInterval;
    MaxActiveAgents = 50000;
    MaxFullDetailAgents = 200;
    LODSwitchDistance = 2000.0f;
    FleeResponseRadius = 600.0f;
    CurrentActiveCount = 0;
}

void ACrowdSimulationManager::BeginPlay()
{
    Super::BeginPlay();

    // Register default waypoints matching MinPlayableMap layout
    RegisterWaypoint(FVector(0.0f, 0.0f, 80.0f),       FName("WP_Center"));
    RegisterWaypoint(FVector(800.0f, 0.0f, 80.0f),     FName("WP_East"));
    RegisterWaypoint(FVector(-800.0f, 0.0f, 80.0f),    FName("WP_West"));
    RegisterWaypoint(FVector(0.0f, 800.0f, 80.0f),     FName("WP_North"));
    RegisterWaypoint(FVector(0.0f, -800.0f, 80.0f),    FName("WP_South"));
    RegisterWaypoint(FVector(1500.0f, 800.0f, 80.0f),  FName("WP_TribeCamp"));

    // Connect waypoints (simple star topology from center)
    if (WaypointGraph.Num() >= 6)
    {
        WaypointGraph[0].ConnectedNodeIndices = {1, 2, 3, 4, 5};
        WaypointGraph[1].ConnectedNodeIndices = {0, 5};
        WaypointGraph[2].ConnectedNodeIndices = {0, 4};
        WaypointGraph[3].ConnectedNodeIndices = {0, 5};
        WaypointGraph[4].ConnectedNodeIndices = {0, 2};
        WaypointGraph[5].ConnectedNodeIndices = {0, 1, 3};
    }

    // Spawn default tribe at camp
    SpawnTribeGroup(FVector(1500.0f, 800.0f, 100.0f), 8, 1);

    // Register LOD clusters for distant herds
    FCrowd_LODCluster HerdA;
    HerdA.CenterLocation = FVector(3000.0f, 1500.0f, 100.0f);
    HerdA.AgentCount = 24;
    HerdA.ClusterRadius = 600.0f;
    HerdA.ClusterType = ECrowd_GroupType::HerdAnimal;
    LODClusters.Add(HerdA);

    FCrowd_LODCluster HerdB;
    HerdB.CenterLocation = FVector(-2500.0f, 2000.0f, 100.0f);
    HerdB.AgentCount = 18;
    HerdB.ClusterRadius = 500.0f;
    HerdB.ClusterType = ECrowd_GroupType::HerdAnimal;
    LODClusters.Add(HerdB);

    FCrowd_LODCluster Nomads;
    Nomads.CenterLocation = FVector(500.0f, -3000.0f, 100.0f);
    Nomads.AgentCount = 12;
    Nomads.ClusterRadius = 400.0f;
    Nomads.ClusterType = ECrowd_GroupType::NomadGroup;
    LODClusters.Add(Nomads);

    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Initialized with %d agents, %d waypoints, %d LOD clusters"),
        CurrentActiveCount, WaypointGraph.Num(), LODClusters.Num());
}

void ACrowdSimulationManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    TickAgents(DeltaTime);
}

void ACrowdSimulationManager::SpawnTribeGroup(FVector CampCenter, int32 MemberCount, int32 GroupID)
{
    const int32 Clamped = FMath::Clamp(MemberCount, 1, 32);
    const float Radius = 300.0f;

    for (int32 i = 0; i < Clamped; ++i)
    {
        if (ActiveAgents.Num() >= MaxActiveAgents)
        {
            UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: MaxActiveAgents (%d) reached"), MaxActiveAgents);
            break;
        }

        const float Angle = (float)i / (float)Clamped * 2.0f * PI;
        const float OffsetX = FMath::Cos(Angle) * Radius * FMath::FRandRange(0.3f, 1.0f);
        const float OffsetY = FMath::Sin(Angle) * Radius * FMath::FRandRange(0.3f, 1.0f);

        FCrowd_AgentData Agent;
        Agent.Location = CampCenter + FVector(OffsetX, OffsetY, 0.0f);
        Agent.State = (i == 0) ? ECrowd_AgentState::Working : ECrowd_AgentState::Wandering;
        Agent.GroupType = ECrowd_GroupType::TribeMember;
        Agent.MoveSpeed = FMath::FRandRange(80.0f, 160.0f);
        Agent.FleeRadius = FleeResponseRadius;
        Agent.GroupID = GroupID;

        ActiveAgents.Add(Agent);
        ++CurrentActiveCount;
    }

    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Spawned tribe group %d with %d members at (%.0f,%.0f)"),
        GroupID, Clamped, CampCenter.X, CampCenter.Y);
}

void ACrowdSimulationManager::TriggerFleeEvent(FVector ThreatLocation, float ThreatRadius)
{
    int32 AffectedCount = 0;

    for (FCrowd_AgentData& Agent : ActiveAgents)
    {
        const float Dist = FVector::Dist(Agent.Location, ThreatLocation);
        if (Dist <= ThreatRadius)
        {
            Agent.State = ECrowd_AgentState::Fleeing;
            ++AffectedCount;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: FleeEvent at (%.0f,%.0f) radius=%.0f affected %d agents"),
        ThreatLocation.X, ThreatLocation.Y, ThreatRadius, AffectedCount);
}

void ACrowdSimulationManager::RegisterWaypoint(FVector WorldPos, FName Label)
{
    FCrowd_WaypointNode Node;
    Node.WorldPosition = WorldPos;
    Node.NodeLabel = Label;
    Node.TrafficWeight = 1.0f;
    WaypointGraph.Add(Node);
}

void ACrowdSimulationManager::UpdateLODClusters(FVector PlayerLocation)
{
    for (FCrowd_LODCluster& Cluster : LODClusters)
    {
        const float Dist = FVector::Dist(Cluster.CenterLocation, PlayerLocation);
        Cluster.bIsVisible = (Dist <= LODSwitchDistance * 3.0f);
    }
}

int32 ACrowdSimulationManager::GetActiveAgentCount() const
{
    return CurrentActiveCount;
}

void ACrowdSimulationManager::SetAgentState(int32 AgentIndex, ECrowd_AgentState NewState)
{
    if (ActiveAgents.IsValidIndex(AgentIndex))
    {
        ActiveAgents[AgentIndex].State = NewState;
    }
}

FCrowd_AgentData ACrowdSimulationManager::GetAgentData(int32 AgentIndex) const
{
    if (ActiveAgents.IsValidIndex(AgentIndex))
    {
        return ActiveAgents[AgentIndex];
    }
    return FCrowd_AgentData();
}

// ─── Private ──────────────────────────────────────────────────────────────────

void ACrowdSimulationManager::TickAgents(float DeltaTime)
{
    // Only tick full-detail agents (first MaxFullDetailAgents)
    const int32 TickCount = FMath::Min(ActiveAgents.Num(), MaxFullDetailAgents);

    for (int32 i = 0; i < TickCount; ++i)
    {
        UpdateAgentMovement(ActiveAgents[i], DeltaTime);
    }
}

void ACrowdSimulationManager::UpdateAgentMovement(FCrowd_AgentData& Agent, float DeltaTime)
{
    switch (Agent.State)
    {
    case ECrowd_AgentState::Wandering:
    {
        // Simple random wander within local area
        const FVector Target = GetNextWaypointTarget(Agent);
        const FVector Dir = (Target - Agent.Location).GetSafeNormal();
        Agent.Location += Dir * Agent.MoveSpeed * DeltaTime;
        break;
    }
    case ECrowd_AgentState::Fleeing:
    {
        // Move away from group center (simplified flee)
        const FVector FleeDir = FVector(FMath::FRandRange(-1.0f, 1.0f), FMath::FRandRange(-1.0f, 1.0f), 0.0f).GetSafeNormal();
        Agent.Location += FleeDir * Agent.MoveSpeed * 2.0f * DeltaTime;

        // After fleeing, transition to wandering
        AccumulatedTime += DeltaTime;
        if (AccumulatedTime > 5.0f)
        {
            Agent.State = ECrowd_AgentState::Wandering;
        }
        break;
    }
    case ECrowd_AgentState::Idle:
    case ECrowd_AgentState::Working:
    case ECrowd_AgentState::Gathering:
    case ECrowd_AgentState::Sleeping:
    default:
        // Stationary states — no movement
        break;
    }
}

FVector ACrowdSimulationManager::GetNextWaypointTarget(const FCrowd_AgentData& Agent) const
{
    if (WaypointGraph.Num() == 0)
    {
        return Agent.Location;
    }

    // Find nearest waypoint
    int32 NearestIdx = 0;
    float NearestDist = MAX_FLT;
    for (int32 i = 0; i < WaypointGraph.Num(); ++i)
    {
        const float D = FVector::Dist(Agent.Location, WaypointGraph[i].WorldPosition);
        if (D < NearestDist)
        {
            NearestDist = D;
            NearestIdx = i;
        }
    }

    // Pick a connected node randomly
    const FCrowd_WaypointNode& Nearest = WaypointGraph[NearestIdx];
    if (Nearest.ConnectedNodeIndices.Num() > 0)
    {
        const int32 RandConn = FMath::RandRange(0, Nearest.ConnectedNodeIndices.Num() - 1);
        const int32 TargetIdx = Nearest.ConnectedNodeIndices[RandConn];
        if (WaypointGraph.IsValidIndex(TargetIdx))
        {
            return WaypointGraph[TargetIdx].WorldPosition;
        }
    }

    return WaypointGraph[NearestIdx].WorldPosition;
}

bool ACrowdSimulationManager::IsAgentInLODRange(const FCrowd_AgentData& Agent, const FVector& PlayerLoc) const
{
    return FVector::Dist(Agent.Location, PlayerLoc) <= LODSwitchDistance;
}
