// CrowdHerdBehavior.cpp
// Agent #13 — Crowd & Traffic Simulation
// Implementation of herd flocking behavior for dinosaur crowds

#include "CrowdHerdBehavior.h"
#include "Math/UnrealMathUtility.h"

UCrowdHerdBehavior::UCrowdHerdBehavior()
{
    HerdData.HerdID = 0;
    HerdData.CurrentState = ECrowd_HerdState::Grazing;
    HerdData.AgentCount = 0;
    HerdData.CohesionRadius = 500.0f;
    HerdData.SeparationRadius = 150.0f;
    HerdData.AlignmentRadius = 300.0f;
}

void UCrowdHerdBehavior::InitializeHerd(int32 HerdID, const FString& Species, int32 NumAgents)
{
    HerdData.HerdID = HerdID;
    HerdData.SpeciesName = Species;
    HerdData.AgentCount = NumAgents;
    HerdData.CurrentState = ECrowd_HerdState::Grazing;

    Agents.Empty();
    Agents.Reserve(NumAgents);

    for (int32 i = 0; i < NumAgents; i++)
    {
        FCrowd_HerdAgent Agent;
        Agent.AgentID = i;
        Agent.Role = (i == 0) ? ECrowd_AgentRole::Leader : ECrowd_AgentRole::Follower;
        // Scatter agents in a rough circle around origin
        float Angle = (float)i / (float)NumAgents * 2.0f * PI;
        float Radius = FMath::RandRange(100.0f, 400.0f);
        Agent.Position = FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0.0f);
        Agent.FearLevel = 0.0f;
        Agent.StaminaLevel = 1.0f;
        Agent.bIsAlive = true;
        Agents.Add(Agent);
    }

    UpdateCenterOfMass();
}

void UCrowdHerdBehavior::UpdateHerdState(float DeltaTime)
{
    if (Agents.Num() == 0) return;

    // Drain stamina during stampede/flee
    if (HerdData.CurrentState == ECrowd_HerdState::Stampeding ||
        HerdData.CurrentState == ECrowd_HerdState::Fleeing)
    {
        for (FCrowd_HerdAgent& Agent : Agents)
        {
            Agent.StaminaLevel = FMath::Max(0.0f, Agent.StaminaLevel - DeltaTime * 0.05f);
            if (Agent.StaminaLevel <= 0.0f)
            {
                // Exhausted — slow down, become straggler
                Agent.Role = ECrowd_AgentRole::Straggler;
            }
        }
    }

    // Recover fear over time when not threatened
    if (HerdData.ThreatLevel <= 0.0f)
    {
        for (FCrowd_HerdAgent& Agent : Agents)
        {
            Agent.FearLevel = FMath::Max(0.0f, Agent.FearLevel - DeltaTime * 0.1f);
        }

        // Transition back to grazing if fear is low
        float AvgFear = 0.0f;
        for (const FCrowd_HerdAgent& Agent : Agents)
        {
            AvgFear += Agent.FearLevel;
        }
        AvgFear /= (float)Agents.Num();

        if (AvgFear < 0.1f && HerdData.CurrentState == ECrowd_HerdState::Fleeing)
        {
            HerdData.CurrentState = ECrowd_HerdState::Grazing;
        }
        else if (AvgFear < 0.05f && HerdData.CurrentState == ECrowd_HerdState::Stampeding)
        {
            HerdData.CurrentState = ECrowd_HerdState::Fleeing;
        }
    }

    // Apply flocking forces to each agent
    for (FCrowd_HerdAgent& Agent : Agents)
    {
        if (!Agent.bIsAlive) continue;
        FVector FlockForce = ComputeFlockingForce(Agent);
        Agent.Velocity = (Agent.Velocity + FlockForce * DeltaTime).GetClampedToMaxSize(600.0f);
        Agent.Position += Agent.Velocity * DeltaTime;
    }

    UpdateCenterOfMass();
    HerdData.ThreatLevel = FMath::Max(0.0f, HerdData.ThreatLevel - DeltaTime * 0.2f);
}

