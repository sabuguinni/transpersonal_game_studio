#include "Crowd_MassEntityManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassRepresentationFragments.h"
#include "Components/StaticMeshComponent.h"

UCrowd_MassEntityManager::UCrowd_MassEntityManager()
{
    MassEntitySubsystem = nullptr;
    CrowdDensityMultiplier = 1.0f;
    MaxTotalEntities = 50000;
    bEnableLODSystem = true;
    LODUpdateFrequency = 0.5f;
}

void UCrowd_MassEntityManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("UCrowd_MassEntityManager::Initialize"));
    
    // Get Mass Entity Subsystem
    MassEntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get MassEntitySubsystem"));
        return;
    }

    InitializeBiomeConfigs();
    InitializeMassEntitySystem();
}

void UCrowd_MassEntityManager::Deinitialize()
{
    ClearAllCrowdEntities();
    
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(LODUpdateTimer);
    }
    
    Super::Deinitialize();
}

void UCrowd_MassEntityManager::InitializeMassEntitySystem()
{
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("MassEntitySubsystem not available"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("Mass Entity System initialized for crowd simulation"));

    // Start LOD update timer
    if (bEnableLODSystem && GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(LODUpdateTimer, this, 
            &UCrowd_MassEntityManager::OnLODUpdateTick, LODUpdateFrequency, true);
    }
}

void UCrowd_MassEntityManager::InitializeBiomeConfigs()
{
    BiomeConfigs.Empty();

    // Savana biome
    FCrowd_BiomeCrowdConfig SavanaConfig;
    SavanaConfig.BiomeType = EBiomeType::Savana;
    SavanaConfig.MaxEntities = 12000;
    SavanaConfig.SpawnRadius = 45000.0f;
    SavanaConfig.BiomeCenter = FVector(0, 0, 100);
    SavanaConfig.LODDistance1 = 5000.0f;
    SavanaConfig.LODDistance2 = 15000.0f;
    SavanaConfig.CullDistance = 30000.0f;
    BiomeConfigs.Add(SavanaConfig);

    // Floresta biome
    FCrowd_BiomeCrowdConfig FlorestaConfig;
    FlorestaConfig.BiomeType = EBiomeType::Floresta;
    FlorestaConfig.MaxEntities = 15000;
    FlorestaConfig.SpawnRadius = 40000.0f;
    FlorestaConfig.BiomeCenter = FVector(-45000, 40000, 100);
    FlorestaConfig.LODDistance1 = 4000.0f;
    FlorestaConfig.LODDistance2 = 12000.0f;
    FlorestaConfig.CullDistance = 25000.0f;
    BiomeConfigs.Add(FlorestaConfig);

    // Deserto biome
    FCrowd_BiomeCrowdConfig DesertoConfig;
    DesertoConfig.BiomeType = EBiomeType::Deserto;
    DesertoConfig.MaxEntities = 8000;
    DesertoConfig.SpawnRadius = 50000.0f;
    DesertoConfig.BiomeCenter = FVector(55000, 0, 100);
    DesertoConfig.LODDistance1 = 6000.0f;
    DesertoConfig.LODDistance2 = 18000.0f;
    DesertoConfig.CullDistance = 35000.0f;
    BiomeConfigs.Add(DesertoConfig);

    // Pantano biome
    FCrowd_BiomeCrowdConfig PantanoConfig;
    PantanoConfig.BiomeType = EBiomeType::Pantano;
    PantanoConfig.MaxEntities = 10000;
    PantanoConfig.SpawnRadius = 35000.0f;
    PantanoConfig.BiomeCenter = FVector(-50000, -45000, 100);
    PantanoConfig.LODDistance1 = 4500.0f;
    PantanoConfig.LODDistance2 = 13000.0f;
    PantanoConfig.CullDistance = 28000.0f;
    BiomeConfigs.Add(PantanoConfig);

    // Montanha biome
    FCrowd_BiomeCrowdConfig MontanhaConfig;
    MontanhaConfig.BiomeType = EBiomeType::Montanha;
    MontanhaConfig.MaxEntities = 5000;
    MontanhaConfig.SpawnRadius = 30000.0f;
    MontanhaConfig.BiomeCenter = FVector(40000, 50000, 500);
    MontanhaConfig.LODDistance1 = 7000.0f;
    MontanhaConfig.LODDistance2 = 20000.0f;
    MontanhaConfig.CullDistance = 40000.0f;
    BiomeConfigs.Add(MontanhaConfig);

    UE_LOG(LogTemp, Warning, TEXT("Initialized %d biome crowd configurations"), BiomeConfigs.Num());
}

