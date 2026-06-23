// BiomeManager.cpp
// Transpersonal Game Studio — Engine Architect #02
// P1 World Generation: Biome system — noise-based classification,
// weather state machine, runtime data queries.

#include "BiomeManager.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"

// ============================================================
// Constructor
// ============================================================
ABiomeManager::ABiomeManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10 Hz — weather tick is cheap

    InitialiseDefaultWeather();
    InitialiseDefaultTransitions();
}

// ============================================================
// AActor overrides
// ============================================================
void ABiomeManager::BeginPlay()
{
    Super::BeginPlay();
    InitialiseDefaultWeather();
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: BeginPlay — WorldSize=%.0f Seed=%d"), WorldSizeXY, NoiseSeed);
}

void ABiomeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    TickWeather(DeltaTime);
}

// ============================================================
// Public API
// ============================================================
EBiomeType ABiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    const float Altitude = GetAltitudeAtLocation(WorldLocation);
    const float NoiseVal = SampleNoise(WorldLocation.X * BiomeNoiseFrequency,
                                       WorldLocation.Y * BiomeNoiseFrequency,
                                       NoiseSeed);
    return ClassifyBiome(NoiseVal, Altitude);
}

FEng_BiomeRuntimeData ABiomeManager::GetBiomeDataAtLocation(const FVector& WorldLocation) const
{
    FEng_BiomeRuntimeData Data;
    Data.BiomeType   = GetBiomeAtLocation(WorldLocation);
    Data.BlendWeight = 1.0f; // full weight at centre; transitions blend toward 0

    // Per-biome base stats
    switch (Data.BiomeType)
    {
        case EBiomeType::Grassland:
            Data.Temperature       = 28.0f;
            Data.Humidity          = 0.45f;
            Data.VegetationDensity = 0.55f;
            Data.DominantSpecies   = EDinosaurSpecies::Raptor;
            break;

        case EBiomeType::Forest:
            Data.Temperature       = 24.0f;
            Data.Humidity          = 0.75f;
            Data.VegetationDensity = 0.95f;
            Data.DominantSpecies   = EDinosaurSpecies::TRex;
            break;

        case EBiomeType::Desert:
            Data.Temperature       = 42.0f;
            Data.Humidity          = 0.10f;
            Data.VegetationDensity = 0.10f;
            Data.DominantSpecies   = EDinosaurSpecies::Raptor;
            break;

        case EBiomeType::Swamp:
            Data.Temperature       = 30.0f;
            Data.Humidity          = 0.90f;
            Data.VegetationDensity = 0.85f;
            Data.DominantSpecies   = EDinosaurSpecies::Brachiosaurus;
            break;

        case EBiomeType::Mountain:
            Data.Temperature       = 10.0f;
            Data.Humidity          = 0.30f;
            Data.VegetationDensity = 0.20f;
            Data.DominantSpecies   = EDinosaurSpecies::Raptor;
            break;

        case EBiomeType::Coastal:
            Data.Temperature       = 26.0f;
            Data.Humidity          = 0.65f;
            Data.VegetationDensity = 0.40f;
            Data.DominantSpecies   = EDinosaurSpecies::Brachiosaurus;
            break;

        default:
            Data.Temperature       = 25.0f;
            Data.Humidity          = 0.50f;
            Data.VegetationDensity = 0.50f;
            Data.DominantSpecies   = EDinosaurSpecies::Raptor;
            break;
    }

    // Apply transition blend: find nearest transition zone and reduce BlendWeight
    for (const FEng_BiomeTransition& Trans : BiomeTransitions)
    {
        if (Trans.FromBiome == Data.BiomeType || Trans.ToBiome == Data.BiomeType)
        {
            // Simple distance-based blend (placeholder — full implementation uses
            // Voronoi cell distance to biome boundary)
            const float HalfWorld = WorldSizeXY * 0.5f;
            const float DistEdge  = FMath::Min(
                FMath::Abs(WorldLocation.X) - HalfWorld,
                FMath::Abs(WorldLocation.Y) - HalfWorld);
            if (DistEdge < Trans.TransitionWidth)
            {
                const float t = FMath::Clamp(DistEdge / Trans.TransitionWidth, 0.0f, 1.0f);
                Data.BlendWeight = FMath::Pow(t, Trans.BlendExponent);
            }
        }
    }

    return Data;
}

