#include "Crowd_MassSimulationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassSimulationSubsystem.h"
#include "MassEntityTemplateRegistry.h"
#include "MassEntityTemplate.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassRepresentationFragments.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

// UCrowd_MovementProcessor Implementation
UCrowd_MovementProcessor::UCrowd_MovementProcessor()
{
    bAutoRegisterWithProcessingPhases = true;
    ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
    ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Movement;
    ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Behavior);
}

void UCrowd_MovementProcessor::ConfigureQueries()
{
    EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FCrowd_MovementFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FCrowd_BehaviorFragment>(EMassFragmentAccess::ReadOnly);
}

void UCrowd_MovementProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    EntityQuery.ForEachEntityChunk(EntityManager, Context, [&](FMassExecutionContext& Context)
    {
        const TArrayView<FTransformFragment> TransformList = Context.GetMutableFragmentView<FTransformFragment>();
        const TArrayView<FCrowd_MovementFragment> MovementList = Context.GetMutableFragmentView<FCrowd_MovementFragment>();
        const TConstArrayView<FCrowd_BehaviorFragment> BehaviorList = Context.GetFragmentView<FCrowd_BehaviorFragment>();

        const float DeltaTime = Context.GetDeltaTimeSeconds();
        const int32 NumEntities = Context.GetNumEntities();

        for (int32 i = 0; i < NumEntities; ++i)
        {
            FTransformFragment& Transform = TransformList[i];
            FCrowd_MovementFragment& Movement = MovementList[i];
            const FCrowd_BehaviorFragment& Behavior = BehaviorList[i];

            FVector CurrentLocation = Transform.GetTransform().GetLocation();
            FVector TargetDirection = FVector::ZeroVector;

            // Wandering behavior
            if (Movement.bIsWandering)
            {
                float DistanceToTarget = FVector::Dist(CurrentLocation, Movement.TargetLocation);
                if (DistanceToTarget < 100.0f || Movement.TargetLocation.IsZero())
                {
                    // Generate new random target within wander radius
                    FVector RandomDirection = FMath::VRand();
                    RandomDirection.Z = 0.0f; // Keep on ground
                    Movement.TargetLocation = CurrentLocation + (RandomDirection * Movement.WanderRadius);
                }

                TargetDirection = (Movement.TargetLocation - CurrentLocation).GetSafeNormal();
            }

            // Apply flocking behavior
            FVector FlockingForce = FVector::ZeroVector;
            FVector SeparationForce = FVector::ZeroVector;
            int32 NeighborCount = 0;

            // Simple flocking simulation (would be optimized with spatial partitioning in production)
            for (int32 j = 0; j < NumEntities; ++j)
            {
                if (i == j) continue;

                FVector OtherLocation = TransformList[j].GetTransform().GetLocation();
                float Distance = FVector::Dist(CurrentLocation, OtherLocation);

                if (Distance < Behavior.SocialRadius)
                {
                    NeighborCount++;
                    
                    // Cohesion: move towards average position of neighbors
                    FlockingForce += OtherLocation;

                    // Separation: avoid getting too close
                    if (Distance < Movement.SeparationDistance)
                    {
                        FVector AwayVector = (CurrentLocation - OtherLocation).GetSafeNormal();
                        SeparationForce += AwayVector * (Movement.SeparationDistance - Distance) / Movement.SeparationDistance;
                    }
                }
            }

            if (NeighborCount > 0)
            {
                FlockingForce = (FlockingForce / NeighborCount - CurrentLocation).GetSafeNormal();
            }

            // Combine forces
            FVector FinalDirection = TargetDirection + 
                                   (FlockingForce * Movement.FlockingStrength) + 
                                   (SeparationForce * 2.0f);
            FinalDirection = FinalDirection.GetSafeNormal();

            // Apply movement
            FVector NewLocation = CurrentLocation + (FinalDirection * Movement.MovementSpeed * DeltaTime);
            
            // Keep entities on ground (simple terrain following)
            NewLocation.Z = FMath::Max(NewLocation.Z, 100.0f);

            // Update transform
            FTransform NewTransform = Transform.GetTransform();
            NewTransform.SetLocation(NewLocation);
            
            // Rotate to face movement direction
            if (!FinalDirection.IsNearlyZero())
            {
                FRotator NewRotation = FinalDirection.Rotation();
                NewRotation.Pitch = 0.0f; // Keep upright
                NewTransform.SetRotation(NewRotation.Quaternion());
            }

            Transform.SetTransform(NewTransform);
        }
    });
}

// UCrowd_BehaviorProcessor Implementation
UCrowd_BehaviorProcessor::UCrowd_BehaviorProcessor()
{
    bAutoRegisterWithProcessingPhases = true;
    ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
    ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Behavior;
    ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::Movement);
}

