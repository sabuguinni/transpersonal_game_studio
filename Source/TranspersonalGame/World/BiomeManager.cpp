// BiomeManager.cpp
// Engine Architect #02 — Transpersonal Game Studio
// Biome system implementation for prehistoric survival world
// P1 Priority: World Generation

#include "BiomeManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UBiomeManager::UBiomeManager()
{
    // Default biome configuration — Cretaceous period
    CurrentBiomeType = EEng_BiomeType::CretaceousForest;
    bBiomeSystemInitialized = false;
    BiomeTransitionBlendRadius = 2000.0f;
    GlobalTemperatureMultiplier = 1.0f;
    GlobalHumidityMultiplier = 1.0f;
}

void UBiomeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    InitializeBiomeData();
    bBiomeSystemInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Initialized with %d biome definitions"), BiomeDefinitions.Num());
}

void UBiomeManager::Deinitialize()
{
    BiomeDefinitions.Empty();
    ActiveBiomeZones.Empty();
    bBiomeSystemInitialized = false;
    Super::Deinitialize();
}

void UBiomeManager::InitializeBiomeData()
{
    BiomeDefinitions.Empty();

    // ── Cretaceous Forest ──
    {
        FEng_BiomeDefinition Forest;
        Forest.BiomeType = EEng_BiomeType::CretaceousForest;
        Forest.DisplayName = FText::FromString(TEXT("Cretaceous Forest"));
        Forest.BaseTemperature = 28.0f;
        Forest.BaseHumidity = 0.75f;
        Forest.FoliageDensity = 0.9f;
        Forest.DangerLevel = 0.6f;
        Forest.FogDensity = 0.03f;
        Forest.AmbientColorTint = FLinearColor(0.4f, 0.6f, 0.3f, 1.0f);
        Forest.DinosaurSpecies.Add(TEXT("Raptor"));
        Forest.DinosaurSpecies.Add(TEXT("Triceratops"));
        Forest.DinosaurSpecies.Add(TEXT("Brachiosaurus"));
        BiomeDefinitions.Add(EEng_BiomeType::CretaceousForest, Forest);
    }

    // ── Volcanic Badlands ──
    {
        FEng_BiomeDefinition Volcanic;
        Volcanic.BiomeType = EEng_BiomeType::VolcanicBadlands;
        Volcanic.DisplayName = FText::FromString(TEXT("Volcanic Badlands"));
        Volcanic.BaseTemperature = 55.0f;
        Volcanic.BaseHumidity = 0.15f;
        Volcanic.FoliageDensity = 0.1f;
        Volcanic.DangerLevel = 0.95f;
        Volcanic.FogDensity = 0.08f;
        Volcanic.AmbientColorTint = FLinearColor(0.8f, 0.3f, 0.1f, 1.0f);
        Volcanic.DinosaurSpecies.Add(TEXT("TRex"));
        Volcanic.DinosaurSpecies.Add(TEXT("Ankylosaur"));
        BiomeDefinitions.Add(EEng_BiomeType::VolcanicBadlands, Volcanic);
    }

    // ── Coastal Wetlands ──
    {
        FEng_BiomeDefinition Wetlands;
        Wetlands.BiomeType = EEng_BiomeType::CoastalWetlands;
        Wetlands.DisplayName = FText::FromString(TEXT("Coastal Wetlands"));
        Wetlands.BaseTemperature = 32.0f;
        Wetlands.BaseHumidity = 0.95f;
        Wetlands.FoliageDensity = 0.7f;
        Wetlands.DangerLevel = 0.5f;
        Wetlands.FogDensity = 0.06f;
        Wetlands.AmbientColorTint = FLinearColor(0.3f, 0.5f, 0.6f, 1.0f);
        Wetlands.DinosaurSpecies.Add(TEXT("Spinosaurus"));
        Wetlands.DinosaurSpecies.Add(TEXT("Pterodactyl"));
        BiomeDefinitions.Add(EEng_BiomeType::CoastalWetlands, Wetlands);
    }

    // ── Open Savanna ──
    {
        FEng_BiomeDefinition Savanna;
        Savanna.BiomeType = EEng_BiomeType::OpenSavanna;
        Savanna.DisplayName = FText::FromString(TEXT("Open Savanna"));
        Savanna.BaseTemperature = 38.0f;
        Savanna.BaseHumidity = 0.3f;
        Savanna.FoliageDensity = 0.3f;
        Savanna.DangerLevel = 0.7f;
        Savanna.FogDensity = 0.01f;
        Savanna.AmbientColorTint = FLinearColor(0.9f, 0.8f, 0.5f, 1.0f);
        Savanna.DinosaurSpecies.Add(TEXT("TRex"));
        Savanna.DinosaurSpecies.Add(TEXT("Raptor"));
        Savanna.DinosaurSpecies.Add(TEXT("Triceratops"));
        BiomeDefinitions.Add(EEng_BiomeType::OpenSavanna, Savanna);
    }

    // ── Mountain Highlands ──
    {
        FEng_BiomeDefinition Mountains;
        Mountains.BiomeType = EEng_BiomeType::MountainHighlands;
        Mountains.DisplayName = FText::FromString(TEXT("Mountain Highlands"));
        Mountains.BaseTemperature = 12.0f;
        Mountains.BaseHumidity = 0.45f;
        Mountains.FoliageDensity = 0.4f;
        Mountains.DangerLevel = 0.65f;
        Mountains.FogDensity = 0.05f;
        Mountains.AmbientColorTint = FLinearColor(0.6f, 0.7f, 0.8f, 1.0f);
        Mountains.DinosaurSpecies.Add(TEXT("Pterodactyl"));
        Mountains.DinosaurSpecies.Add(TEXT("Pachycephalosaurus"));
        BiomeDefinitions.Add(EEng_BiomeType::MountainHighlands, Mountains);
    }

    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Loaded %d biome definitions"), BiomeDefinitions.Num());
}

