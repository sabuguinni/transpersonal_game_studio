#include "CrowdHerdManager.h"
#include "DrawDebugHelpers.h"

ACrowdHerdManager::ACrowdHerdManager()
{
    PrimaryActorTick.bCanEverTick = true;

    MaxAgents = 50;
    AgentSpeed = 300.f;
    SeparationRadius = 150.f;
    CohesionRadius = 600.f;
    CurrentBehavior = ECrowd_HerdBehavior::Grazing;
}

void ACrowdHerdManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeHerd(MaxAgents, GetActorLocation(), 1000.f);
}

void ACrowdHerdManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    StepBoidsSimulation(DeltaTime);
}

void ACrowdHerdManager::InitializeHerd(int32 AgentCount, FVector CenterLocation, float Radius)
{
    Agents.Empty();
    Agents.Reserve(AgentCount);

    for (int32 i = 0; i < AgentCount; ++i)
    {
        FCrowd_HerdAgent Agent;
        float Angle = FMath::RandRange(0.f, 360.f);
        float Dist  = FMath::RandRange(0.f, Radius);
        Agent.Location = CenterLocation + FVector(
            FMath::Cos(FMath::DegreesToRadians(Angle)) * Dist,
            FMath::Sin(FMath::DegreesToRadians(Angle)) * Dist,
            0.f
        );
        Agent.Velocity = FVector(FMath::RandRange(-1.f, 1.f), FMath::RandRange(-1.f, 1.f), 0.f).GetSafeNormal() * AgentSpeed * 0.5f;
        Agent.Behavior = CurrentBehavior;
        Agent.Health   = 100.f;
        Agents.Add(Agent);
    }
}

void ACrowdHerdManager::SetHerdBehavior(ECrowd_HerdBehavior NewBehavior)
{
    CurrentBehavior = NewBehavior;
    for (FCrowd_HerdAgent& Agent : Agents)
    {
        Agent.Behavior = NewBehavior;
    }
}

void ACrowdHerdManager::TriggerFleeFromLocation(FVector ThreatLocation, float ThreatRadius)
{
    for (FCrowd_HerdAgent& Agent : Agents)
    {
        float Dist = FVector::Dist(Agent.Location, ThreatLocation);
        if (Dist < ThreatRadius)
        {
            Agent.Behavior = ECrowd_HerdBehavior::Fleeing;
            FVector FleeDir = (Agent.Location - ThreatLocation).GetSafeNormal();
            Agent.Velocity  = FleeDir * AgentSpeed * 1.5f;
        }
    }
    CurrentBehavior = ECrowd_HerdBehavior::Fleeing;
}

int32 ACrowdHerdManager::GetAgentCount() const
{
    return Agents.Num();
}

void ACrowdHerdManager::StepBoidsSimulation(float DeltaTime)
{
    for (int32 i = 0; i < Agents.Num(); ++i)
    {
        FCrowd_HerdAgent& Agent = Agents[i];

        if (Agent.Behavior == ECrowd_HerdBehavior::Resting)
            continue;

        FVector Separation = ComputeSeparation(i) * 1.5f;
        FVector Alignment  = ComputeAlignment(i)  * 1.0f;
        FVector Cohesion   = ComputeCohesion(i)   * 0.8f;

        float SpeedMult = (Agent.Behavior == ECrowd_HerdBehavior::Fleeing) ? 1.5f : 1.0f;
        Agent.Velocity += (Separation + Alignment + Cohesion) * DeltaTime;
        Agent.Velocity  = Agent.Velocity.GetClampedToMaxSize(AgentSpeed * SpeedMult);

        Agent.Location += Agent.Velocity * DeltaTime;
    }
}

FVector ACrowdHerdManager::ComputeSeparation(int32 AgentIndex) const
{
    FVector Force = FVector::ZeroVector;
    const FCrowd_HerdAgent& Self = Agents[AgentIndex];

    for (int32 j = 0; j < Agents.Num(); ++j)
    {
        if (j == AgentIndex) continue;
        float Dist = FVector::Dist(Self.Location, Agents[j].Location);
        if (Dist < SeparationRadius && Dist > 0.f)
        {
            Force += (Self.Location - Agents[j].Location).GetSafeNormal() * (SeparationRadius - Dist);
        }
    }
    return Force;
}

FVector ACrowdHerdManager::ComputeAlignment(int32 AgentIndex) const
{
    FVector AvgVel = FVector::ZeroVector;
    int32 Count = 0;
    const FCrowd_HerdAgent& Self = Agents[AgentIndex];

    for (int32 j = 0; j < Agents.Num(); ++j)
    {
        if (j == AgentIndex) continue;
        if (FVector::Dist(Self.Location, Agents[j].Location) < CohesionRadius)
        {
            AvgVel += Agents[j].Velocity;
            ++Count;
        }
    }
    if (Count > 0) AvgVel /= (float)Count;
    return AvgVel;
}

FVector ACrowdHerdManager::ComputeCohesion(int32 AgentIndex) const
{
    FVector Center = FVector::ZeroVector;
    int32 Count = 0;
    const FCrowd_HerdAgent& Self = Agents[AgentIndex];

    for (int32 j = 0; j < Agents.Num(); ++j)
    {
        if (j == AgentIndex) continue;
        if (FVector::Dist(Self.Location, Agents[j].Location) < CohesionRadius)
        {
            Center += Agents[j].Location;
            ++Count;
        }
    }
    if (Count > 0)
    {
        Center /= (float)Count;
        return (Center - Self.Location).GetSafeNormal() * AgentSpeed * 0.3f;
    }
    return FVector::ZeroVector;
}
