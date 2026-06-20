// BiomeManager.cpp
// Engine Architect #02 — P1 World Generation
// Implements biome classification, terrain parameter lookup, and runtime biome queries.

#include "BiomeManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

UBiomeManager::UBiomeManager()
{
    // Default biome table — populated at startup; designers can override via DataTable
    BiomeTable.Reserve(8);

    // Coastal wetlands
    FEng_BiomeDefinition Coastal;
    Coastal.BiomeType          = EEng_BiomeType::CoastalWetlands;
    Coastal.DisplayName        = FText::FromString(TEXT("Coastal Wetlands"));
    Coastal.MinTemperature     = 18.f;
    Coastal.MaxTemperature     = 32.f;
    Coastal.MinHumidity        = 0.7f;
    Coastal.MaxHumidity        = 1.0f;
    Coastal.MinAltitude        = -10.f;
    Coastal.MaxAltitude        = 80.f;
    Coastal.FoliageDensity     = 0.85f;
    Coastal.WaterCoverage      = 0.6f;
    Coastal.DangerLevel        = 2;
    Coastal.GroundColor        = FLinearColor(0.18f, 0.25f, 0.08f, 1.f);
    BiomeTable.Add(Coastal);

    // Dense jungle
    FEng_BiomeDefinition Jungle;
    Jungle.BiomeType           = EEng_BiomeType::DenseJungle;
    Jungle.DisplayName         = FText::FromString(TEXT("Dense Jungle"));
    Jungle.MinTemperature      = 24.f;
    Jungle.MaxTemperature      = 38.f;
    Jungle.MinHumidity         = 0.8f;
    Jungle.MaxHumidity         = 1.0f;
    Jungle.MinAltitude         = 0.f;
    Jungle.MaxAltitude         = 400.f;
    Jungle.FoliageDensity      = 1.0f;
    Jungle.WaterCoverage       = 0.15f;
    Jungle.DangerLevel         = 5;
    Jungle.GroundColor         = FLinearColor(0.08f, 0.22f, 0.04f, 1.f);
    BiomeTable.Add(Jungle);

    // Open savanna
    FEng_BiomeDefinition Savanna;
    Savanna.BiomeType          = EEng_BiomeType::OpenSavanna;
    Savanna.DisplayName        = FText::FromString(TEXT("Open Savanna"));
    Savanna.MinTemperature     = 20.f;
    Savanna.MaxTemperature     = 42.f;
    Savanna.MinHumidity        = 0.1f;
    Savanna.MaxHumidity        = 0.45f;
    Savanna.MinAltitude        = 50.f;
    Savanna.MaxAltitude        = 600.f;
    Savanna.FoliageDensity     = 0.3f;
    Savanna.WaterCoverage      = 0.05f;
    Savanna.DangerLevel        = 3;
    Savanna.GroundColor        = FLinearColor(0.55f, 0.45f, 0.12f, 1.f);
    BiomeTable.Add(Savanna);

    // Volcanic highlands
    FEng_BiomeDefinition Volcanic;
    Volcanic.BiomeType         = EEng_BiomeType::VolcanicHighlands;
    Volcanic.DisplayName       = FText::FromString(TEXT("Volcanic Highlands"));
    Volcanic.MinTemperature    = 30.f;
    Volcanic.MaxTemperature    = 60.f;
    Volcanic.MinHumidity       = 0.0f;
    Volcanic.MaxHumidity       = 0.3f;
    Volcanic.MinAltitude       = 800.f;
    Volcanic.MaxAltitude       = 3000.f;
    Volcanic.FoliageDensity    = 0.05f;
    Volcanic.WaterCoverage     = 0.0f;
    Volcanic.DangerLevel       = 8;
    Volcanic.GroundColor       = FLinearColor(0.25f, 0.08f, 0.02f, 1.f);
    BiomeTable.Add(Volcanic);

    // River delta
    FEng_BiomeDefinition RiverDelta;
    RiverDelta.BiomeType       = EEng_BiomeType::RiverDelta;
    RiverDelta.DisplayName     = FText::FromString(TEXT("River Delta"));
    RiverDelta.MinTemperature  = 16.f;
    RiverDelta.MaxTemperature  = 30.f;
    RiverDelta.MinHumidity     = 0.6f;
    RiverDelta.MaxHumidity     = 0.95f;
    RiverDelta.MinAltitude     = -5.f;
    RiverDelta.MaxAltitude     = 50.f;
    RiverDelta.FoliageDensity  = 0.7f;
    RiverDelta.WaterCoverage   = 0.8f;
    RiverDelta.DangerLevel     = 4;
    RiverDelta.GroundColor     = FLinearColor(0.3f, 0.2f, 0.05f, 1.f);
    BiomeTable.Add(RiverDelta);

    // Arid badlands
    FEng_BiomeDefinition Badlands;
    Badlands.BiomeType         = EEng_BiomeType::AridBadlands;
    Badlands.DisplayName       = FText::FromString(TEXT("Arid Badlands"));
    Badlands.MinTemperature    = 28.f;
    Badlands.MaxTemperature    = 55.f;
    Badlands.MinHumidity       = 0.0f;
    Badlands.MaxHumidity       = 0.15f;
    Badlands.MinAltitude       = 100.f;
    Badlands.MaxAltitude       = 900.f;
    Badlands.FoliageDensity    = 0.02f;
    Badlands.WaterCoverage     = 0.0f;
    Badlands.DangerLevel       = 6;
    Badlands.GroundColor       = FLinearColor(0.65f, 0.35f, 0.1f, 1.f);
    BiomeTable.Add(Badlands);

    // Temperate forest
    FEng_BiomeDefinition TempForest;
    TempForest.BiomeType       = EEng_BiomeType::TemperateForest;
    TempForest.DisplayName     = FText::FromString(TEXT("Temperate Forest"));
    TempForest.MinTemperature  = 5.f;
    TempForest.MaxTemperature  = 25.f;
    TempForest.MinHumidity     = 0.4f;
    TempForest.MaxHumidity     = 0.75f;
    TempForest.MinAltitude     = 100.f;
    TempForest.MaxAltitude     = 1200.f;
    TempForest.FoliageDensity  = 0.75f;
    TempForest.WaterCoverage   = 0.1f;
    TempForest.DangerLevel     = 3;
    TempForest.GroundColor     = FLinearColor(0.12f, 0.28f, 0.06f, 1.f);
    BiomeTable.Add(TempForest);

    // Mountain peaks
    FEng_BiomeDefinition Mountain;
    Mountain.BiomeType         = EEng_BiomeType::MountainPeaks;
    Mountain.DisplayName       = FText::FromString(TEXT("Mountain Peaks"));
    Mountain.MinTemperature    = -10.f;
    Mountain.MaxTemperature    = 10.f;
    Mountain.MinHumidity       = 0.2f;
    Mountain.MaxHumidity       = 0.6f;
    Mountain.MinAltitude       = 1500.f;
    Mountain.MaxAltitude       = 5000.f;
    Mountain.FoliageDensity    = 0.05f;
    Mountain.WaterCoverage     = 0.0f;
    Mountain.DangerLevel       = 7;
    Mountain.GroundColor       = FLinearColor(0.5f, 0.5f, 0.5f, 1.f);
    BiomeTable.Add(Mountain);

    bIsInitialized = false;
}

