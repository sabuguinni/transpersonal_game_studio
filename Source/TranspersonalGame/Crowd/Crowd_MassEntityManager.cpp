#include "Crowd_MassEntityManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassEntityTemplateRegistry.h"
#include "Components/StaticMeshComponent.h"

ACrowd_MassEntityManager::ACrowd_MassEntityManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f;

    MaxEntitiesPerBiome = 1000;
    SpawnRadius = 15000.0f;
    TotalActiveEntities = 0;

    MassEntitySubsystem = nullptr;
    MassSpawnerSubsystem = nullptr;
}

void ACrowd_MassEntityManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeMassSubsystems();
    InitializeBiomeSpawning();
    
    // Auto-spawn entities on begin play
    FTimerHandle SpawnTimer;
    GetWorld()->GetTimerManager().SetTimer(SpawnTimer, this, &ACrowd_MassEntityManager::SpawnMassEntitiesAcrossAllBiomes, 2.0f, false);
}

void ACrowd_MassEntityManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update entity positions every tick
    UpdateEntityPositions();
    
    // Cleanup invalid entities periodically
    static float CleanupTimer = 0.0f;
    CleanupTimer += DeltaTime;
    if (CleanupTimer >= 10.0f)
    {
        CleanupInvalidEntities();
        CleanupTimer = 0.0f;
    }
}

