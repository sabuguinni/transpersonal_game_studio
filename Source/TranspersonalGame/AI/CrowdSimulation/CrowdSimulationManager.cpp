// CrowdSimulationManager.cpp
// Agent #13 — Crowd & Traffic Simulation
// Implements prehistoric herd behaviour: grazing clusters, migration lines, watering groups

#include "CrowdSimulationManager.h"
#include "Engine/World.h"
#include "Engine/SkeletalMeshActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UCrowdSimulationManager::UCrowdSimulationManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f; // Update every 0.5s for performance

    MaxCrowdAgents = 50;
    HerdUpdateRadius = 5000.0f;
    bCrowdSystemActive = true;
}

void UCrowdSimulationManager::BeginPlay()
{
    Super::BeginPlay();

    if (bCrowdSystemActive)
    {
        // Register all Crowd_ labelled actors as managed agents
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);

        for (AActor* Actor : AllActors)
        {
            if (Actor && Actor->GetActorLabel().StartsWith(TEXT("Crowd_")))
            {
                FCrowd_AgentState NewAgent;
                NewAgent.AgentActor = Actor;
                NewAgent.HomeLocation = Actor->GetActorLocation();
                NewAgent.CurrentBehaviour = ECrowd_HerdBehaviour::Grazing;
                NewAgent.AlertLevel = 0.0f;
                NewAgent.bIsLeader = (ManagedAgents.Num() % 5 == 0); // Every 5th is a leader

                ManagedAgents.Add(NewAgent);
            }
        }

        UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Registered %d crowd agents"), ManagedAgents.Num());
    }
}

void UCrowdSimulationManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bCrowdSystemActive) return;

    // Update each agent's behaviour state
    for (FCrowd_AgentState& Agent : ManagedAgents)
    {
        if (!Agent.AgentActor) continue;

        UpdateAgentBehaviour(Agent, DeltaTime);
    }
}

void UCrowdSimulationManager::UpdateAgentBehaviour(FCrowd_AgentState& Agent, float DeltaTime)
{
    // Simple state machine: Grazing → Alert → Flee → Return
    switch (Agent.CurrentBehaviour)
    {
        case ECrowd_HerdBehaviour::Grazing:
        {
            // Slowly drift within home radius
            FVector CurrentLoc = Agent.AgentActor->GetActorLocation();
            FVector HomeOffset = CurrentLoc - Agent.HomeLocation;
            if (HomeOffset.Size() > 800.0f)
            {
                // Drift back toward home
                FVector ReturnDir = (Agent.HomeLocation - CurrentLoc).GetSafeNormal();
                Agent.AgentActor->SetActorLocation(CurrentLoc + ReturnDir * 30.0f * DeltaTime, false);
            }
            break;
        }

        case ECrowd_HerdBehaviour::Fleeing:
        {
            // Move away from threat location
            if (Agent.ThreatLocation != FVector::ZeroVector)
            {
                FVector FleeDir = (Agent.AgentActor->GetActorLocation() - Agent.ThreatLocation).GetSafeNormal();
                Agent.AgentActor->SetActorLocation(
                    Agent.AgentActor->GetActorLocation() + FleeDir * FleeSpeed * DeltaTime, false
                );
            }

            // Reduce alert over time
            Agent.AlertLevel = FMath::Max(0.0f, Agent.AlertLevel - DeltaTime * 0.3f);
            if (Agent.AlertLevel <= 0.0f)
            {
                Agent.CurrentBehaviour = ECrowd_HerdBehaviour::Returning;
            }
            break;
        }

        case ECrowd_HerdBehaviour::Returning:
        {
            FVector CurrentLoc = Agent.AgentActor->GetActorLocation();
            FVector ToHome = Agent.HomeLocation - CurrentLoc;
            if (ToHome.Size() < 150.0f)
            {
                Agent.CurrentBehaviour = ECrowd_HerdBehaviour::Grazing;
            }
            else
            {
                FVector ReturnDir = ToHome.GetSafeNormal();
                Agent.AgentActor->SetActorLocation(CurrentLoc + ReturnDir * WalkSpeed * DeltaTime, false);
            }
            break;
        }

        default:
            break;
    }
}

void UCrowdSimulationManager::TriggerHerdFlee(FVector ThreatLocation, float AlertRadius)
{
    int32 AlertedCount = 0;

    for (FCrowd_AgentState& Agent : ManagedAgents)
    {
        if (!Agent.AgentActor) continue;

        float DistToThreat = FVector::Dist(Agent.AgentActor->GetActorLocation(), ThreatLocation);
        if (DistToThreat <= AlertRadius)
        {
            Agent.CurrentBehaviour = ECrowd_HerdBehaviour::Fleeing;
            Agent.ThreatLocation = ThreatLocation;
            Agent.AlertLevel = 1.0f;
            AlertedCount++;

            // Propagate alert to nearby agents (herd instinct)
            PropagateAlert(Agent, ThreatLocation, AlertRadius * 0.6f);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Herd flee triggered: %d agents alerted (radius=%.0f)"), AlertedCount, AlertRadius);
}

void UCrowdSimulationManager::PropagateAlert(const FCrowd_AgentState& SourceAgent, FVector ThreatLocation, float PropagationRadius)
{
    for (FCrowd_AgentState& NearbyAgent : ManagedAgents)
    {
        if (!NearbyAgent.AgentActor) continue;
        if (NearbyAgent.CurrentBehaviour == ECrowd_HerdBehaviour::Fleeing) continue;

        float Dist = FVector::Dist(NearbyAgent.AgentActor->GetActorLocation(), SourceAgent.AgentActor->GetActorLocation());
        if (Dist <= PropagationRadius)
        {
            NearbyAgent.CurrentBehaviour = ECrowd_HerdBehaviour::Fleeing;
            NearbyAgent.ThreatLocation = ThreatLocation;
            NearbyAgent.AlertLevel = 0.7f; // Secondary alert — less intense
        }
    }
}

int32 UCrowdSimulationManager::GetActiveAgentCount() const
{
    int32 Count = 0;
    for (const FCrowd_AgentState& Agent : ManagedAgents)
    {
        if (Agent.AgentActor) Count++;
    }
    return Count;
}
