#include "MassEntityCrowdManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassSimulationSubsystem.h"
#include "MassEntityTemplateRegistry.h"
#include "MassArchetypeTypes.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassRepresentationFragments.h"

DEFINE_LOG_CATEGORY(LogCrowdSimulation);

AMassEntityCrowdManager::AMassEntityCrowdManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PrePhysics;
    
    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    
    // Initialize default values
    MaxCrowdAgents = 10000;
    SpawnRadius = 5000.0f;
    AgentSpeed = 150.0f;
    AgentRadius = 50.0f;
    
    HighLODDistance = 500.0f;
    MediumLODDistance = 2000.0f;
    LowLODDistance = 5000.0f;
    
    // Initialize spawn and destination points
    SpawnPoints.Add(FVector::ZeroVector);
    DestinationPoints.Add(FVector(1000.0f, 0.0f, 0.0f));
}

void AMassEntityCrowdManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogCrowdSimulation, Log, TEXT("Mass Entity Crowd Manager starting up"));
    
    InitializeMassEntityFramework();
    CreateCrowdArchetype();
    
    // Auto-spawn initial crowd if configured
    if (MaxCrowdAgents > 0)
    {
        SpawnCrowdAgents(FMath::Min(1000, MaxCrowdAgents)); // Start with 1000 agents
    }
}

void AMassEntityCrowdManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    DespawnAllAgents();
    Super::EndPlay(EndPlayReason);
}

void AMassEntityCrowdManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (!MassEntitySubsystem || !MassSimulationSubsystem)
    {
        return;
    }
    
    UpdateCrowdLOD(DeltaTime);
    UpdateCrowdBehavior(DeltaTime);
    
    // Performance monitoring
    LastPerformanceCheck += DeltaTime;
    if (LastPerformanceCheck >= PerformanceCheckInterval)
    {
        LastPerformanceCheck = 0.0f;
        
        // Log crowd statistics
        UE_LOG(LogCrowdSimulation, VeryVerbose, TEXT("Active Crowd Agents: %d (High LOD: %d, Medium LOD: %d, Low LOD: %d)"), 
            GetActiveCrowdCount(), HighLODAgentCount, MediumLODAgentCount, LowLODAgentCount);
    }
}

void AMassEntityCrowdManager::InitializeMassEntityFramework()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogCrowdSimulation, Error, TEXT("Failed to get world for Mass Entity initialization"));
        return;
    }
    
    // Get Mass Entity subsystems
    MassEntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
    MassSpawnerSubsystem = World->GetSubsystem<UMassSpawnerSubsystem>();
    MassSimulationSubsystem = World->GetSubsystem<UMassSimulationSubsystem>();
    
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogCrowdSimulation, Error, TEXT("Failed to get MassEntitySubsystem"));
        return;
    }
    
    if (!MassSpawnerSubsystem)
    {
        UE_LOG(LogCrowdSimulation, Error, TEXT("Failed to get MassSpawnerSubsystem"));
        return;
    }
    
    if (!MassSimulationSubsystem)
    {
        UE_LOG(LogCrowdSimulation, Error, TEXT("Failed to get MassSimulationSubsystem"));
        return;
    }
    
    UE_LOG(LogCrowdSimulation, Log, TEXT("Mass Entity Framework initialized successfully"));
}

void AMassEntityCrowdManager::CreateCrowdArchetype()
{
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogCrowdSimulation, Error, TEXT("Cannot create archetype - MassEntitySubsystem is null"));
        return;
    }
    
    // Create archetype composition for crowd agents
    FMassArchetypeCompositionDescriptor Composition;
    
    // Add essential fragments for crowd simulation
    Composition.Fragments.Add(FMassTransformFragment::StaticStruct());
    Composition.Fragments.Add(FMassVelocityFragment::StaticStruct());
    Composition.Fragments.Add(FMassRepresentationFragment::StaticStruct());
    Composition.Fragments.Add(FMassRepresentationLODFragment::StaticStruct());
    
    // Create the archetype
    CrowdArchetype = MassEntitySubsystem->CreateArchetype(Composition);
    
    if (CrowdArchetype.IsValid())
    {
        UE_LOG(LogCrowdSimulation, Log, TEXT("Crowd archetype created successfully"));
    }
    else
    {
        UE_LOG(LogCrowdSimulation, Error, TEXT("Failed to create crowd archetype"));
    }
}

