#include "CrowdStampedeController.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"
#include "DrawDebugHelpers.h"

ACrowdStampedeController::ACrowdStampedeController()
{
    PrimaryActorTick.bCanEverTick = true;

    SeparationWeight  = 2.5f;
    AlignmentWeight   = 1.0f;
    CohesionWeight    = 1.0f;
    FleeWeight        = 4.0f;
    NeighborRadius    = 400.f;
    MaxStampedeSpeed  = 900.f;
    FearDecayRate     = 0.3f;
    MaxAgents         = 50;
    CurrentState      = ECrowd_StampedeState::Idle;
    PanicOrigin       = FVector::ZeroVector;
}

void ACrowdStampedeController::BeginPlay()
{
    Super::BeginPlay();
}

void ACrowdStampedeController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (CurrentState == ECrowd_StampedeState::Idle || Agents.Num() == 0)
    {
        return;
    }

    UpdateFlockingBehavior(DeltaTime);
    UpdateAgentMeshPositions();

    // Decay fear over time
    bool bAllCalm = true;
    for (FCrowd_StampedeAgent& Agent : Agents)
    {
        Agent.FearLevel = FMath::Max(0.f, Agent.FearLevel - FearDecayRate * DeltaTime);
        if (Agent.FearLevel > 0.1f) bAllCalm = false;
    }

    if (bAllCalm && CurrentState == ECrowd_StampedeState::Stampeding)
    {
        SetStampedeState(ECrowd_StampedeState::Dispersed);
    }
}

void ACrowdStampedeController::TriggerStampede(FVector PanicSource, float PanicRadius)
{
    PanicOrigin = PanicSource;
    SetStampedeState(ECrowd_StampedeState::Stampeding);

    for (FCrowd_StampedeAgent& Agent : Agents)
    {
        float Dist = FVector::Dist(Agent.Location, PanicSource);
        if (Dist < PanicRadius)
        {
            float NormalizedDist = 1.0f - (Dist / PanicRadius);
            Agent.FearLevel = FMath::Clamp(NormalizedDist * 1.5f, 0.f, 1.f);
        }
        else
        {
            // Propagate fear from neighbors — herd contagion
            Agent.FearLevel = FMath::Max(Agent.FearLevel, 0.4f);
        }
    }
}

void ACrowdStampedeController::SpawnHerd(int32 HerdSize, FVector CenterLocation, float SpreadRadius)
{
    Agents.Empty();
    int32 ActualSize = FMath::Min(HerdSize, MaxAgents);

    for (int32 i = 0; i < ActualSize; ++i)
    {
        FCrowd_StampedeAgent NewAgent;
        float Angle = (float)i / (float)ActualSize * 2.f * PI;
        float Radius = FMath::FRandRange(SpreadRadius * 0.2f, SpreadRadius);
        NewAgent.Location = CenterLocation + FVector(
            FMath::Cos(Angle) * Radius,
            FMath::Sin(Angle) * Radius,
            0.f
        );
        NewAgent.Velocity = FVector::ZeroVector;
        NewAgent.Speed = FMath::FRandRange(400.f, 600.f);
        NewAgent.FearLevel = 0.f;
        NewAgent.bIsLeader = (i == 0);
        Agents.Add(NewAgent);
    }

    SetStampedeState(ECrowd_StampedeState::Idle);
}

void ACrowdStampedeController::UpdateFlockingBehavior(float DeltaTime)
{
    for (int32 i = 0; i < Agents.Num(); ++i)
    {
        FCrowd_StampedeAgent& Agent = Agents[i];
        FVector Force = ComputeFlockingForce(Agent, i);

        // Integrate velocity
        Agent.Velocity += Force * DeltaTime;

        // Clamp to max speed based on fear
        float CurrentMaxSpeed = FMath::Lerp(Agent.Speed, MaxStampedeSpeed, Agent.FearLevel);
        if (Agent.Velocity.SizeSquared() > CurrentMaxSpeed * CurrentMaxSpeed)
        {
            Agent.Velocity = Agent.Velocity.GetSafeNormal() * CurrentMaxSpeed;
        }

        Agent.Location += Agent.Velocity * DeltaTime;
    }
}

