// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "CrowdSimulationSubsystem.h"
#include "MassEntityProcessors.h"
#include "MassEntityManager.h"
#include "MassProcessingTypes.h"
#include "MassSpawnerSubsystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UCrowdSimulationSubsystem::UCrowdSimulationSubsystem()
{
    // Initialize default values
    TargetFrameTime = 16.67f; // 60 FPS
    MaxActiveEntities = 50000;
    CurrentQualityLevel = 2; // High quality
    PerformanceScalingFactor = 0.8f;
    bIsSimulationActive = false;
    bIsSimulationPaused = false;
    bDebugVisualizationEnabled = false;
    LastUpdateTime = 0.0f;
    AccumulatedDeltaTime = 0.0f;
    FrameCounter = 0;
    NextHerdID = 1;
}

void UCrowdSimulationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationSubsystem: Initializing..."));

    // Initialize Mass Entity system
    InitializeMassEntitySystem();
    
    // Initialize processors
    InitializeProcessors();
    
    // Initialize default species configurations
    InitializeDefaultSpecies();
    
    // Initialize spawn zones
    InitializeSpawnZones();

    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationSubsystem: Initialization complete"));
}

void UCrowdSimulationSubsystem::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationSubsystem: Deinitializing..."));

    // Stop simulation
    StopSimulation();
    
    // Clean up entities
    DespawnAllEntities();
    
    // Clean up processors
    Processors.Empty();
    
    // Clean up entity manager
    EntityManager = nullptr;

    Super::Deinitialize();
}

bool UCrowdSimulationSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    // Only create in game worlds, not in editor preview worlds
    if (UWorld* World = Cast<UWorld>(Outer))
    {
        return World->IsGameWorld();
    }
    return false;
}

void UCrowdSimulationSubsystem::StartSimulation()
{
    if (bIsSimulationActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationSubsystem: Simulation already active"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationSubsystem: Starting simulation..."));

    bIsSimulationActive = true;
    bIsSimulationPaused = false;
    LastUpdateTime = GetWorld()->GetTimeSeconds();

    // Start processing spawn zones
    UpdateSpawnZones();

    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationSubsystem: Simulation started successfully"));
}

void UCrowdSimulationSubsystem::StopSimulation()
{
    if (!bIsSimulationActive)
    {
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationSubsystem: Stopping simulation..."));

    bIsSimulationActive = false;
    bIsSimulationPaused = false;

    // Despawn all entities
    DespawnAllEntities();

    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationSubsystem: Simulation stopped"));
}

void UCrowdSimulationSubsystem::PauseSimulation()
{
    if (!bIsSimulationActive || bIsSimulationPaused)
    {
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationSubsystem: Pausing simulation"));
    bIsSimulationPaused = true;
}

void UCrowdSimulationSubsystem::ResumeSimulation()
{
    if (!bIsSimulationActive || !bIsSimulationPaused)
    {
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationSubsystem: Resuming simulation"));
    bIsSimulationPaused = false;
    LastUpdateTime = GetWorld()->GetTimeSeconds();
}

void UCrowdSimulationSubsystem::SpawnDinosaurHerd(int32 SpeciesID, FVector Location, int32 HerdSize, float SpreadRadius)
{
    if (!SpeciesConfigs.Contains(SpeciesID))
    {
        UE_LOG(LogTemp, Error, TEXT("CrowdSimulationSubsystem: Unknown species ID %d"), SpeciesID);
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationSubsystem: Spawning herd of %d entities (Species %d) at %s"), 
           HerdSize, SpeciesID, *Location.ToString());

    TArray<FMassEntityHandle> HerdMembers;
    int32 CurrentHerdID = NextHerdID++;

    // Spawn herd members in a circle pattern
    for (int32 i = 0; i < HerdSize; i++)
    {
        float Angle = (2.0f * PI * i) / HerdSize;
        float Distance = FMath::RandRange(SpreadRadius * 0.3f, SpreadRadius);
        
        FVector SpawnLocation = Location + FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.0f
        );

        FRotator SpawnRotation = FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f);
        
        FMassEntityHandle EntityHandle = CreateDinosaurEntity(SpeciesID, SpawnLocation, SpawnRotation);
        
        if (EntityHandle.IsValid())
        {
            HerdMembers.Add(EntityHandle);
            
            // Set herd membership
            if (EntityManager && EntityManager->IsEntityValid(EntityHandle))
            {
                if (FMassHerdBehaviorFragment* HerdBehavior = EntityManager->GetFragmentDataPtr<FMassHerdBehaviorFragment>(EntityHandle))
                {
                    HerdBehavior->HerdID = CurrentHerdID;
                    HerdBehavior->bIsHerdLeader = (i == 0); // First spawned is leader
                    
                    if (i == 0)
                    {
                        HerdBehavior->HerdLeaderEntity = EntityHandle;
                    }
                    else if (HerdMembers.Num() > 0)
                    {
                        HerdBehavior->HerdLeaderEntity = HerdMembers[0];
                    }
                }
            }

            OnEntitySpawned.Broadcast(SpeciesID, SpawnLocation);
        }
    }

    // Store herd membership
    if (HerdMembers.Num() > 0)
    {
        HerdMembership.Add(CurrentHerdID, HerdMembers);
        UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationSubsystem: Created herd %d with %d members"), 
               CurrentHerdID, HerdMembers.Num());
    }
}

