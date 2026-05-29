#include "Crowd_MassSimulationSubsystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassRepresentationFragments.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"

UCrowd_MassSimulationSubsystem::UCrowd_MassSimulationSubsystem()
{
    bCrowdSimulationEnabled = true;
    SimulationUpdateRate = 0.1f;
    MaxTotalCrowdEntities = 50000;
    CrowdEntityLifespan = 300.0f;
    CurrentEntityCount = 0;
    LastUpdateTime = 0.0f;
}

void UCrowd_MassSimulationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd Mass Simulation Subsystem Initialized"));
    
    // Initialize default biome configurations
    BiomeConfigurations.Empty();
    
    // Savana biome
    FCrowd_BiomeConfig SavanaConfig;
    SavanaConfig.BiomeType = EBiomeType::Savana;
    SavanaConfig.BiomeCenter = FVector(0.0f, 0.0f, 100.0f);
    SavanaConfig.BiomeRadius = 5000.0f;
    SavanaConfig.MaxCrowdEntities = 2000;
    SavanaConfig.EntityDensity = 0.15f;
    BiomeConfigurations.Add(SavanaConfig);
    
    // Forest biome
    FCrowd_BiomeConfig ForestConfig;
    ForestConfig.BiomeType = EBiomeType::Forest;
    ForestConfig.BiomeCenter = FVector(-45000.0f, 40000.0f, 100.0f);
    ForestConfig.BiomeRadius = 4000.0f;
    ForestConfig.MaxCrowdEntities = 1500;
    ForestConfig.EntityDensity = 0.12f;
    BiomeConfigurations.Add(ForestConfig);
    
    // Desert biome
    FCrowd_BiomeConfig DesertConfig;
    DesertConfig.BiomeType = EBiomeType::Desert;
    DesertConfig.BiomeCenter = FVector(55000.0f, 0.0f, 100.0f);
    DesertConfig.BiomeRadius = 6000.0f;
    DesertConfig.MaxCrowdEntities = 800;
    DesertConfig.EntityDensity = 0.08f;
    BiomeConfigurations.Add(DesertConfig);
    
    // Initialize Mass Entity system
    InitializeMassEntitySystem();
}

void UCrowd_MassSimulationSubsystem::Deinitialize()
{
    ClearAllCrowdEntities();
    MassEntitySubsystem = nullptr;
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd Mass Simulation Subsystem Deinitialized"));
    
    Super::Deinitialize();
}

bool UCrowd_MassSimulationSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    return Super::ShouldCreateSubsystem(Outer);
}

void UCrowd_MassSimulationSubsystem::InitializeMassEntitySystem()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Crowd Simulation: No valid world found"));
        return;
    }
    
    MassEntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("Crowd Simulation: Mass Entity Subsystem not available"));
        return;
    }
    
    CreateMassEntityArchetype();
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd Simulation: Mass Entity system initialized"));
}

void UCrowd_MassSimulationSubsystem::CreateMassEntityArchetype()
{
    if (!MassEntitySubsystem)
    {
        return;
    }
    
    // Create archetype with basic fragments for crowd entities
    FMassArchetypeCompositionDescriptor Descriptor;
    Descriptor.Fragments.Add<FTransformFragment>();
    Descriptor.Fragments.Add<FMassVelocityFragment>();
    Descriptor.Fragments.Add<FMassForceFragment>();
    
    CrowdArchetype = MassEntitySubsystem->CreateArchetype(Descriptor);
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd Simulation: Mass Entity archetype created"));
}

void UCrowd_MassSimulationSubsystem::SpawnCrowdEntitiesInBiome(EBiomeType BiomeType, int32 EntityCount)
{
    if (!MassEntitySubsystem || !bCrowdSimulationEnabled)
    {
        return;
    }
    
    FVector BiomeCenter = GetBiomeCenter(BiomeType);
    float SpawnRadius = 2000.0f;
    
    for (int32 i = 0; i < EntityCount && CurrentEntityCount < MaxTotalCrowdEntities; i++)
    {
        // Generate random position within biome
        float Angle = FMath::RandRange(0.0f, 2.0f * PI);
        float Distance = FMath::RandRange(100.0f, SpawnRadius);
        
        FVector SpawnLocation = BiomeCenter + FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            FMath::RandRange(-50.0f, 50.0f)
        );
        
        SpawnEntityAtLocation(SpawnLocation, BiomeType);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd Simulation: Spawned %d entities in biome %d"), EntityCount, (int32)BiomeType);
}

void UCrowd_MassSimulationSubsystem::SpawnEntityAtLocation(const FVector& Location, EBiomeType BiomeType)
{
    if (!MassEntitySubsystem)
    {
        return;
    }
    
    // Create new Mass Entity
    FMassEntityHandle Entity = MassEntitySubsystem->CreateEntity(CrowdArchetype);
    if (!Entity.IsValid())
    {
        return;
    }
    
    // Set initial transform
    FTransformFragment* TransformFragment = MassEntitySubsystem->GetFragmentDataPtr<FTransformFragment>(Entity);
    if (TransformFragment)
    {
        TransformFragment->GetMutableTransform().SetLocation(Location);
        TransformFragment->GetMutableTransform().SetRotation(FQuat::Identity);
        TransformFragment->GetMutableTransform().SetScale3D(FVector::OneVector);
    }
    
    // Set initial velocity
    FMassVelocityFragment* VelocityFragment = MassEntitySubsystem->GetFragmentDataPtr<FMassVelocityFragment>(Entity);
    if (VelocityFragment)
    {
        VelocityFragment->Value = FVector::ZeroVector;
    }
    
    // Create entity config
    FCrowd_EntityConfig EntityConfig;
    EntityConfig.SpawnLocation = Location;
    EntityConfig.BiomeType = BiomeType;
    EntityConfig.MovementSpeed = FMath::RandRange(100.0f, 200.0f);
    EntityConfig.WanderRadius = FMath::RandRange(500.0f, 1500.0f);
    EntityConfig.bIsActive = true;
    
    ActiveCrowdEntities.Add(EntityConfig);
    CurrentEntityCount++;
}

