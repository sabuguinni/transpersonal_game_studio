// PCGWorldGenerator.cpp
// Procedural World Generator — Agent #5
// Transpersonal Game Studio — Dinosaur Survival Game

#include "PCGWorldGenerator.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

// ============================================================
// Constructor
// ============================================================
APCGWorldGenerator::APCGWorldGenerator()
{
    PrimaryActorTick.bCanEverTick = false;

    // Default biome parameters
    BiomeSeed = 42;
    WorldSizeX = 100000.0f;
    WorldSizeY = 100000.0f;
    TerrainHeightScale = 5000.0f;
    bGenerateOnBeginPlay = false;

    // Default biome weights (Cretaceous ecosystem)
    BiomeWeights.Add(EWorld_BiomeType::JungleForest, 0.35f);
    BiomeWeights.Add(EWorld_BiomeType::OpenSavanna, 0.25f);
    BiomeWeights.Add(EWorld_BiomeType::RockyHighlands, 0.20f);
    BiomeWeights.Add(EWorld_BiomeType::RiverDelta, 0.10f);
    BiomeWeights.Add(EWorld_BiomeType::CoastalFlats, 0.10f);
}

// ============================================================
// BeginPlay
// ============================================================
void APCGWorldGenerator::BeginPlay()
{
    Super::BeginPlay();

    if (bGenerateOnBeginPlay)
    {
        GenerateWorld();
    }
}

// ============================================================
// GenerateWorld — entry point for full world generation
// ============================================================
void APCGWorldGenerator::GenerateWorld()
{
    UE_LOG(LogTemp, Log, TEXT("[PCGWorldGenerator] GenerateWorld() called. Seed=%d"), BiomeSeed);

    GenerateBiomeMap();
    GenerateRiverNetwork();
    PlaceTerrainLandmarks();

    UE_LOG(LogTemp, Log, TEXT("[PCGWorldGenerator] World generation complete. Biomes=%d"), ActiveBiomes.Num());
}

// ============================================================
// GenerateBiomeMap — creates biome regions using noise
// ============================================================
void APCGWorldGenerator::GenerateBiomeMap()
{
    ActiveBiomes.Empty();

    // Simple deterministic biome placement based on seed
    FRandomStream RNG(BiomeSeed);

    // Generate biome centers
    const int32 NumBiomeCells = 12;
    for (int32 i = 0; i < NumBiomeCells; ++i)
    {
        FWorld_BiomeCell Cell;
        Cell.CenterLocation = FVector(
            RNG.FRandRange(0.0f, WorldSizeX),
            RNG.FRandRange(0.0f, WorldSizeY),
            0.0f
        );
        Cell.Radius = RNG.FRandRange(8000.0f, 20000.0f);

        // Assign biome type based on weighted random
        float Roll = RNG.FRandRange(0.0f, 1.0f);
        float Cumulative = 0.0f;
        Cell.BiomeType = EWorld_BiomeType::OpenSavanna; // default
        for (auto& Pair : BiomeWeights)
        {
            Cumulative += Pair.Value;
            if (Roll <= Cumulative)
            {
                Cell.BiomeType = Pair.Key;
                break;
            }
        }

        Cell.Temperature = GetBiomeTemperature(Cell.BiomeType);
        Cell.Humidity = GetBiomeHumidity(Cell.BiomeType);
        Cell.VegetationDensity = GetBiomeVegetationDensity(Cell.BiomeType);

        ActiveBiomes.Add(Cell);
    }

    UE_LOG(LogTemp, Log, TEXT("[PCGWorldGenerator] Generated %d biome cells"), ActiveBiomes.Num());
}

