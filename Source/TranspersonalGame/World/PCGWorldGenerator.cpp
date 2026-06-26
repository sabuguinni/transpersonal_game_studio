// PCGWorldGenerator.cpp — Procedural World Generator #05
// Cycle: PROD_CYCLE_AUTO_20260626_008
// Implements biome system, terrain variation, and river generation

#include "PCGWorldGenerator.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UPCGWorldGenerator::UPCGWorldGenerator()
{
    BiomeSeed = 42;
    TerrainScale = 1.0f;
    RiverCount = 3;
    bEnableBiomeTransitions = true;
    WorldExtentXY = 10000.0f;
    HeightVariationScale = 500.0f;
}

void UPCGWorldGenerator::InitializeWorld(int32 Seed)
{
    BiomeSeed = Seed;
    RandomStream.Initialize(Seed);
    UE_LOG(LogTemp, Log, TEXT("PCGWorldGenerator: Initialized with seed %d"), Seed);

    GenerateBiomeMap();
    GenerateRivers();
}

void UPCGWorldGenerator::GenerateBiomeMap()
{
    BiomeZones.Empty();

    // Define 5 core biomes for the Cretaceous world
    FWorld_BiomeZone Forest;
    Forest.BiomeType = EWorld_BiomeType::TropicalForest;
    Forest.CenterLocation = FVector(1500.f, 1500.f, 30.f);
    Forest.Radius = 2500.f;
    Forest.VegetationDensity = 0.9f;
    Forest.GroundHeightOffset = 30.f;
    BiomeZones.Add(Forest);

    FWorld_BiomeZone Plains;
    Plains.BiomeType = EWorld_BiomeType::OpenPlains;
    Plains.CenterLocation = FVector(-1000.f, 1000.f, 0.f);
    Plains.Radius = 3000.f;
    Plains.VegetationDensity = 0.3f;
    Plains.GroundHeightOffset = 0.f;
    BiomeZones.Add(Plains);

    FWorld_BiomeZone Rocky;
    Rocky.BiomeType = EWorld_BiomeType::RockyHighlands;
    Rocky.CenterLocation = FVector(-500.f, -1500.f, 60.f);
    Rocky.Radius = 2000.f;
    Rocky.VegetationDensity = 0.2f;
    Rocky.GroundHeightOffset = 60.f;
    BiomeZones.Add(Rocky);

    FWorld_BiomeZone Valley;
    Valley.BiomeType = EWorld_BiomeType::RiverValley;
    Valley.CenterLocation = FVector(2500.f, 0.f, -20.f);
    Valley.Radius = 1500.f;
    Valley.VegetationDensity = 0.7f;
    Valley.GroundHeightOffset = -20.f;
    BiomeZones.Add(Valley);

    FWorld_BiomeZone Jungle;
    Jungle.BiomeType = EWorld_BiomeType::TropicalForest;
    Jungle.CenterLocation = FVector(0.f, 2500.f, 20.f);
    Jungle.Radius = 2200.f;
    Jungle.VegetationDensity = 1.0f;
    Jungle.GroundHeightOffset = 20.f;
    BiomeZones.Add(Jungle);

    UE_LOG(LogTemp, Log, TEXT("PCGWorldGenerator: Generated %d biome zones"), BiomeZones.Num());
}

EWorld_BiomeType UPCGWorldGenerator::GetBiomeAtLocation(FVector WorldLocation) const
{
    float ClosestDist = MAX_FLT;
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

float UPCGWorldGenerator::GetTerrainHeightAtLocation(FVector WorldLocation) const
{
    // Layered noise for organic terrain height
    float x = WorldLocation.X / 2000.f;
    float y = WorldLocation.Y / 2000.f;

    // Base large-scale hills
    float h = FMath::Sin(x * 0.8f) * FMath::Cos(y * 0.6f) * 200.f;
    // Mid-frequency detail
    h += FMath::Sin(x * 2.3f + 0.5f) * FMath::Cos(y * 1.9f + 0.3f) * 80.f;
    // Fine detail
    h += FMath::Sin(x * 5.1f + 1.2f) * FMath::Cos(y * 4.7f + 0.8f) * 30.f;

    return h * TerrainScale;
}

void UPCGWorldGenerator::GenerateRivers()
{
    RiverPaths.Empty();

    // Main river: flows from highlands to valley
    TArray<FVector> MainRiver;
    for (int32 i = 0; i <= 20; i++)
    {
        float t = (float)i / 20.f;
        float rx = FMath::Lerp(-2000.f, 3500.f, t) + FMath::Sin(t * 6.28f) * 300.f;
        float ry = FMath::Lerp(500.f, -500.f, t) + FMath::Cos(t * 4.71f) * 200.f;
        MainRiver.Add(FVector(rx, ry, -15.f));
    }
    RiverPaths.Add(MainRiver);

    // Tributary river
    TArray<FVector> Tributary;
    for (int32 i = 0; i <= 12; i++)
    {
        float t = (float)i / 12.f;
        float rx = FMath::Lerp(0.f, 2000.f, t) + FMath::Sin(t * 9.42f) * 150.f;
        float ry = FMath::Lerp(2000.f, 200.f, t);
        Tributary.Add(FVector(rx, ry, -10.f));
    }
    RiverPaths.Add(Tributary);

    UE_LOG(LogTemp, Log, TEXT("PCGWorldGenerator: Generated %d river paths"), RiverPaths.Num());
}

TArray<FWorld_BiomeZone> UPCGWorldGenerator::GetAllBiomeZones() const
{
    return BiomeZones;
}

int32 UPCGWorldGenerator::GetRiverCount() const
{
    return RiverPaths.Num();
}

FWorld_TerrainSample UPCGWorldGenerator::SampleTerrain(FVector Location) const
{
    FWorld_TerrainSample Sample;
    Sample.WorldLocation = Location;
    Sample.Height = GetTerrainHeightAtLocation(Location);
    Sample.BiomeType = GetBiomeAtLocation(Location);

    // Slope approximation via finite difference
    float dx = GetTerrainHeightAtLocation(Location + FVector(100.f, 0.f, 0.f)) - Sample.Height;
    float dy = GetTerrainHeightAtLocation(Location + FVector(0.f, 100.f, 0.f)) - Sample.Height;
    Sample.SlopeAngle = FMath::RadiansToDegrees(FMath::Atan2(FMath::Sqrt(dx*dx + dy*dy), 100.f));
    Sample.bIsWater = (Sample.Height < -10.f);

    return Sample;
}