void UCrowdHerdBehavior::TriggerFlee(FVector ThreatLocation, float ThreatRadius)
{
    HerdData.ThreatLevel = 1.0f;
    HerdData.CurrentState = ECrowd_HerdState::Fleeing;

    for (FCrowd_HerdAgent& Agent : Agents)
    {
        float Dist = FVector::Dist(Agent.Position, ThreatLocation);
        if (Dist < ThreatRadius)
        {
            Agent.FearLevel = FMath::Clamp(1.0f - (Dist / ThreatRadius), 0.0f, 1.0f);
        }
    }
}

void UCrowdHerdBehavior::TriggerStampede(FVector Direction)
{
    HerdData.ThreatLevel = 1.0f;
    HerdData.CurrentState = ECrowd_HerdState::Stampeding;
    FVector NormDir = Direction.GetSafeNormal();

    for (FCrowd_HerdAgent& Agent : Agents)
    {
        Agent.FearLevel = 1.0f;
        // Blast all agents in the stampede direction
        Agent.Velocity = NormDir * 800.0f + FVector(
            FMath::RandRange(-100.0f, 100.0f),
            FMath::RandRange(-100.0f, 100.0f),
            0.0f);
    }
}

FVector UCrowdHerdBehavior::ComputeFlockingForce(const FCrowd_HerdAgent& Agent) const
{
    FVector Cohesion = ComputeCohesion(Agent);
    FVector Separation = ComputeSeparation(Agent);
    FVector Alignment = ComputeAlignment(Agent);

    // Weight the forces
    float FearMult = 1.0f + Agent.FearLevel * 3.0f;
    return (Cohesion * 0.3f + Separation * 1.5f + Alignment * 0.8f) * FearMult;
}

FVector UCrowdHerdBehavior::ComputeCohesion(const FCrowd_HerdAgent& Agent) const
{
    // Steer toward center of mass
    FVector ToCenter = HerdData.CenterOfMass - Agent.Position;
    float Dist = ToCenter.Size();
    if (Dist < 1.0f) return FVector::ZeroVector;
    return ToCenter.GetSafeNormal() * FMath::Min(Dist / HerdData.CohesionRadius, 1.0f);
}

FVector UCrowdHerdBehavior::ComputeSeparation(const FCrowd_HerdAgent& Agent) const
{
    FVector SepForce = FVector::ZeroVector;
    for (const FCrowd_HerdAgent& Other : Agents)
    {
        if (Other.AgentID == Agent.AgentID || !Other.bIsAlive) continue;
        FVector Delta = Agent.Position - Other.Position;
        float Dist = Delta.Size();
        if (Dist < HerdData.SeparationRadius && Dist > 0.1f)
        {
            SepForce += Delta.GetSafeNormal() * (1.0f - Dist / HerdData.SeparationRadius);
        }
    }
    return SepForce;
}

FVector UCrowdHerdBehavior::ComputeAlignment(const FCrowd_HerdAgent& Agent) const
{
    FVector AvgVelocity = FVector::ZeroVector;
    int32 Count = 0;
    for (const FCrowd_HerdAgent& Other : Agents)
    {
        if (Other.AgentID == Agent.AgentID || !Other.bIsAlive) continue;
        float Dist = FVector::Dist(Agent.Position, Other.Position);
        if (Dist < HerdData.AlignmentRadius)
        {
            AvgVelocity += Other.Velocity;
            Count++;
        }
    }
    if (Count == 0) return FVector::ZeroVector;
    AvgVelocity /= (float)Count;
    return (AvgVelocity - Agent.Velocity).GetSafeNormal() * 0.5f;
}

void UCrowdHerdBehavior::UpdateCenterOfMass()
{
    if (Agents.Num() == 0)
    {
        HerdData.CenterOfMass = FVector::ZeroVector;
        return;
    }
    FVector Sum = FVector::ZeroVector;
    int32 AliveCount = 0;
    for (const FCrowd_HerdAgent& Agent : Agents)
    {
        if (Agent.bIsAlive)
        {
            Sum += Agent.Position;
            AliveCount++;
        }
    }
    HerdData.CenterOfMass = (AliveCount > 0) ? Sum / (float)AliveCount : FVector::ZeroVector;
    HerdData.AgentCount = AliveCount;
}