void UCrowd_MassEntityManager::SpawnCrowdEntitiesInBiome(EBiomeType BiomeType, int32 EntityCount)
{
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot spawn crowd entities - MassEntitySubsystem not available"));
        return;
    }

    // Find biome config
    FCrowd_BiomeCrowdConfig* BiomeConfig = BiomeConfigs.FindByPredicate([BiomeType](const FCrowd_BiomeCrowdConfig& Config)
    {
        return Config.BiomeType == BiomeType;
    });

    if (!BiomeConfig)
    {
        UE_LOG(LogTemp, Error, TEXT("No biome config found for biome type %d"), (int32)BiomeType);
        return;
    }

    // Clamp entity count to biome maximum
    int32 ClampedEntityCount = FMath::Min(EntityCount, BiomeConfig->MaxEntities);
    ClampedEntityCount = FMath::Min(ClampedEntityCount, MaxTotalEntities - GetTotalEntityCount());

    UE_LOG(LogTemp, Warning, TEXT("Spawning %d crowd entities in biome %d"), ClampedEntityCount, (int32)BiomeType);

    // Initialize biome entity array if needed
    if (!BiomeEntities.Contains(BiomeType))
    {
        BiomeEntities.Add(BiomeType, TArray<FMassEntityHandle>());
    }

    // Spawn entities
    for (int32 i = 0; i < ClampedEntityCount; i++)
    {
        FCrowd_EntitySpawnData SpawnData;
        SpawnData.Location = GetBiomeSpawnLocation(BiomeType, BiomeConfig->SpawnRadius);
        SpawnData.Rotation = FRotator(0, FMath::RandRange(0.0f, 360.0f), 0);
        SpawnData.BiomeType = BiomeType;
        SpawnData.MovementSpeed = FMath::RandRange(80.0f, 150.0f);
        SpawnData.WanderRadius = FMath::RandRange(500.0f, 2000.0f);

        SpawnCrowdEntity(SpawnData);
    }

    UE_LOG(LogTemp, Warning, TEXT("Spawned %d entities in biome %d. Total entities: %d"), 
        ClampedEntityCount, (int32)BiomeType, GetTotalEntityCount());
}

void UCrowd_MassEntityManager::SpawnCrowdEntity(const FCrowd_EntitySpawnData& SpawnData)
{
    if (!MassEntitySubsystem)
    {
        return;
    }

    // Create entity
    FMassEntityHandle EntityHandle = MassEntitySubsystem->CreateEntity();
    if (!EntityHandle.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create Mass Entity"));
        return;
    }

    // Setup entity fragments
    SetupEntityFragments(EntityHandle, SpawnData);

    // Add to tracking arrays
    AllCrowdEntities.Add(EntityHandle);
    
    if (!BiomeEntities.Contains(SpawnData.BiomeType))
    {
        BiomeEntities.Add(SpawnData.BiomeType, TArray<FMassEntityHandle>());
    }
    BiomeEntities[SpawnData.BiomeType].Add(EntityHandle);
}

void UCrowd_MassEntityManager::SetupEntityFragments(FMassEntityHandle EntityHandle, const FCrowd_EntitySpawnData& SpawnData)
{
    if (!MassEntitySubsystem || !EntityHandle.IsValid())
    {
        return;
    }

    // Add Transform fragment
    FMassEntityManager& EntityManager = MassEntitySubsystem->GetMutableEntityManager();
    
    // Transform fragment
    FTransformFragment TransformFragment;
    TransformFragment.GetMutableTransform().SetLocation(SpawnData.Location);
    TransformFragment.GetMutableTransform().SetRotation(SpawnData.Rotation.Quaternion());
    EntityManager.AddFragmentToEntity(EntityHandle, TransformFragment);

    // Velocity fragment for movement
    FMassVelocityFragment VelocityFragment;
    VelocityFragment.Value = FVector::ZeroVector;
    EntityManager.AddFragmentToEntity(EntityHandle, VelocityFragment);

    // Force fragment for physics
    FMassForceFragment ForceFragment;
    ForceFragment.Value = FVector::ZeroVector;
    EntityManager.AddFragmentToEntity(EntityHandle, ForceFragment);
}

void UCrowd_MassEntityManager::ClearAllCrowdEntities()
{
    if (!MassEntitySubsystem)
    {
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("Clearing all crowd entities (%d total)"), AllCrowdEntities.Num());

    FMassEntityManager& EntityManager = MassEntitySubsystem->GetMutableEntityManager();
    
    for (FMassEntityHandle EntityHandle : AllCrowdEntities)
    {
        if (EntityHandle.IsValid())
        {
            EntityManager.DestroyEntity(EntityHandle);
        }
    }

    AllCrowdEntities.Empty();
    BiomeEntities.Empty();

    UE_LOG(LogTemp, Warning, TEXT("All crowd entities cleared"));
}

