// BiomeManager.cpp
// Transpersonal Game Studio — Engine Architect #02
// Full implementation of the Biome system for the prehistoric survival world.
// Manages biome classification, blending, and environmental parameter queries.

#include "BiomeManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

// ============================================================
// Constructor
// ============================================================

UBiomeManager::UBiomeManager()
{
    // Default biome table — populated at runtime from data assets
    // These defaults ensure the system works even without external data
    BiomeTable.Empty();

    // Register default biomes so the system is never empty
    FEng_BiomeDefinition Forest;
    Forest.BiomeID = EEng_BiomeType::TropicalForest;
    Forest.DisplayName = FText::FromString(TEXT("Tropical Forest"));
    Forest.TemperatureRange = FVector2D(20.0f, 35.0f);
    Forest.HumidityRange    = FVector2D(0.6f, 1.0f);
    Forest.AltitudeRange    = FVector2D(0.0f, 400.0f);
    Forest.FoliageDensity   = 0.85f;
    Forest.DangerLevel      = 0.6f;
    Forest.bHasPredators    = true;
    BiomeTable.Add(Forest);

    FEng_BiomeDefinition Plains;
    Plains.BiomeID = EEng_BiomeType::OpenPlains;
    Plains.DisplayName = FText::FromString(TEXT("Open Plains"));
    Plains.TemperatureRange = FVector2D(15.0f, 40.0f);
    Plains.HumidityRange    = FVector2D(0.2f, 0.5f);
    Plains.AltitudeRange    = FVector2D(0.0f, 200.0f);
    Plains.FoliageDensity   = 0.25f;
    Plains.DangerLevel      = 0.4f;
    Plains.bHasPredators    = true;
    BiomeTable.Add(Plains);

    FEng_BiomeDefinition Swamp;
    Swamp.BiomeID = EEng_BiomeType::Swampland;
    Swamp.DisplayName = FText::FromString(TEXT("Swampland"));
    Swamp.TemperatureRange = FVector2D(18.0f, 30.0f);
    Swamp.HumidityRange    = FVector2D(0.8f, 1.0f);
    Swamp.AltitudeRange    = FVector2D(-20.0f, 50.0f);
    Swamp.FoliageDensity   = 0.7f;
    Swamp.DangerLevel      = 0.75f;
    Swamp.bHasPredators    = true;
    BiomeTable.Add(Swamp);

    FEng_BiomeDefinition Volcanic;
    Volcanic.BiomeID = EEng_BiomeType::VolcanicBadlands;
    Volcanic.DisplayName = FText::FromString(TEXT("Volcanic Badlands"));
    Volcanic.TemperatureRange = FVector2D(35.0f, 80.0f);
    Volcanic.HumidityRange    = FVector2D(0.0f, 0.2f);
    Volcanic.AltitudeRange    = FVector2D(200.0f, 1200.0f);
    Volcanic.FoliageDensity   = 0.05f;
    Volcanic.DangerLevel      = 0.95f;
    Volcanic.bHasPredators    = false;
    BiomeTable.Add(Volcanic);

    FEng_BiomeDefinition Coastal;
    Coastal.BiomeID = EEng_BiomeType::CoastalShallows;
    Coastal.DisplayName = FText::FromString(TEXT("Coastal Shallows"));
    Coastal.TemperatureRange = FVector2D(18.0f, 28.0f);
    Coastal.HumidityRange    = FVector2D(0.7f, 1.0f);
    Coastal.AltitudeRange    = FVector2D(-50.0f, 20.0f);
    Coastal.FoliageDensity   = 0.3f;
    Coastal.DangerLevel      = 0.5f;
    Coastal.bHasPredators    = true;
    BiomeTable.Add(Coastal);

    FEng_BiomeDefinition Mountain;
    Mountain.BiomeID = EEng_BiomeType::MountainRidge;
    Mountain.DisplayName = FText::FromString(TEXT("Mountain Ridge"));
    Mountain.TemperatureRange = FVector2D(-5.0f, 15.0f);
    Mountain.HumidityRange    = FVector2D(0.3f, 0.6f);
    Mountain.AltitudeRange    = FVector2D(600.0f, 2000.0f);
    Mountain.FoliageDensity   = 0.15f;
    Mountain.DangerLevel      = 0.55f;
    Mountain.bHasPredators    = false;
    BiomeTable.Add(Mountain);

    // Noise scale for biome sampling
    BiomeNoiseScale = 0.0005f;
    BlendRadius     = 500.0f;
}

