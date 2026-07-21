// PCGBiomeZoneSystem.cpp
// Agent #05 — Procedural World Generator
// Full implementation of biome zone classification system

#include "PCGBiomeZoneSystem.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────
APCGBiomeZoneSystem::APCGBiomeZoneSystem()
{
    PrimaryActorTick.bCanEverTick = false;

    // Default zones are populated via InitializeDefaultBiomeZones()
    // so the constructor stays crash-safe (no world queries here)
}

// ─────────────────────────────────────────────────────────────────────────────
// BeginPlay
// ─────────────────────────────────────────────────────────────────────────────
void APCGBiomeZoneSystem::BeginPlay()
{
    Super::BeginPlay();

    if (BiomeZones.Num() == 0)
    {
        InitializeDefaultBiomeZones();
    }

    RebuildSortedCache();
}

// ─────────────────────────────────────────────────────────────────────────────
// InitializeDefaultBiomeZones
// Matches the MinPlayableMap layout (centre ~2000,2000)
// ─────────────────────────────────────────────────────────────────────────────
void APCGBiomeZoneSystem::InitializeDefaultBiomeZones()
{
    BiomeZones.Empty();

    // ── Tropical Jungle (central, around the dino clearing) ──────────────────
    {
        FWorld_BiomeZoneConfig Jungle;
        Jungle.ZoneType                  = EWorld_BiomeZoneType::TropicalJungle;
        Jungle.ZoneCenter                = FVector(2000.f, 2000.f, 0.f);
        Jungle.ZoneRadius                = 3000.f;
        Jungle.FoliageDensity            = 2.0f;
        Jungle.PrimaryVegetationPath     = TEXT("/Game/Tropical_Jungle_Pack/Meshes/SM_Tree_01");
        Jungle.SecondaryVegetationPath   = TEXT("/Game/Tropical_Jungle_Pack/Meshes/SM_Fern_01");
        Jungle.RockDensity               = 0.3f;
        Jungle.bHasWater                 = false;
        Jungle.AmbientTemperatureCelsius = 32.f;
        Jungle.Humidity                  = 0.85f;
        Jungle.PreferredDinoSpecies      = { TEXT("Velociraptor"), TEXT("Parasaurolophus") };
        BiomeZones.Add(Jungle);
    }

    // ── Open Savanna (north-east of jungle) ──────────────────────────────────
    {
        FWorld_BiomeZoneConfig Savanna;
        Savanna.ZoneType                  = EWorld_BiomeZoneType::OpenSavanna;
        Savanna.ZoneCenter                = FVector(5000.f, 5000.f, 0.f);
        Savanna.ZoneRadius                = 4000.f;
        Savanna.FoliageDensity            = 0.5f;
        Savanna.PrimaryVegetationPath     = TEXT("/Game/Tropical_Jungle_Pack/Meshes/SM_Palm_01");
        Savanna.SecondaryVegetationPath   = TEXT("");
        Savanna.RockDensity               = 0.4f;
        Savanna.bHasWater                 = false;
        Savanna.AmbientTemperatureCelsius = 38.f;
        Savanna.Humidity                  = 0.3f;
        Savanna.PreferredDinoSpecies      = { TEXT("Triceratops"), TEXT("Brachiosaurus"), TEXT("TRex") };
        BiomeZones.Add(Savanna);
    }

    // ── River Delta (south-west, water present) ───────────────────────────────
    {
        FWorld_BiomeZoneConfig River;
        River.ZoneType                  = EWorld_BiomeZoneType::RiverDelta;
        River.ZoneCenter                = FVector(-1000.f, 3000.f, 0.f);
        River.ZoneRadius                = 2500.f;
        River.FoliageDensity            = 1.5f;
        River.PrimaryVegetationPath     = TEXT("/Game/Tropical_Jungle_Pack/Meshes/SM_Tree_02");
        River.SecondaryVegetationPath   = TEXT("/Game/Tropical_Jungle_Pack/Meshes/SM_Fern_01");
        River.RockDensity               = 0.2f;
        River.bHasWater                 = true;
        River.AmbientTemperatureCelsius = 28.f;
        River.Humidity                  = 0.95f;
        River.PreferredDinoSpecies      = { TEXT("Ankylosaurus"), TEXT("Protoceratops") };
        BiomeZones.Add(River);
    }

    // ── Volcanic Plains (far north, harsh) ───────────────────────────────────
    {
        FWorld_BiomeZoneConfig Volcanic;
        Volcanic.ZoneType                  = EWorld_BiomeZoneType::VolcanicPlains;
        Volcanic.ZoneCenter                = FVector(2000.f, -4000.f, 0.f);
        Volcanic.ZoneRadius                = 3500.f;
        Volcanic.FoliageDensity            = 0.1f;
        Volcanic.PrimaryVegetationPath     = TEXT("");
        Volcanic.SecondaryVegetationPath   = TEXT("");
        Volcanic.RockDensity               = 1.5f;
        Volcanic.bHasWater                 = false;
        Volcanic.AmbientTemperatureCelsius = 55.f;
        Volcanic.Humidity                  = 0.1f;
        Volcanic.PreferredDinoSpecies      = { TEXT("TRex") };
        BiomeZones.Add(Volcanic);
    }

    // ── Rocky Highlands (west) ────────────────────────────────────────────────
    {
        FWorld_BiomeZoneConfig Highlands;
        Highlands.ZoneType                  = EWorld_BiomeZoneType::RockyHighlands;
        Highlands.ZoneCenter                = FVector(-3000.f, 0.f, 500.f);
        Highlands.ZoneRadius                = 3000.f;
        Highlands.FoliageDensity            = 0.4f;
        Highlands.PrimaryVegetationPath     = TEXT("/Game/Tropical_Jungle_Pack/Meshes/SM_Tree_03");
        Highlands.SecondaryVegetationPath   = TEXT("");
        Highlands.RockDensity               = 1.8f;
        Highlands.bHasWater                 = false;
        Highlands.AmbientTemperatureCelsius = 18.f;
        Highlands.Humidity                  = 0.4f;
        Highlands.PreferredDinoSpecies      = { TEXT("Pachycephalosaurus"), TEXT("Tsintaosaurus") };
        BiomeZones.Add(Highlands);
    }

    RebuildSortedCache();

    UE_LOG(LogTemp, Log, TEXT("[PCGBiomeZoneSystem] Initialized %d default biome zones."), BiomeZones.Num());
}