void UCrowdSimulationSubsystem::SpawnSolitaryDinosaur(int32 SpeciesID, FVector Location, float TerritoryRadius)
{
    if (!SpeciesConfigs.Contains(SpeciesID))
    {
        UE_LOG(LogTemp, Error, TEXT("CrowdSimulationSubsystem: Unknown species ID %d"), SpeciesID);
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationSubsystem: Spawning solitary dinosaur (Species %d) at %s"), 
           SpeciesID, *Location.ToString());

    FRotator SpawnRotation = FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f);
    FMassEntityHandle EntityHandle = CreateDinosaurEntity(SpeciesID, Location, SpawnRotation);

    if (EntityHandle.IsValid() && EntityManager && EntityManager->IsEntityValid(EntityHandle))
    {
        // Set as solitary (no herd)
        if (FMassHerdBehaviorFragment* HerdBehavior = EntityManager->GetFragmentDataPtr<FMassHerdBehaviorFragment>(EntityHandle))
        {
            HerdBehavior->HerdID = -1; // No herd
            HerdBehavior->bIsHerdLeader = false;
        }

        // Set territory
        if (FMassDinosaurSpeciesFragment* Species = EntityManager->GetFragmentDataPtr<FMassDinosaurSpeciesFragment>(EntityHandle))
        {
            Species->TerritorialRadius = TerritoryRadius;
        }

        if (FMassDinosaurLifecycleFragment* Lifecycle = EntityManager->GetFragmentDataPtr<FMassDinosaurLifecycleFragment>(EntityHandle))
        {
            Lifecycle->TerritoryCenter = Location;
        }

        OnEntitySpawned.Broadcast(SpeciesID, Location);
    }
}

