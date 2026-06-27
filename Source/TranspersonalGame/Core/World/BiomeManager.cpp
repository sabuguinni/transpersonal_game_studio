// BiomeManager.cpp — Biome System Implementation
// Agent #05 — Procedural World Generator
// Manages biome zones matching the MinPlayableMap layout

#include "BiomeManager.h"
#include "Math/UnrealMathUtility.h"

ABiomeManager::ABiomeManager()
{
    PrimaryActorTick.bCanEverTick = false;
    bBiomesInitialized = false;
}

void ABiomeManager::BeginPlay()
{
    Super::BeginPlay();

    if (!bBiomesInitialized)
    {
        InitializeDefaultBiomes();
    }
}

void ABiomeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ABiomeManager::InitializeDefaultBiomes()
{
    RegisteredBiomes.Empty();

    // ── River Valley (southwest) ──────────────────────────────────────────
    {
        FWorld_BiomeData RiverValley;
        RiverValley.BiomeType = EWorld_BiomeType::RiverValley;
        RiverValley.BiomeName = TEXT("River Valley");
        RiverValley.MinTemperature = 18.0f;
        RiverValley.MaxTemperature = 28.0f;
        RiverValley.Humidity = 0.85f;
        RiverValley.DangerLevel = 0.25f;
        RiverValley.ResourceDensity = 0.90f;  // High — water + food nearby
        RiverValley.FogDensityMultiplier = 1.4f;
        RiverValley.AmbientSoundTag = FName("Biome_RiverValley");
        RiverValley.ZoneCenter = FVector(-500.0f, 200.0f, 15.0f);
        RiverValley.ZoneRadius = 1800.0f;
        RegisteredBiomes.Add(RiverValley);
    }

    // ── Dense Forest (northwest) ──────────────────────────────────────────
    {
        FWorld_BiomeData Forest;
        Forest.BiomeType = EWorld_BiomeType::DenseForest;
        Forest.BiomeName = TEXT("Dense Forest");
        Forest.MinTemperature = 20.0f;
        Forest.MaxTemperature = 30.0f;
        Forest.Humidity = 0.75f;
        Forest.DangerLevel = 0.55f;  // Raptors hunt in forest
        Forest.ResourceDensity = 0.70f;
        Forest.FogDensityMultiplier = 1.6f;
        Forest.AmbientSoundTag = FName("Biome_DenseForest");
        Forest.ZoneCenter = FVector(-2100.0f, 2000.0f, 85.0f);
        Forest.ZoneRadius = 1500.0f;
        RegisteredBiomes.Add(Forest);
    }

    // ── Open Plains (center) ──────────────────────────────────────────────
    {
        FWorld_BiomeData Plains;
        Plains.BiomeType = EWorld_BiomeType::OpenPlains;
        Plains.BiomeName = TEXT("Open Plains");
        Plains.MinTemperature = 22.0f;
        Plains.MaxTemperature = 38.0f;
        Plains.Humidity = 0.35f;
        Plains.DangerLevel = 0.40f;  // Visible from far — T-Rex territory
        Plains.ResourceDensity = 0.50f;
        Plains.FogDensityMultiplier = 0.8f;
        Plains.AmbientSoundTag = FName("Biome_OpenPlains");
        Plains.ZoneCenter = FVector(600.0f, 600.0f, 5.0f);
        Plains.ZoneRadius = 2000.0f;
        RegisteredBiomes.Add(Plains);
    }

    // ── Volcanic Highlands (northeast) ────────────────────────────────────
    {
        FWorld_BiomeData Volcanic;
        Volcanic.BiomeType = EWorld_BiomeType::VolcanicHighlands;
        Volcanic.BiomeName = TEXT("Volcanic Highlands");
        Volcanic.MinTemperature = 35.0f;
        Volcanic.MaxTemperature = 65.0f;
        Volcanic.Humidity = 0.15f;
        Volcanic.DangerLevel = 0.80f;  // Extreme danger — heat + predators
        Volcanic.ResourceDensity = 0.20f;  // Rare resources but valuable
        Volcanic.FogDensityMultiplier = 2.0f;  // Ash/steam fog
        Volcanic.AmbientSoundTag = FName("Biome_VolcanicHighlands");
        Volcanic.ZoneCenter = FVector(3400.0f, 3000.0f, 200.0f);
        Volcanic.ZoneRadius = 1200.0f;
        RegisteredBiomes.Add(Volcanic);
    }

    // ── Rocky Badlands (east) ─────────────────────────────────────────────
    {
        FWorld_BiomeData Badlands;
        Badlands.BiomeType = EWorld_BiomeType::RockyBadlands;
        Badlands.BiomeName = TEXT("Rocky Badlands");
        Badlands.MinTemperature = 25.0f;
        Badlands.MaxTemperature = 45.0f;
        Badlands.Humidity = 0.20f;
        Badlands.DangerLevel = 0.60f;
        Badlands.ResourceDensity = 0.40f;  // Stone/flint for crafting
        Badlands.FogDensityMultiplier = 0.9f;
        Badlands.AmbientSoundTag = FName("Biome_RockyBadlands");
        Badlands.ZoneCenter = FVector(2500.0f, 0.0f, 120.0f);
        Badlands.ZoneRadius = 1600.0f;
        RegisteredBiomes.Add(Badlands);
    }

    bBiomesInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Initialized %d biomes"), RegisteredBiomes.Num());
}

