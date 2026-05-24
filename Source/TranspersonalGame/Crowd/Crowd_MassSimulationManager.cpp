#include "Crowd_MassSimulationManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassSimulationSubsystem.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassRepresentationFragments.h"

UCrowd_MassSimulationManager::UCrowd_MassSimulationManager()
{
    MassEntitySubsystem = nullptr;
    bSimulationActive = false;
    bSimulationPaused = false;
    bDebugVisualization = false;
    MaxTotalEntities = 5000;
    SimulationTickRate = 60.0f;
    bUseAdaptiveLOD = true;
    LODDistance1 = 5000.0f;
    LODDistance2 = 15000.0f;
    LODDistance3 = 30000.0f;
    LastStatsUpdateTime = 0.0f;
}

void UCrowd_MassSimulationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("UCrowd_MassSimulationManager::Initialize - Starting crowd simulation subsystem"));
    
    // Get Mass Entity Subsystem
    if (UWorld* World = GetWorld())
    {
        MassEntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
        if (!MassEntitySubsystem)
        {
            UE_LOG(LogTemp, Error, TEXT("UCrowd_MassSimulationManager: Failed to get MassEntitySubsystem"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("UCrowd_MassSimulationManager: MassEntitySubsystem acquired successfully"));
        }
    }
    
    // Initialize default configurations
    InitializeDefaultConfigurations();
    
    // Setup Mass Entity processors
    SetupMassEntityProcessors();
    
    UE_LOG(LogTemp, Warning, TEXT("UCrowd_MassSimulationManager: Initialization complete"));
}

void UCrowd_MassSimulationManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("UCrowd_MassSimulationManager::Deinitialize - Shutting down crowd simulation"));
    
    StopSimulation();
    
    // Clear all entities
    ActiveEntities.Empty();
    EntitiesByBiome.Empty();
    
    MassEntitySubsystem = nullptr;
    
    Super::Deinitialize();
}

bool UCrowd_MassSimulationManager::ShouldCreateSubsystem(UObject* Outer) const
{
    return true;
}

void UCrowd_MassSimulationManager::InitializeMassSimulation()
{
    UE_LOG(LogTemp, Warning, TEXT("UCrowd_MassSimulationManager::InitializeMassSimulation - Setting up Mass Entity simulation"));
    
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("InitializeMassSimulation: MassEntitySubsystem is null"));
        return;
    }
    
    // Clear existing entities
    ActiveEntities.Empty();
    EntitiesByBiome.Empty();
    
    // Reset stats
    CurrentStats = FCrowd_SimulationStats();
    
    UE_LOG(LogTemp, Warning, TEXT("Mass simulation initialized with %d max entities"), MaxTotalEntities);
}

void UCrowd_MassSimulationManager::StartSimulation()
{
    UE_LOG(LogTemp, Warning, TEXT("UCrowd_MassSimulationManager::StartSimulation - Starting crowd simulation"));
    
    if (bSimulationActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Simulation already active"));
        return;
    }
    
    InitializeMassSimulation();
    
    // Spawn initial entities in each configured biome
    for (const auto& BiomeConfig : BiomeConfigs)
    {
        EBiomeType BiomeType = BiomeConfig.Key;
        const FCrowd_BiomeSpawnConfig& Config = BiomeConfig.Value;
        
        int32 EntitiesToSpawn = FMath::Min(Config.MaxEntities, MaxTotalEntities / BiomeConfigs.Num());
        SpawnEntitiesInBiome(BiomeType, EntitiesToSpawn);
    }
    
    bSimulationActive = true;
    bSimulationPaused = false;
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd simulation started with %d total entities"), ActiveEntities.Num());
}

