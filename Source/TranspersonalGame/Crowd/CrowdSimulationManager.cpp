// CrowdSimulationManager.cpp
// Agent #13 — Crowd & Traffic Simulation
// Prehistoric survival crowd AI: human tribes + dinosaur herds

#include "CrowdSimulationManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

UCrowdSimulationManager::UCrowdSimulationManager()
{
    MaxAgents = 500;
    LODDistanceNear = 2000.0f;
    LODDistanceMid = 8000.0f;
    LODDistanceFar = 20000.0f;
    TickInterval = 0.1f;
    bSimulationActive = false;
}

void UCrowdSimulationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    bSimulationActive = true;
    AgentPool.Reserve(MaxAgents);
    unreal::log("CrowdSimulationManager initialized");
}

void UCrowdSimulationManager::Deinitialize()
{
    bSimulationActive = false;
    AgentPool.Empty();
    Super::Deinitialize();
}

void UCrowdSimulationManager::RegisterAgent(FCrowd_AgentData& AgentData)
{
    if (AgentPool.Num() >= MaxAgents)
    {
        return;
    }
    AgentData.AgentID = NextAgentID++;
    AgentData.LODLevel = ECrowd_LODLevel::Near;
    AgentPool.Add(AgentData);
}

void UCrowdSimulationManager::UnregisterAgent(int32 AgentID)
{
    AgentPool.RemoveAll([AgentID](const FCrowd_AgentData& A) {
        return A.AgentID == AgentID;
    });
}

void UCrowdSimulationManager::UpdateLOD(const FVector& PlayerLocation)
{
    for (FCrowd_AgentData& Agent : AgentPool)
    {
        if (!Agent.AgentActor) continue;

        float Dist = FVector::Dist(PlayerLocation, Agent.AgentActor->GetActorLocation());

        if (Dist < LODDistanceNear)
        {
            Agent.LODLevel = ECrowd_LODLevel::Near;
            Agent.TickRate = 0.05f;  // Full update rate
        }
        else if (Dist < LODDistanceMid)
        {
            Agent.LODLevel = ECrowd_LODLevel::Mid;
            Agent.TickRate = 0.2f;   // Reduced update rate
        }
        else if (Dist < LODDistanceFar)
        {
            Agent.LODLevel = ECrowd_LODLevel::Far;
            Agent.TickRate = 1.0f;   // Minimal update rate
        }
        else
        {
            Agent.LODLevel = ECrowd_LODLevel::Culled;
            Agent.TickRate = 0.0f;   // No update — fully culled
        }
    }
}

void UCrowdSimulationManager::UpdateAgentBehavior(FCrowd_AgentData& Agent, float DeltaTime)
{
    if (!Agent.AgentActor || Agent.LODLevel == ECrowd_LODLevel::Culled)
    {
        return;
    }

    switch (Agent.BehaviorState)
    {
        case ECrowd_BehaviorState::Idle:
            UpdateIdleBehavior(Agent, DeltaTime);
            break;
        case ECrowd_BehaviorState::Wandering:
            UpdateWanderBehavior(Agent, DeltaTime);
            break;
        case ECrowd_BehaviorState::Fleeing:
            UpdateFleeBehavior(Agent, DeltaTime);
            break;
        case ECrowd_BehaviorState::Herding:
            UpdateHerdBehavior(Agent, DeltaTime);
            break;
        case ECrowd_BehaviorState::Foraging:
            UpdateForageBehavior(Agent, DeltaTime);
            break;
        default:
            break;
    }
}

void UCrowdSimulationManager::UpdateIdleBehavior(FCrowd_AgentData& Agent, float DeltaTime)
{
    Agent.IdleTimer += DeltaTime;
    if (Agent.IdleTimer > Agent.IdleDuration)
    {
        Agent.IdleTimer = 0.0f;
        Agent.BehaviorState = ECrowd_BehaviorState::Wandering;
        Agent.WanderTarget = GetRandomWanderTarget(Agent);
    }
}

void UCrowdSimulationManager::UpdateWanderBehavior(FCrowd_AgentData& Agent, float DeltaTime)
{
    if (!Agent.AgentActor) return;

    FVector CurrentLoc = Agent.AgentActor->GetActorLocation();
    FVector ToTarget = Agent.WanderTarget - CurrentLoc;
    float DistToTarget = ToTarget.Size();

    if (DistToTarget < 100.0f)
    {
        // Reached wander target — go idle
        Agent.BehaviorState = ECrowd_BehaviorState::Idle;
        Agent.IdleDuration = FMath::RandRange(3.0f, 10.0f);
        return;
    }

    // Move toward target
    FVector Direction = ToTarget.GetSafeNormal();
    FVector NewLoc = CurrentLoc + Direction * Agent.MoveSpeed * DeltaTime;
    Agent.AgentActor->SetActorLocation(NewLoc, true);

    // Face movement direction
    FRotator NewRot = Direction.Rotation();
    Agent.AgentActor->SetActorRotation(NewRot);
}

