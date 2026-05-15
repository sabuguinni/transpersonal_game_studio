#include "Crowd_MassEntitySubsystem.h"
#include "Engine/World.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassRepresentationFragments.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"

UCrowd_MassEntitySubsystem::UCrowd_MassEntitySubsystem()
{
    MassEntitySubsystem = nullptr;
    bIsInitialized = false;
    LastUpdateTime = 0.0f;
    MaxEntitiesPerFrame = 100;
    CurrentFrameEntityCount = 0;
}

void UCrowd_MassEntitySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("UCrowd_MassEntitySubsystem::Initialize - Starting crowd system initialization"));
    
    // Get Mass Entity Subsystem
    if (UWorld* World = GetWorld())
    {
        MassEntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
        if (MassEntitySubsystem)
        {
            UE_LOG(LogTemp, Warning, TEXT("UCrowd_MassEntitySubsystem::Initialize - MassEntitySubsystem found"));
            InitializeCrowdSystem();
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("UCrowd_MassEntitySubsystem::Initialize - Failed to get MassEntitySubsystem"));
        }
    }
}

void UCrowd_MassEntitySubsystem::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("UCrowd_MassEntitySubsystem::Deinitialize - Cleaning up crowd system"));
    
    // Clean up active entities
    if (MassEntitySubsystem && ActiveCrowdEntities.Num() > 0)
    {
        for (const FMassEntityHandle& EntityHandle : ActiveCrowdEntities)
        {
            if (MassEntitySubsystem->IsEntityValid(EntityHandle))
            {
                MassEntitySubsystem->DestroyEntity(EntityHandle);
            }
        }
        ActiveCrowdEntities.Empty();
    }
    
    // Clear spawners
    BiomeSpawners.Empty();
    RegisteredBiomes.Empty();
    
    bIsInitialized = false;
    Super::Deinitialize();
}

bool UCrowd_MassEntitySubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    return Super::ShouldCreateSubsystem(Outer);
}

