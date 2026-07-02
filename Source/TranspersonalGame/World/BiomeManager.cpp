// BiomeManager.cpp — Transpersonal Game Studio
// Engine Architect #02 — PROD_CYCLE_AUTO_20260702_004
// Full implementation of the Biome system for Cretaceous world generation

#include "BiomeManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

// ============================================================
// Constructor
// ============================================================
UBiomeManager::UBiomeManager()
{
    // Default biome parameters
    CurrentBiomeType = EEng_BiomeType::Jungle;
    WorldSeedValue = 42;
    BiomeBlendRadius = 500.0f;
    bBiomesInitialized = false;

    // Register default biome configs
    FEng_BiomeConfig JungleConfig;
    JungleConfig.BiomeType = EEng_BiomeType::Jungle;
    JungleConfig.DisplayName = FName("Cretaceous Jungle");
    JungleConfig.BaseTemperature = 32.0f;
    JungleConfig.Humidity = 0.85f;
    JungleConfig.FoliageDensity = 0.9f;
    JungleConfig.PredatorSpawnWeight = 0.6f;
    JungleConfig.HerbivoreSpawnWeight = 0.8f;
    JungleConfig.FogDensity = 0.04f;
    JungleConfig.AmbientLightIntensity = 0.6f;
    BiomeConfigs.Add(EEng_BiomeType::Jungle, JungleConfig);

    FEng_BiomeConfig PlainConfig;
    PlainConfig.BiomeType = EEng_BiomeType::OpenPlain;
    PlainConfig.DisplayName = FName("Open Savanna");
    PlainConfig.BaseTemperature = 38.0f;
    PlainConfig.Humidity = 0.35f;
    PlainConfig.FoliageDensity = 0.3f;
    PlainConfig.PredatorSpawnWeight = 0.8f;
    PlainConfig.HerbivoreSpawnWeight = 0.9f;
    PlainConfig.FogDensity = 0.01f;
    PlainConfig.AmbientLightIntensity = 1.0f;
    BiomeConfigs.Add(EEng_BiomeType::OpenPlain, PlainConfig);

    FEng_BiomeConfig SwampConfig;
    SwampConfig.BiomeType = EEng_BiomeType::Swamp;
    SwampConfig.DisplayName = FName("Primordial Swamp");
    SwampConfig.BaseTemperature = 28.0f;
    SwampConfig.Humidity = 0.95f;
    SwampConfig.FoliageDensity = 0.7f;
    SwampConfig.PredatorSpawnWeight = 0.5f;
    SwampConfig.HerbivoreSpawnWeight = 0.4f;
    SwampConfig.FogDensity = 0.08f;
    SwampConfig.AmbientLightIntensity = 0.4f;
    BiomeConfigs.Add(EEng_BiomeType::Swamp, SwampConfig);

    FEng_BiomeConfig VolcanoConfig;
    VolcanoConfig.BiomeType = EEng_BiomeType::Volcanic;
    VolcanoConfig.DisplayName = FName("Volcanic Badlands");
    VolcanoConfig.BaseTemperature = 55.0f;
    VolcanoConfig.Humidity = 0.1f;
    VolcanoConfig.FoliageDensity = 0.05f;
    VolcanoConfig.PredatorSpawnWeight = 0.3f;
    VolcanoConfig.HerbivoreSpawnWeight = 0.1f;
    VolcanoConfig.FogDensity = 0.06f;
    VolcanoConfig.AmbientLightIntensity = 0.8f;
    BiomeConfigs.Add(EEng_BiomeType::Volcanic, VolcanoConfig);

    FEng_BiomeConfig RiverConfig;
    RiverConfig.BiomeType = EEng_BiomeType::RiverDelta;
    RiverConfig.DisplayName = FName("River Delta");
    RiverConfig.BaseTemperature = 26.0f;
    RiverConfig.Humidity = 0.75f;
    RiverConfig.FoliageDensity = 0.6f;
    RiverConfig.PredatorSpawnWeight = 0.4f;
    RiverConfig.HerbivoreSpawnWeight = 0.7f;
    RiverConfig.FogDensity = 0.03f;
    RiverConfig.AmbientLightIntensity = 0.75f;
    BiomeConfigs.Add(EEng_BiomeType::RiverDelta, RiverConfig);

    FEng_BiomeConfig CoastConfig;
    CoastConfig.BiomeType = EEng_BiomeType::Coastal;
    CoastConfig.DisplayName = FName("Cretaceous Coast");
    CoastConfig.BaseTemperature = 30.0f;
    CoastConfig.Humidity = 0.6f;
    CoastConfig.FoliageDensity = 0.4f;
    CoastConfig.PredatorSpawnWeight = 0.35f;
    CoastConfig.HerbivoreSpawnWeight = 0.5f;
    CoastConfig.FogDensity = 0.02f;
    CoastConfig.AmbientLightIntensity = 0.9f;
    BiomeConfigs.Add(EEng_BiomeType::Coastal, CoastConfig);
}

