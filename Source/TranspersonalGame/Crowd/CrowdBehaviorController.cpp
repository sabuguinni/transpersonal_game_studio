#include "CrowdBehaviorController.h"
#include "Math/UnrealMathUtility.h"

ACrowdBehaviorController::ACrowdBehaviorController()
{
    PrimaryActorTick.bCanEverTick = true;

    MaxHerdSize = 30;
    HerdSpecies = ECrowd_HerdSpecies::Gallimimus;
    SpawnRadius = 2000.0f;
    FlockingRadius = 600.0f;
    SeparationRadius = 150.0f;
    MaxSpeed = 800.0f;
    FearDecayRate = 0.5f;
    bStampedeActive = false;
}

void ACrowdBehaviorController::BeginPlay()
{
    Super::BeginPlay();
    InitializeHerd();
}

void ACrowdBehaviorController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateFlockingBehavior(DeltaTime);
}

void ACrowdBehaviorController::InitializeHerd()
{
    HerdAgents.Empty();
    HerdAgents.Reserve(MaxHerdSize);

    FVector Origin = GetActorLocation();

    for (int32 i = 0; i < MaxHerdSize; ++i)
    {
        FCrowd_HerdAgent Agent;
        float Angle = FMath::RandRange(0.0f, 360.0f);
        float Dist = FMath::RandRange(0.0f, SpawnRadius);
        Agent.Location = Origin + FVector(
            FMath::Cos(FMath::DegreesToRadians(Angle)) * Dist,
            FMath::Sin(FMath::DegreesToRadians(Angle)) * Dist,
            0.0f
        );
        Agent.Velocity = FVector(
            FMath::RandRange(-100.0f, 100.0f),
            FMath::RandRange(-100.0f, 100.0f),
            0.0f
        );
        Agent.State = ECrowd_AgentState::Grazing;
        Agent.Species = HerdSpecies;
        Agent.FearLevel = 0.0f;
        Agent.StaminaLevel = 100.0f;
        HerdAgents.Add(Agent);
    }

    UE_LOG(LogTemp, Log, TEXT("CrowdBehaviorController: Initialized herd of %d agents (Species: %d)"),
        HerdAgents.Num(), (int32)HerdSpecies);
}