FMassEntityHandle UCrowdSimulationSubsystem::CreateDinosaurEntity(int32 SpeciesID, FVector Location, FRotator Rotation)
{
    if (!EntityManager)
    {
        UE_LOG(LogTemp, Error, TEXT("CrowdSimulationSubsystem: EntityManager not initialized"));
        return FMassEntityHandle();
    }

    if (!SpeciesConfigs.Contains(SpeciesID))
    {
        UE_LOG(LogTemp, Error, TEXT("CrowdSimulationSubsystem: Unknown species ID %d"), SpeciesID);
        return FMassEntityHandle();
    }

    const FDinosaurSpeciesConfig& SpeciesConfig = SpeciesConfigs[SpeciesID];

    // Create entity
    FMassEntityHandle EntityHandle = EntityManager->CreateEntity();
    
    if (!EntityHandle.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("CrowdSimulationSubsystem: Failed to create entity"));
        return FMassEntityHandle();
    }

    // Add transform fragment
    FTransformFragment& Transform = EntityManager->AddFragmentToEntity<FTransformFragment>(EntityHandle);
    Transform.SetTransform(FTransform(Rotation, Location, FVector::OneVector));

    // Add velocity fragment
    FMassVelocityFragment& Velocity = EntityManager->AddFragmentToEntity<FMassVelocityFragment>(EntityHandle);
    Velocity.Value = FVector::ZeroVector;

    // Add species fragment
    FMassDinosaurSpeciesFragment& Species = EntityManager->AddFragmentToEntity<FMassDinosaurSpeciesFragment>(EntityHandle);
    Species.SpeciesID = SpeciesID;
    Species.AggressionLevel = SpeciesConfig.BaseAggressionLevel + FMath::RandRange(-0.2f, 0.2f);
    Species.HerdInstinct = SpeciesConfig.BaseHerdInstinct + FMath::RandRange(-0.1f, 0.1f);
    Species.TerritorialRadius = SpeciesConfig.BaseTerritorialRadius;
    Species.bIsCarnivore = SpeciesConfig.bIsCarnivore;
    Species.bIsPackHunter = SpeciesConfig.bIsPackHunter;
    Species.BodySize = SpeciesConfig.BaseBodySize + FMath::RandRange(-0.2f, 0.2f);
    Species.MaxSpeed = SpeciesConfig.BaseMaxSpeed + FMath::RandRange(-100.0f, 100.0f);
    Species.DetectionRange = SpeciesConfig.BaseDetectionRange;

    // Add lifecycle fragment
    FMassDinosaurLifecycleFragment& Lifecycle = EntityManager->AddFragmentToEntity<FMassDinosaurLifecycleFragment>(EntityHandle);
    Lifecycle.CurrentActivity = 0; // Idle
    Lifecycle.ActivityStartTime = GetWorld()->GetTimeSeconds();
    Lifecycle.ActivityDuration = FMath::RandRange(60.0f, 300.0f);
    Lifecycle.HungerLevel = FMath::RandRange(0.3f, 0.7f);
    Lifecycle.ThirstLevel = FMath::RandRange(0.3f, 0.7f);
    Lifecycle.EnergyLevel = FMath::RandRange(0.7f, 1.0f);
    Lifecycle.SocialNeed = FMath::RandRange(0.2f, 0.6f);
    Lifecycle.TerritoryCenter = Location;

    // Add herd behavior fragment
    FMassHerdBehaviorFragment& HerdBehavior = EntityManager->AddFragmentToEntity<FMassHerdBehaviorFragment>(EntityHandle);
    HerdBehavior.HerdID = -1; // Will be set by spawning functions
    HerdBehavior.bIsHerdLeader = false;
    HerdBehavior.CohesionWeight = 1.0f;
    HerdBehavior.SeparationWeight = 2.0f;
    HerdBehavior.AlignmentWeight = 1.0f;
    HerdBehavior.AlertLevel = 0;

    // Add environment awareness fragment
    FMassEnvironmentAwarenessFragment& Environment = EntityManager->AddFragmentToEntity<FMassEnvironmentAwarenessFragment>(EntityHandle);
    Environment.PreferredBiome = SpeciesConfig.PreferredBiomes.Num() > 0 ? SpeciesConfig.PreferredBiomes[0] : 0;
    Environment.TerrainSuitability = 1.0f;
    Environment.bIsNocturnal = SpeciesConfig.bIsNocturnal;
    Environment.WeatherComfort = 1.0f;
    Environment.CurrentDestination = Location;

    // Add visualization fragment
    FMassVisualizationFragment& Visualization = EntityManager->AddFragmentToEntity<FMassVisualizationFragment>(EntityHandle);
    Visualization.CurrentLOD = 0; // High detail initially
    Visualization.DistanceToPlayer = 10000.0f;
    Visualization.HighDetailMesh = SpeciesConfig.HighDetailMesh;
    Visualization.MediumDetailMesh = SpeciesConfig.MediumDetailMesh;
    Visualization.LowDetailMesh = SpeciesConfig.LowDetailMesh;
    Visualization.SpeciesMaterial = SpeciesConfig.BaseMaterial;
    Visualization.AnimationState = 0; // Idle
    Visualization.AnimationSpeed = 1.0f;
    Visualization.ColorVariation = FLinearColor(
        FMath::RandRange(0.8f, 1.2f),
        FMath::RandRange(0.8f, 1.2f),
        FMath::RandRange(0.8f, 1.2f),
        1.0f
    );
    Visualization.SizeVariation = FMath::RandRange(0.8f, 1.2f);

    // Add performance fragment
    FMassPerformanceFragment& Performance = EntityManager->AddFragmentToEntity<FMassPerformanceFragment>(EntityHandle);
    Performance.UpdateInterval = 0.1f;
    Performance.LastUpdateTime = GetWorld()->GetTimeSeconds();
    Performance.ImportanceScore = 1.0f;
    Performance.bIsVisible = true;
    Performance.bIsInPlayerRange = false;
    Performance.bRequiresHighFrequencyUpdate = false;
    Performance.ProcessingBudget = 100;
    Performance.RenderingBudget = 50;

    // Track entity
    EntityToSpeciesMap.Add(EntityHandle, SpeciesID);
    if (!SpeciesToEntitiesMap.Contains(SpeciesID))
    {
        SpeciesToEntitiesMap.Add(SpeciesID, TArray<FMassEntityHandle>());
    }
    SpeciesToEntitiesMap[SpeciesID].Add(EntityHandle);
    ActiveEntities.Add(EntityHandle);

    // Update metrics
    CurrentMetrics.TotalActiveEntities = ActiveEntities.Num();

    return EntityHandle;
}