void UCrowd_MassSimulationManager::StopSimulation()
{
    UE_LOG(LogTemp, Warning, TEXT("UCrowd_MassSimulationManager::StopSimulation - Stopping crowd simulation"));
    
    if (!bSimulationActive)
    {
        return;
    }
    
    // Destroy all active entities
    if (MassEntitySubsystem)
    {
        for (const FMassEntityHandle& EntityHandle : ActiveEntities)
        {
            if (MassEntitySubsystem->IsEntityValid(EntityHandle))
            {
                MassEntitySubsystem->DestroyEntity(EntityHandle);
            }
        }
    }
    
    ActiveEntities.Empty();
    EntitiesByBiome.Empty();
    
    bSimulationActive = false;
    bSimulationPaused = false;
    
    // Reset stats
    CurrentStats = FCrowd_SimulationStats();
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd simulation stopped"));
}

void UCrowd_MassSimulationManager::PauseSimulation()
{
    UE_LOG(LogTemp, Warning, TEXT("UCrowd_MassSimulationManager::PauseSimulation"));
    
    if (bSimulationActive && !bSimulationPaused)
    {
        bSimulationPaused = true;
        UE_LOG(LogTemp, Warning, TEXT("Crowd simulation paused"));
    }
}

void UCrowd_MassSimulationManager::ResumeSimulation()
{
    UE_LOG(LogTemp, Warning, TEXT("UCrowd_MassSimulationManager::ResumeSimulation"));
    
    if (bSimulationActive && bSimulationPaused)
    {
        bSimulationPaused = false;
        UE_LOG(LogTemp, Warning, TEXT("Crowd simulation resumed"));
    }
}

void UCrowd_MassSimulationManager::ConfigureBiomeSpawning(EBiomeType BiomeType, const FCrowd_BiomeSpawnConfig& Config)
{
    UE_LOG(LogTemp, Warning, TEXT("UCrowd_MassSimulationManager::ConfigureBiomeSpawning - Configuring biome: %d"), (int32)BiomeType);
    
    BiomeConfigs.Add(BiomeType, Config);
    
    UE_LOG(LogTemp, Warning, TEXT("Biome %d configured: Center(%f,%f,%f), Radius=%f, MaxEntities=%d"), 
           (int32)BiomeType, Config.SpawnCenter.X, Config.SpawnCenter.Y, Config.SpawnCenter.Z, 
           Config.SpawnRadius, Config.MaxEntities);
}

void UCrowd_MassSimulationManager::SpawnEntitiesInBiome(EBiomeType BiomeType, int32 EntityCount)
{
    UE_LOG(LogTemp, Warning, TEXT("UCrowd_MassSimulationManager::SpawnEntitiesInBiome - Spawning %d entities in biome %d"), EntityCount, (int32)BiomeType);
    
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("SpawnEntitiesInBiome: MassEntitySubsystem is null"));
        return;
    }
    
    const FCrowd_BiomeSpawnConfig* BiomeConfig = BiomeConfigs.Find(BiomeType);
    if (!BiomeConfig)
    {
        UE_LOG(LogTemp, Error, TEXT("SpawnEntitiesInBiome: No configuration found for biome %d"), (int32)BiomeType);
        return;
    }
    
    // Ensure we don't exceed total entity limit
    int32 CurrentTotalEntities = ActiveEntities.Num();
    int32 ActualSpawnCount = FMath::Min(EntityCount, MaxTotalEntities - CurrentTotalEntities);
    
    if (ActualSpawnCount <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("SpawnEntitiesInBiome: Cannot spawn entities, limit reached"));
        return;
    }
    
    TArray<FMassEntityHandle>& BiomeEntities = EntitiesByBiome.FindOrAdd(BiomeType);
    
    for (int32 i = 0; i < ActualSpawnCount; i++)
    {
        // Create entity
        FMassEntityHandle NewEntity = MassEntitySubsystem->CreateEntity();
        
        if (!MassEntitySubsystem->IsEntityValid(NewEntity))
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to create entity %d in biome %d"), i, (int32)BiomeType);
            continue;
        }
        
        // Get spawn location
        FVector SpawnLocation = GetRandomSpawnLocationInBiome(BiomeType);
        
        // Add transform fragment
        FMassTransformFragment TransformFragment;
        TransformFragment.SetTransform(FTransform(FRotator::ZeroRotator, SpawnLocation, FVector::OneVector));
        MassEntitySubsystem->AddFragmentToEntity(NewEntity, TransformFragment);
        
        // Add velocity fragment
        FMassVelocityFragment VelocityFragment;
        VelocityFragment.Value = FVector::ZeroVector;
        MassEntitySubsystem->AddFragmentToEntity(NewEntity, VelocityFragment);
        
        // Store entity references
        ActiveEntities.Add(NewEntity);
        BiomeEntities.Add(NewEntity);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Successfully spawned %d entities in biome %d. Total active: %d"), 
           ActualSpawnCount, (int32)BiomeType, ActiveEntities.Num());
    
    // Update stats
    UpdateSimulationStats();
}

