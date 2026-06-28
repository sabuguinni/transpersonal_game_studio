#include "CrowdStampedeController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ACrowd_StampedeController::ACrowd_StampedeController()
{
    PrimaryActorTick.bCanEverTick = true;

    MaxAgents = 50000;
    HerdRadius = 60000.0f;       // 600m herd spread
    StampedeSpeed = 1200.0f;     // 12 m/s stampede velocity
    FearDecayRate = 0.05f;       // Fear dissipates over ~20s
    AlertRadius = 3000.0f;       // 30m alert detection radius

    LODNearDistance = 5000.0f;   // 50m — full simulation
    LODMidDistance = 15000.0f;   // 150m — simplified physics
    LODFarDistance = 40000.0f;   // 400m — billboard/impostor

    HerdState = ECrowd_StampedeState::Grazing;
    bStampedeActive = false;
    StampedeTimer = 0.0f;
    RegroupTimer = 0.0f;
    ActiveThreatLocation = FVector::ZeroVector;
}

void ACrowd_StampedeController::BeginPlay()
{
    Super::BeginPlay();

    // Initialize agent pool (lightweight structs — no actors spawned at runtime)
    Agents.Reserve(FMath::Min(MaxAgents, 500)); // Cap initial pool at 500 for editor safety
    for (int32 i = 0; i < FMath::Min(MaxAgents, 500); ++i)
    {
        FCrowd_StampedeAgent Agent;
        float Angle = (i / 500.0f) * 2.0f * PI;
        float Radius = HerdRadius * FMath::RandRange(0.1f, 1.0f);
        Agent.Location = GetActorLocation() + FVector(
            FMath::Cos(Angle) * Radius,
            FMath::Sin(Angle) * Radius,
            0.0f
        );
        Agent.State = ECrowd_StampedeState::Grazing;
        Agent.FearLevel = 0.0f;
        Agent.LODDistance = (Agent.Location - GetActorLocation()).Size();
        Agents.Add(Agent);
    }
}

void ACrowd_StampedeController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bStampedeActive)
    {
        StampedeTimer += DeltaTime;
        UpdateAgentStates(DeltaTime);

        // Stampede ends after 30s if no new threat
        if (StampedeTimer > 30.0f)
        {
            bStampedeActive = false;
            HerdState = ECrowd_StampedeState::Regrouping;
            RegroupTimer = 0.0f;
            StampedeTimer = 0.0f;
        }
    }
    else if (HerdState == ECrowd_StampedeState::Regrouping)
    {
        RegroupTimer += DeltaTime;
        if (RegroupTimer > 60.0f)
        {
            HerdState = ECrowd_StampedeState::Grazing;
            RegroupTimer = 0.0f;
            // Reset all agents to grazing
            for (FCrowd_StampedeAgent& Agent : Agents)
            {
                Agent.State = ECrowd_StampedeState::Grazing;
                Agent.FearLevel = FMath::Max(0.0f, Agent.FearLevel - FearDecayRate * 60.0f);
            }
        }
    }
}

void ACrowd_StampedeController::TriggerStampede(FVector ThreatLocation, float ThreatRadius)
{
    ActiveThreatLocation = ThreatLocation;
    bStampedeActive = true;
    StampedeTimer = 0.0f;
    HerdState = ECrowd_StampedeState::Fleeing;

    // Propagate fear through herd — agents near threat flee first
    for (FCrowd_StampedeAgent& Agent : Agents)
    {
        float DistToThreat = (Agent.Location - ThreatLocation).Size();
        if (DistToThreat < ThreatRadius)
        {
            Agent.FearLevel = 1.0f;
            Agent.State = ECrowd_StampedeState::Fleeing;
            Agent.Velocity = GetFleeDirection(Agent.Location, ThreatLocation) * StampedeSpeed;
        }
        else if (DistToThreat < ThreatRadius * 2.5f)
        {
            // Contagion — nearby agents become alert
            Agent.FearLevel = FMath::Lerp(0.3f, 0.8f, 1.0f - (DistToThreat / (ThreatRadius * 2.5f)));
            Agent.State = ECrowd_StampedeState::Alert;
        }
    }
}

void ACrowd_StampedeController::UpdateAgentStates(float DeltaTime)
{
    for (FCrowd_StampedeAgent& Agent : Agents)
    {
        // Update LOD distance
        Agent.LODDistance = (Agent.Location - GetActorLocation()).Size();

        // Skip full simulation for far agents (LOD)
        if (Agent.LODDistance > LODFarDistance)
        {
            continue;
        }

        // Fear propagation — alert agents can become fleeing
        if (Agent.State == ECrowd_StampedeState::Alert)
        {
            Agent.FearLevel = FMath::Min(1.0f, Agent.FearLevel + DeltaTime * 0.5f);
            if (Agent.FearLevel > 0.7f)
            {
                Agent.State = ECrowd_StampedeState::Fleeing;
                Agent.Velocity = GetFleeDirection(Agent.Location, ActiveThreatLocation) * StampedeSpeed;
            }
        }

        // Move fleeing agents
        if (Agent.State == ECrowd_StampedeState::Fleeing)
        {
            Agent.Location += Agent.Velocity * DeltaTime;
            // Decay fear over time
            Agent.FearLevel = FMath::Max(0.0f, Agent.FearLevel - FearDecayRate * DeltaTime);
            if (Agent.FearLevel < 0.1f)
            {
                Agent.State = ECrowd_StampedeState::Scattered;
                Agent.Velocity = FVector::ZeroVector;
            }
        }
    }
}

FVector ACrowd_StampedeController::GetFleeDirection(FVector AgentLocation, FVector ThreatLocation) const
{
    FVector FleeDir = (AgentLocation - ThreatLocation).GetSafeNormal();
    // Add slight lateral randomness to prevent all agents fleeing in same direction
    FVector Lateral = FVector(-FleeDir.Y, FleeDir.X, 0.0f);
    float RandomLateral = FMath::RandRange(-0.3f, 0.3f);
    return (FleeDir + Lateral * RandomLateral).GetSafeNormal();
}

float ACrowd_StampedeController::GetLODDistance(int32 AgentIndex) const
{
    if (!Agents.IsValidIndex(AgentIndex))
    {
        return 0.0f;
    }
    return Agents[AgentIndex].LODDistance;
}
