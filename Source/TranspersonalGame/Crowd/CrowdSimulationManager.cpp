#include "CrowdSimulationManager.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"

ACrowdSimulationManager::ACrowdSimulationManager()
{
    PrimaryActorTick.bCanEverTick = true;

    MaxAgents           = 50;
    AgentUpdateInterval = 0.1f;
    FleeRadius          = 2000.0f;
    WanderRadius        = 1500.0f;
    ActiveAgentCount    = 0;
    TimeSinceLastUpdate = 0.0f;
}

void ACrowdSimulationManager::BeginPlay()
{
    Super::BeginPlay();
    // Auto-spawn a small tribe near the player start
    SpawnCrowdAgents(20, FVector(0.0f, 0.0f, 400.0f), 800.0f);
}

void ACrowdSimulationManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    TimeSinceLastUpdate += DeltaTime;
    if (TimeSinceLastUpdate >= AgentUpdateInterval)
    {
        UpdateAgentStates(TimeSinceLastUpdate);
        TimeSinceLastUpdate = 0.0f;
    }
}

void ACrowdSimulationManager::SpawnCrowdAgents(int32 Count, FVector CenterLocation, float SpawnRadius)
{
    int32 ToSpawn = FMath::Min(Count, MaxAgents - ActiveAgentCount);

    for (int32 i = 0; i < ToSpawn; ++i)
    {
        FCrowd_AgentData NewAgent;

        // Random position within spawn radius
        float Angle  = FMath::RandRange(0.0f, 360.0f);
        float Dist   = FMath::RandRange(0.0f, SpawnRadius);
        NewAgent.Location = CenterLocation + FVector(
            FMath::Cos(FMath::DegreesToRadians(Angle)) * Dist,
            FMath::Sin(FMath::DegreesToRadians(Angle)) * Dist,
            0.0f
        );

        // Assign role based on distribution
        int32 RoleRoll = FMath::RandRange(0, 4);
        switch (RoleRoll)
        {
            case 0: NewAgent.Role = ECrowd_AgentRole::Hunter;   break;
            case 1: NewAgent.Role = ECrowd_AgentRole::Gatherer; break;
            case 2: NewAgent.Role = ECrowd_AgentRole::Scout;    break;
            case 3: NewAgent.Role = ECrowd_AgentRole::Elder;    break;
            case 4: NewAgent.Role = ECrowd_AgentRole::Child;    break;
            default: NewAgent.Role = ECrowd_AgentRole::Hunter;  break;
        }

        NewAgent.State  = ECrowd_AgentState::Wandering;
        NewAgent.Fear   = 0.0f;
        NewAgent.Energy = FMath::RandRange(0.5f, 1.0f);

        AgentPool.Add(NewAgent);
        ActiveAgentCount++;
    }
}

void ACrowdSimulationManager::TriggerFleeEvent(FVector ThreatLocation, float ThreatRadius)
{
    for (FCrowd_AgentData& Agent : AgentPool)
    {
        float DistToThreat = FVector::Dist(Agent.Location, ThreatLocation);
        if (DistToThreat <= ThreatRadius)
        {
            Agent.State = ECrowd_AgentState::Fleeing;
            // Fear scales inversely with distance
            Agent.Fear = FMath::Clamp(1.0f - (DistToThreat / ThreatRadius), 0.0f, 1.0f);
        }
    }
}

void ACrowdSimulationManager::UpdateAgentStates(float DeltaTime)
{
    for (FCrowd_AgentData& Agent : AgentPool)
    {
        UpdateSingleAgent(Agent, DeltaTime);
    }
}

int32 ACrowdSimulationManager::GetActiveAgentCount() const
{
    return ActiveAgentCount;
}

void ACrowdSimulationManager::SetAgentRole(int32 AgentIndex, ECrowd_AgentRole NewRole)
{
    if (AgentPool.IsValidIndex(AgentIndex))
    {
        AgentPool[AgentIndex].Role = NewRole;
    }
}

void ACrowdSimulationManager::UpdateSingleAgent(FCrowd_AgentData& Agent, float DeltaTime)
{
    // Fear decay over time
    Agent.Fear = FMath::Max(0.0f, Agent.Fear - DeltaTime * 0.2f);

    // Energy recovery when resting
    if (Agent.State == ECrowd_AgentState::Resting)
    {
        Agent.Energy = FMath::Min(1.0f, Agent.Energy + DeltaTime * 0.1f);
    }
    else
    {
        Agent.Energy = FMath::Max(0.0f, Agent.Energy - DeltaTime * 0.02f);
    }

    // State transitions
    switch (Agent.State)
    {
        case ECrowd_AgentState::Fleeing:
            // Stop fleeing when fear drops below threshold
            if (Agent.Fear < 0.1f)
            {
                Agent.State = ECrowd_AgentState::Wandering;
            }
            else
            {
                // Move away from threat direction
                Agent.Location += GetFleeDirection(Agent, Agent.Location) * DeltaTime * 300.0f;
            }
            break;

        case ECrowd_AgentState::Wandering:
            // Rest when energy is low
            if (Agent.Energy < 0.2f)
            {
                Agent.State = ECrowd_AgentState::Resting;
            }
            else
            {
                // Slow wander movement
                FVector WanderTarget = GetWanderTarget(Agent);
                FVector Dir = (WanderTarget - Agent.Location).GetSafeNormal();
                Agent.Location += Dir * DeltaTime * 80.0f;
            }
            break;

        case ECrowd_AgentState::Resting:
            // Resume wandering after recovering energy
            if (Agent.Energy > 0.8f)
            {
                Agent.State = ECrowd_AgentState::Wandering;
            }
            break;

        case ECrowd_AgentState::Hunting:
            Agent.Energy = FMath::Max(0.0f, Agent.Energy - DeltaTime * 0.05f);
            if (Agent.Energy < 0.3f)
            {
                Agent.State = ECrowd_AgentState::Resting;
            }
            break;

        case ECrowd_AgentState::Gathering:
            Agent.Energy = FMath::Max(0.0f, Agent.Energy - DeltaTime * 0.03f);
            break;

        default:
            break;
    }
}

FVector ACrowdSimulationManager::GetWanderTarget(const FCrowd_AgentData& Agent) const
{
    float Angle = FMath::RandRange(0.0f, 360.0f);
    float Dist  = FMath::RandRange(100.0f, WanderRadius * 0.3f);
    return Agent.Location + FVector(
        FMath::Cos(FMath::DegreesToRadians(Angle)) * Dist,
        FMath::Sin(FMath::DegreesToRadians(Angle)) * Dist,
        0.0f
    );
}

FVector ACrowdSimulationManager::GetFleeDirection(const FCrowd_AgentData& Agent, FVector ThreatLocation) const
{
    // Move away from the threat
    FVector AwayDir = (Agent.Location - ThreatLocation).GetSafeNormal();
    // Add slight random offset to prevent all agents fleeing in exact same direction
    float RandomAngle = FMath::RandRange(-30.0f, 30.0f);
    FQuat RandomRot   = FQuat(FVector::UpVector, FMath::DegreesToRadians(RandomAngle));
    return RandomRot.RotateVector(AwayDir);
}
