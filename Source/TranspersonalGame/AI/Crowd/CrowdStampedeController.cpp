#include "CrowdStampedeController.h"
#include "Math/UnrealMathUtility.h"

UCrowdStampedeController::UCrowdStampedeController()
{
    PrimaryComponentTick.bCanEverTick = true;
    MaxAgents = 200;
    SeparationRadius = 120.0f;
    CohesionRadius = 400.0f;
    AlignmentRadius = 300.0f;
    PanicDecayRate = 0.15f;
    StampedeSpeedMultiplier = 2.5f;
}

void UCrowdStampedeController::BeginPlay()
{
    Super::BeginPlay();
    Agents.Reserve(MaxAgents);
}

void UCrowdStampedeController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Decay active triggers
    for (int32 i = ActiveTriggers.Num() - 1; i >= 0; --i)
    {
        ActiveTriggers[i].Duration -= DeltaTime;
        if (ActiveTriggers[i].Duration <= 0.0f)
        {
            ActiveTriggers.RemoveAt(i);
        }
    }

    // Update all agents
    for (FCrowd_StampedeAgent& Agent : Agents)
    {
        UpdateAgentPanic(Agent, DeltaTime);
        UpdateAgentFlocking(Agent, DeltaTime);
        Agent.Location += Agent.Velocity * DeltaTime;
    }
}

void UCrowdStampedeController::TriggerStampede(FVector Origin, float Radius, float PanicStrength)
{
    FCrowd_StampedeTrigger NewTrigger;
    NewTrigger.TriggerLocation = Origin;
    NewTrigger.TriggerRadius = Radius;
    NewTrigger.PanicStrength = PanicStrength;
    NewTrigger.Duration = 12.0f;
    ActiveTriggers.Add(NewTrigger);

    // Immediately panic nearby agents
    for (FCrowd_StampedeAgent& Agent : Agents)
    {
        float Dist = FVector::Dist(Agent.Location, Origin);
        if (Dist <= Radius)
        {
            float PanicAmount = PanicStrength * (1.0f - (Dist / Radius));
            Agent.PanicLevel = FMath::Clamp(Agent.PanicLevel + PanicAmount, 0.0f, 1.0f);
            Agent.State = ECrowd_StampedeState::Stampeding;
        }
    }
}

void UCrowdStampedeController::AddAgent(FVector Location, float Speed)
{
    if (Agents.Num() >= MaxAgents) return;

    FCrowd_StampedeAgent NewAgent;
    NewAgent.Location = Location;
    NewAgent.Speed = Speed;
    NewAgent.PanicLevel = 0.0f;
    NewAgent.State = ECrowd_StampedeState::Calm;
    NewAgent.Velocity = FVector::ZeroVector;
    Agents.Add(NewAgent);
}

void UCrowdStampedeController::ClearAllAgents()
{
    Agents.Empty();
    ActiveTriggers.Empty();
}

int32 UCrowdStampedeController::GetActiveAgentCount() const
{
    return Agents.Num();
}

ECrowd_StampedeState UCrowdStampedeController::GetOverallHerdState() const
{
    if (Agents.Num() == 0) return ECrowd_StampedeState::Calm;

    int32 StampedeCount = 0;
    int32 FleeingCount = 0;
    int32 AlertCount = 0;

    for (const FCrowd_StampedeAgent& Agent : Agents)
    {
        switch (Agent.State)
        {
        case ECrowd_StampedeState::Stampeding: ++StampedeCount; break;
        case ECrowd_StampedeState::Fleeing:    ++FleeingCount;  break;
        case ECrowd_StampedeState::Alert:      ++AlertCount;    break;
        default: break;
        }
    }

    float Total = (float)Agents.Num();
    if ((float)StampedeCount / Total > 0.5f) return ECrowd_StampedeState::Stampeding;
    if ((float)FleeingCount  / Total > 0.3f) return ECrowd_StampedeState::Fleeing;
    if ((float)AlertCount    / Total > 0.4f) return ECrowd_StampedeState::Alert;
    return ECrowd_StampedeState::Calm;
}

void UCrowdStampedeController::SetMaxAgents(int32 NewMax)
{
    MaxAgents = FMath::Clamp(NewMax, 1, 50000);
    Agents.Reserve(MaxAgents);
}

