#include "CrowdStampedeController.h"
#include "Math/UnrealMathUtility.h"

UCrowd_StampedeController::UCrowd_StampedeController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.05f; // 20Hz — sufficient for crowd sim

    AgentCount = 50;
    StampedeSpeed = 700.f;
    PanicPropagationRadius = 800.f;
    PanicDecayRate = 0.15f;
    SeparationRadius = 150.f;
    CohesionRadius = 400.f;
    AlignmentWeight = 1.2f;
    CohesionWeight = 0.8f;
    SeparationWeight = 1.5f;

    CurrentState = ECrowd_StampedeState::Idle;
    AveragePanicLevel = 0.f;
    StateTimer = 0.f;
    StampedeDirection = FVector(1.f, 0.f, 0.f);
}

void UCrowd_StampedeController::BeginPlay()
{
    Super::BeginPlay();

    if (AActor* Owner = GetOwner())
    {
        InitializeAgents(Owner->GetActorLocation(), 600.f);
    }
}

void UCrowd_StampedeController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (Agents.Num() == 0) return;

    PropagatePanic(DeltaTime);
    UpdateFlocking(DeltaTime);
    UpdateState();

    StateTimer += DeltaTime;
}

void UCrowd_StampedeController::InitializeAgents(FVector CenterLocation, float SpawnRadius)
{
    Agents.Empty();
    Agents.Reserve(AgentCount);

    for (int32 i = 0; i < AgentCount; ++i)
    {
        FCrowd_StampedeAgent Agent;
        float Angle = FMath::RandRange(0.f, 360.f);
        float Dist = FMath::RandRange(0.f, SpawnRadius);
        Agent.Location = CenterLocation + FVector(
            FMath::Cos(FMath::DegreesToRadians(Angle)) * Dist,
            FMath::Sin(FMath::DegreesToRadians(Angle)) * Dist,
            0.f
        );
        Agent.Speed = FMath::RandRange(500.f, 750.f);
        Agent.PanicLevel = 0.f;
        Agent.bIsLeader = (i == 0); // First agent is herd leader
        Agents.Add(Agent);
    }
}

void UCrowd_StampedeController::TriggerStampede(const FCrowd_StampedeTrigger& Trigger)
{
    CurrentState = ECrowd_StampedeState::Alert;
    StateTimer = 0.f;

    // Propagate initial panic to agents within trigger radius
    for (FCrowd_StampedeAgent& Agent : Agents)
    {
        float Dist = FVector::Dist(Agent.Location, Trigger.TriggerLocation);
        if (Dist <= Trigger.TriggerRadius)
        {
            float PanicFactor = 1.f - (Dist / Trigger.TriggerRadius);
            Agent.PanicLevel = FMath::Clamp(Agent.PanicLevel + Trigger.PanicIntensity * PanicFactor, 0.f, 1.f);
        }
    }

    // Set stampede direction away from trigger
    if (AActor* Owner = GetOwner())
    {
        StampedeDirection = (Owner->GetActorLocation() - Trigger.TriggerLocation).GetSafeNormal();
        StampedeDirection.Z = 0.f;
    }
}

void UCrowd_StampedeController::SetStampedeDirection(FVector Direction)
{
    StampedeDirection = Direction.GetSafeNormal();
    StampedeDirection.Z = 0.f;
}

void UCrowd_StampedeController::CalmHerd()
{
    CurrentState = ECrowd_StampedeState::Dispersing;
    StateTimer = 0.f;
    for (FCrowd_StampedeAgent& Agent : Agents)
    {
        Agent.PanicLevel = FMath::Max(0.f, Agent.PanicLevel - 0.5f);
    }
}

int32 UCrowd_StampedeController::GetAgentsInRadius(FVector Center, float Radius) const
{
    int32 Count = 0;
    for (const FCrowd_StampedeAgent& Agent : Agents)
    {
        if (FVector::Dist(Agent.Location, Center) <= Radius)
        {
            ++Count;
        }
    }
    return Count;
}

void UCrowd_StampedeController::PropagatePanic(float DeltaTime)
{
    float TotalPanic = 0.f;

    for (int32 i = 0; i < Agents.Num(); ++i)
    {
        FCrowd_StampedeAgent& AgentA = Agents[i];

        // Propagate panic from nearby panicking agents
        for (int32 j = 0; j < Agents.Num(); ++j)
        {
            if (i == j) continue;
            const FCrowd_StampedeAgent& AgentB = Agents[j];
            float Dist = FVector::Dist(AgentA.Location, AgentB.Location);
            if (Dist < PanicPropagationRadius && AgentB.PanicLevel > 0.3f)
            {
                float PropagationStrength = (1.f - Dist / PanicPropagationRadius) * AgentB.PanicLevel * 0.5f;
                AgentA.PanicLevel = FMath::Clamp(AgentA.PanicLevel + PropagationStrength * DeltaTime, 0.f, 1.f);
            }
        }

        // Decay panic over time when not stampeding
        if (CurrentState == ECrowd_StampedeState::Dispersing || CurrentState == ECrowd_StampedeState::Settled)
        {
            AgentA.PanicLevel = FMath::Max(0.f, AgentA.PanicLevel - PanicDecayRate * DeltaTime);
        }

        TotalPanic += AgentA.PanicLevel;
    }

    AveragePanicLevel = Agents.Num() > 0 ? TotalPanic / Agents.Num() : 0.f;
}

