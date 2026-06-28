// CrowdStampedeController.cpp
// Agent #13 — Crowd & Traffic Simulation
// Stampede system implementation: flocking AI, tribal reactions, LOD-aware crowd

#include "CrowdStampedeController.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Math/UnrealMathUtility.h"

ACrowdStampedeController::ACrowdStampedeController()
{
    PrimaryActorTick.bCanEverTick = true;

    HerdSize = 8;
    StampedeSpeed = 1200.0f;
    CalmSpeed = 300.0f;
    PanicDecayTime = 15.0f;
    SeparationDistance = 200.0f;
    CohesionStrength = 0.3f;
    AlignmentStrength = 0.5f;

    CurrentStampedeState = ECrowd_StampedeState::Calm;
    StampedeDirection = FVector(1.0f, 1.0f, 0.0f).GetSafeNormal();
    PanicTimer = 0.0f;
}

void ACrowdStampedeController::BeginPlay()
{
    Super::BeginPlay();

    // Initialize herd agents at spawn location
    FVector BaseLocation = GetActorLocation();
    for (int32 i = 0; i < HerdSize; i++)
    {
        FCrowd_HerdAgent Agent;
        float OffsetX = FMath::RandRange(-400.0f, 400.0f);
        float OffsetY = FMath::RandRange(-300.0f, 300.0f);
        Agent.Position = BaseLocation + FVector(OffsetX, OffsetY, 0.0f);
        Agent.Velocity = FVector::ZeroVector;
        Agent.State = ECrowd_StampedeState::Calm;
        HerdAgents.Add(Agent);
    }

    // Initialize tribal agents at camp positions
    TArray<FVector> CampOffsets = {
        FVector(-200, 100, 0), FVector(-300, -100, 0), FVector(-100, 200, 0),
        FVector(100, -50, 0), FVector(200, 100, 0), FVector(-400, 50, 0),
        FVector(0, -200, 0), FVector(-150, 300, 0), FVector(300, -150, 0)
    };

    TArray<ECrowd_TribalReaction> Reactions = {
        ECrowd_TribalReaction::Idle, ECrowd_TribalReaction::Idle, ECrowd_TribalReaction::Idle,
        ECrowd_TribalReaction::Idle, ECrowd_TribalReaction::Idle, ECrowd_TribalReaction::Idle,
        ECrowd_TribalReaction::Idle, ECrowd_TribalReaction::Idle, ECrowd_TribalReaction::Idle
    };

    for (int32 i = 0; i < CampOffsets.Num(); i++)
    {
        FCrowd_TribalAgent Agent;
        Agent.Position = BaseLocation + CampOffsets[i];
        Agent.Reaction = Reactions[i];
        Agent.bIsProtector = (i >= 7); // Last 2 are protectors
        TribalAgents.Add(Agent);
    }
}

void ACrowdStampedeController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Decay panic timer
    if (CurrentStampedeState == ECrowd_StampedeState::Panic)
    {
        PanicTimer -= DeltaTime;
        if (PanicTimer <= 0.0f)
        {
            CurrentStampedeState = ECrowd_StampedeState::Dispersing;
        }
    }
    else if (CurrentStampedeState == ECrowd_StampedeState::Dispersing)
    {
        PanicTimer -= DeltaTime;
        if (PanicTimer <= -PanicDecayTime)
        {
            CalmHerd();
        }
    }

    // Update flocking every tick when active
    if (CurrentStampedeState != ECrowd_StampedeState::Calm)
    {
        UpdateHerdFlocking(DeltaTime);
    }
}

void ACrowdStampedeController::TriggerStampede(FVector PredatorLocation, float PredatorThreatRadius)
{
    if (CurrentStampedeState == ECrowd_StampedeState::Panic)
    {
        return; // Already panicking — reinforce direction
    }

    // Compute flee direction: away from predator
    FVector HerdCenter = FVector::ZeroVector;
    for (const FCrowd_HerdAgent& Agent : HerdAgents)
    {
        HerdCenter += Agent.Position;
    }
    if (HerdAgents.Num() > 0)
    {
        HerdCenter /= HerdAgents.Num();
    }

    StampedeDirection = (HerdCenter - PredatorLocation).GetSafeNormal2D();
    CurrentStampedeState = ECrowd_StampedeState::Panic;
    PanicTimer = PanicDecayTime;

    // Set all agents to panic state
    for (FCrowd_HerdAgent& Agent : HerdAgents)
    {
        Agent.State = ECrowd_StampedeState::Panic;
        // Initial velocity burst in flee direction
        Agent.Velocity = StampedeDirection * StampedeSpeed;
    }

    UE_LOG(LogTemp, Warning, TEXT("CrowdStampedeController: STAMPEDE TRIGGERED! Direction: %s"),
        *StampedeDirection.ToString());
}

