#include "CrowdSimulationManager.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"

ACrowdSimulationManager::ACrowdSimulationManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10 FPS for crowd management
    
    // Initialize default biome data
    InitializeBiomeData();
}

void ACrowdSimulationManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Get Mass subsystems
    MassEntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
    MassSpawnerSubsystem = GetWorld()->GetSubsystem<UMassSpawnerSubsystem>();
    
    if (!MassEntitySubsystem || !MassSpawnerSubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("CrowdSimulationManager: Failed to get Mass subsystems"));
        return;
    }
    
    // Start with initial population
    UpdatePopulationDensity();
    
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Initialized with %d max entities"), MaxTotalEntities);
}

void ACrowdSimulationManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    LastFrameTime = DeltaTime;
    
    // Update time of day
    UpdateTimeOfDay(DeltaTime);
    
    // Update population periodically
    PopulationUpdateTimer += DeltaTime;
    if (PopulationUpdateTimer >= PopulationUpdateInterval)
    {
        UpdatePopulationDensity();
        PopulationUpdateTimer = 0.0f;
    }
    
    // Process emergent behaviors
    ProcessEmergentTriggers(DeltaTime);
    
    // Optimize performance if needed
    if (bPerformanceOptimizationEnabled)
    {
        OptimizePerformance();
    }
}

void ACrowdSimulationManager::InitializeBiomeData()
{
    BiomePopulations.Empty();
    
    // Dense Forest - High herbivore density, moderate carnivores
    FBiomePopulationData DenseForest;
    DenseForest.BiomeType = EBiomeType::DenseForest;
    DenseForest.MaxHerbivores = 300;
    DenseForest.MaxCarnivores = 75;
    DenseForest.MaxSmallCreatures = 800;
    DenseForest.ActivityMultiplier = 0.8f; // Slower movement in dense forest
    DenseForest.TimeOfDayMultipliers = {0.6f, 1.0f, 0.8f, 1.2f, 1.5f, 0.3f}; // Dawn to Night
    BiomePopulations.Add(DenseForest);
    
    // Open Plains - High visibility, large herds
    FBiomePopulationData OpenPlains;
    OpenPlains.BiomeType = EBiomeType::OpenPlains;
    OpenPlains.MaxHerbivores = 500;
    OpenPlains.MaxCarnivores = 100;
    OpenPlains.MaxSmallCreatures = 400;
    OpenPlains.ActivityMultiplier = 1.3f; // Faster movement in open areas
    OpenPlains.TimeOfDayMultipliers = {1.2f, 1.5f, 1.0f, 1.3f, 1.8f, 0.2f};
    BiomePopulations.Add(OpenPlains);
    
    // River Banks - Water-dependent species
    FBiomePopulationData RiverBanks;
    RiverBanks.BiomeType = EBiomeType::RiverBanks;
    RiverBanks.MaxHerbivores = 250;
    RiverBanks.MaxCarnivores = 80;
    RiverBanks.MaxSmallCreatures = 600;
    RiverBanks.ActivityMultiplier = 1.1f;
    RiverBanks.TimeOfDayMultipliers = {1.8f, 1.2f, 0.8f, 1.0f, 2.0f, 0.4f}; // High activity at dawn/dusk
    BiomePopulations.Add(RiverBanks);
    
    // Rocky Outcrops - Specialized climbers and ambush predators
    FBiomePopulationData RockyOutcrops;
    RockyOutcrops.BiomeType = EBiomeType::RockyOutcrops;
    RockyOutcrops.MaxHerbivores = 150;
    RockyOutcrops.MaxCarnivores = 120;
    RockyOutcrops.MaxSmallCreatures = 300;
    RockyOutcrops.ActivityMultiplier = 0.7f; // Careful movement on rocks
    RockyOutcrops.TimeOfDayMultipliers = {0.8f, 1.0f, 1.2f, 1.0f, 0.9f, 1.5f}; // More active at night
    BiomePopulations.Add(RockyOutcrops);
    
    // Swamp Lands - Amphibious and specialized species
    FBiomePopulationData SwampLands;
    SwampLands.BiomeType = EBiomeType::SwampLands;
    SwampLands.MaxHerbivores = 200;
    SwampLands.MaxCarnivores = 150;
    SwampLands.MaxSmallCreatures = 700;
    SwampLands.ActivityMultiplier = 0.6f; // Slow movement in swamp
    SwampLands.TimeOfDayMultipliers = {1.0f, 0.8f, 0.6f, 0.8f, 1.2f, 1.8f}; // Most active at night
    BiomePopulations.Add(SwampLands);
    
    // Caves - Specialized cave dwellers
    FBiomePopulationData Caves;
    Caves.BiomeType = EBiomeType::Caves;
    Caves.MaxHerbivores = 50;
    Caves.MaxCarnivores = 80;
    Caves.MaxSmallCreatures = 200;
    Caves.ActivityMultiplier = 0.5f; // Limited movement in caves
    Caves.TimeOfDayMultipliers = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}; // Constant activity
    BiomePopulations.Add(Caves);
}