// ============================================================
// UGameInstanceSubsystem interface
// ============================================================

void UBiomeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogTemp, Log, TEXT("[BiomeManager] Initialized — %d biomes registered"), BiomeTable.Num());
}

void UBiomeManager::Deinitialize()
{
    BiomeTable.Empty();
    Super::Deinitialize();
}

// ============================================================
// Core biome query
// ============================================================

EEng_BiomeType UBiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    // Sample noise to determine temperature and humidity at location
    const float T = SampleTemperature(WorldLocation);
    const float H = SampleHumidity(WorldLocation);
    const float A = WorldLocation.Z; // Altitude in cm (UE units)

    // Find best matching biome
    float BestScore = -1.0f;
    EEng_BiomeType BestBiome = EEng_BiomeType::OpenPlains;

    for (const FEng_BiomeDefinition& Def : BiomeTable)
    {
        float Score = ScoreBiomeMatch(Def, T, H, A);
        if (Score > BestScore)
        {
            BestScore = Score;
            BestBiome = Def.BiomeID;
        }
    }

    return BestBiome;
}

FEng_BiomeDefinition UBiomeManager::GetBiomeDefinition(EEng_BiomeType BiomeType) const
{
    for (const FEng_BiomeDefinition& Def : BiomeTable)
    {
        if (Def.BiomeID == BiomeType)
        {
            return Def;
        }
    }

    // Return default if not found
    return BiomeTable.Num() > 0 ? BiomeTable[0] : FEng_BiomeDefinition();
}

TArray<FEng_BiomeBlendWeight> UBiomeManager::GetBiomeBlendWeightsAtLocation(const FVector& WorldLocation) const
{
    TArray<FEng_BiomeBlendWeight> Result;

    const float T = SampleTemperature(WorldLocation);
    const float H = SampleHumidity(WorldLocation);
    const float A = WorldLocation.Z;

    float TotalScore = 0.0f;
    TArray<TPair<EEng_BiomeType, float>> Scores;

    for (const FEng_BiomeDefinition& Def : BiomeTable)
    {
        float Score = ScoreBiomeMatch(Def, T, H, A);
        if (Score > 0.01f)
        {
            Scores.Add(TPair<EEng_BiomeType, float>(Def.BiomeID, Score));
            TotalScore += Score;
        }
    }

    if (TotalScore <= 0.0f)
    {
        FEng_BiomeBlendWeight Default;
        Default.BiomeType = EEng_BiomeType::OpenPlains;
        Default.Weight    = 1.0f;
        Result.Add(Default);
        return Result;
    }

    // Normalize weights
    for (const TPair<EEng_BiomeType, float>& Pair : Scores)
    {
        FEng_BiomeBlendWeight BW;
        BW.BiomeType = Pair.Key;
        BW.Weight    = Pair.Value / TotalScore;
        Result.Add(BW);
    }

    // Sort descending by weight
    Result.Sort([](const FEng_BiomeBlendWeight& A, const FEng_BiomeBlendWeight& B)
    {
        return A.Weight > B.Weight;
    });

    return Result;
}

// ============================================================
// Environmental parameter queries
// ============================================================

float UBiomeManager::GetTemperatureAtLocation(const FVector& WorldLocation) const
{
    return SampleTemperature(WorldLocation);
}

float UBiomeManager::GetHumidityAtLocation(const FVector& WorldLocation) const
{
    return SampleHumidity(WorldLocation);
}

float UBiomeManager::GetDangerLevelAtLocation(const FVector& WorldLocation) const
{
    EEng_BiomeType Biome = GetBiomeAtLocation(WorldLocation);
    FEng_BiomeDefinition Def = GetBiomeDefinition(Biome);
    return Def.DangerLevel;
}

float UBiomeManager::GetFoliageDensityAtLocation(const FVector& WorldLocation) const
{
    EEng_BiomeType Biome = GetBiomeAtLocation(WorldLocation);
    FEng_BiomeDefinition Def = GetBiomeDefinition(Biome);
    return Def.FoliageDensity;
}

