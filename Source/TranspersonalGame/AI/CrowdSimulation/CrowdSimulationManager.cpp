#include "CrowdSimulationManager.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

ACrowdSimulationManager::ACrowdSimulationManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;

    MaxCrowdAgents = 50;
    CrowdUpdateInterval = 0.5f;
    ThreatDetectionRadius = 1500.0f;
    FleeRadius = 800.0f;

    ActiveAgentCount = 0;
    GlobalAlertLevel = 0.0f;
    bTribeInPanic = false;

    TimeSinceLastUpdate = 0.0f;
    bHasActiveThreat = false;
    LastKnownThreatLocation = FVector::ZeroVector;

    // Default waypoints for camp layout
    FCrowd_WaypointData CampCenter;
    CampCenter.Location = FVector(0, 0, 50);
    CampCenter.WaypointName = TEXT("Camp_Center");
    CampCenter.Radius = 300.0f;
    CampCenter.bIsDangerZone = false;
    Waypoints.Add(CampCenter);

    FCrowd_WaypointData HuntEast;
    HuntEast.Location = FVector(600, 0, 50);
    HuntEast.WaypointName = TEXT("Hunt_East");
    HuntEast.Radius = 200.0f;
    HuntEast.bIsDangerZone = false;
    Waypoints.Add(HuntEast);

    FCrowd_WaypointData GatherWest;
    GatherWest.Location = FVector(-600, 0, 50);
    GatherWest.WaypointName = TEXT("Gather_West");
    GatherWest.Radius = 200.0f;
    GatherWest.bIsDangerZone = false;
    Waypoints.Add(GatherWest);

    FCrowd_WaypointData WaterNorth;
    WaterNorth.Location = FVector(0, 600, 50);
    WaterNorth.WaypointName = TEXT("Water_North");
    WaterNorth.Radius = 150.0f;
    WaterNorth.bIsDangerZone = false;
    Waypoints.Add(WaterNorth);

    FCrowd_WaypointData LookoutSouth;
    LookoutSouth.Location = FVector(0, -600, 50);
    LookoutSouth.WaypointName = TEXT("Lookout_South");
    LookoutSouth.Radius = 150.0f;
    LookoutSouth.bIsDangerZone = false;
    Waypoints.Add(LookoutSouth);
}

void ACrowdSimulationManager::BeginPlay()
{
    Super::BeginPlay();
    ActiveAgentCount = AgentRegistry.Num();
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Started with %d agents, %d waypoints"), ActiveAgentCount, Waypoints.Num());
}

void ACrowdSimulationManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    TimeSinceLastUpdate += DeltaTime;
    if (TimeSinceLastUpdate >= CrowdUpdateInterval)
    {
        UpdateCrowdBehavior(TimeSinceLastUpdate);
        TimeSinceLastUpdate = 0.0f;
    }

    // Decay alert level over time
    if (GlobalAlertLevel > 0.0f && !bHasActiveThreat)
    {
        GlobalAlertLevel = FMath::Max(0.0f, GlobalAlertLevel - DeltaTime * 0.1f);
        if (GlobalAlertLevel <= 0.0f)
        {
            bTribeInPanic = false;
        }
    }
}

void ACrowdSimulationManager::RegisterAgent(FCrowd_AgentData AgentData)
{
    if (AgentRegistry.Num() >= MaxCrowdAgents)
    {
        UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Max agents reached (%d)"), MaxCrowdAgents);
        return;
    }
    AgentRegistry.Add(AgentData);
    ActiveAgentCount = AgentRegistry.Num();
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Agent registered. Total: %d"), ActiveAgentCount);
}

