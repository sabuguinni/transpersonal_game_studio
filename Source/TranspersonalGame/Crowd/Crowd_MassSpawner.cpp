#include "Crowd_MassSpawner.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassEntityTemplateRegistry.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"

ACrowd_MassSpawner::ACrowd_MassSpawner()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Initialize default spawn config
    SpawnConfig.MaxEntities = 1000;
    SpawnConfig.SpawnRadius = 5000.0f;
    SpawnConfig.BiomeType = ECrowd_BiomeType::Savana;
    SpawnConfig.EntityType = ECrowd_EntityType::Herbivore;

    CurrentEntityCount = 0;
    bIsSpawning = false;
    MassEntitySubsystem = nullptr;
    LastUpdateTime = 0.0f;
}

void ACrowd_MassSpawner::BeginPlay()
{
    Super::BeginPlay();

    // Get Mass Entity Subsystem
    if (UWorld* World = GetWorld())
    {
        MassEntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
        if (MassEntitySubsystem)
        {
            UE_LOG(LogTemp, Log, TEXT("Crowd_MassSpawner: Mass Entity Subsystem found"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Crowd_MassSpawner: Mass Entity Subsystem not found"));
        }
    }

    // Initialize biome configs if empty
    if (BiomeConfigs.Num() == 0)
    {
        InitializeBiomeConfigs();
    }

    // Start spawning after a short delay
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ACrowd_MassSpawner::StartMassSpawning, 2.0f, false);
}

void ACrowd_MassSpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopMassSpawning();
    Super::EndPlay(EndPlayReason);
}

void ACrowd_MassSpawner::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsSpawning && GetWorld())
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - LastUpdateTime >= UpdateInterval)
        {
            UpdateEntityCount();
            LastUpdateTime = CurrentTime;
        }
    }
}

void ACrowd_MassSpawner::StartMassSpawning()
{
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("Crowd_MassSpawner: Cannot start spawning - Mass Entity Subsystem not available"));
        return;
    }

    if (bIsSpawning)
    {
        UE_LOG(LogTemp, Warning, TEXT("Crowd_MassSpawner: Already spawning"));
        return;
    }

    bIsSpawning = true;
    UE_LOG(LogTemp, Log, TEXT("Crowd_MassSpawner: Starting mass spawning"));

    // Distribute entities across all biomes
    DistributeEntitiesAcrossBiomes();
}

void ACrowd_MassSpawner::StopMassSpawning()
{
    if (!bIsSpawning)
    {
        return;
    }

    bIsSpawning = false;
    UE_LOG(LogTemp, Log, TEXT("Crowd_MassSpawner: Stopping mass spawning"));

    // Clean up spawned entities
    if (MassEntitySubsystem)
    {
        for (const FMassEntityHandle& EntityHandle : SpawnedEntities)
        {
            if (MassEntitySubsystem->IsEntityValid(EntityHandle))
            {
                MassEntitySubsystem->DestroyEntity(EntityHandle);
            }
        }
        SpawnedEntities.Empty();
    }

    CurrentEntityCount = 0;
}

void ACrowd_MassSpawner::SpawnEntitiesInBiome(ECrowd_BiomeType BiomeType, int32 Count)
{
    if (!MassEntitySubsystem || Count <= 0)
    {
        return;
    }

    FVector BiomeCenter = GetBiomeCenterLocation(BiomeType);
    
    // Find config for this biome
    FCrowd_SpawnConfig* Config = BiomeConfigs.FindByPredicate([BiomeType](const FCrowd_SpawnConfig& Cfg)
    {
        return Cfg.BiomeType == BiomeType;
    });

    if (!Config)
    {
        // Use default config
        FCrowd_SpawnConfig DefaultConfig;
        DefaultConfig.BiomeType = BiomeType;
        DefaultConfig.MaxEntities = Count;
        SpawnMassEntities(DefaultConfig, BiomeCenter);
    }
    else
    {
        // Use existing config but override count
        FCrowd_SpawnConfig TempConfig = *Config;
        TempConfig.MaxEntities = Count;
        SpawnMassEntities(TempConfig, BiomeCenter);
    }

    UE_LOG(LogTemp, Log, TEXT("Crowd_MassSpawner: Spawned %d entities in biome %d at location %s"), 
           Count, (int32)BiomeType, *BiomeCenter.ToString());
}

