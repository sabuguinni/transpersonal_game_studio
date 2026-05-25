#include "Crowd_MassEntitySpawner.h"
#include "MassEntitySubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

ACrowd_MassEntitySpawner::ACrowd_MassEntitySpawner()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize default biome configurations
    FCrowd_BiomeSpawnConfig SavanaConfig;
    SavanaConfig.BiomeType = ECrowd_BiomeType::Savana;
    SavanaConfig.BiomeCenter = FVector(0.0f, 0.0f, 100.0f);
    SavanaConfig.SpawnRadius = 15000.0f;
    SavanaConfig.MaxEntities = 12000;
    SavanaConfig.EntityDensity = 0.8f;
    
    FCrowd_BiomeSpawnConfig ForestConfig;
    ForestConfig.BiomeType = ECrowd_BiomeType::Forest;
    ForestConfig.BiomeCenter = FVector(-45000.0f, 40000.0f, 100.0f);
    ForestConfig.SpawnRadius = 12000.0f;
    ForestConfig.MaxEntities = 10000;
    ForestConfig.EntityDensity = 0.6f;
    
    FCrowd_BiomeSpawnConfig DesertConfig;
    DesertConfig.BiomeType = ECrowd_BiomeType::Desert;
    DesertConfig.BiomeCenter = FVector(55000.0f, 0.0f, 100.0f);
    DesertConfig.SpawnRadius = 18000.0f;
    DesertConfig.MaxEntities = 8000;
    DesertConfig.EntityDensity = 0.4f;
    
    FCrowd_BiomeSpawnConfig SwampConfig;
    SwampConfig.BiomeType = ECrowd_BiomeType::Swamp;
    SwampConfig.BiomeCenter = FVector(-50000.0f, -45000.0f, 50.0f);
    SwampConfig.SpawnRadius = 10000.0f;
    SwampConfig.MaxEntities = 6000;
    SwampConfig.EntityDensity = 0.5f;
    
    FCrowd_BiomeSpawnConfig MountainConfig;
    MountainConfig.BiomeType = ECrowd_BiomeType::Mountain;
    MountainConfig.BiomeCenter = FVector(40000.0f, 50000.0f, 500.0f);
    MountainConfig.SpawnRadius = 14000.0f;
    MountainConfig.MaxEntities = 4000;
    MountainConfig.EntityDensity = 0.3f;
    
    BiomeConfigs.Add(SavanaConfig);
    BiomeConfigs.Add(ForestConfig);
    BiomeConfigs.Add(DesertConfig);
    BiomeConfigs.Add(SwampConfig);
    BiomeConfigs.Add(MountainConfig);
    
    TotalEntityLimit = 50000;
    SpawnInterval = 0.5f;
    bAutoSpawn = true;
    CurrentEntityCount = 0;
}

void ACrowd_MassEntitySpawner::BeginPlay()
{
    Super::BeginPlay();
    
    MassEntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("ACrowd_MassEntitySpawner: Failed to get MassEntitySubsystem"));
        return;
    }
    
    InitializeBiomes();
    
    if (bAutoSpawn)
    {
        GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &ACrowd_MassEntitySpawner::OnSpawnTimer, SpawnInterval, true);
    }
}

void ACrowd_MassEntitySpawner::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update LOD system for performance optimization
    UpdateEntityLOD();
}

void ACrowd_MassEntitySpawner::InitializeBiomes()
{
    UE_LOG(LogTemp, Warning, TEXT("ACrowd_MassEntitySpawner: Initializing %d biomes"), BiomeConfigs.Num());
    
    for (const FCrowd_BiomeSpawnConfig& BiomeConfig : BiomeConfigs)
    {
        // Pre-spawn initial entities in each biome
        int32 InitialSpawnCount = FMath::RoundToInt(BiomeConfig.MaxEntities * 0.3f); // Start with 30% capacity
        SpawnEntitiesInBiome(BiomeConfig.BiomeType, InitialSpawnCount);
    }
}

void ACrowd_MassEntitySpawner::SpawnEntitiesInBiome(ECrowd_BiomeType BiomeType, int32 Count)
{
    if (!MassEntitySubsystem || !CanSpawnMoreEntities())
    {
        return;
    }
    
    const FCrowd_BiomeSpawnConfig* BiomeConfig = BiomeConfigs.FindByPredicate([BiomeType](const FCrowd_BiomeSpawnConfig& Config)
    {
        return Config.BiomeType == BiomeType;
    });
    
    if (!BiomeConfig)
    {
        UE_LOG(LogTemp, Error, TEXT("ACrowd_MassEntitySpawner: BiomeConfig not found for biome type"));
        return;
    }
    
    int32 CurrentBiomeCount = GetEntityCountInBiome(BiomeType);
    int32 SpawnableCount = FMath::Min(Count, BiomeConfig->MaxEntities - CurrentBiomeCount);
    SpawnableCount = FMath::Min(SpawnableCount, TotalEntityLimit - CurrentEntityCount);
    
    for (int32 i = 0; i < SpawnableCount; i++)
    {
        FVector SpawnLocation = GenerateRandomLocationInBiome(*BiomeConfig);
        SpawnSingleEntity(SpawnLocation, BiomeType);
    }
    
    UE_LOG(LogTemp, Log, TEXT("ACrowd_MassEntitySpawner: Spawned %d entities in biome %d"), SpawnableCount, (int32)BiomeType);
}

