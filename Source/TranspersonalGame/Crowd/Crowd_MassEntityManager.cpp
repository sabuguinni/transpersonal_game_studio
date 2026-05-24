#include "Crowd_MassEntityManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "MassEntitySubsystem.h"
#include "MassEntityConfigAsset.h"
#include "MassSpawnerSubsystem.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassRepresentationFragments.h"
#include "Components/StaticMeshComponent.h"

ACrowd_MassEntityManager::ACrowd_MassEntityManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;

    // Initialize default values
    MaxEntities = 5000;
    SpawnRadius = 10000.0f;
    UpdateInterval = 0.1f;
    bAutoSpawn = true;
    CurrentBiome = ECrowd_BiomeType::Savana;
    
    MassEntitySubsystem = nullptr;
    LastUpdateTime = 0.0f;
    bSystemInitialized = false;

    // Setup default biome locations
    SetupBiomeLocations();

    // Initialize entity templates
    EntityTemplates.SetNum(3);
    
    // Human template
    EntityTemplates[0].EntityType = 0;
    EntityTemplates[0].MovementSpeed = 150.0f;
    EntityTemplates[0].SpawnLocation = FVector::ZeroVector;
    EntityTemplates[0].TargetLocation = FVector(1000, 0, 0);

    // Dinosaur template
    EntityTemplates[1].EntityType = 1;
    EntityTemplates[1].MovementSpeed = 300.0f;
    EntityTemplates[1].SpawnLocation = FVector::ZeroVector;
    EntityTemplates[1].TargetLocation = FVector(2000, 1000, 0);

    // Animal template
    EntityTemplates[2].EntityType = 2;
    EntityTemplates[2].MovementSpeed = 200.0f;
    EntityTemplates[2].SpawnLocation = FVector::ZeroVector;
    EntityTemplates[2].TargetLocation = FVector(500, 500, 0);
}

void ACrowd_MassEntityManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeMassSystem();
    
    if (bAutoSpawn && bSystemInitialized)
    {
        // Auto-spawn entities in current biome
        SpawnMassEntities(1000, CurrentBiome);
    }
}

void ACrowd_MassEntityManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (!bSystemInitialized)
    {
        return;
    }

    LastUpdateTime += DeltaTime;
    if (LastUpdateTime >= UpdateInterval)
    {
        UpdateEntityMovement();
        LastUpdateTime = 0.0f;
    }
}

