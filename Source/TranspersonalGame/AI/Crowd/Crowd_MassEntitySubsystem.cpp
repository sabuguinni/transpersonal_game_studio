#include "Crowd_MassEntitySubsystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"

UCrowd_MassEntitySubsystem::UCrowd_MassEntitySubsystem()
{
    bIsInitialized = false;
    LastUpdateTime = 0.0f;
    CurrentParameters = FCrowd_SpawnParameters();
}

void UCrowd_MassEntitySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Crowd Mass Entity Subsystem Initializing"));
    InitializeCrowdSystem();
}

void UCrowd_MassEntitySubsystem::Deinitialize()
{
    ClearAllAgents();
    bIsInitialized = false;
    
    UE_LOG(LogTemp, Log, TEXT("Crowd Mass Entity Subsystem Deinitialized"));
    Super::Deinitialize();
}

void UCrowd_MassEntitySubsystem::InitializeCrowdSystem()
{
    if (bIsInitialized)
    {
        return;
    }

    // Clear any existing agents
    ActiveAgents.Empty();
    
    // Set default parameters
    CurrentParameters.MaxAgents = 50;
    CurrentParameters.SpawnRadius = 200.0f;
    CurrentParameters.SeparationDistance = 100.0f;
    CurrentParameters.FollowDistance = 300.0f;
    CurrentParameters.SpawnCenter = FVector::ZeroVector;
    
    bIsInitialized = true;
    LastUpdateTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Log, TEXT("Crowd system initialized with %d max agents"), CurrentParameters.MaxAgents);
}

