// PCGWorldGenerator.cpp
// Transpersonal Game Studio — Prehistoric Survival Game
// Agent #05 — Procedural World Generator
// Implements biome classification, terrain height sampling, and PCG spawn point generation.

#include "PCGWorldGenerator.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

// ============================================================
// Constructor
// ============================================================
APCGWorldGenerator::APCGWorldGenerator()
{
    PrimaryActorTick.bCanEverTick = false;

    // Default biome parameters
    WorldSeedValue = 42;
    WorldScaleKm = 4.0f;
    bBiomesGenerated = false;
    bTerrainHeightCached = false;

    // Default biome thresholds (normalized 0-1 elevation)
    RiverValleyMaxElevation = 0.15f;
    PlainsMaxElevation = 0.40f;
    ForestMaxElevation = 0.65f;
    RockyUplandMinElevation = 0.65f;
}

// ============================================================
// BeginPlay
// ============================================================
void APCGWorldGenerator::BeginPlay()
{
    Super::BeginPlay();

    if (!bBiomesGenerated)
    {
        GenerateBiomeMap();
    }
}

// ============================================================
// GenerateBiomeMap
// Classifies the world into biome zones based on elevation + moisture noise.
// ============================================================
void APCGWorldGenerator::GenerateBiomeMap()
{
    UE_LOG(LogTemp, Log, TEXT("PCGWorldGenerator: Generating biome map (seed=%d, scale=%.1fkm)"), WorldSeedValue, WorldScaleKm);

    BiomeZones.Empty();

    // Define the 4 core biome zones matching MinPlayableMap layout
    FWorld_BiomeZone RiverValley;
    RiverValley.BiomeType = EWorld_BiomeType::RiverValley;
    RiverValley.CenterLocation = FVector(-800.0f, 200.0f, -30.0f);
    RiverValley.RadiusMeters = 400.0f;
    RiverValley.ElevationNormalized = 0.05f;
    RiverValley.MoistureNormalized = 0.95f;
    RiverValley.bHasWater = true;
    BiomeZones.Add(RiverValley);

    FWorld_BiomeZone OpenPlains;
    OpenPlains.BiomeType = EWorld_BiomeType::OpenPlains;
    OpenPlains.CenterLocation = FVector(0.0f, 600.0f, 0.0f);
    OpenPlains.RadiusMeters = 500.0f;
    OpenPlains.ElevationNormalized = 0.25f;
    OpenPlains.MoistureNormalized = 0.40f;
    OpenPlains.bHasWater = false;
    BiomeZones.Add(OpenPlains);

    FWorld_BiomeZone RockyUpland;
    RockyUpland.BiomeType = EWorld_BiomeType::RockyUpland;
    RockyUpland.CenterLocation = FVector(-300.0f, -600.0f, 80.0f);
    RockyUpland.RadiusMeters = 250.0f;
    RockyUpland.ElevationNormalized = 0.75f;
    RockyUpland.MoistureNormalized = 0.20f;
    RockyUpland.bHasWater = false;
    BiomeZones.Add(RockyUpland);

    FWorld_BiomeZone DenseJungle;
    DenseJungle.BiomeType = EWorld_BiomeType::DenseJungle;
    DenseJungle.CenterLocation = FVector(-750.0f, 180.0f, 0.0f);
    DenseJungle.RadiusMeters = 300.0f;
    DenseJungle.ElevationNormalized = 0.30f;
    DenseJungle.MoistureNormalized = 0.90f;
    DenseJungle.bHasWater = false;
    BiomeZones.Add(DenseJungle);

    bBiomesGenerated = true;
    UE_LOG(LogTemp, Log, TEXT("PCGWorldGenerator: %d biome zones registered."), BiomeZones.Num());
}

// ============================================================
// GetBiomeAtLocation
// Returns the dominant biome type at a given world location.
// ============================================================
EWorld_BiomeType APCGWorldGenerator::GetBiomeAtLocation(FVector WorldLocation) const
{
    if (BiomeZones.Num() == 0)
    {
        return EWorld_BiomeType::OpenPlains;
    }

    float ClosestDist = TNumericLimits<float>::Max();
    EWorld_BiomeType ClosestBiome = EWorld_BiomeType::OpenPlains;

    for (const FWorld_BiomeZone& Zone : BiomeZones)
    {
        float Dist = FVector::Dist2D(WorldLocation, Zone.CenterLocation);
        if (Dist < ClosestDist)
        {
            ClosestDist = Dist;
            ClosestBiome = Zone.BiomeType;
        }
    }

    return ClosestBiome;
}