void ACrowdSimulationManager::UpdatePopulationDensity()
{
    if (!MassSpawnerSubsystem)
        return;
    
    CurrentActiveEntities = GetActiveEntityCount();
    
    // Calculate desired population based on time of day and biome
    int32 DesiredTotal = 0;
    
    for (const FBiomePopulationData& BiomeData : BiomePopulations)
    {
        float TimeMultiplier = 1.0f;
        if (BiomeData.TimeOfDayMultipliers.IsValidIndex(static_cast<int32>(CurrentTimeOfDay)))
        {
            TimeMultiplier = BiomeData.TimeOfDayMultipliers[static_cast<int32>(CurrentTimeOfDay)];
        }
        
        int32 BiomeDesired = FMath::RoundToInt(
            (BiomeData.MaxHerbivores + BiomeData.MaxCarnivores + BiomeData.MaxSmallCreatures) * 
            TimeMultiplier * BiomeData.ActivityMultiplier
        );
        
        DesiredTotal += BiomeDesired;
    }
    
    // Clamp to maximum
    DesiredTotal = FMath::Min(DesiredTotal, MaxTotalEntities);
    
    // Adjust population if needed
    int32 PopulationDifference = DesiredTotal - CurrentActiveEntities;
    
    if (FMath::Abs(PopulationDifference) > MaxTotalEntities * 0.1f) // 10% threshold
    {
        if (PopulationDifference > 0)
        {
            // Spawn more entities
            UE_LOG(LogTemp, Log, TEXT("CrowdSimulation: Spawning %d entities (Current: %d, Target: %d)"), 
                   PopulationDifference, CurrentActiveEntities, DesiredTotal);
        }
        else
        {
            // Despawn excess entities
            UE_LOG(LogTemp, Log, TEXT("CrowdSimulation: Despawning %d entities (Current: %d, Target: %d)"), 
                   -PopulationDifference, CurrentActiveEntities, DesiredTotal);
        }
    }
}

void ACrowdSimulationManager::SetTimeOfDay(ETimeOfDay NewTimeOfDay)
{
    if (CurrentTimeOfDay != NewTimeOfDay)
    {
        CurrentTimeOfDay = NewTimeOfDay;
        UE_LOG(LogTemp, Log, TEXT("CrowdSimulation: Time of day changed to %d"), static_cast<int32>(NewTimeOfDay));
        
        // Trigger immediate population update
        UpdatePopulationDensity();
    }
}

int32 ACrowdSimulationManager::GetActiveEntityCount() const
{
    if (MassEntitySubsystem)
    {
        // This would need to be implemented based on actual Mass Entity counting
        // For now, return tracked count
        return CurrentActiveEntities;
    }
    return 0;
}

float ACrowdSimulationManager::GetBiomeActivityLevel(EBiomeType BiomeType) const
{
    for (const FBiomePopulationData& BiomeData : BiomePopulations)
    {
        if (BiomeData.BiomeType == BiomeType)
        {
            float TimeMultiplier = 1.0f;
            if (BiomeData.TimeOfDayMultipliers.IsValidIndex(static_cast<int32>(CurrentTimeOfDay)))
            {
                TimeMultiplier = BiomeData.TimeOfDayMultipliers[static_cast<int32>(CurrentTimeOfDay)];
            }
            return BiomeData.ActivityMultiplier * TimeMultiplier;
        }
    }
    return 1.0f;
}

void ACrowdSimulationManager::TriggerEmergentBehavior(const FString& TriggerName, FVector Location, float Intensity)
{
    FEmergentBehaviorTrigger NewTrigger;
    NewTrigger.TriggerName = TriggerName;
    NewTrigger.TriggerRadius = 1000.0f * Intensity;
    NewTrigger.IntensityMultiplier = 1.0f + Intensity;
    NewTrigger.DurationSeconds = 30.0f;
    
    ActiveTriggers.Add(NewTrigger);
    
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulation: Triggered emergent behavior '%s' at intensity %.2f"), 
           *TriggerName, Intensity);
}