void UCrowd_MassEntitySubsystem::SpawnCrowdAgents(const FCrowd_SpawnParameters& SpawnParams)
{
    if (!bIsInitialized)
    {
        InitializeCrowdSystem();
    }

    // Clear existing agents first
    ClearAllAgents();
    
    // Update parameters
    CurrentParameters = SpawnParams;
    
    // Spawn new agents
    for (int32 i = 0; i < SpawnParams.MaxAgents; ++i)
    {
        FCrowd_AgentData NewAgent;
        NewAgent.AgentID = i;
        
        // Generate random spawn position within radius
        float Angle = FMath::RandRange(0.0f, 2.0f * PI);
        float Distance = FMath::RandRange(50.0f, SpawnParams.SpawnRadius);
        
        FVector SpawnOffset = FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.0f
        );
        
        NewAgent.Position = SpawnParams.SpawnCenter + SpawnOffset;
        NewAgent.MovementSpeed = FMath::RandRange(100.0f, 200.0f);
        NewAgent.Velocity = FVector::ZeroVector;
        NewAgent.BehaviorState = ECrowd_BehaviorState::Wandering;
        
        // Validate spawn location
        if (IsValidSpawnLocation(NewAgent.Position))
        {
            ActiveAgents.Add(NewAgent);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Spawned %d crowd agents"), ActiveAgents.Num());
}

void UCrowd_MassEntitySubsystem::UpdateCrowdSimulation(float DeltaTime)
{
    if (!bIsInitialized || ActiveAgents.Num() == 0)
    {
        return;
    }

    LastUpdateTime = GetWorld()->GetTimeSeconds();
    
    // Update each agent
    for (FCrowd_AgentData& Agent : ActiveAgents)
    {
        UpdateAgentBehavior(Agent, DeltaTime);
        ApplyFlockingBehavior(Agent);
        ApplyAvoidanceBehavior(Agent);
        
        // Update position
        Agent.Position += Agent.Velocity * DeltaTime;
        
        // Keep agents within bounds
        float MaxDistance = CurrentParameters.SpawnRadius * 2.0f;
        FVector ToCenter = CurrentParameters.SpawnCenter - Agent.Position;
        if (ToCenter.Size() > MaxDistance)
        {
            Agent.Velocity += ToCenter.GetSafeNormal() * 100.0f;
        }
        
        // Clamp velocity
        if (Agent.Velocity.Size() > Agent.MovementSpeed)
        {
            Agent.Velocity = Agent.Velocity.GetSafeNormal() * Agent.MovementSpeed;
        }
    }
}

void UCrowd_MassEntitySubsystem::ClearAllAgents()
{
    ActiveAgents.Empty();
    UE_LOG(LogTemp, Log, TEXT("Cleared all crowd agents"));
}

int32 UCrowd_MassEntitySubsystem::GetActiveAgentCount() const
{
    return ActiveAgents.Num();
}

TArray<FCrowd_AgentData> UCrowd_MassEntitySubsystem::GetAllAgentData() const
{
    return ActiveAgents;
}

void UCrowd_MassEntitySubsystem::SetCrowdParameters(const FCrowd_SpawnParameters& NewParams)
{
    CurrentParameters = NewParams;
    UE_LOG(LogTemp, Log, TEXT("Updated crowd parameters: MaxAgents=%d, SpawnRadius=%.1f"), 
           NewParams.MaxAgents, NewParams.SpawnRadius);
}

void UCrowd_MassEntitySubsystem::UpdateAgentBehavior(FCrowd_AgentData& Agent, float DeltaTime)
{
    switch (Agent.BehaviorState)
    {
        case ECrowd_BehaviorState::Wandering:
            // Random wandering behavior
            if (FMath::RandRange(0.0f, 1.0f) < 0.1f * DeltaTime)
            {
                FVector RandomDirection = FVector(
                    FMath::RandRange(-1.0f, 1.0f),
                    FMath::RandRange(-1.0f, 1.0f),
                    0.0f
                ).GetSafeNormal();
                
                Agent.Velocity += RandomDirection * 50.0f;
            }
            break;
            
        case ECrowd_BehaviorState::Following:
            // Follow behavior - move toward spawn center
            {
                FVector ToTarget = CurrentParameters.SpawnCenter - Agent.Position;
                if (ToTarget.Size() > CurrentParameters.FollowDistance)
                {
                    Agent.Velocity += ToTarget.GetSafeNormal() * 75.0f;
                }
            }
            break;
            
        case ECrowd_BehaviorState::Fleeing:
            // Flee behavior - move away from spawn center
            {
                FVector FromTarget = Agent.Position - CurrentParameters.SpawnCenter;
                Agent.Velocity += FromTarget.GetSafeNormal() * 100.0f;
            }
            break;
            
        default:
            break;
    }
}

void UCrowd_MassEntitySubsystem::ApplyFlockingBehavior(FCrowd_AgentData& Agent)
{
    FVector Separation = CalculateSeparation(Agent);
    FVector Alignment = CalculateAlignment(Agent);
    FVector Cohesion = CalculateCohesion(Agent);
    
    // Apply flocking forces with weights
    Agent.Velocity += Separation * 2.0f;  // Strong separation
    Agent.Velocity += Alignment * 1.0f;   // Moderate alignment
    Agent.Velocity += Cohesion * 0.5f;    // Weak cohesion
}

void UCrowd_MassEntitySubsystem::ApplyAvoidanceBehavior(FCrowd_AgentData& Agent)
{
    // Simple obstacle avoidance - avoid getting too close to other agents
    for (const FCrowd_AgentData& OtherAgent : ActiveAgents)
    {
        if (OtherAgent.AgentID == Agent.AgentID)
        {
            continue;
        }
        
        FVector ToOther = OtherAgent.Position - Agent.Position;
        float Distance = ToOther.Size();
        
        if (Distance < CurrentParameters.SeparationDistance && Distance > 0.0f)
        {
            FVector AvoidanceForce = -ToOther.GetSafeNormal() * (CurrentParameters.SeparationDistance - Distance);
            Agent.Velocity += AvoidanceForce;
        }
    }
}

FVector UCrowd_MassEntitySubsystem::CalculateSeparation(const FCrowd_AgentData& Agent)
{
    FVector SeparationForce = FVector::ZeroVector;
    int32 NeighborCount = 0;
    
    for (const FCrowd_AgentData& OtherAgent : ActiveAgents)
    {
        if (OtherAgent.AgentID == Agent.AgentID)
        {
            continue;
        }
        
        FVector ToOther = OtherAgent.Position - Agent.Position;
        float Distance = ToOther.Size();
        
        if (Distance < CurrentParameters.SeparationDistance && Distance > 0.0f)
        {
            SeparationForce -= ToOther.GetSafeNormal() / Distance;
            NeighborCount++;
        }
    }
    
    if (NeighborCount > 0)
    {
        SeparationForce /= NeighborCount;
    }
    
    return SeparationForce;
}

FVector UCrowd_MassEntitySubsystem::CalculateAlignment(const FCrowd_AgentData& Agent)
{
    FVector AverageVelocity = FVector::ZeroVector;
    int32 NeighborCount = 0;
    
    for (const FCrowd_AgentData& OtherAgent : ActiveAgents)
    {
        if (OtherAgent.AgentID == Agent.AgentID)
        {
            continue;
        }
        
        float Distance = FVector::Dist(Agent.Position, OtherAgent.Position);
        if (Distance < CurrentParameters.FollowDistance)
        {
            AverageVelocity += OtherAgent.Velocity;
            NeighborCount++;
        }
    }
    
    if (NeighborCount > 0)
    {
        AverageVelocity /= NeighborCount;
        return AverageVelocity - Agent.Velocity;
    }
    
    return FVector::ZeroVector;
}

FVector UCrowd_MassEntitySubsystem::CalculateCohesion(const FCrowd_AgentData& Agent)
{
    FVector CenterOfMass = FVector::ZeroVector;
    int32 NeighborCount = 0;
    
    for (const FCrowd_AgentData& OtherAgent : ActiveAgents)
    {
        if (OtherAgent.AgentID == Agent.AgentID)
        {
            continue;
        }
        
        float Distance = FVector::Dist(Agent.Position, OtherAgent.Position);
        if (Distance < CurrentParameters.FollowDistance)
        {
            CenterOfMass += OtherAgent.Position;
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

bool UCrowd_MassEntitySubsystem::IsValidSpawnLocation(const FVector& Location)
{
    // Basic validation - check if location is not too close to existing agents
    for (const FCrowd_AgentData& ExistingAgent : ActiveAgents)
    {
        if (FVector::Dist(Location, ExistingAgent.Position) < 50.0f)
        {
            return false;
        }
    }
    
    return true;
}