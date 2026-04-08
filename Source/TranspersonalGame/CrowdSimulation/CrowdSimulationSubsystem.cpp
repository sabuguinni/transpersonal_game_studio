#include "CrowdSimulationSubsystem.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

void UCrowdSimulationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationSubsystem: Initializing..."));
    
    // Get Mass Entity subsystem reference
    MassEntitySubsystem = Collection.InitializeDependency<UMassEntitySubsystem>();
    MassSpawnerSubsystem = Collection.InitializeDependency<UMassSpawnerSubsystem>();
    
    if (!MassEntitySubsystem || !MassSpawnerSubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("CrowdSimulationSubsystem: Failed to get Mass subsystems"));
        return;
    }
    
    // Initialize default settings
    CrowdSettings.MaxHerbivores = 200;
    CrowdSettings.MaxCarnivores = 50;
    CrowdSettings.MaxFlyers = 100;
    CrowdSettings.SimulationRadius = 5000.0f;
    CrowdSettings.FullDetailDistance = 1000.0f;
    CrowdSettings.MediumDetailDistance = 2500.0f;
    CrowdSettings.LowDetailDistance = 5000.0f;
    
    bIsInitialized = true;
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationSubsystem: Initialized successfully"));
}

void UCrowdSimulationSubsystem::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationSubsystem: Deinitializing..."));
    
    // Clean up active herds
    ActiveHerds.Empty();
    HerdCenters.Empty();
    HerdLastUpdateTime.Empty();
    
    bIsInitialized = false;
    
    Super::Deinitialize();
}

bool UCrowdSimulationSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    // Only create in game worlds, not in editor
    if (UWorld* World = Cast<UWorld>(Outer))
    {
        return World->IsGameWorld();
    }
    return false;
}

void UCrowdSimulationSubsystem::InitializeCrowdSimulation()
{
    if (!bIsInitialized)
    {
        UE_LOG(LogTemp, Error, TEXT("CrowdSimulationSubsystem: Cannot initialize crowd simulation - subsystem not ready"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationSubsystem: Initializing crowd simulation..."));
    
    SetupMassEntityProcessors();
    CreateHerdBehaviorProcessors();
    
    // Spawn initial herds based on world zones
    SpawnInitialHerds();
    
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationSubsystem: Crowd simulation initialized"));
}

void UCrowdSimulationSubsystem::SetPlayerLocation(const FVector& PlayerLocation)
{
    CurrentPlayerLocation = PlayerLocation;
    
    // Update simulation bounds around player
    UpdateSimulationBounds();
    
    // Check if LOD update is needed
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastLODUpdateTime > LODUpdateInterval)
    {
        UpdateCrowdLOD();
        LastLODUpdateTime = CurrentTime;
    }
}

void UCrowdSimulationSubsystem::SpawnHerd(const FDinosaurHerdData& HerdData, const FVector& SpawnLocation)
{
    if (!bIsInitialized || !MassSpawnerSubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("CrowdSimulationSubsystem: Cannot spawn herd - system not ready"));
        return;
    }
    
    // Create new herd with unique ID
    FDinosaurHerdData NewHerd = HerdData;
    NewHerd.HerdID = NextHerdID++;
    
    // Add to active herds
    ActiveHerds.Add(NewHerd);
    HerdCenters.Add(NewHerd.HerdID, SpawnLocation);
    HerdLastUpdateTime.Add(NewHerd.HerdID, GetWorld()->GetTimeSeconds());
    
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationSubsystem: Spawned herd %d of species %s at location %s"), 
           NewHerd.HerdID, *NewHerd.SpeciesName, *SpawnLocation.ToString());
    
    // TODO: Actually spawn Mass entities for this herd
    // This will be implemented when Mass Entity processors are set up
}

void UCrowdSimulationSubsystem::TriggerPredatorAlert(const FVector& PredatorLocation, float AlertRadius)
{
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationSubsystem: Predator alert at %s with radius %f"), 
           *PredatorLocation.ToString(), AlertRadius);
    
    // Find all herds within alert radius and trigger flee behavior
    for (const FDinosaurHerdData& Herd : ActiveHerds)
    {
        if (FVector* HerdCenter = HerdCenters.Find(Herd.HerdID))
        {
            float DistanceToHerd = FVector::Dist(*HerdCenter, PredatorLocation);
            
            if (DistanceToHerd <= AlertRadius)
            {
                // Trigger flee behavior for this herd
                TriggerHerdFlee(Herd.HerdID, PredatorLocation);
            }
        }
    }
}

