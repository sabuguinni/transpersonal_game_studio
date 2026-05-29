#include "Crowd_MassEntityManager.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassRepresentationFragments.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

UCrowd_MassEntityManager::UCrowd_MassEntityManager()
{
    MaxEntities = 50000;
    CrowdDensity = 1.0f;
    DefaultMovementSpeed = 150.0f;
    HighLODDistance = 1000.0f;
    MediumLODDistance = 5000.0f;
    LowLODDistance = 15000.0f;
    CurrentEntityCount = 0;
}

void UCrowd_MassEntityManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("UCrowd_MassEntityManager::Initialize - Starting crowd simulation system"));
    
    // Get Mass Entity Subsystem
    if (UWorld* World = GetWorld())
    {
        MassEntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
        if (MassEntitySubsystem)
        {
            UE_LOG(LogTemp, Warning, TEXT("Mass Entity Subsystem found and connected"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Mass Entity Subsystem not found - crowd simulation disabled"));
        }
    }
    
    InitializeBiomeSpawnCenters();
    CreateEntityArchetype();
}

void UCrowd_MassEntityManager::Deinitialize()
{
    DespawnAllEntities();
    MassEntitySubsystem = nullptr;
    Super::Deinitialize();
}

void UCrowd_MassEntityManager::InitializeBiomeSpawnCenters()
{
    // Initialize biome spawn centers based on world layout
    BiomeSpawnCenters.Empty();
    BiomeSpawnCenters.Add(EBiomeType::Savana, FVector(0.0f, 0.0f, 100.0f));
    BiomeSpawnCenters.Add(EBiomeType::Floresta, FVector(-45000.0f, 40000.0f, 100.0f));
    BiomeSpawnCenters.Add(EBiomeType::Deserto, FVector(50000.0f, -40000.0f, 100.0f));
    BiomeSpawnCenters.Add(EBiomeType::Montanha, FVector(-30000.0f, -30000.0f, 500.0f));
    BiomeSpawnCenters.Add(EBiomeType::Pantano, FVector(30000.0f, 30000.0f, 50.0f));
    
    UE_LOG(LogTemp, Warning, TEXT("Biome spawn centers initialized for 5 biomes"));
}

void UCrowd_MassEntityManager::CreateEntityArchetype()
{
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot create entity archetype - Mass Entity Subsystem is null"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Entity archetype creation ready"));
}

void UCrowd_MassEntityManager::SpawnCrowdGroup(const FCrowd_EntitySpawnData& SpawnData)
{
    if (!MassEntitySubsystem || CurrentEntityCount >= MaxEntities)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot spawn crowd group - limit reached or subsystem unavailable"));
        return;
    }
    
    FCrowd_EntityGroup NewGroup;
    NewGroup.GroupCenter = SpawnData.Location;
    NewGroup.BiomeType = SpawnData.BiomeType;
    NewGroup.GroupRadius = 500.0f;
    NewGroup.bIsActive = true;
    
    // Spawn entities in group formation
    for (int32 i = 0; i < SpawnData.GroupSize && CurrentEntityCount < MaxEntities; ++i)
    {
        FVector SpawnLocation = GetRandomLocationInBiome(SpawnData.BiomeType, NewGroup.GroupRadius);
        SpawnLocation += SpawnData.Location;
        
        // Create entity with Mass Entity system
        FMassEntityHandle NewEntity = MassEntitySubsystem->CreateEntity();
        if (NewEntity.IsValid())
        {
            NewGroup.Entities.Add(NewEntity);
            AllSpawnedEntities.Add(NewEntity);
            CurrentEntityCount++;
            
            UE_LOG(LogTemp, Log, TEXT("Spawned crowd entity %d at location %s"), i, *SpawnLocation.ToString());
        }
    }
    
    CrowdGroups.Add(NewGroup);
    UE_LOG(LogTemp, Warning, TEXT("Spawned crowd group with %d entities in biome %d"), NewGroup.Entities.Num(), (int32)SpawnData.BiomeType);
}

