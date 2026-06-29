// BiomeManager.cpp
// P1 World Generation — Biome classification and environment parameter system
// Engine Architect #02 — Transpersonal Game Studio

#include "BiomeManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

// ============================================================
// Constructor
// ============================================================
ABiomeManager::ABiomeManager()
{
    PrimaryActorTick.bCanEverTick = false;

    // Sensible defaults for a Cretaceous world
    WorldSizeCm = 400000.0f;
    BiomeGridResolution = 64;
    BiomeNoiseScale = 0.0001f;
    CoastalHeightThreshold = 500.0f;
    VolcanicHeightThreshold = 8000.0f;

    InitDefaultBiomeData();
}

// ============================================================
// BeginPlay
// ============================================================
void ABiomeManager::BeginPlay()
{
    Super::BeginPlay();
    RefreshBiomeMap();
}

// ============================================================
// OnConstruction — rebuild biome grid when actor is placed/moved
// ============================================================
void ABiomeManager::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    InitDefaultBiomeData();
    RefreshBiomeMap();
}

// ============================================================
// InitDefaultBiomeData — hard-coded Cretaceous biome parameters
// ============================================================
void ABiomeManager::InitDefaultBiomeData()
{
    DefaultBiomeData.Empty();

    // Jungle
    FEng_BiomeData Jungle;
    Jungle.BiomeType = EEng_BiomeType::Jungle;
    Jungle.BiomeName = TEXT("Cretaceous Jungle");
    Jungle.FogColor = FLinearColor(0.4f, 0.7f, 0.3f, 1.0f);
    Jungle.FogDensity = 0.04f;
    Jungle.AmbientTemperature = 32.0f;
    Jungle.Humidity = 0.95f;
    Jungle.VegetationDensity = 1.0f;
    Jungle.DinosaurSpawnWeight = 1.2f;
    DefaultBiomeData.Add(EEng_BiomeType::Jungle, Jungle);

    // Savanna
    FEng_BiomeData Savanna;
    Savanna.BiomeType = EEng_BiomeType::Savanna;
    Savanna.BiomeName = TEXT("Cretaceous Savanna");
    Savanna.FogColor = FLinearColor(0.9f, 0.8f, 0.5f, 1.0f);
    Savanna.FogDensity = 0.01f;
    Savanna.AmbientTemperature = 38.0f;
    Savanna.Humidity = 0.3f;
    Savanna.VegetationDensity = 0.4f;
    Savanna.DinosaurSpawnWeight = 1.5f;
    DefaultBiomeData.Add(EEng_BiomeType::Savanna, Savanna);

    // Swamp
    FEng_BiomeData Swamp;
    Swamp.BiomeType = EEng_BiomeType::Swamp;
    Swamp.BiomeName = TEXT("Cretaceous Swamp");
    Swamp.FogColor = FLinearColor(0.3f, 0.5f, 0.2f, 1.0f);
    Swamp.FogDensity = 0.08f;
    Swamp.AmbientTemperature = 30.0f;
    Swamp.Humidity = 1.0f;
    Swamp.VegetationDensity = 0.9f;
    Swamp.DinosaurSpawnWeight = 0.8f;
    DefaultBiomeData.Add(EEng_BiomeType::Swamp, Swamp);

    // Volcanic
    FEng_BiomeData Volcanic;
    Volcanic.BiomeType = EEng_BiomeType::Volcanic;
    Volcanic.BiomeName = TEXT("Volcanic Highlands");
    Volcanic.FogColor = FLinearColor(0.8f, 0.4f, 0.1f, 1.0f);
    Volcanic.FogDensity = 0.06f;
    Volcanic.AmbientTemperature = 55.0f;
    Volcanic.Humidity = 0.1f;
    Volcanic.VegetationDensity = 0.1f;
    Volcanic.DinosaurSpawnWeight = 0.3f;
    DefaultBiomeData.Add(EEng_BiomeType::Volcanic, Volcanic);

    // Coastal
    FEng_BiomeData Coastal;
    Coastal.BiomeType = EEng_BiomeType::Coastal;
    Coastal.BiomeName = TEXT("Coastal Shallows");
    Coastal.FogColor = FLinearColor(0.5f, 0.8f, 0.9f, 1.0f);
    Coastal.FogDensity = 0.03f;
    Coastal.AmbientTemperature = 27.0f;
    Coastal.Humidity = 0.85f;
    Coastal.VegetationDensity = 0.5f;
    Coastal.DinosaurSpawnWeight = 0.6f;
    DefaultBiomeData.Add(EEng_BiomeType::Coastal, Coastal);

    // Forest
    FEng_BiomeData Forest;
    Forest.BiomeType = EEng_BiomeType::Forest;
    Forest.BiomeName = TEXT("Conifer Forest");
    Forest.FogColor = FLinearColor(0.5f, 0.7f, 0.5f, 1.0f);
    Forest.FogDensity = 0.025f;
    Forest.AmbientTemperature = 22.0f;
    Forest.Humidity = 0.75f;
    Forest.VegetationDensity = 0.85f;
    Forest.DinosaurSpawnWeight = 1.0f;
    DefaultBiomeData.Add(EEng_BiomeType::Forest, Forest);

    // Plains
    FEng_BiomeData Plains;
    Plains.BiomeType = EEng_BiomeType::Plains;
    Plains.BiomeName = TEXT("Open Plains");
    Plains.FogColor = FLinearColor(0.7f, 0.85f, 0.7f, 1.0f);
    Plains.FogDensity = 0.015f;
    Plains.AmbientTemperature = 30.0f;
    Plains.Humidity = 0.5f;
    Plains.VegetationDensity = 0.55f;
    Plains.DinosaurSpawnWeight = 1.8f;
    DefaultBiomeData.Add(EEng_BiomeType::Plains, Plains);

    // Unknown fallback
    FEng_BiomeData Unknown;
    Unknown.BiomeType = EEng_BiomeType::Unknown;
    Unknown.BiomeName = TEXT("Unknown");
    DefaultBiomeData.Add(EEng_BiomeType::Unknown, Unknown);
}

