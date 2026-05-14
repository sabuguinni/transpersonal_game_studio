#include "Crowd_MassSimulationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassSimulationSubsystem.h"
#include "MassEntityTemplateRegistry.h"
#include "MassArchetypeData.h"
#include "MassEntityManager.h"
#include "Components/SceneComponent.h"

ACrowd_MassSimulationManager::ACrowd_MassSimulationManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default simulation settings
    SimulationSettings.MaxCrowdAgents = 1000;
    SimulationSettings.SpawnRadius = 5000.0f;
    SimulationSettings.AgentSpeed = 150.0f;
    SimulationSettings.SeparationDistance = 100.0f;
    SimulationSettings.bEnableFlocking = true;
    SimulationSettings.bEnableAvoidance = true;

    // Initialize default agent group
    FCrowd_AgentGroup DefaultGroup;
    DefaultGroup.GroupName = "MainCrowd";
    DefaultGroup.AgentCount = 100;
    DefaultGroup.SpawnLocation = FVector(0.0f, 0.0f, 100.0f);
    DefaultGroup.GroupRadius = 1000.0f;
    AgentGroups.Add(DefaultGroup);

    bSimulationActive = false;
    bSimulationPaused = false;
    SimulationTime = 0.0f;

    MassEntitySubsystem = nullptr;
    MassSpawnerSubsystem = nullptr;
    MassSimulationSubsystem = nullptr;
}

void ACrowd_MassSimulationManager::BeginPlay()
{
    Super::BeginPlay();

    InitializeMassSubsystems();
    InitializeCrowdSimulation();

    UE_LOG(LogTemp, Warning, TEXT("Crowd Mass Simulation Manager initialized with %d agent groups"), AgentGroups.Num());
}

void ACrowd_MassSimulationManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    ClearAllAgents();
    bSimulationActive = false;

    Super::EndPlay(EndPlayReason);
}

void ACrowd_MassSimulationManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bSimulationActive && !bSimulationPaused)
    {
        UpdateSimulation(DeltaTime);
        SimulationTime += DeltaTime;
    }
}

void ACrowd_MassSimulationManager::InitializeMassSubsystems()
{
    if (UWorld* World = GetWorld())
    {
        MassEntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
        MassSpawnerSubsystem = World->GetSubsystem<UMassSpawnerSubsystem>();
        MassSimulationSubsystem = World->GetSubsystem<UMassSimulationSubsystem>();

        if (!MassEntitySubsystem)
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to get MassEntitySubsystem"));
        }
        if (!MassSpawnerSubsystem)
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to get MassSpawnerSubsystem"));
        }
        if (!MassSimulationSubsystem)
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to get MassSimulationSubsystem"));
        }
    }
}

void ACrowd_MassSimulationManager::InitializeCrowdSimulation()
{
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot initialize crowd simulation - MassEntitySubsystem not available"));
        return;
    }

    CreateEntityArchetype();

    // Spawn initial agent groups
    for (const FCrowd_AgentGroup& Group : AgentGroups)
    {
        SpawnAgentGroup(Group);
    }

    bSimulationActive = true;
    UE_LOG(LogTemp, Warning, TEXT("Crowd simulation initialized successfully"));
}

void ACrowd_MassSimulationManager::CreateEntityArchetype()
{
    // This would normally create a Mass Entity archetype with required components
    // For now, we'll create a basic setup that can be expanded
    UE_LOG(LogTemp, Log, TEXT("Creating Mass Entity archetype for crowd agents"));
}

void ACrowd_MassSimulationManager::SpawnAgentGroup(const FCrowd_AgentGroup& GroupSettings)
{
    if (!MassEntitySubsystem || !bSimulationActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot spawn agent group - simulation not ready"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Spawning agent group '%s' with %d agents"), *GroupSettings.GroupName, GroupSettings.AgentCount);

    // Spawn entities in a circular pattern around the spawn location
    for (int32 i = 0; i < GroupSettings.AgentCount; ++i)
    {
        float Angle = (2.0f * PI * i) / GroupSettings.AgentCount;
        float RandomRadius = FMath::RandRange(0.0f, GroupSettings.GroupRadius);
        
        FVector SpawnPos = GroupSettings.SpawnLocation + FVector(
            FMath::Cos(Angle) * RandomRadius,
            FMath::Sin(Angle) * RandomRadius,
            0.0f
        );

        // Create entity (simplified for now)
        // In a full implementation, this would use MassEntitySubsystem->CreateEntity()
        // with proper archetype and component setup
    }
}

void ACrowd_MassSimulationManager::UpdateSimulation(float DeltaTime)
{
    if (!bSimulationActive || bSimulationPaused)
    {
        return;
    }

    UpdateAgentBehavior(DeltaTime);
}

void ACrowd_MassSimulationManager::UpdateAgentBehavior(float DeltaTime)
{
    // Update crowd agent behaviors
    // This would typically involve:
    // - Flocking behavior calculations
    // - Obstacle avoidance
    // - Goal seeking
    // - LOD management based on distance to player
    
    // For now, we'll just log periodic updates
    static float LogTimer = 0.0f;
    LogTimer += DeltaTime;
    
    if (LogTimer >= 5.0f)
    {
        UE_LOG(LogTemp, Log, TEXT("Crowd simulation running - Active agents: %d, Simulation time: %.2f"), 
               GetActiveAgentCount(), SimulationTime);
        LogTimer = 0.0f;
    }
}

void ACrowd_MassSimulationManager::SetSimulationSettings(const FCrowd_SimulationSettings& NewSettings)
{
    SimulationSettings = NewSettings;
    UE_LOG(LogTemp, Log, TEXT("Simulation settings updated - Max agents: %d, Speed: %.2f"), 
           SimulationSettings.MaxCrowdAgents, SimulationSettings.AgentSpeed);
}

int32 ACrowd_MassSimulationManager::GetActiveAgentCount() const
{
    return SpawnedEntities.Num();
}

void ACrowd_MassSimulationManager::PauseSimulation()
{
    bSimulationPaused = true;
    UE_LOG(LogTemp, Log, TEXT("Crowd simulation paused"));
}

void ACrowd_MassSimulationManager::ResumeSimulation()
{
    bSimulationPaused = false;
    UE_LOG(LogTemp, Log, TEXT("Crowd simulation resumed"));
}

void ACrowd_MassSimulationManager::ClearAllAgents()
{
    if (MassEntitySubsystem)
    {
        CleanupEntities();
    }
    
    SpawnedEntities.Empty();
    bSimulationActive = false;
    UE_LOG(LogTemp, Log, TEXT("All crowd agents cleared"));
}

void ACrowd_MassSimulationManager::CleanupEntities()
{
    // Clean up spawned Mass entities
    for (const FMassEntityHandle& EntityHandle : SpawnedEntities)
    {
        if (MassEntitySubsystem && EntityHandle.IsValid())
        {
            // MassEntitySubsystem->DestroyEntity(EntityHandle);
        }
    }
}