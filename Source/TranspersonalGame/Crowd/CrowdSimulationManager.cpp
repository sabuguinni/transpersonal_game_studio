// CrowdSimulationManager.cpp
// Agent #13 — Crowd & Traffic Simulation
// Prehistoric tribe crowd simulation using Mass AI patterns

#include "CrowdSimulationManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "NavigationSystem.h"
#include "NavMesh/NavMeshBoundsVolume.h"

UCrowdSimulationManager::UCrowdSimulationManager()
{
    MaxCrowdAgents = 200;
    AgentUpdateRadius = 5000.0f;
    LODDistanceClose = 1500.0f;
    LODDistanceMedium = 4000.0f;
    bCrowdSystemActive = false;
    TickInterval = 0.1f;
}

void UCrowdSimulationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    bCrowdSystemActive = true;

    // Register default tribe waypoints
    FCrowd_Waypoint WaterSource;
    WaterSource.WaypointID = 0;
    WaterSource.WorldLocation = FVector(800.f, 200.f, 50.f);
    WaterSource.WaypointType = ECrowd_WaypointType::WaterSource;
    WaterSource.MaxOccupants = 10;
    WaterSource.CurrentOccupants = 0;
    RegisteredWaypoints.Add(WaterSource);

    FCrowd_Waypoint HuntingGround;
    HuntingGround.WaypointID = 1;
    HuntingGround.WorldLocation = FVector(-600.f, 900.f, 80.f);
    HuntingGround.WaypointType = ECrowd_WaypointType::HuntingGround;
    HuntingGround.MaxOccupants = 20;
    HuntingGround.CurrentOccupants = 0;
    RegisteredWaypoints.Add(HuntingGround);

    FCrowd_Waypoint CampFire;
    CampFire.WaypointID = 2;
    CampFire.WorldLocation = FVector(0.f, 0.f, 50.f);
    CampFire.WaypointType = ECrowd_WaypointType::CampFire;
    CampFire.MaxOccupants = 30;
    CampFire.CurrentOccupants = 0;
    RegisteredWaypoints.Add(CampFire);

    FCrowd_Waypoint ForestEdge;
    ForestEdge.WaypointID = 3;
    ForestEdge.WorldLocation = FVector(1200.f, -400.f, 60.f);
    ForestEdge.WaypointType = ECrowd_WaypointType::ForestEdge;
    ForestEdge.MaxOccupants = 15;
    ForestEdge.CurrentOccupants = 0;
    RegisteredWaypoints.Add(ForestEdge);

    FCrowd_Waypoint CaveEntrance;
    CaveEntrance.WaypointID = 4;
    CaveEntrance.WorldLocation = FVector(-900.f, -700.f, 100.f);
    CaveEntrance.WaypointType = ECrowd_WaypointType::CaveEntrance;
    CaveEntrance.MaxOccupants = 8;
    CaveEntrance.CurrentOccupants = 0;
    RegisteredWaypoints.Add(CaveEntrance);

    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager initialized with %d waypoints"), RegisteredWaypoints.Num());
}

void UCrowdSimulationManager::Deinitialize()
{
    bCrowdSystemActive = false;
    ActiveAgents.Empty();
    RegisteredWaypoints.Empty();
    Super::Deinitialize();
}

void UCrowdSimulationManager::RegisterCrowdAgent(const FCrowd_AgentData& AgentData)
{
    if (ActiveAgents.Num() >= MaxCrowdAgents)
    {
        UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Max agent cap (%d) reached"), MaxCrowdAgents);
        return;
    }
    ActiveAgents.Add(AgentData);
}

void UCrowdSimulationManager::UnregisterCrowdAgent(int32 AgentID)
{
    ActiveAgents.RemoveAll([AgentID](const FCrowd_AgentData& Agent)
    {
        return Agent.AgentID == AgentID;
    });
}

FCrowd_Waypoint* UCrowdSimulationManager::FindNearestWaypoint(const FVector& FromLocation, ECrowd_WaypointType WaypointType)
{
    FCrowd_Waypoint* Nearest = nullptr;
    float BestDist = MAX_FLT;

    for (FCrowd_Waypoint& WP : RegisteredWaypoints)
    {
        if (WP.WaypointType != WaypointType)
            continue;
        if (WP.CurrentOccupants >= WP.MaxOccupants)
            continue;

        float Dist = FVector::Dist(FromLocation, WP.WorldLocation);
        if (Dist < BestDist)
        {
            BestDist = Dist;
            Nearest = &WP;
        }
    }
    return Nearest;
}

ECrowd_LODLevel UCrowdSimulationManager::GetAgentLOD(const FVector& AgentLocation, const FVector& PlayerLocation) const
{
    float Dist = FVector::Dist(AgentLocation, PlayerLocation);
    if (Dist <= LODDistanceClose)
        return ECrowd_LODLevel::Full;
    if (Dist <= LODDistanceMedium)
        return ECrowd_LODLevel::Medium;
    return ECrowd_LODLevel::Minimal;
}

void UCrowdSimulationManager::TriggerFleeResponse(const FVector& ThreatLocation, float ThreatRadius)
{
    int32 Fleeing = 0;
    for (FCrowd_AgentData& Agent : ActiveAgents)
    {
        float Dist = FVector::Dist(Agent.CurrentLocation, ThreatLocation);
        if (Dist <= ThreatRadius)
        {
            Agent.CurrentBehavior = ECrowd_AgentBehavior::Fleeing;
            Fleeing++;
        }
    }
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: FleeResponse triggered — %d agents fleeing from threat at (%.0f,%.0f,%.0f)"),
        Fleeing, ThreatLocation.X, ThreatLocation.Y, ThreatLocation.Z);
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
            Result.Add(Agent);
    }
    return Result;
}
