// CrowdSimulationManager.cpp
// Crowd & Traffic Simulation Agent #13
// Implements prehistoric crowd simulation: tribal NPCs, dinosaur herds, migration paths

#include "CrowdSimulationManager.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Math/UnrealMathUtility.h"

ACrowdSimulationManager::ACrowdSimulationManager()
{
    PrimaryActorTick.bCanEverTick = true;

    MaxAgents = 200;
    ThreatDetectionRadius = 1500.0f;
    AgentSeparationDistance = 150.0f;
    CohesionStrength = 0.5f;
    ActiveAgentCount = 0;

    // Default migration corridor (matches waypoints placed in MinPlayableMap)
    FCrowd_MigrationWaypoint WP0, WP1, WP2, WP3, WP4;
    WP0.WorldPosition = FVector(-2000, -1500, 100); WP0.WaypointIndex = 0; WP0.Radius = 300.0f;
    WP1.WorldPosition = FVector(-1000, -800, 100);  WP1.WaypointIndex = 1; WP1.Radius = 300.0f;
    WP2.WorldPosition = FVector(0, 0, 100);          WP2.WaypointIndex = 2; WP2.Radius = 300.0f;
    WP3.WorldPosition = FVector(1000, 800, 100);    WP3.WaypointIndex = 3; WP3.Radius = 300.0f;
    WP4.WorldPosition = FVector(2000, 1500, 100);   WP4.WaypointIndex = 4; WP4.Radius = 300.0f;
    MigrationWaypoints.Add(WP0);
    MigrationWaypoints.Add(WP1);
    MigrationWaypoints.Add(WP2);
    MigrationWaypoints.Add(WP3);
    MigrationWaypoints.Add(WP4);
}

void ACrowdSimulationManager::BeginPlay()
{
    Super::BeginPlay();
    ActiveAgentCount = 0;
}

void ACrowdSimulationManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update all active agents
    for (FCrowd_AgentData& Agent : ActiveAgents)
    {
        UpdateAgentState(Agent, DeltaTime);
    }

    ActiveAgentCount = ActiveAgents.Num();
}

void ACrowdSimulationManager::SpawnTribalGroup(FVector CenterLocation, int32 GroupSize, int32 GroupID)
{
    if (ActiveAgents.Num() + GroupSize > MaxAgents)
    {
        GroupSize = MaxAgents - ActiveAgents.Num();
    }
    if (GroupSize <= 0) return;

    for (int32 i = 0; i < GroupSize; i++)
    {
        FCrowd_AgentData NewAgent;
        float Angle = (float(i) / float(GroupSize)) * 2.0f * PI;
        float Radius = FMath::RandRange(100.0f, 400.0f);
        NewAgent.Location = CenterLocation + FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0.0f);
        NewAgent.Velocity = FVector::ZeroVector;
        NewAgent.State = ECrowd_AgentState::Idle;
        NewAgent.AgentType = ECrowd_AgentType::TribalHuman;
        NewAgent.FearLevel = 0.0f;
        NewAgent.Energy = FMath::RandRange(60.0f, 100.0f);
        NewAgent.GroupID = GroupID;
        ActiveAgents.Add(NewAgent);
    }
}

void ACrowdSimulationManager::SpawnDinosaurHerd(FVector StartLocation, int32 HerdSize, int32 GroupID)
{
    if (ActiveAgents.Num() + HerdSize > MaxAgents)
    {
        HerdSize = MaxAgents - ActiveAgents.Num();
    }
    if (HerdSize <= 0) return;

    for (int32 i = 0; i < HerdSize; i++)
    {
        FCrowd_AgentData NewAgent;
        float OffsetX = FMath::RandRange(-500.0f, 500.0f);
        float OffsetY = FMath::RandRange(-500.0f, 500.0f);
        NewAgent.Location = StartLocation + FVector(OffsetX, OffsetY, 0.0f);
        NewAgent.Velocity = FVector(100.0f, 0.0f, 0.0f); // Initial forward velocity
        NewAgent.State = ECrowd_AgentState::Herding;
        NewAgent.AgentType = ECrowd_AgentType::DinosaurHerd;
        NewAgent.FearLevel = 0.0f;
        NewAgent.Energy = 100.0f;
        NewAgent.GroupID = GroupID;
        ActiveAgents.Add(NewAgent);
    }
}