EEng_BiomeType UBiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    if (!bBiomeSystemInitialized)
    {
        return EEng_BiomeType::CretaceousForest;
    }

    // Check active biome zones first
    for (const FEng_BiomeZone& Zone : ActiveBiomeZones)
    {
        float DistSq = FVector::DistSquared(WorldLocation, Zone.ZoneCenter);
        if (DistSq <= (Zone.ZoneRadius * Zone.ZoneRadius))
        {
            return Zone.BiomeType;
        }
    }

    // Fallback: determine biome by altitude and distance from origin
    float Altitude = WorldLocation.Z;
    float DistFromOrigin = FVector2D(WorldLocation.X, WorldLocation.Y).Size();

    if (Altitude > 5000.0f)
    {
        return EEng_BiomeType::MountainHighlands;
    }
    else if (Altitude < -500.0f || DistFromOrigin > 50000.0f)
    {
        return EEng_BiomeType::CoastalWetlands;
    }
    else if (DistFromOrigin > 30000.0f)
    {
        return EEng_BiomeType::VolcanicBadlands;
    }
    else if (DistFromOrigin > 15000.0f)
    {
        return EEng_BiomeType::OpenSavanna;
    }

    return EEng_BiomeType::CretaceousForest;
}

FEng_BiomeDefinition UBiomeManager::GetBiomeDefinition(EEng_BiomeType BiomeType) const
{
    const FEng_BiomeDefinition* Found = BiomeDefinitions.Find(BiomeType);
    if (Found)
    {
        return *Found;
    }

    // Return default forest biome if not found
    FEng_BiomeDefinition Default;
    Default.BiomeType = EEng_BiomeType::CretaceousForest;
    Default.DisplayName = FText::FromString(TEXT("Unknown Biome"));
    Default.BaseTemperature = 25.0f;
    Default.BaseHumidity = 0.5f;
    Default.FoliageDensity = 0.5f;
    Default.DangerLevel = 0.5f;
    Default.FogDensity = 0.02f;
    return Default;
}

float UBiomeManager::GetTemperatureAtLocation(const FVector& WorldLocation) const
{
    EEng_BiomeType Biome = GetBiomeAtLocation(WorldLocation);
    FEng_BiomeDefinition Def = GetBiomeDefinition(Biome);

    // Altitude cooling: -6.5°C per 1000m (realistic lapse rate)
    float AltitudeCooling = FMath::Max(0.0f, WorldLocation.Z / 100.0f) * 0.65f;

    return (Def.BaseTemperature - AltitudeCooling) * GlobalTemperatureMultiplier;
}

float UBiomeManager::GetHumidityAtLocation(const FVector& WorldLocation) const
{
    EEng_BiomeType Biome = GetBiomeAtLocation(WorldLocation);
    FEng_BiomeDefinition Def = GetBiomeDefinition(Biome);
    return FMath::Clamp(Def.BaseHumidity * GlobalHumidityMultiplier, 0.0f, 1.0f);
}

void UBiomeManager::RegisterBiomeZone(const FEng_BiomeZone& Zone)
{
    // Check for duplicate zone IDs
    for (FEng_BiomeZone& Existing : ActiveBiomeZones)
    {
        if (Existing.ZoneID == Zone.ZoneID)
        {
            Existing = Zone;  // Update existing
            UE_LOG(LogTemp, Log, TEXT("BiomeManager: Updated zone %s"), *Zone.ZoneID.ToString());
            return;
        }
    }

    ActiveBiomeZones.Add(Zone);
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Registered zone %s (biome=%d, radius=%.0f)"),
        *Zone.ZoneID.ToString(), (int32)Zone.BiomeType, Zone.ZoneRadius);
}

void UBiomeManager::UnregisterBiomeZone(const FName& ZoneID)
{
    ActiveBiomeZones.RemoveAll([&ZoneID](const FEng_BiomeZone& Zone)
    {
        return Zone.ZoneID == ZoneID;
    });
}

TArray<FName> UBiomeManager::GetDinosaurSpeciesForBiome(EEng_BiomeType BiomeType) const
{
    FEng_BiomeDefinition Def = GetBiomeDefinition(BiomeType);
    return Def.DinosaurSpecies;
}

float UBiomeManager::GetBiomeBlendWeight(const FVector& WorldLocation, EEng_BiomeType BiomeType) const
{
    // Find nearest zone of this biome type
    float MinDist = TNumericLimits<float>::Max();

    for (const FEng_BiomeZone& Zone : ActiveBiomeZones)
    {
        if (Zone.BiomeType == BiomeType)
        {
            float Dist = FVector::Dist(WorldLocation, Zone.ZoneCenter);
            MinDist = FMath::Min(MinDist, Dist);
        }
    }

    if (MinDist == TNumericLimits<float>::Max())
    {
        // No zones of this type — check if it's the default biome
        EEng_BiomeType DefaultBiome = GetBiomeAtLocation(WorldLocation);
        return (DefaultBiome == BiomeType) ? 1.0f : 0.0f;
    }

    // Blend weight falls off over BiomeTransitionBlendRadius
    return FMath::Clamp(1.0f - (MinDist / BiomeTransitionBlendRadius), 0.0f, 1.0f);
}