void ACrowdStampedeController::UpdateHerdFlocking(float DeltaTime)
{
    float CurrentSpeed = (CurrentStampedeState == ECrowd_StampedeState::Panic)
        ? StampedeSpeed : CalmSpeed;

    for (int32 i = 0; i < HerdAgents.Num(); i++)
    {
        FCrowd_HerdAgent& Agent = HerdAgents[i];

        // Boids flocking: separation + alignment + cohesion
        FVector Separation = ComputeSeparationForce(i);
        FVector Alignment = ComputeAlignmentForce(i);
        FVector Cohesion = ComputeCohesionForce(i);

        // Stampede direction force (dominant during panic)
        FVector PanicForce = StampedeDirection * CurrentSpeed;

        // Combine forces
        FVector DesiredVelocity = PanicForce
            + Separation * Agent.SeparationWeight
            + Alignment * AlignmentStrength
            + Cohesion * CohesionStrength;

        // Smooth velocity transition
        Agent.Velocity = FMath::VInterpTo(Agent.Velocity, DesiredVelocity, DeltaTime, 3.0f);

        // Clamp to max speed
        if (Agent.Velocity.SizeSquared() > CurrentSpeed * CurrentSpeed)
        {
            Agent.Velocity = Agent.Velocity.GetSafeNormal() * CurrentSpeed;
        }

        // Update position
        Agent.Position += Agent.Velocity * DeltaTime;
    }
}

void ACrowdStampedeController::TriggerTribalReaction(FVector InStampedeDirection)
{
    // Assign behavioral roles based on agent index
    for (int32 i = 0; i < TribalAgents.Num(); i++)
    {
        FCrowd_TribalAgent& Agent = TribalAgents[i];

        if (Agent.bIsProtector)
        {
            // Protectors move toward camp center
            Agent.Reaction = ECrowd_TribalReaction::ProtectYoung;
            Agent.FleeTarget = GetActorLocation(); // Camp center
        }
        else if (i < 4)
        {
            // First group flees away from stampede
            Agent.Reaction = ECrowd_TribalReaction::Flee;
            Agent.FleeTarget = Agent.Position + (-InStampedeDirection * 2000.0f);
        }
        else
        {
            // Middle group forms defensive line
            Agent.Reaction = ECrowd_TribalReaction::Defend;
            Agent.FleeTarget = Agent.Position + (InStampedeDirection * 500.0f);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("CrowdStampedeController: Tribal reaction triggered for %d agents"),
        TribalAgents.Num());
}

void ACrowdStampedeController::CalmHerd()
{
    CurrentStampedeState = ECrowd_StampedeState::Calm;
    PanicTimer = 0.0f;

    for (FCrowd_HerdAgent& Agent : HerdAgents)
    {
        Agent.State = ECrowd_StampedeState::Calm;
        Agent.Velocity = FVector::ZeroVector;
    }

    for (FCrowd_TribalAgent& Agent : TribalAgents)
    {
        Agent.Reaction = ECrowd_TribalReaction::Idle;
    }

    UE_LOG(LogTemp, Log, TEXT("CrowdStampedeController: Herd calmed — returning to idle state"));
}

FVector ACrowdStampedeController::ComputeSeparationForce(int32 AgentIndex) const
{
    FVector Force = FVector::ZeroVector;
    const FCrowd_HerdAgent& Agent = HerdAgents[AgentIndex];

    for (int32 j = 0; j < HerdAgents.Num(); j++)
    {
        if (j == AgentIndex) continue;

        FVector Diff = Agent.Position - HerdAgents[j].Position;
        float Dist = Diff.Size();

        if (Dist < SeparationDistance && Dist > 0.0f)
        {
            // Stronger repulsion when closer
            Force += Diff.GetSafeNormal() * (SeparationDistance - Dist) / SeparationDistance;
        }
    }

    return Force;
}

FVector ACrowdStampedeController::ComputeAlignmentForce(int32 AgentIndex) const
{
    FVector AvgVelocity = FVector::ZeroVector;
    int32 NeighborCount = 0;
    const FCrowd_HerdAgent& Agent = HerdAgents[AgentIndex];

    for (int32 j = 0; j < HerdAgents.Num(); j++)
    {
        if (j == AgentIndex) continue;

        float Dist = FVector::Dist(Agent.Position, HerdAgents[j].Position);
        if (Dist < SeparationDistance * 3.0f)
        {
            AvgVelocity += HerdAgents[j].Velocity;
            NeighborCount++;
        }
    }

    if (NeighborCount > 0)
    {
        AvgVelocity /= NeighborCount;
        return (AvgVelocity - Agent.Velocity).GetSafeNormal();
    }

    return FVector::ZeroVector;
}

FVector ACrowdStampedeController::ComputeCohesionForce(int32 AgentIndex) const
{
    FVector CenterOfMass = FVector::ZeroVector;
    int32 NeighborCount = 0;
    const FCrowd_HerdAgent& Agent = HerdAgents[AgentIndex];

    for (int32 j = 0; j < HerdAgents.Num(); j++)
    {
        if (j == AgentIndex) continue;

        float Dist = FVector::Dist(Agent.Position, HerdAgents[j].Position);
        if (Dist < SeparationDistance * 4.0f)
        {
            CenterOfMass += HerdAgents[j].Position;
            NeighborCount++;
        }
    }

    if (NeighborCount > 0)
    {
        CenterOfMass /= NeighborCount;
        return (CenterOfMass - Agent.Position).GetSafeNormal();
    }

    return FVector::ZeroVector;
}
