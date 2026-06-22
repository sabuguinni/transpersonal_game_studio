// CrowdSimulationManager.cpp
// Agent #13 — Crowd & Traffic Simulation
// Prehistoric herd/flock crowd simulation using UE5 Mass-style entity management

#include "CrowdSimulationManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

UCrowdSimulationManager::UCrowdSimulationManager()
{
    MaxHerdAgents = 50;
    MaxFlockAgents = 30;
    HerdCohesionRadius = 800.0f;
    HerdSeparationRadius = 150.0f;
    FlockAltitudeMin = 300.0f;
    FlockAltitudeMax = 600.0f;
    AgentMoveSpeed = 120.0f;
    bSimulationActive = false;
    SimulationTickRate = 0.1f;
    AccumulatedTime = 0.0f;
}

void UCrowdSimulationManager::InitializeSimulation(UWorld* InWorld)
{
    if (!InWorld)
    {
        UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: InitializeSimulation called with null world"));
        return;
    }

    WorldRef = InWorld;
    bSimulationActive = true;
    HerdAgents.Empty();
    FlockAgents.Empty();

    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Simulation initialized. MaxHerd=%d MaxFlock=%d"),
        MaxHerdAgents, MaxFlockAgents);
}

void UCrowdSimulationManager::RegisterHerdAgent(AActor* Agent, ECrowd_HerdType HerdType)
{
    if (!Agent) return;

    FCrowd_HerdAgentData Data;
    Data.AgentActor = Agent;
    Data.HerdType = HerdType;
    Data.CurrentVelocity = FVector::ZeroVector;
    Data.TargetLocation = Agent->GetActorLocation();
    Data.bIsFleeing = false;
    Data.FleeTimer = 0.0f;
    Data.AgentState = ECrowd_AgentState::Grazing;

    HerdAgents.Add(Data);

    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Registered herd agent %s type=%d"),
        *Agent->GetName(), (int32)HerdType);
}

void UCrowdSimulationManager::RegisterFlockAgent(AActor* Agent)
{
    if (!Agent) return;

    FCrowd_FlockAgentData Data;
    Data.AgentActor = Agent;
    Data.CurrentVelocity = FVector(50.0f, 0.0f, 0.0f);
    Data.TargetLocation = Agent->GetActorLocation();
    Data.CircleCenter = Agent->GetActorLocation();
    Data.CircleRadius = 400.0f;
    Data.CircleAngle = FMath::RandRange(0.0f, 360.0f);
    Data.AgentState = ECrowd_AgentState::Patrolling;

    FlockAgents.Add(Data);
}

void UCrowdSimulationManager::TickSimulation(float DeltaTime)
{
    if (!bSimulationActive || !WorldRef) return;

    AccumulatedTime += DeltaTime;
    if (AccumulatedTime < SimulationTickRate) return;
    AccumulatedTime = 0.0f;

    TickHerdBehavior(SimulationTickRate);
    TickFlockBehavior(SimulationTickRate);
}