void ACrowdSimulationManager::TriggerFearResponse(FVector ThreatLocation, float FearRadius, float FearIntensity)
{
    for (FCrowd_AgentData& Agent : ActiveAgents)
    {
        float Distance = FVector::Dist(Agent.Location, ThreatLocation);
        if (Distance < FearRadius)
        {
            float FearFalloff = 1.0f - (Distance / FearRadius);
            Agent.FearLevel = FMath::Clamp(Agent.FearLevel + FearIntensity * FearFalloff, 0.0f, 1.0f);
            Agent.State = ECrowd_AgentState::Fleeing;

            // Flee direction — away from threat
            FVector FleeDir = (Agent.Location - ThreatLocation).GetSafeNormal();
            Agent.Velocity = FleeDir * 600.0f * FearFalloff;
        }
    }
}

void ACrowdSimulationManager::UpdateFlockingBehavior(float DeltaTime)
{
    for (FCrowd_AgentData& Agent : ActiveAgents)
    {
        if (Agent.State == ECrowd_AgentState::Herding || Agent.State == ECrowd_AgentState::Wandering)
        {
            FVector Separation = ComputeFlockingSeparation(Agent);
            FVector Alignment  = ComputeFlockingAlignment(Agent);
            FVector Cohesion   = ComputeFlockingCohesion(Agent);
            FVector Migration  = ComputeMigrationSteering(Agent);

            FVector SteeringForce = Separation * 2.0f + Alignment * 1.0f + Cohesion * CohesionStrength + Migration * 1.5f;
            Agent.Velocity = (Agent.Velocity + SteeringForce * DeltaTime).GetClampedToMaxSize(400.0f);
        }
    }
}

bool ACrowdSimulationManager::GetNearestAgent(FVector WorldPosition, FCrowd_AgentData& OutAgent)
{
    float NearestDist = FLT_MAX;
    bool bFound = false;

    for (const FCrowd_AgentData& Agent : ActiveAgents)
    {
        float Dist = FVector::Dist(Agent.Location, WorldPosition);
        if (Dist < NearestDist)
        {
            NearestDist = Dist;
            OutAgent = Agent;
            bFound = true;
        }
    }
    return bFound;
}

void ACrowdSimulationManager::ClearAllAgents()
{
    ActiveAgents.Empty();
    ActiveAgentCount = 0;
}

void ACrowdSimulationManager::ApplyDistanceLOD(FVector PlayerLocation, float FullSimRadius, float LowSimRadius)
{
    for (FCrowd_AgentData& Agent : ActiveAgents)
    {
        float Distance = FVector::Dist(Agent.Location, PlayerLocation);
        if (Distance > LowSimRadius)
        {
            // Beyond LOD range — freeze agent, save CPU
            Agent.Velocity = FVector::ZeroVector;
        }
        else if (Distance > FullSimRadius)
        {
            // Reduced sim — simple linear movement only, no flocking
            Agent.Location += Agent.Velocity * 0.016f; // ~60fps tick approximation
        }
        // Within FullSimRadius — full simulation handled in Tick()
    }
}

// --- Private helpers ---

void ACrowdSimulationManager::UpdateAgentState(FCrowd_AgentData& Agent, float DeltaTime)
{
    // Decay fear over time
    if (Agent.FearLevel > 0.0f)
    {
        Agent.FearLevel = FMath::Max(0.0f, Agent.FearLevel - DeltaTime * 0.2f);
        if (Agent.FearLevel < 0.1f && Agent.State == ECrowd_AgentState::Fleeing)
        {
            Agent.State = ECrowd_AgentState::Wandering;
        }
    }

    // Move agent
    Agent.Location += Agent.Velocity * DeltaTime;

    // Decay velocity (friction)
    Agent.Velocity *= 0.95f;

    // Energy drain
    Agent.Energy = FMath::Max(0.0f, Agent.Energy - DeltaTime * 0.5f);
    if (Agent.Energy < 20.0f)
    {
        Agent.State = ECrowd_AgentState::Resting;
        Agent.Velocity = FVector::ZeroVector;
    }
    else if (Agent.State == ECrowd_AgentState::Resting)
    {
        Agent.Energy = FMath::Min(100.0f, Agent.Energy + DeltaTime * 2.0f);
        if (Agent.Energy > 50.0f)
        {
            Agent.State = ECrowd_AgentState::Wandering;
        }
    }
}