void UCrowd_MassSimulationManager::ClearEntitiesInBiome(EBiomeType BiomeType)
{
    UE_LOG(LogTemp, Warning, TEXT("UCrowd_MassSimulationManager::ClearEntitiesInBiome - Clearing entities in biome %d"), (int32)BiomeType);
    
    TArray<FMassEntityHandle>* BiomeEntities = EntitiesByBiome.Find(BiomeType);
    if (!BiomeEntities || !MassEntitySubsystem)
    {
        return;
    }
    
    int32 ClearedCount = 0;
    for (const FMassEntityHandle& EntityHandle : *BiomeEntities)
    {
        if (MassEntitySubsystem->IsEntityValid(EntityHandle))
        {
            MassEntitySubsystem->DestroyEntity(EntityHandle);
            ActiveEntities.Remove(EntityHandle);
            ClearedCount++;
        }
    }
    
    BiomeEntities->Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("Cleared %d entities from biome %d"), ClearedCount, (int32)BiomeType);
    
    UpdateSimulationStats();
}

void UCrowd_MassSimulationManager::RegisterEntityArchetype(ECrowdEntityType EntityType, const FCrowd_EntityArchetype& Archetype)
{
    UE_LOG(LogTemp, Warning, TEXT("UCrowd_MassSimulationManager::RegisterEntityArchetype - Registering archetype for entity type %d"), (int32)EntityType);
    
    EntityArchetypes.Add(EntityType, Archetype);
}

FCrowd_EntityArchetype UCrowd_MassSimulationManager::GetEntityArchetype(ECrowdEntityType EntityType) const
{
    const FCrowd_EntityArchetype* FoundArchetype = EntityArchetypes.Find(EntityType);
    if (FoundArchetype)
    {
        return *FoundArchetype;
    }
    
    // Return default archetype
    return FCrowd_EntityArchetype();
}

void UCrowd_MassSimulationManager::SetMaxEntityCount(int32 MaxCount)
{
    UE_LOG(LogTemp, Warning, TEXT("UCrowd_MassSimulationManager::SetMaxEntityCount - Setting max entities to %d"), MaxCount);
    
    MaxTotalEntities = FMath::Max(0, MaxCount);
    
    // If we currently have more entities than the new limit, remove excess
    if (ActiveEntities.Num() > MaxTotalEntities)
    {
        int32 ExcessCount = ActiveEntities.Num() - MaxTotalEntities;
        UE_LOG(LogTemp, Warning, TEXT("Removing %d excess entities"), ExcessCount);
        
        if (MassEntitySubsystem)
        {
            for (int32 i = 0; i < ExcessCount; i++)
            {
                int32 LastIndex = ActiveEntities.Num() - 1;
                if (LastIndex >= 0)
                {
                    FMassEntityHandle EntityToRemove = ActiveEntities[LastIndex];
                    if (MassEntitySubsystem->IsEntityValid(EntityToRemove))
                    {
                        MassEntitySubsystem->DestroyEntity(EntityToRemove);
                    }
                    ActiveEntities.RemoveAt(LastIndex);
                }
            }
        }
        
        UpdateSimulationStats();
    }
}

