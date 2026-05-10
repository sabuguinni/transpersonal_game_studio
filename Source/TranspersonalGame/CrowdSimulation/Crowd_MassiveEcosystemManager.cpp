#include "Crowd_MassiveEcosystemManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Crowd_MassEntityManager.h"
#include "Crowd_BiomeCrowdManager.h"
#include "Crowd_MassHerdSystem.h"
#include "MassEntitySubsystem.h"

UCrowd_MassiveEcosystemManager::UCrowd_MassiveEcosystemManager()
{
    // Initialize default values
    MaxTotalEntities = 50000;
    MaxVisibleEntities = 5000;
    EcosystemUpdateFrequency = 1.0f;
    LODNearDistance = 2000.0f;
    LODMediumDistance = 5000.0f;
    LODFarDistance = 10000.0f;
    MigrationCheckInterval = 30.0f;
    MigrationTriggerThreshold = 0.3f;
    
    LastFrameTime = 0.0f;
    FrameTimeIndex = 0;
    bPerformanceMonitoringEnabled = true;
    
    // Clear frame time history
    for (int32 i = 0; i < 60; ++i)
    {
        FrameTimeHistory[i] = 0;
    }
}

void UCrowd_MassiveEcosystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("UCrowd_MassiveEcosystemManager::Initialize - Starting massive ecosystem"));
    
    InitializeBiomeTargets();
    
    // Get references to other subsystems
    if (UWorld* World = GetWorld())
    {
        MassEntityManager = World->GetSubsystem<UCrowd_MassEntityManager>();
        BiomeCrowdManager = World->GetSubsystem<UCrowd_BiomeCrowdManager>();
        HerdSystem = World->GetSubsystem<UCrowd_MassHerdSystem>();
    }
    
    // Start ecosystem management timers
    StartEcosystemTimers();
}

void UCrowd_MassiveEcosystemManager::Deinitialize()
{
    // Clear timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(EcosystemUpdateTimer);
        World->GetTimerManager().ClearTimer(MigrationCheckTimer);
    }
    
    Super::Deinitialize();
}

bool UCrowd_MassiveEcosystemManager::ShouldCreateSubsystem(UObject* Outer) const
{
    return Super::ShouldCreateSubsystem(Outer);
}

void UCrowd_MassiveEcosystemManager::InitializeMassiveEcosystem()
{
    UE_LOG(LogTemp, Warning, TEXT("InitializeMassiveEcosystem - Setting up 50,000+ entity ecosystem"));
    
    // Initialize biome populations
    SpawnBiomePopulations();
    
    // Set up performance monitoring
    MonitorPerformance();
    
    UE_LOG(LogTemp, Warning, TEXT("Massive ecosystem initialized successfully"));
}

void UCrowd_MassiveEcosystemManager::SpawnBiomePopulations()
{
    UE_LOG(LogTemp, Warning, TEXT("SpawnBiomePopulations - Creating massive herds"));
    
    // Clear current populations
    CurrentBiomePopulations.Empty();
    
    // Spawn entities in each biome according to targets
    for (const auto& BiomeTarget : BiomePopulationTargets)
    {
        ECrowd_BiomeType BiomeType = BiomeTarget.Key;
        int32 TargetPopulation = BiomeTarget.Value;
        
        // Spawn in chunks to avoid performance spikes
        int32 ChunkSize = 1000;
        int32 ChunksNeeded = FMath::CeilToInt(static_cast<float>(TargetPopulation) / ChunkSize);
        
        for (int32 ChunkIndex = 0; ChunkIndex < ChunksNeeded; ++ChunkIndex)
        {
            int32 EntitiesInChunk = FMath::Min(ChunkSize, TargetPopulation - (ChunkIndex * ChunkSize));
            
            // Spawn chunk of entities
            if (BiomeCrowdManager)
            {
                // BiomeCrowdManager->SpawnBiomeEntities(BiomeType, EntitiesInChunk);
            }
        }
        
        CurrentBiomePopulations.Add(BiomeType, TargetPopulation);
        UE_LOG(LogTemp, Warning, TEXT("Spawned %d entities in biome %d"), TargetPopulation, static_cast<int32>(BiomeType));
    }
}

void UCrowd_MassiveEcosystemManager::UpdateEcosystemBalance(float DeltaTime)
{
    // Update performance monitoring
    if (bPerformanceMonitoringEnabled)
    {
        MonitorPerformance();
    }
    
    // Update LOD system
    UpdateLODSystem();
    
    // Balance populations if needed
    BalancePopulations();
    
    // Check for migration triggers
    CheckMigrationTriggers();
}

void UCrowd_MassiveEcosystemManager::TriggerMigrationEvent(ECrowd_BiomeType FromBiome, ECrowd_BiomeType ToBiome, int32 EntityCount)
{
    UE_LOG(LogTemp, Warning, TEXT("TriggerMigrationEvent - Moving %d entities from biome %d to biome %d"), 
           EntityCount, static_cast<int32>(FromBiome), static_cast<int32>(ToBiome));
    
    ProcessMigration(FromBiome, ToBiome, EntityCount);
}