void ACrowd_MassSpawner::DistributeEntitiesAcrossBiomes()
{
    if (!MassEntitySubsystem)
    {
        return;
    }

    // Distribute 20% to each biome as per memory requirements
    int32 EntitiesPerBiome = SpawnConfig.MaxEntities / 5;

    SpawnEntitiesInBiome(ECrowd_BiomeType::Savana, EntitiesPerBiome);
    SpawnEntitiesInBiome(ECrowd_BiomeType::Pantano, EntitiesPerBiome);
    SpawnEntitiesInBiome(ECrowd_BiomeType::Floresta, EntitiesPerBiome);
    SpawnEntitiesInBiome(ECrowd_BiomeType::Deserto, EntitiesPerBiome);
    SpawnEntitiesInBiome(ECrowd_BiomeType::Montanha, EntitiesPerBiome);

    UE_LOG(LogTemp, Log, TEXT("Crowd_MassSpawner: Distributed %d entities across 5 biomes (%d each)"), 
           SpawnConfig.MaxEntities, EntitiesPerBiome);
}

void ACrowd_MassSpawner::InitializeBiomeConfigs()
{
    BiomeConfigs.Empty();

    // Create configs for each biome
    TArray<ECrowd_BiomeType> Biomes = {
        ECrowd_BiomeType::Savana,
        ECrowd_BiomeType::Pantano,
        ECrowd_BiomeType::Floresta,
        ECrowd_BiomeType::Deserto,
        ECrowd_BiomeType::Montanha
    };

    for (ECrowd_BiomeType BiomeType : Biomes)
    {
        FCrowd_SpawnConfig Config;
        Config.BiomeType = BiomeType;
        Config.MaxEntities = 200; // 200 per biome = 1000 total
        Config.SpawnRadius = 15000.0f; // 15km radius per biome
        
        // Set entity type based on biome
        switch (BiomeType)
        {
            case ECrowd_BiomeType::Savana:
            case ECrowd_BiomeType::Floresta:
                Config.EntityType = ECrowd_EntityType::Herbivore;
                break;
            case ECrowd_BiomeType::Pantano:
            case ECrowd_BiomeType::Montanha:
                Config.EntityType = ECrowd_EntityType::Carnivore;
                break;
            case ECrowd_BiomeType::Deserto:
                Config.EntityType = ECrowd_EntityType::Scavenger;
                break;
        }

        BiomeConfigs.Add(Config);
    }

    UE_LOG(LogTemp, Log, TEXT("Crowd_MassSpawner: Initialized %d biome configs"), BiomeConfigs.Num());
}

void ACrowd_MassSpawner::SpawnMassEntities(const FCrowd_SpawnConfig& Config, const FVector& CenterLocation)
{
    if (!MassEntitySubsystem)
    {
        return;
    }

    // Create entities in batches for performance
    const int32 BatchSize = 100;
    int32 RemainingEntities = Config.MaxEntities;

    while (RemainingEntities > 0)
    {
        int32 CurrentBatchSize = FMath::Min(BatchSize, RemainingEntities);
        
        for (int32 i = 0; i < CurrentBatchSize; i++)
        {
            // Generate random position within spawn radius
            FVector RandomOffset = FVector(
                FMath::RandRange(-Config.SpawnRadius, Config.SpawnRadius),
                FMath::RandRange(-Config.SpawnRadius, Config.SpawnRadius),
                100.0f // Keep entities above ground
            );
            
            FVector SpawnLocation = CenterLocation + RandomOffset;
            
            // Create mass entity (simplified for now)
            FMassEntityHandle EntityHandle = MassEntitySubsystem->CreateEntity();
            if (MassEntitySubsystem->IsEntityValid(EntityHandle))
            {
                SpawnedEntities.Add(EntityHandle);
                CurrentEntityCount++;
            }
        }

        RemainingEntities -= CurrentBatchSize;
    }
}

FVector ACrowd_MassSpawner::GetBiomeCenterLocation(ECrowd_BiomeType BiomeType)
{
    // Biome coordinates from memory ID 709
    switch (BiomeType)
    {
        case ECrowd_BiomeType::Savana:
            return FVector(0.0f, 0.0f, 100.0f);
        case ECrowd_BiomeType::Pantano:
            return FVector(-50000.0f, -45000.0f, 100.0f);
        case ECrowd_BiomeType::Floresta:
            return FVector(-45000.0f, 40000.0f, 100.0f);
        case ECrowd_BiomeType::Deserto:
            return FVector(55000.0f, 0.0f, 100.0f);
        case ECrowd_BiomeType::Montanha:
            return FVector(40000.0f, 50000.0f, 100.0f);
        default:
            return FVector::ZeroVector;
    }
}

void ACrowd_MassSpawner::UpdateEntityCount()
{
    if (!MassEntitySubsystem)
    {
        return;
    }

    // Clean up invalid entities
    SpawnedEntities.RemoveAll([this](const FMassEntityHandle& Handle)
    {
        return !MassEntitySubsystem->IsEntityValid(Handle);
    });

    CurrentEntityCount = SpawnedEntities.Num();
}