void UCrowdSimulationManager::TickHerdBehavior(float DeltaTime)
{
    if (HerdAgents.Num() == 0) return;

    // Compute herd centroid
    FVector Centroid = FVector::ZeroVector;
    int32 ValidCount = 0;
    for (const FCrowd_HerdAgentData& Agent : HerdAgents)
    {
        if (Agent.AgentActor)
        {
            Centroid += Agent.AgentActor->GetActorLocation();
            ValidCount++;
        }
    }
    if (ValidCount > 0) Centroid /= ValidCount;

    for (FCrowd_HerdAgentData& Agent : HerdAgents)
    {
        if (!Agent.AgentActor) continue;

        FVector AgentPos = Agent.AgentActor->GetActorLocation();

        if (Agent.bIsFleeing)
        {
            Agent.FleeTimer -= DeltaTime;
            if (Agent.FleeTimer <= 0.0f)
            {
                Agent.bIsFleeing = false;
                Agent.AgentState = ECrowd_AgentState::Grazing;
            }
            // Move away from threat
            FVector FleeDir = (AgentPos - Agent.ThreatLocation).GetSafeNormal();
            FVector NewPos = AgentPos + FleeDir * AgentMoveSpeed * 2.0f * DeltaTime;
            NewPos.Z = AgentPos.Z;
            Agent.AgentActor->SetActorLocation(NewPos, false);
            continue;
        }

        // Cohesion: move toward centroid
        FVector CohesionForce = (Centroid - AgentPos);
        float DistToCentroid = CohesionForce.Size();
        if (DistToCentroid > HerdCohesionRadius)
        {
            CohesionForce.Normalize();
            CohesionForce *= AgentMoveSpeed * DeltaTime;
        }
        else
        {
            CohesionForce = FVector::ZeroVector;
        }

        // Separation: avoid neighbors
        FVector SeparationForce = FVector::ZeroVector;
        for (const FCrowd_HerdAgentData& Other : HerdAgents)
        {
            if (!Other.AgentActor || Other.AgentActor == Agent.AgentActor) continue;
            FVector Diff = AgentPos - Other.AgentActor->GetActorLocation();
            float Dist = Diff.Size();
            if (Dist < HerdSeparationRadius && Dist > 0.1f)
            {
                SeparationForce += Diff.GetSafeNormal() * (HerdSeparationRadius - Dist);
            }
        }

        // Wander: small random drift
        FVector WanderForce = FVector(
            FMath::RandRange(-30.0f, 30.0f),
            FMath::RandRange(-30.0f, 30.0f),
            0.0f) * DeltaTime;

        FVector TotalForce = CohesionForce + SeparationForce * 0.5f + WanderForce;
        TotalForce.Z = 0.0f;

        FVector NewPos = AgentPos + TotalForce;
        if (!TotalForce.IsNearlyZero())
        {
            Agent.AgentActor->SetActorLocation(NewPos, false);
            FRotator NewRot = TotalForce.Rotation();
            NewRot.Pitch = 0.0f;
            NewRot.Roll = 0.0f;
            Agent.AgentActor->SetActorRotation(NewRot);
        }
    }
}

void UCrowdSimulationManager::TickFlockBehavior(float DeltaTime)
{
    for (FCrowd_FlockAgentData& Agent : FlockAgents)
    {
        if (!Agent.AgentActor) continue;

        // Circular patrol pattern at altitude
        Agent.CircleAngle += 45.0f * DeltaTime; // degrees/sec
        if (Agent.CircleAngle > 360.0f) Agent.CircleAngle -= 360.0f;

        float RadAngle = FMath::DegreesToRadians(Agent.CircleAngle);
        FVector NewPos;
        NewPos.X = Agent.CircleCenter.X + FMath::Cos(RadAngle) * Agent.CircleRadius;
        NewPos.Y = Agent.CircleCenter.Y + FMath::Sin(RadAngle) * Agent.CircleRadius;
        NewPos.Z = Agent.CircleCenter.Z + FMath::Sin(RadAngle * 2.0f) * 30.0f; // gentle altitude variation

        // Clamp altitude
        NewPos.Z = FMath::Clamp(NewPos.Z, FlockAltitudeMin, FlockAltitudeMax);

        Agent.AgentActor->SetActorLocation(NewPos, false);

        // Face movement direction
        FVector MoveDir = NewPos - Agent.AgentActor->GetActorLocation();
        if (!MoveDir.IsNearlyZero())
        {
            Agent.AgentActor->SetActorRotation(MoveDir.Rotation());
        }
    }
}

void UCrowdSimulationManager::TriggerFleeResponse(FVector ThreatLocation, float ThreatRadius)
{
    int32 AffectedCount = 0;
    for (FCrowd_HerdAgentData& Agent : HerdAgents)
    {
        if (!Agent.AgentActor) continue;
        float Dist = FVector::Dist(Agent.AgentActor->GetActorLocation(), ThreatLocation);
        if (Dist <= ThreatRadius)
        {
            Agent.bIsFleeing = true;
            Agent.FleeTimer = FMath::RandRange(5.0f, 12.0f);
            Agent.ThreatLocation = ThreatLocation;
            Agent.AgentState = ECrowd_AgentState::Fleeing;
            AffectedCount++;
        }
    }
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: FleeResponse triggered. Affected=%d radius=%.0f"),
        AffectedCount, ThreatRadius);
}

void UCrowdSimulationManager::GetCrowdStats(int32& OutHerdCount, int32& OutFlockCount, bool& OutSimActive) const
{
    OutHerdCount = HerdAgents.Num();
    OutFlockCount = FlockAgents.Num();
    OutSimActive = bSimulationActive;
}

void UCrowdSimulationManager::StopSimulation()
{
    bSimulationActive = false;
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Simulation stopped."));
}