void UCrowdSimulationSubsystem::InitializeMassEntitySystem()
{
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationSubsystem: Initializing Mass Entity system..."));

    // Get or create entity manager
    if (UWorld* World = GetWorld())
    {
        if (UMassSpawnerSubsystem* SpawnerSubsystem = World->GetSubsystem<UMassSpawnerSubsystem>())
        {
            EntityManager = &SpawnerSubsystem->GetMutableEntityManager();
            UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationSubsystem: Entity manager acquired"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("CrowdSimulationSubsystem: Failed to get MassSpawnerSubsystem"));
        }
    }
}

void UCrowdSimulationSubsystem::InitializeProcessors()
{
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationSubsystem: Initializing processors..."));

    // Create and configure processors
    if (UMassDinosaurLifecycleProcessor* LifecycleProcessor = NewObject<UMassDinosaurLifecycleProcessor>(this))
    {
        Processors.Add(LifecycleProcessor);
        UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationSubsystem: Lifecycle processor created"));
    }

    if (UMassHerdBehaviorProcessor* HerdProcessor = NewObject<UMassHerdBehaviorProcessor>(this))
    {
        Processors.Add(HerdProcessor);
        UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationSubsystem: Herd behavior processor created"));
    }

    if (UMassEnvironmentProcessor* EnvironmentProcessor = NewObject<UMassEnvironmentProcessor>(this))
    {
        Processors.Add(EnvironmentProcessor);
        UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationSubsystem: Environment processor created"));
    }

    if (UMassVisualizationProcessor* VisualizationProcessor = NewObject<UMassVisualizationProcessor>(this))
    {
        Processors.Add(VisualizationProcessor);
        UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationSubsystem: Visualization processor created"));
    }

    if (UMassPerformanceOptimizationProcessor* PerformanceProcessor = NewObject<UMassPerformanceOptimizationProcessor>(this))
    {
        Processors.Add(PerformanceProcessor);
        UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationSubsystem: Performance processor created"));
    }

    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationSubsystem: %d processors initialized"), Processors.Num());
}

void UCrowdSimulationSubsystem::InitializeDefaultSpecies()
{
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationSubsystem: Initializing default species..."));

    // Triceratops - Herbivore herd animal
    FDinosaurSpeciesConfig Triceratops;
    Triceratops.SpeciesName = TEXT("Triceratops");
    Triceratops.SpeciesID = 1;
    Triceratops.BaseAggressionLevel = 0.3f;
    Triceratops.BaseHerdInstinct = 0.8f;
    Triceratops.BaseTerritorialRadius = 500.0f;
    Triceratops.bIsCarnivore = false;
    Triceratops.bIsPackHunter = false;
    Triceratops.bIsNocturnal = false;
    Triceratops.BaseBodySize = 2.0f;
    Triceratops.BaseMaxSpeed = 400.0f;
    Triceratops.BaseDetectionRange = 1500.0f;
    Triceratops.MaxPopulation = 200;
    Triceratops.PreferredBiomes = {0, 1}; // Forest, Plains
    RegisterDinosaurSpecies(Triceratops);

    // Velociraptor - Carnivore pack hunter
    FDinosaurSpeciesConfig Velociraptor;
    Velociraptor.SpeciesName = TEXT("Velociraptor");
    Velociraptor.SpeciesID = 2;
    Velociraptor.BaseAggressionLevel = 0.9f;
    Velociraptor.BaseHerdInstinct = 0.7f;
    Velociraptor.BaseTerritorialRadius = 2000.0f;
    Velociraptor.bIsCarnivore = true;
    Velociraptor.bIsPackHunter = true;
    Velociraptor.bIsNocturnal = false;
    Velociraptor.BaseBodySize = 0.8f;
    Velociraptor.BaseMaxSpeed = 800.0f;
    Velociraptor.BaseDetectionRange = 3000.0f;
    Velociraptor.MaxPopulation = 100;
    Velociraptor.PreferredBiomes = {0, 1}; // Forest, Plains
    RegisterDinosaurSpecies(Velociraptor);

    // T-Rex - Solitary apex predator
    FDinosaurSpeciesConfig TRex;
    TRex.SpeciesName = TEXT("Tyrannosaurus Rex");
    TRex.SpeciesID = 3;
    TRex.BaseAggressionLevel = 1.0f;
    TRex.BaseHerdInstinct = 0.1f;
    TRex.BaseTerritorialRadius = 5000.0f;
    TRex.bIsCarnivore = true;
    TRex.bIsPackHunter = false;
    TRex.bIsNocturnal = false;
    TRex.BaseBodySize = 4.0f;
    TRex.BaseMaxSpeed = 600.0f;
    TRex.BaseDetectionRange = 4000.0f;
    TRex.MaxPopulation = 10;
    TRex.PreferredBiomes = {0, 1}; // Forest, Plains
    RegisterDinosaurSpecies(TRex);

    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationSubsystem: %d default species registered"), SpeciesConfigs.Num());
}