void ACrowdSimulationManager::TriggerThreatAlert(FVector ThreatLocation, float ThreatRadius)
{
    bHasActiveThreat = true;
    LastKnownThreatLocation = ThreatLocation;
    GlobalAlertLevel = FMath::Clamp(GlobalAlertLevel + 0.5f, 0.0f, 1.0f);

    if (GlobalAlertLevel >= 0.7f)
    {
        bTribeInPanic = true;
    }

    // Update all agents to fleeing state if within threat radius
    for (FCrowd_AgentData& Agent : AgentRegistry)
    {
        float DistToThreat = FVector::Dist(Agent.HomeLocation, ThreatLocation);
        if (DistToThreat <= ThreatRadius)
        {
            Agent.State = ECrowd_AgentState::Fleeing;
            Agent.AlertLevel = 1.0f;

            // Set flee target away from threat
            FVector FleeDir = (Agent.HomeLocation - ThreatLocation).GetSafeNormal();
            Agent.CurrentTarget = Agent.HomeLocation + FleeDir * FleeRadius;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: THREAT ALERT at %s — GlobalAlert=%.2f, Panic=%s"),
        *ThreatLocation.ToString(), GlobalAlertLevel, bTribeInPanic ? TEXT("YES") : TEXT("NO"));
}

void ACrowdSimulationManager::ClearThreatAlert()
{
    bHasActiveThreat = false;
    LastKnownThreatLocation = FVector::ZeroVector;

    // Return agents to normal state
    for (FCrowd_AgentData& Agent : AgentRegistry)
    {
        if (Agent.State == ECrowd_AgentState::Fleeing)
        {
            Agent.State = ECrowd_AgentState::Patrolling;
            Agent.AlertLevel = FMath::Max(0.0f, Agent.AlertLevel - 0.5f);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Threat cleared. Agents returning to patrol."));
}

void ACrowdSimulationManager::UpdateCrowdBehavior(float DeltaTime)
{
    for (FCrowd_AgentData& Agent : AgentRegistry)
    {
        switch (Agent.State)
        {
        case ECrowd_AgentState::Idle:
            // Idle agents occasionally start patrolling
            if (FMath::RandRange(0, 10) > 7)
            {
                Agent.State = ECrowd_AgentState::Patrolling;
                FCrowd_WaypointData NearWP = GetNearestWaypoint(Agent.HomeLocation);
                Agent.CurrentTarget = NearWP.Location;
            }
            break;

        case ECrowd_AgentState::Patrolling:
            // Check if reached target
            if (FVector::Dist(Agent.HomeLocation, Agent.CurrentTarget) < 100.0f)
            {
                Agent.State = ECrowd_AgentState::Working;
            }
            break;

        case ECrowd_AgentState::Working:
            // Work for a bit then return to idle
            if (FMath::RandRange(0, 10) > 8)
            {
                Agent.State = ECrowd_AgentState::Idle;
                Agent.CurrentTarget = Agent.HomeLocation;
            }
            break;

        case ECrowd_AgentState::Fleeing:
            // Keep fleeing while threat active
            if (!bHasActiveThreat)
            {
                Agent.State = ECrowd_AgentState::Patrolling;
            }
            break;

        case ECrowd_AgentState::Socializing:
        case ECrowd_AgentState::Sleeping:
        default:
            break;
        }
    }
}

FCrowd_WaypointData ACrowdSimulationManager::GetNearestWaypoint(FVector FromLocation) const
{
    FCrowd_WaypointData Nearest;
    float MinDist = MAX_FLT;

    for (const FCrowd_WaypointData& WP : Waypoints)
    {
        float Dist = FVector::Dist(FromLocation, WP.Location);
        if (Dist < MinDist)
        {
            MinDist = Dist;
            Nearest = WP;
        }
    }

    return Nearest;
}

int32 ACrowdSimulationManager::GetAgentCountByRole(ECrowd_AgentRole Role) const
{
    int32 Count = 0;
    for (const FCrowd_AgentData& Agent : AgentRegistry)
    {
        if (Agent.Role == Role)
        {
            Count++;
        }
    }
    return Count;
}

bool ACrowdSimulationManager::IsTribeInDanger() const
{
    return bTribeInPanic || GlobalAlertLevel >= 0.5f;
}

void ACrowdSimulationManager::DebugDrawCrowdState()
{
#if WITH_EDITOR
    UWorld* World = GetWorld();
    if (!World) return;

    // Draw waypoints
    for (const FCrowd_WaypointData& WP : Waypoints)
    {
        FColor WPColor = WP.bIsDangerZone ? FColor::Red : FColor::Green;
        DrawDebugSphere(World, WP.Location, WP.Radius, 12, WPColor, false, 5.0f);
        DrawDebugString(World, WP.Location + FVector(0, 0, 50), WP.WaypointName, nullptr, WPColor, 5.0f);
    }

    // Draw threat radius if active
    if (bHasActiveThreat)
    {
        DrawDebugSphere(World, LastKnownThreatLocation, ThreatDetectionRadius, 16, FColor::Orange, false, 5.0f);
    }

    UE_LOG(LogTemp, Log, TEXT("CrowdDebug: %d agents, AlertLevel=%.2f, Panic=%s"),
        ActiveAgentCount, GlobalAlertLevel, bTribeInPanic ? TEXT("YES") : TEXT("NO"));
#endif
}
