#include "CrowdSimulationManager.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

ACrowdSimulationManager::ACrowdSimulationManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10 FPS for crowd management
    
    // Initialize default values
    MaxSimultaneousAgents = 50000;
    EcosystemUpdateInterval = 1.0f;
    PredatorPreyRatio = 0.1f;
    
    HerdCohesionRadius = 2000.0f;
    HerdSeparationRadius = 500.0f;
    HerdAlignmentRadius = 1500.0f;
    
    HuntingRadius = 5000.0f;
    PredatorStalkDistance = 3000.0f;
    
    LODLevel0Distance = 1000;
    LODLevel1Distance = 5000;
    LODLevel2Distance = 10000;
    
    LastEcosystemUpdate = 0.0f;
}

void ACrowdSimulationManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Get Mass Entity subsystems
    MassEntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
    MassSpawnerSubsystem = GetWorld()->GetSubsystem<UMassSpawnerSubsystem>();
    
    if (!MassEntitySubsystem || !MassSpawnerSubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("CrowdSimulationManager: Failed to get Mass Entity subsystems"));
        return;
    }
    
    // Initialize the ecosystem
    InitializeEcosystem();
}

void ACrowdSimulationManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    LastEcosystemUpdate += DeltaTime;
    
    if (LastEcosystemUpdate >= EcosystemUpdateInterval)
    {
        UpdateLODLevels();
        OptimizeAgentCount();
        LastEcosystemUpdate = 0.0f;
    }
}

void ACrowdSimulationManager::InitializeEcosystem()
{
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Initializing Jurassic ecosystem..."));
    
    // Initialize species populations
    SpeciesPopulation.Empty();
    
    // Herbivore species (90% of population)
    SpeciesPopulation.Add(TEXT("Triceratops"), 0);
    SpeciesPopulation.Add(TEXT("Brachiosaurus"), 0);
    SpeciesPopulation.Add(TEXT("Stegosaurus"), 0);
    SpeciesPopulation.Add(TEXT("Parasaurolophus"), 0);
    SpeciesPopulation.Add(TEXT("Ankylosaurus"), 0);
    
    // Carnivore species (10% of population)
    SpeciesPopulation.Add(TEXT("Velociraptor"), 0);
    SpeciesPopulation.Add(TEXT("Allosaurus"), 0);
    SpeciesPopulation.Add(TEXT("TyrannosaurusRex"), 0);
    SpeciesPopulation.Add(TEXT("Carnotaurus"), 0);
    
    // Set up initial migration routes
    MigrationRoutes.Empty();
    MigrationRoutes.Add(FVector(0, 0, 0));      // River valley
    MigrationRoutes.Add(FVector(10000, 0, 0));  // Plains
    MigrationRoutes.Add(FVector(5000, 10000, 0)); // Forest edge
    MigrationRoutes.Add(FVector(-5000, 5000, 0)); // Mountain foothills
    
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Ecosystem initialized with %d species"), SpeciesPopulation.Num());
}

void ACrowdSimulationManager::SpawnHerd(const FVector& Location, int32 HerdSize, const FString& SpeciesType)
{
    if (!MassSpawnerSubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("CrowdSimulationManager: MassSpawnerSubsystem not available"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Spawning herd of %d %s at location %s"), 
           HerdSize, *SpeciesType, *Location.ToString());
    
    // Update species population count
    if (SpeciesPopulation.Contains(SpeciesType))
    {
        SpeciesPopulation[SpeciesType] += HerdSize;
    }
    
    // TODO: Implement actual Mass Entity spawning
    // This would use MassSpawnerSubsystem to create entities with proper fragments
    // Each entity would have: Transform, Velocity, Species, Herd ID, Behavior State
}

void ACrowdSimulationManager::SpawnPredatorPack(const FVector& Location, int32 PackSize, const FString& SpeciesType)
{
    if (!MassSpawnerSubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("CrowdSimulationManager: MassSpawnerSubsystem not available"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Spawning predator pack of %d %s at location %s"), 
           PackSize, *SpeciesType, *Location.ToString());
    
    // Update species population count
    if (SpeciesPopulation.Contains(SpeciesType))
    {
        SpeciesPopulation[SpeciesType] += PackSize;
    }
    
    // TODO: Implement actual Mass Entity spawning for predators
    // Predators have different behavior patterns: hunting, stalking, territorial
}

void ACrowdSimulationManager::TriggerStampedeEvent(const FVector& ThreatLocation, float ThreatRadius)
{
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: STAMPEDE triggered at %s, radius %f"), 
           *ThreatLocation.ToString(), ThreatRadius);
    
    // TODO: Implement stampede behavior
    // All herbivores within ThreatRadius should enter panic state
    // Panic state: increase speed, follow nearest escape route, ignore normal herd behavior
    // Stampede can trample smaller creatures and the player
}

void ACrowdSimulationManager::SetMigrationRoute(const TArray<FVector>& WayPoints, const FString& SpeciesType)
{
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Setting migration route for %s with %d waypoints"), 
           *SpeciesType, WayPoints.Num());
    
    // TODO: Implement migration behavior
    // Herds of specified species will follow waypoints over time
    // Migration is seasonal and affects entire populations
}

void ACrowdSimulationManager::OnPlayerDetectedByHerd(const FVector& PlayerLocation, float DetectionRadius)
{
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Player detected at %s, detection radius %f"), 
           *PlayerLocation.ToString(), DetectionRadius);
    
    // TODO: Implement player detection response
    // Herbivores: cautious observation, possible flight response
    // Predators: interest, possible stalking behavior
    // Response depends on species temperament and player behavior
}

void ACrowdSimulationManager::OnPlayerThreatLevel(float ThreatLevel)
{
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Player threat level changed to %f"), ThreatLevel);
    
    // TODO: Implement dynamic threat response
    // ThreatLevel 0.0: Player is hidden/peaceful - normal behavior
    // ThreatLevel 0.5: Player is visible but non-threatening - cautious behavior
    // ThreatLevel 1.0: Player is immediate threat - fight or flight response
}

void ACrowdSimulationManager::UpdateLODLevels()
{
    // TODO: Implement LOD system for crowd simulation
    // LOD 0 (0-1000m): Full simulation - individual behavior, detailed animations
    // LOD 1 (1000-5000m): Reduced simulation - simplified behavior, basic animations
    // LOD 2 (5000-10000m): Minimal simulation - group behavior only, no individual AI
    // LOD 3 (10000m+): Static representation or culled entirely
    
    if (!MassEntitySubsystem)
        return;
    
    // Get player location for distance calculations
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
        return;
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // TODO: Query all Mass entities and update their LOD based on distance to player
}

void ACrowdSimulationManager::OptimizeAgentCount()
{
    // TODO: Implement dynamic agent optimization
    // If agent count exceeds MaxSimultaneousAgents:
    // 1. Cull distant agents first
    // 2. Merge small herds into larger ones
    // 3. Reduce population in low-priority areas
    // 4. Maintain ecosystem balance (predator/prey ratio)
    
    int32 CurrentAgentCount = 0;
    for (const auto& Population : SpeciesPopulation)
    {
        CurrentAgentCount += Population.Value;
    }
    
    if (CurrentAgentCount > MaxSimultaneousAgents)
    {
        UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Agent count (%d) exceeds maximum (%d), optimizing..."), 
               CurrentAgentCount, MaxSimultaneousAgents);
        
        // TODO: Implement actual optimization logic
    }
}