FVector ACrowdStampedeController::ComputeFlockingForce(const FCrowd_StampedeAgent& Agent, int32 AgentIndex)
{
    FVector Separation = ComputeSeparation(Agent, AgentIndex) * SeparationWeight;
    FVector Alignment  = ComputeAlignment(Agent, AgentIndex)  * AlignmentWeight;
    FVector Cohesion   = ComputeCohesion(Agent, AgentIndex)   * CohesionWeight;
    FVector Flee       = ComputeFlee(Agent)                   * FleeWeight * Agent.FearLevel;

    return Separation + Alignment + Cohesion + Flee;
}

FVector ACrowdStampedeController::ComputeSeparation(const FCrowd_StampedeAgent& Agent, int32 AgentIndex)
{
    FVector Force = FVector::ZeroVector;
    int32 Count = 0;
    float MinSeparation = NeighborRadius * 0.3f;

    for (int32 j = 0; j < Agents.Num(); ++j)
    {
        if (j == AgentIndex) continue;
        float Dist = FVector::Dist(Agent.Location, Agents[j].Location);
        if (Dist < MinSeparation && Dist > 0.f)
        {
            FVector Away = (Agent.Location - Agents[j].Location).GetSafeNormal();
            Force += Away / Dist;
            ++Count;
        }
    }

    if (Count > 0) Force /= (float)Count;
    return Force;
}

FVector ACrowdStampedeController::ComputeAlignment(const FCrowd_StampedeAgent& Agent, int32 AgentIndex)
{
    FVector AvgVelocity = FVector::ZeroVector;
    int32 Count = 0;

    for (int32 j = 0; j < Agents.Num(); ++j)
    {
        if (j == AgentIndex) continue;
        float Dist = FVector::Dist(Agent.Location, Agents[j].Location);
        if (Dist < NeighborRadius)
        {
            AvgVelocity += Agents[j].Velocity;
            ++Count;
        }
    }

    if (Count > 0)
    {
        AvgVelocity /= (float)Count;
        return (AvgVelocity - Agent.Velocity).GetSafeNormal();
    }
    return FVector::ZeroVector;
}

FVector ACrowdStampedeController::ComputeCohesion(const FCrowd_StampedeAgent& Agent, int32 AgentIndex)
{
    FVector CenterOfMass = FVector::ZeroVector;
    int32 Count = 0;

    for (int32 j = 0; j < Agents.Num(); ++j)
    {
        if (j == AgentIndex) continue;
        float Dist = FVector::Dist(Agent.Location, Agents[j].Location);
        if (Dist < NeighborRadius)
        {
            CenterOfMass += Agents[j].Location;
            ++Count;
        }
    }

    if (Count > 0)
    {
        CenterOfMass /= (float)Count;
        return (CenterOfMass - Agent.Location).GetSafeNormal();
    }
    return FVector::ZeroVector;
}

FVector ACrowdStampedeController::ComputeFlee(const FCrowd_StampedeAgent& Agent)
{
    if (PanicOrigin.IsZero()) return FVector::ZeroVector;
    return (Agent.Location - PanicOrigin).GetSafeNormal();
}

void ACrowdStampedeController::SetStampedeState(ECrowd_StampedeState NewState)
{
    CurrentState = NewState;
}

void ACrowdStampedeController::UpdateAgentMeshPositions()
{
    for (int32 i = 0; i < SpawnedMeshActors.Num() && i < Agents.Num(); ++i)
    {
        if (SpawnedMeshActors[i])
        {
            SpawnedMeshActors[i]->SetActorLocation(Agents[i].Location);
            if (!Agents[i].Velocity.IsNearlyZero())
            {
                FRotator FaceDir = Agents[i].Velocity.Rotation();
                SpawnedMeshActors[i]->SetActorRotation(FaceDir);
            }
        }
    }
}