void ACrowd_MassEntitySpawner::SpawnSingleEntity(const FVector& Location, ECrowd_BiomeType BiomeType)
{
    if (!MassEntitySubsystem || !CanSpawnMoreEntities())
    {
        return;
    }
    
    // Create entity spawn data
    FCrowd_EntitySpawnData NewEntityData;
    NewEntityData.SpawnLocation = Location;
    NewEntityData.SpawnRotation = FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f);
    NewEntityData.AssignedBiome = BiomeType;
    NewEntityData.MovementSpeed = FMath::RandRange(50.0f, 150.0f);
    NewEntityData.bIsActive = true;
    
    // Create Mass Entity (simplified for now - would need proper archetype setup)
    FMassEntityHandle NewEntity = MassEntitySubsystem->CreateEntity();
    if (NewEntity.IsValid())
    {
        SpawnedEntities.Add(NewEntityData);
        CurrentEntityCount++;
        LastSpawnedEntity = NewEntity;
    }
}

void ACrowd_MassEntitySpawner::ClearAllEntities()
{
    if (MassEntitySubsystem)
    {
        // Clear all spawned entities
        for (const FCrowd_EntitySpawnData& EntityData : SpawnedEntities)
        {
            // Would destroy individual entities here if we had proper handles
        }
    }
    
    SpawnedEntities.Empty();
    CurrentEntityCount = 0;
    
    UE_LOG(LogTemp, Warning, TEXT("ACrowd_MassEntitySpawner: Cleared all entities"));
}

void ACrowd_MassEntitySpawner::SetSpawnRate(float NewInterval)
{
    SpawnInterval = FMath::Max(0.1f, NewInterval);
    
    if (SpawnTimerHandle.IsValid())
    {
        GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
        GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &ACrowd_MassEntitySpawner::OnSpawnTimer, SpawnInterval, true);
    }
}

int32 ACrowd_MassEntitySpawner::GetEntityCountInBiome(ECrowd_BiomeType BiomeType) const
{
    int32 Count = 0;
    for (const FCrowd_EntitySpawnData& EntityData : SpawnedEntities)
    {
        if (EntityData.AssignedBiome == BiomeType && EntityData.bIsActive)
        {
            Count++;
        }
    }
    return Count;
}

FVector ACrowd_MassEntitySpawner::GetBiomeCenter(ECrowd_BiomeType BiomeType) const
{
    const FCrowd_BiomeSpawnConfig* BiomeConfig = BiomeConfigs.FindByPredicate([BiomeType](const FCrowd_BiomeSpawnConfig& Config)
    {
        return Config.BiomeType == BiomeType;
    });
    
    return BiomeConfig ? BiomeConfig->BiomeCenter : FVector::ZeroVector;
}

void ACrowd_MassEntitySpawner::OnSpawnTimer()
{
    if (!CanSpawnMoreEntities())
    {
        return;
    }
    
    // Randomly select a biome to spawn in
    if (BiomeConfigs.Num() > 0)
    {
        int32 RandomBiomeIndex = FMath::RandRange(0, BiomeConfigs.Num() - 1);
        ECrowd_BiomeType SelectedBiome = BiomeConfigs[RandomBiomeIndex].BiomeType;
        
        // Spawn 1-5 entities per timer tick
        int32 SpawnCount = FMath::RandRange(1, 5);
        SpawnEntitiesInBiome(SelectedBiome, SpawnCount);
    }
}

FVector ACrowd_MassEntitySpawner::GenerateRandomLocationInBiome(const FCrowd_BiomeSpawnConfig& BiomeConfig) const
{
    float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
    float RandomRadius = FMath::RandRange(0.0f, BiomeConfig.SpawnRadius);
    
    FVector RandomOffset;
    RandomOffset.X = RandomRadius * FMath::Cos(RandomAngle);
    RandomOffset.Y = RandomRadius * FMath::Sin(RandomAngle);
    RandomOffset.Z = FMath::RandRange(-100.0f, 200.0f); // Height variation
    
    return BiomeConfig.BiomeCenter + RandomOffset;
}

bool ACrowd_MassEntitySpawner::CanSpawnMoreEntities() const
{
    return CurrentEntityCount < TotalEntityLimit;
}

void ACrowd_MassEntitySpawner::UpdateEntityLOD()
{
    // Simple LOD system - would be more sophisticated in production
    if (CurrentEntityCount > 30000)
    {
        // High entity count - reduce update frequency
        SetActorTickInterval(0.1f);
    }
    else if (CurrentEntityCount > 15000)
    {
        // Medium entity count - normal update frequency
        SetActorTickInterval(0.05f);
    }
    else
    {
        // Low entity count - full update frequency
        SetActorTickInterval(0.0f);
    }
}