int32 ABiomeManager::FindClosestBiomeIndex(const FVector& WorldLocation) const
{
    if (RegisteredBiomes.Num() == 0)
    {
        return INDEX_NONE;
    }

    int32 ClosestIndex = 0;
    float ClosestDist = FVector::DistXY(WorldLocation, RegisteredBiomes[0].ZoneCenter);

    for (int32 i = 1; i < RegisteredBiomes.Num(); ++i)
    {
        float Dist = FVector::DistXY(WorldLocation, RegisteredBiomes[i].ZoneCenter);
        if (Dist < ClosestDist)
        {
            ClosestDist = Dist;
            ClosestIndex = i;
        }
    }

    return ClosestIndex;
}

EWorld_BiomeType ABiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    int32 Idx = FindClosestBiomeIndex(WorldLocation);
    if (Idx == INDEX_NONE)
    {
        return EWorld_BiomeType::None;
    }
    return RegisteredBiomes[Idx].BiomeType;
}

FWorld_BiomeData ABiomeManager::GetBiomeDataAtLocation(const FVector& WorldLocation) const
{
    int32 Idx = FindClosestBiomeIndex(WorldLocation);
    if (Idx == INDEX_NONE)
    {
        return FWorld_BiomeData();
    }
    return RegisteredBiomes[Idx];
}

float ABiomeManager::GetTemperatureAtLocation(const FVector& WorldLocation) const
{
    int32 Idx = FindClosestBiomeIndex(WorldLocation);
    if (Idx == INDEX_NONE)
    {
        return 25.0f;  // Default temperature
    }

    const FWorld_BiomeData& Biome = RegisteredBiomes[Idx];
    float Dist = FVector::DistXY(WorldLocation, Biome.ZoneCenter);

    // Blend temperature at biome edges
    float BlendAlpha = FMath::Clamp(1.0f - (Dist / Biome.ZoneRadius), 0.0f, 1.0f);
    float MidTemp = (Biome.MinTemperature + Biome.MaxTemperature) * 0.5f;

    return FMath::Lerp(25.0f, MidTemp, BlendAlpha);
}

float ABiomeManager::GetDangerLevelAtLocation(const FVector& WorldLocation) const
{
    int32 Idx = FindClosestBiomeIndex(WorldLocation);
    if (Idx == INDEX_NONE)
    {
        return 0.3f;
    }

    const FWorld_BiomeData& Biome = RegisteredBiomes[Idx];
    float Dist = FVector::DistXY(WorldLocation, Biome.ZoneCenter);
    float BlendAlpha = FMath::Clamp(1.0f - (Dist / Biome.ZoneRadius), 0.0f, 1.0f);

    return FMath::Lerp(0.3f, Biome.DangerLevel, BlendAlpha);
}

float ABiomeManager::GetResourceDensityAtLocation(const FVector& WorldLocation) const
{
    int32 Idx = FindClosestBiomeIndex(WorldLocation);
    if (Idx == INDEX_NONE)
    {
        return 0.5f;
    }

    const FWorld_BiomeData& Biome = RegisteredBiomes[Idx];
    float Dist = FVector::DistXY(WorldLocation, Biome.ZoneCenter);
    float BlendAlpha = FMath::Clamp(1.0f - (Dist / Biome.ZoneRadius), 0.0f, 1.0f);

    return FMath::Lerp(0.5f, Biome.ResourceDensity, BlendAlpha);
}
