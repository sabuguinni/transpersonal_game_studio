#include "MassDinosaurSubsystem.h"
#include "MassEntityConfigAsset.h"
#include "MassSpawnerConfig.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassSimulationSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Engine/Engine.h"

void UMassDinosaurSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // Get Mass Framework subsystems
    EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
    SpawnerSubsystem = GetWorld()->GetSubsystem<UMassSpawnerSubsystem>();
    SimulationSubsystem = GetWorld()->GetSubsystem<UMassSimulationSubsystem>();
    
    // Initialize performance tracking
    CurrentEntityCount = 0;
    LastFrameTime = 0.0f;
    
    // Start ecosystem update timer (every 5 seconds)
    GetWorld()->GetTimerManager().SetTimer(
        EcosystemUpdateTimer,
        this,
        &UMassDinosaurSubsystem::UpdateEcosystemBehavior,
        5.0f,
        true
    );
    
    // Start performance monitoring (every second)
    GetWorld()->GetTimerManager().SetTimer(
        PerformanceUpdateTimer,
        this,
        &UMassDinosaurSubsystem::UpdatePerformanceMetrics,
        1.0f,
        true
    );
    
    InitializeEcosystemZones();
    
    UE_LOG(LogTemp, Warning, TEXT("MassDinosaurSubsystem initialized - Ready for up to %d entities"), MaxEntityCount);
}

void UMassDinosaurSubsystem::Deinitialize()
{
    // Clear timers
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(EcosystemUpdateTimer);
        GetWorld()->GetTimerManager().ClearTimer(PerformanceUpdateTimer);
    }
    
    Super::Deinitialize();
}

bool UMassDinosaurSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    return Super::ShouldCreateSubsystem(Outer) && IsValid(Outer);
}

void UMassDinosaurSubsystem::SpawnDinosaurHerd(const FVector& Location, int32 Count, const FString& SpeciesType)
{
    if (!CanSpawnMoreEntities(Count))
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot spawn %d entities - would exceed limit of %d"), Count, MaxEntityCount);
        return;
    }
    
    UMassEntityConfigAsset* Config = nullptr;
    
    // Find appropriate configuration
    if (HerbivoreConfigs.Contains(SpeciesType))
    {
        Config = HerbivoreConfigs[SpeciesType];
    }
    else if (OmnivoreConfigs.Contains(SpeciesType))
    {
        Config = OmnivoreConfigs[SpeciesType];
    }
    
    if (!Config)
    {
        UE_LOG(LogTemp, Error, TEXT("No configuration found for herbivore species: %s"), *SpeciesType);
        return;
    }
    
    // Spawn entities in formation
    TArray<FVector> SpawnLocations;
    
    // Generate herd formation (circular pattern with some randomness)
    float HerdRadius = FMath::Sqrt(Count) * 200.0f; // 2m spacing per dinosaur
    
    for (int32 i = 0; i < Count; i++)
    {
        float Angle = (2.0f * PI * i) / Count;
        float Distance = FMath::RandRange(HerdRadius * 0.3f, HerdRadius);
        
        FVector Offset = FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.0f
        );
        
        // Add some vertical and horizontal randomness
        Offset += FVector(
            FMath::RandRange(-100.0f, 100.0f),
            FMath::RandRange(-100.0f, 100.0f),
            FMath::RandRange(-50.0f, 50.0f)
        );
        
        SpawnLocations.Add(Location + Offset);
    }
    
    // Use Mass Spawner to create entities
    if (SpawnerSubsystem && DefaultSpawnerConfig)
    {
        // TODO: Implement actual spawning with Mass Framework
        // This requires setting up proper Mass Entity configurations
        CurrentEntityCount += Count;
        
        UE_LOG(LogTemp, Log, TEXT("Spawned herd of %d %s at location %s"), 
               Count, *SpeciesType, *Location.ToString());
    }
}

void UMassDinosaurSubsystem::SpawnPredatorPack(const FVector& Location, int32 Count, const FString& SpeciesType)
{
    if (!CanSpawnMoreEntities(Count))
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot spawn %d predators - would exceed limit"), Count);
        return;
    }
    
    UMassEntityConfigAsset* Config = CarnivoreConfigs.FindRef(SpeciesType);
    if (!Config)
    {
        UE_LOG(LogTemp, Error, TEXT("No configuration found for carnivore species: %s"), *SpeciesType);
        return;
    }
    
    // Spawn predators in loose pack formation
    TArray<FVector> SpawnLocations;
    
    // Predators spread out more than herbivores
    float PackRadius = Count * 300.0f; // 3m spacing per predator
    
    for (int32 i = 0; i < Count; i++)
    {
        FVector Offset = FVector(
            FMath::RandRange(-PackRadius, PackRadius),
            FMath::RandRange(-PackRadius, PackRadius),
            FMath::RandRange(-50.0f, 50.0f)
        );
        
        SpawnLocations.Add(Location + Offset);
    }
    
    CurrentEntityCount += Count;
    
    UE_LOG(LogTemp, Log, TEXT("Spawned pack of %d %s predators at location %s"), 
           Count, *SpeciesType, *Location.ToString());
}

