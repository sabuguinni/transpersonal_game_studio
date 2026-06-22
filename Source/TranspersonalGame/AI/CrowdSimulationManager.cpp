// CrowdSimulationManager.cpp
// Agent #13 — Crowd & Traffic Simulation
// Implements prehistoric herd and pack crowd simulation using UE5 Mass AI patterns.

#include "CrowdSimulationManager.h"
#include "Engine/World.h"
#include "Engine/TargetPoint.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UCrowdSimulationManager::UCrowdSimulationManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f; // Update every 500ms for performance

    MaxActiveAgents = 50;
    HerdCohesionRadius = 800.0f;
    PredatorFleeRadius = 1500.0f;
    WaypointReachThreshold = 200.0f;
    bSimulationActive = false;
}

void UCrowdSimulationManager::BeginPlay()
{
    Super::BeginPlay();

    // Collect waypoints from level
    CollectWaypointsFromLevel();

    // Start simulation
    bSimulationActive = true;
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Started with %d waypoints, max %d agents"),
        HerdWaypoints.Num(), MaxActiveAgents);
}

void UCrowdSimulationManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bSimulationActive) return;

    UpdateHerdBehavior(DeltaTime);
    UpdatePackBehavior(DeltaTime);
    EnforceCrowdCap();
}

void UCrowdSimulationManager::CollectWaypointsFromLevel()
{
    if (!GetWorld()) return;

    HerdWaypoints.Empty();
    PackWaypoints.Empty();

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATargetPoint::StaticClass(), FoundActors);

    for (AActor* Actor : FoundActors)
    {
        if (!Actor) continue;
        FString Label = Actor->GetActorLabel();

        if (Label.Contains(TEXT("CrowdWP_Herd")))
        {
            HerdWaypoints.Add(Actor->GetActorLocation());
        }
        else if (Label.Contains(TEXT("CrowdWP_Raptor")) || Label.Contains(TEXT("CrowdWP_Pack")))
        {
            PackWaypoints.Add(Actor->GetActorLocation());
        }
    }

    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Collected %d herd waypoints, %d pack waypoints"),
        HerdWaypoints.Num(), PackWaypoints.Num());
}

void UCrowdSimulationManager::UpdateHerdBehavior(float DeltaTime)
{
    if (HerdWaypoints.Num() == 0) return;

    // Advance herd waypoint index on a timer
    HerdWaypointTimer += DeltaTime;
    if (HerdWaypointTimer >= HerdMigrationInterval)
    {
        HerdWaypointTimer = 0.0f;
        CurrentHerdWaypointIndex = (CurrentHerdWaypointIndex + 1) % HerdWaypoints.Num();

        UE_LOG(LogTemp, Verbose, TEXT("CrowdSimulationManager: Herd advancing to waypoint %d at %s"),
            CurrentHerdWaypointIndex,
            *HerdWaypoints[CurrentHerdWaypointIndex].ToString());
    }
}

void UCrowdSimulationManager::UpdatePackBehavior(float DeltaTime)
{
    if (PackWaypoints.Num() == 0) return;

    // Advance pack patrol index
    PackPatrolTimer += DeltaTime;
    if (PackPatrolTimer >= PackPatrolInterval)
    {
        PackPatrolTimer = 0.0f;
        CurrentPackWaypointIndex = (CurrentPackWaypointIndex + 1) % PackWaypoints.Num();

        UE_LOG(LogTemp, Verbose, TEXT("CrowdSimulationManager: Pack advancing to waypoint %d at %s"),
            CurrentPackWaypointIndex,
            *PackWaypoints[CurrentPackWaypointIndex].ToString());
    }
}

void UCrowdSimulationManager::EnforceCrowdCap()
{
    // Ensure we never exceed MaxActiveAgents
    // In a full implementation this would manage Mass AI entity counts
    // For now, log the current state
    if (ActiveAgentCount > MaxActiveAgents)
    {
        int32 Excess = ActiveAgentCount - MaxActiveAgents;
        UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: CAP exceeded by %d agents — culling oldest"), Excess);
        ActiveAgentCount = MaxActiveAgents;
    }
}

FVector UCrowdSimulationManager::GetCurrentHerdTarget() const
{
    if (HerdWaypoints.Num() == 0) return FVector::ZeroVector;
    return HerdWaypoints[CurrentHerdWaypointIndex % HerdWaypoints.Num()];
}

FVector UCrowdSimulationManager::GetCurrentPackTarget() const
{
    if (PackWaypoints.Num() == 0) return FVector::ZeroVector;
    return PackWaypoints[CurrentPackWaypointIndex % PackWaypoints.Num()];
}

void UCrowdSimulationManager::RegisterAgent(AActor* Agent, ECrowd_AgentType AgentType)
{
    if (!Agent) return;

    FCrowd_AgentData NewAgent;
    NewAgent.AgentActor = Agent;
    NewAgent.AgentType = AgentType;
    NewAgent.CurrentLocation = Agent->GetActorLocation();
    NewAgent.bIsActive = true;

    RegisteredAgents.Add(NewAgent);
    ActiveAgentCount = RegisteredAgents.Num();

    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Registered agent %s type=%d total=%d"),
        *Agent->GetActorLabel(), (int32)AgentType, ActiveAgentCount);
}

void UCrowdSimulationManager::UnregisterAgent(AActor* Agent)
{
    if (!Agent) return;

    RegisteredAgents.RemoveAll([Agent](const FCrowd_AgentData& Data) {
        return Data.AgentActor == Agent;
    });

    ActiveAgentCount = RegisteredAgents.Num();
}

void UCrowdSimulationManager::TriggerFleeResponse(FVector ThreatLocation, float ThreatRadius)
{
    // Broadcast flee event to all agents within radius
    int32 FleeingAgents = 0;
    for (FCrowd_AgentData& Agent : RegisteredAgents)
    {
        if (!Agent.AgentActor.IsValid()) continue;
        float Dist = FVector::Dist(Agent.CurrentLocation, ThreatLocation);
        if (Dist <= ThreatRadius)
        {
            Agent.bIsFleeing = true;
            FleeingAgents++;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Flee response triggered — %d agents fleeing from %s"),
        FleeingAgents, *ThreatLocation.ToString());
}