FVector ACrowdSimulationManager::ComputeFlockingSeparation(const FCrowd_AgentData& Agent)
{
    FVector Steer = FVector::ZeroVector;
    int32 Count = 0;

    for (const FCrowd_AgentData& Other : ActiveAgents)
    {
        if (&Other == &Agent) continue;
        float Dist = FVector::Dist(Agent.Location, Other.Location);
        if (Dist < AgentSeparationDistance && Dist > 0.0f)
        {
            FVector Diff = (Agent.Location - Other.Location).GetSafeNormal() / Dist;
            Steer += Diff;
            Count++;
        }
    }
    if (Count > 0) Steer /= float(Count);
    return Steer;
}

FVector ACrowdSimulationManager::ComputeFlockingAlignment(const FCrowd_AgentData& Agent)
{
    FVector AvgVelocity = FVector::ZeroVector;
    int32 Count = 0;

    for (const FCrowd_AgentData& Other : ActiveAgents)
    {
        if (&Other == &Agent) continue;
        if (Other.GroupID == Agent.GroupID)
        {
            float Dist = FVector::Dist(Agent.Location, Other.Location);
            if (Dist < AgentSeparationDistance * 3.0f)
            {
                AvgVelocity += Other.Velocity;
                Count++;
            }
        }
    }
    if (Count > 0)
    {
        AvgVelocity /= float(Count);
        return (AvgVelocity - Agent.Velocity) * 0.1f;
    }
    return FVector::ZeroVector;
}

FVector ACrowdSimulationManager::ComputeFlockingCohesion(const FCrowd_AgentData& Agent)
{
    FVector CenterOfMass = FVector::ZeroVector;
    int32 Count = 0;

    for (const FCrowd_AgentData& Other : ActiveAgents)
    {
        if (&Other == &Agent) continue;
        if (Other.GroupID == Agent.GroupID)
        {
            float Dist = FVector::Dist(Agent.Location, Other.Location);
            if (Dist < AgentSeparationDistance * 5.0f)
            {
                CenterOfMass += Other.Location;
                Count++;
            }
        }
    }
    if (Count > 0)
    {
        CenterOfMass /= float(Count);
        return (CenterOfMass - Agent.Location) * 0.01f;
    }
    return FVector::ZeroVector;
}

FVector ACrowdSimulationManager::ComputeMigrationSteering(const FCrowd_AgentData& Agent)
{
    if (MigrationWaypoints.Num() == 0) return FVector::ZeroVector;

    // Find nearest waypoint ahead
    float NearestDist = FLT_MAX;
    int32 TargetIdx = 0;
    for (int32 i = 0; i < MigrationWaypoints.Num(); i++)
    {
        float Dist = FVector::Dist(Agent.Location, MigrationWaypoints[i].WorldPosition);
        if (Dist < NearestDist)
        {
            NearestDist = Dist;
            TargetIdx = i;
        }
    }

    // If within waypoint radius, target next waypoint
    if (NearestDist < MigrationWaypoints[TargetIdx].Radius)
    {
        TargetIdx = GetNextWaypointIndex(TargetIdx);
    }

    FVector ToWaypoint = (MigrationWaypoints[TargetIdx].WorldPosition - Agent.Location).GetSafeNormal();
    return ToWaypoint * 200.0f;
}

int32 ACrowdSimulationManager::GetNextWaypointIndex(int32 CurrentWaypoint) const
{
    if (MigrationWaypoints.Num() == 0) return 0;
    return (CurrentWaypoint + 1) % MigrationWaypoints.Num();
}
