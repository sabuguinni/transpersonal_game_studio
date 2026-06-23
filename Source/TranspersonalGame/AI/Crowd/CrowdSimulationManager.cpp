// CrowdSimulationManager.cpp
// Agent #13 — Crowd & Traffic Simulation
// Implements prehistoric crowd simulation: herd migration, tribe camps, raptor pack patrols

#include "CrowdSimulationManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "NavigationSystem.h"
#include "DrawDebugHelpers.h"

UCrowdSimulationManager::UCrowdSimulationManager()
{
    MaxHerdAgents = 50;
    MaxTribeMembers = 20;
    MaxRaptorPackSize = 8;
    HerdMigrationSpeed = 150.0f;
    TribeMemberWanderRadius = 300.0f;
    RaptorPatrolSpeed = 250.0f;
    bHerdMigrationActive = false;
    bTribeCampActive = false;
    bRaptorPackActive = false;
}

void UCrowdSimulationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager initialized — prehistoric crowd systems online"));
}

void UCrowdSimulationManager::Deinitialize()
{
    HerdWaypoints.Empty();
    TribeMemberLocations.Empty();
    RaptorPatrolNodes.Empty();
    Super::Deinitialize();
}

void UCrowdSimulationManager::RegisterHerdWaypoint(const FVector& WaypointLocation)
{
    if (HerdWaypoints.Num() < 16)
    {
        HerdWaypoints.Add(WaypointLocation);
        UE_LOG(LogTemp, Log, TEXT("CrowdSim: Herd waypoint registered at (%.0f, %.0f, %.0f)"),
            WaypointLocation.X, WaypointLocation.Y, WaypointLocation.Z);
    }
}

void UCrowdSimulationManager::RegisterTribeMemberLocation(const FVector& Location)
{
    if (TribeMemberLocations.Num() < MaxTribeMembers)
    {
        TribeMemberLocations.Add(Location);
    }
}

void UCrowdSimulationManager::RegisterRaptorPatrolNode(const FVector& NodeLocation)
{
    if (RaptorPatrolNodes.Num() < 16)
    {
        RaptorPatrolNodes.Add(NodeLocation);
    }
}

void UCrowdSimulationManager::StartHerdMigration()
{
    if (HerdWaypoints.Num() < 2)
    {
        UE_LOG(LogTemp, Warning, TEXT("CrowdSim: Cannot start herd migration — need at least 2 waypoints"));
        return;
    }
    bHerdMigrationActive = true;
    CurrentHerdWaypointIndex = 0;
    UE_LOG(LogTemp, Log, TEXT("CrowdSim: Herd migration started — %d waypoints, %d agents"),
        HerdWaypoints.Num(), MaxHerdAgents);
}

void UCrowdSimulationManager::StopHerdMigration()
{
    bHerdMigrationActive = false;
    UE_LOG(LogTemp, Log, TEXT("CrowdSim: Herd migration stopped"));
}

void UCrowdSimulationManager::ActivateTribeCamp()
{
    if (TribeMemberLocations.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("CrowdSim: No tribe member locations registered"));
        return;
    }
    bTribeCampActive = true;
    UE_LOG(LogTemp, Log, TEXT("CrowdSim: Tribe camp activated — %d members"), TribeMemberLocations.Num());
}

void UCrowdSimulationManager::ActivateRaptorPack()
{
    if (RaptorPatrolNodes.Num() < 2)
    {
        UE_LOG(LogTemp, Warning, TEXT("CrowdSim: Cannot activate raptor pack — need at least 2 patrol nodes"));
        return;
    }
    bRaptorPackActive = true;
    CurrentPatrolNodeIndex = 0;
    UE_LOG(LogTemp, Log, TEXT("CrowdSim: Raptor pack patrol activated — %d nodes, pack size %d"),
        RaptorPatrolNodes.Num(), MaxRaptorPackSize);
}

FVector UCrowdSimulationManager::GetNextHerdWaypoint() const
{
    if (HerdWaypoints.Num() == 0)
    {
        return FVector::ZeroVector;
    }
    int32 SafeIndex = CurrentHerdWaypointIndex % HerdWaypoints.Num();
    return HerdWaypoints[SafeIndex];
}

FVector UCrowdSimulationManager::GetNextPatrolNode() const
{
    if (RaptorPatrolNodes.Num() == 0)
    {
        return FVector::ZeroVector;
    }
    int32 SafeIndex = CurrentPatrolNodeIndex % RaptorPatrolNodes.Num();
    return RaptorPatrolNodes[SafeIndex];
}

int32 UCrowdSimulationManager::GetActiveCrowdAgentCount() const
{
    int32 Total = 0;
    if (bHerdMigrationActive) Total += MaxHerdAgents;
    if (bTribeCampActive) Total += TribeMemberLocations.Num();
    if (bRaptorPackActive) Total += MaxRaptorPackSize;
    return Total;
}

ECrowd_LODLevel UCrowdSimulationManager::GetLODLevelForDistance(float DistanceFromPlayer) const
{
    // LOD chain: Full detail < 1500u, Medium < 4000u, Low < 8000u, Culled beyond
    if (DistanceFromPlayer < 1500.0f)  return ECrowd_LODLevel::Full;
    if (DistanceFromPlayer < 4000.0f)  return ECrowd_LODLevel::Medium;
    if (DistanceFromPlayer < 8000.0f)  return ECrowd_LODLevel::Low;
    return ECrowd_LODLevel::Culled;
}

void UCrowdSimulationManager::TickHerdMigration(float DeltaTime)
{
    if (!bHerdMigrationActive || HerdWaypoints.Num() < 2) return;
    // Advance waypoint index based on simulated progress
    // In full implementation: iterate over herd agents, move toward current waypoint,
    // advance index when all agents reach threshold distance
    // Stub: log progress
    UE_LOG(LogTemp, Verbose, TEXT("CrowdSim: Herd tick — waypoint %d/%d"),
        CurrentHerdWaypointIndex + 1, HerdWaypoints.Num());
}

void UCrowdSimulationManager::TickTribeCamp(float DeltaTime)
{
    if (!bTribeCampActive) return;
    // In full implementation: wander behaviour, idle animations, reaction to player proximity
    // Stub: no-op tick
}

void UCrowdSimulationManager::TickRaptorPack(float DeltaTime)
{
    if (!bRaptorPackActive || RaptorPatrolNodes.Num() < 2) return;
    // In full implementation: coordinated flanking, pack leader + followers, attack triggers
    // Stub: log patrol state
    UE_LOG(LogTemp, Verbose, TEXT("CrowdSim: Raptor pack tick — node %d/%d"),
        CurrentPatrolNodeIndex + 1, RaptorPatrolNodes.Num());
}
