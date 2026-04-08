#include "DinosaurMassSpawner.h"
#include "MassSpawnerSubsystem.h"
#include "MassEntitySubsystem.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

ADinosaurMassSpawner::ADinosaurMassSpawner()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10 FPS for spawner logic
}

void ADinosaurMassSpawner::BeginPlay()
{
    Super::BeginPlay();
    
    MassSpawnerSubsystem = GetWorld()->GetSubsystem<UMassSpawnerSubsystem>();
    
    if (!MassSpawnerSubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("DinosaurMassSpawner: MassSpawnerSubsystem not found!"));
        return;
    }

    // Initialize default herd configurations if none are set
    if (HerdConfigurations.Num() == 0)
    {
        SetupDefaultHerdConfigurations();
    }

    // Start spawning herds around the world
    InitializeHerdSpawning();
}

void ADinosaurMassSpawner::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bSimulationActive || !MassSpawnerSubsystem)
    {
        return;
    }

    UpdateLODLevels();
    ProcessEntityCulling();
    
    // Spawn new herds if needed
    LastSpawnTime += DeltaTime;
    if (LastSpawnTime >= 5.0f) // Check every 5 seconds
    {
        ConsiderNewHerdSpawning();
        LastSpawnTime = 0.0f;
    }
}

void ADinosaurMassSpawner::SpawnHerd(const FDinosaurHerdConfig& Config, const FVector& Location)
{
    if (!MassSpawnerSubsystem || !DinosaurEntityConfig)
    {
        UE_LOG(LogTemp, Warning, TEXT("DinosaurMassSpawner: Cannot spawn herd - missing subsystem or config"));
        return;
    }

    if (SpawnedEntities.Num() >= MaxSimultaneousEntities)
    {
        UE_LOG(LogTemp, Warning, TEXT("DinosaurMassSpawner: Max entity limit reached"));
        return;
    }

    int32 HerdSize = FMath::RandRange(Config.MinHerdSize, Config.MaxHerdSize);
    
    for (int32 i = 0; i < HerdSize; i++)
    {
        // Calculate spawn position within herd formation
        FVector SpawnOffset = FVector(
            FMath::RandRange(-Config.SpawnRadius, Config.SpawnRadius),
            FMath::RandRange(-Config.SpawnRadius, Config.SpawnRadius),
            0.0f
        );
        
        FVector SpawnLocation = Location + SpawnOffset;
        
        // Ensure spawn location is on ground
        FHitResult HitResult;
        FVector TraceStart = SpawnLocation + FVector(0, 0, 1000);
        FVector TraceEnd = SpawnLocation - FVector(0, 0, 1000);
        
        if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic))
        {
            SpawnLocation = HitResult.Location + FVector(0, 0, 50); // Offset slightly above ground
        }

        // Create spawn parameters
        FMassEntitySpawnDataGeneratorBase SpawnData;
        SpawnData.Transform = FTransform(FRotator::ZeroRotator, SpawnLocation);
        
        // Add herd-specific data
        // This would be expanded to include behavior fragments, AI components, etc.
        
        FMassEntityHandle EntityHandle = MassSpawnerSubsystem->SpawnEntity(DinosaurEntityConfig, SpawnData);
        
        if (EntityHandle.IsValid())
        {
            SpawnedEntities.Add(EntityHandle);
            
            // Set up entity-specific data based on herd type
            SetupEntityBehavior(EntityHandle, Config);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("DinosaurMassSpawner: Spawned herd of %d %s at %s"), 
           HerdSize, 
           *UEnum::GetValueAsString(Config.HerdType), 
           *Location.ToString());
}

void ADinosaurMassSpawner::DespawnEntitiesInRadius(const FVector& Location, float Radius)
{
    if (!MassSpawnerSubsystem)
    {
        return;
    }

    UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
    if (!EntitySubsystem)
    {
        return;
    }

    TArray<FMassEntityHandle> EntitiesToRemove;
    
    for (const FMassEntityHandle& EntityHandle : SpawnedEntities)
    {
        if (!EntityHandle.IsValid())
        {
            EntitiesToRemove.Add(EntityHandle);
            continue;
        }

        // Get entity transform (this would need proper fragment access in real implementation)
        FVector EntityLocation = GetEntityLocation(EntityHandle);
        
        if (FVector::Dist(Location, EntityLocation) <= Radius)
        {
            EntitySubsystem->DestroyEntity(EntityHandle);
            EntitiesToRemove.Add(EntityHandle);
        }
    }

    // Remove despawned entities from tracking array
    for (const FMassEntityHandle& EntityHandle : EntitiesToRemove)
    {
        SpawnedEntities.Remove(EntityHandle);
    }
}

