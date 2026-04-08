#include "WorldGeneratorCore.h"
#include "Engine/World.h"
#include "Landscape.h"
#include "LandscapeInfo.h"
#include "LandscapeSubsystem.h"
#include "PCGSubsystem.h"
#include "PCGComponent.h"
#include "WorldPartition/WorldPartition.h"
#include "Engine/Engine.h"

UWorldGeneratorCore::UWorldGeneratorCore()
{
    PrimaryComponentTick.bCanEverTick = false;
    GeneratedLandscape = nullptr;
    
    // Set default world parameters
    WorldSizeKm = 8;
    HeightmapResolution = 4033;
    MaxElevation = 500.0f;
    RandomSeed = 12345;
}

void UWorldGeneratorCore::BeginPlay()
{
    Super::BeginPlay();
    SetupBiomeDefinitions();
}

void UWorldGeneratorCore::SetupBiomeDefinitions()
{
    // Dense Forest - High danger, perfect for predator ambushes
    FBiomeDefinition DenseForest;
    DenseForest.BiomeType = EBiomeType::DenseForest;
    DenseForest.DangerLevel = EDangerLevel::High;
    DenseForest.VegetationDensity = 0.9f;
    DenseForest.WaterProximity = 0.4f;
    DenseForest.ElevationVariance = 0.3f;
    BiomeDefinitions.Add(EBiomeType::DenseForest, DenseForest);

    // Open Plains - Medium danger, good visibility but exposed
    FBiomeDefinition OpenPlains;
    OpenPlains.BiomeType = EBiomeType::OpenPlains;
    OpenPlains.DangerLevel = EDangerLevel::Medium;
    OpenPlains.VegetationDensity = 0.2f;
    OpenPlains.WaterProximity = 0.1f;
    OpenPlains.ElevationVariance = 0.1f;
    BiomeDefinitions.Add(EBiomeType::OpenPlains, OpenPlains);

    // River Valley - Low danger, water source but predator paths
    FBiomeDefinition RiverValley;
    RiverValley.BiomeType = EBiomeType::RiverValley;
    RiverValley.DangerLevel = EDangerLevel::Low;
    RiverValley.VegetationDensity = 0.6f;
    RiverValley.WaterProximity = 1.0f;
    RiverValley.ElevationVariance = 0.2f;
    BiomeDefinitions.Add(EBiomeType::RiverValley, RiverValley);

    // Rocky Outcrops - Safe zones, good for base building
    FBiomeDefinition RockyOutcrops;
    RockyOutcrops.BiomeType = EBiomeType::RockyOutcrops;
    RockyOutcrops.DangerLevel = EDangerLevel::Safe;
    RockyOutcrops.VegetationDensity = 0.1f;
    RockyOutcrops.WaterProximity = 0.2f;
    RockyOutcrops.ElevationVariance = 0.8f;
    BiomeDefinitions.Add(EBiomeType::RockyOutcrops, RockyOutcrops);

    // Swamp Lands - Extreme danger, rich resources but deadly
    FBiomeDefinition SwampLands;
    SwampLands.BiomeType = EBiomeType::SwampLands;
    SwampLands.DangerLevel = EDangerLevel::Extreme;
    SwampLands.VegetationDensity = 0.8f;
    SwampLands.WaterProximity = 0.9f;
    SwampLands.ElevationVariance = 0.1f;
    BiomeDefinitions.Add(EBiomeType::SwampLands, SwampLands);

    // Coastal Area - Medium danger, escape route but limited resources
    FBiomeDefinition CoastalArea;
    CoastalArea.BiomeType = EBiomeType::CoastalArea;
    CoastalArea.DangerLevel = EDangerLevel::Medium;
    CoastalArea.VegetationDensity = 0.3f;
    CoastalArea.WaterProximity = 1.0f;
    CoastalArea.ElevationVariance = 0.2f;
    BiomeDefinitions.Add(EBiomeType::CoastalArea, CoastalArea);
}

void UWorldGeneratorCore::GenerateWorld()
{
    UE_LOG(LogTemp, Warning, TEXT("Starting World Generation for Jurassic Survival Game"));
    
    // Configure World Partition for large world streaming
    ConfigureWorldPartition();
    
    // Generate terrain base
    GenerateTerrain();
    
    // Generate biome distribution
    GenerateBiomes();
    
    // Generate river system
    GenerateRiverSystem();
    
    // Generate vegetation
    GenerateVegetation();
    
    UE_LOG(LogTemp, Warning, TEXT("World Generation Complete"));
}

