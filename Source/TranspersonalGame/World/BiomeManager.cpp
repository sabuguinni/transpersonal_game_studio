// BiomeManager.cpp
// Engine Architect #02 — Cycle 013
// Prehistoric survival game: Biome query system for world generation

#include "BiomeManager.h"
#include "Math/UnrealMathUtility.h"
#include "DrawDebugHelpers.h"

// ============================================================
// Constructor
// ============================================================

ABiomeManager::ABiomeManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.5f; // Update every 0.5s — not every frame

    WorldSizeKm = 4.0f;
    BiomeSeed = 42;
    BiomeNoiseScale = 0.0005f;
    BiomeGridResolution = 64;
    bDebugDrawBiomeBoundaries = false;

    InitializeDefaultBiomes();
}

// ============================================================
// BeginPlay
// ============================================================

void ABiomeManager::BeginPlay()
{
    Super::BeginPlay();

    // Pre-compute biome grid for fast spatial lookup
    BiomeGrid.SetNum(BiomeGridResolution * BiomeGridResolution);
    const float WorldSizeCm = WorldSizeKm * 100000.0f;
    const float CellSize = WorldSizeCm / BiomeGridResolution;

    for (int32 Y = 0; Y < BiomeGridResolution; ++Y)
    {
        for (int32 X = 0; X < BiomeGridResolution; ++X)
        {
            const float WorldX = (X * CellSize) - (WorldSizeCm * 0.5f);
            const float WorldY = (Y * CellSize) - (WorldSizeCm * 0.5f);
            BiomeGrid[Y * BiomeGridResolution + X] = GetBiomeAtLocation(FVector(WorldX, WorldY, 0.0f));
        }
    }

    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Grid computed (%dx%d), %d biome definitions active"),
        BiomeGridResolution, BiomeGridResolution, BiomeDefinitions.Num());
}

// ============================================================
// Tick
// ============================================================

void ABiomeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

#if WITH_EDITOR
    if (bDebugDrawBiomeBoundaries)
    {
        // Draw debug grid in editor — only in editor builds
        const UWorld* World = GetWorld();
        if (!World) return;

        const float WorldSizeCm = WorldSizeKm * 100000.0f;
        const float CellSize = WorldSizeCm / 8.0f; // Coarse debug grid

        for (int32 Y = 0; Y < 8; ++Y)
        {
            for (int32 X = 0; X < 8; ++X)
            {
                const float WX = (X * CellSize) - (WorldSizeCm * 0.5f);
                const float WY = (Y * CellSize) - (WorldSizeCm * 0.5f);
                const FVector Center(WX + CellSize * 0.5f, WY + CellSize * 0.5f, 100.0f);
                DrawDebugBox(World, Center, FVector(CellSize * 0.45f, CellSize * 0.45f, 50.0f),
                    FColor::Green, false, DeltaTime * 1.1f, 0, 10.0f);
            }
        }
    }
#endif
}

// ============================================================
// Query API
// ============================================================

EBiomeType ABiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    const float WorldSizeCm = WorldSizeKm * 100000.0f;
    const float HalfWorld = WorldSizeCm * 0.5f;

    // Normalize to [0, 1]
    const float NX = (WorldLocation.X + HalfWorld) / WorldSizeCm;
    const float NY = (WorldLocation.Y + HalfWorld) / WorldSizeCm;

    // Sample two noise octaves for biome determination
    const float NoiseVal1 = SampleBiomeNoise(WorldLocation.X * BiomeNoiseScale, WorldLocation.Y * BiomeNoiseScale);
    const float NoiseVal2 = SampleBiomeNoise(WorldLocation.X * BiomeNoiseScale * 2.3f + 100.0f,
                                              WorldLocation.Y * BiomeNoiseScale * 2.3f + 100.0f);

    // Altitude factor from Z
    const float AltitudeFactor = FMath::Clamp(WorldLocation.Z / 5000.0f, 0.0f, 1.0f);

    // Combine noise for biome selection
    const float Combined = (NoiseVal1 * 0.6f) + (NoiseVal2 * 0.3f) + (AltitudeFactor * 0.1f);

    // Map combined value to biome — 10 biomes across [0, 1]
    if (Combined < 0.08f)  return EBiomeType::River;
    if (Combined < 0.18f)  return EBiomeType::Coastal;
    if (Combined < 0.30f)  return EBiomeType::Swamp;
    if (Combined < 0.42f)  return EBiomeType::Jungle;
    if (Combined < 0.54f)  return EBiomeType::Forest;
    if (Combined < 0.64f)  return EBiomeType::Savanna;
    if (Combined < 0.73f)  return EBiomeType::Desert;
    if (Combined < 0.82f)  return EBiomeType::Mountain;
    if (Combined < 0.92f)  return EBiomeType::Volcanic;
    return EBiomeType::Mountain;
}