int32 ADinosaurMassSpawner::GetActiveEntityCount() const
{
    return SpawnedEntities.Num();
}

void ADinosaurMassSpawner::SetSimulationActive(bool bActive)
{
    bSimulationActive = bActive;
    
    if (!bActive)
    {
        // Pause all entity processing
        UE_LOG(LogTemp, Log, TEXT("DinosaurMassSpawner: Simulation paused"));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("DinosaurMassSpawner: Simulation resumed"));
    }
}

void ADinosaurMassSpawner::UpdateLODLevels()
{
    FVector PlayerLocation = GetPlayerLocation();
    
    // This would update LOD levels for all entities based on distance from player
    // Implementation would involve updating Mass fragments for LOD management
}

void ADinosaurMassSpawner::ProcessEntityCulling()
{
    FVector PlayerLocation = GetPlayerLocation();
    TArray<FMassEntityHandle> EntitiesToCull;
    
    for (const FMassEntityHandle& EntityHandle : SpawnedEntities)
    {
        if (!EntityHandle.IsValid())
        {
            EntitiesToCull.Add(EntityHandle);
            continue;
        }

        FVector EntityLocation = GetEntityLocation(EntityHandle);
        float Distance = FVector::Dist(PlayerLocation, EntityLocation);
        
        if (Distance > DespawnDistance)
        {
            EntitiesToCull.Add(EntityHandle);
        }
    }

    // Remove culled entities
    for (const FMassEntityHandle& EntityHandle : EntitiesToCull)
    {
        if (UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>())
        {
            EntitySubsystem->DestroyEntity(EntityHandle);
        }
        SpawnedEntities.Remove(EntityHandle);
    }
}

FVector ADinosaurMassSpawner::GetPlayerLocation() const
{
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        if (APawn* PlayerPawn = PC->GetPawn())
        {
            return PlayerPawn->GetActorLocation();
        }
    }
    return FVector::ZeroVector;
}

void ADinosaurMassSpawner::SetupDefaultHerdConfigurations()
{
    // Small Herbivores (Compsognathus-like)
    FDinosaurHerdConfig SmallHerbivores;
    SmallHerbivores.HerdType = EDinosaurHerdType::Herbivore_Small;
    SmallHerbivores.MinHerdSize = 10;
    SmallHerdivores.MaxHerdSize = 30;
    SmallHerbivores.SpawnRadius = 500.0f;
    SmallHerbivores.MovementSpeed = 400.0f;
    SmallHerbivores.TerritoryRadius = 2000.0f;
    SmallHerbivores.bMigratory = true;
    SmallHerbivores.AggressionLevel = 0.1f;
    SmallHerbivores.PredatorTypes = {EDinosaurHerdType::Carnivore_Pack, EDinosaurHerdType::Carnivore_Solo};
    HerdConfigurations.Add(SmallHerbivores);

    // Medium Herbivores (Triceratops-like)
    FDinosaurHerdConfig MediumHerbivores;
    MediumHerbivores.HerdType = EDinosaurHerdType::Herbivore_Medium;
    MediumHerbivores.MinHerdSize = 5;
    MediumHerbivores.MaxHerdSize = 15;
    MediumHerbivores.SpawnRadius = 1000.0f;
    MediumHerbivores.MovementSpeed = 250.0f;
    MediumHerbivores.TerritoryRadius = 3000.0f;
    MediumHerbivores.bMigratory = true;
    MediumHerbivores.AggressionLevel = 0.3f;
    MediumHerbivores.PredatorTypes = {EDinosaurHerdType::Carnivore_Solo};
    HerdConfigurations.Add(MediumHerbivores);

    // Pack Hunters (Velociraptor-like)
    FDinosaurHerdConfig PackHunters;
    PackHunters.HerdType = EDinosaurHerdType::Carnivore_Pack;
    PackHunters.MinHerdSize = 3;
    PackHunters.MaxHerdSize = 8;
    PackHunters.SpawnRadius = 300.0f;
    PackHunters.MovementSpeed = 600.0f;
    PackHunters.TerritoryRadius = 5000.0f;
    PackHunters.bMigratory = false;
    PackHunters.AggressionLevel = 0.8f;
    PackHunters.PreyTypes = {EDinosaurHerdType::Herbivore_Small, EDinosaurHerdType::Herbivore_Medium};
    HerdConfigurations.Add(PackHunters);

    // Solo Predators (T-Rex-like)
    FDinosaurHerdConfig SoloPredators;
    SoloPredators.HerdType = EDinosaurHerdType::Carnivore_Solo;
    SoloPredators.MinHerdSize = 1;
    SoloPredators.MaxHerdSize = 2;
    SoloPredators.SpawnRadius = 100.0f;
    SoloPredators.MovementSpeed = 500.0f;
    SoloPredators.TerritoryRadius = 8000.0f;
    SoloPredators.bMigratory = false;
    SoloPredators.AggressionLevel = 1.0f;
    SoloPredators.PreyTypes = {EDinosaurHerdType::Herbivore_Medium, EDinosaurHerdType::Herbivore_Large};
    HerdConfigurations.Add(SoloPredators);
}

