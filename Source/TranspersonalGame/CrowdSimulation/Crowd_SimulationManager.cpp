#include "Crowd_SimulationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "AI/NavigationSystemBase.h"

UCrowd_SimulationManager::UCrowd_SimulationManager()
{
    TickInterval = 0.1f;
    SimulationSpeed = 1.0f;
    MaxAgents = 5000;
    LODDistance = 2000.0f;
    bSimulationActive = false;
    bSimulationPaused = false;

    // Flocking parameters
    SeparationWeight = 2.0f;
    AlignmentWeight = 1.0f;
    CohesionWeight = 1.0f;
    AvoidanceWeight = 3.0f;
    SeekWeight = 1.5f;
}

void UCrowd_SimulationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd Simulation Manager initialized"));
    
    // Reserve space for agents and groups
    Agents.Reserve(MaxAgents);
    Groups.Reserve(MaxAgents / 10); // Assume average group size of 10
    FreeAgentIndices.Reserve(MaxAgents);
    FreeGroupIndices.Reserve(MaxAgents / 10);
}

void UCrowd_SimulationManager::Deinitialize()
{
    StopSimulation();
    Agents.Empty();
    Groups.Empty();
    FreeAgentIndices.Empty();
    FreeGroupIndices.Empty();
    
    Super::Deinitialize();
}

int32 UCrowd_SimulationManager::RegisterAgent(const FVector& StartPosition, int32 GroupID)
{
    if (Agents.Num() >= MaxAgents)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot register agent: Maximum agent count reached"));
        return -1;
    }

    int32 AgentIndex = GetNextAvailableAgentIndex();
    
    FCrowd_AgentData NewAgent;
    NewAgent.Position = StartPosition;
    NewAgent.TargetLocation = StartPosition;
    NewAgent.GroupID = GroupID;
    NewAgent.BehaviorState = ECrowd_BehaviorState::Idle;

    if (AgentIndex < Agents.Num())
    {
        Agents[AgentIndex] = NewAgent;
    }
    else
    {
        Agents.Add(NewAgent);
    }

    UE_LOG(LogTemp, Log, TEXT("Registered agent %d at position %s"), AgentIndex, *StartPosition.ToString());
    return AgentIndex;
}

void UCrowd_SimulationManager::UnregisterAgent(int32 AgentIndex)
{
    if (!IsValidAgentIndex(AgentIndex))
    {
        return;
    }

    // Remove from group if assigned
    int32 GroupID = Agents[AgentIndex].GroupID;
    if (GroupID >= 0 && IsValidGroupIndex(GroupID))
    {
        Groups[GroupID].AgentIndices.Remove(AgentIndex);
    }

    // Mark index as free
    FreeAgentIndices.Add(AgentIndex);
    
    // Reset agent data
    Agents[AgentIndex] = FCrowd_AgentData();
    
    UE_LOG(LogTemp, Log, TEXT("Unregistered agent %d"), AgentIndex);
}

void UCrowd_SimulationManager::UpdateAgentTarget(int32 AgentIndex, const FVector& NewTarget)
{
    if (!IsValidAgentIndex(AgentIndex))
    {
        return;
    }

    Agents[AgentIndex].TargetLocation = NewTarget;
    Agents[AgentIndex].BehaviorState = ECrowd_BehaviorState::Moving;
}

void UCrowd_SimulationManager::SetAgentBehavior(int32 AgentIndex, ECrowd_BehaviorState NewBehavior)
{
    if (!IsValidAgentIndex(AgentIndex))
    {
        return;
    }

    Agents[AgentIndex].BehaviorState = NewBehavior;
}