void UCrowd_MassSimulationSubsystem::UpdateCrowdSimulation(float DeltaTime)
{
    if (!bCrowdSimulationEnabled || !MassEntitySubsystem)
    {
        return;
    }
    
    LastUpdateTime += DeltaTime;
    
    // Update at specified rate
    if (LastUpdateTime >= SimulationUpdateRate)
    {
        UpdateEntityMovement(LastUpdateTime);
        CleanupExpiredEntities();
        LastUpdateTime = 0.0f;
    }
}

void UCrowd_MassSimulationSubsystem::UpdateEntityMovement(float DeltaTime)
{
    // Mass Entity movement updates would be handled by Mass processors
    // This is a simplified update for demonstration
    
    for (FCrowd_EntityConfig& EntityConfig : ActiveCrowdEntities)
    {
        if (!EntityConfig.bIsActive)
        {
            continue;
        }
        
        // Simple wander behavior
        FVector RandomDirection = FVector(
            FMath::RandRange(-1.0f, 1.0f),
            FMath::RandRange(-1.0f, 1.0f),
            0.0f
        ).GetSafeNormal();
        
        FVector NewLocation = EntityConfig.SpawnLocation + RandomDirection * EntityConfig.MovementSpeed * DeltaTime;
        EntityConfig.SpawnLocation = NewLocation;
    }
}

void UCrowd_MassSimulationSubsystem::CleanupExpiredEntities()
{
    // Remove entities that are too far from their biome or inactive
    ActiveCrowdEntities.RemoveAll([this](const FCrowd_EntityConfig& EntityConfig)
    {
        if (!EntityConfig.bIsActive)
        {
            CurrentEntityCount--;
            return true;
        }
        
        FVector BiomeCenter = GetBiomeCenter(EntityConfig.BiomeType);
        float Distance = FVector::Dist(EntityConfig.SpawnLocation, BiomeCenter);
        
        if (Distance > EntityConfig.WanderRadius * 2.0f)
        {
            CurrentEntityCount--;
            return true;
        }
        
        return false;
    });
}

void UCrowd_MassSimulationSubsystem::SetBiomeConfiguration(const TArray<FCrowd_BiomeConfig>& BiomeConfigs)
{
    BiomeConfigurations = BiomeConfigs;
    UE_LOG(LogTemp, Warning, TEXT("Crowd Simulation: Updated biome configurations"));
}

int32 UCrowd_MassSimulationSubsystem::GetActiveCrowdEntityCount() const
{
    return CurrentEntityCount;
}

void UCrowd_MassSimulationSubsystem::EnableCrowdSimulation(bool bEnabled)
{
    bCrowdSimulationEnabled = bEnabled;
    UE_LOG(LogTemp, Warning, TEXT("Crowd Simulation: %s"), bEnabled ? TEXT("Enabled") : TEXT("Disabled"));
}

void UCrowd_MassSimulationSubsystem::ClearAllCrowdEntities()
{
    ActiveCrowdEntities.Empty();
    CurrentEntityCount = 0;
    UE_LOG(LogTemp, Warning, TEXT("Crowd Simulation: All entities cleared"));
}

void UCrowd_MassSimulationSubsystem::ConfigureSavanaCrowd()
{
    SpawnCrowdEntitiesInBiome(EBiomeType::Savana, 500);
}

void UCrowd_MassSimulationSubsystem::ConfigureForestCrowd()
{
    SpawnCrowdEntitiesInBiome(EBiomeType::Forest, 300);
}

void UCrowd_MassSimulationSubsystem::ConfigureDesertCrowd()
{
    SpawnCrowdEntitiesInBiome(EBiomeType::Desert, 200);
}

FVector UCrowd_MassSimulationSubsystem::GetBiomeCenter(EBiomeType BiomeType) const
{
    for (const FCrowd_BiomeConfig& Config : BiomeConfigurations)
    {
        if (Config.BiomeType == BiomeType)
        {
            return Config.BiomeCenter;
        }
    }
    
    // Default fallback coordinates
    switch (BiomeType)
    {
        case EBiomeType::Savana:
            return FVector(0.0f, 0.0f, 100.0f);
        case EBiomeType::Forest:
            return FVector(-45000.0f, 40000.0f, 100.0f);
        case EBiomeType::Desert:
            return FVector(55000.0f, 0.0f, 100.0f);
        case EBiomeType::Mountain:
            return FVector(40000.0f, 50000.0f, 100.0f);
        case EBiomeType::Swamp:
            return FVector(-50000.0f, -45000.0f, 100.0f);
        default:
            return FVector::ZeroVector;
    }
}