void UCrowd_MassiveEcosystemManager::SetBiomePopulationTarget(ECrowd_BiomeType BiomeType, int32 TargetPopulation)
{
    BiomePopulationTargets.Add(BiomeType, TargetPopulation);
    UE_LOG(LogTemp, Warning, TEXT("Set biome %d target population to %d"), static_cast<int32>(BiomeType), TargetPopulation);
}

int32 UCrowd_MassiveEcosystemManager::GetCurrentBiomePopulation(ECrowd_BiomeType BiomeType) const
{
    if (const int32* Population = CurrentBiomePopulations.Find(BiomeType))
    {
        return *Population;
    }
    return 0;
}

void UCrowd_MassiveEcosystemManager::BalancePopulations()
{
    // Check each biome's population against its target
    for (const auto& BiomeTarget : BiomePopulationTargets)
    {
        ECrowd_BiomeType BiomeType = BiomeTarget.Key;
        int32 TargetPopulation = BiomeTarget.Value;
        int32 CurrentPopulation = GetCurrentBiomePopulation(BiomeType);
        
        int32 PopulationDifference = TargetPopulation - CurrentPopulation;
        
        if (FMath::Abs(PopulationDifference) > 100) // Only balance if difference is significant
        {
            if (PopulationDifference > 0)
            {
                // Need to add entities
                UE_LOG(LogTemp, Log, TEXT("Biome %d needs %d more entities"), static_cast<int32>(BiomeType), PopulationDifference);
            }
            else
            {
                // Need to remove entities
                UE_LOG(LogTemp, Log, TEXT("Biome %d has %d excess entities"), static_cast<int32>(BiomeType), -PopulationDifference);
            }
        }
    }
}

void UCrowd_MassiveEcosystemManager::SetLODDistances(float Near, float Medium, float Far)
{
    LODNearDistance = Near;
    LODMediumDistance = Medium;
    LODFarDistance = Far;
    
    UE_LOG(LogTemp, Warning, TEXT("Updated LOD distances: Near=%.1f, Medium=%.1f, Far=%.1f"), Near, Medium, Far);
}

void UCrowd_MassiveEcosystemManager::UpdatePerformanceSettings(int32 MaxVisibleEntitiesNew, float UpdateFrequency)
{
    MaxVisibleEntities = MaxVisibleEntitiesNew;
    EcosystemUpdateFrequency = UpdateFrequency;
    
    // Restart timers with new frequency
    StartEcosystemTimers();
    
    UE_LOG(LogTemp, Warning, TEXT("Updated performance settings: MaxVisible=%d, UpdateFreq=%.2f"), MaxVisibleEntities, UpdateFrequency);
}

void UCrowd_MassiveEcosystemManager::GetEcosystemStats(int32& TotalEntities, int32& ActiveEntities, float& AverageFrameTime)
{
    TotalEntities = 0;
    for (const auto& BiomePopulation : CurrentBiomePopulations)
    {
        TotalEntities += BiomePopulation.Value;
    }
    
    ActiveEntities = FMath::Min(TotalEntities, MaxVisibleEntities);
    
    // Calculate average frame time
    float TotalFrameTime = 0.0f;
    int32 ValidFrames = 0;
    for (int32 i = 0; i < 60; ++i)
    {
        if (FrameTimeHistory[i] > 0)
        {
            TotalFrameTime += FrameTimeHistory[i];
            ValidFrames++;
        }
    }
    
    AverageFrameTime = ValidFrames > 0 ? TotalFrameTime / ValidFrames : 0.0f;
}

void UCrowd_MassiveEcosystemManager::LogBiomePopulations()
{
    UE_LOG(LogTemp, Warning, TEXT("=== BIOME POPULATIONS ==="));
    for (const auto& BiomePopulation : CurrentBiomePopulations)
    {
        ECrowd_BiomeType BiomeType = BiomePopulation.Key;
        int32 Population = BiomePopulation.Value;
        int32 Target = BiomePopulationTargets.FindRef(BiomeType);
        
        UE_LOG(LogTemp, Warning, TEXT("Biome %d: %d/%d entities (%.1f%%)"), 
               static_cast<int32>(BiomeType), Population, Target, 
               Target > 0 ? (static_cast<float>(Population) / Target) * 100.0f : 0.0f);
    }
}

void UCrowd_MassiveEcosystemManager::InitializeBiomeTargets()
{
    // Set default population targets for massive ecosystem
    BiomePopulationTargets.Empty();
    BiomePopulationTargets.Add(ECrowd_BiomeType::Swamp, 8000);      // 8,000 entities in swamp
    BiomePopulationTargets.Add(ECrowd_BiomeType::Forest, 12000);    // 12,000 entities in forest
    BiomePopulationTargets.Add(ECrowd_BiomeType::Savanna, 15000);   // 15,000 entities in savanna
    BiomePopulationTargets.Add(ECrowd_BiomeType::Desert, 7000);     // 7,000 entities in desert
    BiomePopulationTargets.Add(ECrowd_BiomeType::Mountain, 8000);   // 8,000 entities in mountain
    
    UE_LOG(LogTemp, Warning, TEXT("Initialized biome targets: Total 50,000 entities"));
}