// ─────────────────────────────────────────────────────────────────────────────
// UGameInstanceSubsystem interface
// ─────────────────────────────────────────────────────────────────────────────

void UBiomeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    bIsInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Initialized with %d biome definitions."), BiomeTable.Num());
}

void UBiomeManager::Deinitialize()
{
    BiomeTable.Empty();
    BiomeCache.Empty();
    bIsInitialized = false;
    Super::Deinitialize();
}

// ─────────────────────────────────────────────────────────────────────────────
// Core biome query
// ─────────────────────────────────────────────────────────────────────────────

EEng_BiomeType UBiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    // Check cache first (grid-snapped to 1000 UU cells)
    const FIntPoint CacheKey = WorldLocationToCacheKey(WorldLocation);
    if (const EEng_BiomeType* Cached = BiomeCache.Find(CacheKey))
    {
        return *Cached;
    }

    // Sample climate parameters at this location
    const float Temperature = SampleTemperature(WorldLocation);
    const float Humidity    = SampleHumidity(WorldLocation);
    const float Altitude    = WorldLocation.Z;

    EEng_BiomeType Result = ClassifyBiome(Temperature, Humidity, Altitude);

    // Cache result
    BiomeCache.Add(CacheKey, Result);
    return Result;
}

bool UBiomeManager::GetBiomeDefinition(EEng_BiomeType BiomeType, FEng_BiomeDefinition& OutDefinition) const
{
    for (const FEng_BiomeDefinition& Def : BiomeTable)
    {
        if (Def.BiomeType == BiomeType)
        {
            OutDefinition = Def;
            return true;
        }
    }
    return false;
}

