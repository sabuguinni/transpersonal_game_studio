#include "CrowdSimulationSubsystem.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

void UCrowdSimulationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // Get Mass Entity subsystem reference
    MassEntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
    MassSpawnerSubsystem = GetWorld()->GetSubsystem<UMassSpawnerSubsystem>();
    
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("CrowdSimulationSubsystem: Failed to get MassEntitySubsystem"));
        return;
    }

    if (!MassSpawnerSubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("CrowdSimulationSubsystem: Failed to get MassSpawnerSubsystem"));
        return;
    }

    // Initialize default simulation parameters
    CurrentLODLevel = 1;
    MaxSimultaneousEntities = 50000;
    
    bIsInitialized = true;
    
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationSubsystem: Initialized successfully"));
}

void UCrowdSimulationSubsystem::Deinitialize()
{
    // Clean up active simulations
    ActiveHerds.Empty();
    ActivePacks.Empty();
    ActiveFlocks.Empty();
    
    bIsInitialized = false;
    
    Super::Deinitialize();
}

void UCrowdSimulationSubsystem::Tick(float DeltaTime)
{
    if (!bIsInitialized || !MassEntitySubsystem)
    {
        return;
    }

    AccumulatedDeltaTime += DeltaTime;
    LastFrameTime = DeltaTime;

    // Update behaviors at different frequencies based on LOD
    float UpdateFrequency = 1.0f / (CurrentLODLevel + 1); // LOD 0 = 1.0s, LOD 1 = 0.5s, etc.
    
    if (AccumulatedDeltaTime >= UpdateFrequency)
    {
        UpdateHerdBehaviors(AccumulatedDeltaTime);
        UpdatePackBehaviors(AccumulatedDeltaTime);
        UpdateFlockBehaviors(AccumulatedDeltaTime);
        UpdatePerformanceLOD(AccumulatedDeltaTime);
        
        AccumulatedDeltaTime = 0.0f;
    }
}

void UCrowdSimulationSubsystem::SpawnDinosaurHerd(const FDinosaurHerdData& HerdData, const FVector& SpawnLocation)
{
    if (!bIsInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationSubsystem: Cannot spawn herd - system not initialized"));
        return;
    }

    // Add to active herds
    FDinosaurHerdData NewHerd = HerdData;
    NewHerd.HerdCenter = SpawnLocation;
    ActiveHerds.Add(NewHerd);

    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationSubsystem: Spawned %s herd of %d at %s"), 
           *HerdData.SpeciesType, HerdData.HerdSize, *SpawnLocation.ToString());

    // TODO: Implement actual Mass Entity spawning logic
    // This will be connected to MassSpawnerSubsystem in next iteration
}

void UCrowdSimulationSubsystem::SpawnPredatorPack(const FPredatorPackData& PackData, const FVector& SpawnLocation)
{
    if (!bIsInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationSubsystem: Cannot spawn pack - system not initialized"));
        return;
    }

    FPredatorPackData NewPack = PackData;
    NewPack.TerritoryCenter = SpawnLocation;
    ActivePacks.Add(NewPack);

    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationSubsystem: Spawned %s pack of %d at %s"), 
           *PackData.SpeciesType, PackData.PackSize, *SpawnLocation.ToString());
}

void UCrowdSimulationSubsystem::SpawnAerialFlock(const FAerialFlockData& FlockData, const FVector& SpawnLocation)
{
    if (!bIsInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationSubsystem: Cannot spawn flock - system not initialized"));
        return;
    }

    FAerialFlockData NewFlock = FlockData;
    NewFlock.FlightPath = SpawnLocation;
    ActiveFlocks.Add(NewFlock);

    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationSubsystem: Spawned %s flock of %d at %s"), 
           *FlockData.SpeciesType, FlockData.FlockSize, *SpawnLocation.ToString());
}

void UCrowdSimulationSubsystem::TriggerPredatorAlert(const FVector& AlertLocation, float AlertRadius)
{
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationSubsystem: Predator alert triggered at %s (radius: %.1f)"), 
           *AlertLocation.ToString(), AlertRadius);

    // Increase alert level for all herds within radius
    for (FDinosaurHerdData& Herd : ActiveHerds)
    {
        float Distance = FVector::Dist(Herd.HerdCenter, AlertLocation);
        if (Distance <= AlertRadius)
        {
            Herd.AlertLevel = FMath::Clamp(Herd.AlertLevel + 0.7f, 0.0f, 1.0f);
            Herd.MovementSpeed *= 2.0f; // Panic speed increase
            
            UE_LOG(LogTemp, Log, TEXT("Herd %s entering panic state (Alert Level: %.2f)"), 
                   *Herd.SpeciesType, Herd.AlertLevel);
        }
    }
}

void UCrowdSimulationSubsystem::TriggerStampede(const FVector& StampedeOrigin, const FVector& StampedeDirection)
{
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationSubsystem: Stampede triggered from %s towards %s"), 
           *StampedeOrigin.ToString(), *StampedeDirection.ToString());

    // All herds within 5km start stampeding
    float StampedeRadius = 500000.0f; // 5km
    
    for (FDinosaurHerdData& Herd : ActiveHerds)
    {
        float Distance = FVector::Dist(Herd.HerdCenter, StampedeOrigin);
        if (Distance <= StampedeRadius)
        {
            Herd.AlertLevel = 1.0f; // Maximum panic
            Herd.MovementSpeed *= 3.0f; // Stampede speed
            
            // TODO: Set herd movement direction towards StampedeDirection
        }
    }
}

