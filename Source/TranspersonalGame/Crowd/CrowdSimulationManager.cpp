// CrowdSimulationManager.cpp
// Agent #13 — Crowd & Traffic Simulation
// Implements tribal crowd simulation for prehistoric survival game

#include "CrowdSimulationManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "NavMesh/NavMeshBoundsVolume.h"

UCrowdSimulationManager::UCrowdSimulationManager()
{
    MaxAgents = 500;
    AgentUpdateRadius = 5000.0f;
    FlockingCohesionRadius = 300.0f;
    FlockingSeparationRadius = 80.0f;
    FlockingAlignmentRadius = 200.0f;
    ThreatResponseRadius = 1500.0f;
    bSimulationActive = false;
    CurrentAgentCount = 0;
    SimulationTickRate = 0.1f;
    AccumulatedDeltaTime = 0.0f;
}

void UCrowdSimulationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    bSimulationActive = true;
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Initialized — MaxAgents=%d, UpdateRadius=%.0f"), MaxAgents, AgentUpdateRadius);
}

void UCrowdSimulationManager::Deinitialize()
{
    bSimulationActive = false;
    RegisteredAgents.Empty();
    ActiveHerds.Empty();
    Super::Deinitialize();
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Deinitialized"));
}

void UCrowdSimulationManager::RegisterCrowdAgent(FCrowd_AgentData& AgentData)
{
    if (CurrentAgentCount >= MaxAgents)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CrowdSim] Agent cap reached (%d). Cannot register more agents."), MaxAgents);
        return;
    }

    AgentData.AgentID = FGuid::NewGuid();
    AgentData.bIsActive = true;
    AgentData.CurrentState = ECrowd_AgentState::Idle;
    RegisteredAgents.Add(AgentData.AgentID, AgentData);
    CurrentAgentCount++;

    UE_LOG(LogTemp, Verbose, TEXT("[CrowdSim] Agent registered: %s (Type=%d)"), *AgentData.AgentID.ToString(), (int32)AgentData.AgentType);
}

void UCrowdSimulationManager::UnregisterCrowdAgent(const FGuid& AgentID)
{
    if (RegisteredAgents.Remove(AgentID) > 0)
    {
        CurrentAgentCount = FMath::Max(0, CurrentAgentCount - 1);
        UE_LOG(LogTemp, Verbose, TEXT("[CrowdSim] Agent unregistered: %s"), *AgentID.ToString());
    }
}

void UCrowdSimulationManager::RegisterHerd(FCrowd_HerdData& HerdData)
{
    HerdData.HerdID = FGuid::NewGuid();
    ActiveHerds.Add(HerdData.HerdID, HerdData);
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Herd registered: %s (%d members)"), *HerdData.HerdName.ToString(), HerdData.MemberIDs.Num());
}

void UCrowdSimulationManager::BroadcastThreatAlert(const FVector& ThreatLocation, float ThreatRadius, ECrowd_ThreatType ThreatType)
{
    if (!bSimulationActive) return;

    int32 AlertedCount = 0;
    for (auto& Pair : RegisteredAgents)
    {
        FCrowd_AgentData& Agent = Pair.Value;
        if (!Agent.bIsActive) continue;

        float DistSq = FVector::DistSquared(Agent.CurrentLocation, ThreatLocation);
        float RadiusSq = ThreatRadius * ThreatRadius;

        if (DistSq <= RadiusSq)
        {
            // Escalate state based on threat type
            switch (ThreatType)
            {
                case ECrowd_ThreatType::Predator:
                    Agent.CurrentState = ECrowd_AgentState::Fleeing;
                    Agent.ThreatLocation = ThreatLocation;
                    Agent.ThreatLevel = FMath::Clamp(1.0f - (FMath::Sqrt(DistSq) / ThreatRadius), 0.1f, 1.0f);
                    break;
                case ECrowd_ThreatType::Fire:
                    Agent.CurrentState = ECrowd_AgentState::Fleeing;
                    Agent.ThreatLocation = ThreatLocation;
                    Agent.ThreatLevel = 0.8f;
                    break;
                case ECrowd_ThreatType::Earthquake:
                    Agent.CurrentState = ECrowd_AgentState::Panicking;
                    Agent.ThreatLevel = 1.0f;
                    break;
                case ECrowd_ThreatType::PlayerPresence:
                    if (Agent.CurrentState == ECrowd_AgentState::Idle || Agent.CurrentState == ECrowd_AgentState::Wandering)
                    {
                        Agent.CurrentState = ECrowd_AgentState::Alert;
                        Agent.ThreatLevel = 0.4f;
                    }
                    break;
                default:
                    Agent.CurrentState = ECrowd_AgentState::Alert;
                    Agent.ThreatLevel = 0.3f;
                    break;
            }
            AlertedCount++;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] ThreatAlert broadcast: Type=%d, Radius=%.0f, Alerted=%d agents"),
        (int32)ThreatType, ThreatRadius, AlertedCount);
}