void UCrowd_SimulationManager::TriggerPanicResponse(const FVector& ThreatLocation, float ThreatRadius)
{
    for (int32 i = 0; i < Agents.Num(); ++i)
    {
        if (FreeAgentIndices.Contains(i))
        {
            continue;
        }

        float Distance = FVector::Dist(Agents[i].Position, ThreatLocation);
        if (Distance <= ThreatRadius)
        {
            // Set panic behavior
            Agents[i].BehaviorState = ECrowd_BehaviorState::Fleeing;
            Agents[i].PanicLevel = FMath::Clamp(1.0f - (Distance / ThreatRadius), 0.1f, 1.0f);
            
            // Set flee target away from threat
            FVector FleeDirection = (Agents[i].Position - ThreatLocation).GetSafeNormal();
            Agents[i].TargetLocation = Agents[i].Position + FleeDirection * 1000.0f;
            Agents[i].Speed = 300.0f; // Panic speed
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Panic response triggered at %s with radius %f"), *ThreatLocation.ToString(), ThreatRadius);
}

int32 UCrowd_SimulationManager::CreateGroup(const TArray<int32>& AgentIndices)
{
    int32 GroupIndex = GetNextAvailableGroupIndex();
    
    FCrowd_GroupData NewGroup;
    for (int32 AgentIndex : AgentIndices)
    {
        if (IsValidAgentIndex(AgentIndex))
        {
            NewGroup.AgentIndices.Add(AgentIndex);
            Agents[AgentIndex].GroupID = GroupIndex;
        }
    }

    if (GroupIndex < Groups.Num())
    {
        Groups[GroupIndex] = NewGroup;
    }
    else
    {
        Groups.Add(NewGroup);
    }

    UE_LOG(LogTemp, Log, TEXT("Created group %d with %d agents"), GroupIndex, NewGroup.AgentIndices.Num());
    return GroupIndex;
}

void UCrowd_SimulationManager::SetGroupTarget(int32 GroupID, const FVector& Target)
{
    if (!IsValidGroupIndex(GroupID))
    {
        return;
    }

    Groups[GroupID].GroupTarget = Target;
    
    // Update all agents in group
    for (int32 AgentIndex : Groups[GroupID].AgentIndices)
    {
        if (IsValidAgentIndex(AgentIndex))
        {
            Agents[AgentIndex].TargetLocation = Target;
            Agents[AgentIndex].BehaviorState = ECrowd_BehaviorState::Moving;
        }
    }
}

void UCrowd_SimulationManager::SetGroupBehavior(int32 GroupID, ECrowd_BehaviorState Behavior)
{
    if (!IsValidGroupIndex(GroupID))
    {
        return;
    }

    Groups[GroupID].GroupBehavior = Behavior;
    
    // Update all agents in group
    for (int32 AgentIndex : Groups[GroupID].AgentIndices)
    {
        if (IsValidAgentIndex(AgentIndex))
        {
            Agents[AgentIndex].BehaviorState = Behavior;
        }
    }
}

void UCrowd_SimulationManager::StartSimulation()
{
    if (bSimulationActive)
    {
        return;
    }

    bSimulationActive = true;
    bSimulationPaused = false;

    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().SetTimer(
            SimulationTimerHandle,
            this,
            &UCrowd_SimulationManager::TickSimulation,
            TickInterval,
            true
        );
    }

    UE_LOG(LogTemp, Warning, TEXT("Crowd simulation started"));
}

void UCrowd_SimulationManager::StopSimulation()
{
    if (!bSimulationActive)
    {
        return;
    }

    bSimulationActive = false;
    bSimulationPaused = false;

    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().ClearTimer(SimulationTimerHandle);
    }

    UE_LOG(LogTemp, Warning, TEXT("Crowd simulation stopped"));
}

void UCrowd_SimulationManager::PauseSimulation(bool bPause)
{
    bSimulationPaused = bPause;
    UE_LOG(LogTemp, Log, TEXT("Crowd simulation %s"), bPause ? TEXT("paused") : TEXT("resumed"));
}

void UCrowd_SimulationManager::SetSimulationSpeed(float Speed)
{
    SimulationSpeed = FMath::Clamp(Speed, 0.1f, 5.0f);
}

void UCrowd_SimulationManager::SetLODDistance(float Distance)
{
    LODDistance = FMath::Max(Distance, 100.0f);
}

void UCrowd_SimulationManager::UpdateLODForAgent(int32 AgentIndex, float DistanceToPlayer)
{
    if (!IsValidAgentIndex(AgentIndex))
    {
        return;
    }

    // Simple LOD system - reduce update frequency for distant agents
    if (DistanceToPlayer > LODDistance * 2.0f)
    {
        // Very low LOD - minimal updates
        Agents[AgentIndex].Speed = Agents[AgentIndex].Speed * 0.5f;
    }
    else if (DistanceToPlayer > LODDistance)
    {
        // Medium LOD - reduced updates
        Agents[AgentIndex].Speed = Agents[AgentIndex].Speed * 0.75f;
    }
    // High LOD - full updates (no modification needed)
}

TArray<FCrowd_AgentData> UCrowd_SimulationManager::GetAgentsInRadius(const FVector& Center, float Radius)
{
    TArray<FCrowd_AgentData> Result;
    
    for (int32 i = 0; i < Agents.Num(); ++i)
    {
        if (FreeAgentIndices.Contains(i))
        {
            continue;
        }

        if (FVector::Dist(Agents[i].Position, Center) <= Radius)
        {
            Result.Add(Agents[i]);
        }
    }

    return Result;
}

FCrowd_AgentData UCrowd_SimulationManager::GetAgentData(int32 AgentIndex)
{
    if (IsValidAgentIndex(AgentIndex))
    {
        return Agents[AgentIndex];
    }
    
    return FCrowd_AgentData();
}

int32 UCrowd_SimulationManager::GetActiveAgentCount() const
{
    return Agents.Num() - FreeAgentIndices.Num();
}

