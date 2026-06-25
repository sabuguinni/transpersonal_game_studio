// BiomeManager.cpp
// Core Systems Programmer — Agent #3
// P1 World Generation: Biome system implementation

#include "BiomeManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogBiomeManager, Log, All);

// ─────────────────────────────────────────────────────────────────────────────
// UBiomeManager — UWorldSubsystem
// ─────────────────────────────────────────────────────────────────────────────

void UBiomeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Seed the random stream
    NoiseRandomStream.Initialize(NoiseSeed);

    // Populate default biome definitions if none exist
    if (BiomeDefinitions.Num() == 0)
    {
        RegisterDefaultBiomes();
    }

    UE_LOG(LogBiomeManager, Log, TEXT("BiomeManager initialized. %d biomes registered. Seed=%d"),
        BiomeDefinitions.Num(), NoiseSeed);
}

void UBiomeManager::Deinitialize()
{
    BiomeDefinitions.Empty();
    Super::Deinitialize();
}

// ─────────────────────────────────────────────────────────────────────────────
// Default biome registration
// ─────────────────────────────────────────────────────────────────────────────

void UBiomeManager::RegisterDefaultBiomes()
{
    // ── Tropical Forest ──
    {
        FCore_BiomeDefinition Biome;
        Biome.BiomeType          = ECore_BiomeType::TropicalForest;
        Biome.DisplayName        = FText::FromString(TEXT("Tropical Forest"));
        Biome.MinTemperature     = 22.0f;
        Biome.MaxTemperature     = 38.0f;
        Biome.MinHumidity        = 0.65f;
        Biome.MaxHumidity        = 1.0f;
        Biome.MinAltitude        = 0.0f;
        Biome.MaxAltitude        = 800.0f;
        Biome.FoliageDensity     = 0.85f;
        Biome.WaterPresence      = 0.6f;
        Biome.DangerLevel        = 0.7f;
        Biome.FogDensity         = 0.4f;
        Biome.GroundColor        = FLinearColor(0.05f, 0.25f, 0.04f);
        Biome.FoliageColor       = FLinearColor(0.08f, 0.45f, 0.06f);
        Biome.SkyTint            = FLinearColor(0.5f, 0.8f, 1.0f);
        Biome.DinosaurSpecies.Add(TEXT("Velociraptor"));
        Biome.DinosaurSpecies.Add(TEXT("Triceratops"));
        Biome.DinosaurSpecies.Add(TEXT("Parasaurolophus"));
        Biome.ResourceTypes.Add(TEXT("Wood"));
        Biome.ResourceTypes.Add(TEXT("Fruit"));
        Biome.ResourceTypes.Add(TEXT("Medicinal_Plants"));
        BiomeDefinitions.Add(Biome);
    }

    // ── Savanna ──
    {
        FCore_BiomeDefinition Biome;
        Biome.BiomeType          = ECore_BiomeType::Savanna;
        Biome.DisplayName        = FText::FromString(TEXT("Savanna"));
        Biome.MinTemperature     = 18.0f;
        Biome.MaxTemperature     = 42.0f;
        Biome.MinHumidity        = 0.1f;
        Biome.MaxHumidity        = 0.4f;
        Biome.MinAltitude        = 0.0f;
        Biome.MaxAltitude        = 600.0f;
        Biome.FoliageDensity     = 0.25f;
        Biome.WaterPresence      = 0.2f;
        Biome.DangerLevel        = 0.8f;
        Biome.FogDensity         = 0.05f;
        Biome.GroundColor        = FLinearColor(0.55f, 0.42f, 0.12f);
        Biome.FoliageColor       = FLinearColor(0.45f, 0.52f, 0.08f);
        Biome.SkyTint            = FLinearColor(0.8f, 0.7f, 0.5f);
        Biome.DinosaurSpecies.Add(TEXT("TRex"));
        Biome.DinosaurSpecies.Add(TEXT("Triceratops"));
        Biome.DinosaurSpecies.Add(TEXT("Brachiosaurus"));
        Biome.ResourceTypes.Add(TEXT("Flint"));
        Biome.ResourceTypes.Add(TEXT("Bone"));
        Biome.ResourceTypes.Add(TEXT("Grass_Fiber"));
        BiomeDefinitions.Add(Biome);
    }

    // ── Swamp ──
    {
        FCore_BiomeDefinition Biome;
        Biome.BiomeType          = ECore_BiomeType::Swamp;
        Biome.DisplayName        = FText::FromString(TEXT("Swamp"));
        Biome.MinTemperature     = 18.0f;
        Biome.MaxTemperature     = 30.0f;
        Biome.MinHumidity        = 0.8f;
        Biome.MaxHumidity        = 1.0f;
        Biome.MinAltitude        = -50.0f;
        Biome.MaxAltitude        = 100.0f;
        Biome.FoliageDensity     = 0.7f;
        Biome.WaterPresence      = 0.9f;
        Biome.DangerLevel        = 0.85f;
        Biome.FogDensity         = 0.75f;
        Biome.GroundColor        = FLinearColor(0.1f, 0.15f, 0.05f);
        Biome.FoliageColor       = FLinearColor(0.12f, 0.28f, 0.08f);
        Biome.SkyTint            = FLinearColor(0.4f, 0.55f, 0.4f);
        Biome.DinosaurSpecies.Add(TEXT("Spinosaurus"));
        Biome.DinosaurSpecies.Add(TEXT("Sarcosuchus"));
        Biome.ResourceTypes.Add(TEXT("Clay"));
        Biome.ResourceTypes.Add(TEXT("Reed"));
        Biome.ResourceTypes.Add(TEXT("Fish"));
        BiomeDefinitions.Add(Biome);
    }

    // ── Mountain ──
    {
        FCore_BiomeDefinition Biome;
        Biome.BiomeType          = ECore_BiomeType::Mountain;
        Biome.DisplayName        = FText::FromString(TEXT("Mountain"));
        Biome.MinTemperature     = -5.0f;
        Biome.MaxTemperature     = 15.0f;
        Biome.MinHumidity        = 0.2f;
        Biome.MaxHumidity        = 0.6f;
        Biome.MinAltitude        = 800.0f;
        Biome.MaxAltitude        = 3000.0f;
        Biome.FoliageDensity     = 0.15f;
        Biome.WaterPresence      = 0.35f;
        Biome.DangerLevel        = 0.6f;
        Biome.FogDensity         = 0.3f;
        Biome.GroundColor        = FLinearColor(0.4f, 0.38f, 0.35f);
        Biome.FoliageColor       = FLinearColor(0.2f, 0.35f, 0.15f);
        Biome.SkyTint            = FLinearColor(0.6f, 0.75f, 1.0f);
        Biome.DinosaurSpecies.Add(TEXT("Pachycephalosaurus"));
        Biome.DinosaurSpecies.Add(TEXT("Pteranodon"));
        Biome.ResourceTypes.Add(TEXT("Granite"));
        Biome.ResourceTypes.Add(TEXT("Obsidian"));
        Biome.ResourceTypes.Add(TEXT("Cave_Crystal"));
        BiomeDefinitions.Add(Biome);
    }

    // ── Desert ──
    {
        FCore_BiomeDefinition Biome;
        Biome.BiomeType          = ECore_BiomeType::Desert;
        Biome.DisplayName        = FText::FromString(TEXT("Desert"));
        Biome.MinTemperature     = 25.0f;
        Biome.MaxTemperature     = 55.0f;
        Biome.MinHumidity        = 0.0f;
        Biome.MaxHumidity        = 0.15f;
        Biome.MinAltitude        = 0.0f;
        Biome.MaxAltitude        = 500.0f;
        Biome.FoliageDensity     = 0.05f;
        Biome.WaterPresence      = 0.05f;
        Biome.DangerLevel        = 0.65f;
        Biome.FogDensity         = 0.02f;
        Biome.GroundColor        = FLinearColor(0.75f, 0.6f, 0.2f);
        Biome.FoliageColor       = FLinearColor(0.5f, 0.45f, 0.1f);
        Biome.SkyTint            = FLinearColor(1.0f, 0.85f, 0.5f);
        Biome.DinosaurSpecies.Add(TEXT("Protoceratops"));
        Biome.DinosaurSpecies.Add(TEXT("Oviraptor"));
        Biome.ResourceTypes.Add(TEXT("Sandstone"));
        Biome.ResourceTypes.Add(TEXT("Cactus_Fiber"));
        BiomeDefinitions.Add(Biome);
    }

    // ── River Delta ──
    {
        FCore_BiomeDefinition Biome;
        Biome.BiomeType          = ECore_BiomeType::RiverDelta;
        Biome.DisplayName        = FText::FromString(TEXT("River Delta"));
        Biome.MinTemperature     = 15.0f;
        Biome.MaxTemperature     = 32.0f;
        Biome.MinHumidity        = 0.55f;
        Biome.MaxHumidity        = 0.85f;
        Biome.MinAltitude        = -20.0f;
        Biome.MaxAltitude        = 200.0f;
        Biome.FoliageDensity     = 0.55f;
        Biome.WaterPresence      = 0.8f;
        Biome.DangerLevel        = 0.5f;
        Biome.FogDensity         = 0.2f;
        Biome.GroundColor        = FLinearColor(0.2f, 0.3f, 0.1f);
        Biome.FoliageColor       = FLinearColor(0.15f, 0.4f, 0.1f);
        Biome.SkyTint            = FLinearColor(0.55f, 0.75f, 0.95f);
        Biome.DinosaurSpecies.Add(TEXT("Iguanodon"));
        Biome.DinosaurSpecies.Add(TEXT("Diplodocus"));
        Biome.ResourceTypes.Add(TEXT("Fresh_Water"));
        Biome.ResourceTypes.Add(TEXT("River_Clay"));
        Biome.ResourceTypes.Add(TEXT("Fish"));
        BiomeDefinitions.Add(Biome);
    }

    UE_LOG(LogBiomeManager, Log, TEXT("Registered %d default biomes"), BiomeDefinitions.Num());
}

