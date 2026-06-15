#include "Crowd_MassSimulationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ACrowd_MassSimulationManager::ACrowd_MassSimulationManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    MaxActiveAgents = 1000;
    UpdateFrequency = 0.1f;
    CullingDistance = 5000.0f;
    CurrentAgentCount = 0;
    LastUpdateTime = 0.0f;
    
    LODSettings = FCrowd_LODSettings();
}

void ACrowd_MassSimulationManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeSimulation();
}

void ACrowd_MassSimulationManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    LastUpdateTime += DeltaTime;
    if (LastUpdateTime >= UpdateFrequency)
    {
        UpdateSimulation(LastUpdateTime);
        LastUpdateTime = 0.0f;
    }
}

void ACrowd_MassSimulationManager::InitializeSimulation()
{
    ActiveAgents.Empty();
    CurrentAgentCount = 0;
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            TEXT("Mass Simulation Manager Initialized"));
    }
}

void ACrowd_MassSimulationManager::UpdateSimulation(float DeltaTime)
{
    if (ActiveAgents.Num() == 0)
    {
        return;
    }
    
    FVector PlayerLocation = GetPlayerLocation();
    
    // Update agents based on LOD
    for (FCrowd_AgentData& Agent : ActiveAgents)
    {
        if (ShouldUpdateAgent(Agent, DeltaTime))
        {
            float DistanceToPlayer = FVector::Dist(Agent.Location, PlayerLocation);
            UpdateAgentLOD(Agent, DistanceToPlayer);
            ProcessAgentMovement(Agent, DeltaTime);
        }
    }
    
    // Cull distant agents periodically
    if (FMath::RandRange(0, 100) < 10) // 10% chance each update
    {
        CullDistantAgents();
    }
    
    OptimizePerformance();
}

void ACrowd_MassSimulationManager::SpawnAgentGroup(FVector Location, ECrowd_AgentType AgentType, int32 Count)
{
    if (CurrentAgentCount + Count > MaxActiveAgents)
    {
        Count = FMath::Max(0, MaxActiveAgents - CurrentAgentCount);
    }
    
    for (int32 i = 0; i < Count; i++)
    {
        FCrowd_AgentData NewAgent;
        
        // Random position around spawn location
        FVector RandomOffset = FVector(
            FMath::RandRange(-200.0f, 200.0f),
            FMath::RandRange(-200.0f, 200.0f),
            0.0f
        );
        
        NewAgent.Location = Location + RandomOffset;
        NewAgent.AgentType = AgentType;
        NewAgent.Health = FMath::RandRange(80.0f, 100.0f);
        NewAgent.Stamina = FMath::RandRange(70.0f, 100.0f);
        NewAgent.GroupID = ActiveAgents.Num() / 10; // Groups of 10
        
        ActiveAgents.Add(NewAgent);
        CurrentAgentCount++;
    }
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue,
            FString::Printf(TEXT("Spawned %d agents. Total: %d"), Count, CurrentAgentCount));
    }
}

void ACrowd_MassSimulationManager::UpdateAgentLOD(FCrowd_AgentData& Agent, float DistanceToPlayer)
{
    if (DistanceToPlayer <= LODSettings.HighDetailDistance)
    {
        // High detail - full AI processing
        Agent.Stamina = FMath::Clamp(Agent.Stamina - 0.1f, 0.0f, 100.0f);
    }
    else if (DistanceToPlayer <= LODSettings.MediumDetailDistance)
    {
        // Medium detail - reduced processing
        Agent.Stamina = FMath::Clamp(Agent.Stamina - 0.05f, 0.0f, 100.0f);
    }
    else if (DistanceToPlayer <= LODSettings.LowDetailDistance)
    {
        // Low detail - minimal processing
        // Only update position occasionally
    }
    // Beyond low detail distance - agent is culled
}

void ACrowd_MassSimulationManager::ProcessAgentMovement(FCrowd_AgentData& Agent, float DeltaTime)
{
    // Get nearby agents for flocking
    TArray<FCrowd_AgentData> NearbyAgents = GetNearbyAgents(Agent.Location, 300.0f);
    
    // Apply flocking behavior
    ApplyFlockingBehavior(Agent, NearbyAgents);
    
    // Apply velocity to position
    Agent.Location += Agent.Velocity * DeltaTime;
    
    // Simple ground clamping
    Agent.Location.Z = FMath::Max(Agent.Location.Z, 100.0f);
    
    // Damping
    Agent.Velocity *= 0.98f;
}

void ACrowd_MassSimulationManager::CullDistantAgents()
{
    FVector PlayerLocation = GetPlayerLocation();
    
    ActiveAgents.RemoveAll([this, PlayerLocation](const FCrowd_AgentData& Agent)
    {
        float Distance = FVector::Dist(Agent.Location, PlayerLocation);
        if (Distance > CullingDistance)
        {
            CurrentAgentCount--;
            return true;
        }
        return false;
    });
}

