// BiomeManager.cpp — Agent #2 Engine Architect
// Full implementation of biome classification and query system
// Prehistoric survival game — NO spiritual content

#include "BiomeManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

// ─────────────────────────────────────────────────────────────────────────────
// Static biome definitions (data-driven, no magic numbers in logic)
// ─────────────────────────────────────────────────────────────────────────────
namespace
{
    struct FBiomeStaticData
    {
        EEng_BiomeType  Type;
        FName           Name;
        float           MinTemp;   // Celsius
        float           MaxTemp;
        float           MinRain;   // mm/year
        float           MaxRain;
        float           TreeDensity;   // 0-1
        float           GrassDensity;  // 0-1
        FLinearColor    FogColor;
        float           FogDensity;
    };

    static const FBiomeStaticData GBiomeTable[] =
    {
        // Type                           Name              Temp          Rain           Trees  Grass  FogColor                         FogDen
        { EEng_BiomeType::Grassland,     "Grassland",      15.f, 28.f,   400.f,  900.f, 0.15f, 0.90f, FLinearColor(0.7f,0.8f,0.6f,1.f), 0.02f },
        { EEng_BiomeType::Forest,        "Forest",         10.f, 25.f,   900.f, 2000.f, 0.85f, 0.60f, FLinearColor(0.4f,0.6f,0.4f,1.f), 0.04f },
        { EEng_BiomeType::Desert,        "Desert",         25.f, 50.f,     0.f,  250.f, 0.02f, 0.10f, FLinearColor(0.9f,0.8f,0.6f,1.f), 0.01f },
        { EEng_BiomeType::Swamp,         "Swamp",          18.f, 32.f,  1500.f, 3000.f, 0.55f, 0.40f, FLinearColor(0.3f,0.5f,0.3f,1.f), 0.08f },
        { EEng_BiomeType::Volcanic,      "Volcanic",       30.f, 60.f,   100.f,  500.f, 0.05f, 0.05f, FLinearColor(0.6f,0.3f,0.2f,1.f), 0.10f },
        { EEng_BiomeType::CoastalPlain,  "CoastalPlain",   18.f, 30.f,   600.f, 1200.f, 0.30f, 0.75f, FLinearColor(0.6f,0.7f,0.8f,1.f), 0.03f },
    };
    static constexpr int32 GBiomeCount = UE_ARRAY_COUNT(GBiomeTable);
}

// ─────────────────────────────────────────────────────────────────────────────
// UBiomeManager — UObject lifecycle
// ─────────────────────────────────────────────────────────────────────────────

UBiomeManager::UBiomeManager()
{
    CurrentBiome    = EEng_BiomeType::Grassland;
    bBiomesLoaded   = false;
    BiomeBlendAlpha = 0.f;
    PreviousBiome   = EEng_BiomeType::Grassland;
}

void UBiomeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    LoadBiomeData();
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Initialized with %d biome definitions."), BiomeDataMap.Num());
}

void UBiomeManager::Deinitialize()
{
    BiomeDataMap.Empty();
    Super::Deinitialize();
}

// ─────────────────────────────────────────────────────────────────────────────
// Core API
// ─────────────────────────────────────────────────────────────────────────────

void UBiomeManager::LoadBiomeData()
{
    BiomeDataMap.Empty();

    for (int32 i = 0; i < GBiomeCount; ++i)
    {
        const FBiomeStaticData& Src = GBiomeTable[i];

        FEng_BiomeData Data;
        Data.BiomeType      = Src.Type;
        Data.BiomeName      = Src.Name;
        Data.MinTemperature = Src.MinTemp;
        Data.MaxTemperature = Src.MaxTemp;
        Data.MinRainfall    = Src.MinRain;
        Data.MaxRainfall    = Src.MaxRain;
        Data.TreeDensity    = Src.TreeDensity;
        Data.GrassDensity   = Src.GrassDensity;
        Data.FogColor       = Src.FogColor;
        Data.FogDensity     = Src.FogDensity;

        BiomeDataMap.Add(Src.Type, Data);
    }

    bBiomesLoaded = true;
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Loaded %d biomes."), BiomeDataMap.Num());
}