void AMassEntityCrowdManager::SpawnCrowdAgents(int32 NumAgents)
{
    if (!MassEntitySubsystem || !CrowdArchetype.IsValid())
    {
        UE_LOG(LogCrowdSimulation, Error, TEXT("Cannot spawn agents - Mass Entity system not ready"));
        return;
    }
    
    // Clamp to maximum allowed agents
    NumAgents = FMath::Min(NumAgents, MaxCrowdAgents - GetActiveCrowdCount());
    
    if (NumAgents <= 0)
    {
        UE_LOG(LogCrowdSimulation, Warning, TEXT("Cannot spawn agents - at maximum capacity or invalid count"));
        return;
    }
    
    UE_LOG(LogCrowdSimulation, Log, TEXT("Spawning %d crowd agents"), NumAgents);
    
    // Batch create entities
    TArray<FMassEntityHandle> NewEntities;
    MassEntitySubsystem->BatchCreateEntities(CrowdArchetype, NumAgents, NewEntities);
    
    // Initialize each entity
    for (int32 i = 0; i < NewEntities.Num(); i++)
    {
        FMassEntityHandle Entity = NewEntities[i];
        if (!Entity.IsValid())
        {
            continue;
        }
        
        // Set initial transform
        FVector SpawnLocation = GetRandomSpawnLocation();
        FMassTransformFragment& TransformFragment = MassEntitySubsystem->GetFragmentDataChecked<FMassTransformFragment>(Entity);
        TransformFragment.SetTransform(FTransform(SpawnLocation));
        
        // Set initial velocity
        FMassVelocityFragment& VelocityFragment = MassEntitySubsystem->GetFragmentDataChecked<FMassVelocityFragment>(Entity);
        VelocityFragment.Value = FVector::ZeroVector;
        
        // Set representation
        FMassRepresentationFragment& RepresentationFragment = MassEntitySubsystem->GetFragmentDataChecked<FMassRepresentationFragment>(Entity);
        RepresentationFragment.CurrentRepresentation = EMassRepresentationType::StaticMeshInstance;
        
        // Set LOD
        FMassRepresentationLODFragment& LODFragment = MassEntitySubsystem->GetFragmentDataChecked<FMassRepresentationLODFragment>(Entity);
        LODFragment.LOD = EMassLOD::High;
        
        CrowdEntities.Add(Entity);
    }
    
    UE_LOG(LogCrowdSimulation, Log, TEXT("Successfully spawned %d crowd agents. Total active: %d"), 
        NewEntities.Num(), GetActiveCrowdCount());
}

void AMassEntityCrowdManager::DespawnAllAgents()
{
    if (!MassEntitySubsystem)
    {
        return;
    }
    
    UE_LOG(LogCrowdSimulation, Log, TEXT("Despawning all crowd agents"));
    
    // Destroy all crowd entities
    for (const FMassEntityHandle& Entity : CrowdEntities)
    {
        if (Entity.IsValid())
        {
            MassEntitySubsystem->DestroyEntity(Entity);
        }
    }
    
    CrowdEntities.Empty();
    
    // Reset LOD counters
    HighLODAgentCount = 0;
    MediumLODAgentCount = 0;
    LowLODAgentCount = 0;
}

void AMassEntityCrowdManager::SetCrowdDestination(const FVector& NewDestination)
{
    // Clear existing destinations and add new one
    DestinationPoints.Empty();
    DestinationPoints.Add(NewDestination);
    
    UE_LOG(LogCrowdSimulation, Log, TEXT("Set crowd destination to: %s"), *NewDestination.ToString());
}

void AMassEntityCrowdManager::TriggerCrowdPanic(const FVector& PanicCenter, float PanicRadius)
{
    if (!MassEntitySubsystem)
    {
        return;
    }
    
    UE_LOG(LogCrowdSimulation, Log, TEXT("Triggering crowd panic at %s with radius %f"), 
        *PanicCenter.ToString(), PanicRadius);
    
    // Apply panic behavior to agents within radius
    for (const FMassEntityHandle& Entity : CrowdEntities)
    {
        if (!Entity.IsValid())
        {
            continue;
        }
        
        FMassTransformFragment& TransformFragment = MassEntitySubsystem->GetFragmentDataChecked<FMassTransformFragment>(Entity);
        FVector AgentLocation = TransformFragment.GetTransform().GetLocation();
        
        float DistanceToCenter = FVector::Dist(AgentLocation, PanicCenter);
        if (DistanceToCenter <= PanicRadius)
        {
            // Calculate flee direction (away from panic center)
            FVector FleeDirection = (AgentLocation - PanicCenter).GetSafeNormal();
            
            // Apply panic velocity
            FMassVelocityFragment& VelocityFragment = MassEntitySubsystem->GetFragmentDataChecked<FMassVelocityFragment>(Entity);
            VelocityFragment.Value = FleeDirection * AgentSpeed * 2.0f; // Panic speed multiplier
        }
    }
}

void AMassEntityCrowdManager::AddSpawnPoint(const FVector& SpawnLocation)
{
    SpawnPoints.AddUnique(SpawnLocation);
    UE_LOG(LogCrowdSimulation, Log, TEXT("Added spawn point: %s"), *SpawnLocation.ToString());
}

void AMassEntityCrowdManager::AddDestinationPoint(const FVector& DestinationLocation)
{
    DestinationPoints.AddUnique(DestinationLocation);
    UE_LOG(LogCrowdSimulation, Log, TEXT("Added destination point: %s"), *DestinationLocation.ToString());
}

int32 AMassEntityCrowdManager::GetActiveCrowdCount() const
{
    return CrowdEntities.Num();
}