void ACrowdBehaviorController::TriggerStampede(FVector ThreatLocation, float ThreatRadius)
{
    bStampedeActive = true;
    ActiveStampede.TriggerLocation = ThreatLocation;
    ActiveStampede.TriggerRadius = ThreatRadius;
    ActiveStampede.StampedeSpeed = MaxSpeed * 1.5f;
    ActiveStampede.AffectedAgentCount = 0;

    // Flee direction = away from threat
    FVector ToThreat = ThreatLocation - GetActorLocation();
    ActiveStampede.FleeDirection = (-ToThreat).GetSafeNormal();

    for (FCrowd_HerdAgent& Agent : HerdAgents)
    {
        float DistToThreat = FVector::Dist(Agent.Location, ThreatLocation);
        if (DistToThreat <= ThreatRadius)
        {
            Agent.State = ECrowd_AgentState::Stampeding;
            Agent.FearLevel = 1.0f;
            Agent.Velocity = ActiveStampede.FleeDirection * ActiveStampede.StampedeSpeed;
            ++ActiveStampede.AffectedAgentCount;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("CrowdBehaviorController: Stampede triggered! %d agents affected"),
        ActiveStampede.AffectedAgentCount);
}

void ACrowdBehaviorController::UpdateFlockingBehavior(float DeltaTime)
{
    for (int32 i = 0; i < HerdAgents.Num(); ++i)
    {
        FCrowd_HerdAgent& Agent = HerdAgents[i];

        // Decay fear over time
        if (Agent.FearLevel > 0.0f)
        {
            Agent.FearLevel = FMath::Max(0.0f, Agent.FearLevel - FearDecayRate * DeltaTime);
        }

        // Transition out of stampede when fear drops
        if (Agent.State == ECrowd_AgentState::Stampeding && Agent.FearLevel <= 0.1f)
        {
            Agent.State = ECrowd_AgentState::Grazing;
        }

        if (Agent.State == ECrowd_AgentState::Grazing || Agent.State == ECrowd_AgentState::Foraging)
        {
            // Apply flocking rules
            FVector Sep = ComputeSeparation(i);
            FVector Ali = ComputeAlignment(i);
            FVector Coh = ComputeCohesion(i);

            FVector Steering = Sep * 2.0f + Ali * 1.0f + Coh * 1.0f;
            Agent.Velocity += Steering * DeltaTime;

            // Clamp to max speed (reduced when grazing)
            float GrazingSpeed = MaxSpeed * 0.2f;
            if (Agent.Velocity.Size() > GrazingSpeed)
            {
                Agent.Velocity = Agent.Velocity.GetSafeNormal() * GrazingSpeed;
            }
        }

        // Update position
        Agent.Location += Agent.Velocity * DeltaTime;
    }

    // Check if stampede is over
    if (bStampedeActive)
    {
        bool bAnyStampeding = false;
        for (const FCrowd_HerdAgent& Agent : HerdAgents)
        {
            if (Agent.State == ECrowd_AgentState::Stampeding)
            {
                bAnyStampeding = true;
                break;
            }
        }
        if (!bAnyStampeding)
        {
            bStampedeActive = false;
            UE_LOG(LogTemp, Log, TEXT("CrowdBehaviorController: Stampede ended — herd calmed"));
        }
    }
}

int32 ACrowdBehaviorController::GetActiveAgentCount() const
{
    return HerdAgents.Num();
}

void ACrowdBehaviorController::SetHerdFearLevel(float FearLevel)
{
    float ClampedFear = FMath::Clamp(FearLevel, 0.0f, 1.0f);
    for (FCrowd_HerdAgent& Agent : HerdAgents)
    {
        Agent.FearLevel = ClampedFear;
        if (ClampedFear > 0.5f)
        {
            Agent.State = ECrowd_AgentState::Fleeing;
        }
    }
}

FVector ACrowdBehaviorController::ComputeSeparation(int32 AgentIndex) const
{
    FVector Steer = FVector::ZeroVector;
    int32 Count = 0;
    const FCrowd_HerdAgent& Self = HerdAgents[AgentIndex];

    for (int32 j = 0; j < HerdAgents.Num(); ++j)
    {
        if (j == AgentIndex) continue;
        float Dist = FVector::Dist(Self.Location, HerdAgents[j].Location);
        if (Dist < SeparationRadius && Dist > 0.0f)
        {
            FVector Away = (Self.Location - HerdAgents[j].Location).GetSafeNormal();
            Steer += Away / Dist;
            ++Count;
        }
    }

    if (Count > 0) Steer /= (float)Count;
    return Steer;
}

FVector ACrowdBehaviorController::ComputeAlignment(int32 AgentIndex) const
{
    FVector AvgVelocity = FVector::ZeroVector;
    int32 Count = 0;
    const FCrowd_HerdAgent& Self = HerdAgents[AgentIndex];

    for (int32 j = 0; j < HerdAgents.Num(); ++j)
    {
        if (j == AgentIndex) continue;
        float Dist = FVector::Dist(Self.Location, HerdAgents[j].Location);
        if (Dist < FlockingRadius)
        {
            AvgVelocity += HerdAgents[j].Velocity;
            ++Count;
        }
    }

    if (Count > 0)
    {
        AvgVelocity /= (float)Count;
        return (AvgVelocity - Self.Velocity) * 0.1f;
    }
    return FVector::ZeroVector;
}

FVector ACrowdBehaviorController::ComputeCohesion(int32 AgentIndex) const
{
    FVector Center = FVector::ZeroVector;
    int32 Count = 0;
    const FCrowd_HerdAgent& Self = HerdAgents[AgentIndex];

    for (int32 j = 0; j < HerdAgents.Num(); ++j)
    {
        if (j == AgentIndex) continue;
        float Dist = FVector::Dist(Self.Location, HerdAgents[j].Location);
        if (Dist < FlockingRadius)
        {
            Center += HerdAgents[j].Location;
            ++Count;
        }
    }

    if (Count > 0)
    {
        Center /= (float)Count;
        return (Center - Self.Location) * 0.01f;
    }
    return FVector::ZeroVector;
}