void UCrowd_MassEntityManager::SpawnCrowdInBiome(EBiomeType BiomeType, int32 EntityCount)
{
    if (!BiomeSpawnCenters.Contains(BiomeType))
    {
        UE_LOG(LogTemp, Error, TEXT("Unknown biome type for crowd spawning: %d"), (int32)BiomeType);
        return;
    }
    
    FVector BiomeCenter = BiomeSpawnCenters[BiomeType];
    
    FCrowd_EntitySpawnData SpawnData;
    SpawnData.Location = BiomeCenter;
    SpawnData.BiomeType = BiomeType;
    SpawnData.MovementSpeed = DefaultMovementSpeed;
    SpawnData.GroupSize = FMath::Min(EntityCount, 50); // Spawn in groups of max 50
    
    int32 GroupsToSpawn = FMath::CeilToInt((float)EntityCount / 50.0f);
    
    for (int32 GroupIndex = 0; GroupIndex < GroupsToSpawn; ++GroupIndex)
    {
        FVector GroupOffset = FVector(
            FMath::RandRange(-5000.0f, 5000.0f),
            FMath::RandRange(-5000.0f, 5000.0f),
            0.0f
        );
        SpawnData.Location = BiomeCenter + GroupOffset;
        
        SpawnCrowdGroup(SpawnData);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Spawned %d crowd entities across %d groups in biome %d"), EntityCount, GroupsToSpawn, (int32)BiomeType);
}

void UCrowd_MassEntityManager::DespawnAllEntities()
{
    if (!MassEntitySubsystem)
    {
        return;
    }
    
    for (FMassEntityHandle Entity : AllSpawnedEntities)
    {
        if (Entity.IsValid())
        {
            MassEntitySubsystem->DestroyEntity(Entity);
        }
    }
    
    AllSpawnedEntities.Empty();
    CrowdGroups.Empty();
    CurrentEntityCount = 0;
    
    UE_LOG(LogTemp, Warning, TEXT("All crowd entities despawned"));
}

void UCrowd_MassEntityManager::UpdateCrowdMovement(float DeltaTime)
{
    if (!MassEntitySubsystem)
    {
        return;
    }
    
    // Update movement for all active groups
    for (FCrowd_EntityGroup& Group : CrowdGroups)
    {
        if (!Group.bIsActive)
        {
            continue;
        }
        
        for (FMassEntityHandle Entity : Group.Entities)
        {
            if (Entity.IsValid())
            {
                // Basic movement update - entities wander around group center
                // In a full implementation, this would use Mass Movement processors
            }
        }
    }
}

int32 UCrowd_MassEntityManager::GetActiveEntityCount() const
{
    return CurrentEntityCount;
}

void UCrowd_MassEntityManager::SetCrowdDensity(float Density)
{
    CrowdDensity = FMath::Clamp(Density, 0.1f, 5.0f);
    UE_LOG(LogTemp, Warning, TEXT("Crowd density set to %f"), CrowdDensity);
}

void UCrowd_MassEntityManager::SetMovementSpeed(float Speed)
{
    DefaultMovementSpeed = FMath::Clamp(Speed, 50.0f, 500.0f);
    UE_LOG(LogTemp, Warning, TEXT("Crowd movement speed set to %f"), DefaultMovementSpeed);
}

void UCrowd_MassEntityManager::UpdateLODForDistance(const FVector& PlayerLocation)
{
    for (FCrowd_EntityGroup& Group : CrowdGroups)
    {
        if (!Group.bIsActive)
        {
            continue;
        }
        
        float DistanceToGroup = FVector::Dist(PlayerLocation, Group.GroupCenter);
        
        for (FMassEntityHandle Entity : Group.Entities)
        {
            if (Entity.IsValid())
            {
                ApplyLODToEntity(Entity, DistanceToGroup);
            }
        }
    }
}

void UCrowd_MassEntityManager::SetLODDistances(float HighLOD, float MediumLOD, float LowLOD)
{
    HighLODDistance = HighLOD;
    MediumLODDistance = MediumLOD;
    LowLODDistance = LowLOD;
    
    UE_LOG(LogTemp, Warning, TEXT("LOD distances updated: High=%f, Medium=%f, Low=%f"), HighLOD, MediumLOD, LowLOD);
}

void UCrowd_MassEntityManager::DebugSpawnTestGroup()
{
    FCrowd_EntitySpawnData TestData;
    TestData.Location = FVector(1000.0f, 1000.0f, 100.0f);
    TestData.BiomeType = EBiomeType::Savana;
    TestData.GroupSize = 10;
    TestData.MovementSpeed = 100.0f;
    
    SpawnCrowdGroup(TestData);
    UE_LOG(LogTemp, Warning, TEXT("Debug test group spawned"));
}

void UCrowd_MassEntityManager::PrintCrowdStats()
{
    UE_LOG(LogTemp, Warning, TEXT("=== CROWD SIMULATION STATS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Active Entities: %d / %d"), CurrentEntityCount, MaxEntities);
    UE_LOG(LogTemp, Warning, TEXT("Active Groups: %d"), CrowdGroups.Num());
    UE_LOG(LogTemp, Warning, TEXT("Crowd Density: %f"), CrowdDensity);
    UE_LOG(LogTemp, Warning, TEXT("Movement Speed: %f"), DefaultMovementSpeed);
    
    for (int32 i = 0; i < CrowdGroups.Num(); ++i)
    {
        const FCrowd_EntityGroup& Group = CrowdGroups[i];
        UE_LOG(LogTemp, Warning, TEXT("Group %d: %d entities, Biome %d, Center %s"), 
               i, Group.Entities.Num(), (int32)Group.BiomeType, *Group.GroupCenter.ToString());
    }
}

FVector UCrowd_MassEntityManager::GetRandomLocationInBiome(EBiomeType BiomeType, float Radius)
{
    FVector RandomOffset = FVector(
        FMath::RandRange(-Radius, Radius),
        FMath::RandRange(-Radius, Radius),
        FMath::RandRange(-50.0f, 50.0f)
    );
    
    return RandomOffset;
}

void UCrowd_MassEntityManager::ApplyLODToEntity(FMassEntityHandle Entity, float Distance)
{
    if (!Entity.IsValid())
    {
        return;
    }
    
    // Apply LOD based on distance
    if (Distance <= HighLODDistance)
    {
        // High LOD - full detail
    }
    else if (Distance <= MediumLODDistance)
    {
        // Medium LOD - reduced detail
    }
    else if (Distance <= LowLODDistance)
    {
        // Low LOD - minimal detail
    }
    else
    {
        // Very far - consider culling
    }
}