void ACrowdSimulationManager::HandlePlayerMovement(FVector PlayerLocation, bool bIsPlayerStealth)
{
    float EffectiveRadius = PlayerInfluenceRadius;
    if (bIsPlayerStealth)
    {
        EffectiveRadius *= PlayerStealthMultiplier;
    }
    
    // Create temporary influence trigger
    TriggerEmergentBehavior(TEXT("PlayerMovement"), PlayerLocation, bIsPlayerStealth ? 0.3f : 1.0f);
}

void ACrowdSimulationManager::HandleLargePredatorSighting(FVector Location, float ThreatLevel)
{
    // Large predators cause widespread panic
    TriggerEmergentBehavior(TEXT("PredatorSighting"), Location, ThreatLevel * 2.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulation: Large predator sighted! Threat level: %.2f"), ThreatLevel);
}

void ACrowdSimulationManager::HandleHerdMovement(FVector HerdCenter, FVector Direction, int32 HerdSize)
{
    float HerdInfluence = FMath::Clamp(HerdSize / 50.0f, 0.5f, 3.0f);
    TriggerEmergentBehavior(TEXT("HerdMovement"), HerdCenter, HerdInfluence);
}

void ACrowdSimulationManager::SimulatePredatorPreyInteractions()
{
    // This would implement complex predator-prey dynamics
    // For now, log the simulation
    UE_LOG(LogTemp, Verbose, TEXT("CrowdSimulation: Simulating predator-prey interactions"));
}

void ACrowdSimulationManager::UpdateMigrationPatterns()
{
    // Seasonal migration patterns based on time and environment
    UE_LOG(LogTemp, Verbose, TEXT("CrowdSimulation: Updating migration patterns"));
}

void ACrowdSimulationManager::HandleWeatherEffects(float RainIntensity, float WindStrength)
{
    // Weather affects animal behavior significantly
    float WeatherMultiplier = 1.0f - (RainIntensity * 0.3f) - (WindStrength * 0.2f);
    WeatherMultiplier = FMath::Clamp(WeatherMultiplier, 0.2f, 1.0f);
    
    // Apply weather effects to all biomes
    for (FBiomePopulationData& BiomeData : BiomePopulations)
    {
        BiomeData.ActivityMultiplier *= WeatherMultiplier;
    }
    
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulation: Weather effects applied - Rain: %.2f, Wind: %.2f"), 
           RainIntensity, WindStrength);
}

void ACrowdSimulationManager::UpdateTimeOfDay(float DeltaTime)
{
    TimeOfDayTimer += DeltaTime;
    
    // Each time period lasts DayDurationMinutes / 6
    float TimePerPeriod = (DayDurationMinutes * 60.0f) / 6.0f;
    
    if (TimeOfDayTimer >= TimePerPeriod)
    {
        int32 NextTime = (static_cast<int32>(CurrentTimeOfDay) + 1) % 6;
        SetTimeOfDay(static_cast<ETimeOfDay>(NextTime));
        TimeOfDayTimer = 0.0f;
    }
}

void ACrowdSimulationManager::ProcessEmergentTriggers(float DeltaTime)
{
    for (int32 i = ActiveTriggers.Num() - 1; i >= 0; i--)
    {
        ActiveTriggers[i].DurationSeconds -= DeltaTime;
        
        if (ActiveTriggers[i].DurationSeconds <= 0.0f)
        {
            UE_LOG(LogTemp, Verbose, TEXT("CrowdSimulation: Emergent trigger '%s' expired"), 
                   *ActiveTriggers[i].TriggerName);
            ActiveTriggers.RemoveAt(i);
        }
    }
}

void ACrowdSimulationManager::OptimizePerformance()
{
    // Performance optimization based on frame time
    if (LastFrameTime > 0.033f) // If frame time > 33ms (30 FPS)
    {
        // Reduce population slightly
        MaxTotalEntities = FMath::Max(MaxTotalEntities * 0.95f, 10000);
        UE_LOG(LogTemp, Warning, TEXT("CrowdSimulation: Performance optimization - reducing max entities to %d"), 
               MaxTotalEntities);
    }
    else if (LastFrameTime < 0.016f && CurrentActiveEntities < MaxTotalEntities * 0.8f) // If frame time < 16ms (60+ FPS)
    {
        // Can increase population
        MaxTotalEntities = FMath::Min(MaxTotalEntities * 1.02f, 50000);
    }
}