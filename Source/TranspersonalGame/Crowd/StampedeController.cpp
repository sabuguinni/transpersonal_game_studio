// StampedeController.cpp
// Agent #13 — Crowd & Traffic Simulation
// Implements herd stampede logic for prehistoric dinosaur crowds

#include "StampedeController.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UCrowd_StampedeController::UCrowd_StampedeController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.05f; // 20 Hz tick for crowd

    MaxHerdSize = 30;
    StampedeSpeed = 1200.0f;
    StampedeDuration = 15.0f;
    PanicRadius = 2000.0f;
    SeparationWeight = 1.5f;
    CohesionWeight = 1.0f;
    AlignmentWeight = 1.2f;
    bStampedeActive = false;
    StampedeElapsed = 0.0f;
}

void UCrowd_StampedeController::BeginPlay()
{
    Super::BeginPlay();
    InitializeHerd();
}

void UCrowd_StampedeController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bStampedeActive)
    {
        UpdateStampede(DeltaTime);
    }
    else
    {
        UpdateFlocking(DeltaTime);
    }
}

void UCrowd_StampedeController::InitializeHerd()
{
    HerdAgents.Empty();
    HerdAgents.Reserve(MaxHerdSize);

    AActor* Owner = GetOwner();
    if (!Owner) return;

    FVector BaseLocation = Owner->GetActorLocation();

    for (int32 i = 0; i < MaxHerdSize; i++)
    {
        FCrowd_AgentData Agent;
        Agent.AgentID = i;
        Agent.CurrentState = ECrowd_BehaviorState::Foraging;
        Agent.HerdRole = (i == 0) ? ECrowd_HerdRole::Alpha : ECrowd_HerdRole::Follower;
        Agent.bIsLeader = (i == 0);

        // Scatter agents around the owner
        float Angle = (float)i / (float)MaxHerdSize * 2.0f * PI;
        float Radius = FMath::RandRange(200.0f, 800.0f);
        Agent.TargetLocation = BaseLocation + FVector(
            FMath::Cos(Angle) * Radius,
            FMath::Sin(Angle) * Radius,
            0.0f
        );

        Agent.WalkSpeed = FMath::RandRange(180.0f, 240.0f);
        Agent.FleeSpeed = FMath::RandRange(900.0f, 1400.0f);
        Agent.SeparationRadius = FMath::RandRange(120.0f, 180.0f);

        HerdAgents.Add(Agent);
    }

    UE_LOG(LogTemp, Log, TEXT("[Crowd] Herd initialized with %d agents"), HerdAgents.Num());
}

void UCrowd_StampedeController::TriggerStampede(FVector ThreatLocation)
{
    if (bStampedeActive) return;

    AActor* Owner = GetOwner();
    if (!Owner) return;

    bStampedeActive = true;
    StampedeElapsed = 0.0f;

    // Flee direction = away from threat
    ActiveStampede.TriggerLocation = ThreatLocation;
    ActiveStampede.FleeDirection = (Owner->GetActorLocation() - ThreatLocation).GetSafeNormal();
    ActiveStampede.StampedeDuration = StampedeDuration;
    ActiveStampede.StampedeSpeed = StampedeSpeed;
    ActiveStampede.bIsActive = true;

    // Set all agents to stampede state
    for (FCrowd_AgentData& Agent : HerdAgents)
    {
        Agent.CurrentState = ECrowd_BehaviorState::Stampeding;
        Agent.ThreatLevel = ECrowd_ThreatLevel::Panic;
        Agent.StampedeTimer = StampedeDuration;
    }

    UE_LOG(LogTemp, Warning, TEXT("[Crowd] STAMPEDE TRIGGERED! Herd of %d fleeing from %s"),
        HerdAgents.Num(), *ThreatLocation.ToString());

    OnStampedeTriggered.Broadcast(ThreatLocation, ActiveStampede.FleeDirection);
}