EEng_BiomeType UBiomeManager::GetBiomeAtLocation(FVector WorldLocation) const
{
    // Classify biome from world position using simple noise-based heuristic.
    // X axis → temperature gradient (south = hot, north = cold)
    // Y axis → rainfall gradient (west = dry, east = wet)
    // Z axis → elevation (high = volcanic/rocky)

    const float NormX = FMath::Clamp(WorldLocation.X / 100000.f, -1.f, 1.f); // ±100km
    const float NormY = FMath::Clamp(WorldLocation.Y / 100000.f, -1.f, 1.f);
    const float NormZ = FMath::Clamp(WorldLocation.Z / 5000.f,    0.f,  1.f); // 0-5km

    // Elevation → volcanic
    if (NormZ > 0.75f)
    {
        return EEng_BiomeType::Volcanic;
    }

    // Temperature (X) and rainfall (Y) matrix
    const float Temperature = FMath::Lerp(10.f, 50.f, (NormX + 1.f) * 0.5f);
    const float Rainfall    = FMath::Lerp(0.f, 3000.f, (NormY + 1.f) * 0.5f);

    // Coastal strip near sea level
    if (NormZ < 0.05f && Rainfall > 500.f)
    {
        return EEng_BiomeType::CoastalPlain;
    }

    if (Temperature > 30.f && Rainfall < 300.f)  return EEng_BiomeType::Desert;
    if (Temperature > 20.f && Rainfall > 1400.f)  return EEng_BiomeType::Swamp;
    if (Rainfall > 800.f)                          return EEng_BiomeType::Forest;

    return EEng_BiomeType::Grassland;
}

FEng_BiomeData UBiomeManager::GetBiomeData(EEng_BiomeType BiomeType) const
{
    if (const FEng_BiomeData* Found = BiomeDataMap.Find(BiomeType))
    {
        return *Found;
    }

    // Fallback: return grassland defaults
    FEng_BiomeData Default;
    Default.BiomeType = EEng_BiomeType::Grassland;
    Default.BiomeName = "Default";
    return Default;
}

void UBiomeManager::SetCurrentBiome(EEng_BiomeType NewBiome)
{
    if (NewBiome == CurrentBiome) return;

    PreviousBiome   = CurrentBiome;
    CurrentBiome    = NewBiome;
    BiomeBlendAlpha = 0.f;

    const FEng_BiomeData& Data = GetBiomeData(NewBiome);
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Transition %d -> %d (%s)"),
        (int32)PreviousBiome, (int32)NewBiome, *Data.BiomeName.ToString());

    OnBiomeChanged.Broadcast(PreviousBiome, NewBiome);
}

float UBiomeManager::GetTemperatureAtLocation(FVector WorldLocation) const
{
    const EEng_BiomeType Biome = GetBiomeAtLocation(WorldLocation);
    const FEng_BiomeData Data  = GetBiomeData(Biome);
    // Mid-range temperature for the biome
    return FMath::Lerp(Data.MinTemperature, Data.MaxTemperature, 0.5f);
}

float UBiomeManager::GetRainfallAtLocation(FVector WorldLocation) const
{
    const EEng_BiomeType Biome = GetBiomeAtLocation(WorldLocation);
    const FEng_BiomeData Data  = GetBiomeData(Biome);
    return FMath::Lerp(Data.MinRainfall, Data.MaxRainfall, 0.5f);
}

TArray<EEng_BiomeType> UBiomeManager::GetAllBiomeTypes() const
{
    TArray<EEng_BiomeType> Out;
    BiomeDataMap.GetKeys(Out);
    return Out;
}

bool UBiomeManager::IsBiomeHabitable(EEng_BiomeType BiomeType) const
{
    // Volcanic is not habitable for the player without special gear
    return BiomeType != EEng_BiomeType::Volcanic;
}

void UBiomeManager::UpdateBiomeBlend(float DeltaTime)
{
    if (BiomeBlendAlpha < 1.f)
    {
        constexpr float BlendSpeed = 0.5f; // full blend in 2 seconds
        BiomeBlendAlpha = FMath::Clamp(BiomeBlendAlpha + DeltaTime * BlendSpeed, 0.f, 1.f);
    }
}