float AMassEntityCrowdManager::GetCrowdDensity(const FVector& Location, float Radius) const
{
    if (!MassEntitySubsystem)
    {
        return 0.0f;
    }
    
    int32 AgentsInRadius = 0;
    
    for (const FMassEntityHandle& Entity : CrowdEntities)
    {
        if (!Entity.IsValid())
        {
            continue;
        }
        
        const FMassTransformFragment& TransformFragment = MassEntitySubsystem->GetFragmentDataChecked<FMassTransformFragment>(Entity);
        FVector AgentLocation = TransformFragment.GetTransform().GetLocation();
        
        if (FVector::Dist(AgentLocation, Location) <= Radius)
        {
            AgentsInRadius++;
        }
    }
    
    // Calculate density as agents per square meter
    float Area = PI * Radius * Radius;
    return Area > 0.0f ? AgentsInRadius / Area : 0.0f;
}

FVector AMassEntityCrowdManager::GetRandomSpawnLocation() const
{
    if (SpawnPoints.Num() == 0)
    {
        return GetActorLocation();
    }
    
    // Pick random spawn point
    const FVector& BaseSpawn = SpawnPoints[FMath::RandRange(0, SpawnPoints.Num() - 1)];
    
    // Add random offset within spawn radius
    FVector RandomOffset = FVector(
        FMath::RandRange(-SpawnRadius, SpawnRadius),
        FMath::RandRange(-SpawnRadius, SpawnRadius),
        0.0f
    );
    
    return BaseSpawn + RandomOffset;
}

void AMassEntityCrowdManager::UpdateCrowdLOD(float DeltaTime)
{
    if (!MassEntitySubsystem)
    {
        return;
    }
    
    // Get player location for LOD calculations
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Reset LOD counters
    HighLODAgentCount = 0;
    MediumLODAgentCount = 0;
    LowLODAgentCount = 0;
    
    // Update LOD for each agent based on distance to player
    for (const FMassEntityHandle& Entity : CrowdEntities)
    {
        if (!Entity.IsValid())
        {
            continue;
        }
        
        const FMassTransformFragment& TransformFragment = MassEntitySubsystem->GetFragmentDataChecked<FMassTransformFragment>(Entity);
        FVector AgentLocation = TransformFragment.GetTransform().GetLocation();
        
        float DistanceToPlayer = FVector::Dist(AgentLocation, PlayerLocation);
        
        FMassRepresentationLODFragment& LODFragment = MassEntitySubsystem->GetFragmentDataChecked<FMassRepresentationLODFragment>(Entity);
        
        if (DistanceToPlayer <= HighLODDistance)
        {
            LODFragment.LOD = EMassLOD::High;
            HighLODAgentCount++;
        }
        else if (DistanceToPlayer <= MediumLODDistance)
        {
            LODFragment.LOD = EMassLOD::Medium;
            MediumLODAgentCount++;
        }
        else if (DistanceToPlayer <= LowLODDistance)
        {
            LODFragment.LOD = EMassLOD::Low;
            LowLODAgentCount++;
        }
        else
        {
            LODFragment.LOD = EMassLOD::Off;
        }
    }
}

void AMassEntityCrowdManager::UpdateCrowdBehavior(float DeltaTime)
{
    if (!MassEntitySubsystem || DestinationPoints.Num() == 0)
    {
        return;
    }
    
    // Simple movement toward destinations
    for (const FMassEntityHandle& Entity : CrowdEntities)
    {
        if (!Entity.IsValid())
        {
            continue;
        }
        
        FMassTransformFragment& TransformFragment = MassEntitySubsystem->GetFragmentDataChecked<FMassTransformFragment>(Entity);
        FMassVelocityFragment& VelocityFragment = MassEntitySubsystem->GetFragmentDataChecked<FMassVelocityFragment>(Entity);
        
        FVector AgentLocation = TransformFragment.GetTransform().GetLocation();
        
        // Find closest destination
        FVector ClosestDestination = DestinationPoints[0];
        float ClosestDistance = FVector::Dist(AgentLocation, ClosestDestination);
        
        for (int32 i = 1; i < DestinationPoints.Num(); i++)
        {
            float Distance = FVector::Dist(AgentLocation, DestinationPoints[i]);
            if (Distance < ClosestDistance)
            {
                ClosestDistance = Distance;
                ClosestDestination = DestinationPoints[i];
            }
        }
        
        // Calculate movement direction
        FVector MoveDirection = (ClosestDestination - AgentLocation).GetSafeNormal();
        
        // Apply movement
        if (ClosestDistance > AgentRadius)
        {
            VelocityFragment.Value = MoveDirection * AgentSpeed;
            
            // Update position
            FVector NewLocation = AgentLocation + VelocityFragment.Value * DeltaTime;
            TransformFragment.SetTransform(FTransform(NewLocation));
        }
        else
        {
            // Agent reached destination, stop movement
            VelocityFragment.Value = FVector::ZeroVector;
        }
    }
}