// ─────────────────────────────────────────────────────────────────────────────
// Biome query API
// ─────────────────────────────────────────────────────────────────────────────

ECore_BiomeType UBiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    // Sample temperature and humidity from noise at this location
    const float Temperature = SampleTemperatureAtLocation(WorldLocation);
    const float Humidity    = SampleHumidityAtLocation(WorldLocation);
    const float Altitude    = WorldLocation.Z;

    return DeterminebiomeFromClimate(Temperature, Humidity, Altitude);
}

FCore_BiomeDefinition UBiomeManager::GetBiomeDefinition(ECore_BiomeType BiomeType) const
{
    for (const FCore_BiomeDefinition& Def : BiomeDefinitions)
    {
        if (Def.BiomeType == BiomeType)
        {
            return Def;
        }
    }

    // Return default (first biome) if not found
    if (BiomeDefinitions.Num() > 0)
    {
        UE_LOG(LogBiomeManager, Warning, TEXT("BiomeType %d not found — returning default"), (int32)BiomeType);
        return BiomeDefinitions[0];
    }

    return FCore_BiomeDefinition();
}

float UBiomeManager::GetBiomeBlendWeight(const FVector& WorldLocation, ECore_BiomeType BiomeType) const
{
    // Simple distance-based blend: sample 8 nearby points and average
    const float SampleRadius = BiomeBlendRadius;
    float TotalWeight = 0.0f;
    int32 MatchCount  = 0;
    const int32 Samples = 8;

    for (int32 i = 0; i < Samples; ++i)
    {
        const float Angle = (float)i / (float)Samples * 2.0f * PI;
        const FVector SamplePos = WorldLocation + FVector(
            FMath::Cos(Angle) * SampleRadius,
            FMath::Sin(Angle) * SampleRadius,
            0.0f
        );
        if (GetBiomeAtLocation(SamplePos) == BiomeType)
        {
            MatchCount++;
        }
    }

    // Include center point
    if (GetBiomeAtLocation(WorldLocation) == BiomeType)
    {
        MatchCount++;
    }

    TotalWeight = (float)MatchCount / (float)(Samples + 1);
    return TotalWeight;
}