// ============================================================
// UGameInstanceSubsystem interface
// ============================================================
void UBiomeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    InitializeBiomes();
    UE_LOG(LogTemp, Log, TEXT("[BiomeManager] Initialized with %d biome configs"), BiomeConfigs.Num());
}

void UBiomeManager::Deinitialize()
{
    BiomeConfigs.Empty();
    ActiveBiomeZones.Empty();
    bBiomesInitialized = false;
    Super::Deinitialize();
}

// ============================================================
// Core biome logic
// ============================================================
void UBiomeManager::InitializeBiomes()
{
    if (bBiomesInitialized)
    {
        return;
    }

    // Seed the random stream for deterministic world generation
    BiomeRandom.Initialize(WorldSeedValue);

    // Generate initial biome zone layout
    GenerateBiomeZones();

    bBiomesInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("[BiomeManager] Biome zones generated: %d zones"), ActiveBiomeZones.Num());
}

void UBiomeManager::GenerateBiomeZones()
{
    ActiveBiomeZones.Empty();

    // Fixed biome zone layout for the starting area (Cretaceous world)
    // These are the 6 primary biome zones around the player spawn
    struct FBiomeZoneData
    {
        EEng_BiomeType Type;
        FVector2D Center;
        float Radius;
    };

    TArray<FBiomeZoneData> ZoneLayout = {
        { EEng_BiomeType::OpenPlain,  FVector2D(0.0f, 0.0f),       3000.0f },
        { EEng_BiomeType::Jungle,     FVector2D(4000.0f, 2000.0f),  2500.0f },
        { EEng_BiomeType::Swamp,      FVector2D(-3000.0f, 3000.0f), 2000.0f },
        { EEng_BiomeType::RiverDelta, FVector2D(1000.0f, -4000.0f), 1800.0f },
        { EEng_BiomeType::Coastal,    FVector2D(-4000.0f, -2000.0f),2200.0f },
        { EEng_BiomeType::Volcanic,   FVector2D(6000.0f, -3000.0f), 1500.0f },
    };

    for (const auto& ZoneData : ZoneLayout)
    {
        FEng_BiomeZone Zone;
        Zone.ZoneID = ActiveBiomeZones.Num();
        Zone.BiomeType = ZoneData.Type;
        Zone.Center = ZoneData.Center;
        Zone.Radius = ZoneData.Radius;
        Zone.bIsActive = true;

        if (BiomeConfigs.Contains(ZoneData.Type))
        {
            Zone.Config = BiomeConfigs[ZoneData.Type];
        }

        ActiveBiomeZones.Add(Zone);
    }
}

EEng_BiomeType UBiomeManager::GetBiomeAtLocation(FVector WorldLocation) const
{
    if (!bBiomesInitialized || ActiveBiomeZones.Num() == 0)
    {
        return EEng_BiomeType::OpenPlain;
    }

    FVector2D Loc2D(WorldLocation.X, WorldLocation.Y);
    float ClosestDist = FLT_MAX;
    EEng_BiomeType ClosestBiome = EEng_BiomeType::OpenPlain;

    for (const FEng_BiomeZone& Zone : ActiveBiomeZones)
    {
        if (!Zone.bIsActive) continue;

        float Dist = FVector2D::Distance(Loc2D, Zone.Center);
        if (Dist < ClosestDist)
        {
            ClosestDist = Dist;
            ClosestBiome = Zone.BiomeType;
        }
    }

    return ClosestBiome;
}