TArray<FCrowd_AgentData> ACrowd_MassSimulationManager::GetNearbyAgents(FVector Location, float Radius)
{
    TArray<FCrowd_AgentData> NearbyAgents;
    
    for (const FCrowd_AgentData& Agent : ActiveAgents)
    {
        float Distance = FVector::Dist(Agent.Location, Location);
        if (Distance <= Radius)
        {
            NearbyAgents.Add(Agent);
        }
    }
    
    return NearbyAgents;
}

void ACrowd_MassSimulationManager::SetMaxActiveAgents(int32 NewMax)
{
    MaxActiveAgents = FMath::Max(0, NewMax);
    
    // Cull excess agents if necessary
    if (CurrentAgentCount > MaxActiveAgents)
    {
        int32 ExcessCount = CurrentAgentCount - MaxActiveAgents;
        ActiveAgents.RemoveAt(ActiveAgents.Num() - ExcessCount, ExcessCount);
        CurrentAgentCount = MaxActiveAgents;
    }
}

int32 ACrowd_MassSimulationManager::GetActiveAgentCount() const
{
    return CurrentAgentCount;
}

void ACrowd_MassSimulationManager::OptimizePerformance()
{
    // Limit high detail agents
    FVector PlayerLocation = GetPlayerLocation();
    int32 HighDetailCount = 0;
    
    for (const FCrowd_AgentData& Agent : ActiveAgents)
    {
        float Distance = FVector::Dist(Agent.Location, PlayerLocation);
        if (Distance <= LODSettings.HighDetailDistance)
        {
            HighDetailCount++;
        }
    }
    
    if (HighDetailCount > LODSettings.MaxHighDetailAgents)
    {
        // Reduce update frequency for distant agents
        UpdateFrequency = FMath::Min(UpdateFrequency * 1.1f, 0.5f);
    }
    else
    {
        // Restore normal update frequency
        UpdateFrequency = FMath::Max(UpdateFrequency * 0.95f, 0.1f);
    }
}

FVector ACrowd_MassSimulationManager::GetPlayerLocation() const
{
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            if (APawn* PlayerPawn = PC->GetPawn())
            {
                return PlayerPawn->GetActorLocation();
            }
        }
    }
    return FVector::ZeroVector;
}

bool ACrowd_MassSimulationManager::ShouldUpdateAgent(const FCrowd_AgentData& Agent, float DeltaTime) const
{
    FVector PlayerLocation = GetPlayerLocation();
    float Distance = FVector::Dist(Agent.Location, PlayerLocation);
    
    if (Distance <= LODSettings.HighDetailDistance)
    {
        return true; // Always update high detail
    }
    else if (Distance <= LODSettings.MediumDetailDistance)
    {
        return FMath::RandRange(0, 100) < 70; // 70% chance
    }
    else if (Distance <= LODSettings.LowDetailDistance)
    {
        return FMath::RandRange(0, 100) < 30; // 30% chance
    }
    
    return false; // Don't update very distant agents
}

void ACrowd_MassSimulationManager::ApplyFlockingBehavior(FCrowd_AgentData& Agent, const TArray<FCrowd_AgentData>& NearbyAgents)
{
    if (NearbyAgents.Num() <= 1) // Only the agent itself
    {
        // Random wandering
        FVector RandomDirection = FVector(
            FMath::RandRange(-1.0f, 1.0f),
            FMath::RandRange(-1.0f, 1.0f),
            0.0f
        ).GetSafeNormal();
        
        Agent.Velocity += RandomDirection * 50.0f;
        return;
    }
    
    // Flocking forces
    FVector Separation = FVector::ZeroVector;
    FVector Alignment = FVector::ZeroVector;
    FVector Cohesion = FVector::ZeroVector;
    
    int32 ValidNeighbors = 0;
    
    for (const FCrowd_AgentData& Neighbor : NearbyAgents)
    {
        if (FVector::Dist(Neighbor.Location, Agent.Location) < 10.0f) // Skip self
            continue;
            
        ValidNeighbors++;
        
        // Separation
        FVector ToNeighbor = Agent.Location - Neighbor.Location;
        float Distance = ToNeighbor.Size();
        if (Distance > 0.0f && Distance < 100.0f)
        {
            Separation += ToNeighbor.GetSafeNormal() / Distance;
        }
        
        // Alignment
        Alignment += Neighbor.Velocity;
        
        // Cohesion
        Cohesion += Neighbor.Location;
    }
    
    if (ValidNeighbors > 0)
    {
        // Apply flocking forces
        Separation = Separation.GetSafeNormal() * 100.0f;
        Alignment = (Alignment / ValidNeighbors).GetSafeNormal() * 50.0f;
        Cohesion = ((Cohesion / ValidNeighbors) - Agent.Location).GetSafeNormal() * 30.0f;
        
        Agent.Velocity += Separation + Alignment + Cohesion;
    }
    
    // Clamp velocity
    float MaxSpeed = 200.0f;
    if (Agent.Velocity.Size() > MaxSpeed)
    {
        Agent.Velocity = Agent.Velocity.GetSafeNormal() * MaxSpeed;
    }
}