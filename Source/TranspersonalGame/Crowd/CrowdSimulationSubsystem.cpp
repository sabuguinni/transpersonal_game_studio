#include "CrowdSimulationSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "MassLODFragments.h"
#include "Engine/Engine.h"

DEFINE_LOG_CATEGORY(LogCrowdSimulation);

void UCrowdSimulationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogCrowdSimulation, Log, TEXT("Initializing Crowd Simulation Subsystem"));
    
    // Get Mass Entity subsystems
    MassEntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
    MassSpawnerSubsystem = GetWorld()->GetSubsystem<UMassSpawnerSubsystem>();
    
    if (!MassEntitySubsystem || !MassSpawnerSubsystem)
    {
        UE_LOG(LogCrowdSimulation, Error, TEXT("Failed to get required Mass subsystems"));
        return;
    }
    
    // Initialize performance tracking
    CurrentPerformanceMetric = 0.0f;
    ActiveCrowdCount = 0;
    
    // Setup update timers
    GetWorld()->GetTimerManager().SetTimer(
        LODUpdateTimer,
        this,
        &UCrowdSimulationSubsystem::ManageCrowdLOD,
        LODUpdateFrequency,
        true
    );
    
    GetWorld()->GetTimerManager().SetTimer(
        PerformanceUpdateTimer,
        this,
        &UCrowdSimulationSubsystem::UpdatePerformanceMetrics,
        0.1f, // 10 times per second
        true
    );
    
    GetWorld()->GetTimerManager().SetTimer(
        EcosystemUpdateTimer,
        this,
        &UCrowdSimulationSubsystem::ProcessEcosystemEvents,
        1.0f, // Once per second
        true
    );
    
    UE_LOG(LogCrowdSimulation, Log, TEXT("Crowd Simulation Subsystem initialized successfully"));
}

void UCrowdSimulationSubsystem::Deinitialize()
{
    UE_LOG(LogCrowdSimulation, Log, TEXT("Deinitializing Crowd Simulation Subsystem"));
    
    // Clear timers
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(LODUpdateTimer);
        GetWorld()->GetTimerManager().ClearTimer(PerformanceUpdateTimer);
        GetWorld()->GetTimerManager().ClearTimer(EcosystemUpdateTimer);
    }
    
    // Clear ecosystem state
    PredatorThreatZones.Empty();
    ActiveMigrationRoutes.Empty();
    
    Super::Deinitialize();
}

bool UCrowdSimulationSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    // Only create in game worlds, not in editor or other contexts
    if (UWorld* World = Cast<UWorld>(Outer))
    {
        return World->IsGameWorld();
    }
    return false;
}

void UCrowdSimulationSubsystem::SpawnDinosaurHerd(const FVector& Location, int32 Count, TSubclassOf<class ADinosaurAgent> DinosaurClass)
{
    if (!MassSpawnerSubsystem || !DinosaurClass)
    {
        UE_LOG(LogCrowdSimulation, Warning, TEXT("Cannot spawn herd - missing spawner subsystem or dinosaur class"));
        return;
    }
    
    // Check if we're at capacity
    if (ActiveCrowdCount + Count > MaxCrowdEntities)
    {
        int32 AvailableSlots = FMath::Max(0, MaxCrowdEntities - ActiveCrowdCount);
        UE_LOG(LogCrowdSimulation, Warning, TEXT("Reducing herd size from %d to %d due to capacity limits"), Count, AvailableSlots);
        Count = AvailableSlots;
    }
    
    if (Count <= 0)
    {
        return;
    }
    
    // Calculate spawn positions in a natural herd formation
    TArray<FVector> SpawnPositions;
    SpawnPositions.Reserve(Count);
    
    // Use a loose cluster formation with some randomness
    float HerdRadius = FMath::Sqrt(Count) * 50.0f; // Scale radius based on herd size
    
    for (int32 i = 0; i < Count; i++)
    {
        // Generate position within herd radius with natural clustering
        float Angle = FMath::RandRange(0.0f, 2.0f * PI);
        float Distance = FMath::RandRange(0.0f, HerdRadius) * FMath::Sqrt(FMath::FRand()); // Square root for more natural distribution
        
        FVector Offset = FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.0f
        );
        
        SpawnPositions.Add(Location + Offset);
    }
    
    // TODO: Implement actual Mass Entity spawning
    // This will be integrated with the Mass Entity Framework
    ActiveCrowdCount += Count;
    
    UE_LOG(LogCrowdSimulation, Log, TEXT("Spawned herd of %d dinosaurs at location %s. Total active: %d"), 
           Count, *Location.ToString(), ActiveCrowdCount);
}

void UCrowdSimulationSubsystem::DespawnCrowdInRadius(const FVector& Center, float Radius)
{
    if (!MassEntitySubsystem)
    {
        return;
    }
    
    // TODO: Implement Mass Entity despawning within radius
    // This will query entities within the radius and despawn them
    
    UE_LOG(LogCrowdSimulation, Log, TEXT("Despawning crowd in radius %f around %s"), Radius, *Center.ToString());
}