void UCrowdSimulationSubsystem::UpdateCrowdLOD()
{
    if (!bIsInitialized)
    {
        return;
    }
    
    // Update LOD for all active herds based on distance to player
    for (const FDinosaurHerdData& Herd : ActiveHerds)
    {
        if (FVector* HerdCenter = HerdCenters.Find(Herd.HerdID))
        {
            float DistanceToPlayer = FVector::Dist(*HerdCenter, CurrentPlayerLocation);
            
            int32 LODLevel = 0; // Full detail
            if (DistanceToPlayer > CrowdSettings.LowDetailDistance)
            {
                LODLevel = 3; // Minimal detail
            }
            else if (DistanceToPlayer > CrowdSettings.MediumDetailDistance)
            {
                LODLevel = 2; // Low detail
            }
            else if (DistanceToPlayer > CrowdSettings.FullDetailDistance)
            {
                LODLevel = 1; // Medium detail
            }
            
            // Apply LOD to herd entities
            ApplyLODToHerd(Herd.HerdID, LODLevel);
        }
    }
}

void UCrowdSimulationSubsystem::SetupMassEntityProcessors()
{
    if (!MassEntitySubsystem)
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationSubsystem: Setting up Mass Entity processors..."));
    
    // TODO: Set up custom processors for dinosaur behavior
    // - Flocking behavior processor
    // - Predator avoidance processor
    // - Grazing behavior processor
    // - Daily routine processor
}

void UCrowdSimulationSubsystem::CreateHerdBehaviorProcessors()
{
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationSubsystem: Creating herd behavior processors..."));
    
    // TODO: Implement herd-specific behavior processors
    // - Cohesion: Keep herd members together
    // - Separation: Avoid crowding
    // - Alignment: Move in same direction as neighbors
    // - Flee: Escape from predators
}

void UCrowdSimulationSubsystem::UpdateSimulationBounds()
{
    // Update the active simulation area around the player
    // Entities outside this area will be culled or simplified
    
    // TODO: Implement spatial partitioning for efficient updates
}

void UCrowdSimulationSubsystem::SpawnInitialHerds()
{
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationSubsystem: Spawning initial herds..."));
    
    // Spawn some default herds for testing
    FDinosaurHerdData TriceratopsHerd;
    TriceratopsHerd.SpeciesName = TEXT("Triceratops");
    TriceratopsHerd.MinHerdSize = 5;
    TriceratopsHerd.MaxHerdSize = 12;
    TriceratopsHerd.CohesionRadius = 600.0f;
    TriceratopsHerd.SeparationRadius = 150.0f;
    TriceratopsHerd.FleeRadius = 1000.0f;
    
    SpawnHerd(TriceratopsHerd, FVector(1000, 0, 0));
    
    FDinosaurHerdData ParasaurolophusHerd;
    ParasaurolophusHerd.SpeciesName = TEXT("Parasaurolophus");
    ParasaurolophusHerd.MinHerdSize = 8;
    ParasaurolophusHerd.MaxHerdSize = 20;
    ParasaurolophusHerd.CohesionRadius = 800.0f;
    ParasaurolophusHerd.SeparationRadius = 120.0f;
    ParasaurolophusHerd.FleeRadius = 1200.0f;
    
    SpawnHerd(ParasaurolophusHerd, FVector(-1500, 1000, 0));
}

void UCrowdSimulationSubsystem::TriggerHerdFlee(int32 HerdID, const FVector& ThreatLocation)
{
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationSubsystem: Triggering flee behavior for herd %d"), HerdID);
    
    // TODO: Implement actual flee behavior using Mass Entity system
    // - Calculate flee direction (away from threat)
    // - Increase movement speed
    // - Override normal behavior temporarily
    // - Spread panic to nearby herds
}

void UCrowdSimulationSubsystem::ApplyLODToHerd(int32 HerdID, int32 LODLevel)
{
    // TODO: Apply LOD settings to herd entities
    // LOD 0: Full detail - individual behavior, full animations
    // LOD 1: Medium detail - simplified behavior, reduced animation frequency
    // LOD 2: Low detail - group behavior only, basic animations
    // LOD 3: Minimal detail - statistical simulation, no individual entities
}