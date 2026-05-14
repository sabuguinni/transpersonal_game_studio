#include "Crowd_MassSimulationManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/GameModeBase.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassAgentComponent.h"
#include "NavigationSystem.h"
#include "AI/NavigationSystemBase.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

ACrowd_MassSimulationManager::ACrowd_MassSimulationManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10Hz update rate
    
    // Initialize simulation parameters
    MaxSimulationAgents = 50000;
    CurrentActiveAgents = 0;
    bSimulationActive = false;
    bUseHierarchicalLOD = true;
    bEnableFlowFields = true;
    bEnableCollisionAvoidance = true;
    
    // LOD distances
    HighDetailDistance = 1000.0f;
    MediumDetailDistance = 5000.0f;
    LowDetailDistance = 15000.0f;
    
    // Performance thresholds
    TargetFrameRate = 60.0f;
    MaxSpawnPerFrame = 100;
    MaxProcessingTimeMs = 8.0f;
    
    // Crowd behavior parameters
    DefaultMovementSpeed = 300.0f;
    FlockingRadius = 500.0f;
    SeparationDistance = 150.0f;
    CohesionStrength = 0.5f;
    AlignmentStrength = 0.3f;
    SeparationStrength = 0.8f;
    
    // Initialize mesh component for visualization
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    
    // Set default mesh for crowd agents
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        DefaultAgentMesh = CubeMeshAsset.Object;
    }
}

void ACrowd_MassSimulationManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize Mass Entity subsystem
    if (UWorld* World = GetWorld())
    {
        MassEntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
        MassSpawnerSubsystem = World->GetSubsystem<UMassSpawnerSubsystem>();
        
        if (MassEntitySubsystem && MassSpawnerSubsystem)
        {
            UE_LOG(LogTemp, Warning, TEXT("Mass Entity subsystems initialized successfully"));
            InitializeMassEntityArchetypes();
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to initialize Mass Entity subsystems"));
        }
    }
    
    // Start simulation if auto-start is enabled
    if (bAutoStartSimulation)
    {
        StartCrowdSimulation();
    }
}

void ACrowd_MassSimulationManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (!bSimulationActive || !MassEntitySubsystem)
    {
        return;
    }
    
    // Performance monitoring
    float CurrentFrameRate = 1.0f / DeltaTime;
    if (CurrentFrameRate < TargetFrameRate * 0.8f) // 20% tolerance
    {
        AdaptPerformance(DeltaTime);
    }
    
    // Update crowd behaviors
    UpdateFlockingBehavior(DeltaTime);
    UpdateLODSystem(DeltaTime);
    UpdateFlowFields(DeltaTime);
    
    // Process spawn/despawn queue
    ProcessAgentQueue(DeltaTime);
}

void ACrowd_MassSimulationManager::StartCrowdSimulation()
{
    if (bSimulationActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Crowd simulation already active"));
        return;
    }
    
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot start simulation - Mass Entity subsystem not available"));
        return;
    }
    
    bSimulationActive = true;
    CurrentActiveAgents = 0;
    
    // Spawn initial crowd agents
    SpawnInitialCrowdAgents();
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd simulation started with %d agents"), CurrentActiveAgents);
}

void ACrowd_MassSimulationManager::StopCrowdSimulation()
{
    if (!bSimulationActive)
    {
        return;
    }
    
    bSimulationActive = false;
    
    // Despawn all agents
    DespawnAllAgents();
    
    CurrentActiveAgents = 0;
    UE_LOG(LogTemp, Warning, TEXT("Crowd simulation stopped"));
}