void UCrowdSimulationSubsystem::SetCrowdDensityMultiplier(float Multiplier)
{
    CrowdDensityMultiplier = FMath::Clamp(Multiplier, 0.1f, 2.0f);
    UE_LOG(LogCrowdSimulation, Log, TEXT("Crowd density multiplier set to %f"), CrowdDensityMultiplier);
}

void UCrowdSimulationSubsystem::UpdateLODLevels(const FVector& PlayerLocation)
{
    if (!MassEntitySubsystem)
    {
        return;
    }
    
    // TODO: Implement LOD updates based on distance from player
    // This will update the LOD fragments of Mass Entities based on distance
    
    // For now, just log the update
    static int32 LODUpdateCount = 0;
    LODUpdateCount++;
    
    if (LODUpdateCount % 10 == 0) // Log every 10th update to avoid spam
    {
        UE_LOG(LogCrowdSimulation, VeryVerbose, TEXT("Updated LOD levels for player at %s"), *PlayerLocation.ToString());
    }
}

void UCrowdSimulationSubsystem::TriggerPredatorEvent(const FVector& Location, float Radius)
{
    // Add threat zone
    PredatorThreatZones.Add(Location, Radius);
    
    // TODO: Implement predator response behavior
    // - Herbivores flee from the area
    // - Other predators might be attracted or repelled
    // - Scavengers might be drawn to aftermath
    
    UE_LOG(LogCrowdSimulation, Log, TEXT("Predator event triggered at %s with radius %f"), *Location.ToString(), Radius);
}

void UCrowdSimulationSubsystem::TriggerHerdMigration(const FVector& FromLocation, const FVector& ToLocation)
{
    ActiveMigrationRoutes.Add(ToLocation);
    
    // TODO: Implement herd migration behavior
    // - Groups of herbivores move from one area to another
    // - Creates dynamic movement patterns
    // - Affects local ecosystem balance
    
    UE_LOG(LogCrowdSimulation, Log, TEXT("Herd migration triggered from %s to %s"), 
           *FromLocation.ToString(), *ToLocation.ToString());
}

void UCrowdSimulationSubsystem::UpdatePerformanceMetrics()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Calculate current frame time
    float CurrentFrameTime = GetWorld()->GetDeltaSeconds() * 1000.0f; // Convert to milliseconds
    
    // Update performance metric (smoothed frame time)
    CurrentPerformanceMetric = FMath::Lerp(CurrentPerformanceMetric, CurrentFrameTime, 0.1f);
    
    // Adaptive crowd management based on performance
    if (CurrentPerformanceMetric > TargetFrameTime * 1.2f) // 20% over target
    {
        // Reduce crowd density if performance is poor
        if (CrowdDensityMultiplier > 0.5f)
        {
            CrowdDensityMultiplier = FMath::Max(0.5f, CrowdDensityMultiplier - 0.05f);
            UE_LOG(LogCrowdSimulation, Log, TEXT("Performance degraded, reducing crowd density to %f"), CrowdDensityMultiplier);
        }
    }
    else if (CurrentPerformanceMetric < TargetFrameTime * 0.8f) // 20% under target
    {
        // Increase crowd density if performance is good
        if (CrowdDensityMultiplier < 1.5f)
        {
            CrowdDensityMultiplier = FMath::Min(1.5f, CrowdDensityMultiplier + 0.02f);
        }
    }
}

void UCrowdSimulationSubsystem::ProcessEcosystemEvents()
{
    // Decay threat zones over time
    TArray<FVector> ExpiredThreatZones;
    
    for (auto& ThreatZone : PredatorThreatZones)
    {
        ThreatZone.Value -= 100.0f; // Reduce radius by 100 units per second
        
        if (ThreatZone.Value <= 0.0f)
        {
            ExpiredThreatZones.Add(ThreatZone.Key);
        }
    }
    
    // Remove expired threat zones
    for (const FVector& ExpiredLocation : ExpiredThreatZones)
    {
        PredatorThreatZones.Remove(ExpiredLocation);
    }
    
    // Process migration routes
    // TODO: Implement migration completion logic
    
    // Log ecosystem state periodically
    static int32 EcosystemLogCount = 0;
    EcosystemLogCount++;
    
    if (EcosystemLogCount % 30 == 0) // Every 30 seconds
    {
        UE_LOG(LogCrowdSimulation, Log, TEXT("Ecosystem state: %d active threats, %d migration routes, %d active entities"), 
               PredatorThreatZones.Num(), ActiveMigrationRoutes.Num(), ActiveCrowdCount);
    }
}

void UCrowdSimulationSubsystem::ManageCrowdLOD()
{
    if (!GetWorld() || !GetWorld()->GetFirstPlayerController())
    {
        return;
    }
    
    // Get player location
    FVector PlayerLocation = GetWorld()->GetFirstPlayerController()->GetPawn() ? 
        GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation() : 
        FVector::ZeroVector;
    
    // Update LOD levels based on player position
    UpdateLODLevels(PlayerLocation);
}