void UCrowdSimulationSubsystem::SetGlobalWeatherState(bool bIsStormy)
{
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationSubsystem: Weather state changed - Stormy: %s"), 
           bIsStormy ? TEXT("True") : TEXT("False"));

    if (bIsStormy)
    {
        // Aerial flocks seek shelter
        for (FAerialFlockData& Flock : ActiveFlocks)
        {
            Flock.FlightAltitude *= 0.3f; // Fly lower
            Flock.FlightSpeed *= 0.7f; // Slower in storm
        }

        // Herds become more alert
        for (FDinosaurHerdData& Herd : ActiveHerds)
        {
            Herd.AlertLevel = FMath::Clamp(Herd.AlertLevel + 0.3f, 0.0f, 1.0f);
        }
    }
    else
    {
        // Return to normal behavior
        for (FAerialFlockData& Flock : ActiveFlocks)
        {
            Flock.FlightAltitude = 1500.0f; // Default altitude
            Flock.FlightSpeed = 1200.0f; // Default speed
        }
    }
}

void UCrowdSimulationSubsystem::SetSimulationLOD(int32 LODLevel)
{
    CurrentLODLevel = FMath::Clamp(LODLevel, 0, 3);
    
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationSubsystem: LOD Level set to %d"), CurrentLODLevel);

    // Adjust entity limits based on LOD
    switch (CurrentLODLevel)
    {
        case 0: MaxSimultaneousEntities = 50000; break; // Ultra
        case 1: MaxSimultaneousEntities = 25000; break; // High
        case 2: MaxSimultaneousEntities = 10000; break; // Medium
        case 3: MaxSimultaneousEntities = 5000; break;  // Low
    }
}

int32 UCrowdSimulationSubsystem::GetActiveEntityCount() const
{
    int32 TotalEntities = 0;
    
    for (const FDinosaurHerdData& Herd : ActiveHerds)
    {
        TotalEntities += Herd.HerdSize;
    }
    
    for (const FPredatorPackData& Pack : ActivePacks)
    {
        TotalEntities += Pack.PackSize;
    }
    
    for (const FAerialFlockData& Flock : ActiveFlocks)
    {
        TotalEntities += Flock.FlockSize;
    }
    
    return TotalEntities;
}

float UCrowdSimulationSubsystem::GetCurrentPerformanceMetric() const
{
    // Simple performance metric based on frame time and entity count
    float EntityDensity = static_cast<float>(GetActiveEntityCount()) / MaxSimultaneousEntities;
    float FrameTimeMetric = LastFrameTime * 60.0f; // Convert to frame budget (60fps = 1.0)
    
    return EntityDensity * FrameTimeMetric;
}

void UCrowdSimulationSubsystem::UpdateHerdBehaviors(float DeltaTime)
{
    for (FDinosaurHerdData& Herd : ActiveHerds)
    {
        // Gradually reduce alert level over time
        if (Herd.AlertLevel > 0.0f)
        {
            Herd.AlertLevel = FMath::Clamp(Herd.AlertLevel - (DeltaTime * 0.1f), 0.0f, 1.0f);
            
            // Reduce speed as alert level decreases
            if (Herd.AlertLevel < 0.5f)
            {
                Herd.MovementSpeed = FMath::Lerp(300.0f, Herd.MovementSpeed, Herd.AlertLevel * 2.0f);
            }
        }

        // TODO: Implement actual herd movement logic
        // This will integrate with Mass Entity movement processors
    }
}

void UCrowdSimulationSubsystem::UpdatePackBehaviors(float DeltaTime)
{
    for (FPredatorPackData& Pack : ActivePacks)
    {
        // Increase hunger over time
        Pack.HungerLevel = FMath::Clamp(Pack.HungerLevel + (DeltaTime * 0.05f), 0.0f, 1.0f);
        
        // Start hunting when hungry enough
        if (Pack.HungerLevel > 0.7f && !Pack.bIsHunting)
        {
            Pack.bIsHunting = true;
            UE_LOG(LogTemp, Log, TEXT("Pack %s started hunting (Hunger: %.2f)"), 
                   *Pack.SpeciesType, Pack.HungerLevel);
        }

        // TODO: Implement pack hunting AI
        // This will integrate with Combat AI Agent's systems
    }
}

void UCrowdSimulationSubsystem::UpdateFlockBehaviors(float DeltaTime)
{
    for (FAerialFlockData& Flock : ActiveFlocks)
    {
        // TODO: Implement flocking behavior (boids algorithm)
        // Separation, Alignment, Cohesion rules
        // Migration patterns based on time of day/season
    }
}

void UCrowdSimulationSubsystem::UpdatePerformanceLOD(float DeltaTime)
{
    float CurrentPerformance = GetCurrentPerformanceMetric();
    
    // Auto-adjust LOD based on performance
    if (CurrentPerformance > 1.5f && CurrentLODLevel < 3)
    {
        SetSimulationLOD(CurrentLODLevel + 1);
        UE_LOG(LogTemp, Warning, TEXT("Performance degraded - increasing LOD to %d"), CurrentLODLevel);
    }
    else if (CurrentPerformance < 0.8f && CurrentLODLevel > 0)
    {
        SetSimulationLOD(CurrentLODLevel - 1);
        UE_LOG(LogTemp, Log, TEXT("Performance improved - decreasing LOD to %d"), CurrentLODLevel);
    }
}