float UCrowd_SimulationManager::GetAverageAgentDensity() const
{
    int32 ActiveAgents = GetActiveAgentCount();
    if (ActiveAgents == 0)
    {
        return 0.0f;
    }

    // Calculate density based on agents per square meter in a 1000x1000 area
    return static_cast<float>(ActiveAgents) / (1000.0f * 1000.0f);
}

void UCrowd_SimulationManager::TickSimulation()
{
    if (!bSimulationActive || bSimulationPaused)
    {
        return;
    }

    float DeltaTime = TickInterval * SimulationSpeed;

    // Update all active agents
    for (int32 i = 0; i < Agents.Num(); ++i)
    {
        if (!FreeAgentIndices.Contains(i))
        {
            UpdateAgent(i, DeltaTime);
        }
    }

    // Update all active groups
    for (int32 i = 0; i < Groups.Num(); ++i)
    {
        if (!FreeGroupIndices.Contains(i))
        {
            UpdateGroup(i, DeltaTime);
        }
    }
}

void UCrowd_SimulationManager::UpdateAgent(int32 AgentIndex, float DeltaTime)
{
    if (!IsValidAgentIndex(AgentIndex))
    {
        return;
    }

    FCrowd_AgentData& Agent = Agents[AgentIndex];
    
    // Calculate steering forces
    FVector SteeringForce = FVector::ZeroVector;
    
    switch (Agent.BehaviorState)
    {
        case ECrowd_BehaviorState::Moving:
        case ECrowd_BehaviorState::Following:
            SteeringForce += CalculateSeek(AgentIndex, Agent.TargetLocation) * SeekWeight;
            SteeringForce += CalculateSeparation(AgentIndex) * SeparationWeight;
            SteeringForce += CalculateAlignment(AgentIndex) * AlignmentWeight;
            SteeringForce += CalculateCohesion(AgentIndex) * CohesionWeight;
            break;
            
        case ECrowd_BehaviorState::Fleeing:
            SteeringForce += CalculateSeek(AgentIndex, Agent.TargetLocation) * SeekWeight * 2.0f;
            SteeringForce += CalculateSeparation(AgentIndex) * SeparationWeight * 2.0f;
            break;
            
        case ECrowd_BehaviorState::Idle:
        case ECrowd_BehaviorState::Waiting:
        default:
            SteeringForce += CalculateSeparation(AgentIndex) * SeparationWeight * 0.5f;
            break;
    }

    // Add avoidance
    SteeringForce += CalculateAvoidance(AgentIndex) * AvoidanceWeight;

    // Apply steering force to velocity
    Agent.Velocity += SteeringForce * DeltaTime;
    
    // Limit velocity to max speed
    float MaxSpeed = Agent.Speed * (1.0f + Agent.PanicLevel);
    if (Agent.Velocity.SizeSquared() > MaxSpeed * MaxSpeed)
    {
        Agent.Velocity = Agent.Velocity.GetSafeNormal() * MaxSpeed;
    }

    // Update position
    Agent.Position += Agent.Velocity * DeltaTime;

    // Reduce panic level over time
    if (Agent.PanicLevel > 0.0f)
    {
        Agent.PanicLevel = FMath::Max(0.0f, Agent.PanicLevel - DeltaTime * 0.5f);
        
        if (Agent.PanicLevel <= 0.1f)
        {
            Agent.BehaviorState = ECrowd_BehaviorState::Idle;
            Agent.Speed = 150.0f; // Reset to normal speed
        }
    }
}

void UCrowd_SimulationManager::UpdateGroup(int32 GroupIndex, float DeltaTime)
{
    if (!IsValidGroupIndex(GroupIndex))
    {
        return;
    }

    FCrowd_GroupData& Group = Groups[GroupIndex];
    
    // Calculate group center
    FVector CenterSum = FVector::ZeroVector;
    int32 ValidAgents = 0;
    
    for (int32 AgentIndex : Group.AgentIndices)
    {
        if (IsValidAgentIndex(AgentIndex))
        {
            CenterSum += Agents[AgentIndex].Position;
            ValidAgents++;
        }
    }
    
    if (ValidAgents > 0)
    {
        Group.GroupCenter = CenterSum / ValidAgents;
    }
}