// ============================================================
// RefreshBiomeMap — rebuild the flat biome grid
// ============================================================
void ABiomeManager::RefreshBiomeMap()
{
    const int32 TotalCells = BiomeGridResolution * BiomeGridResolution;
    BiomeGrid.SetNumUninitialized(TotalCells);

    for (int32 Y = 0; Y < BiomeGridResolution; ++Y)
    {
        for (int32 X = 0; X < BiomeGridResolution; ++X)
        {
            const float U = static_cast<float>(X) / static_cast<float>(BiomeGridResolution - 1);
            const float V = static_cast<float>(Y) / static_cast<float>(BiomeGridResolution - 1);
            BiomeGrid[Y * BiomeGridResolution + X] = SampleBiomeAtUV(U, V);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("BiomeManager: RefreshBiomeMap complete — %d cells at resolution %d"),
        TotalCells, BiomeGridResolution);
}

// ============================================================
// GetBiomeAtLocation
// ============================================================
EEng_BiomeType ABiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    if (BiomeGrid.Num() == 0)
    {
        return EEng_BiomeType::Unknown;
    }

    // Height-based overrides
    if (WorldLocation.Z < CoastalHeightThreshold)
    {
        return EEng_BiomeType::Coastal;
    }
    if (WorldLocation.Z > VolcanicHeightThreshold)
    {
        return EEng_BiomeType::Volcanic;
    }

    // Map world XY to UV
    const float HalfWorld = WorldSizeCm * 0.5f;
    const float U = FMath::Clamp((WorldLocation.X + HalfWorld) / WorldSizeCm, 0.0f, 1.0f);
    const float V = FMath::Clamp((WorldLocation.Y + HalfWorld) / WorldSizeCm, 0.0f, 1.0f);

    return SampleBiomeAtUV(U, V);
}

