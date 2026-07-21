#include "Crowd_MassSimulationSubsystem.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

UCrowd_MassSimulationSubsystem::UCrowd_MassSimulationSubsystem()
{
    MaxCrowdAgents = 1000;
    AgentSeparationRadius = 100.0f;
    MaxSpeed = 200.0f;
    bIsInitialized = false;
}

void UCrowd_MassSimulationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd Mass Simulation Subsystem Initialized"));
    InitializeMassSimulation();
}

void UCrowd_MassSimulationSubsystem::Deinitialize()
{
    CrowdAgents.Empty();
    bIsInitialized = false;
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd Mass Simulation Subsystem Deinitialized"));
    Super::Deinitialize();
}

void UCrowd_MassSimulationSubsystem::InitializeMassSimulation()
{
    if (bIsInitialized)
    {
        return;
    }

    CrowdAgents.Reserve(MaxCrowdAgents);
    GlobalTarget = FVector(0.0f, 0.0f, 100.0f);
    bIsInitialized = true;

    UE_LOG(LogTemp, Warning, TEXT("Mass Simulation initialized with capacity: %d"), MaxCrowdAgents);
}

void UCrowd_MassSimulationSubsystem::SpawnCrowdAgents(int32 NumAgents, FVector SpawnCenter, float SpawnRadius)
{
    if (!bIsInitialized)
    {
        InitializeMassSimulation();
    }

    int32 AgentsToSpawn = FMath::Min(NumAgents, MaxCrowdAgents - CrowdAgents.Num());
    
    for (int32 i = 0; i < AgentsToSpawn; i++)
    {
        FCrowd_AgentData NewAgent;
        
        // Random spawn position within radius
        float Angle = FMath::RandRange(0.0f, 2.0f * PI);
        float Distance = FMath::RandRange(0.0f, SpawnRadius);
        
        NewAgent.Location = SpawnCenter + FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.0f
        );
        
        NewAgent.Velocity = FVector::ZeroVector;
        NewAgent.Speed = FMath::RandRange(100.0f, MaxSpeed);
        NewAgent.TargetLocation = GlobalTarget;
        NewAgent.AgentID = CrowdAgents.Num() + i;
        NewAgent.bIsActive = true;
        
        CrowdAgents.Add(NewAgent);
    }

    UE_LOG(LogTemp, Warning, TEXT("Spawned %d crowd agents. Total: %d"), AgentsToSpawn, CrowdAgents.Num());
}

void UCrowd_MassSimulationSubsystem::UpdateCrowdSimulation(float DeltaTime)
{
    if (!bIsInitialized || CrowdAgents.Num() == 0)
    {
        return;
    }

    for (FCrowd_AgentData& Agent : CrowdAgents)
    {
        if (Agent.bIsActive)
        {
            UpdateAgentMovement(Agent, DeltaTime);
        }
    }
}

void UCrowd_MassSimulationSubsystem::SetCrowdTarget(FVector NewTarget)
{
    GlobalTarget = NewTarget;
    
    for (FCrowd_AgentData& Agent : CrowdAgents)
    {
        Agent.TargetLocation = GlobalTarget;
    }

    UE_LOG(LogTemp, Log, TEXT("Crowd target set to: %s"), *GlobalTarget.ToString());
}

int32 UCrowd_MassSimulationSubsystem::GetActiveCrowdCount() const
{
    int32 ActiveCount = 0;
    for (const FCrowd_AgentData& Agent : CrowdAgents)
    {
        if (Agent.bIsActive)
        {
            ActiveCount++;
        }
    }
    return ActiveCount;
}

void UCrowd_MassSimulationSubsystem::UpdateAgentMovement(FCrowd_AgentData& Agent, float DeltaTime)
{
    FVector SteeringForce = CalculateSteeringForce(Agent);
    
    // Apply steering force to velocity
    Agent.Velocity += SteeringForce * DeltaTime;
    
    // Limit velocity to max speed
    if (Agent.Velocity.Size() > Agent.Speed)
    {
        Agent.Velocity = Agent.Velocity.GetSafeNormal() * Agent.Speed;
    }
    
    // Update position
    Agent.Location += Agent.Velocity * DeltaTime;
}

FVector UCrowd_MassSimulationSubsystem::CalculateSteeringForce(const FCrowd_AgentData& Agent)
{
    FVector SeekForce = CalculateSeekForce(Agent);
    FVector SeparationForce = CalculateSeparationForce(Agent);
    
    // Combine forces with weights
    FVector TotalForce = SeekForce * 0.6f + SeparationForce * 0.4f;
    
    return TotalForce;
}

FVector UCrowd_MassSimulationSubsystem::CalculateSeekForce(const FCrowd_AgentData& Agent)
{
    FVector DesiredVelocity = (Agent.TargetLocation - Agent.Location).GetSafeNormal() * Agent.Speed;
    return DesiredVelocity - Agent.Velocity;
}

FVector UCrowd_MassSimulationSubsystem::CalculateSeparationForce(const FCrowd_AgentData& Agent)
{
    FVector SeparationForce = FVector::ZeroVector;
    int32 NeighborCount = 0;
    
    for (const FCrowd_AgentData& OtherAgent : CrowdAgents)
    {
        if (OtherAgent.AgentID != Agent.AgentID && OtherAgent.bIsActive)
        {
            float Distance = FVector::Dist(Agent.Location, OtherAgent.Location);
            
            if (Distance < AgentSeparationRadius && Distance > 0.0f)
            {
                FVector AwayVector = (Agent.Location - OtherAgent.Location).GetSafeNormal();
                SeparationForce += AwayVector / Distance; // Closer agents have stronger repulsion
                NeighborCount++;
            }
        }
    }
    
    if (NeighborCount > 0)
    {
        SeparationForce /= NeighborCount;
        SeparationForce = SeparationForce.GetSafeNormal() * Agent.Speed;
    }
    
    return SeparationForce;
}