void UCrowd_BehaviorProcessor::ConfigureQueries()
{
    EntityQuery.AddRequirement<FCrowd_BehaviorFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FCrowd_MovementFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FCrowd_SpeciesFragment>(EMassFragmentAccess::ReadOnly);
}

void UCrowd_BehaviorProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    const float CurrentTime = Context.GetWorld()->GetTimeSeconds();
    
    // Update behavior every 0.5 seconds to reduce CPU load
    if (CurrentTime - LastBehaviorUpdate < 0.5f)
    {
        return;
    }
    LastBehaviorUpdate = CurrentTime;

    EntityQuery.ForEachEntityChunk(EntityManager, Context, [&](FMassExecutionContext& Context)
    {
        const TArrayView<FCrowd_BehaviorFragment> BehaviorList = Context.GetMutableFragmentView<FCrowd_BehaviorFragment>();
        const TArrayView<FCrowd_MovementFragment> MovementList = Context.GetMutableFragmentView<FCrowd_MovementFragment>();
        const TConstArrayView<FCrowd_SpeciesFragment> SpeciesList = Context.GetFragmentView<FCrowd_SpeciesFragment>();

        const int32 NumEntities = Context.GetNumEntities();

        for (int32 i = 0; i < NumEntities; ++i)
        {
            FCrowd_BehaviorFragment& Behavior = BehaviorList[i];
            FCrowd_MovementFragment& Movement = MovementList[i];
            const FCrowd_SpeciesFragment& Species = SpeciesList[i];

            // Adjust behavior based on species traits
            if (Species.bIsCarnivore)
            {
                Behavior.AggressionLevel = FMath::Clamp(Behavior.AggressionLevel + 0.01f, 0.0f, 1.0f);
                Movement.MovementSpeed = FMath::Max(Movement.MovementSpeed, 200.0f);
            }
            else if (Species.bIsHerbivore)
            {
                Behavior.FearLevel = FMath::Clamp(Behavior.FearLevel + 0.005f, 0.0f, 1.0f);
                Movement.FlockingStrength = FMath::Max(Movement.FlockingStrength, 0.7f);
            }

            // Health affects behavior
            float HealthRatio = Species.CurrentHealth / Species.MaxHealth;
            if (HealthRatio < 0.5f)
            {
                Behavior.FearLevel = FMath::Clamp(Behavior.FearLevel + 0.02f, 0.0f, 1.0f);
                Movement.MovementSpeed *= 0.8f; // Slower when injured
            }

            // Random behavior variations
            if (FMath::RandRange(0.0f, 1.0f) < 0.05f) // 5% chance per update
            {
                Behavior.CuriosityLevel = FMath::Clamp(
                    Behavior.CuriosityLevel + FMath::RandRange(-0.1f, 0.1f), 0.0f, 1.0f);
            }
        }
    });
}

// ACrowd_MassSimulationManager Implementation
ACrowd_MassSimulationManager::ACrowd_MassSimulationManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10 times per second

    // Initialize biome configurations
    SetupBiomeConfigs();
}

void ACrowd_MassSimulationManager::BeginPlay()
{
    Super::BeginPlay();

    // Get Mass subsystems
    UWorld* World = GetWorld();
    if (World)
    {
        MassEntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
        MassSpawnerSubsystem = World->GetSubsystem<UMassSpawnerSubsystem>();
        MassSimulationSubsystem = World->GetSubsystem<UMassSimulationSubsystem>();

        if (MassEntitySubsystem && MassSpawnerSubsystem && MassSimulationSubsystem)
        {
            UE_LOG(LogTemp, Log, TEXT("Crowd_MassSimulationManager: Mass subsystems initialized"));
            InitializeCrowdSimulation();
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Crowd_MassSimulationManager: Failed to get Mass subsystems"));
        }
    }
}

void ACrowd_MassSimulationManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    LastUpdateTime += DeltaTime;
    
    if (LastUpdateTime >= UpdateFrequency)
    {
        ProcessCrowdMovement(DeltaTime);
        ProcessCrowdBehavior(DeltaTime);
        UpdatePerformanceMetrics();
        LastUpdateTime = 0.0f;
    }

    // Update LOD based on player location
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
    if (PlayerPawn && bEnableLODSystem)
    {
        UpdateCrowdLOD(PlayerPawn->GetActorLocation());
    }
}