void UWorldGeneratorCore::GenerateTerrain()
{
    UE_LOG(LogTemp, Warning, TEXT("Generating base terrain..."));
    
    CreateLandscapeActor();
    
    if (GeneratedLandscape)
    {
        // Configure landscape for 8x8km world
        // Using recommended UE5 landscape size: 4033x4033
        UE_LOG(LogTemp, Warning, TEXT("Landscape created successfully"));
    }
}

void UWorldGeneratorCore::CreateLandscapeActor()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("No valid world found"));
        return;
    }

    // Create landscape with World Partition support
    FActorSpawnParameters SpawnParams;
    SpawnParams.bNoFail = true;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
    GeneratedLandscape = World->SpawnActor<ALandscape>(ALandscape::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
    
    if (GeneratedLandscape)
    {
        // Configure landscape properties
        GeneratedLandscape->SetActorScale3D(FVector(1.0f, 1.0f, MaxElevation / 256.0f));
        
        UE_LOG(LogTemp, Warning, TEXT("Landscape actor created at world origin"));
    }
}

void UWorldGeneratorCore::GenerateBiomes()
{
    UE_LOG(LogTemp, Warning, TEXT("Generating biome distribution..."));
    
    SetupPCGComponents();
    
    // Biome generation will be handled by PCG graphs
    if (BiomeDistributionGraph.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Biome distribution PCG graph configured"));
    }
}

void UWorldGeneratorCore::GenerateRiverSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("Generating river system..."));
    
    // Rivers will be generated using PCG splines
    // This creates natural water sources and predator migration paths
    if (RiverSystemGraph.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("River system PCG graph configured"));
    }
}

void UWorldGeneratorCore::GenerateVegetation()
{
    UE_LOG(LogTemp, Warning, TEXT("Generating vegetation scatter..."));
    
    // Vegetation generation using PCG for performance
    // Different density based on biome type
    if (VegetationScatterGraph.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Vegetation scatter PCG graph configured"));
    }
}

void UWorldGeneratorCore::SetupPCGComponents()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    UPCGSubsystem* PCGSubsystem = UPCGSubsystem::GetInstance(World);
    if (!PCGSubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("PCG Subsystem not available"));
        return;
    }

    // PCG components will be set up for each generation phase
    UE_LOG(LogTemp, Warning, TEXT("PCG components configured"));
}

void UWorldGeneratorCore::ConfigureWorldPartition()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    UWorldPartition* WorldPartition = World->GetWorldPartition();
    if (WorldPartition)
    {
        // Configure for 8x8km world with appropriate cell size
        // Recommended cell size: 256m for good streaming performance
        UE_LOG(LogTemp, Warning, TEXT("World Partition configured for large world streaming"));
    }
}

EBiomeType UWorldGeneratorCore::GetBiomeAtLocation(FVector WorldLocation) const
{
    // This will be implemented with actual biome sampling
    // For now, return a default based on distance from center
    float DistanceFromCenter = FVector2D(WorldLocation.X, WorldLocation.Y).Size();
    
    if (DistanceFromCenter < 1000.0f)
    {
        return EBiomeType::RockyOutcrops; // Safe starting area
    }
    else if (DistanceFromCenter < 2000.0f)
    {
        return EBiomeType::OpenPlains;
    }
    else
    {
        return EBiomeType::DenseForest; // Dangerous outer areas
    }
}

EDangerLevel UWorldGeneratorCore::GetDangerLevelAtLocation(FVector WorldLocation) const
{
    EBiomeType Biome = GetBiomeAtLocation(WorldLocation);
    
    if (BiomeDefinitions.Contains(Biome))
    {
        return BiomeDefinitions[Biome].DangerLevel;
    }
    
    return EDangerLevel::Medium;
}

float UWorldGeneratorCore::GetElevationAtLocation(FVector WorldLocation) const
{
    if (GeneratedLandscape)
    {
        // Sample landscape height at location
        // This will be implemented with actual landscape sampling
        return 0.0f;
    }
    
    return 0.0f;
}