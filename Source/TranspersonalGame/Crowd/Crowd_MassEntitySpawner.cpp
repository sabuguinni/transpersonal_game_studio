#include "Crowd_MassEntitySpawner.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassEntityTemplateRegistry.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ACrowd_MassEntitySpawner::ACrowd_MassEntitySpawner()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
    
    // Initialize default biome data
    FCrowd_BiomeSpawnData SavanaBiome;
    SavanaBiome.BiomeName = TEXT("Savana");
    SavanaBiome.BiomeCenter = FVector(0.0f, 0.0f, 100.0f);
    SavanaBiome.SpawnConfig.MaxEntities = 1000;
    SavanaBiome.SpawnConfig.SpawnRadius = 10000.0f;
    
    FCrowd_BiomeSpawnData ForestBiome;
    ForestBiome.BiomeName = TEXT("Forest");
    ForestBiome.BiomeCenter = FVector(-45000.0f, 40000.0f, 100.0f);
    ForestBiome.SpawnConfig.MaxEntities = 800;
    ForestBiome.SpawnConfig.SpawnRadius = 8000.0f;
    
    FCrowd_BiomeSpawnData DesertBiome;
    DesertBiome.BiomeName = TEXT("Desert");
    DesertBiome.BiomeCenter = FVector(55000.0f, 0.0f, 100.0f);
    DesertBiome.SpawnConfig.MaxEntities = 600;
    DesertBiome.SpawnConfig.SpawnRadius = 12000.0f;
    
    BiomeSpawnData.Add(SavanaBiome);
    BiomeSpawnData.Add(ForestBiome);
    BiomeSpawnData.Add(DesertBiome);
}

void ACrowd_MassEntitySpawner::BeginPlay()
{
    Super::BeginPlay();
    
    if (UWorld* World = GetWorld())
    {
        MassEntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
        if (MassEntitySubsystem)
        {
            UE_LOG(LogTemp, Log, TEXT("MassEntitySubsystem found and initialized"));
            InitializeBiomes();
            
            if (bAutoSpawnOnBeginPlay)
            {
                // Delay spawn to ensure all systems are ready
                FTimerHandle SpawnTimer;
                GetWorldTimerManager().SetTimer(SpawnTimer, this, &ACrowd_MassEntitySpawner::SpawnEntitiesInAllBiomes, 2.0f, false);
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("MassEntitySubsystem not found!"));
        }
    }
}

void ACrowd_MassEntitySpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    ClearAllEntities();
    Super::EndPlay(EndPlayReason);
}

void ACrowd_MassEntitySpawner::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    LastUpdateTime += DeltaTime;
    if (LastUpdateTime >= UpdateInterval)
    {
        LastUpdateTime = 0.0f;
        
        // Update entity count
        CurrentEntityCount = 0;
        for (const auto& BiomePair : BiomeEntityMap)
        {
            CurrentEntityCount += BiomePair.Value.Num();
        }
        
        // Debug draw biome centers
        if (GEngine && GEngine->bEnableOnScreenDebugMessages)
        {
            for (const FCrowd_BiomeSpawnData& BiomeData : BiomeSpawnData)
            {
                DrawDebugSphere(GetWorld(), BiomeData.BiomeCenter, 500.0f, 12, FColor::Green, false, UpdateInterval + 0.1f);
                
                FString DebugText = FString::Printf(TEXT("%s: %d entities"), 
                    *BiomeData.BiomeName, 
                    GetEntityCountInBiome(BiomeData.BiomeName));
                    
                DrawDebugString(GetWorld(), BiomeData.BiomeCenter + FVector(0, 0, 600), DebugText, nullptr, FColor::White, UpdateInterval + 0.1f);
            }
        }
    }
}

void ACrowd_MassEntitySpawner::InitializeBiomes()
{
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot initialize biomes - MassEntitySubsystem is null"));
        return;
    }
    
    // Clear existing data
    BiomeEntityMap.Empty();
    
    // Initialize entity arrays for each biome
    for (const FCrowd_BiomeSpawnData& BiomeData : BiomeSpawnData)
    {
        BiomeEntityMap.Add(BiomeData.BiomeName, TArray<FMassEntityHandle>());
        UE_LOG(LogTemp, Log, TEXT("Initialized biome: %s at location %s"), 
            *BiomeData.BiomeName, 
            *BiomeData.BiomeCenter.ToString());
    }
    
    bIsInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("Crowd spawner initialized with %d biomes"), BiomeSpawnData.Num());
}