FBiomeData ABiomeManager::GetBiomeData(EBiomeType BiomeType) const
{
    for (const FBiomeData& Data : BiomeDefinitions)
    {
        if (Data.BiomeType == BiomeType)
        {
            return Data;
        }
    }

    // Return default if not found
    FBiomeData Default;
    Default.BiomeType = BiomeType;
    Default.DisplayName = TEXT("Unknown");
    return Default;
}

float ABiomeManager::GetTemperatureAtLocation(const FVector& WorldLocation) const
{
    const FBiomeData Data = GetBiomeData(GetBiomeAtLocation(WorldLocation));

    // Add local noise variation ±3°C
    const float Noise = SampleBiomeNoise(WorldLocation.X * 0.0001f, WorldLocation.Y * 0.0001f);
    const float BaseTemp = FMath::Lerp(Data.MinTemperatureCelsius, Data.MaxTemperatureCelsius, Noise);

    // Altitude cooling: -0.65°C per 100m
    const float AltitudeCooling = (WorldLocation.Z / 100.0f) * 0.65f;

    return FMath::Clamp(BaseTemp - AltitudeCooling, -20.0f, 60.0f);
}

float ABiomeManager::GetHumidityAtLocation(const FVector& WorldLocation) const
{
    const FBiomeData Data = GetBiomeData(GetBiomeAtLocation(WorldLocation));
    const float Noise = SampleBiomeNoise(WorldLocation.X * 0.00015f + 50.0f, WorldLocation.Y * 0.00015f + 50.0f);
    return FMath::Clamp(Data.HumidityPercent + (Noise - 0.5f) * 20.0f, 0.0f, 100.0f);
}

float ABiomeManager::GetDangerLevelAtLocation(const FVector& WorldLocation) const
{
    const FBiomeData Data = GetBiomeData(GetBiomeAtLocation(WorldLocation));
    return Data.DangerLevel;
}

bool ABiomeManager::IsInTransitionZone(const FVector& WorldLocation) const
{
    // Sample biome at location and at small offsets — if they differ, we're in a transition
    const EBiomeType Center = GetBiomeAtLocation(WorldLocation);
    const float CheckRadius = 500.0f; // 5 meters

    const EBiomeType North = GetBiomeAtLocation(WorldLocation + FVector(CheckRadius, 0, 0));
    const EBiomeType East  = GetBiomeAtLocation(WorldLocation + FVector(0, CheckRadius, 0));

    return (North != Center) || (East != Center);
}

TArray<EBiomeType> ABiomeManager::GetAllActiveBiomes() const
{
    TArray<EBiomeType> ActiveBiomes;
    for (const FBiomeData& Data : BiomeDefinitions)
    {
        ActiveBiomes.AddUnique(Data.BiomeType);
    }
    return ActiveBiomes;
}

// ============================================================
// Private helpers
// ============================================================