// ============================================================
// GenerateRiverNetwork — places river splines through terrain
// ============================================================
void APCGWorldGenerator::GenerateRiverNetwork()
{
    // River generation: find high-elevation points and flow downhill
    // In a full implementation this would use heightmap data
    // For now, log the intent and store river source points
    RiverSourcePoints.Empty();

    FRandomStream RNG(BiomeSeed + 1000);
    const int32 NumRivers = 3;

    for (int32 i = 0; i < NumRivers; ++i)
    {
        FVector Source(
            RNG.FRandRange(WorldSizeX * 0.3f, WorldSizeX * 0.7f),
            RNG.FRandRange(WorldSizeY * 0.3f, WorldSizeY * 0.7f),
            TerrainHeightScale * 0.8f
        );
        RiverSourcePoints.Add(Source);
    }

    UE_LOG(LogTemp, Log, TEXT("[PCGWorldGenerator] River network: %d rivers planned"), NumRivers);
}

// ============================================================
// PlaceTerrainLandmarks — volcanic peaks, rock formations, etc.
// ============================================================
void APCGWorldGenerator::PlaceTerrainLandmarks()
{
    LandmarkLocations.Empty();

    FRandomStream RNG(BiomeSeed + 2000);

    // Volcanic peak (central landmark)
    LandmarkLocations.Add(FVector(WorldSizeX * 0.5f, WorldSizeY * 0.5f, 0.0f));

    // Rocky outcrops in highlands biomes
    for (const FWorld_BiomeCell& Cell : ActiveBiomes)
    {
        if (Cell.BiomeType == EWorld_BiomeType::RockyHighlands)
        {
            LandmarkLocations.Add(Cell.CenterLocation);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("[PCGWorldGenerator] Placed %d terrain landmarks"), LandmarkLocations.Num());
}

// ============================================================
// GetBiomeAtLocation — returns biome type for a world position
// ============================================================
EWorld_BiomeType APCGWorldGenerator::GetBiomeAtLocation(FVector WorldLocation) const
{
    EWorld_BiomeType ClosestBiome = EWorld_BiomeType::OpenSavanna;
    float ClosestDist = FLT_MAX;

    for (const FWorld_BiomeCell& Cell : ActiveBiomes)
    {
        float Dist = FVector::Dist2D(WorldLocation, Cell.CenterLocation);
        if (Dist < ClosestDist)
        {
            ClosestDist = Dist;
            ClosestBiome = Cell.BiomeType;
        }
    }

    return ClosestBiome;
}

// ============================================================
// GetBiomeTemperature — realistic Cretaceous temperatures (°C)
// ============================================================
float APCGWorldGenerator::GetBiomeTemperature(EWorld_BiomeType BiomeType) const
{
    switch (BiomeType)
    {
    case EWorld_BiomeType::JungleForest:    return 32.0f;
    case EWorld_BiomeType::OpenSavanna:     return 28.0f;
    case EWorld_BiomeType::RockyHighlands:  return 18.0f;
    case EWorld_BiomeType::RiverDelta:      return 30.0f;
    case EWorld_BiomeType::CoastalFlats:    return 25.0f;
    default:                                return 25.0f;
    }
}

// ============================================================
// GetBiomeHumidity — 0.0 (arid) to 1.0 (saturated)
// ============================================================
float APCGWorldGenerator::GetBiomeHumidity(EWorld_BiomeType BiomeType) const
{
    switch (BiomeType)
    {
    case EWorld_BiomeType::JungleForest:    return 0.90f;
    case EWorld_BiomeType::OpenSavanna:     return 0.40f;
    case EWorld_BiomeType::RockyHighlands:  return 0.25f;
    case EWorld_BiomeType::RiverDelta:      return 0.85f;
    case EWorld_BiomeType::CoastalFlats:    return 0.60f;
    default:                                return 0.50f;
    }
}

// ============================================================
// GetBiomeVegetationDensity — 0.0 (bare) to 1.0 (dense)
// ============================================================
float APCGWorldGenerator::GetBiomeVegetationDensity(EWorld_BiomeType BiomeType) const
{
    switch (BiomeType)
    {
    case EWorld_BiomeType::JungleForest:    return 0.95f;
    case EWorld_BiomeType::OpenSavanna:     return 0.35f;
    case EWorld_BiomeType::RockyHighlands:  return 0.15f;
    case EWorld_BiomeType::RiverDelta:      return 0.70f;
    case EWorld_BiomeType::CoastalFlats:    return 0.45f;
    default:                                return 0.40f;
    }
}