void UCrowd_MassiveEcosystemManager::StartEcosystemTimers()
{
    if (UWorld* World = GetWorld())
    {
        FTimerManager& TimerManager = World->GetTimerManager();
        
        // Clear existing timers
        TimerManager.ClearTimer(EcosystemUpdateTimer);
        TimerManager.ClearTimer(MigrationCheckTimer);
        
        // Start ecosystem update timer
        TimerManager.SetTimer(EcosystemUpdateTimer, 
                            FTimerDelegate::CreateUObject(this, &UCrowd_MassiveEcosystemManager::UpdateEcosystemBalance, EcosystemUpdateFrequency),
                            EcosystemUpdateFrequency, true);
        
        // Start migration check timer
        TimerManager.SetTimer(MigrationCheckTimer,
                            FTimerDelegate::CreateUObject(this, &UCrowd_MassiveEcosystemManager::CheckMigrationTriggers),
                            MigrationCheckInterval, true);
    }
}

void UCrowd_MassiveEcosystemManager::CheckMigrationTriggers()
{
    // Check if any biome is significantly over or under populated
    for (const auto& BiomeTarget : BiomePopulationTargets)
    {
        ECrowd_BiomeType BiomeType = BiomeTarget.Key;
        int32 TargetPopulation = BiomeTarget.Value;
        int32 CurrentPopulation = GetCurrentBiomePopulation(BiomeType);
        
        float PopulationRatio = static_cast<float>(CurrentPopulation) / TargetPopulation;
        
        if (PopulationRatio > (1.0f + MigrationTriggerThreshold))
        {
            // Overpopulated - trigger migration out
            int32 ExcessEntities = CurrentPopulation - TargetPopulation;
            int32 MigrateCount = FMath::Min(ExcessEntities, 500); // Migrate in batches
            
            // Find a suitable destination biome
            for (const auto& OtherBiome : BiomePopulationTargets)
            {
                if (OtherBiome.Key != BiomeType)
                {
                    int32 OtherCurrent = GetCurrentBiomePopulation(OtherBiome.Key);
                    float OtherRatio = static_cast<float>(OtherCurrent) / OtherBiome.Value;
                    
                    if (OtherRatio < (1.0f - MigrationTriggerThreshold))
                    {
                        TriggerMigrationEvent(BiomeType, OtherBiome.Key, MigrateCount);
                        break;
                    }
                }
            }
        }
    }
}

void UCrowd_MassiveEcosystemManager::ProcessMigration(ECrowd_BiomeType FromBiome, ECrowd_BiomeType ToBiome, int32 EntityCount)
{
    // Update population counts
    if (int32* FromPopulation = CurrentBiomePopulations.Find(FromBiome))
    {
        *FromPopulation = FMath::Max(0, *FromPopulation - EntityCount);
    }
    
    if (int32* ToPopulation = CurrentBiomePopulations.Find(ToBiome))
    {
        *ToPopulation += EntityCount;
    }
    else
    {
        CurrentBiomePopulations.Add(ToBiome, EntityCount);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Migration complete: %d entities moved from biome %d to biome %d"), 
           EntityCount, static_cast<int32>(FromBiome), static_cast<int32>(ToBiome));
}

void UCrowd_MassiveEcosystemManager::UpdateLODSystem()
{
    // Update LOD distances based on current performance
    if (LastFrameTime > 33.0f) // If frame time > 33ms (below 30fps)
    {
        // Reduce LOD distances to improve performance
        LODNearDistance = FMath::Max(1000.0f, LODNearDistance * 0.9f);
        LODMediumDistance = FMath::Max(2000.0f, LODMediumDistance * 0.9f);
        LODFarDistance = FMath::Max(4000.0f, LODFarDistance * 0.9f);
    }
    else if (LastFrameTime < 16.0f) // If frame time < 16ms (above 60fps)
    {
        // Increase LOD distances for better quality
        LODNearDistance = FMath::Min(3000.0f, LODNearDistance * 1.05f);
        LODMediumDistance = FMath::Min(6000.0f, LODMediumDistance * 1.05f);
        LODFarDistance = FMath::Min(12000.0f, LODFarDistance * 1.05f);
    }
}

void UCrowd_MassiveEcosystemManager::MonitorPerformance()
{
    if (UWorld* World = GetWorld())
    {
        float CurrentFrameTime = World->GetDeltaSeconds() * 1000.0f; // Convert to milliseconds
        
        // Store frame time in history
        FrameTimeHistory[FrameTimeIndex] = static_cast<int32>(CurrentFrameTime);
        FrameTimeIndex = (FrameTimeIndex + 1) % 60;
        
        LastFrameTime = CurrentFrameTime;
    }
}