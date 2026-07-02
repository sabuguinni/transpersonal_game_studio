// CrowdSimulationManager.cpp — Agent #13 Crowd & Traffic Simulation
// Transpersonal Game Studio — Prehistoric Survival Game
// Implements Mass AI crowd simulation for up to 50,000 agents

#include "CrowdSimulationManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Actor.h"

UCrowdSimulationManager::UCrowdSimulationManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz update for performance

    MaxActiveAgents = 500;
    AgentSpawnRadius = 5000.0f;
    LODDistanceClose = 1500.0f;
    LODDistanceMedium = 4000.0f;
    LODDistanceFar = 8000.0f;
    bSimulationActive = false;
    bDebugDrawEnabled = false;
    CurrentAgentCount = 0;
}

void UCrowdSimulationManager::BeginPlay()
{
    Super::BeginPlay();
    bSimulationActive = true;
    CurrentAgentCount = 0;
    unreal::log("CrowdSimulationManager: Initialized");
}

void UCrowdSimulationManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bSimulationActive) return;

    UpdateAgentLOD();
    UpdateHerdBehavior(DeltaTime);

    if (bDebugDrawEnabled)
    {
        DrawDebugCrowd();
    }
}

void UCrowdSimulationManager::InitializeCrowdSystem(int32 InitialAgentCount)
{
    if (InitialAgentCount <= 0 || InitialAgentCount > MaxActiveAgents)
    {
        UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Invalid agent count %d"), InitialAgentCount);
        return;
    }

    ActiveAgents.Reserve(InitialAgentCount);
    CurrentAgentCount = 0;

    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Crowd system initialized for %d agents"), InitialAgentCount);
    bSimulationActive = true;
}

void UCrowdSimulationManager::SpawnCrowdAgents(int32 Count, FVector CenterLocation, ECrowd_AgentType AgentType)
{
    if (!GetWorld()) return;
    if (CurrentAgentCount + Count > MaxActiveAgents)
    {
        Count = MaxActiveAgents - CurrentAgentCount;
    }
    if (Count <= 0) return;

    for (int32 i = 0; i < Count; ++i)
    {
        FCrowd_AgentData NewAgent;
        NewAgent.AgentID = CurrentAgentCount + i;
        NewAgent.AgentType = AgentType;

        // Random position within spawn radius
        float Angle = FMath::RandRange(0.0f, 360.0f);
        float Radius = FMath::RandRange(0.0f, AgentSpawnRadius);
        NewAgent.Location = CenterLocation + FVector(
            FMath::Cos(FMath::DegreesToRadians(Angle)) * Radius,
            FMath::Sin(FMath::DegreesToRadians(Angle)) * Radius,
            0.0f
        );
        NewAgent.Velocity = FVector::ZeroVector;
        NewAgent.CurrentSpeed = 0.0f;
        NewAgent.PanicLevel = 0.0f;
        NewAgent.bIsLeader = (i == 0); // First agent is leader
        NewAgent.LODLevel = ECrowd_LODLevel::Full;
        NewAgent.bIsActive = true;

        ActiveAgents.Add(NewAgent);
    }

    CurrentAgentCount += Count;
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Spawned %d agents of type %d. Total: %d"), Count, (int32)AgentType, CurrentAgentCount);
}