void UCrowdSimulationManager::UpdateFleeBehavior(FCrowd_AgentData& Agent, float DeltaTime)
{
    if (!Agent.AgentActor || !Agent.ThreatActor) return;

    FVector CurrentLoc = Agent.AgentActor->GetActorLocation();
    FVector ThreatLoc = Agent.ThreatActor->GetActorLocation();
    FVector FleeDir = (CurrentLoc - ThreatLoc).GetSafeNormal();

    // Move away from threat at panic speed
    float PanicSpeed = Agent.MoveSpeed * 2.5f;
    FVector NewLoc = CurrentLoc + FleeDir * PanicSpeed * DeltaTime;
    Agent.AgentActor->SetActorLocation(NewLoc, true);

    // Reduce fear over distance
    float DistToThreat = FVector::Dist(CurrentLoc, ThreatLoc);
    if (DistToThreat > Agent.FleeRadius)
    {
        Agent.ThreatActor = nullptr;
        Agent.BehaviorState = ECrowd_BehaviorState::Idle;
        Agent.IdleDuration = 5.0f;  // Recover from panic
    }
}

void UCrowdSimulationManager::UpdateHerdBehavior(FCrowd_AgentData& Agent, float DeltaTime)
{
    if (!Agent.AgentActor) return;

    // Boids-style flocking: separation, alignment, cohesion
    FVector Separation = FVector::ZeroVector;
    FVector Alignment = FVector::ZeroVector;
    FVector Cohesion = FVector::ZeroVector;
    int32 Neighbors = 0;

    FVector MyLoc = Agent.AgentActor->GetActorLocation();

    for (const FCrowd_AgentData& Other : AgentPool)
    {
        if (Other.AgentID == Agent.AgentID || !Other.AgentActor) continue;
        if (Other.HerdID != Agent.HerdID) continue;

        FVector OtherLoc = Other.AgentActor->GetActorLocation();
        float Dist = FVector::Dist(MyLoc, OtherLoc);

        if (Dist < Agent.HerdRadius)
        {
            // Separation — avoid crowding
            if (Dist < Agent.SeparationRadius && Dist > 0.0f)
            {
                Separation += (MyLoc - OtherLoc) / Dist;
            }
            // Alignment — match velocity direction
            Alignment += Other.Velocity;
            // Cohesion — move toward center
            Cohesion += OtherLoc;
            Neighbors++;
        }
    }

    if (Neighbors > 0)
    {
        Alignment /= Neighbors;
        Cohesion = (Cohesion / Neighbors) - MyLoc;

        FVector SteeringForce = Separation * 1.5f + Alignment * 1.0f + Cohesion * 0.8f;
        SteeringForce = SteeringForce.GetSafeNormal() * Agent.MoveSpeed;

        Agent.Velocity = FMath::VInterpTo(Agent.Velocity, SteeringForce, DeltaTime, 3.0f);
        FVector NewLoc = MyLoc + Agent.Velocity * DeltaTime;
        Agent.AgentActor->SetActorLocation(NewLoc, true);

        if (!Agent.Velocity.IsNearlyZero())
        {
            Agent.AgentActor->SetActorRotation(Agent.Velocity.Rotation());
        }
    }
}

void UCrowdSimulationManager::UpdateForageBehavior(FCrowd_AgentData& Agent, float DeltaTime)
{
    // Foraging — move slowly, stop frequently, simulate resource gathering
    Agent.ForageTimer += DeltaTime;

    if (Agent.ForageTimer > Agent.ForageInterval)
    {
        Agent.ForageTimer = 0.0f;
        // Pick new forage spot nearby
        Agent.WanderTarget = GetRandomWanderTarget(Agent, 300.0f);
        Agent.BehaviorState = ECrowd_BehaviorState::Wandering;
    }
}

void UCrowdSimulationManager::TriggerFleeResponse(AActor* ThreatActor, float ThreatRadius)
{
    if (!ThreatActor) return;

    FVector ThreatLoc = ThreatActor->GetActorLocation();

    for (FCrowd_AgentData& Agent : AgentPool)
    {
        if (!Agent.AgentActor) continue;

        float Dist = FVector::Dist(ThreatLoc, Agent.AgentActor->GetActorLocation());
        if (Dist < ThreatRadius)
        {
            Agent.BehaviorState = ECrowd_BehaviorState::Fleeing;
            Agent.ThreatActor = ThreatActor;
            Agent.FleeRadius = ThreatRadius * 1.5f;
        }
    }
}

FVector UCrowdSimulationManager::GetRandomWanderTarget(const FCrowd_AgentData& Agent, float Radius)
{
    if (!Agent.AgentActor) return FVector::ZeroVector;

    FVector CurrentLoc = Agent.AgentActor->GetActorLocation();
    float Angle = FMath::RandRange(0.0f, 360.0f);
    float Distance = FMath::RandRange(Radius * 0.3f, Radius);

    return CurrentLoc + FVector(
        FMath::Cos(FMath::DegreesToRadians(Angle)) * Distance,
        FMath::Sin(FMath::DegreesToRadians(Angle)) * Distance,
        0.0f
    );
}

int32 UCrowdSimulationManager::GetActiveAgentCount() const
{
    return AgentPool.Num();
}

TArray<FCrowd_AgentData> UCrowdSimulationManager::GetAgentsInRadius(const FVector& Center, float Radius) const
{
    TArray<FCrowd_AgentData> Result;
    for (const FCrowd_AgentData& Agent : AgentPool)
    {
        if (!Agent.AgentActor) continue;
        if (FVector::Dist(Center, Agent.AgentActor->GetActorLocation()) <= Radius)
        {
            Result.Add(Agent);
        }
    }
    return Result;
}
