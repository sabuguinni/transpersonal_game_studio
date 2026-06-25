// PCGWorldGenerator.cpp
// Agent #05 — Procedural World Generator
// Cycle: PROD_CYCLE_AUTO_20260625_004
// Implements biome zone management, terrain variation, and dino placement logic.

#include "PCGWorldGenerator.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogPCGWorld, Log, All);

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────
APCGWorldGenerator::APCGWorldGenerator()
{
    PrimaryActorTick.bCanEverTick = false;

    // Default biome configuration
    WorldSeed = 42;
    bAutoGenerateOnBeginPlay = false;
    TerrainHeightScale = 1000.0f;
    BiomeBlendRadius = 500.0f;

    // Initialize default biome zones
    FWorld_BiomeZone JungleZone;
    JungleZone.ZoneID = TEXT("jungle_core");
    JungleZone.ZoneName = TEXT("Cretaceous Jungle Core");
    JungleZone.Center = FVector(2000.0f, 2500.0f, 400.0f);
    JungleZone.Radius = 3000.0f;
    JungleZone.BiomeType = EWorld_BiomeType::TropicalJungle;
    JungleZone.VegetationDensity = 0.85f;
    JungleZone.TerrainHeightVariation = 600.0f;
    BiomeZones.Add(JungleZone);

    FWorld_BiomeZone SavannaZone;
    SavannaZone.ZoneID = TEXT("open_savanna");
    SavannaZone.ZoneName = TEXT("Cretaceous Savanna");
    SavannaZone.Center = FVector(4000.0f, 4000.0f, 200.0f);
    SavannaZone.Radius = 4000.0f;
    SavannaZone.BiomeType = EWorld_BiomeType::OpenSavanna;
    SavannaZone.VegetationDensity = 0.35f;
    SavannaZone.TerrainHeightVariation = 200.0f;
    BiomeZones.Add(SavannaZone);

    FWorld_BiomeZone VolcanicZone;
    VolcanicZone.ZoneID = TEXT("volcanic_highlands");
    VolcanicZone.ZoneName = TEXT("Volcanic Highlands");
    VolcanicZone.Center = FVector(6000.0f, 2000.0f, 1200.0f);
    VolcanicZone.Radius = 2500.0f;
    VolcanicZone.BiomeType = EWorld_BiomeType::VolcanicHighlands;
    VolcanicZone.VegetationDensity = 0.15f;
    VolcanicZone.TerrainHeightVariation = 1500.0f;
    BiomeZones.Add(VolcanicZone);

    FWorld_BiomeZone RiverZone;
    RiverZone.ZoneID = TEXT("river_delta");
    RiverZone.ZoneName = TEXT("River Delta Wetlands");
    RiverZone.Center = FVector(1000.0f, 5000.0f, 100.0f);
    RiverZone.Radius = 2000.0f;
    RiverZone.BiomeType = EWorld_BiomeType::RiverWetlands;
    RiverZone.VegetationDensity = 0.70f;
    RiverZone.TerrainHeightVariation = 100.0f;
    BiomeZones.Add(RiverZone);
}

// ─────────────────────────────────────────────────────────────────────────────
// BeginPlay
// ─────────────────────────────────────────────────────────────────────────────
void APCGWorldGenerator::BeginPlay()
{
    Super::BeginPlay();

    if (bAutoGenerateOnBeginPlay)
    {
        GenerateWorld();
    }

    UE_LOG(LogPCGWorld, Log, TEXT("PCGWorldGenerator initialized — %d biome zones loaded, seed=%d"),
        BiomeZones.Num(), WorldSeed);
}

// ─────────────────────────────────────────────────────────────────────────────
// GenerateWorld — Main entry point
// ─────────────────────────────────────────────────────────────────────────────
void APCGWorldGenerator::GenerateWorld()
{
    UE_LOG(LogPCGWorld, Log, TEXT("GenerateWorld() called — seed=%d, biomes=%d"),
        WorldSeed, BiomeZones.Num());

    for (const FWorld_BiomeZone& Zone : BiomeZones)
    {
        ProcessBiomeZone(Zone);
    }

    UE_LOG(LogPCGWorld, Log, TEXT("World generation complete."));
}

// ─────────────────────────────────────────────────────────────────────────────
// GetBiomeAtLocation — Returns the dominant biome at a world position
// ─────────────────────────────────────────────────────────────────────────────
EWorld_BiomeType APCGWorldGenerator::GetBiomeAtLocation(FVector WorldLocation) const
{
    float ClosestDist = FLT_MAX;
    EWorld_BiomeType DominantBiome = EWorld_BiomeType::OpenSavanna;

    for (const FWorld_BiomeZone& Zone : BiomeZones)
    {
        float Dist = FVector::Dist2D(WorldLocation, Zone.Center);
        if (Dist < Zone.Radius && Dist < ClosestDist)
        {
            ClosestDist = Dist;
            DominantBiome = Zone.BiomeType;
        }
    }

    return DominantBiome;
}