void ACrowd_MassSimulationManager::SpawnCrowdAgents(const FVector& Location, int32 Count, ECrowd_AgentType AgentType)
{
    if (!bSimulationActive || !MassEntitySubsystem)
    {
        return;
    }
    
    if (CurrentActiveAgents + Count > MaxSimulationAgents)
    {
        Count = FMath::Max(0, MaxSimulationAgents - CurrentActiveAgents);
        UE_LOG(LogTemp, Warning, TEXT("Capping spawn count to %d to stay within limit"), Count);
    }
    
    // Queue spawn requests for processing
    for (int32 i = 0; i < Count; i++)
    {
        FCrowd_SpawnRequest SpawnRequest;
        SpawnRequest.Location = Location + FVector(FMath::RandRange(-500.0f, 500.0f), FMath::RandRange(-500.0f, 500.0f), 0.0f);
        SpawnRequest.AgentType = AgentType;
        SpawnRequest.bPendingSpawn = true;
        
        SpawnQueue.Add(SpawnRequest);
    }
}

void ACrowd_MassSimulationManager::DespawnCrowdAgents(const FVector& Location, float Radius)
{
    if (!bSimulationActive || !MassEntitySubsystem)
    {
        return;
    }
    
    // Mark agents for despawn within radius
    for (auto& Agent : ActiveAgents)
    {
        float Distance = FVector::Dist(Agent.Location, Location);
        if (Distance <= Radius)
        {
            Agent.bPendingDespawn = true;
        }
    }
}

void ACrowd_MassSimulationManager::SetCrowdDestination(const FVector& Destination, float Radius)
{
    GlobalDestination = Destination;
    DestinationRadius = Radius;
    bHasGlobalDestination = true;
    
    // Update flow fields for new destination
    if (bEnableFlowFields)
    {
        GenerateFlowField(Destination, Radius);
    }
}

void ACrowd_MassSimulationManager::InitializeMassEntityArchetypes()
{
    if (!MassEntitySubsystem)
    {
        return;
    }
    
    // Create archetype for crowd agents
    // This would typically involve setting up Mass Entity fragments
    // For now, we'll use a simplified approach
    
    UE_LOG(LogTemp, Warning, TEXT("Mass Entity archetypes initialized"));
}

void ACrowd_MassSimulationManager::SpawnInitialCrowdAgents()
{
    // Spawn agents in clusters around the map
    TArray<FVector> SpawnLocations = {
        FVector(2000.0f, 2000.0f, 100.0f),
        FVector(-2000.0f, 2000.0f, 100.0f),
        FVector(2000.0f, -2000.0f, 100.0f),
        FVector(-2000.0f, -2000.0f, 100.0f),
        FVector(0.0f, 0.0f, 100.0f)
    };
    
    int32 AgentsPerCluster = FMath::Min(1000, MaxSimulationAgents / SpawnLocations.Num());
    
    for (const FVector& Location : SpawnLocations)
    {
        SpawnCrowdAgents(Location, AgentsPerCluster, ECrowd_AgentType::Civilian);
    }
}

void ACrowd_MassSimulationManager::ProcessAgentQueue(float DeltaTime)
{
    int32 ProcessedThisFrame = 0;
    
    // Process spawn queue
    for (int32 i = SpawnQueue.Num() - 1; i >= 0 && ProcessedThisFrame < MaxSpawnPerFrame; i--)
    {
        if (SpawnQueue[i].bPendingSpawn)
        {
            // Create new agent
            FCrowd_AgentData NewAgent;
            NewAgent.Location = SpawnQueue[i].Location;
            NewAgent.Velocity = FVector::ZeroVector;
            NewAgent.AgentType = SpawnQueue[i].AgentType;
            NewAgent.LODLevel = CalculateLODLevel(NewAgent.Location);
            NewAgent.bPendingDespawn = false;
            
            ActiveAgents.Add(NewAgent);
            CurrentActiveAgents++;
            
            SpawnQueue.RemoveAt(i);
            ProcessedThisFrame++;
        }
    }
    
    // Process despawn queue
    for (int32 i = ActiveAgents.Num() - 1; i >= 0 && ProcessedThisFrame < MaxSpawnPerFrame; i--)
    {
        if (ActiveAgents[i].bPendingDespawn)
        {
            ActiveAgents.RemoveAt(i);
            CurrentActiveAgents--;
            ProcessedThisFrame++;
        }
    }
}