// ─────────────────────────────────────────────────────────────────────────────
// RebuildSortedCache — sort zones by radius ascending (smallest = highest priority)
// ─────────────────────────────────────────────────────────────────────────────
void APCGBiomeZoneSystem::RebuildSortedCache()
{
    SortedZoneCache = BiomeZones;
    SortedZoneCache.Sort([](const FWorld_BiomeZoneConfig& A, const FWorld_BiomeZoneConfig& B)
    {
        return A.ZoneRadius < B.ZoneRadius;
    });
}

// ─────────────────────────────────────────────────────────────────────────────
// GetBiomeAtLocation
// ─────────────────────────────────────────────────────────────────────────────
EWorld_BiomeZoneType APCGBiomeZoneSystem::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    for (const FWorld_BiomeZoneConfig& Zone : SortedZoneCache)
    {
        const float DistSq = FVector::DistSquaredXY(WorldLocation, Zone.ZoneCenter);
        if (DistSq <= (Zone.ZoneRadius * Zone.ZoneRadius))
        {
            return Zone.ZoneType;
        }
    }
    // Default fallback
    return EWorld_BiomeZoneType::OpenSavanna;
}

// ─────────────────────────────────────────────────────────────────────────────
// GetBiomeConfigAtLocation
// ─────────────────────────────────────────────────────────────────────────────
bool APCGBiomeZoneSystem::GetBiomeConfigAtLocation(const FVector& WorldLocation, FWorld_BiomeZoneConfig& OutConfig) const
{
    for (const FWorld_BiomeZoneConfig& Zone : SortedZoneCache)
    {
        const float DistSq = FVector::DistSquaredXY(WorldLocation, Zone.ZoneCenter);
        if (DistSq <= (Zone.ZoneRadius * Zone.ZoneRadius))
        {
            OutConfig = Zone;
            return true;
        }
    }
    return false;
}

// ─────────────────────────────────────────────────────────────────────────────
// GetFoliageDensityAtLocation
// ─────────────────────────────────────────────────────────────────────────────
float APCGBiomeZoneSystem::GetFoliageDensityAtLocation(const FVector& WorldLocation) const
{
    FWorld_BiomeZoneConfig Config;
    if (GetBiomeConfigAtLocation(WorldLocation, Config))
    {
        return Config.FoliageDensity;
    }
    return 1.f; // default density
}

// ─────────────────────────────────────────────────────────────────────────────
// GetTemperatureAtLocation
// ─────────────────────────────────────────────────────────────────────────────
float APCGBiomeZoneSystem::GetTemperatureAtLocation(const FVector& WorldLocation) const
{
    FWorld_BiomeZoneConfig Config;
    if (GetBiomeConfigAtLocation(WorldLocation, Config))
    {
        return Config.AmbientTemperatureCelsius;
    }
    return 25.f; // default temperature
}

// ─────────────────────────────────────────────────────────────────────────────
// GetHumidityAtLocation
// ─────────────────────────────────────────────────────────────────────────────
float APCGBiomeZoneSystem::GetHumidityAtLocation(const FVector& WorldLocation) const
{
    FWorld_BiomeZoneConfig Config;
    if (GetBiomeConfigAtLocation(WorldLocation, Config))
    {
        return Config.Humidity;
    }
    return 0.5f; // default humidity
}

// ─────────────────────────────────────────────────────────────────────────────
// DrawDebugZones — editor helper
// ─────────────────────────────────────────────────────────────────────────────
void APCGBiomeZoneSystem::DrawDebugZones()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    static const TMap<EWorld_BiomeZoneType, FColor> ZoneColors = {
        { EWorld_BiomeZoneType::TropicalJungle,  FColor::Green  },
        { EWorld_BiomeZoneType::VolcanicPlains,  FColor::Red    },
        { EWorld_BiomeZoneType::RiverDelta,      FColor::Blue   },
        { EWorld_BiomeZoneType::OpenSavanna,     FColor::Yellow },
        { EWorld_BiomeZoneType::RockyHighlands,  FColor::White  },
    };

    for (const FWorld_BiomeZoneConfig& Zone : BiomeZones)
    {
        const FColor* ColorPtr = ZoneColors.Find(Zone.ZoneType);
        const FColor  Color    = ColorPtr ? *ColorPtr : FColor::Magenta;

        DrawDebugCylinder(
            World,
            Zone.ZoneCenter - FVector(0.f, 0.f, 100.f),
            Zone.ZoneCenter + FVector(0.f, 0.f, 100.f),
            Zone.ZoneRadius,
            32,
            Color,
            false,  // persistent
            10.f    // lifetime seconds
        );

        DrawDebugString(
            World,
            Zone.ZoneCenter + FVector(0.f, 0.f, 200.f),
            UEnum::GetValueAsString(Zone.ZoneType),
            nullptr,
            Color,
            10.f
        );
    }
}