void UCrowdSimulationSubsystem::InitializeSpawnZones()
{
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationSubsystem: Initializing spawn zones..."));

    // Forest zone for herbivores
    FDinosaurSpawnZone ForestZone;
    ForestZone.Center = FVector(0.0f, 0.0f, 0.0f);
    ForestZone.Radius = 10000.0f;
    ForestZone.BiomeType = 0; // Forest
    ForestZone.AllowedSpecies = {1}; // Triceratops
    ForestZone.MaxEntitiesInZone = 500;
    ForestZone.SpawnDensity = 1.0f;
    ForestZone.bIsActiveSpawnZone = true;
    SpawnZones.Add(ForestZone);

    // Plains zone for mixed species
    FDinosaurSpawnZone PlainsZone;
    PlainsZone.Center = FVector(15000.0f, 0.0f, 0.0f);
    PlainsZone.Radius = 8000.0f;
    PlainsZone.BiomeType = 1; // Plains
    PlainsZone.AllowedSpecies = {1, 2}; // Triceratops, Velociraptor
    PlainsZone.MaxEntitiesInZone = 300;
    PlainsZone.SpawnDensity = 0.8f;
    PlainsZone.bIsActiveSpawnZone = true;
    SpawnZones.Add(PlainsZone);

    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationSubsystem: %d spawn zones initialized"), SpawnZones.Num());
}

void UCrowdSimulationSubsystem::RegisterDinosaurSpecies(const FDinosaurSpeciesConfig& SpeciesConfig)
{
    SpeciesConfigs.Add(SpeciesConfig.SpeciesID, SpeciesConfig);
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationSubsystem: Registered species %d: %s"), 
           SpeciesConfig.SpeciesID, *SpeciesConfig.SpeciesName);
}

FCrowdSimulationMetrics UCrowdSimulationSubsystem::GetSimulationMetrics() const
{
    return CurrentMetrics;
}

void UCrowdSimulationSubsystem::DespawnAllEntities()
{
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationSubsystem: Despawning all entities..."));

    if (EntityManager)
    {
        for (const FMassEntityHandle& EntityHandle : ActiveEntities)
        {
            if (EntityManager->IsEntityValid(EntityHandle))
            {
                EntityManager->DestroyEntity(EntityHandle);
            }
        }
    }

    // Clear tracking data
    ActiveEntities.Empty();
    EntityToSpeciesMap.Empty();
    SpeciesToEntitiesMap.Empty();
    HerdMembership.Empty();
    NextHerdID = 1;

    // Update metrics
    CurrentMetrics.TotalActiveEntities = 0;
    CurrentMetrics.HighDetailEntities = 0;
    CurrentMetrics.MediumDetailEntities = 0;
    CurrentMetrics.LowDetailEntities = 0;
    CurrentMetrics.CulledEntities = 0;

    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationSubsystem: All entities despawned"));
}

void UCrowdSimulationSubsystem::UpdateSpawnZones()
{
    if (!bIsSimulationActive || bIsSimulationPaused)
    {
        return;
    }

    float CurrentTime = GetWorld()->GetTimeSeconds();
    float DeltaTime = CurrentTime - LastUpdateTime;
    LastUpdateTime = CurrentTime;

    // Process each spawn zone
    for (const FDinosaurSpawnZone& Zone : SpawnZones)
    {
        if (Zone.bIsActiveSpawnZone)
        {
            ProcessSpawnZones(DeltaTime);
        }
    }
}

void UCrowdSimulationSubsystem::ProcessSpawnZones(float DeltaTime)
{
    // Simple spawn logic - spawn some entities if below target
    if (ActiveEntities.Num() < 100) // Target 100 entities for testing
    {
        // Spawn a small herd
        FVector SpawnLocation = FVector(
            FMath::RandRange(-5000.0f, 5000.0f),
            FMath::RandRange(-5000.0f, 5000.0f),
            100.0f
        );
        
        SpawnDinosaurHerd(1, SpawnLocation, 5, 300.0f); // Triceratops herd
    }
}