void UCrowd_MassEntitySubsystem::InitializeCrowdSystem()
{
    if (bIsInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("UCrowd_MassEntitySubsystem::InitializeCrowdSystem - Already initialized"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("UCrowd_MassEntitySubsystem::InitializeCrowdSystem - Setting up crowd system"));
    
    // Initialize default configuration
    CrowdConfig = FCrowd_EntityConfig();
    
    // Create default biomes
    CreateDefaultBiomes();
    
    // Initialize Mass Entity archetype
    InitializeMassEntityArchetype();
    
    bIsInitialized = true;
    UE_LOG(LogTemp, Warning, TEXT("UCrowd_MassEntitySubsystem::InitializeCrowdSystem - Crowd system initialized successfully"));
}

void UCrowd_MassEntitySubsystem::CreateDefaultBiomes()
{
    // Clear existing biomes
    RegisteredBiomes.Empty();
    
    // Savana (center) - coordinates from memory
    FCrowd_BiomeSettings SavanaBiome;
    SavanaBiome.BiomeName = TEXT("Savana");
    SavanaBiome.BiomeCenter = FVector(0.0f, 0.0f, 100.0f);
    SavanaBiome.BiomeRadius = 15000.0f;
    SavanaBiome.TargetPopulation = 800;
    RegisteredBiomes.Add(SavanaBiome);
    
    // Pantano (southwest) - coordinates from memory
    FCrowd_BiomeSettings PantanoBiome;
    PantanoBiome.BiomeName = TEXT("Pantano");
    PantanoBiome.BiomeCenter = FVector(-50000.0f, -45000.0f, 100.0f);
    PantanoBiome.BiomeRadius = 12000.0f;
    PantanoBiome.TargetPopulation = 600;
    RegisteredBiomes.Add(PantanoBiome);
    
    // Floresta (northwest) - coordinates from memory
    FCrowd_BiomeSettings FlorestaBiome;
    FlorestaBiome.BiomeName = TEXT("Floresta");
    FlorestaBiome.BiomeCenter = FVector(-45000.0f, 40000.0f, 100.0f);
    FlorestaBiome.BiomeRadius = 13000.0f;
    FlorestaBiome.TargetPopulation = 700;
    RegisteredBiomes.Add(FlorestaBiome);
    
    // Deserto (east) - coordinates from memory
    FCrowd_BiomeSettings DesertoBiome;
    DesertoBiome.BiomeName = TEXT("Deserto");
    DesertoBiome.BiomeCenter = FVector(55000.0f, 0.0f, 100.0f);
    DesertoBiome.BiomeRadius = 11000.0f;
    DesertoBiome.TargetPopulation = 400;
    RegisteredBiomes.Add(DesertoBiome);
    
    // Montanha (northeast) - coordinates from memory
    FCrowd_BiomeSettings MontanhaBiome;
    MontanhaBiome.BiomeName = TEXT("Montanha");
    MontanhaBiome.BiomeCenter = FVector(40000.0f, 50000.0f, 500.0f);
    MontanhaBiome.BiomeRadius = 10000.0f;
    MontanhaBiome.TargetPopulation = 300;
    RegisteredBiomes.Add(MontanhaBiome);
    
    UE_LOG(LogTemp, Warning, TEXT("UCrowd_MassEntitySubsystem::CreateDefaultBiomes - Created %d default biomes"), RegisteredBiomes.Num());
}

void UCrowd_MassEntitySubsystem::InitializeMassEntityArchetype()
{
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("UCrowd_MassEntitySubsystem::InitializeMassEntityArchetype - MassEntitySubsystem is null"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("UCrowd_MassEntitySubsystem::InitializeMassEntityArchetype - Setting up Mass Entity archetype"));
    
    // Note: In a full implementation, we would create a proper archetype here
    // For now, we'll handle entity creation in the spawn methods
}

void UCrowd_MassEntitySubsystem::SpawnCrowdEntitiesInBiome(const FCrowd_BiomeSettings& BiomeSettings)
{
    if (!MassEntitySubsystem || !bIsInitialized)
    {
        UE_LOG(LogTemp, Error, TEXT("UCrowd_MassEntitySubsystem::SpawnCrowdEntitiesInBiome - System not initialized"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("UCrowd_MassEntitySubsystem::SpawnCrowdEntitiesInBiome - Spawning %d entities in %s"), 
           BiomeSettings.TargetPopulation, *BiomeSettings.BiomeName);
    
    int32 EntitiesToSpawn = FMath::Min(BiomeSettings.TargetPopulation, CrowdConfig.MaxEntities - ActiveCrowdEntities.Num());
    
    for (int32 i = 0; i < EntitiesToSpawn; ++i)
    {
        // Generate random position within biome radius
        float Angle = FMath::RandRange(0.0f, 2.0f * PI);
        float Distance = FMath::RandRange(0.0f, BiomeSettings.BiomeRadius);
        
        FVector SpawnLocation = BiomeSettings.BiomeCenter + FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.0f
        );
        
        FMassEntityHandle NewEntity = SpawnCrowdEntity(SpawnLocation, BiomeSettings);
        if (NewEntity.IsValid())
        {
            ActiveCrowdEntities.Add(NewEntity);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("UCrowd_MassEntitySubsystem::SpawnCrowdEntitiesInBiome - Spawned %d entities, total active: %d"), 
           EntitiesToSpawn, ActiveCrowdEntities.Num());
}

FMassEntityHandle UCrowd_MassEntitySubsystem::SpawnCrowdEntity(const FVector& Location, const FCrowd_BiomeSettings& BiomeSettings)
{
    if (!MassEntitySubsystem)
    {
        return FMassEntityHandle();
    }
    
    // Create entity with basic fragments
    FMassEntityHandle EntityHandle = MassEntitySubsystem->CreateEntity();
    
    if (EntityHandle.IsValid())
    {
        // Add transform fragment
        FTransformFragment TransformFragment;
        TransformFragment.GetMutableTransform().SetLocation(Location);
        MassEntitySubsystem->AddFragmentToEntity(EntityHandle, TransformFragment);
        
        // Add velocity fragment for movement
        FMassVelocityFragment VelocityFragment;
        VelocityFragment.Value = FVector::ZeroVector;
        MassEntitySubsystem->AddFragmentToEntity(EntityHandle, VelocityFragment);
    }
    
    return EntityHandle;
}

void UCrowd_MassEntitySubsystem::UpdateCrowdBehavior(float DeltaTime)
{
    if (!bIsInitialized || !MassEntitySubsystem)
    {
        return;
    }
    
    LastUpdateTime += DeltaTime;
    
    // Update at 10Hz for performance
    if (LastUpdateTime < 0.1f)
    {
        return;
    }
    
    // Process entities in batches for performance
    int32 EntitiesProcessed = 0;
    for (const FMassEntityHandle& EntityHandle : ActiveCrowdEntities)
    {
        if (EntitiesProcessed >= MaxEntitiesPerFrame)
        {
            break;
        }
        
        if (MassEntitySubsystem->IsEntityValid(EntityHandle))
        {
            UpdateEntityMovement(EntityHandle, LastUpdateTime);
            EntitiesProcessed++;
        }
    }
    
    LastUpdateTime = 0.0f;
}

void UCrowd_MassEntitySubsystem::UpdateEntityMovement(FMassEntityHandle EntityHandle, float DeltaTime)
{
    if (!MassEntitySubsystem || !EntityHandle.IsValid())
    {
        return;
    }
    
    // Get current transform
    if (FTransformFragment* TransformFragment = MassEntitySubsystem->GetFragmentDataPtr<FTransformFragment>(EntityHandle))
    {
        FTransform& Transform = TransformFragment->GetMutableTransform();
        
        // Simple wandering behavior
        FVector CurrentLocation = Transform.GetLocation();
        FVector RandomDirection = FVector(
            FMath::RandRange(-1.0f, 1.0f),
            FMath::RandRange(-1.0f, 1.0f),
            0.0f
        ).GetSafeNormal();
        
        FVector NewLocation = CurrentLocation + (RandomDirection * CrowdConfig.MovementSpeed * DeltaTime);
        Transform.SetLocation(NewLocation);
    }
}

void UCrowd_MassEntitySubsystem::HandleCombatDetection(const FVector& CombatLocation, float CombatRadius)
{
    if (!bIsInitialized || !MassEntitySubsystem)
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("UCrowd_MassEntitySubsystem::HandleCombatDetection - Combat detected at %s, radius %f"), 
           *CombatLocation.ToString(), CombatRadius);
    
    // Process combat avoidance for all entities
    for (const FMassEntityHandle& EntityHandle : ActiveCrowdEntities)
    {
        if (MassEntitySubsystem->IsEntityValid(EntityHandle))
        {
            ProcessCombatAvoidance(EntityHandle, CombatLocation, CombatRadius);
        }
    }
}

void UCrowd_MassEntitySubsystem::ProcessCombatAvoidance(FMassEntityHandle EntityHandle, const FVector& CombatLocation, float CombatRadius)
{
    if (!MassEntitySubsystem || !EntityHandle.IsValid())
    {
        return;
    }
    
    if (FTransformFragment* TransformFragment = MassEntitySubsystem->GetFragmentDataPtr<FTransformFragment>(EntityHandle))
    {
        FVector EntityLocation = TransformFragment->GetTransform().GetLocation();
        float DistanceToCombat = FVector::Dist(EntityLocation, CombatLocation);
        
        // If within combat radius, flee
        if (DistanceToCombat < CombatRadius + CrowdConfig.CombatFleeDistance)
        {
            FVector FleeDirection = (EntityLocation - CombatLocation).GetSafeNormal();
            FVector FleeLocation = EntityLocation + (FleeDirection * CrowdConfig.CombatFleeDistance);
            
            FTransform& Transform = TransformFragment->GetMutableTransform();
            Transform.SetLocation(FleeLocation);
            
            UE_LOG(LogTemp, Log, TEXT("UCrowd_MassEntitySubsystem::ProcessCombatAvoidance - Entity fleeing from combat"));
        }
    }
}

int32 UCrowd_MassEntitySubsystem::GetActiveCrowdEntityCount() const
{
    return ActiveCrowdEntities.Num();
}

void UCrowd_MassEntitySubsystem::SetCrowdConfiguration(const FCrowd_EntityConfig& NewConfig)
{
    CrowdConfig = NewConfig;
    UE_LOG(LogTemp, Warning, TEXT("UCrowd_MassEntitySubsystem::SetCrowdConfiguration - Updated crowd config, max entities: %d"), 
           CrowdConfig.MaxEntities);
}

FCrowd_EntityConfig UCrowd_MassEntitySubsystem::GetCrowdConfiguration() const
{
    return CrowdConfig;
}

void UCrowd_MassEntitySubsystem::RegisterBiome(const FCrowd_BiomeSettings& BiomeSettings)
{
    RegisteredBiomes.Add(BiomeSettings);
    UE_LOG(LogTemp, Warning, TEXT("UCrowd_MassEntitySubsystem::RegisterBiome - Registered biome: %s"), *BiomeSettings.BiomeName);
}

void UCrowd_MassEntitySubsystem::PopulateAllBiomes()
{
    if (!bIsInitialized)
    {
        UE_LOG(LogTemp, Error, TEXT("UCrowd_MassEntitySubsystem::PopulateAllBiomes - System not initialized"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("UCrowd_MassEntitySubsystem::PopulateAllBiomes - Populating %d biomes"), RegisteredBiomes.Num());
    
    for (const FCrowd_BiomeSettings& BiomeSettings : RegisteredBiomes)
    {
        SpawnCrowdEntitiesInBiome(BiomeSettings);
    }
}

TArray<FCrowd_BiomeSettings> UCrowd_MassEntitySubsystem::GetRegisteredBiomes() const
{
    return RegisteredBiomes;
}