void UCrowdStampedeController::UpdateAgentPanic(FCrowd_StampedeAgent& Agent, float DeltaTime)
{
    // Apply panic from active triggers
    for (const FCrowd_StampedeTrigger& Trigger : ActiveTriggers)
    {
        float Dist = FVector::Dist(Agent.Location, Trigger.TriggerLocation);
        if (Dist <= Trigger.TriggerRadius)
        {
            float PanicAdd = Trigger.PanicStrength * (1.0f - (Dist / Trigger.TriggerRadius)) * DeltaTime;
            Agent.PanicLevel = FMath::Clamp(Agent.PanicLevel + PanicAdd, 0.0f, 1.0f);
        }
    }

    // Decay panic over time
    Agent.PanicLevel = FMath::Clamp(Agent.PanicLevel - PanicDecayRate * DeltaTime, 0.0f, 1.0f);

    // Update state from panic level
    if (Agent.PanicLevel > 0.8f)
        Agent.State = ECrowd_StampedeState::Stampeding;
    else if (Agent.PanicLevel > 0.5f)
        Agent.State = ECrowd_StampedeState::Fleeing;
    else if (Agent.PanicLevel > 0.2f)
        Agent.State = ECrowd_StampedeState::Alert;
    else
        Agent.State = ECrowd_StampedeState::Calm;
}

void UCrowdStampedeController::UpdateAgentFlocking(FCrowd_StampedeAgent& Agent, float DeltaTime)
{
    FVector Separation = ComputeSeparationForce(Agent);
    FVector Cohesion   = ComputeCohesionForce(Agent);
    FVector Alignment  = ComputeAlignmentForce(Agent);
    FVector PanicFlee  = ComputePanicFleeForce(Agent);

    float CurrentSpeed = Agent.Speed;
    if (Agent.State == ECrowd_StampedeState::Stampeding || Agent.State == ECrowd_StampedeState::Fleeing)
    {
        CurrentSpeed *= StampedeSpeedMultiplier * Agent.PanicLevel;
    }

    FVector DesiredVelocity = (Separation * 1.5f) + (Cohesion * 0.8f) + (Alignment * 1.0f) + (PanicFlee * 2.0f * Agent.PanicLevel);

    if (!DesiredVelocity.IsNearlyZero())
    {
        DesiredVelocity = DesiredVelocity.GetSafeNormal() * CurrentSpeed;
    }

    // Smooth velocity transition
    Agent.Velocity = FMath::VInterpTo(Agent.Velocity, DesiredVelocity, DeltaTime, 3.0f);
}

FVector UCrowdStampedeController::ComputeSeparationForce(const FCrowd_StampedeAgent& Agent) const
{
    FVector Force = FVector::ZeroVector;
    int32 Count = 0;

    for (const FCrowd_StampedeAgent& Other : Agents)
    {
        if (&Other == &Agent) continue;
        float Dist = FVector::Dist(Agent.Location, Other.Location);
        if (Dist < SeparationRadius && Dist > KINDA_SMALL_NUMBER)
        {
            FVector Away = (Agent.Location - Other.Location).GetSafeNormal();
            Force += Away * (1.0f - (Dist / SeparationRadius));
            ++Count;
        }
    }

    return Count > 0 ? Force / (float)Count : FVector::ZeroVector;
}

FVector UCrowdStampedeController::ComputeCohesionForce(const FCrowd_StampedeAgent& Agent) const
{
    FVector CenterOfMass = FVector::ZeroVector;
    int32 Count = 0;

    for (const FCrowd_StampedeAgent& Other : Agents)
    {
        if (&Other == &Agent) continue;
        float Dist = FVector::Dist(Agent.Location, Other.Location);
        if (Dist < CohesionRadius)
        {
            CenterOfMass += Other.Location;
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

FVector UCrowdStampedeController::ComputeAlignmentForce(const FCrowd_StampedeAgent& Agent) const
{
    FVector AverageVelocity = FVector::ZeroVector;
    int32 Count = 0;

    for (const FCrowd_StampedeAgent& Other : Agents)
    {
        if (&Other == &Agent) continue;
        float Dist = FVector::Dist(Agent.Location, Other.Location);
        if (Dist < AlignmentRadius && !Other.Velocity.IsNearlyZero())
        {
            AverageVelocity += Other.Velocity.GetSafeNormal();
            ++Count;
        }
    }

    return Count > 0 ? (AverageVelocity / (float)Count).GetSafeNormal() : FVector::ZeroVector;
}

FVector UCrowdStampedeController::ComputePanicFleeForce(const FCrowd_StampedeAgent& Agent) const
{
    if (ActiveTriggers.Num() == 0 || Agent.PanicLevel < 0.1f)
        return FVector::ZeroVector;

    FVector FleeForce = FVector::ZeroVector;
    for (const FCrowd_StampedeTrigger& Trigger : ActiveTriggers)
    {
        float Dist = FVector::Dist(Agent.Location, Trigger.TriggerLocation);
        if (Dist < Trigger.TriggerRadius * 2.0f && Dist > KINDA_SMALL_NUMBER)
        {
            FVector Away = (Agent.Location - Trigger.TriggerLocation).GetSafeNormal();
            FleeForce += Away * Trigger.PanicStrength;
        }
    }

    return FleeForce.GetSafeNormal();
}
