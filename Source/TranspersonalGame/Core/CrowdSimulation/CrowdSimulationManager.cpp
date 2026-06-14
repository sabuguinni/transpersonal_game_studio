#include "CrowdSimulationManager.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

ACrowdSimulationManager::ACrowdSimulationManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    MaxAgents = 500;
    SpawnRadius = 2000.0f;
    AgentSpeed = 150.0f;
    FlockingRadius = 300.0f;
    bIsFleeingActive = false;
    FleeTargetLocation = FVector::ZeroVector;
}

void ACrowdSimulationManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Auto-spawn initial crowd
    SpawnCrowdAgents(FMath::Min(MaxAgents, 100));
}

void ACrowdSimulationManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateAgentMovement(DeltaTime);
}

void ACrowdSimulationManager::SpawnCrowdAgents(int32 NumAgents)
{
    CrowdAgents.Empty();
    
    for (int32 i = 0; i < NumAgents; i++)
    {
        FCrowd_AgentData NewAgent;
        NewAgent.AgentID = i;
        
        // Random spawn position within radius
        float Angle = FMath::RandRange(0.0f, 2.0f * PI);
        float Distance = FMath::RandRange(100.0f, SpawnRadius);
        
        NewAgent.Location = GetActorLocation() + FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.0f
        );
        
        // Random initial velocity
        NewAgent.Velocity = FVector(
            FMath::RandRange(-1.0f, 1.0f),
            FMath::RandRange(-1.0f, 1.0f),
            0.0f
        ).GetSafeNormal() * AgentSpeed;
        
        NewAgent.Speed = FMath::RandRange(AgentSpeed * 0.8f, AgentSpeed * 1.2f);
        NewAgent.BehaviorState = ECrowd_BehaviorState::Wandering;
        
        CrowdAgents.Add(NewAgent);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulation: Spawned %d agents"), NumAgents);
}

void ACrowdSimulationManager::UpdateAgentBehavior(int32 AgentIndex, ECrowd_BehaviorState NewState)
{
    if (CrowdAgents.IsValidIndex(AgentIndex))
    {
        CrowdAgents[AgentIndex].BehaviorState = NewState;
    }
}

void ACrowdSimulationManager::SetFleeTarget(FVector FleeFromLocation)
{
    FleeTargetLocation = FleeFromLocation;
    bIsFleeingActive = true;
    
    // Set all agents to flee state
    for (FCrowd_AgentData& Agent : CrowdAgents)
    {
        Agent.BehaviorState = ECrowd_BehaviorState::Fleeing;
    }
}

void ACrowdSimulationManager::UpdateAgentMovement(float DeltaTime)
{
    for (int32 i = 0; i < CrowdAgents.Num(); i++)
    {
        FCrowd_AgentData& Agent = CrowdAgents[i];
        
        switch (Agent.BehaviorState)
        {
            case ECrowd_BehaviorState::Wandering:
                ApplyFlocking(i);
                break;
                
            case ECrowd_BehaviorState::Fleeing:
                if (bIsFleeingActive)
                {
                    ApplyFleeBehavior(i, FleeTargetLocation);
                }
                break;
                
            case ECrowd_BehaviorState::Following:
                // Follow player or leader
                ApplyFlocking(i);
                break;
                
            default:
                ApplyFlocking(i);
                break;
        }
        
        // Update position
        Agent.Location += Agent.Velocity * DeltaTime;
        
        // Keep agents within bounds
        FVector CenterOffset = Agent.Location - GetActorLocation();
        if (CenterOffset.Size() > SpawnRadius * 1.5f)
        {
            Agent.Velocity = -CenterOffset.GetSafeNormal() * Agent.Speed;
        }
    }
}

void ACrowdSimulationManager::ApplyFlocking(int32 AgentIndex)
{
    if (!CrowdAgents.IsValidIndex(AgentIndex))
        return;
        
    FCrowd_AgentData& Agent = CrowdAgents[AgentIndex];
    
    FVector Separation = FVector::ZeroVector;
    FVector Alignment = FVector::ZeroVector;
    FVector Cohesion = FVector::ZeroVector;
    int32 NeighborCount = 0;
    
    // Check neighbors within flocking radius
    for (int32 j = 0; j < CrowdAgents.Num(); j++)
    {
        if (j == AgentIndex) continue;
        
        const FCrowd_AgentData& Other = CrowdAgents[j];
        float Distance = FVector::Dist(Agent.Location, Other.Location);
        
        if (Distance < FlockingRadius)
        {
            NeighborCount++;
            
            // Separation - avoid crowding
            if (Distance < FlockingRadius * 0.3f && Distance > 0.0f)
            {
                FVector Diff = Agent.Location - Other.Location;
                Separation += Diff.GetSafeNormal() / Distance;
            }
            
            // Alignment - steer towards average heading
            Alignment += Other.Velocity;
            
            // Cohesion - steer towards average position
            Cohesion += Other.Location;
        }
    }
    
    if (NeighborCount > 0)
    {
        // Average the forces
        Alignment /= NeighborCount;
        Cohesion /= NeighborCount;
        Cohesion = (Cohesion - Agent.Location).GetSafeNormal() * Agent.Speed;
        
        // Apply weights
        FVector NewVelocity = Agent.Velocity;
        NewVelocity += Separation * 2.0f;
        NewVelocity += Alignment * 0.5f;
        NewVelocity += Cohesion * 0.3f;
        
        Agent.Velocity = NewVelocity.GetSafeNormal() * Agent.Speed;
    }
    
    // Add some randomness
    FVector RandomForce = FVector(
        FMath::RandRange(-1.0f, 1.0f),
        FMath::RandRange(-1.0f, 1.0f),
        0.0f
    ) * 10.0f;
    
    Agent.Velocity += RandomForce;
    Agent.Velocity = Agent.Velocity.GetSafeNormal() * Agent.Speed;
}

void ACrowdSimulationManager::ApplyFleeBehavior(int32 AgentIndex, FVector FleeTarget)
{
    if (!CrowdAgents.IsValidIndex(AgentIndex))
        return;
        
    FCrowd_AgentData& Agent = CrowdAgents[AgentIndex];
    
    FVector FleeDirection = Agent.Location - FleeTarget;
    float Distance = FleeDirection.Size();
    
    if (Distance > 0.0f)
    {
        // Stronger flee force when closer to target
        float FleeStrength = FMath::Clamp(1000.0f / Distance, 0.5f, 3.0f);
        Agent.Velocity = FleeDirection.GetSafeNormal() * Agent.Speed * FleeStrength;
    }
    
    // Stop fleeing when far enough
    if (Distance > SpawnRadius)
    {
        Agent.BehaviorState = ECrowd_BehaviorState::Wandering;
        bIsFleeingActive = false;
    }
}