void UCrowd_MassSimulationManager::SetSimulationQuality(float QualityLevel)
{
    UE_LOG(LogTemp, Warning, TEXT("UCrowd_MassSimulationManager::SetSimulationQuality - Setting quality to %f"), QualityLevel);
    
    float ClampedQuality = FMath::Clamp(QualityLevel, 0.1f, 1.0f);
    SimulationTickRate = 60.0f * ClampedQuality;
    
    // Adjust LOD distances based on quality
    float QualityMultiplier = ClampedQuality;
    LODDistance1 = 5000.0f * QualityMultiplier;
    LODDistance2 = 15000.0f * QualityMultiplier;
    LODDistance3 = 30000.0f * QualityMultiplier;
}

void UCrowd_MassSimulationManager::EnableLODSystem(bool bEnabled)
{
    UE_LOG(LogTemp, Warning, TEXT("UCrowd_MassSimulationManager::EnableLODSystem - %s"), bEnabled ? TEXT("Enabled") : TEXT("Disabled"));
    
    bUseAdaptiveLOD = bEnabled;
}

FCrowd_SimulationStats UCrowd_MassSimulationManager::GetSimulationStats() const
{
    return CurrentStats;
}

int32 UCrowd_MassSimulationManager::GetEntityCountInBiome(EBiomeType BiomeType) const
{
    const TArray<FMassEntityHandle>* BiomeEntities = EntitiesByBiome.Find(BiomeType);
    return BiomeEntities ? BiomeEntities->Num() : 0;
}

float UCrowd_MassSimulationManager::GetCurrentFrameTime() const
{
    return CurrentStats.AverageFrameTime;
}

void UCrowd_MassSimulationManager::EnableDebugVisualization(bool bEnabled)
{
    UE_LOG(LogTemp, Warning, TEXT("UCrowd_MassSimulationManager::EnableDebugVisualization - %s"), bEnabled ? TEXT("Enabled") : TEXT("Disabled"));
    
    bDebugVisualization = bEnabled;
}

void UCrowd_MassSimulationManager::DrawDebugInfo()
{
    if (!bDebugVisualization || !GetWorld())
    {
        return;
    }
    
    // Draw biome boundaries
    for (const auto& BiomeConfig : BiomeConfigs)
    {
        const FCrowd_BiomeSpawnConfig& Config = BiomeConfig.Value;
        FColor BiomeColor = FColor::Green;
        
        switch (BiomeConfig.Key)
        {
        case EBiomeType::Savana:
            BiomeColor = FColor::Yellow;
            break;
        case EBiomeType::Forest:
            BiomeColor = FColor::Green;
            break;
        case EBiomeType::Desert:
            BiomeColor = FColor::Orange;
            break;
        case EBiomeType::Swamp:
            BiomeColor = FColor::Purple;
            break;
        case EBiomeType::Mountain:
            BiomeColor = FColor::Blue;
            break;
        }
        
        DrawDebugSphere(GetWorld(), Config.SpawnCenter, Config.SpawnRadius, 32, BiomeColor, false, 0.1f, 0, 50.0f);
    }
}

void UCrowd_MassSimulationManager::TestSpawnEntities()
{
    UE_LOG(LogTemp, Warning, TEXT("UCrowd_MassSimulationManager::TestSpawnEntities - Testing entity spawn"));
    
    // Configure test biome (Savana)
    FCrowd_BiomeSpawnConfig TestConfig;
    TestConfig.BiomeType = EBiomeType::Savana;
    TestConfig.SpawnCenter = FVector(0.0f, 0.0f, 100.0f);
    TestConfig.SpawnRadius = 5000.0f;
    TestConfig.MaxEntities = 100;
    TestConfig.EntityDensity = 0.1f;
    
    ConfigureBiomeSpawning(EBiomeType::Savana, TestConfig);
    
    // Initialize and start simulation
    InitializeMassSimulation();
    SpawnEntitiesInBiome(EBiomeType::Savana, 50);
    
    UE_LOG(LogTemp, Warning, TEXT("Test spawn completed. Active entities: %d"), ActiveEntities.Num());
}

