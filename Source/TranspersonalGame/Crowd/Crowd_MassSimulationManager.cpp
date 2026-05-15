#include "Crowd_MassSimulationManager.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassSimulationSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ACrowd_MassSimulationManager::ACrowd_MassSimulationManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    VisualizationMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualizationMesh"));
    RootComponent = VisualizationMesh;
    
    // Initialize default values
    bSimulationActive = false;
    LastLODUpdateTime = 0.0f;
    CurrentDestination = FVector::ZeroVector;
    
    // Set default simulation settings
    SimulationSettings.MaxCrowdAgents = 1000;
    SimulationSettings.SpawnRadius = 5000.0f;
    SimulationSettings.AgentSpeed = 150.0f;
    SimulationSettings.AvoidanceRadius = 100.0f;
    
    // Create default agent group
    FCrowd_AgentGroup DefaultGroup;
    DefaultGroup.GroupName = "DefaultCrowd";
    DefaultGroup.AgentCount = 50;
    DefaultGroup.SpawnLocation = FVector(0, 0, 100);
    DefaultGroup.GroupRadius = 1000.0f;
    AgentGroups.Add(DefaultGroup);
}

void ACrowd_MassSimulationManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Get Mass Entity Subsystem
    MassEntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
    
    if (MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("Crowd_MassSimulationManager: Mass Entity Subsystem found"));
        InitializeCrowdSimulation();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Crowd_MassSimulationManager: Mass Entity Subsystem not found"));
    }
}

void ACrowd_MassSimulationManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bSimulationActive && MassEntitySubsystem)
    {
        UpdateAgentMovement(DeltaTime);
        
        // Update LOD every 0.5 seconds
        LastLODUpdateTime += DeltaTime;
        if (LastLODUpdateTime >= 0.5f)
        {
            UpdateCrowdLOD();
            LastLODUpdateTime = 0.0f;
        }
    }
}

void ACrowd_MassSimulationManager::InitializeCrowdSimulation()
{
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot initialize crowd simulation - Mass Entity Subsystem is null"));
        return;
    }
    
    // Clear existing entities
    ClearAllCrowdAgents();
    
    // Spawn initial crowd groups
    for (const FCrowd_AgentGroup& Group : AgentGroups)
    {
        if (Group.bIsActive)
        {
            SpawnCrowdAgents(Group);
        }
    }
    
    bSimulationActive = true;
    UE_LOG(LogTemp, Warning, TEXT("Crowd simulation initialized with %d active entities"), SpawnedEntities.Num());
}