TArray<ECore_BiomeType> UBiomeManager::GetNeighbouringBiomes(const FVector& WorldLocation) const
{
    TArray<ECore_BiomeType> Result;
    const float CheckRadius = BiomeBlendRadius;
    const int32 Directions  = 8;

    for (int32 i = 0; i < Directions; ++i)
    {
        const float Angle = (float)i / (float)Directions * 2.0f * PI;
        const FVector CheckPos = WorldLocation + FVector(
            FMath::Cos(Angle) * CheckRadius,
            FMath::Sin(Angle) * CheckRadius,
            0.0f
        );
        const ECore_BiomeType NeighbourBiome = GetBiomeAtLocation(CheckPos);
        Result.AddUnique(NeighbourBiome);
    }

    return Result;
}

bool UBiomeManager::IsLocationInBiome(const FVector& WorldLocation, ECore_BiomeType BiomeType) const
{
    return GetBiomeAtLocation(WorldLocation) == BiomeType;
}

TArray<FCore_BiomeDefinition> UBiomeManager::GetAllBiomeDefinitions() const
{
    return BiomeDefinitions;
}

// ─────────────────────────────────────────────────────────────────────────────
// Climate noise sampling
// ─────────────────────────────────────────────────────────────────────────────

float UBiomeManager::SampleTemperatureAtLocation(const FVector& WorldLocation) const
{
    // Layered noise for temperature variation
    const float Scale1 = 1.0f / (NoiseScale * 3.0f);
    const float Scale2 = 1.0f / (NoiseScale * 1.0f);

    const float NX1 = WorldLocation.X * Scale1;
    const float NY1 = WorldLocation.Y * Scale1;
    const float NX2 = WorldLocation.X * Scale2;
    const float NY2 = WorldLocation.Y * Scale2;

    // Perlin-like using FMath::PerlinNoise2D
    const float Noise1 = FMath::PerlinNoise2D(FVector2D(NX1, NY1));
    const float Noise2 = FMath::PerlinNoise2D(FVector2D(NX2, NY2)) * 0.5f;
    const float Combined = (Noise1 + Noise2) / 1.5f; // [-1, 1]

    // Map to temperature range [MinTemp, MaxTemp]
    const float T = (Combined + 1.0f) * 0.5f; // [0, 1]
    return FMath::Lerp(MinTemperature, MaxTemperature, T);
}