void UCrowd_MassSimulationManager::InitializeDefaultConfigurations()
{
    UE_LOG(LogTemp, Warning, TEXT("UCrowd_MassSimulationManager::InitializeDefaultConfigurations"));
    
    // Configure default biomes with correct coordinates from memory
    FCrowd_BiomeSpawnConfig SavanaConfig;
    SavanaConfig.BiomeType = EBiomeType::Savana;
    SavanaConfig.SpawnCenter = FVector(0.0f, 0.0f, 100.0f);
    SavanaConfig.SpawnRadius = 10000.0f;
    SavanaConfig.MaxEntities = 1500;
    SavanaConfig.AllowedEntityTypes = {ECrowdEntityType::Herbivore, ECrowdEntityType::LargeCarnivore};
    BiomeConfigs.Add(EBiomeType::Savana, SavanaConfig);
    
    FCrowd_BiomeSpawnConfig ForestConfig;
    ForestConfig.BiomeType = EBiomeType::Forest;
    ForestConfig.SpawnCenter = FVector(-45000.0f, 40000.0f, 100.0f);
    ForestConfig.SpawnRadius = 8000.0f;
    ForestConfig.MaxEntities = 1200;
    ForestConfig.AllowedEntityTypes = {ECrowdEntityType::Herbivore, ECrowdEntityType::SmallCarnivore};
    BiomeConfigs.Add(EBiomeType::Forest, ForestConfig);
    
    FCrowd_BiomeSpawnConfig DesertConfig;
    DesertConfig.BiomeType = EBiomeType::Desert;
    DesertConfig.SpawnCenter = FVector(55000.0f, 0.0f, 100.0f);
    DesertConfig.SpawnRadius = 12000.0f;
    DesertConfig.MaxEntities = 800;
    DesertConfig.AllowedEntityTypes = {ECrowdEntityType::SmallCarnivore};
    BiomeConfigs.Add(EBiomeType::Desert, DesertConfig);
    
    FCrowd_BiomeSpawnConfig SwampConfig;
    SwampConfig.BiomeType = EBiomeType::Swamp;
    SwampConfig.SpawnCenter = FVector(-50000.0f, -45000.0f, 100.0f);
    SwampConfig.SpawnRadius = 7000.0f;
    SwampConfig.MaxEntities = 1000;
    SwampConfig.AllowedEntityTypes = {ECrowdEntityType::Herbivore, ECrowdEntityType::Aquatic};
    BiomeConfigs.Add(EBiomeType::Swamp, SwampConfig);
    
    FCrowd_BiomeSpawnConfig MountainConfig;
    MountainConfig.BiomeType = EBiomeType::Mountain;
    MountainConfig.SpawnCenter = FVector(40000.0f, 50000.0f, 500.0f);
    MountainConfig.SpawnRadius = 9000.0f;
    MountainConfig.MaxEntities = 600;
    MountainConfig.AllowedEntityTypes = {ECrowdEntityType::Flying};
    BiomeConfigs.Add(EBiomeType::Mountain, MountainConfig);
    
    // Register default entity archetypes
    FCrowd_EntityArchetype HerbivoreArchetype;
    HerbivoreArchetype.EntityType = ECrowdEntityType::Herbivore;
    HerbivoreArchetype.MovementSpeed = 250.0f;
    HerbivoreArchetype.DetectionRadius = 1200.0f;
    HerbivoreArchetype.FlockingRadius = 600.0f;
    HerbivoreArchetype.bCanAttack = false;
    EntityArchetypes.Add(ECrowdEntityType::Herbivore, HerbivoreArchetype);
    
    FCrowd_EntityArchetype CarnivoreArchetype;
    CarnivoreArchetype.EntityType = ECrowdEntityType::SmallCarnivore;
    CarnivoreArchetype.MovementSpeed = 400.0f;
    CarnivoreArchetype.DetectionRadius = 2000.0f;
    CarnivoreArchetype.FlockingRadius = 300.0f;
    CarnivoreArchetype.bCanAttack = true;
    CarnivoreArchetype.AttackDamage = 25.0f;
    EntityArchetypes.Add(ECrowdEntityType::SmallCarnivore, CarnivoreArchetype);
    
    UE_LOG(LogTemp, Warning, TEXT("Default configurations initialized for %d biomes"), BiomeConfigs.Num());
}