void ACrowd_MassSimulationManager::SpawnCrowdAgents(const FCrowd_AgentGroup& AgentGroup)
{
    if (!MassEntitySubsystem)
    {
        return;
    }
    
    // Create entities for this group
    for (int32 i = 0; i < AgentGroup.AgentCount; ++i)
    {
        // Calculate spawn position within group radius
        float Angle = FMath::RandRange(0.0f, 2.0f * PI);
        float Distance = FMath::RandRange(0.0f, AgentGroup.GroupRadius);
        
        FVector SpawnOffset = FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.0f
        );
        
        FVector SpawnLocation = AgentGroup.SpawnLocation + SpawnOffset;
        
        // Create Mass Entity
        FMassEntityHandle NewEntity = MassEntitySubsystem->CreateEntity();
        if (NewEntity.IsValid())
        {
            SpawnedEntities.Add(NewEntity);
            
            // Debug visualization
            DrawDebugSphere(GetWorld(), SpawnLocation, 50.0f, 8, FColor::Blue, false, 5.0f);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Spawned %d crowd agents for group: %s"), AgentGroup.AgentCount, *AgentGroup.GroupName);
}

void ACrowd_MassSimulationManager::UpdateCrowdLOD()
{
    if (!GetWorld() || !GetWorld()->GetFirstPlayerController())
    {
        return;
    }
    
    APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Process LOD for each spawned entity
    for (const FMassEntityHandle& Entity : SpawnedEntities)
    {
        if (Entity.IsValid())
        {
            // Calculate distance to player (simplified - would need actual entity position)
            float Distance = FVector::Dist(GetActorLocation(), PlayerLocation);
            
            // Apply LOD based on distance
            if (Distance < SimulationSettings.LODDistance1)
            {
                // High LOD - full simulation
            }
            else if (Distance < SimulationSettings.LODDistance2)
            {
                // Medium LOD - reduced update rate
            }
            else if (Distance < SimulationSettings.LODDistance3)
            {
                // Low LOD - minimal simulation
            }
            else
            {
                // Ultra low LOD - static or culled
            }
        }
    }
}

void ACrowd_MassSimulationManager::SetCrowdDestination(FVector Destination)
{
    CurrentDestination = Destination;
    UE_LOG(LogTemp, Warning, TEXT("Crowd destination set to: %s"), *Destination.ToString());
}

int32 ACrowd_MassSimulationManager::GetActiveCrowdCount() const
{
    return SpawnedEntities.Num();
}

void ACrowd_MassSimulationManager::PauseCrowdSimulation()
{
    bSimulationActive = false;
    UE_LOG(LogTemp, Warning, TEXT("Crowd simulation paused"));
}

void ACrowd_MassSimulationManager::ResumeCrowdSimulation()
{
    bSimulationActive = true;
    UE_LOG(LogTemp, Warning, TEXT("Crowd simulation resumed"));
}

void ACrowd_MassSimulationManager::ClearAllCrowdAgents()
{
    if (MassEntitySubsystem)
    {
        for (const FMassEntityHandle& Entity : SpawnedEntities)
        {
            if (Entity.IsValid())
            {
                MassEntitySubsystem->DestroyEntity(Entity);
            }
        }
    }
    
    SpawnedEntities.Empty();
    UE_LOG(LogTemp, Warning, TEXT("All crowd agents cleared"));
}

void ACrowd_MassSimulationManager::UpdateAgentMovement(float DeltaTime)
{
    // Update movement for all active entities
    for (const FMassEntityHandle& Entity : SpawnedEntities)
    {
        if (Entity.IsValid())
        {
            // Calculate flocking behavior
            FVector FlockingForce = CalculateFlockingBehavior(Entity);
            FVector AvoidanceForce = CalculateAvoidance(Entity);
            
            // Combine forces and apply movement
            FVector TotalForce = FlockingForce + AvoidanceForce;
            TotalForce = TotalForce.GetClampedToMaxSize(SimulationSettings.AgentSpeed);
            
            // Apply movement (simplified - would need actual entity transform component)
        }
    }
}

void ACrowd_MassSimulationManager::ProcessAgentLOD()
{
    // Process LOD changes for agents based on distance
    UpdateCrowdLOD();
}

void ACrowd_MassSimulationManager::HandleAgentCollisions()
{
    // Handle collision detection and response between agents
    for (int32 i = 0; i < SpawnedEntities.Num(); ++i)
    {
        for (int32 j = i + 1; j < SpawnedEntities.Num(); ++j)
        {
            const FMassEntityHandle& EntityA = SpawnedEntities[i];
            const FMassEntityHandle& EntityB = SpawnedEntities[j];
            
            if (EntityA.IsValid() && EntityB.IsValid())
            {
                // Check collision and apply separation force
                // (Implementation would require actual entity positions)
            }
        }
    }
}

FVector ACrowd_MassSimulationManager::CalculateFlockingBehavior(const FMassEntityHandle& Entity)
{
    if (!Entity.IsValid())
    {
        return FVector::ZeroVector;
    }
    
    // Calculate separation, alignment, and cohesion
    FVector Separation = CalculateAvoidance(Entity);
    FVector Alignment = CalculateAlignment(Entity);
    FVector Cohesion = CalculateCohesion(Entity);
    
    // Weight and combine forces
    FVector FlockingForce = (Separation * 2.0f) + (Alignment * 1.0f) + (Cohesion * 1.0f);
    
    return FlockingForce.GetSafeNormal() * SimulationSettings.AgentSpeed;
}

FVector ACrowd_MassSimulationManager::CalculateAvoidance(const FMassEntityHandle& Entity)
{
    if (!Entity.IsValid())
    {
        return FVector::ZeroVector;
    }
    
    FVector AvoidanceForce = FVector::ZeroVector;
    
    // Calculate avoidance force from nearby agents
    for (const FMassEntityHandle& OtherEntity : SpawnedEntities)
    {
        if (OtherEntity.IsValid() && OtherEntity != Entity)
        {
            // Calculate separation force (simplified)
            // Would need actual entity positions for real implementation
            FVector RandomSeparation = FVector(
                FMath::RandRange(-1.0f, 1.0f),
                FMath::RandRange(-1.0f, 1.0f),
                0.0f
            );
            AvoidanceForce += RandomSeparation;
        }
    }
    
    return AvoidanceForce.GetSafeNormal();
}

FVector ACrowd_MassSimulationManager::CalculateCohesion(const FMassEntityHandle& Entity)
{
    if (!Entity.IsValid())
    {
        return FVector::ZeroVector;
    }
    
    FVector CenterOfMass = FVector::ZeroVector;
    int32 NeighborCount = 0;
    
    // Calculate center of mass of nearby agents
    for (const FMassEntityHandle& OtherEntity : SpawnedEntities)
    {
        if (OtherEntity.IsValid() && OtherEntity != Entity)
        {
            // Would need actual entity positions
            CenterOfMass += GetActorLocation(); // Placeholder
            NeighborCount++;
        }
    }
    
    if (NeighborCount > 0)
    {
        CenterOfMass /= NeighborCount;
        return (CenterOfMass - GetActorLocation()).GetSafeNormal();
    }
    
    return FVector::ZeroVector;
}

FVector ACrowd_MassSimulationManager::CalculateAlignment(const FMassEntityHandle& Entity)
{
    if (!Entity.IsValid())
    {
        return FVector::ZeroVector;
    }
    
    FVector AverageVelocity = FVector::ZeroVector;
    int32 NeighborCount = 0;
    
    // Calculate average velocity of nearby agents
    for (const FMassEntityHandle& OtherEntity : SpawnedEntities)
    {
        if (OtherEntity.IsValid() && OtherEntity != Entity)
        {
            // Would need actual entity velocity
            AverageVelocity += FVector(1, 0, 0); // Placeholder
            NeighborCount++;
        }
    }
    
    if (NeighborCount > 0)
    {
        AverageVelocity /= NeighborCount;
        return AverageVelocity.GetSafeNormal();
    }
    
    return FVector::ZeroVector;
}