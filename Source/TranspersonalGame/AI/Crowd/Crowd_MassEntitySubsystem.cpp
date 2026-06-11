#include "Crowd_MassEntitySubsystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Crowd_SpawnPoint.h"
#include "Crowd_Waypoint.h"

UCrowd_MassEntitySubsystem::UCrowd_MassEntitySubsystem()
{
    // Initialize default spawn configuration
    SpawnConfig.MaxAgents = 50000;
    SpawnConfig.VisibleAgents = 1000;
    SpawnConfig.SpawnRadius = 2000.0f;
    SpawnConfig.CullingDistance = 2000.0f;
    SpawnConfig.UpdateFrequencyHigh = 30.0f;
    SpawnConfig.UpdateFrequencyLow = 5.0f;
}

void UCrowd_MassEntitySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd Mass Entity Subsystem Initialized"));
    
    // Initialize crowd simulation
    InitializeCrowdSimulation();
}

void UCrowd_MassEntitySubsystem::Deinitialize()
{
    // Clean up active agents
    ActiveAgents.Empty();
    SpawnPoints.Empty();
    Waypoints.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd Mass Entity Subsystem Deinitialized"));
    
    Super::Deinitialize();
}

bool UCrowd_MassEntitySubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    return Super::ShouldCreateSubsystem(Outer);
}

