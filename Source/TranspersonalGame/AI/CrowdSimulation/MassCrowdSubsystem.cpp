#include "MassCrowdSubsystem.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassSimulationSubsystem.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

void UMassCrowdSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    MassEntitySubsystem = Collection.InitializeDependency<UMassEntitySubsystem>();
    
    InitializeBiomeConfigurations();
    RegisterMassProcessors();
    SetupDefaultDensities();
    
    UE_LOG(LogTemp, Warning, TEXT("MassCrowdSubsystem initialized - Ready for prehistoric crowd simulation"));
}

void UMassCrowdSubsystem::Deinitialize()
{
    // Clean up all active herds and packs
    BiomeHerds.Empty();
    BiomePredatorPacks.Empty();
    CurrentActiveAgents = 0;
    
    Super::Deinitialize();
}

bool UMassCrowdSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    return Super::ShouldCreateSubsystem(Outer);
}

void UMassCrowdSubsystem::SpawnHerdInBiome(const FString& BiomeName, const FVector& SpawnLocation, const FDinosaurHerdConfig& HerdConfig)
{
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("MassEntitySubsystem not available for herd spawning"));
        return;
    }

    // Calculate herd size within configured range
    int32 HerdSize = FMath::RandRange(HerdConfig.MinHerdSize, HerdConfig.MaxHerdSize);
    
    // Check if we're within agent limits
    if (CurrentActiveAgents + HerdSize > MaxActiveAgents)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot spawn herd - would exceed max agent limit (%d)"), MaxActiveAgents);
        return;
    }

    TArray<FMassEntityHandle> HerdEntities;
    
    // Spawn herd members in formation
    for (int32 i = 0; i < HerdSize; i++)
    {
        // Create circular formation around spawn point
        float Angle = (2.0f * PI * i) / HerdSize;
        float Distance = FMath::RandRange(100.0f, HerdConfig.HerdCohesionRadius * 0.3f);
        
        FVector MemberLocation = SpawnLocation + FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.0f
        );

        // TODO: Create Mass Entity with appropriate fragments
        // This will be implemented when Mass Entity fragments are defined
        
        CurrentActiveAgents++;
    }

    // Store herd reference
    if (!BiomeHerds.Contains(BiomeName))
    {
        BiomeHerds.Add(BiomeName, TArray<FMassEntityHandle>());
    }
    BiomeHerds[BiomeName].Append(HerdEntities);

    UE_LOG(LogTemp, Log, TEXT("Spawned herd of %d %s in biome %s at location %s"), 
           HerdSize, 
           HerdConfig.DinosaurClass ? *HerdConfig.DinosaurClass->GetName() : TEXT("Unknown"),
           *BiomeName, 
           *SpawnLocation.ToString());
}

void UMassCrowdSubsystem::SpawnPredatorPackInBiome(const FString& BiomeName, const FVector& SpawnLocation, const FPredatorPackConfig& PackConfig)
{
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("MassEntitySubsystem not available for pack spawning"));
        return;
    }

    int32 PackSize = FMath::RandRange(PackConfig.MinPackSize, PackConfig.MaxPackSize);
    
    if (CurrentActiveAgents + PackSize > MaxActiveAgents)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot spawn predator pack - would exceed max agent limit"));
        return;
    }

    TArray<FMassEntityHandle> PackEntities;
    
    // Spawn pack members in loose formation
    for (int32 i = 0; i < PackSize; i++)
    {
        FVector MemberLocation = SpawnLocation + FVector(
            FMath::RandRange(-PackConfig.PackCohesionRadius * 0.5f, PackConfig.PackCohesionRadius * 0.5f),
            FMath::RandRange(-PackConfig.PackCohesionRadius * 0.5f, PackConfig.PackCohesionRadius * 0.5f),
            0.0f
        );

        // TODO: Create Mass Entity with predator-specific fragments
        
        CurrentActiveAgents++;
    }

    // Store pack reference
    if (!BiomePredatorPacks.Contains(BiomeName))
    {
        BiomePredatorPacks.Add(BiomeName, TArray<FMassEntityHandle>());
    }
    BiomePredatorPacks[BiomeName].Append(PackEntities);

    UE_LOG(LogTemp, Log, TEXT("Spawned predator pack of %d %s in biome %s"), 
           PackSize, 
           PackConfig.PredatorClass ? *PackConfig.PredatorClass->GetName() : TEXT("Unknown"),
           *BiomeName);
}

void UMassCrowdSubsystem::TriggerMigrationEvent(const FString& BiomeName, const FVector& MigrationTarget)
{
    if (!BiomeHerds.Contains(BiomeName))
    {
        UE_LOG(LogTemp, Warning, TEXT("No herds found in biome %s for migration"), *BiomeName);
        return;
    }

    // Set migration target for all herds in the biome
    for (const FMassEntityHandle& HerdEntity : BiomeHerds[BiomeName])
    {
        // TODO: Update Mass Entity with migration target
        // This will set the FMassMoveTargetFragment to the migration destination
    }

    UE_LOG(LogTemp, Log, TEXT("Triggered migration event in biome %s towards %s"), 
           *BiomeName, 
           *MigrationTarget.ToString());
}