void ADinosaurMassSpawner::InitializeHerdSpawning()
{
    FVector PlayerLocation = GetPlayerLocation();
    
    // Spawn initial herds around the world
    for (const FDinosaurHerdConfig& Config : HerdConfigurations)
    {
        int32 NumHerdsToSpawn = FMath::RandRange(2, 5);
        
        for (int32 i = 0; i < NumHerdsToSpawn; i++)
        {
            // Generate random location within simulation radius
            FVector RandomDirection = FVector(
                FMath::RandRange(-1.0f, 1.0f),
                FMath::RandRange(-1.0f, 1.0f),
                0.0f
            ).GetSafeNormal();
            
            float Distance = FMath::RandRange(1000.0f, SimulationRadius * 0.8f);
            FVector SpawnLocation = PlayerLocation + (RandomDirection * Distance);
            
            SpawnHerd(Config, SpawnLocation);
        }
    }
}

void ADinosaurMassSpawner::ConsiderNewHerdSpawning()
{
    FVector PlayerLocation = GetPlayerLocation();
    
    // Check if we need to spawn new herds at the edge of simulation radius
    for (const FDinosaurHerdConfig& Config : HerdConfigurations)
    {
        int32 CurrentHerdsOfType = CountHerdsOfType(Config.HerdType);
        int32 DesiredHerdsOfType = GetDesiredHerdCount(Config.HerdType);
        
        if (CurrentHerdsOfType < DesiredHerdsOfType)
        {
            // Spawn new herd at edge of simulation radius
            FVector RandomDirection = FVector(
                FMath::RandRange(-1.0f, 1.0f),
                FMath::RandRange(-1.0f, 1.0f),
                0.0f
            ).GetSafeNormal();
            
            FVector SpawnLocation = PlayerLocation + (RandomDirection * SimulationRadius * 0.9f);
            SpawnHerd(Config, SpawnLocation);
        }
    }
}

FVector ADinosaurMassSpawner::GetEntityLocation(const FMassEntityHandle& EntityHandle) const
{
    // This would access the transform fragment of the entity
    // For now, returning zero vector as placeholder
    return FVector::ZeroVector;
}

void ADinosaurMassSpawner::SetupEntityBehavior(const FMassEntityHandle& EntityHandle, const FDinosaurHerdConfig& Config)
{
    // This would set up behavior fragments based on herd type
    // Implementation would involve adding specific fragments for:
    // - Movement behavior
    // - AI state
    // - Interaction rules
    // - Animation state
    // etc.
}

int32 ADinosaurMassSpawner::CountHerdsOfType(EDinosaurHerdType HerdType) const
{
    // This would count existing herds of a specific type
    // Placeholder implementation
    return 0;
}

int32 ADinosaurMassSpawner::GetDesiredHerdCount(EDinosaurHerdType HerdType) const
{
    // Return desired number of herds based on herd type
    switch (HerdType)
    {
        case EDinosaurHerdType::Herbivore_Small:
            return 8;
        case EDinosaurHerdType::Herbivore_Medium:
            return 6;
        case EDinosaurHerdType::Herbivore_Large:
            return 3;
        case EDinosaurHerdType::Carnivore_Pack:
            return 4;
        case EDinosaurHerdType::Carnivore_Solo:
            return 2;
        case EDinosaurHerdType::Scavenger:
            return 5;
        case EDinosaurHerdType::Aquatic:
            return 3;
        default:
            return 1;
    }
}