FVector UCrowd_StampedeController::ComputeFlockingForce(int32 AgentIndex) const
{
    const FCrowd_StampedeAgent& Agent = Agents[AgentIndex];
    FVector Separation = FVector::ZeroVector;
    FVector Cohesion = FVector::ZeroVector;
    FVector Alignment = FVector::ZeroVector;
    int32 NeighborCount = 0;

    for (int32 j = 0; j < Agents.Num(); ++j)
    {
        if (j == AgentIndex) continue;
        const FCrowd_StampedeAgent& Other = Agents[j];
        float Dist = FVector::Dist(Agent.Location, Other.Location);

        if (Dist < SeparationRadius && Dist > 0.f)
        {
            Separation += (Agent.Location - Other.Location).GetSafeNormal() / Dist;
        }

        if (Dist < CohesionRadius)
        {
            Cohesion += Other.Location;
            Alignment += Other.Velocity;
            ++NeighborCount;
        }
    }

    if (NeighborCount > 0)
    {
        Cohesion = (Cohesion / NeighborCount - Agent.Location).GetSafeNormal();
        Alignment = (Alignment / NeighborCount).GetSafeNormal();
    }

    return (Separation * SeparationWeight) + (Cohesion * CohesionWeight) + (Alignment * AlignmentWeight);
}

void UCrowd_StampedeController::UpdateFlocking(float DeltaTime)
{
    for (int32 i = 0; i < Agents.Num(); ++i)
    {
        FCrowd_StampedeAgent& Agent = Agents[i];

        FVector DesiredVelocity = FVector::ZeroVector;

        if (CurrentState == ECrowd_StampedeState::Stampeding)
        {
            // Panic-driven stampede: blend flocking with stampede direction
            FVector FlockForce = ComputeFlockingForce(i);
            float PanicBlend = Agent.PanicLevel;
            DesiredVelocity = FMath::Lerp(FlockForce, StampedeDirection, PanicBlend);
            DesiredVelocity = DesiredVelocity.GetSafeNormal() * Agent.Speed * (0.5f + Agent.PanicLevel * 0.5f);
        }
        else if (CurrentState == ECrowd_StampedeState::Alert)
        {
            // Milling behavior — agents move nervously
            FVector FlockForce = ComputeFlockingForce(i);
            DesiredVelocity = FlockForce.GetSafeNormal() * Agent.Speed * 0.3f;
        }
        else if (CurrentState == ECrowd_StampedeState::Dispersing)
        {
            // Slow down and spread out
            FVector FlockForce = ComputeFlockingForce(i);
            DesiredVelocity = FlockForce.GetSafeNormal() * Agent.Speed * 0.15f;
        }

        // Smooth velocity transition
        Agent.Velocity = FMath::VInterpTo(Agent.Velocity, DesiredVelocity, DeltaTime, 3.f);
        Agent.Location += Agent.Velocity * DeltaTime;
    }
}

void UCrowd_StampedeController::UpdateState()
{
    switch (CurrentState)
    {
    case ECrowd_StampedeState::Idle:
        if (AveragePanicLevel > 0.1f)
        {
            CurrentState = ECrowd_StampedeState::Alert;
            StateTimer = 0.f;
        }
        break;

    case ECrowd_StampedeState::Alert:
        if (AveragePanicLevel > 0.4f)
        {
            CurrentState = ECrowd_StampedeState::Stampeding;
            StateTimer = 0.f;
        }
        else if (AveragePanicLevel < 0.05f && StateTimer > 5.f)
        {
            CurrentState = ECrowd_StampedeState::Settled;
            StateTimer = 0.f;
        }
        break;

    case ECrowd_StampedeState::Stampeding:
        if (AveragePanicLevel < 0.2f)
        {
            CurrentState = ECrowd_StampedeState::Dispersing;
            StateTimer = 0.f;
        }
        break;

    case ECrowd_StampedeState::Dispersing:
        if (AveragePanicLevel < 0.02f && StateTimer > 8.f)
        {
            CurrentState = ECrowd_StampedeState::Settled;
            StateTimer = 0.f;
        }
        break;

    case ECrowd_StampedeState::Settled:
        if (AveragePanicLevel < 0.01f && StateTimer > 10.f)
        {
            CurrentState = ECrowd_StampedeState::Idle;
            StateTimer = 0.f;
        }
        break;
    }
}
