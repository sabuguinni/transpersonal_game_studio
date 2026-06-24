#include "CrowdHerdBehavior.h"
#include "Math/UnrealMathUtility.h"

ACrowd_HerdBehaviorActor::ACrowd_HerdBehaviorActor()
{
    PrimaryActorTick.bCanEverTick = true;

    HerdSize = 20;
    CohesionRadius = 500.0f;
    SeparationRadius = 150.0f;
    AlignmentRadius = 300.0f;
    MaxSpeed = 400.0f;
    CurrentHerdState = ECrowd_HerdState::Grazing;
    bHasThreat = false;
    ThreatDecayTimer = 0.0f;
    ActiveThreatLocation = FVector::ZeroVector;
}

void ACrowd_HerdBehaviorActor::BeginPlay()
{
    Super::BeginPlay();
    InitializeHerd(HerdSize, GetActorLocation(), 800.0f);
}

void ACrowd_HerdBehaviorActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bHasThreat)
    {
        ThreatDecayTimer -= DeltaTime;
        if (ThreatDecayTimer <= 0.0f)
        {
            bHasThreat = false;
            SetHerdState(ECrowd_HerdState::Grazing);
        }
    }

    UpdateBoids(DeltaTime);
}

void ACrowd_HerdBehaviorActor::InitializeHerd(int32 Count, FVector Origin, float SpreadRadius)
{
    Agents.Empty();
    Agents.Reserve(Count);

    for (int32 i = 0; i < Count; ++i)
    {
        FCrowd_HerdAgent Agent;
        float Angle = FMath::RandRange(0.0f, 2.0f * PI);
        float Dist = FMath::RandRange(0.0f, SpreadRadius);
        Agent.Location = Origin + FVector(FMath::Cos(Angle) * Dist, FMath::Sin(Angle) * Dist, 0.0f);
        Agent.Velocity = FVector(FMath::RandRange(-100.0f, 100.0f), FMath::RandRange(-100.0f, 100.0f), 0.0f);
        Agent.State = ECrowd_HerdState::Grazing;
        Agent.FearLevel = 0.0f;
        Agents.Add(Agent);
    }
}

void ACrowd_HerdBehaviorActor::TriggerFlee(FVector ThreatLocation, float ThreatRadius)
{
    ActiveThreatLocation = ThreatLocation;
    bHasThreat = true;
    ThreatDecayTimer = 15.0f;
    SetHerdState(ECrowd_HerdState::Fleeing);

    for (FCrowd_HerdAgent& Agent : Agents)
    {
        float Dist = FVector::Dist(Agent.Location, ThreatLocation);
        float Fear = FMath::Clamp(1.0f - (Dist / ThreatRadius), 0.0f, 1.0f);
        Agent.FearLevel = Fear;
        Agent.State = ECrowd_HerdState::Fleeing;
    }
}

void ACrowd_HerdBehaviorActor::SetHerdState(ECrowd_HerdState NewState)
{
    CurrentHerdState = NewState;
    for (FCrowd_HerdAgent& Agent : Agents)
    {
        Agent.State = NewState;
    }
}

int32 ACrowd_HerdBehaviorActor::GetAgentCount() const
{
    return Agents.Num();
}

void ACrowd_HerdBehaviorActor::UpdateBoids(float DeltaTime)
{
    for (int32 i = 0; i < Agents.Num(); ++i)
    {
        FCrowd_HerdAgent& Agent = Agents[i];

        FVector Steering = FVector::ZeroVector;

        if (Agent.State == ECrowd_HerdState::Fleeing && bHasThreat)
        {
            Steering += ComputeFlee(i, ActiveThreatLocation) * 2.0f;
        }
        else if (Agent.State == ECrowd_HerdState::Grazing || Agent.State == ECrowd_HerdState::Migrating)
        {
            Steering += ComputeCohesion(i) * 0.8f;
            Steering += ComputeSeparation(i) * 1.5f;
            Steering += ComputeAlignment(i) * 1.0f;
        }

        Agent.Velocity += Steering * DeltaTime;

        float Speed = Agent.Velocity.Size();
        if (Speed > MaxSpeed)
        {
            Agent.Velocity = Agent.Velocity.GetSafeNormal() * MaxSpeed;
        }

        Agent.Location += Agent.Velocity * DeltaTime;
        Agent.Location.Z = 0.0f;
    }
}

FVector ACrowd_HerdBehaviorActor::ComputeCohesion(int32 AgentIndex)
{
    FVector Center = FVector::ZeroVector;
    int32 Count = 0;
    const FCrowd_HerdAgent& Self = Agents[AgentIndex];

    for (int32 j = 0; j < Agents.Num(); ++j)
    {
        if (j == AgentIndex) continue;
        float Dist = FVector::Dist(Self.Location, Agents[j].Location);
        if (Dist < CohesionRadius)
        {
            Center += Agents[j].Location;
            ++Count;
        }
    }

    if (Count == 0) return FVector::ZeroVector;
    Center /= (float)Count;
    return (Center - Self.Location).GetSafeNormal() * 50.0f;
}

FVector ACrowd_HerdBehaviorActor::ComputeSeparation(int32 AgentIndex)
{
    FVector Repulsion = FVector::ZeroVector;
    const FCrowd_HerdAgent& Self = Agents[AgentIndex];

    for (int32 j = 0; j < Agents.Num(); ++j)
    {
        if (j == AgentIndex) continue;
        float Dist = FVector::Dist(Self.Location, Agents[j].Location);
        if (Dist < SeparationRadius && Dist > 0.0f)
        {
            FVector Away = (Self.Location - Agents[j].Location).GetSafeNormal();
            Repulsion += Away * (SeparationRadius / Dist);
        }
    }

    return Repulsion;
}

FVector ACrowd_HerdBehaviorActor::ComputeAlignment(int32 AgentIndex)
{
    FVector AvgVelocity = FVector::ZeroVector;
    int32 Count = 0;
    const FCrowd_HerdAgent& Self = Agents[AgentIndex];

    for (int32 j = 0; j < Agents.Num(); ++j)
    {
        if (j == AgentIndex) continue;
        float Dist = FVector::Dist(Self.Location, Agents[j].Location);
        if (Dist < AlignmentRadius)
        {
            AvgVelocity += Agents[j].Velocity;
            ++Count;
        }
    }

    if (Count == 0) return FVector::ZeroVector;
    AvgVelocity /= (float)Count;
    return AvgVelocity.GetSafeNormal() * 30.0f;
}

FVector ACrowd_HerdBehaviorActor::ComputeFlee(int32 AgentIndex, FVector ThreatPos)
{
    const FCrowd_HerdAgent& Self = Agents[AgentIndex];
    FVector Away = (Self.Location - ThreatPos).GetSafeNormal();
    float FearMultiplier = 1.0f + Self.FearLevel * 3.0f;
    return Away * MaxSpeed * FearMultiplier;
}