void ACrowd_MassEntityManager::InitializeMassSystem()
{
    if (UWorld* World = GetWorld())
    {
        MassEntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
        if (MassEntitySubsystem)
        {
            CreateEntityArchetype();
            bSystemInitialized = true;
            UE_LOG(LogTemp, Warning, TEXT("Mass Entity System initialized successfully"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to get Mass Entity Subsystem"));
        }
    }
}

void ACrowd_MassEntityManager::CreateEntityArchetype()
{
    if (!MassEntitySubsystem)
    {
        return;
    }

    // Create archetype with common fragments
    FMassArchetypeCreationParams CreationParams;
    CreationParams.Fragments.Add<FMassTransformFragment>();
    CreationParams.Fragments.Add<FMassVelocityFragment>();
    CreationParams.Fragments.Add<FMassRepresentationFragment>();
    
    EntityArchetype = MassEntitySubsystem->CreateArchetype(CreationParams);
    
    UE_LOG(LogTemp, Warning, TEXT("Mass Entity Archetype created"));
}

void ACrowd_MassEntityManager::SpawnMassEntities(int32 Count, ECrowd_BiomeType Biome)
{
    if (!bSystemInitialized || !MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("Mass system not initialized"));
        return;
    }

    FVector BiomeLocation = GetBiomeSpawnLocation(Biome);
    
    // Clear existing entities first
    ClearAllEntities();
    
    // Spawn new entities
    for (int32 i = 0; i < FMath::Min(Count, MaxEntities); i++)
    {
        // Random spawn position within radius
        FVector RandomOffset = FVector(
            FMath::RandRange(-SpawnRadius, SpawnRadius),
            FMath::RandRange(-SpawnRadius, SpawnRadius),
            FMath::RandRange(0, 200)
        );
        
        FVector SpawnLocation = BiomeLocation + RandomOffset;
        
        // Create entity
        FMassEntityHandle NewEntity = MassEntitySubsystem->CreateEntity(EntityArchetype);
        
        if (NewEntity.IsValid())
        {
            // Set transform
            if (FMassTransformFragment* TransformFragment = 
                MassEntitySubsystem->GetFragmentDataPtr<FMassTransformFragment>(NewEntity))
            {
                TransformFragment->SetTransform(FTransform(FRotator::ZeroRotator, SpawnLocation));
            }
            
            // Set velocity
            if (FMassVelocityFragment* VelocityFragment = 
                MassEntitySubsystem->GetFragmentDataPtr<FMassVelocityFragment>(NewEntity))
            {
                FVector RandomVelocity = FVector(
                    FMath::RandRange(-100, 100),
                    FMath::RandRange(-100, 100),
                    0
                );
                VelocityFragment->Value = RandomVelocity;
            }
            
            SpawnedEntities.Add(NewEntity);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Spawned %d mass entities in %s biome"), 
           SpawnedEntities.Num(), *UEnum::GetValueAsString(Biome));
}

void ACrowd_MassEntityManager::ClearAllEntities()
{
    if (!MassEntitySubsystem)
    {
        return;
    }

    for (FMassEntityHandle& Entity : SpawnedEntities)
    {
        if (Entity.IsValid())
        {
            MassEntitySubsystem->DestroyEntity(Entity);
        }
    }
    
    SpawnedEntities.Empty();
    UE_LOG(LogTemp, Warning, TEXT("Cleared all mass entities"));
}

void ACrowd_MassEntityManager::UpdateEntityMovement()
{
    if (!MassEntitySubsystem)
    {
        return;
    }

    // Update entity positions based on velocity
    for (FMassEntityHandle& Entity : SpawnedEntities)
    {
        if (!Entity.IsValid())
        {
            continue;
        }

        FMassTransformFragment* TransformFragment = 
            MassEntitySubsystem->GetFragmentDataPtr<FMassTransformFragment>(Entity);
        FMassVelocityFragment* VelocityFragment = 
            MassEntitySubsystem->GetFragmentDataPtr<FMassVelocityFragment>(Entity);

        if (TransformFragment && VelocityFragment)
        {
            FVector CurrentLocation = TransformFragment->GetTransform().GetLocation();
            FVector NewLocation = CurrentLocation + (VelocityFragment->Value * UpdateInterval);
            
            // Simple boundary check
            FVector BiomeCenter = GetBiomeSpawnLocation(CurrentBiome);
            float DistanceFromCenter = FVector::Dist2D(NewLocation, BiomeCenter);
            
            if (DistanceFromCenter > SpawnRadius)
            {
                // Reverse direction when hitting boundary
                VelocityFragment->Value *= -1.0f;
                NewLocation = CurrentLocation + (VelocityFragment->Value * UpdateInterval);
            }
            
            TransformFragment->SetTransform(FTransform(FRotator::ZeroRotator, NewLocation));
        }
    }
}

int32 ACrowd_MassEntityManager::GetActiveEntityCount() const
{
    return SpawnedEntities.Num();
}

void ACrowd_MassEntityManager::SetBiomeConfiguration(ECrowd_BiomeType Biome, FVector Location)
{
    BiomeLocations.Add(Biome, Location);
    UE_LOG(LogTemp, Warning, TEXT("Set biome %s location to %s"), 
           *UEnum::GetValueAsString(Biome), *Location.ToString());
}

void ACrowd_MassEntityManager::TestSpawnEntities()
{
    UE_LOG(LogTemp, Warning, TEXT("Test spawning 500 entities in current biome"));
    SpawnMassEntities(500, CurrentBiome);
}

FVector ACrowd_MassEntityManager::GetBiomeSpawnLocation(ECrowd_BiomeType Biome) const
{
    if (const FVector* Location = BiomeLocations.Find(Biome))
    {
        return *Location;
    }
    
    // Return default location if biome not found
    return FVector::ZeroVector;
}

void ACrowd_MassEntityManager::SetupBiomeLocations()
{
    // Setup biome locations based on game design
    BiomeLocations.Add(ECrowd_BiomeType::Savana, FVector(0, 0, 100));
    BiomeLocations.Add(ECrowd_BiomeType::Forest, FVector(-45000, 40000, 100));
    BiomeLocations.Add(ECrowd_BiomeType::Desert, FVector(55000, 0, 100));
    BiomeLocations.Add(ECrowd_BiomeType::Swamp, FVector(-50000, -45000, 100));
    BiomeLocations.Add(ECrowd_BiomeType::Mountain, FVector(40000, 50000, 500));
}