float UBiomeManager::GetFoliageDensityAt(const FVector& WorldLocation) const
{
    FEng_BiomeDefinition Def;
    if (GetBiomeDefinition(GetBiomeAtLocation(WorldLocation), Def))
    {
        return Def.FoliageDensity;
    }
    return 0.5f;
}

int32 UBiomeManager::GetDangerLevelAt(const FVector& WorldLocation) const
{
    FEng_BiomeDefinition Def;
    if (GetBiomeDefinition(GetBiomeAtLocation(WorldLocation), Def))
    {
        return Def.DangerLevel;
    }
    return 1;
}

FLinearColor UBiomeManager::GetGroundColorAt(const FVector& WorldLocation) const
{
    FEng_BiomeDefinition Def;
    if (GetBiomeDefinition(GetBiomeAtLocation(WorldLocation), Def))
    {
        return Def.GroundColor;
    }
    return FLinearColor(0.3f, 0.25f, 0.1f, 1.f);
}

TArray<EEng_BiomeType> UBiomeManager::GetAllBiomeTypes() const
{
    TArray<EEng_BiomeType> Result;
    Result.Reserve(BiomeTable.Num());
    for (const FEng_BiomeDefinition& Def : BiomeTable)
    {
        Result.Add(Def.BiomeType);
    }
    return Result;
}

// ─────────────────────────────────────────────────────────────────────────────
// Climate sampling (noise-based, no external dependencies)
// ─────────────────────────────────────────────────────────────────────────────

float UBiomeManager::SampleTemperature(const FVector& WorldLocation) const
{
    // Simple latitude-based gradient + altitude cooling + noise
    const float LatitudeFactor = FMath::Abs(WorldLocation.Y) / 500000.f; // 0=equator, 1=pole
    const float BaseTemp       = FMath::Lerp(38.f, -5.f, FMath::Clamp(LatitudeFactor, 0.f, 1.f));
    const float AltitudeCool   = FMath::Max(0.f, WorldLocation.Z / 100.f) * 0.65f; // -0.65°C per 100m
    const float Noise          = FMath::Sin(WorldLocation.X * 0.00003f) * 4.f
                               + FMath::Cos(WorldLocation.Y * 0.00005f) * 3.f;
    return BaseTemp - AltitudeCool + Noise;
}

float UBiomeManager::SampleHumidity(const FVector& WorldLocation) const
{
    // Proximity to water bodies approximated by noise
    const float BaseHumidity = 0.5f
        + FMath::Sin(WorldLocation.X * 0.00004f + WorldLocation.Y * 0.00002f) * 0.3f
        + FMath::Cos(WorldLocation.Y * 0.00007f) * 0.2f;
    return FMath::Clamp(BaseHumidity, 0.f, 1.f);
}

// ─────────────────────────────────────────────────────────────────────────────
// Classification logic
// ─────────────────────────────────────────────────────────────────────────────

EEng_BiomeType UBiomeManager::ClassifyBiome(float Temperature, float Humidity, float Altitude) const
{
    // High altitude overrides everything
    if (Altitude >= 1500.f)  return EEng_BiomeType::MountainPeaks;
    if (Altitude >= 800.f && Temperature >= 30.f) return EEng_BiomeType::VolcanicHighlands;

    // Water-dominated low zones
    if (Altitude <= 50.f && Humidity >= 0.6f) return EEng_BiomeType::CoastalWetlands;
    if (Altitude <= 50.f && Humidity >= 0.8f) return EEng_BiomeType::RiverDelta;

    // Hot + wet = jungle
    if (Temperature >= 24.f && Humidity >= 0.75f) return EEng_BiomeType::DenseJungle;

    // Hot + dry = badlands / savanna
    if (Temperature >= 28.f && Humidity <= 0.15f) return EEng_BiomeType::AridBadlands;
    if (Temperature >= 20.f && Humidity <= 0.45f) return EEng_BiomeType::OpenSavanna;

    // Default temperate
    return EEng_BiomeType::TemperateForest;
}

// ─────────────────────────────────────────────────────────────────────────────
// Cache helpers
// ─────────────────────────────────────────────────────────────────────────────

FIntPoint UBiomeManager::WorldLocationToCacheKey(const FVector& WorldLocation) const
{
    // Snap to 1000 UU grid
    return FIntPoint(
        FMath::FloorToInt(WorldLocation.X / 1000.f),
        FMath::FloorToInt(WorldLocation.Y / 1000.f)
    );
}

void UBiomeManager::InvalidateCache()
{
    BiomeCache.Empty();
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Cache invalidated."));
}
