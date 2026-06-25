#include "PCGBiomeZoneConfig.h"

UPCGBiomeZoneConfig::UPCGBiomeZoneConfig()
{
    // Initialize with default Cretaceous world configuration
    InitializeDefaultCretaceousConfig();
}

EWorld_BiomeZone UPCGBiomeZoneConfig::GetBiomeAtLocation(FVector WorldLocation) const
{
    float ClosestDistance = FLT_MAX;
    EWorld_BiomeZone ClosestBiome = EWorld_BiomeZone::Savanna;

    for (const FWorld_BiomeZoneData& Zone : BiomeZones)
    {
        float Dist = FVector::Dist2D(WorldLocation, Zone.ZoneCenter);
        if (Dist < ClosestDistance && Dist <= Zone.ZoneRadius)
        {
            ClosestDistance = Dist;
            ClosestBiome = Zone.ZoneType;
        }
    }

    return ClosestBiome;
}

bool UPCGBiomeZoneConfig::GetBiomeData(EWorld_BiomeZone ZoneType, FWorld_BiomeZoneData& OutData) const
{
    for (const FWorld_BiomeZoneData& Zone : BiomeZones)
    {
        if (Zone.ZoneType == ZoneType)
        {
            OutData = Zone;
            return true;
        }
    }
    return false;
}

void UPCGBiomeZoneConfig::InitializeDefaultCretaceousConfig()
{
    BiomeZones.Empty();

    // === BIOME 0: DENSE JUNGLE ===
    // Center of the map, around the main dinosaur spawns
    {
        FWorld_BiomeZoneData Jungle;
        Jungle.ZoneType = EWorld_BiomeZone::Jungle;
        Jungle.ZoneCenter = FVector(2000.0f, 2500.0f, 0.0f);
        Jungle.ZoneRadius = 3000.0f;
        Jungle.VegetationDensity = 0.95f;
        Jungle.TerrainHeightVariation = 300.0f;
        Jungle.RockDensity = 0.2f;
        Jungle.FogDensityMultiplier = 1.8f;
        Jungle.FoliageTint = FLinearColor(0.1f, 0.55f, 0.05f, 1.0f);
        Jungle.bHasWaterFeatures = true;
        Jungle.PreferredDinoSpecies.Add(FName("Velociraptor"));
        Jungle.PreferredDinoSpecies.Add(FName("Trex"));
        Jungle.PreferredDinoSpecies.Add(FName("Parasaurolophus"));
        BiomeZones.Add(Jungle);
    }

    // === BIOME 1: OPEN SAVANNA ===
    // Eastern area, wide open plains with sparse trees
    {
        FWorld_BiomeZoneData Savanna;
        Savanna.ZoneType = EWorld_BiomeZone::Savanna;
        Savanna.ZoneCenter = FVector(4000.0f, 2000.0f, 0.0f);
        Savanna.ZoneRadius = 3500.0f;
        Savanna.VegetationDensity = 0.3f;
        Savanna.TerrainHeightVariation = 100.0f;
        Savanna.RockDensity = 0.15f;
        Savanna.FogDensityMultiplier = 0.6f;
        Savanna.FoliageTint = FLinearColor(0.45f, 0.55f, 0.1f, 1.0f);
        Savanna.bHasWaterFeatures = false;
        Savanna.PreferredDinoSpecies.Add(FName("Triceratops"));
        Savanna.PreferredDinoSpecies.Add(FName("Brachiosaurus"));
        Savanna.PreferredDinoSpecies.Add(FName("Pachycephalo"));
        BiomeZones.Add(Savanna);
    }

    // === BIOME 2: ROCKY HIGHLANDS ===
    // Northern area, elevated terrain with boulder clusters
    {
        FWorld_BiomeZoneData Highlands;
        Highlands.ZoneType = EWorld_BiomeZone::Highlands;
        Highlands.ZoneCenter = FVector(1200.0f, 4800.0f, 500.0f);
        Highlands.ZoneRadius = 2500.0f;
        Highlands.VegetationDensity = 0.2f;
        Highlands.TerrainHeightVariation = 800.0f;
        Highlands.RockDensity = 0.85f;
        Highlands.FogDensityMultiplier = 1.2f;
        Highlands.FoliageTint = FLinearColor(0.25f, 0.4f, 0.1f, 1.0f);
        Highlands.bHasWaterFeatures = false;
        Highlands.PreferredDinoSpecies.Add(FName("Ankylosaurus"));
        Highlands.PreferredDinoSpecies.Add(FName("Protoceratops"));
        BiomeZones.Add(Highlands);
    }

    // === BIOME 3: RIVER BANK ===
    // Winding river corridor connecting jungle to savanna
    {
        FWorld_BiomeZoneData Riverbank;
        Riverbank.ZoneType = EWorld_BiomeZone::Riverbank;
        Riverbank.ZoneCenter = FVector(3000.0f, 3200.0f, -50.0f);
        Riverbank.ZoneRadius = 1200.0f;
        Riverbank.VegetationDensity = 0.7f;
        Riverbank.TerrainHeightVariation = 80.0f;
        Riverbank.RockDensity = 0.1f;
        Riverbank.FogDensityMultiplier = 2.0f;
        Riverbank.FoliageTint = FLinearColor(0.15f, 0.65f, 0.15f, 1.0f);
        Riverbank.bHasWaterFeatures = true;
        Riverbank.PreferredDinoSpecies.Add(FName("Tsintaosaurus"));
        Riverbank.PreferredDinoSpecies.Add(FName("Brachiosaurus"));
        BiomeZones.Add(Riverbank);
    }
}