FEng_BiomeConfig UBiomeManager::GetBiomeConfig(EEng_BiomeType BiomeType) const
{
    if (BiomeConfigs.Contains(BiomeType))
    {
        return BiomeConfigs[BiomeType];
    }

    // Return default jungle config if not found
    FEng_BiomeConfig Default;
    Default.BiomeType = EEng_BiomeType::Jungle;
    Default.DisplayName = FName("Unknown");
    Default.BaseTemperature = 30.0f;
    Default.Humidity = 0.5f;
    Default.FoliageDensity = 0.5f;
    return Default;
}

float UBiomeManager::GetBlendWeightAtLocation(FVector WorldLocation, EEng_BiomeType BiomeType) const
{
    FVector2D Loc2D(WorldLocation.X, WorldLocation.Y);

    for (const FEng_BiomeZone& Zone : ActiveBiomeZones)
    {
        if (Zone.BiomeType != BiomeType || !Zone.bIsActive) continue;

        float Dist = FVector2D::Distance(Loc2D, Zone.Center);
        if (Dist >= Zone.Radius + BiomeBlendRadius) continue;

        // Smooth blend at zone boundary
        float BlendStart = Zone.Radius - BiomeBlendRadius;
        if (Dist <= BlendStart)
        {
            return 1.0f;
        }

        float BlendRange = BiomeBlendRadius * 2.0f;
        float BlendAlpha = (Dist - BlendStart) / BlendRange;
        return FMath::SmoothStep(0.0f, 1.0f, 1.0f - BlendAlpha);
    }

    return 0.0f;
}

bool UBiomeManager::IsLocationInBiome(FVector WorldLocation, EEng_BiomeType BiomeType) const
{
    return GetBiomeAtLocation(WorldLocation) == BiomeType;
}

TArray<EEng_BiomeType> UBiomeManager::GetNeighboringBiomes(FVector WorldLocation, float SearchRadius) const
{
    TArray<EEng_BiomeType> NeighborBiomes;
    FVector2D Loc2D(WorldLocation.X, WorldLocation.Y);

    for (const FEng_BiomeZone& Zone : ActiveBiomeZones)
    {
        if (!Zone.bIsActive) continue;

        float Dist = FVector2D::Distance(Loc2D, Zone.Center);
        if (Dist <= SearchRadius + Zone.Radius)
        {
            NeighborBiomes.AddUnique(Zone.BiomeType);
        }
    }

    return NeighborBiomes;
}

float UBiomeManager::GetTemperatureAtLocation(FVector WorldLocation) const
{
    EEng_BiomeType Biome = GetBiomeAtLocation(WorldLocation);
    FEng_BiomeConfig Config = GetBiomeConfig(Biome);

    // Add altitude cooling: -6.5°C per 1000m
    float AltitudeCooling = (WorldLocation.Z / 100.0f) * 0.65f;
    return FMath::Max(0.0f, Config.BaseTemperature - AltitudeCooling);
}

float UBiomeManager::GetHumidityAtLocation(FVector WorldLocation) const
{
    EEng_BiomeType Biome = GetBiomeAtLocation(WorldLocation);
    FEng_BiomeConfig Config = GetBiomeConfig(Biome);
    return FMath::Clamp(Config.Humidity, 0.0f, 1.0f);
}

void UBiomeManager::SetWorldSeed(int32 NewSeed)
{
    WorldSeedValue = NewSeed;
    bBiomesInitialized = false;
    InitializeBiomes();
    UE_LOG(LogTemp, Log, TEXT("[BiomeManager] World seed changed to %d, biomes re-generated"), NewSeed);
}

FName UBiomeManager::GetBiomeDisplayName(EEng_BiomeType BiomeType) const
{
    FEng_BiomeConfig Config = GetBiomeConfig(BiomeType);
    return Config.DisplayName;
}

int32 UBiomeManager::GetActiveBiomeZoneCount() const
{
    return ActiveBiomeZones.Num();
}