void ACrowd_MassSimulationManager::SetupBiomeConfigs()
{
    // Savana biome (center)
    SavanaConfig.SpawnCenter = FVector(0.0f, 0.0f, 100.0f);
    SavanaConfig.SpawnRadius = 8000.0f;
    SavanaConfig.MaxEntities = 15000;
    SavanaConfig.BiomeName = TEXT("Savana");
    SavanaConfig.SpeciesTypes = {TEXT("Triceratops"), TEXT("Parasaurolophus"), TEXT("TRex")};

    // Pantano biome (southwest)
    PantanoConfig.SpawnCenter = FVector(-50000.0f, -45000.0f, 50.0f);
    PantanoConfig.SpawnRadius = 6000.0f;
    PantanoConfig.MaxEntities = 8000;
    PantanoConfig.BiomeName = TEXT("Pantano");
    PantanoConfig.SpeciesTypes = {TEXT("Brachiosaurus"), TEXT("Parasaurolophus")};

    // Floresta biome (northwest)
    FlorestaConfig.SpawnCenter = FVector(-45000.0f, 40000.0f, 150.0f);
    FlorestaConfig.SpawnRadius = 7000.0f;
    FlorestaConfig.MaxEntities = 12000;
    FlorestaConfig.BiomeName = TEXT("Floresta");
    FlorestaConfig.SpeciesTypes = {TEXT("Velociraptor"), TEXT("Protoceratops"), TEXT("Pachycephalo")};

    // Deserto biome (east)
    DesertoConfig.SpawnCenter = FVector(55000.0f, 0.0f, 200.0f);
    DesertoConfig.SpawnRadius = 5000.0f;
    DesertoConfig.MaxEntities = 6000;
    DesertoConfig.BiomeName = TEXT("Deserto");
    DesertoConfig.SpeciesTypes = {TEXT("Ankylosaurus"), TEXT("Protoceratops")};

    // Montanha biome (northeast)
    MontanhaConfig.SpawnCenter = FVector(40000.0f, 50000.0f, 400.0f);
    MontanhaConfig.SpawnRadius = 4000.0f;
    MontanhaConfig.MaxEntities = 4000;
    MontanhaConfig.BiomeName = TEXT("Montanha");
    MontanhaConfig.SpeciesTypes = {TEXT("Tsintaosaurus"), TEXT("Pachycephalo")};
}

void ACrowd_MassSimulationManager::InitializeCrowdSimulation()
{
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("InitializeCrowdSimulation: MassEntitySubsystem is null"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Initializing crowd simulation with Mass Entity system"));

    // Spawn crowds in all biomes
    SpawnCrowdInBiome(SavanaConfig);
    SpawnCrowdInBiome(PantanoConfig);
    SpawnCrowdInBiome(FlorestaConfig);
    SpawnCrowdInBiome(DesertoConfig);
    SpawnCrowdInBiome(MontanhaConfig);

    UE_LOG(LogTemp, Log, TEXT("Crowd simulation initialized with %d total entities"), CurrentEntityCount);
}

void ACrowd_MassSimulationManager::SpawnCrowdInBiome(const FCrowd_SpawnConfig& Config)
{
    if (!MassEntitySubsystem)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Spawning crowd in biome: %s"), *Config.BiomeName);

    // Create entities for this biome
    TArray<FMassEntityHandle> NewEntities;
    
    for (int32 i = 0; i < Config.MaxEntities; ++i)
    {
        // Create entity
        FMassEntityHandle Entity = MassEntitySubsystem->CreateEntity();
        
        if (Entity.IsValid())
        {
            // Add transform fragment
            FTransformFragment& Transform = MassEntitySubsystem->GetFragmentDataChecked<FTransformFragment>(Entity);
            
            // Random position within spawn radius
            FVector RandomOffset = FMath::VRand() * FMath::RandRange(0.0f, Config.SpawnRadius);
            RandomOffset.Z = 0.0f; // Keep on ground level
            FVector SpawnLocation = Config.SpawnCenter + RandomOffset;
            SpawnLocation.Z = Config.SpawnCenter.Z;
            
            Transform.SetTransform(FTransform(FRotator::ZeroRotator, SpawnLocation, FVector::OneVector));

            // Add movement fragment
            FCrowd_MovementFragment& Movement = MassEntitySubsystem->GetFragmentDataChecked<FCrowd_MovementFragment>(Entity);
            Movement.MovementSpeed = FMath::RandRange(100.0f, 250.0f);
            Movement.WanderRadius = FMath::RandRange(500.0f, 1500.0f);
            Movement.FlockingStrength = FMath::RandRange(0.3f, 0.8f);
            Movement.SeparationDistance = FMath::RandRange(150.0f, 300.0f);
            Movement.bIsWandering = true;

            // Add behavior fragment
            FCrowd_BehaviorFragment& Behavior = MassEntitySubsystem->GetFragmentDataChecked<FCrowd_BehaviorFragment>(Entity);
            Behavior.AggressionLevel = FMath::RandRange(0.1f, 0.6f);
            Behavior.FearLevel = FMath::RandRange(0.1f, 0.4f);
            Behavior.CuriosityLevel = FMath::RandRange(0.3f, 0.8f);
            Behavior.SocialRadius = FMath::RandRange(300.0f, 800.0f);
            Behavior.GroupID = i / 50; // Groups of ~50 entities

            // Add species fragment
            FCrowd_SpeciesFragment& Species = MassEntitySubsystem->GetFragmentDataChecked<FCrowd_SpeciesFragment>(Entity);
            if (Config.SpeciesTypes.Num() > 0)
            {
                Species.SpeciesName = Config.SpeciesTypes[FMath::RandRange(0, Config.SpeciesTypes.Num() - 1)];
            }
            Species.BodySize = FMath::RandRange(0.8f, 1.5f);
            Species.bIsCarnivore = Species.SpeciesName.Contains(TEXT("Rex")) || Species.SpeciesName.Contains(TEXT("raptor"));
            Species.bIsHerbivore = !Species.bIsCarnivore;
            Species.MaxHealth = FMath::RandRange(80.0f, 150.0f);
            Species.CurrentHealth = Species.MaxHealth;

            NewEntities.Add(Entity);
        }
    }

    SpawnedEntities.Append(NewEntities);
    BiomeEntityCounts.Add(Config.BiomeName, NewEntities.Num());
    CurrentEntityCount += NewEntities.Num();

    UE_LOG(LogTemp, Log, TEXT("Spawned %d entities in biome %s"), NewEntities.Num(), *Config.BiomeName);
}

void ACrowd_MassSimulationManager::UpdateCrowdLOD(const FVector& PlayerLocation)
{
    // LOD system would be implemented here
    // For now, just log the player position for debugging
    static float LastLODUpdate = 0.0f;
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    if (CurrentTime - LastLODUpdate > 2.0f) // Update LOD every 2 seconds
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("Updating crowd LOD based on player location: %s"), 
               *PlayerLocation.ToString());
        LastLODUpdate = CurrentTime;
    }
}