void UCrowd_MassSimulationManager::SetupMassEntityProcessors()
{
    UE_LOG(LogTemp, Warning, TEXT("UCrowd_MassSimulationManager::SetupMassEntityProcessors"));
    
    // Mass Entity processors will be configured here
    // This is where we would set up movement, flocking, and behavior processors
}

void UCrowd_MassSimulationManager::UpdateSimulationStats()
{
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    if (CurrentTime - LastStatsUpdateTime < StatsUpdateInterval)
    {
        return;
    }
    
    LastStatsUpdateTime = CurrentTime;
    
    // Update entity counts
    CurrentStats.TotalActiveEntities = ActiveEntities.Num();
    CurrentStats.EntitiesInSavana = GetEntityCountInBiome(EBiomeType::Savana);
    CurrentStats.EntitiesInForest = GetEntityCountInBiome(EBiomeType::Forest);
    CurrentStats.EntitiesInDesert = GetEntityCountInBiome(EBiomeType::Desert);
    CurrentStats.EntitiesInSwamp = GetEntityCountInBiome(EBiomeType::Swamp);
    CurrentStats.EntitiesInMountain = GetEntityCountInBiome(EBiomeType::Mountain);
    
    // Calculate simulation load
    float EntityRatio = (float)CurrentStats.TotalActiveEntities / (float)MaxTotalEntities;
    CurrentStats.SimulationLoad = EntityRatio * 100.0f;
    
    // Update frame time (simplified)
    CurrentStats.AverageFrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds
}

void UCrowd_MassSimulationManager::ProcessEntityLOD()
{
    if (!bUseAdaptiveLOD || !GetWorld())
    {
        return;
    }
    
    // LOD processing would be implemented here
    // This would adjust entity update rates based on distance from player
}

void UCrowd_MassSimulationManager::HandleEntityCulling()
{
    // Entity culling logic would be implemented here
    // Remove entities that are too far from active areas
}

FVector UCrowd_MassSimulationManager::GetRandomSpawnLocationInBiome(EBiomeType BiomeType) const
{
    const FCrowd_BiomeSpawnConfig* BiomeConfig = BiomeConfigs.Find(BiomeType);
    if (!BiomeConfig)
    {
        return FVector::ZeroVector;
    }
    
    // Generate random location within biome radius
    float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
    float RandomRadius = FMath::RandRange(0.0f, BiomeConfig->SpawnRadius);
    
    FVector RandomOffset;
    RandomOffset.X = FMath::Cos(RandomAngle) * RandomRadius;
    RandomOffset.Y = FMath::Sin(RandomAngle) * RandomRadius;
    RandomOffset.Z = FMath::RandRange(-100.0f, 200.0f); // Some height variation
    
    return BiomeConfig->SpawnCenter + RandomOffset;
}

bool UCrowd_MassSimulationManager::IsLocationValidForSpawn(const FVector& Location, EBiomeType BiomeType) const
{
    // Basic validation - could be expanded with terrain checks
    const FCrowd_BiomeSpawnConfig* BiomeConfig = BiomeConfigs.Find(BiomeType);
    if (!BiomeConfig)
    {
        return false;
    }
    
    float DistanceFromCenter = FVector::Dist(Location, BiomeConfig->SpawnCenter);
    return DistanceFromCenter <= BiomeConfig->SpawnRadius;
}