void UCrowd_MassEntitySubsystem::InitializeCrowdSimulation()
{
    if (!bEnableCrowdSimulation)
    {
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("Initializing crowd simulation for %d max agents"), SpawnConfig.MaxAgents);
    
    // Reserve memory for maximum agents
    ActiveAgents.Reserve(SpawnConfig.MaxAgents);
    
    // Find existing spawn points and waypoints in the world
    UWorld* World = GetWorld();
    if (World)
    {
        // Find spawn points
        TArray<AActor*> FoundSpawnPoints;
        UGameplayStatics::GetAllActorsOfClass(World, ACrowd_SpawnPoint::StaticClass(), FoundSpawnPoints);
        
        for (AActor* Actor : FoundSpawnPoints)
        {
            if (ACrowd_SpawnPoint* SpawnPoint = Cast<ACrowd_SpawnPoint>(Actor))
            {
                RegisterSpawnPoint(SpawnPoint);
            }
        }
        
        // Find waypoints
        TArray<AActor*> FoundWaypoints;
        UGameplayStatics::GetAllActorsOfClass(World, ACrowd_Waypoint::StaticClass(), FoundWaypoints);
        
        for (AActor* Actor : FoundWaypoints)
        {
            if (ACrowd_Waypoint* Waypoint = Cast<ACrowd_Waypoint>(Actor))
            {
                RegisterWaypoint(Waypoint);
            }
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Found %d spawn points and %d waypoints"), SpawnPoints.Num(), Waypoints.Num());
    }
}

void UCrowd_MassEntitySubsystem::SpawnCrowdAgents(int32 NumAgents, FVector SpawnCenter, float SpawnRadius)
{
    if (!bEnableCrowdSimulation || NumAgents <= 0)
    {
        return;
    }

    // Limit to maximum agents
    int32 AgentsToSpawn = FMath::Min(NumAgents, SpawnConfig.MaxAgents - ActiveAgents.Num());
    
    UE_LOG(LogTemp, Warning, TEXT("Spawning %d crowd agents at %s"), AgentsToSpawn, *SpawnCenter.ToString());
    
    for (int32 i = 0; i < AgentsToSpawn; i++)
    {
        // Generate random spawn location within radius
        FVector RandomOffset = FVector(
            FMath::RandRange(-SpawnRadius, SpawnRadius),
            FMath::RandRange(-SpawnRadius, SpawnRadius),
            0.0f
        );
        
        FVector SpawnLocation = SpawnCenter + RandomOffset;
        CreateAgentAtLocation(SpawnLocation);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Total active agents: %d"), ActiveAgents.Num());
}

void UCrowd_MassEntitySubsystem::CreateAgentAtLocation(const FVector& Location)
{
    FCrowd_AgentData NewAgent;
    NewAgent.Location = Location;
    NewAgent.Velocity = FVector::ZeroVector;
    NewAgent.Speed = FMath::RandRange(100.0f, 200.0f);
    NewAgent.BehaviorState = ECrowd_BehaviorState::Idle;
    NewAgent.AgentID = NextAgentID++;
    NewAgent.LODDistance = 0.0f;
    NewAgent.bIsVisible = true;
    
    ActiveAgents.Add(NewAgent);
}

void UCrowd_MassEntitySubsystem::UpdateCrowdSimulation(float DeltaTime)
{
    if (!bEnableCrowdSimulation || ActiveAgents.Num() == 0)
    {
        return;
    }

    LastUpdateTime += DeltaTime;
    
    // Update agents based on LOD
    for (FCrowd_AgentData& Agent : ActiveAgents)
    {
        // Determine update frequency based on distance
        float UpdateFreq = Agent.LODDistance < 500.0f ? SpawnConfig.UpdateFrequencyHigh : SpawnConfig.UpdateFrequencyLow;
        float UpdateInterval = 1.0f / UpdateFreq;
        
        if (LastUpdateTime >= UpdateInterval)
        {
            UpdateAgentBehavior(Agent, DeltaTime);
            UpdateAgentMovement(Agent, DeltaTime);
        }
    }
    
    // Reset update timer
    if (LastUpdateTime >= 1.0f / SpawnConfig.UpdateFrequencyLow)
    {
        LastUpdateTime = 0.0f;
    }
}

void UCrowd_MassEntitySubsystem::UpdateAgentBehavior(FCrowd_AgentData& Agent, float DeltaTime)
{
    // Simple behavior state machine
    switch (Agent.BehaviorState)
    {
        case ECrowd_BehaviorState::Idle:
            // Random chance to start moving
            if (FMath::RandRange(0.0f, 1.0f) < 0.1f * DeltaTime)
            {
                Agent.BehaviorState = ECrowd_BehaviorState::Moving;
            }
            break;
            
        case ECrowd_BehaviorState::Moving:
            // Continue moving or switch to gathering
            if (FMath::RandRange(0.0f, 1.0f) < 0.05f * DeltaTime)
            {
                Agent.BehaviorState = ECrowd_BehaviorState::Gathering;
            }
            break;
            
        case ECrowd_BehaviorState::Gathering:
            // Stay in gathering state for a while
            if (FMath::RandRange(0.0f, 1.0f) < 0.02f * DeltaTime)
            {
                Agent.BehaviorState = ECrowd_BehaviorState::Idle;
            }
            break;
            
        case ECrowd_BehaviorState::Fleeing:
            // Flee for a short time then return to moving
            if (FMath::RandRange(0.0f, 1.0f) < 0.2f * DeltaTime)
            {
                Agent.BehaviorState = ECrowd_BehaviorState::Moving;
            }
            break;
            
        case ECrowd_BehaviorState::Resting:
            // Rest then return to idle
            if (FMath::RandRange(0.0f, 1.0f) < 0.03f * DeltaTime)
            {
                Agent.BehaviorState = ECrowd_BehaviorState::Idle;
            }
            break;
    }
}

void UCrowd_MassEntitySubsystem::UpdateAgentMovement(FCrowd_AgentData& Agent, float DeltaTime)
{
    if (Agent.BehaviorState == ECrowd_BehaviorState::Idle || Agent.BehaviorState == ECrowd_BehaviorState::Resting)
    {
        // Gradually stop movement
        Agent.Velocity *= 0.9f;
        return;
    }
    
    // Calculate steering force
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

FVector UCrowd_MassEntitySubsystem::CalculateSteeringForce(const FCrowd_AgentData& Agent)
{
    FVector SteeringForce = FVector::ZeroVector;
    
    // Seek nearest waypoint
    FVector NearestWaypoint = FindNearestWaypoint(Agent.Location);
    if (!NearestWaypoint.IsZero())
    {
        FVector DesiredVelocity = (NearestWaypoint - Agent.Location).GetSafeNormal() * Agent.Speed;
        SteeringForce += (DesiredVelocity - Agent.Velocity) * 0.5f;
    }
    
    // Separation from other agents (simplified)
    FVector SeparationForce = FVector::ZeroVector;
    int32 NearbyAgents = 0;
    
    for (const FCrowd_AgentData& OtherAgent : ActiveAgents)
    {
        if (OtherAgent.AgentID != Agent.AgentID)
        {
            float Distance = FVector::Dist(Agent.Location, OtherAgent.Location);
            if (Distance < 100.0f && Distance > 0.0f)
            {
                FVector AwayVector = (Agent.Location - OtherAgent.Location) / Distance;
                SeparationForce += AwayVector;
                NearbyAgents++;
            }
        }
    }
    
    if (NearbyAgents > 0)
    {
        SeparationForce /= NearbyAgents;
        SteeringForce += SeparationForce * 200.0f;
    }
    
    // Random wander force
    FVector WanderForce = FVector(
        FMath::RandRange(-1.0f, 1.0f),
        FMath::RandRange(-1.0f, 1.0f),
        0.0f
    ) * 50.0f;
    
    SteeringForce += WanderForce;
    
    return SteeringForce;
}

FVector UCrowd_MassEntitySubsystem::FindNearestWaypoint(const FVector& Location)
{
    if (Waypoints.Num() == 0)
    {
        return FVector::ZeroVector;
    }
    
    float NearestDistance = FLT_MAX;
    FVector NearestWaypointLocation = FVector::ZeroVector;
    
    for (ACrowd_Waypoint* Waypoint : Waypoints)
    {
        if (IsValid(Waypoint))
        {
            float Distance = FVector::Dist(Location, Waypoint->GetActorLocation());
            if (Distance < NearestDistance)
            {
                NearestDistance = Distance;
                NearestWaypointLocation = Waypoint->GetActorLocation();
            }
        }
    }
    
    return NearestWaypointLocation;
}

void UCrowd_MassEntitySubsystem::SetCrowdBehaviorState(ECrowd_BehaviorState NewState)
{
    UE_LOG(LogTemp, Warning, TEXT("Setting crowd behavior state to %d"), (int32)NewState);
    
    // Set all agents to new behavior state
    for (FCrowd_AgentData& Agent : ActiveAgents)
    {
        Agent.BehaviorState = NewState;
    }
}

void UCrowd_MassEntitySubsystem::RegisterSpawnPoint(ACrowd_SpawnPoint* SpawnPoint)
{
    if (IsValid(SpawnPoint) && !SpawnPoints.Contains(SpawnPoint))
    {
        SpawnPoints.Add(SpawnPoint);
        UE_LOG(LogTemp, Warning, TEXT("Registered crowd spawn point: %s"), *SpawnPoint->GetName());
    }
}

void UCrowd_MassEntitySubsystem::RegisterWaypoint(ACrowd_Waypoint* Waypoint)
{
    if (IsValid(Waypoint) && !Waypoints.Contains(Waypoint))
    {
        Waypoints.Add(Waypoint);
        UE_LOG(LogTemp, Warning, TEXT("Registered crowd waypoint: %s"), *Waypoint->GetName());
    }
}

void UCrowd_MassEntitySubsystem::UpdateAgentLOD(const FVector& ViewerLocation)
{
    if (!bEnableLOD)
    {
        return;
    }
    
    for (FCrowd_AgentData& Agent : ActiveAgents)
    {
        Agent.LODDistance = FVector::Dist(Agent.Location, ViewerLocation);
    }
}

void UCrowd_MassEntitySubsystem::CullDistantAgents(const FVector& ViewerLocation)
{
    if (!bEnableCulling)
    {
        return;
    }
    
    for (FCrowd_AgentData& Agent : ActiveAgents)
    {
        float Distance = FVector::Dist(Agent.Location, ViewerLocation);
        Agent.bIsVisible = Distance <= SpawnConfig.CullingDistance;
    }
}

int32 UCrowd_MassEntitySubsystem::GetVisibleAgentCount() const
{
    int32 VisibleCount = 0;
    for (const FCrowd_AgentData& Agent : ActiveAgents)
    {
        if (Agent.bIsVisible)
        {
            VisibleCount++;
        }
    }
    return VisibleCount;
}

bool UCrowd_MassEntitySubsystem::IsAgentVisible(const FCrowd_AgentData& Agent, const FVector& ViewerLocation)
{
    if (!bEnableCulling)
    {
        return true;
    }
    
    float Distance = FVector::Dist(Agent.Location, ViewerLocation);
    return Distance <= SpawnConfig.CullingDistance;
}