void UCrowd_StampedeController::UpdateStampede(float DeltaTime)
{
    StampedeElapsed += DeltaTime;

    if (StampedeElapsed >= StampedeDuration)
    {
        EndStampede();
        return;
    }

    // Gradually slow down as stampede ends
    float Progress = StampedeElapsed / StampedeDuration;
    float CurrentSpeed = FMath::Lerp(StampedeSpeed, 200.0f, Progress * Progress);

    for (FCrowd_AgentData& Agent : HerdAgents)
    {
        // Add slight variation to flee direction per agent
        FVector AgentFlee = ActiveStampede.FleeDirection;
        AgentFlee += FVector(
            FMath::RandRange(-0.2f, 0.2f),
            FMath::RandRange(-0.2f, 0.2f),
            0.0f
        );
        AgentFlee.Normalize();

        Agent.CurrentVelocity = AgentFlee * CurrentSpeed;
        Agent.StampedeTimer -= DeltaTime;
    }
}

void UCrowd_StampedeController::UpdateFlocking(float DeltaTime)
{
    int32 AgentCount = HerdAgents.Num();
    if (AgentCount == 0) return;

    // Compute flocking forces for each agent
    for (int32 i = 0; i < AgentCount; i++)
    {
        FCrowd_AgentData& Agent = HerdAgents[i];

        FVector Separation = FVector::ZeroVector;
        FVector Cohesion = FVector::ZeroVector;
        FVector Alignment = FVector::ZeroVector;
        int32 NeighborCount = 0;

        for (int32 j = 0; j < AgentCount; j++)
        {
            if (i == j) continue;

            FVector Delta = Agent.TargetLocation - HerdAgents[j].TargetLocation;
            float Dist = Delta.Size();

            // Separation
            if (Dist < Agent.SeparationRadius && Dist > 0.0f)
            {
                Separation += Delta.GetSafeNormal() / Dist;
            }

            // Cohesion + Alignment
            if (Dist < Agent.CohesionRadius)
            {
                Cohesion += HerdAgents[j].TargetLocation;
                Alignment += HerdAgents[j].CurrentVelocity;
                NeighborCount++;
            }
        }

        if (NeighborCount > 0)
        {
            Cohesion /= (float)NeighborCount;
            Cohesion = (Cohesion - Agent.TargetLocation).GetSafeNormal();
            Alignment /= (float)NeighborCount;
            Alignment.Normalize();
        }

        FVector SteeringForce = (Separation * SeparationWeight)
                              + (Cohesion * CohesionWeight)
                              + (Alignment * AlignmentWeight);

        Agent.CurrentVelocity = FMath::VInterpTo(
            Agent.CurrentVelocity,
            SteeringForce * Agent.WalkSpeed,
            DeltaTime,
            2.0f
        );

        Agent.TargetLocation += Agent.CurrentVelocity * DeltaTime;
    }
}

void UCrowd_StampedeController::EndStampede()
{
    bStampedeActive = false;
    ActiveStampede.bIsActive = false;
    StampedeElapsed = 0.0f;

    for (FCrowd_AgentData& Agent : HerdAgents)
    {
        Agent.CurrentState = ECrowd_BehaviorState::Resting;
        Agent.ThreatLevel = ECrowd_ThreatLevel::None;
        Agent.CurrentVelocity = FVector::ZeroVector;
        Agent.StampedeTimer = 0.0f;
    }

    UE_LOG(LogTemp, Log, TEXT("[Crowd] Stampede ended. Herd calming down."));
    OnStampedeEnded.Broadcast();
}

bool UCrowd_StampedeController::IsAgentInPanicRadius(FVector ThreatLocation) const
{
    AActor* Owner = GetOwner();
    if (!Owner) return false;

    return FVector::Dist(Owner->GetActorLocation(), ThreatLocation) <= PanicRadius;
}

int32 UCrowd_StampedeController::GetActiveAgentCount() const
{
    return HerdAgents.Num();
}

FCrowd_AgentData UCrowd_StampedeController::GetAgentData(int32 AgentIndex) const
{
    if (HerdAgents.IsValidIndex(AgentIndex))
    {
        return HerdAgents[AgentIndex];
    }
    return FCrowd_AgentData();
}