void UCrowdSimulationManager::UpdateFlockingBehavior(float DeltaTime)
{
    if (!bSimulationActive) return;

    for (auto& Pair : RegisteredAgents)
    {
        FCrowd_AgentData& Agent = Pair.Value;
        if (!Agent.bIsActive || Agent.CurrentState == ECrowd_AgentState::Fleeing || Agent.CurrentState == ECrowd_AgentState::Panicking)
            continue;

        FVector Cohesion = FVector::ZeroVector;
        FVector Separation = FVector::ZeroVector;
        FVector Alignment = FVector::ZeroVector;
        int32 CohesionCount = 0;
        int32 SeparationCount = 0;
        int32 AlignmentCount = 0;

        for (auto& OtherPair : RegisteredAgents)
        {
            if (OtherPair.Key == Pair.Key) continue;
            const FCrowd_AgentData& Other = OtherPair.Value;
            if (!Other.bIsActive) continue;

            float Dist = FVector::Dist(Agent.CurrentLocation, Other.CurrentLocation);

            if (Dist < FlockingCohesionRadius)
            {
                Cohesion += Other.CurrentLocation;
                CohesionCount++;
            }
            if (Dist < FlockingSeparationRadius && Dist > 0.0f)
            {
                Separation += (Agent.CurrentLocation - Other.CurrentLocation) / Dist;
                SeparationCount++;
            }
            if (Dist < FlockingAlignmentRadius)
            {
                Alignment += Other.CurrentVelocity;
                AlignmentCount++;
            }
        }

        FVector SteeringForce = FVector::ZeroVector;

        if (CohesionCount > 0)
        {
            Cohesion /= (float)CohesionCount;
            SteeringForce += (Cohesion - Agent.CurrentLocation).GetSafeNormal() * 0.3f;
        }
        if (SeparationCount > 0)
        {
            SteeringForce += Separation.GetSafeNormal() * 0.8f;
        }
        if (AlignmentCount > 0)
        {
            Alignment /= (float)AlignmentCount;
            SteeringForce += Alignment.GetSafeNormal() * 0.2f;
        }

        Agent.CurrentVelocity += SteeringForce * DeltaTime * 100.0f;
        Agent.CurrentVelocity = Agent.CurrentVelocity.GetClampedToMaxSize(Agent.MaxSpeed);
    }
}

void UCrowdSimulationManager::TickSimulation(float DeltaTime)
{
    if (!bSimulationActive) return;

    AccumulatedDeltaTime += DeltaTime;
    if (AccumulatedDeltaTime < SimulationTickRate) return;

    float SimDelta = AccumulatedDeltaTime;
    AccumulatedDeltaTime = 0.0f;

    UpdateFlockingBehavior(SimDelta);

    // Update agent positions
    for (auto& Pair : RegisteredAgents)
    {
        FCrowd_AgentData& Agent = Pair.Value;
        if (!Agent.bIsActive) continue;

        // State machine transitions
        switch (Agent.CurrentState)
        {
            case ECrowd_AgentState::Idle:
                // Random chance to start wandering
                if (FMath::RandRange(0, 100) < 5)
                {
                    Agent.CurrentState = ECrowd_AgentState::Wandering;
                    Agent.TargetLocation = Agent.HomeLocation + FVector(
                        FMath::RandRange(-500.0f, 500.0f),
                        FMath::RandRange(-500.0f, 500.0f),
                        0.0f
                    );
                }
                break;

            case ECrowd_AgentState::Wandering:
                {
                    FVector ToTarget = Agent.TargetLocation - Agent.CurrentLocation;
                    float DistToTarget = ToTarget.Size();
                    if (DistToTarget < 50.0f)
                    {
                        Agent.CurrentState = ECrowd_AgentState::Idle;
                        Agent.CurrentVelocity = FVector::ZeroVector;
                    }
                    else
                    {
                        Agent.CurrentVelocity = ToTarget.GetSafeNormal() * Agent.MaxSpeed;
                    }
                }
                break;

            case ECrowd_AgentState::Fleeing:
                {
                    FVector AwayFromThreat = (Agent.CurrentLocation - Agent.ThreatLocation).GetSafeNormal();
                    Agent.CurrentVelocity = AwayFromThreat * Agent.MaxSpeed * 2.0f;
                    // Decay threat level
                    Agent.ThreatLevel -= SimDelta * 0.1f;
                    if (Agent.ThreatLevel <= 0.0f)
                    {
                        Agent.CurrentState = ECrowd_AgentState::Alert;
                        Agent.ThreatLevel = 0.0f;
                    }
                }
                break;

            case ECrowd_AgentState::Alert:
                Agent.ThreatLevel -= SimDelta * 0.05f;
                if (Agent.ThreatLevel <= 0.0f)
                {
                    Agent.CurrentState = ECrowd_AgentState::Idle;
                    Agent.ThreatLevel = 0.0f;
                }
                break;

            case ECrowd_AgentState::Panicking:
                {
                    // Random chaotic movement
                    Agent.CurrentVelocity = FVector(
                        FMath::RandRange(-1.0f, 1.0f),
                        FMath::RandRange(-1.0f, 1.0f),
                        0.0f
                    ).GetSafeNormal() * Agent.MaxSpeed * 1.5f;
                }
                break;

            default:
                break;
        }

        // Apply velocity to position
        Agent.CurrentLocation += Agent.CurrentVelocity * SimDelta;
    }
}

int32 UCrowdSimulationManager::GetActiveAgentCount() const
{
    int32 Count = 0;
    for (const auto& Pair : RegisteredAgents)
    {
        if (Pair.Value.bIsActive) Count++;
    }
    return Count;
}

FCrowd_AgentData* UCrowdSimulationManager::GetAgentData(const FGuid& AgentID)
{
    return RegisteredAgents.Find(AgentID);
}

void UCrowdSimulationManager::SetSimulationActive(bool bActive)
{
    bSimulationActive = bActive;
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Simulation %s"), bActive ? TEXT("ACTIVATED") : TEXT("PAUSED"));
}