void ACrowd_MassEntitySpawner::SpawnEntitiesInBiome(const FString& BiomeName)
{
    if (!bIsInitialized || !MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot spawn entities - system not initialized"));
        return;
    }
    
    // Find biome data
    FCrowd_BiomeSpawnData* BiomeData = nullptr;
    for (FCrowd_BiomeSpawnData& Data : BiomeSpawnData)
    {
        if (Data.BiomeName == BiomeName)
        {
            BiomeData = &Data;
            break;
        }
    }
    
    if (!BiomeData)
    {
        UE_LOG(LogTemp, Warning, TEXT("Biome not found: %s"), *BiomeName);
        return;
    }
    
    // Clear existing entities in this biome
    if (TArray<FMassEntityHandle>* ExistingEntities = BiomeEntityMap.Find(BiomeName))
    {
        for (const FMassEntityHandle& EntityHandle : *ExistingEntities)
        {
            if (MassEntitySubsystem->IsEntityValid(EntityHandle))
            {
                MassEntitySubsystem->DestroyEntity(EntityHandle);
            }
        }
        ExistingEntities->Empty();
    }
    
    // Spawn new entities
    TArray<FMassEntityHandle> NewEntities;
    const int32 EntitiesToSpawn = FMath::Min(BiomeData->SpawnConfig.MaxEntities, MaxTotalEntities - CurrentEntityCount);
    
    for (int32 i = 0; i < EntitiesToSpawn; i++)
    {
        FVector SpawnLocation = GetRandomSpawnLocation(BiomeData->SpawnConfig);
        
        if (IsValidSpawnLocation(SpawnLocation))
        {
            // Create a basic Mass Entity
            FMassEntityHandle NewEntity = MassEntitySubsystem->CreateEntity();
            if (MassEntitySubsystem->IsEntityValid(NewEntity))
            {
                NewEntities.Add(NewEntity);
                
                // Note: In a full implementation, you would add components here
                // For now, we just track the entity handles
            }
        }
    }
    
    BiomeEntityMap.Add(BiomeName, NewEntities);
    
    UE_LOG(LogTemp, Log, TEXT("Spawned %d entities in biome %s"), NewEntities.Num(), *BiomeName);
}

void ACrowd_MassEntitySpawner::SpawnEntitiesInAllBiomes()
{
    if (!bIsInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot spawn entities - system not initialized"));
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Spawning entities in all biomes..."));
    
    for (const FCrowd_BiomeSpawnData& BiomeData : BiomeSpawnData)
    {
        SpawnEntitiesInBiome(BiomeData.BiomeName);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Finished spawning entities. Total count: %d"), CurrentEntityCount);
}

void ACrowd_MassEntitySpawner::ClearAllEntities()
{
    if (!MassEntitySubsystem)
    {
        return;
    }
    
    for (auto& BiomePair : BiomeEntityMap)
    {
        for (const FMassEntityHandle& EntityHandle : BiomePair.Value)
        {
            if (MassEntitySubsystem->IsEntityValid(EntityHandle))
            {
                MassEntitySubsystem->DestroyEntity(EntityHandle);
            }
        }
        BiomePair.Value.Empty();
    }
    
    CurrentEntityCount = 0;
    UE_LOG(LogTemp, Log, TEXT("Cleared all crowd entities"));
}

void ACrowd_MassEntitySpawner::SetBiomeSpawnConfig(const FString& BiomeName, const FCrowd_SpawnConfig& NewConfig)
{
    for (FCrowd_BiomeSpawnData& BiomeData : BiomeSpawnData)
    {
        if (BiomeData.BiomeName == BiomeName)
        {
            BiomeData.SpawnConfig = NewConfig;
            UE_LOG(LogTemp, Log, TEXT("Updated spawn config for biome: %s"), *BiomeName);
            return;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Biome not found for config update: %s"), *BiomeName);
}

int32 ACrowd_MassEntitySpawner::GetEntityCountInBiome(const FString& BiomeName) const
{
    if (const TArray<FMassEntityHandle>* Entities = BiomeEntityMap.Find(BiomeName))
    {
        return Entities->Num();
    }
    return 0;
}

FVector ACrowd_MassEntitySpawner::GetRandomSpawnLocation(const FCrowd_SpawnConfig& Config) const
{
    float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
    float RandomRadius = FMath::RandRange(Config.MinSpawnDistance, Config.SpawnRadius);
    
    FVector Offset = FVector(
        FMath::Cos(RandomAngle) * RandomRadius,
        FMath::Sin(RandomAngle) * RandomRadius,
        Config.bUseRandomHeight ? FMath::RandRange(-100.0f, 200.0f) : 0.0f
    );
    
    return Config.SpawnCenter + Offset;
}

bool ACrowd_MassEntitySpawner::IsValidSpawnLocation(const FVector& Location) const
{
    // Basic validation - check if location is not too close to player start
    FVector PlayerStart = FVector::ZeroVector;
    if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
    {
        PlayerStart = PlayerPawn->GetActorLocation();
    }
    
    float DistanceToPlayer = FVector::Dist(Location, PlayerStart);
    return DistanceToPlayer > 500.0f; // Minimum distance from player
}

void ACrowd_MassEntitySpawner::SpawnEntitiesAtLocation(const FVector& Location, int32 Count, const TArray<TSoftObjectPtr<UStaticMesh>>& Meshes)
{
    // This method would be used for spawning specific mesh-based entities
    // Currently simplified for basic Mass Entity spawning
    UE_LOG(LogTemp, Log, TEXT("SpawnEntitiesAtLocation called for %d entities at %s"), Count, *Location.ToString());
}