void ABiomeManager::InitializeDefaultBiomes()
{
    BiomeDefinitions.Empty();

    auto AddBiome = [&](EBiomeType Type, const FString& Name, EBiomeClimate Climate,
                        float TMin, float TMax, float Humidity, float VegDensity,
                        float DinoSpawn, float Danger, float Water, float Food)
    {
        FBiomeData B;
        B.BiomeType = Type;
        B.DisplayName = Name;
        B.Climate = Climate;
        B.MinTemperatureCelsius = TMin;
        B.MaxTemperatureCelsius = TMax;
        B.HumidityPercent = Humidity;
        B.VegetationDensity = VegDensity;
        B.DinosaurSpawnDensity = DinoSpawn;
        B.DangerLevel = Danger;
        B.WaterAvailability = Water;
        B.FoodAvailability = Food;
        BiomeDefinitions.Add(B);
    };

    //                  Type                  Name           Climate                  TMin  TMax  Hum  Veg  Dino  Danger Water Food
    AddBiome(EBiomeType::Jungle,   TEXT("Jungle"),   EBiomeClimate::Tropical,   25.0f, 38.0f, 90.0f, 0.95f, 0.80f, 0.85f, 0.90f, 0.85f);
    AddBiome(EBiomeType::Savanna,  TEXT("Savanna"),  EBiomeClimate::Tropical,   20.0f, 42.0f, 35.0f, 0.40f, 0.90f, 0.75f, 0.35f, 0.60f);
    AddBiome(EBiomeType::Swamp,    TEXT("Swamp"),    EBiomeClimate::Tropical,   18.0f, 32.0f, 95.0f, 0.80f, 0.70f, 0.70f, 0.95f, 0.70f);
    AddBiome(EBiomeType::Volcanic, TEXT("Volcanic"), EBiomeClimate::Volcanic,   30.0f, 65.0f, 20.0f, 0.10f, 0.30f, 0.95f, 0.15f, 0.10f);
    AddBiome(EBiomeType::Coastal,  TEXT("Coastal"),  EBiomeClimate::Coastal,    15.0f, 28.0f, 75.0f, 0.50f, 0.60f, 0.55f, 0.85f, 0.75f);
    AddBiome(EBiomeType::Forest,   TEXT("Forest"),   EBiomeClimate::Temperate,  10.0f, 25.0f, 70.0f, 0.85f, 0.65f, 0.60f, 0.70f, 0.80f);
    AddBiome(EBiomeType::Desert,   TEXT("Desert"),   EBiomeClimate::Arid,       28.0f, 55.0f, 10.0f, 0.05f, 0.20f, 0.40f, 0.05f, 0.15f);
    AddBiome(EBiomeType::Mountain, TEXT("Mountain"), EBiomeClimate::Temperate,  -5.0f, 15.0f, 50.0f, 0.30f, 0.40f, 0.50f, 0.60f, 0.40f);
    AddBiome(EBiomeType::River,    TEXT("River"),    EBiomeClimate::Tropical,   18.0f, 30.0f, 85.0f, 0.70f, 0.55f, 0.45f, 1.00f, 0.90f);
    AddBiome(EBiomeType::None,     TEXT("None"),     EBiomeClimate::Temperate,  10.0f, 20.0f, 50.0f, 0.30f, 0.30f, 0.30f, 0.50f, 0.50f);
}

float ABiomeManager::SampleBiomeNoise(float X, float Y) const
{
    // Deterministic pseudo-noise using sine combination (no external dependency)
    // Seeded by BiomeSeed for reproducible worlds
    const float SeedOffset = static_cast<float>(BiomeSeed) * 0.01f;
    const float N1 = FMath::Sin(X * 127.1f + SeedOffset) * 43758.5453f;
    const float N2 = FMath::Sin(Y * 311.7f + SeedOffset) * 43758.5453f;
    const float N3 = FMath::Sin((X + Y) * 74.3f + SeedOffset) * 43758.5453f;

    const float Raw = FMath::Frac(N1) * 0.4f + FMath::Frac(N2) * 0.4f + FMath::Frac(N3) * 0.2f;
    return FMath::Clamp(Raw, 0.0f, 1.0f);
}
