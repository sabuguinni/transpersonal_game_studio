#include "CrowdStampedeController.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"

ACrowdStampedeController::ACrowdStampedeController()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.05f; // 20Hz tick for crowd simulation

    MaxAgents = 200;
    SpawnRadius = 2000.0f;
    HerdOrigin = FVector::ZeroVector;
    CurrentHerdState = ECrowd_StampedeState::Grazing;
    ActiveAgentCount = 0;
    AverageFearLevel = 0.0f;
    StampedeDirection = FVector::ForwardVector;
    ThreatPosition = FVector::ZeroVector;
    bStampedeActive = false;
    StampedeTimer = 0.0f;
    CalmdownThreshold = 30.0f;
}

void ACrowdStampedeController::BeginPlay()
{
    Super::BeginPlay();

    // Auto-spawn herd on begin play
    if (MaxAgents > 0)
    {
        SpawnHerd(MaxAgents, GetActorLocation(), SpawnRadius);
    }
}

void ACrowdStampedeController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (HerdAgents.Num() == 0) return;

    UpdateFlocking(DeltaTime);
    UpdateFearPropagation(DeltaTime);
    UpdateHerdState();

    // Stampede auto-calm after duration
    if (bStampedeActive)
    {
        StampedeTimer += DeltaTime;
        if (StampedeTimer >= CalmdownThreshold)
        {
            CalmHerd();
        }
    }

    // Update stats
    ActiveAgentCount = HerdAgents.Num();
    float TotalFear = 0.0f;
    for (const FCrowd_HerdAgent& Agent : HerdAgents)
    {
        TotalFear += Agent.FearLevel;
    }
    AverageFearLevel = (ActiveAgentCount > 0) ? (TotalFear / ActiveAgentCount) : 0.0f;
}

void ACrowdStampedeController::SpawnHerd(int32 AgentCount, FVector Origin, float Radius)
{
    HerdAgents.Empty();
    HerdAgents.Reserve(AgentCount);

    for (int32 i = 0; i < AgentCount; ++i)
    {
        FCrowd_HerdAgent NewAgent;
        NewAgent.AgentID = i;

        // Random position within radius using polar coordinates
        float Angle = FMath::RandRange(0.0f, 360.0f);
        float Dist = FMath::RandRange(0.0f, Radius);
        NewAgent.Location = Origin + FVector(
            FMath::Cos(FMath::DegreesToRadians(Angle)) * Dist,
            FMath::Sin(FMath::DegreesToRadians(Angle)) * Dist,
            0.0f
        );

        NewAgent.Velocity = FVector::ZeroVector;
        NewAgent.State = ECrowd_StampedeState::Grazing;
        NewAgent.FearLevel = 0.0f;

        HerdAgents.Add(NewAgent);
    }

    ActiveAgentCount = HerdAgents.Num();
    UE_LOG(LogTemp, Log, TEXT("CrowdStampedeController: Spawned %d herd agents at %s"), AgentCount, *Origin.ToString());
}

void ACrowdStampedeController::TriggerStampede(FVector ThreatLocation, float FearIntensity)
{
    ThreatPosition = ThreatLocation;
    bStampedeActive = true;
    StampedeTimer = 0.0f;
    CurrentHerdState = ECrowd_StampedeState::Stampeding;

    // Immediately set fear on nearby agents
    float TriggerRadiusSq = StampedeConfig.TriggerRadius * StampedeConfig.TriggerRadius;
    for (FCrowd_HerdAgent& Agent : HerdAgents)
    {
        float DistSq = FVector::DistSquared(Agent.Location, ThreatLocation);
        if (DistSq < TriggerRadiusSq)
        {
            float Proximity = 1.0f - (FMath::Sqrt(DistSq) / StampedeConfig.TriggerRadius);
            Agent.FearLevel = FMath::Clamp(FearIntensity * Proximity, 0.0f, 1.0f);
            Agent.State = ECrowd_StampedeState::Fleeing;
        }
    }

    // Compute initial stampede direction — away from threat
    FVector HerdCenter = FVector::ZeroVector;
    for (const FCrowd_HerdAgent& Agent : HerdAgents)
    {
        HerdCenter += Agent.Location;
    }
    if (HerdAgents.Num() > 0)
    {
        HerdCenter /= HerdAgents.Num();
        StampedeDirection = (HerdCenter - ThreatLocation).GetSafeNormal();
    }

    OnStampedeTriggered(ThreatLocation, FearIntensity);
    UE_LOG(LogTemp, Warning, TEXT("CrowdStampedeController: STAMPEDE TRIGGERED! Threat at %s, Fear=%.2f"), *ThreatLocation.ToString(), FearIntensity);
}

void ACrowdStampedeController::CalmHerd()
{
    bStampedeActive = false;
    StampedeTimer = 0.0f;
    CurrentHerdState = ECrowd_StampedeState::Idle;

    for (FCrowd_HerdAgent& Agent : HerdAgents)
    {
        Agent.FearLevel = 0.0f;
        Agent.State = ECrowd_StampedeState::Grazing;
        Agent.Velocity *= 0.1f;
    }

    OnHerdCalmed();
    UE_LOG(LogTemp, Log, TEXT("CrowdStampedeController: Herd calmed. %d agents returning to grazing."), HerdAgents.Num());
}