void ACrowd_MassSimulationManager::UpdateFlockingBehavior(float DeltaTime)
{
    if (!bEnableFlockingBehavior || ActiveAgents.Num() == 0)
    {
        return;
    }
    
    // Simple flocking implementation
    for (auto& Agent : ActiveAgents)
    {
        if (Agent.LODLevel == ECrowd_LODLevel::Disabled)
        {
            continue;
        }
        
        FVector Separation = CalculateSeparation(Agent);
        FVector Cohesion = CalculateCohesion(Agent);
        FVector Alignment = CalculateAlignment(Agent);
        
        // Combine forces
        FVector TotalForce = (Separation * SeparationStrength) + 
                           (Cohesion * CohesionStrength) + 
                           (Alignment * AlignmentStrength);
        
        // Apply movement
        Agent.Velocity += TotalForce * DeltaTime;
        Agent.Velocity = Agent.Velocity.GetClampedToMaxSize(DefaultMovementSpeed);
        Agent.Location += Agent.Velocity * DeltaTime;
    }
}

void ACrowd_MassSimulationManager::UpdateLODSystem(float DeltaTime)
{
    if (!bUseHierarchicalLOD)
    {
        return;
    }
    
    // Get player location for LOD calculations
    FVector PlayerLocation = FVector::ZeroVector;
    if (UWorld* World = GetWorld())
    {
        if (APawn* PlayerPawn = World->GetFirstPlayerController()->GetPawn())
        {
            PlayerLocation = PlayerPawn->GetActorLocation();
        }
    }
    
    // Update LOD levels for all agents
    for (auto& Agent : ActiveAgents)
    {
        Agent.LODLevel = CalculateLODLevel(Agent.Location, PlayerLocation);
    }
}

void ACrowd_MassSimulationManager::UpdateFlowFields(float DeltaTime)
{
    if (!bEnableFlowFields || !bHasGlobalDestination)
    {
        return;
    }
    
    // Update flow field influence on agents
    for (auto& Agent : ActiveAgents)
    {
        if (Agent.LODLevel == ECrowd_LODLevel::Disabled)
        {
            continue;
        }
        
        FVector FlowDirection = GetFlowFieldDirection(Agent.Location);
        Agent.Velocity += FlowDirection * FlowFieldStrength * DeltaTime;
    }
}

void ACrowd_MassSimulationManager::AdaptPerformance(float DeltaTime)
{
    // Reduce LOD quality to maintain performance
    if (bUseHierarchicalLOD)
    {
        HighDetailDistance *= 0.95f;
        MediumDetailDistance *= 0.95f;
        
        // Ensure minimum distances
        HighDetailDistance = FMath::Max(500.0f, HighDetailDistance);
        MediumDetailDistance = FMath::Max(2000.0f, MediumDetailDistance);
    }
    
    // Reduce spawn rate
    MaxSpawnPerFrame = FMath::Max(10, MaxSpawnPerFrame - 5);
    
    UE_LOG(LogTemp, Warning, TEXT("Performance adaptation: LOD distances reduced, spawn rate: %d"), MaxSpawnPerFrame);
}

void ACrowd_MassSimulationManager::DespawnAllAgents()
{
    ActiveAgents.Empty();
    SpawnQueue.Empty();
    CurrentActiveAgents = 0;
}

ECrowd_LODLevel ACrowd_MassSimulationManager::CalculateLODLevel(const FVector& AgentLocation, const FVector& PlayerLocation) const
{
    float Distance = FVector::Dist(AgentLocation, PlayerLocation);
    
    if (Distance <= HighDetailDistance)
    {
        return ECrowd_LODLevel::High;
    }
    else if (Distance <= MediumDetailDistance)
    {
        return ECrowd_LODLevel::Medium;
    }
    else if (Distance <= LowDetailDistance)
    {
        return ECrowd_LODLevel::Low;
    }
    else
    {
        return ECrowd_LODLevel::Disabled;
    }
}

