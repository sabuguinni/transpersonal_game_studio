// ============================================================
// Agent #13 — Crowd & Traffic Simulation
// CrowdGroupConfig.cpp — Implementation of crowd group config
// ============================================================

#include "CrowdGroupConfig.h"
#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogCrowdGroup, Log, All);

UCrowdGroupConfig::UCrowdGroupConfig()
{
    Config.GroupID = NAME_None;
    Config.GroupType = ECrowd_GroupType::HerbivoreHerd;
    Config.GroupSize = 3;
    Config.PatrolRadius = 2000.0f;
    Config.FleeDistanceFromPlayer = 800.0f;
    Config.AggroDistanceFromPlayer = 1200.0f;
    Config.MoveSpeed = 300.0f;
    Config.SpreadRadius = 300.0f;
    Config.CurrentState = ECrowd_BehaviorState::Grazing;
    Config.bIsPlayerThreat = false;
    Config.TerritoryRadius = 3000.0f;
}

void UCrowdGroupConfig::SetGroupState(ECrowd_BehaviorState NewState)
{
    if (Config.CurrentState == NewState)
    {
        return;
    }

    UE_LOG(LogCrowdGroup, Log, TEXT("CrowdGroup [%s]: State %d -> %d"),
        *Config.GroupID.ToString(),
        (int32)Config.CurrentState,
        (int32)NewState);

    Config.CurrentState = NewState;

    // When fleeing, mark player as threat
    if (NewState == ECrowd_BehaviorState::Fleeing || NewState == ECrowd_BehaviorState::Hunting)
    {
        Config.bIsPlayerThreat = true;
    }
    else if (NewState == ECrowd_BehaviorState::Grazing || NewState == ECrowd_BehaviorState::Resting)
    {
        Config.bIsPlayerThreat = false;
    }
}

ECrowd_BehaviorState UCrowdGroupConfig::GetGroupState() const
{
    return Config.CurrentState;
}

bool UCrowdGroupConfig::IsPlayerThreat() const
{
    return Config.bIsPlayerThreat;
}

void UCrowdGroupConfig::AddWaypoint(FVector Location, float WaitTime, ECrowd_BehaviorState BehaviorAtPoint)
{
    FCrowd_WaypointData NewWaypoint;
    NewWaypoint.Location = Location;
    NewWaypoint.WaitTimeSeconds = FMath::Max(0.0f, WaitTime);
    NewWaypoint.BehaviorAtWaypoint = BehaviorAtPoint;
    Config.PatrolWaypoints.Add(NewWaypoint);

    UE_LOG(LogCrowdGroup, Verbose, TEXT("CrowdGroup [%s]: Added waypoint at %s (wait=%.1fs)"),
        *Config.GroupID.ToString(),
        *Location.ToString(),
        WaitTime);
}

int32 UCrowdGroupConfig::GetWaypointCount() const
{
    return Config.PatrolWaypoints.Num();
}