void ACrowdStampedeController::UpdateFlocking(float DeltaTime)
{
    const float NeighborRadius = 300.0f;
    const float NeighborRadiusSq = NeighborRadius * NeighborRadius;

    for (int32 i = 0; i < HerdAgents.Num(); ++i)
    {
        FCrowd_HerdAgent& Agent = HerdAgents[i];

        FVector Separation = ComputeSeparation(i);
        FVector Alignment = ComputeAlignment(i);
        FVector Cohesion = ComputeCohesion(i);
        FVector FearFlight = ComputeFearFlight(i);

        // Weighted steering
        FVector Steering = FVector::ZeroVector;
        Steering += Separation * StampedeConfig.SeparationWeight;
        Steering += Alignment * StampedeConfig.AlignmentWeight;
        Steering += Cohesion * StampedeConfig.CohesionWeight;

        // Fear overrides normal flocking
        if (Agent.FearLevel > 0.1f)
        {
            Steering += FearFlight * (Agent.FearLevel * 3.0f);
        }

        // Apply steering to velocity
        Agent.Velocity += Steering * DeltaTime * 100.0f;

        // Clamp to max speed (fear increases max speed)
        float MaxSpd = StampedeConfig.MaxSpeed * (1.0f + Agent.FearLevel * 0.5f);
        float CurrentSpeed = Agent.Velocity.Size();
        if (CurrentSpeed > MaxSpd)
        {
            Agent.Velocity = Agent.Velocity.GetSafeNormal() * MaxSpd;
        }

        // Update position
        Agent.Location += Agent.Velocity * DeltaTime;

        // Flatten Z (ground-based movement)
        Agent.Location.Z = 0.0f;
    }
}

void ACrowdStampedeController::UpdateFearPropagation(float DeltaTime)
{
    const float PropagationRadius = 400.0f;
    const float PropagationRadiusSq = PropagationRadius * PropagationRadius;

    // Fear spreads from frightened agents to nearby calm agents
    TArray<float> NewFearLevels;
    NewFearLevels.SetNum(HerdAgents.Num());

    for (int32 i = 0; i < HerdAgents.Num(); ++i)
    {
        float MaxNeighborFear = HerdAgents[i].FearLevel;

        for (int32 j = 0; j < HerdAgents.Num(); ++j)
        {
            if (i == j) continue;
            float DistSq = FVector::DistSquared(HerdAgents[i].Location, HerdAgents[j].Location);
            if (DistSq < PropagationRadiusSq)
            {
                MaxNeighborFear = FMath::Max(MaxNeighborFear, HerdAgents[j].FearLevel * 0.8f);
            }
        }

        // Propagate fear, decay over time
        float PropagatedFear = FMath::Lerp(HerdAgents[i].FearLevel, MaxNeighborFear, StampedeConfig.FearPropagationRate * DeltaTime);
        float DecayedFear = PropagatedFear - (0.05f * DeltaTime); // Fear decays when no threat nearby
        NewFearLevels[i] = FMath::Clamp(DecayedFear, 0.0f, 1.0f);
    }

    // Apply new fear levels and update states
    for (int32 i = 0; i < HerdAgents.Num(); ++i)
    {
        HerdAgents[i].FearLevel = NewFearLevels[i];

        if (HerdAgents[i].FearLevel > 0.7f)
            HerdAgents[i].State = ECrowd_StampedeState::Stampeding;
        else if (HerdAgents[i].FearLevel > 0.3f)
            HerdAgents[i].State = ECrowd_StampedeState::Fleeing;
        else if (HerdAgents[i].FearLevel > 0.05f)
            HerdAgents[i].State = ECrowd_StampedeState::Alert;
        else
            HerdAgents[i].State = ECrowd_StampedeState::Grazing;
    }
}

void ACrowdStampedeController::UpdateHerdState()
{
    int32 StampedeCount = 0, FleeCount = 0, AlertCount = 0;
    for (const FCrowd_HerdAgent& Agent : HerdAgents)
    {
        if (Agent.State == ECrowd_StampedeState::Stampeding) StampedeCount++;
        else if (Agent.State == ECrowd_StampedeState::Fleeing) FleeCount++;
        else if (Agent.State == ECrowd_StampedeState::Alert) AlertCount++;
    }

    int32 Total = HerdAgents.Num();
    if (Total == 0) return;

    if ((float)StampedeCount / Total > 0.5f)
        CurrentHerdState = ECrowd_StampedeState::Stampeding;
    else if ((float)FleeCount / Total > 0.3f)
        CurrentHerdState = ECrowd_StampedeState::Fleeing;
    else if ((float)AlertCount / Total > 0.2f)
        CurrentHerdState = ECrowd_StampedeState::Alert;
    else
        CurrentHerdState = ECrowd_StampedeState::Grazing;
}