FVector ACrowd_MassSimulationManager::CalculateSeparation(const FCrowd_AgentData& Agent) const
{
    FVector SeparationForce = FVector::ZeroVector;
    int32 NeighborCount = 0;
    
    for (const auto& OtherAgent : ActiveAgents)
    {
        if (&OtherAgent == &Agent)
        {
            continue;
        }
        
        float Distance = FVector::Dist(Agent.Location, OtherAgent.Location);
        if (Distance < SeparationDistance && Distance > 0.0f)
        {
            FVector AwayVector = (Agent.Location - OtherAgent.Location).GetSafeNormal();
            SeparationForce += AwayVector / Distance; // Closer agents have more influence
            NeighborCount++;
        }
    }
    
    if (NeighborCount > 0)
    {
        SeparationForce /= NeighborCount;
        SeparationForce = SeparationForce.GetSafeNormal() * DefaultMovementSpeed;
    }
    
    return SeparationForce;
}

FVector ACrowd_MassSimulationManager::CalculateCohesion(const FCrowd_AgentData& Agent) const
{
    FVector CenterOfMass = FVector::ZeroVector;
    int32 NeighborCount = 0;
    
    for (const auto& OtherAgent : ActiveAgents)
    {
        if (&OtherAgent == &Agent)
        {
            continue;
        }
        
        float Distance = FVector::Dist(Agent.Location, OtherAgent.Location);
        if (Distance < FlockingRadius)
        {
            CenterOfMass += OtherAgent.Location;
            NeighborCount++;
        }
    }
    
    if (NeighborCount > 0)
    {
        CenterOfMass /= NeighborCount;
        return (CenterOfMass - Agent.Location).GetSafeNormal() * DefaultMovementSpeed;
    }
    
    return FVector::ZeroVector;
}

FVector ACrowd_MassSimulationManager::CalculateAlignment(const FCrowd_AgentData& Agent) const
{
    FVector AverageVelocity = FVector::ZeroVector;
    int32 NeighborCount = 0;
    
    for (const auto& OtherAgent : ActiveAgents)
    {
        if (&OtherAgent == &Agent)
        {
            continue;
        }
        
        float Distance = FVector::Dist(Agent.Location, OtherAgent.Location);
        if (Distance < FlockingRadius)
        {
            AverageVelocity += OtherAgent.Velocity;
            NeighborCount++;
        }
    }
    
    if (NeighborCount > 0)
    {
        AverageVelocity /= NeighborCount;
        return AverageVelocity.GetSafeNormal() * DefaultMovementSpeed;
    }
    
    return FVector::ZeroVector;
}

void ACrowd_MassSimulationManager::GenerateFlowField(const FVector& Destination, float Radius)
{
    // Simplified flow field generation
    // In a full implementation, this would use navigation mesh and pathfinding
    FlowFieldDestination = Destination;
    FlowFieldRadius = Radius;
    
    UE_LOG(LogTemp, Warning, TEXT("Flow field generated for destination: %s"), *Destination.ToString());
}

FVector ACrowd_MassSimulationManager::GetFlowFieldDirection(const FVector& Location) const
{
    if (!bHasGlobalDestination)
    {
        return FVector::ZeroVector;
    }
    
    // Simple direction towards destination
    FVector Direction = (FlowFieldDestination - Location).GetSafeNormal();
    float Distance = FVector::Dist(Location, FlowFieldDestination);
    
    // Reduce influence as we get closer to destination
    float Influence = FMath::Clamp(Distance / FlowFieldRadius, 0.1f, 1.0f);
    
    return Direction * Influence;
}