FVector UCrowd_SimulationManager::CalculateSeparation(int32 AgentIndex)
{
    if (!IsValidAgentIndex(AgentIndex))
    {
        return FVector::ZeroVector;
    }

    const FCrowd_AgentData& Agent = Agents[AgentIndex];
    FVector SeparationForce = FVector::ZeroVector;
    int32 Count = 0;

    TArray<int32> NearbyAgents = GetNearbyAgents(AgentIndex, Agent.PersonalSpace);
    
    for (int32 OtherIndex : NearbyAgents)
    {
        if (OtherIndex != AgentIndex)
        {
            FVector Difference = Agent.Position - Agents[OtherIndex].Position;
            float Distance = Difference.Size();
            
            if (Distance > 0.0f && Distance < Agent.PersonalSpace)
            {
                Difference.Normalize();
                Difference /= Distance; // Weight by distance
                SeparationForce += Difference;
                Count++;
            }
        }
    }

    if (Count > 0)
    {
        SeparationForce /= Count;
        SeparationForce.Normalize();
        SeparationForce *= Agent.Speed;
        SeparationForce -= Agent.Velocity;
    }

    return SeparationForce;
}

FVector UCrowd_SimulationManager::CalculateAlignment(int32 AgentIndex)
{
    if (!IsValidAgentIndex(AgentIndex))
    {
        return FVector::ZeroVector;
    }

    const FCrowd_AgentData& Agent = Agents[AgentIndex];
    FVector AlignmentForce = FVector::ZeroVector;
    int32 Count = 0;

    TArray<int32> NearbyAgents = GetNearbyAgents(AgentIndex, Agent.PersonalSpace * 2.0f);
    
    for (int32 OtherIndex : NearbyAgents)
    {
        if (OtherIndex != AgentIndex)
        {
            AlignmentForce += Agents[OtherIndex].Velocity;
            Count++;
        }
    }

    if (Count > 0)
    {
        AlignmentForce /= Count;
        AlignmentForce.Normalize();
        AlignmentForce *= Agent.Speed;
        AlignmentForce -= Agent.Velocity;
    }

    return AlignmentForce;
}

FVector UCrowd_SimulationManager::CalculateCohesion(int32 AgentIndex)
{
    if (!IsValidAgentIndex(AgentIndex))
    {
        return FVector::ZeroVector;
    }

    const FCrowd_AgentData& Agent = Agents[AgentIndex];
    FVector CohesionForce = FVector::ZeroVector;
    int32 Count = 0;

    TArray<int32> NearbyAgents = GetNearbyAgents(AgentIndex, Agent.PersonalSpace * 3.0f);
    
    for (int32 OtherIndex : NearbyAgents)
    {
        if (OtherIndex != AgentIndex)
        {
            CohesionForce += Agents[OtherIndex].Position;
            Count++;
        }
    }

    if (Count > 0)
    {
        CohesionForce /= Count;
        return CalculateSeek(AgentIndex, CohesionForce);
    }

    return FVector::ZeroVector;
}

FVector UCrowd_SimulationManager::CalculateAvoidance(int32 AgentIndex)
{
    // Simplified obstacle avoidance - in a full implementation,
    // this would raycast for obstacles and calculate avoidance vectors
    return FVector::ZeroVector;
}

FVector UCrowd_SimulationManager::CalculateSeek(int32 AgentIndex, const FVector& Target)
{
    if (!IsValidAgentIndex(AgentIndex))
    {
        return FVector::ZeroVector;
    }

    const FCrowd_AgentData& Agent = Agents[AgentIndex];
    FVector DesiredVelocity = (Target - Agent.Position).GetSafeNormal() * Agent.Speed;
    return DesiredVelocity - Agent.Velocity;
}

TArray<int32> UCrowd_SimulationManager::GetNearbyAgents(int32 AgentIndex, float Radius)
{
    TArray<int32> NearbyAgents;
    
    if (!IsValidAgentIndex(AgentIndex))
    {
        return NearbyAgents;
    }

    const FVector& AgentPosition = Agents[AgentIndex].Position;
    
    for (int32 i = 0; i < Agents.Num(); ++i)
    {
        if (i != AgentIndex && !FreeAgentIndices.Contains(i))
        {
            if (FVector::Dist(AgentPosition, Agents[i].Position) <= Radius)
            {
                NearbyAgents.Add(i);
            }
        }
    }

    return NearbyAgents;
}

bool UCrowd_SimulationManager::IsValidAgentIndex(int32 Index) const
{
    return Index >= 0 && Index < Agents.Num() && !FreeAgentIndices.Contains(Index);
}

bool UCrowd_SimulationManager::IsValidGroupIndex(int32 Index) const
{
    return Index >= 0 && Index < Groups.Num() && !FreeGroupIndices.Contains(Index);
}

int32 UCrowd_SimulationManager::GetNextAvailableAgentIndex()
{
    if (FreeAgentIndices.Num() > 0)
    {
        return FreeAgentIndices.Pop();
    }
    
    return Agents.Num();
}

int32 UCrowd_SimulationManager::GetNextAvailableGroupIndex()
{
    if (FreeGroupIndices.Num() > 0)
    {
        return FreeGroupIndices.Pop();
    }
    
    return Groups.Num();
}