void UCrowd_MassEntityManager::SetBiomeCrowdConfig(const FCrowd_BiomeCrowdConfig& Config)
{
    // Find existing config and update it
    FCrowd_BiomeCrowdConfig* ExistingConfig = BiomeConfigs.FindByPredicate([Config](const FCrowd_BiomeCrowdConfig& Existing)
    {
        return Existing.BiomeType == Config.BiomeType;
    });

    if (ExistingConfig)
    {
        *ExistingConfig = Config;
    }
    else
    {
        BiomeConfigs.Add(Config);
    }

    UE_LOG(LogTemp, Warning, TEXT("Updated biome crowd config for biome %d"), (int32)Config.BiomeType);
}

int32 UCrowd_MassEntityManager::GetTotalEntityCount() const
{
    return AllCrowdEntities.Num();
}

int32 UCrowd_MassEntityManager::GetBiomeEntityCount(EBiomeType BiomeType) const
{
    if (const TArray<FMassEntityHandle>* BiomeEntityArray = BiomeEntities.Find(BiomeType))
    {
        return BiomeEntityArray->Num();
    }
    return 0;
}

FVector UCrowd_MassEntityManager::GetBiomeSpawnLocation(EBiomeType BiomeType, float Radius) const
{
    // Find biome config
    const FCrowd_BiomeCrowdConfig* BiomeConfig = BiomeConfigs.FindByPredicate([BiomeType](const FCrowd_BiomeCrowdConfig& Config)
    {
        return Config.BiomeType == BiomeType;
    });

    if (!BiomeConfig)
    {
        return FVector::ZeroVector;
    }

    // Generate random location within biome radius
    float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
    float RandomDistance = FMath::RandRange(0.0f, Radius);
    
    FVector RandomOffset;
    RandomOffset.X = FMath::Cos(RandomAngle) * RandomDistance;
    RandomOffset.Y = FMath::Sin(RandomAngle) * RandomDistance;
    RandomOffset.Z = 0.0f;

    return BiomeConfig->BiomeCenter + RandomOffset;
}

void UCrowd_MassEntityManager::UpdateCrowdLOD(const FVector& ViewerLocation)
{
    if (!bEnableLODSystem || !MassEntitySubsystem)
    {
        return;
    }

    for (FMassEntityHandle EntityHandle : AllCrowdEntities)
    {
        if (EntityHandle.IsValid())
        {
            // Get entity transform
            FMassEntityManager& EntityManager = MassEntitySubsystem->GetMutableEntityManager();
            if (const FTransformFragment* TransformFragment = EntityManager.GetFragmentDataPtr<FTransformFragment>(EntityHandle))
            {
                float Distance = FVector::Dist(ViewerLocation, TransformFragment->GetTransform().GetLocation());
                UpdateEntityLOD(EntityHandle, Distance);
            }
        }
    }
}

void UCrowd_MassEntityManager::UpdateEntityLOD(FMassEntityHandle EntityHandle, float DistanceToViewer)
{
    // LOD logic based on distance
    // This would typically update representation fragments
    // For now, just log for debugging
    if (DistanceToViewer > 30000.0f)
    {
        // Cull entity
    }
    else if (DistanceToViewer > 15000.0f)
    {
        // Low LOD
    }
    else if (DistanceToViewer > 5000.0f)
    {
        // Medium LOD
    }
    else
    {
        // High LOD
    }
}

void UCrowd_MassEntityManager::SetCrowdDensityMultiplier(float Multiplier)
{
    CrowdDensityMultiplier = FMath::Clamp(Multiplier, 0.1f, 5.0f);
    UE_LOG(LogTemp, Warning, TEXT("Crowd density multiplier set to %f"), CrowdDensityMultiplier);
}

void UCrowd_MassEntityManager::OnLODUpdateTick()
{
    if (!GetWorld())
    {
        return;
    }

    // Get player location for LOD calculations
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn)
    {
        UpdateCrowdLOD(PlayerPawn->GetActorLocation());
    }
}

void UCrowd_MassEntityManager::DebugSpawnTestCrowd()
{
    UE_LOG(LogTemp, Warning, TEXT("Debug spawning test crowd in all biomes"));
    
    SpawnCrowdEntitiesInBiome(EBiomeType::Savana, 100);
    SpawnCrowdEntitiesInBiome(EBiomeType::Floresta, 100);
    SpawnCrowdEntitiesInBiome(EBiomeType::Deserto, 50);
    SpawnCrowdEntitiesInBiome(EBiomeType::Pantano, 75);
    SpawnCrowdEntitiesInBiome(EBiomeType::Montanha, 25);
    
    UE_LOG(LogTemp, Warning, TEXT("Test crowd spawned. Total entities: %d"), GetTotalEntityCount());
}