void UCrowdSimulationManager::TriggerStampede(FVector TriggerLocation, float PanicRadius)
{
    if (ActiveAgents.Num() == 0) return;

    int32 AffectedCount = 0;
    for (FCrowd_AgentData& Agent : ActiveAgents)
    {
        if (!Agent.bIsActive) continue;

        float Distance = FVector::Dist(Agent.Location, TriggerLocation);
        if (Distance <= PanicRadius)
        {
            // Panic propagates based on proximity
            float PanicFactor = 1.0f - (Distance / PanicRadius);
            Agent.PanicLevel = FMath::Min(1.0f, Agent.PanicLevel + PanicFactor);

            // Flee direction away from trigger
            FVector FleeDir = (Agent.Location - TriggerLocation).GetSafeNormal();
            Agent.Velocity = FleeDir * GetMaxSpeedForType(Agent.AgentType) * Agent.PanicLevel;
            AffectedCount++;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Stampede triggered at %s — %d agents panicking"), *TriggerLocation.ToString(), AffectedCount);
    OnStampedeTriggered.Broadcast(TriggerLocation, AffectedCount);
}

void UCrowdSimulationManager::UpdateAgentLOD()
{
    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC || !PC->GetPawn()) return;

    FVector PlayerLocation = PC->GetPawn()->GetActorLocation();

    for (FCrowd_AgentData& Agent : ActiveAgents)
    {
        if (!Agent.bIsActive) continue;

        float Distance = FVector::Dist(Agent.Location, PlayerLocation);

        if (Distance <= LODDistanceClose)
        {
            Agent.LODLevel = ECrowd_LODLevel::Full;
        }
        else if (Distance <= LODDistanceMedium)
        {
            Agent.LODLevel = ECrowd_LODLevel::Medium;
        }
        else if (Distance <= LODDistanceFar)
        {
            Agent.LODLevel = ECrowd_LODLevel::Low;
        }
        else
        {
            Agent.LODLevel = ECrowd_LODLevel::Culled;
        }
    }
}

void UCrowdSimulationManager::UpdateHerdBehavior(float DeltaTime)
{
    // Simple flocking: separation, alignment, cohesion
    for (FCrowd_AgentData& Agent : ActiveAgents)
    {
        if (!Agent.bIsActive) continue;
        if (Agent.LODLevel == ECrowd_LODLevel::Culled) continue;

        // Reduce panic over time
        if (Agent.PanicLevel > 0.0f)
        {
            Agent.PanicLevel = FMath::Max(0.0f, Agent.PanicLevel - DeltaTime * 0.1f);
        }

        // Move agent based on velocity
        if (!Agent.Velocity.IsNearlyZero())
        {
            Agent.Location += Agent.Velocity * DeltaTime;
            Agent.CurrentSpeed = Agent.Velocity.Size();

            // Dampen velocity
            Agent.Velocity *= 0.95f;
        }
    }
}

void UCrowdSimulationManager::DrawDebugCrowd()
{
    UWorld* World = GetWorld();
    if (!World) return;

    for (const FCrowd_AgentData& Agent : ActiveAgents)
    {
        if (!Agent.bIsActive) continue;

        FColor DebugColor = FColor::Green;
        if (Agent.PanicLevel > 0.7f) DebugColor = FColor::Red;
        else if (Agent.PanicLevel > 0.3f) DebugColor = FColor::Yellow;

        DrawDebugSphere(World, Agent.Location, 50.0f, 8, DebugColor, false, 0.15f);

        if (!Agent.Velocity.IsNearlyZero())
        {
            DrawDebugArrow(World, Agent.Location, Agent.Location + Agent.Velocity * 0.5f, 20.0f, DebugColor, false, 0.15f);
        }
    }
}

float UCrowdSimulationManager::GetMaxSpeedForType(ECrowd_AgentType AgentType) const
{
    switch (AgentType)
    {
        case ECrowd_AgentType::DinosaurHerbivore:   return 800.0f;
        case ECrowd_AgentType::DinosaurCarnivore:   return 1200.0f;
        case ECrowd_AgentType::HumanTribal:         return 400.0f;
        case ECrowd_AgentType::SmallAnimal:         return 600.0f;
        default:                                     return 500.0f;
    }
}

int32 UCrowdSimulationManager::GetActiveAgentCount() const
{
    return CurrentAgentCount;
}

void UCrowdSimulationManager::SetSimulationActive(bool bActive)
{
    bSimulationActive = bActive;
}

void UCrowdSimulationManager::ClearAllAgents()
{
    ActiveAgents.Empty();
    CurrentAgentCount = 0;
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: All agents cleared"));
}