float UBiomeManager::SampleHumidityAtLocation(const FVector& WorldLocation) const
{
    // Offset noise coordinates to decorrelate from temperature
    const float OffsetX = 12345.0f;
    const float OffsetY = 67890.0f;
    const float Scale   = 1.0f / (NoiseScale * 2.0f);

    const float NX = (WorldLocation.X + OffsetX) * Scale;
    const float NY = (WorldLocation.Y + OffsetY) * Scale;

    const float Noise = FMath::PerlinNoise2D(FVector2D(NX, NY));
    const float T = (Noise + 1.0f) * 0.5f; // [0, 1]
    return T; // Humidity is naturally [0, 1]
}

ECore_BiomeType UBiomeManager::DeterminebiomeFromClimate(float Temperature, float Humidity, float Altitude) const
{
    // High altitude → Mountain regardless of temperature/humidity
    if (Altitude > 800.0f)
    {
        return ECore_BiomeType::Mountain;
    }

    // Very low altitude with high humidity → Swamp or River Delta
    if (Altitude < 100.0f && Humidity > 0.75f)
    {
        return (Temperature > 20.0f) ? ECore_BiomeType::Swamp : ECore_BiomeType::RiverDelta;
    }

    // Hot and dry → Desert
    if (Temperature > 35.0f && Humidity < 0.2f)
    {
        return ECore_BiomeType::Desert;
    }

    // Warm and wet → Tropical Forest
    if (Temperature > 22.0f && Humidity > 0.55f)
    {
        return ECore_BiomeType::TropicalForest;
    }

    // Warm and moderate → Savanna
    if (Temperature > 18.0f && Humidity < 0.45f)
    {
        return ECore_BiomeType::Savanna;
    }

    // Default: River Delta for remaining cases
    return ECore_BiomeType::RiverDelta;
}

// ─────────────────────────────────────────────────────────────────────────────
// Blueprint callable helpers
// ─────────────────────────────────────────────────────────────────────────────

FText UBiomeManager::GetBiomeDisplayName(ECore_BiomeType BiomeType) const
{
    return GetBiomeDefinition(BiomeType).DisplayName;
}

float UBiomeManager::GetBiomeDangerLevel(const FVector& WorldLocation) const
{
    const ECore_BiomeType Biome = GetBiomeAtLocation(WorldLocation);
    return GetBiomeDefinition(Biome).DangerLevel;
}

TArray<FString> UBiomeManager::GetDinosaurSpeciesForBiome(ECore_BiomeType BiomeType) const
{
    return GetBiomeDefinition(BiomeType).DinosaurSpecies;
}

TArray<FString> UBiomeManager::GetResourceTypesForBiome(ECore_BiomeType BiomeType) const
{
    return GetBiomeDefinition(BiomeType).ResourceTypes;
}