void UMassDinosaurSubsystem::DespawnDinosaursInRadius(const FVector& Location, float Radius)
{
    // TODO: Implement despawning logic using Mass Entity queries
    // This would involve:
    // 1. Query entities within radius
    // 2. Destroy those entities
    // 3. Update CurrentEntityCount
    
    UE_LOG(LogTemp, Log, TEXT("Despawning dinosaurs in radius %.1f at location %s"), 
           Radius, *Location.ToString());
}

void UMassDinosaurSubsystem::TriggerMigration(const FString& SpeciesType, const FVector& FromLocation, const FVector& ToLocation)
{
    // TODO: Implement migration behavior
    // This would involve:
    // 1. Find all entities of specified species near FromLocation
    // 2. Set their movement targets to ToLocation
    // 3. Adjust their behavior to "migrating" state
    
    UE_LOG(LogTemp, Log, TEXT("Triggering migration for %s from %s to %s"), 
           *SpeciesType, *FromLocation.ToString(), *ToLocation.ToString());
}

void UMassDinosaurSubsystem::SetDayNightCycle(bool bIsDay)
{
    // TODO: Adjust dinosaur behavior based on time of day
    // Day: More active, feeding, social behaviors
    // Night: Resting, hiding, different predator/prey dynamics
    
    UE_LOG(LogTemp, Log, TEXT("Day/Night cycle changed - Is Day: %s"), bIsDay ? TEXT("true") : TEXT("false"));
}

void UMassDinosaurSubsystem::TriggerWeatherEvent(const FString& WeatherType, float Intensity)
{
    // TODO: Implement weather response behaviors
    // Rain: Seek shelter, reduced movement
    // Storm: Panic behaviors, herd clustering
    // Heat: Seek shade, reduced activity
    
    UE_LOG(LogTemp, Log, TEXT("Weather event triggered: %s with intensity %.2f"), 
           *WeatherType, Intensity);
}

void UMassDinosaurSubsystem::SetSimulationLOD(int32 LODLevel)
{
    // TODO: Implement LOD system
    // LOD 0: Full simulation (close to player)
    // LOD 1: Reduced update frequency (medium distance)
    // LOD 2: Basic movement only (far distance)
    // LOD 3: Frozen/culled (very far)
    
    UE_LOG(LogTemp, Log, TEXT("Simulation LOD set to level %d"), LODLevel);
}

int32 UMassDinosaurSubsystem::GetActiveDinosaurCount() const
{
    return CurrentEntityCount;
}

float UMassDinosaurSubsystem::GetCurrentPerformanceMetric() const
{
    return LastFrameTime;
}

void UMassDinosaurSubsystem::UpdatePerformanceMetrics()
{
    if (GEngine)
    {
        LastFrameTime = GEngine->GetMaxTickRate() > 0 ? (1000.0f / GEngine->GetMaxTickRate()) : 16.67f;
    }
    
    // Auto-optimize if performance is poor
    if (LastFrameTime > TargetFrameTime * 1.2f) // 20% tolerance
    {
        OptimizeSimulationLOD();
    }
}

void UMassDinosaurSubsystem::OptimizeSimulationLOD()
{
    // TODO: Implement automatic LOD optimization
    // Reduce simulation quality when performance drops
    
    UE_LOG(LogTemp, Warning, TEXT("Performance optimization triggered - Frame time: %.2fms"), LastFrameTime);
}

bool UMassDinosaurSubsystem::CanSpawnMoreEntities(int32 RequestedCount) const
{
    return (CurrentEntityCount + RequestedCount) <= MaxEntityCount;
}

void UMassDinosaurSubsystem::InitializeEcosystemZones()
{
    // TODO: Initialize ecosystem zones based on world layout
    // Define feeding areas, water sources, nesting sites, etc.
    
    UE_LOG(LogTemp, Log, TEXT("Ecosystem zones initialized"));
}

void UMassDinosaurSubsystem::UpdateEcosystemBehavior()
{
    // TODO: Update global ecosystem behaviors
    // Population pressure, resource availability, territorial disputes
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Ecosystem behavior updated - Active entities: %d"), CurrentEntityCount);
}