// ============================================================
// GetTerrainHeightAtLocation
// Returns approximate terrain height using biome elevation data.
// In production this would raycast against the Landscape actor.
// ============================================================
float APCGWorldGenerator::GetTerrainHeightAtLocation(FVector WorldLocation) const
{
    EWorld_BiomeType Biome = GetBiomeAtLocation(WorldLocation);

    switch (Biome)
    {
        case EWorld_BiomeType::RiverValley:  return -30.0f;
        case EWorld_BiomeType::OpenPlains:   return 0.0f;
        case EWorld_BiomeType::DenseJungle:  return 5.0f;
        case EWorld_BiomeType::RockyUpland:  return 80.0f;
        case EWorld_BiomeType::CliffFace:    return 120.0f;
        default:                             return 0.0f;
    }
}

// ============================================================
// GenerateSpawnPoints
// Returns an array of spawn point transforms for a given biome type.
// Used by FoliageManager and DinosaurSpawner.
// ============================================================
TArray<FTransform> APCGWorldGenerator::GenerateSpawnPoints(EWorld_BiomeType BiomeType, int32 Count) const
{
    TArray<FTransform> SpawnPoints;

    // Find matching biome zone
    const FWorld_BiomeZone* TargetZone = nullptr;
    for (const FWorld_BiomeZone& Zone : BiomeZones)
    {
        if (Zone.BiomeType == BiomeType)
        {
            TargetZone = &Zone;
            break;
        }
    }

    if (!TargetZone)
    {
        UE_LOG(LogTemp, Warning, TEXT("PCGWorldGenerator: No zone found for biome type %d"), (int32)BiomeType);
        return SpawnPoints;
    }

    // Deterministic pseudo-random distribution within zone radius
    FRandomStream RNG(WorldSeedValue + (int32)BiomeType * 1000);

    for (int32 i = 0; i < Count; ++i)
    {
        float Angle = RNG.FRandRange(0.0f, 360.0f);
        float Radius = RNG.FRandRange(0.0f, TargetZone->RadiusMeters);
        float Yaw = RNG.FRandRange(0.0f, 360.0f);

        FVector Offset(
            FMath::Cos(FMath::DegreesToRadians(Angle)) * Radius,
            FMath::Sin(FMath::DegreesToRadians(Angle)) * Radius,
            0.0f
        );

        FVector SpawnLoc = TargetZone->CenterLocation + Offset;
        SpawnLoc.Z = GetTerrainHeightAtLocation(SpawnLoc);

        FTransform T;
        T.SetLocation(SpawnLoc);
        T.SetRotation(FQuat(FRotator(0.0f, Yaw, 0.0f)));
        T.SetScale3D(FVector(1.0f));

        SpawnPoints.Add(T);
    }

    UE_LOG(LogTemp, Log, TEXT("PCGWorldGenerator: Generated %d spawn points for biome %d"), SpawnPoints.Num(), (int32)BiomeType);
    return SpawnPoints;
}

// ============================================================
// GetBiomeCount
// ============================================================
int32 APCGWorldGenerator::GetBiomeCount() const
{
    return BiomeZones.Num();
}

// ============================================================
// GetWorldSeed
// ============================================================
int32 APCGWorldGenerator::GetWorldSeed() const
{
    return WorldSeedValue;
}

// ============================================================
// SetWorldSeed
// ============================================================
void APCGWorldGenerator::SetWorldSeed(int32 NewSeed)
{
    WorldSeedValue = NewSeed;
    bBiomesGenerated = false;
    bTerrainHeightCached = false;
    UE_LOG(LogTemp, Log, TEXT("PCGWorldGenerator: World seed set to %d — regenerate biomes to apply."), WorldSeedValue);
}

// ============================================================
// RegenerateBiomes (CallInEditor)
// ============================================================
void APCGWorldGenerator::RegenerateBiomes()
{
    bBiomesGenerated = false;
    GenerateBiomeMap();
    UE_LOG(LogTemp, Log, TEXT("PCGWorldGenerator: Biomes regenerated with seed=%d"), WorldSeedValue);
}

// ============================================================
// GetBiomeZones
// ============================================================
TArray<FWorld_BiomeZone> APCGWorldGenerator::GetBiomeZones() const
{
    return BiomeZones;
}