void ACrowd_MassSimulationManager::SetCrowdDensity(const FString& BiomeName, float DensityMultiplier)
{
    UE_LOG(LogTemp, Log, TEXT("Setting crowd density for %s to %f"), *BiomeName, DensityMultiplier);
    // Implementation would adjust entity counts based on density multiplier
}

void ACrowd_MassSimulationManager::OptimizeCrowdPerformance()
{
    if (CurrentEntityCount > MaxSimulationEntities)
    {
        UE_LOG(LogTemp, Warning, TEXT("Entity count (%d) exceeds maximum (%d), optimizing..."), 
               CurrentEntityCount, MaxSimulationEntities);
        // Implementation would reduce entity counts or increase LOD distances
    }
}

void ACrowd_MassSimulationManager::DebugCrowdStats()
{
    UE_LOG(LogTemp, Log, TEXT("=== Crowd Simulation Stats ==="));
    UE_LOG(LogTemp, Log, TEXT("Total Entities: %d"), CurrentEntityCount);
    UE_LOG(LogTemp, Log, TEXT("Max Entities: %d"), MaxSimulationEntities);
    
    for (const auto& BiomePair : BiomeEntityCounts)
    {
        UE_LOG(LogTemp, Log, TEXT("Biome %s: %d entities"), *BiomePair.Key, BiomePair.Value);
    }
}

void ACrowd_MassSimulationManager::ProcessCrowdMovement(float DeltaTime)
{
    // Movement processing is handled by UCrowd_MovementProcessor
    // This method could be used for high-level movement coordination
}

void ACrowd_MassSimulationManager::ProcessCrowdBehavior(float DeltaTime)
{
    // Behavior processing is handled by UCrowd_BehaviorProcessor
    // This method could be used for high-level behavior coordination
}

void ACrowd_MassSimulationManager::UpdatePerformanceMetrics()
{
    // Update performance metrics for optimization
    static int32 FrameCounter = 0;
    FrameCounter++;
    
    if (FrameCounter % 100 == 0) // Every 100 frames
    {
        float CurrentFPS = 1.0f / GetWorld()->GetDeltaSeconds();
        if (CurrentFPS < 30.0f && CurrentEntityCount > 1000)
        {
            UE_LOG(LogTemp, Warning, TEXT("Low FPS detected (%f), consider reducing entity count"), CurrentFPS);
        }
    }
}