void UMassCrowdSubsystem::TriggerPanicResponse(const FVector& ThreatLocation, float PanicRadius, float PanicIntensity)
{
    // Find all entities within panic radius and trigger flee behavior
    int32 AffectedAgents = 0;
    
    // TODO: Query Mass Entity system for all entities within radius
    // Apply panic behavior modification to affected entities
    
    UE_LOG(LogTemp, Log, TEXT("Triggered panic response at %s - Radius: %.1f, Intensity: %.2f, Affected: %d agents"), 
           *ThreatLocation.ToString(), 
           PanicRadius, 
           PanicIntensity, 
           AffectedAgents);
}

void UMassCrowdSubsystem::SetBiomeDensityConfig(const FBiomeCrowdDensity& DensityConfig)
{
    BiomeDensityConfigs.Add(DensityConfig.BiomeName, DensityConfig);
    
    UE_LOG(LogTemp, Log, TEXT("Updated density configuration for biome: %s"), *DensityConfig.BiomeName);
}

FBiomeCrowdDensity UMassCrowdSubsystem::GetBiomeDensityConfig(const FString& BiomeName) const
{
    if (const FBiomeCrowdDensity* Config = BiomeDensityConfigs.Find(BiomeName))
    {
        return *Config;
    }
    
    // Return default configuration
    FBiomeCrowdDensity DefaultConfig;
    DefaultConfig.BiomeName = BiomeName;
    return DefaultConfig;
}

int32 UMassCrowdSubsystem::GetActiveAgentCount() const
{
    return CurrentActiveAgents;
}

int32 UMassCrowdSubsystem::GetAgentCountInRadius(const FVector& Center, float Radius) const
{
    // TODO: Query Mass Entity system for entities within radius
    // For now, return estimated count
    return 0;
}

TArray<FVector> UMassCrowdSubsystem::GetNearbyHerdCenters(const FVector& Location, float SearchRadius) const
{
    TArray<FVector> HerdCenters;
    
    // TODO: Calculate center positions of nearby herds
    // This will be used by other systems to understand crowd distribution
    
    return HerdCenters;
}

void UMassCrowdSubsystem::InitializeBiomeConfigurations()
{
    // This will be populated with data from the Environment Artist and World Generator
    UE_LOG(LogTemp, Log, TEXT("Initializing biome configurations for crowd simulation"));
}

void UMassCrowdSubsystem::RegisterMassProcessors()
{
    // Register custom Mass processors for dinosaur behavior
    // This will include herd movement, predator hunting, and panic responses
    UE_LOG(LogTemp, Log, TEXT("Registering Mass processors for dinosaur crowd simulation"));
}

void UMassCrowdSubsystem::SetupDefaultDensities()
{
    // Forest Biome Configuration
    FBiomeCrowdDensity ForestDensity;
    ForestDensity.BiomeName = TEXT("Forest");
    ForestDensity.HerbivoreAgentsPerKm2 = 75.0f;
    ForestDensity.CarnivoreAgentsPerKm2 = 8.0f;
    ForestDensity.SmallCreaturesPerKm2 = 300.0f;
    BiomeDensityConfigs.Add(ForestDensity.BiomeName, ForestDensity);

    // Plains Biome Configuration
    FBiomeCrowdDensity PlainsDensity;
    PlainsDensity.BiomeName = TEXT("Plains");
    PlainsDensity.HerbivoreAgentsPerKm2 = 120.0f;
    PlainsDensity.CarnivoreAgentsPerKm2 = 12.0f;
    PlainsDensity.SmallCreaturesPerKm2 = 150.0f;
    BiomeDensityConfigs.Add(PlainsDensity.BiomeName, PlainsDensity);

    // Swamp Biome Configuration
    FBiomeCrowdDensity SwampDensity;
    SwampDensity.BiomeName = TEXT("Swamp");
    SwampDensity.HerbivoreAgentsPerKm2 = 40.0f;
    SwampDensity.CarnivoreAgentsPerKm2 = 15.0f;
    SwampDensity.SmallCreaturesPerKm2 = 400.0f;
    BiomeDensityConfigs.Add(SwampDensity.BiomeName, SwampDensity);

    // Mountain Biome Configuration
    FBiomeCrowdDensity MountainDensity;
    MountainDensity.BiomeName = TEXT("Mountain");
    MountainDensity.HerbivoreAgentsPerKm2 = 25.0f;
    MountainDensity.CarnivoreAgentsPerKm2 = 5.0f;
    MountainDensity.SmallCreaturesPerKm2 = 100.0f;
    BiomeDensityConfigs.Add(MountainDensity.BiomeName, MountainDensity);

    UE_LOG(LogTemp, Log, TEXT("Setup default density configurations for %d biomes"), BiomeDensityConfigs.Num());
}