EWeatherType ABiomeManager::GetWeatherAtLocation(const FVector& WorldLocation) const
{
    const EBiomeType Biome = GetBiomeAtLocation(WorldLocation);
    const EWeatherType* Found = CurrentWeatherMap.Find(Biome);
    return Found ? *Found : EWeatherType::Clear;
}

void ABiomeManager::SetWeatherOverride(EBiomeType Biome, EWeatherType Weather, float DurationSeconds)
{
    CurrentWeatherMap.Add(Biome, Weather);
    WeatherOverrideTimers.Add(Biome, DurationSeconds);
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Weather override set — Biome=%d Weather=%d Duration=%.1fs"),
           (int32)Biome, (int32)Weather, DurationSeconds);
}

bool ABiomeManager::AreSameBiome(const FVector& LocationA, const FVector& LocationB) const
{
    return GetBiomeAtLocation(LocationA) == GetBiomeAtLocation(LocationB);
}

void ABiomeManager::DrawBiomeBoundaries(float Duration)
{
    UWorld* World = GetWorld();
    if (!World) return;

    const int32 GridSteps = 20;
    const float Step = WorldSizeXY / GridSteps;
    const float HalfSize = WorldSizeXY * 0.5f;

    for (int32 ix = 0; ix < GridSteps; ++ix)
    {
        for (int32 iy = 0; iy < GridSteps; ++iy)
        {
            const float X = -HalfSize + ix * Step + Step * 0.5f;
            const float Y = -HalfSize + iy * Step + Step * 0.5f;
            const FVector Centre(X, Y, 100.0f);

            const EBiomeType Biome = GetBiomeAtLocation(Centre);

            FColor DebugColor = FColor::White;
            switch (Biome)
            {
                case EBiomeType::Grassland: DebugColor = FColor::Green;  break;
                case EBiomeType::Forest:    DebugColor = FColor(0, 100, 0); break;
                case EBiomeType::Desert:    DebugColor = FColor::Yellow; break;
                case EBiomeType::Swamp:     DebugColor = FColor(100, 80, 0); break;
                case EBiomeType::Mountain:  DebugColor = FColor::Silver; break;
                case EBiomeType::Coastal:   DebugColor = FColor::Cyan;   break;
                default: break;
            }

            DrawDebugBox(World, Centre, FVector(Step * 0.45f, Step * 0.45f, 50.0f),
                         DebugColor, false, Duration, 0, 30.0f);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Drew %d biome cells (Duration=%.1fs)"), GridSteps * GridSteps, Duration);
}

// ============================================================
// Private helpers
// ============================================================

/**
 * Fast value noise — deterministic, no external dependencies.
 * Returns [0..1].
 */
float ABiomeManager::SampleNoise(float X, float Y, int32 Seed) const
{
    // Hash-based smooth noise (2D)
    const int32 IX = FMath::FloorToInt(X);
    const int32 IY = FMath::FloorToInt(Y);
    const float FX = X - IX;
    const float FY = Y - IY;

    // Smoothstep
    const float UX = FX * FX * (3.0f - 2.0f * FX);
    const float UY = FY * FY * (3.0f - 2.0f * FY);

    auto Hash = [&](int32 Cx, int32 Cy) -> float
    {
        int32 H = Cx * 374761393 + Cy * 668265263 + Seed * 2246822519;
        H = (H ^ (H >> 13)) * 1274126177;
        H = H ^ (H >> 16);
        return (float)(H & 0xFFFF) / 65535.0f;
    };

    const float V00 = Hash(IX,   IY);
    const float V10 = Hash(IX+1, IY);
    const float V01 = Hash(IX,   IY+1);
    const float V11 = Hash(IX+1, IY+1);

    return FMath::Lerp(FMath::Lerp(V00, V10, UX),
                       FMath::Lerp(V01, V11, UX), UY);
}

EBiomeType ABiomeManager::ClassifyBiome(float NoiseValue, float Altitude) const
{
    // Altitude overrides first
    if (Altitude > 2500.0f) return EBiomeType::Mountain;
    if (Altitude < -50.0f)  return EBiomeType::Coastal;

    // Noise bands — 6 biomes across [0..1]
    if (NoiseValue < 0.15f) return EBiomeType::Desert;
    if (NoiseValue < 0.30f) return EBiomeType::Grassland;
    if (NoiseValue < 0.50f) return EBiomeType::Forest;
    if (NoiseValue < 0.65f) return EBiomeType::Grassland;
    if (NoiseValue < 0.80f) return EBiomeType::Swamp;
    if (NoiseValue < 0.90f) return EBiomeType::Forest;
    return EBiomeType::Coastal;
}

float ABiomeManager::GetAltitudeAtLocation(const FVector& WorldLocation) const
{
    // Returns Z component as altitude proxy.
    // In production this would sample the landscape heightmap.
    return WorldLocation.Z;
}

void ABiomeManager::TickWeather(float DeltaTime)
{
    WeatherAccumulator += DeltaTime;
    if (WeatherAccumulator < WeatherTickInterval) return;
    WeatherAccumulator = 0.0f;

    // Decrement override timers
    TArray<EBiomeType> Expired;
    for (auto& Pair : WeatherOverrideTimers)
    {
        Pair.Value -= WeatherTickInterval;
        if (Pair.Value <= 0.0f) Expired.Add(Pair.Key);
    }
    for (EBiomeType B : Expired)
    {
        WeatherOverrideTimers.Remove(B);
        // Revert to biome-default weather
        switch (B)
        {
            case EBiomeType::Forest: CurrentWeatherMap.Add(B, EWeatherType::Cloudy); break;
            case EBiomeType::Swamp:  CurrentWeatherMap.Add(B, EWeatherType::Rainy);  break;
            default:                 CurrentWeatherMap.Add(B, EWeatherType::Clear);  break;
        }
        UE_LOG(LogTemp, Log, TEXT("BiomeManager: Weather override expired for biome %d"), (int32)B);
    }
}

void ABiomeManager::InitialiseDefaultWeather()
{
    CurrentWeatherMap.Add(EBiomeType::Grassland, EWeatherType::Clear);
    CurrentWeatherMap.Add(EBiomeType::Forest,    EWeatherType::Cloudy);
    CurrentWeatherMap.Add(EBiomeType::Desert,    EWeatherType::Clear);
    CurrentWeatherMap.Add(EBiomeType::Swamp,     EWeatherType::Rainy);
    CurrentWeatherMap.Add(EBiomeType::Mountain,  EWeatherType::Stormy);
    CurrentWeatherMap.Add(EBiomeType::Coastal,   EWeatherType::Clear);
}

void ABiomeManager::InitialiseDefaultTransitions()
{
    if (BiomeTransitions.Num() > 0) return; // Already set from editor

    FEng_BiomeTransition T;

    T.FromBiome = EBiomeType::Grassland; T.ToBiome = EBiomeType::Forest;
    T.TransitionWidth = 800.0f;  T.BlendExponent = 1.5f;
    BiomeTransitions.Add(T);

    T.FromBiome = EBiomeType::Desert;    T.ToBiome = EBiomeType::Grassland;
    T.TransitionWidth = 1200.0f; T.BlendExponent = 2.0f;
    BiomeTransitions.Add(T);

    T.FromBiome = EBiomeType::Forest;    T.ToBiome = EBiomeType::Swamp;
    T.TransitionWidth = 600.0f;  T.BlendExponent = 1.2f;
    BiomeTransitions.Add(T);

    T.FromBiome = EBiomeType::Mountain;  T.ToBiome = EBiomeType::Grassland;
    T.TransitionWidth = 2000.0f; T.BlendExponent = 1.8f;
    BiomeTransitions.Add(T);

    T.FromBiome = EBiomeType::Coastal;   T.ToBiome = EBiomeType::Swamp;
    T.TransitionWidth = 500.0f;  T.BlendExponent = 1.0f;
    BiomeTransitions.Add(T);
}