// ============================================================
// GetBiomeData
// ============================================================
FEng_BiomeData ABiomeManager::GetBiomeData(EEng_BiomeType BiomeType) const
{
    // Check DataTable first
    if (BiomeDataTable)
    {
        const FString RowName = UEnum::GetValueAsString(BiomeType);
        const FEng_BiomeData* Row = BiomeDataTable->FindRow<FEng_BiomeData>(FName(*RowName), TEXT("GetBiomeData"));
        if (Row)
        {
            return *Row;
        }
    }

    // Fallback to default data
    if (const FEng_BiomeData* Data = DefaultBiomeData.Find(BiomeType))
    {
        return *Data;
    }

    return FEng_BiomeData();
}

// ============================================================
// GetDominantBiomeInRadius — samples 9 points and returns majority
// ============================================================
EEng_BiomeType ABiomeManager::GetDominantBiomeInRadius(const FVector& Center, float Radius) const
{
    TMap<EEng_BiomeType, int32> Counts;

    const float Step = Radius * 0.5f;
    for (int32 DX = -1; DX <= 1; ++DX)
    {
        for (int32 DY = -1; DY <= 1; ++DY)
        {
            const FVector SamplePos = Center + FVector(DX * Step, DY * Step, 0.0f);
            const EEng_BiomeType B = GetBiomeAtLocation(SamplePos);
            Counts.FindOrAdd(B)++;
        }
    }

    EEng_BiomeType Dominant = EEng_BiomeType::Unknown;
    int32 MaxCount = 0;
    for (const auto& Pair : Counts)
    {
        if (Pair.Value > MaxCount)
        {
            MaxCount = Pair.Value;
            Dominant = Pair.Key;
        }
    }
    return Dominant;
}

// ============================================================
// SampleBiomeAtUV — deterministic biome from UV + noise
// ============================================================
EEng_BiomeType ABiomeManager::SampleBiomeAtUV(float U, float V) const
{
    // Combine UV with noise to get a biome index
    const float Noise = SimpleNoise(U * 100.0f, V * 100.0f);
    const float Combined = (U + V + Noise) * 0.5f; // 0..1 range

    // Distribute biomes across the combined value
    if (Combined < 0.12f) return EEng_BiomeType::Coastal;
    if (Combined < 0.25f) return EEng_BiomeType::Swamp;
    if (Combined < 0.40f) return EEng_BiomeType::Jungle;
    if (Combined < 0.55f) return EEng_BiomeType::Forest;
    if (Combined < 0.68f) return EEng_BiomeType::Plains;
    if (Combined < 0.80f) return EEng_BiomeType::Savanna;
    if (Combined < 0.92f) return EEng_BiomeType::Forest;
    return EEng_BiomeType::Volcanic;
}

// ============================================================
// SimpleNoise — cheap deterministic noise (no external deps)
// ============================================================
float ABiomeManager::SimpleNoise(float X, float Y) const
{
    // Integer hash-based noise
    int32 IX = FMath::FloorToInt(X);
    int32 IY = FMath::FloorToInt(Y);
    float FX = X - IX;
    float FY = Y - IY;

    // Smooth step
    FX = FX * FX * (3.0f - 2.0f * FX);
    FY = FY * FY * (3.0f - 2.0f * FY);

    auto Hash = [](int32 A, int32 B) -> float
    {
        int32 H = A * 374761393 + B * 668265263;
        H = (H ^ (H >> 13)) * 1274126177;
        H = H ^ (H >> 16);
        return static_cast<float>(H & 0xFFFF) / 65535.0f;
    };

    const float A = Hash(IX,     IY);
    const float B = Hash(IX + 1, IY);
    const float C = Hash(IX,     IY + 1);
    const float D = Hash(IX + 1, IY + 1);

    return FMath::Lerp(FMath::Lerp(A, B, FX), FMath::Lerp(C, D, FX), FY);
}