void ACrowd_MassEntityManager::InitializeMassSubsystems()
{
    if (UWorld* World = GetWorld())
    {
        MassEntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
        MassSpawnerSubsystem = World->GetSubsystem<UMassSpawnerSubsystem>();
        
        if (MassEntitySubsystem)
        {
            UE_LOG(LogTemp, Warning, TEXT("MassEntitySubsystem initialized successfully"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to get MassEntitySubsystem"));
        }
    }
}

void ACrowd_MassEntityManager::InitializeBiomeSpawning()
{
    BiomeSpawnData.Empty();
    
    // Initialize spawn data for all 5 biomes
    TArray<TPair<EBiomeType, FVector>> BiomeLocations = {
        {EBiomeType::Savanna, FVector(0, 0, 100)},
        {EBiomeType::Swamp, FVector(-50000, -45000, 100)},
        {EBiomeType::Forest, FVector(-45000, 40000, 100)},
        {EBiomeType::Desert, FVector(55000, 0, 100)},
        {EBiomeType::Mountain, FVector(40000, 50000, 100)}
    };
    
    for (const auto& BiomePair : BiomeLocations)
    {
        FCrowd_EntitySpawnData SpawnData;
        SpawnData.Location = BiomePair.Value;
        SpawnData.Rotation = FRotator::ZeroRotator;
        SpawnData.EntityType = ECrowd_EntityType::Herbivore;
        SpawnData.Count = MaxEntitiesPerBiome / 5; // Distribute evenly
        
        BiomeSpawnData.Add(SpawnData);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Initialized %d biome spawn locations"), BiomeSpawnData.Num());
}

void ACrowd_MassEntityManager::SpawnEntitiesInBiome(EBiomeType BiomeType, int32 Count)
{
    FVector BiomeCenter = GetBiomeCenter(BiomeType);
    
    for (int32 i = 0; i < Count; i++)
    {
        FCrowd_EntitySpawnData SpawnData;
        
        // Random position within biome radius
        float RandomX = FMath::RandRange(-SpawnRadius, SpawnRadius);
        float RandomY = FMath::RandRange(-SpawnRadius, SpawnRadius);
        SpawnData.Location = BiomeCenter + FVector(RandomX, RandomY, 0);
        SpawnData.Rotation = FRotator(0, FMath::RandRange(0.0f, 360.0f), 0);
        SpawnData.EntityType = (i % 3 == 0) ? ECrowd_EntityType::Predator : ECrowd_EntityType::Herbivore;
        SpawnData.Count = 1;
        
        FMassEntityHandle EntityHandle = SpawnSingleMassEntity(SpawnData);
        if (EntityHandle.IsValid())
        {
            ManagedEntities.Add(EntityHandle);
            TotalActiveEntities++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Spawned %d entities in biome at %s"), Count, *BiomeCenter.ToString());
}

void ACrowd_MassEntityManager::SpawnMassEntitiesAcrossAllBiomes()
{
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("MassEntitySubsystem not available for spawning"));
        return;
    }
    
    // Clear existing entities
    ManagedEntities.Empty();
    TotalActiveEntities = 0;
    
    // Spawn in all biomes
    TArray<EBiomeType> AllBiomes = {
        EBiomeType::Savanna,
        EBiomeType::Swamp,
        EBiomeType::Forest,
        EBiomeType::Desert,
        EBiomeType::Mountain
    };
    
    int32 EntitiesPerBiome = MaxEntitiesPerBiome / AllBiomes.Num();
    
    for (EBiomeType BiomeType : AllBiomes)
    {
        SpawnEntitiesInBiome(BiomeType, EntitiesPerBiome);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Mass entity spawning complete: %d total entities across %d biomes"), 
           TotalActiveEntities, AllBiomes.Num());
}

void ACrowd_MassEntityManager::UpdateEntityPositions()
{
    if (!MassEntitySubsystem || ManagedEntities.Num() == 0)
    {
        return;
    }
    
    // Update entity positions using Mass Entity system
    // This would typically involve fragment updates in a real Mass Entity implementation
    
    static float UpdateTimer = 0.0f;
    UpdateTimer += GetWorld()->GetDeltaSeconds();
    
    if (UpdateTimer >= 5.0f) // Update every 5 seconds
    {
        UE_LOG(LogTemp, Log, TEXT("Updating positions for %d mass entities"), ManagedEntities.Num());
        UpdateTimer = 0.0f;
    }
}

void ACrowd_MassEntityManager::CleanupInvalidEntities()
{
    if (!MassEntitySubsystem)
    {
        return;
    }
    
    int32 RemovedCount = 0;
    
    for (int32 i = ManagedEntities.Num() - 1; i >= 0; i--)
    {
        if (!ManagedEntities[i].IsValid())
        {
            ManagedEntities.RemoveAt(i);
            RemovedCount++;
            TotalActiveEntities--;
        }
    }
    
    if (RemovedCount > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cleaned up %d invalid entities. Active count: %d"), 
               RemovedCount, TotalActiveEntities);
    }
}

void ACrowd_MassEntityManager::TestMassEntitySpawning()
{
    UE_LOG(LogTemp, Warning, TEXT("Testing Mass Entity spawning system..."));
    SpawnMassEntitiesAcrossAllBiomes();
}

FVector ACrowd_MassEntityManager::GetBiomeCenter(EBiomeType BiomeType) const
{
    switch (BiomeType)
    {
        case EBiomeType::Savanna: return FVector(0, 0, 100);
        case EBiomeType::Swamp: return FVector(-50000, -45000, 100);
        case EBiomeType::Forest: return FVector(-45000, 40000, 100);
        case EBiomeType::Desert: return FVector(55000, 0, 100);
        case EBiomeType::Mountain: return FVector(40000, 50000, 100);
        default: return FVector::ZeroVector;
    }
}

FMassEntityHandle ACrowd_MassEntityManager::SpawnSingleMassEntity(const FCrowd_EntitySpawnData& SpawnData)
{
    if (!MassEntitySubsystem)
    {
        return FMassEntityHandle();
    }
    
    // In a real implementation, this would create a Mass Entity with proper fragments
    // For now, we'll create a placeholder handle
    
    // This is a simplified version - real Mass Entity spawning requires:
    // 1. Entity archetype definition
    // 2. Fragment composition
    // 3. Proper entity creation through MassEntitySubsystem
    
    UE_LOG(LogTemp, Log, TEXT("Spawning mass entity at %s with type %d"), 
           *SpawnData.Location.ToString(), (int32)SpawnData.EntityType);
    
    // Return a mock handle for now - in real implementation this would be from MassEntitySubsystem
    return FMassEntityHandle();
}