FVector ACrowdStampedeController::ComputeSeparation(int32 AgentIndex) const
{
    const float SepRadius = 150.0f;
    const float SepRadiusSq = SepRadius * SepRadius;
    FVector Steer = FVector::ZeroVector;
    int32 Count = 0;

    for (int32 j = 0; j < HerdAgents.Num(); ++j)
    {
        if (j == AgentIndex) continue;
        float DistSq = FVector::DistSquared(HerdAgents[AgentIndex].Location, HerdAgents[j].Location);
        if (DistSq < SepRadiusSq && DistSq > 0.0f)
        {
            FVector Away = HerdAgents[AgentIndex].Location - HerdAgents[j].Location;
            Away = Away.GetSafeNormal() / FMath::Sqrt(DistSq);
            Steer += Away;
            Count++;
        }
    }

    if (Count > 0) Steer /= Count;
    return Steer;
}

FVector ACrowdStampedeController::ComputeAlignment(int32 AgentIndex) const
{
    const float AlignRadius = 250.0f;
    const float AlignRadiusSq = AlignRadius * AlignRadius;
    FVector AvgVelocity = FVector::ZeroVector;
    int32 Count = 0;

    for (int32 j = 0; j < HerdAgents.Num(); ++j)
    {
        if (j == AgentIndex) continue;
        float DistSq = FVector::DistSquared(HerdAgents[AgentIndex].Location, HerdAgents[j].Location);
        if (DistSq < AlignRadiusSq)
        {
            AvgVelocity += HerdAgents[j].Velocity;
            Count++;
        }
    }

    if (Count > 0)
    {
        AvgVelocity /= Count;
        return (AvgVelocity - HerdAgents[AgentIndex].Velocity).GetSafeNormal();
    }
    return FVector::ZeroVector;
}

FVector ACrowdStampedeController::ComputeCohesion(int32 AgentIndex) const
{
    const float CohRadius = 350.0f;
    const float CohRadiusSq = CohRadius * CohRadius;
    FVector CenterOfMass = FVector::ZeroVector;
    int32 Count = 0;

    for (int32 j = 0; j < HerdAgents.Num(); ++j)
    {
        if (j == AgentIndex) continue;
        float DistSq = FVector::DistSquared(HerdAgents[AgentIndex].Location, HerdAgents[j].Location);
        if (DistSq < CohRadiusSq)
        {
            CenterOfMass += HerdAgents[j].Location;
            Count++;
        }
    }

    if (Count > 0)
    {
        CenterOfMass /= Count;
        return (CenterOfMass - HerdAgents[AgentIndex].Location).GetSafeNormal();
    }
    return FVector::ZeroVector;
}

FVector ACrowdStampedeController::ComputeFearFlight(int32 AgentIndex) const
{
    if (!bStampedeActive) return FVector::ZeroVector;
    FVector AwayFromThreat = HerdAgents[AgentIndex].Location - ThreatPosition;
    return AwayFromThreat.GetSafeNormal();
}

float ACrowdStampedeController::GetHerdFearLevel() const
{
    return AverageFearLevel;
}

FVector ACrowdStampedeController::GetStampedeDirection() const
{
    return StampedeDirection;
}

int32 ACrowdStampedeController::GetAgentsInRadius(FVector Center, float Radius) const
{
    float RadiusSq = Radius * Radius;
    int32 Count = 0;
    for (const FCrowd_HerdAgent& Agent : HerdAgents)
    {
        if (FVector::DistSquared(Agent.Location, Center) < RadiusSq)
        {
            Count++;
        }
    }
    return Count;
}

void ACrowdStampedeController::DebugDrawHerd()
{
#if WITH_EDITOR
    UWorld* World = GetWorld();
    if (!World) return;

    for (const FCrowd_HerdAgent& Agent : HerdAgents)
    {
        FColor DebugColor = FColor::Green;
        if (Agent.State == ECrowd_StampedeState::Stampeding) DebugColor = FColor::Red;
        else if (Agent.State == ECrowd_StampedeState::Fleeing) DebugColor = FColor::Orange;
        else if (Agent.State == ECrowd_StampedeState::Alert) DebugColor = FColor::Yellow;

        DrawDebugSphere(World, Agent.Location, 50.0f, 8, DebugColor, false, 2.0f);
        if (!Agent.Velocity.IsNearlyZero())
        {
            DrawDebugArrow(World, Agent.Location, Agent.Location + Agent.Velocity.GetSafeNormal() * 100.0f, 20.0f, DebugColor, false, 2.0f);
        }
    }

    // Draw stampede direction
    if (bStampedeActive)
    {
        FVector HerdCenter = FVector::ZeroVector;
        for (const FCrowd_HerdAgent& Agent : HerdAgents) HerdCenter += Agent.Location;
        if (HerdAgents.Num() > 0)
        {
            HerdCenter /= HerdAgents.Num();
            DrawDebugArrow(World, HerdCenter, HerdCenter + StampedeDirection * 500.0f, 50.0f, FColor::Red, false, 2.0f);
        }
    }
#endif
}