bool UBiomeManager::HasPredatorsAtLocation(const FVector& WorldLocation) const
{
    EEng_BiomeType Biome = GetBiomeAtLocation(WorldLocation);
    FEng_BiomeDefinition Def = GetBiomeDefinition(Biome);
    return Def.bHasPredators;
}

// ============================================================
// Debug visualization
// ============================================================

void UBiomeManager::DrawBiomeDebug(const FVector& Center, float Radius, int32 GridResolution)
{
#if ENABLE_DRAW_DEBUG
    UWorld* World = GetWorld();
    if (!World) return;

    const float Step = (Radius * 2.0f) / FMath::Max(GridResolution, 1);

    for (int32 X = 0; X < GridResolution; ++X)
    {
        for (int32 Y = 0; Y < GridResolution; ++Y)
        {
            FVector SamplePos = Center + FVector(
                -Radius + X * Step,
                -Radius + Y * Step,
                0.0f
            );

            EEng_BiomeType Biome = GetBiomeAtLocation(SamplePos);
            FColor DebugColor = GetBiomeDebugColor(Biome);

            DrawDebugPoint(World, SamplePos, 10.0f, DebugColor, false, 5.0f);
        }
    }
#endif
}

// ============================================================
// Private helpers
// ============================================================

float UBiomeManager::SampleTemperature(const FVector& WorldLocation) const
{
    // Pseudo-noise using sine waves — deterministic, no external dependency
    const float NX = WorldLocation.X * BiomeNoiseScale;
    const float NY = WorldLocation.Y * BiomeNoiseScale;
    const float AltitudeCooldown = FMath::Max(0.0f, WorldLocation.Z / 100.0f) * 0.006f; // -0.6°C per 100m

    float BaseTemp = 25.0f
        + FMath::Sin(NX * 1.3f + 0.7f) * 12.0f
        + FMath::Sin(NY * 0.9f + 1.2f) * 8.0f
        + FMath::Sin((NX + NY) * 2.1f) * 5.0f;

    return BaseTemp - AltitudeCooldown;
}

float UBiomeManager::SampleHumidity(const FVector& WorldLocation) const
{
    const float NX = WorldLocation.X * BiomeNoiseScale;
    const float NY = WorldLocation.Y * BiomeNoiseScale;

    float H = 0.5f
        + FMath::Sin(NX * 0.7f + 2.3f) * 0.25f
        + FMath::Sin(NY * 1.1f + 0.5f) * 0.2f
        + FMath::Sin((NX - NY) * 1.8f) * 0.1f;

    return FMath::Clamp(H, 0.0f, 1.0f);
}

float UBiomeManager::ScoreBiomeMatch(const FEng_BiomeDefinition& Def, float Temperature, float Humidity, float Altitude) const
{
    // Score how well the sample matches this biome's ranges
    // Returns 0 if outside range, 1 if perfectly centered

    auto RangeScore = [](float Value, FVector2D Range) -> float
    {
        if (Value < Range.X || Value > Range.Y) return 0.0f;
        const float Mid   = (Range.X + Range.Y) * 0.5f;
        const float HalfW = (Range.Y - Range.X) * 0.5f;
        if (HalfW <= 0.0f) return 1.0f;
        return 1.0f - FMath::Abs(Value - Mid) / HalfW;
    };

    const float TScore = RangeScore(Temperature, Def.TemperatureRange);
    const float HScore = RangeScore(Humidity,    Def.HumidityRange);
    const float AScore = RangeScore(Altitude,    Def.AltitudeRange);

    // All three must match — multiplicative
    return TScore * HScore * AScore;
}

FColor UBiomeManager::GetBiomeDebugColor(EEng_BiomeType BiomeType) const
{
    switch (BiomeType)
    {
        case EEng_BiomeType::TropicalForest:   return FColor(34,  139, 34);   // Forest green
        case EEng_BiomeType::OpenPlains:        return FColor(189, 183, 107);  // Khaki
        case EEng_BiomeType::Swampland:         return FColor(85,  107, 47);   // Dark olive
        case EEng_BiomeType::VolcanicBadlands:  return FColor(178, 34,  34);   // Firebrick
        case EEng_BiomeType::CoastalShallows:   return FColor(64,  164, 223);  // Sky blue
        case EEng_BiomeType::MountainRidge:     return FColor(169, 169, 169);  // Grey
        default:                                return FColor::White;
    }
}