// ─────────────────────────────────────────────────────────────────────────────
// GetVegetationDensityAtLocation
// ─────────────────────────────────────────────────────────────────────────────
float APCGWorldGenerator::GetVegetationDensityAtLocation(FVector WorldLocation) const
{
    float TotalWeight = 0.0f;
    float WeightedDensity = 0.0f;

    for (const FWorld_BiomeZone& Zone : BiomeZones)
    {
        float Dist = FVector::Dist2D(WorldLocation, Zone.Center);
        if (Dist < Zone.Radius + BiomeBlendRadius)
        {
            float Weight = FMath::Clamp(1.0f - (Dist / (Zone.Radius + BiomeBlendRadius)), 0.0f, 1.0f);
            WeightedDensity += Zone.VegetationDensity * Weight;
            TotalWeight += Weight;
        }
    }

    if (TotalWeight > 0.0f)
    {
        return WeightedDensity / TotalWeight;
    }

    return 0.1f; // Default sparse vegetation
}

// ─────────────────────────────────────────────────────────────────────────────
// GetTerrainHeightAtLocation — Procedural height using Perlin-like noise
// ─────────────────────────────────────────────────────────────────────────────
float APCGWorldGenerator::GetTerrainHeightAtLocation(FVector WorldLocation) const
{
    // Simple pseudo-noise based on world seed
    float NoiseX = WorldLocation.X * 0.0003f + WorldSeed * 0.1f;
    float NoiseY = WorldLocation.Y * 0.0003f + WorldSeed * 0.07f;

    float Height = FMath::Sin(NoiseX) * FMath::Cos(NoiseY) * 0.5f
                 + FMath::Sin(NoiseX * 2.3f) * FMath::Cos(NoiseY * 1.7f) * 0.25f
                 + FMath::Sin(NoiseX * 5.1f) * FMath::Cos(NoiseY * 4.3f) * 0.125f;

    // Normalize to 0-1 range, then scale
    Height = (Height + 1.0f) * 0.5f;

    // Apply biome height variation
    float BiomeHeight = 0.0f;
    float TotalWeight = 0.0f;
    for (const FWorld_BiomeZone& Zone : BiomeZones)
    {
        float Dist = FVector::Dist2D(WorldLocation, Zone.Center);
        if (Dist < Zone.Radius)
        {
            float Weight = 1.0f - (Dist / Zone.Radius);
            BiomeHeight += Zone.TerrainHeightVariation * Weight;
            TotalWeight += Weight;
        }
    }

    if (TotalWeight > 0.0f)
    {
        BiomeHeight /= TotalWeight;
    }
    else
    {
        BiomeHeight = 200.0f;
    }

    return Height * BiomeHeight;
}

// ─────────────────────────────────────────────────────────────────────────────
// ProcessBiomeZone — Internal per-zone processing
// ─────────────────────────────────────────────────────────────────────────────
void APCGWorldGenerator::ProcessBiomeZone(const FWorld_BiomeZone& Zone)
{
    UE_LOG(LogPCGWorld, Log, TEXT("Processing biome zone: %s (type=%d, density=%.2f)"),
        *Zone.ZoneName, (int32)Zone.BiomeType, Zone.VegetationDensity);
}

// ─────────────────────────────────────────────────────────────────────────────
// DebugDrawBiomeZones — Visualize zones in editor
// ─────────────────────────────────────────────────────────────────────────────
void APCGWorldGenerator::DebugDrawBiomeZones()
{
    UWorld* World = GetWorld();
    if (!World) return;

    for (const FWorld_BiomeZone& Zone : BiomeZones)
    {
        FColor ZoneColor = FColor::Green;
        switch (Zone.BiomeType)
        {
        case EWorld_BiomeType::TropicalJungle:    ZoneColor = FColor::Green; break;
        case EWorld_BiomeType::OpenSavanna:       ZoneColor = FColor::Yellow; break;
        case EWorld_BiomeType::VolcanicHighlands: ZoneColor = FColor::Red; break;
        case EWorld_BiomeType::RiverWetlands:     ZoneColor = FColor::Blue; break;
        default: break;
        }

        DrawDebugCircle(World, Zone.Center, Zone.Radius, 64,
            ZoneColor, false, 10.0f, 0, 20.0f,
            FVector(0, 1, 0), FVector(1, 0, 0));

        DrawDebugString(World, Zone.Center + FVector(0, 0, 200),
            Zone.ZoneName, nullptr, ZoneColor, 10.0f);
    }
}
