#include "BiomeManager.h"

// ============================================================
// Constructor
// ============================================================

ABiomeManager::ABiomeManager()
{
    PrimaryActorTick.bCanEverTick = false;

    // Default biome type if no region matches
    DefaultBiomeType = EBiomeType::Forest;
    bAutoSetupOnBeginPlay = true;
}

// ============================================================
// BeginPlay
// ============================================================

void ABiomeManager::BeginPlay()
{
    Super::BeginPlay();

    if (bAutoSetupOnBeginPlay && RegisteredBiomes.Num() == 0)
    {
        SetupDefaultCretaceousBiomes();
    }
}

// ============================================================
// Biome Registration
// ============================================================

void ABiomeManager::RegisterBiome(const FBiomeData& BiomeData)
{
    RegisteredBiomes.Add(BiomeData);
}

void ABiomeManager::ClearAllBiomes()
{
    RegisteredBiomes.Empty();
}

// ============================================================
// Internal Helpers
// ============================================================

int32 ABiomeManager::FindClosestBiomeIndex(const FVector& WorldLocation) const
{
    if (RegisteredBiomes.Num() == 0)
    {
        return -1;
    }

    int32 ClosestIndex = -1;
    float ClosestDistSq = FLT_MAX;

    for (int32 i = 0; i < RegisteredBiomes.Num(); ++i)
    {
        const FBiomeData& Biome = RegisteredBiomes[i];
        const float DistSq = FVector::DistSquared2D(WorldLocation, Biome.WorldCenter);
        const float RadiusSq = Biome.Radius * Biome.Radius;

        // Prefer biomes that contain the point; among those, pick closest center
        if (DistSq <= RadiusSq)
        {
            if (DistSq < ClosestDistSq)
            {
                ClosestDistSq = DistSq;
                ClosestIndex = i;
            }
        }
    }

    // If no biome contains the point, fall back to globally closest center
    if (ClosestIndex == -1)
    {
        for (int32 i = 0; i < RegisteredBiomes.Num(); ++i)
        {
            const float DistSq = FVector::DistSquared2D(WorldLocation, RegisteredBiomes[i].WorldCenter);
            if (DistSq < ClosestDistSq)
            {
                ClosestDistSq = DistSq;
                ClosestIndex = i;
            }
        }
    }

    return ClosestIndex;
}

int32 ABiomeManager::FindBiomeIndexByType(EBiomeType BiomeType) const
{
    for (int32 i = 0; i < RegisteredBiomes.Num(); ++i)
    {
        if (RegisteredBiomes[i].BiomeType == BiomeType)
        {
            return i;
        }
    }
    return -1;
}

// ============================================================
// Biome Queries
// ============================================================

EBiomeType ABiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    const int32 Index = FindClosestBiomeIndex(WorldLocation);
    if (Index == -1)
    {
        return DefaultBiomeType;
    }
    return RegisteredBiomes[Index].BiomeType;
}

FBiomeData ABiomeManager::GetBiomeDataAtLocation(const FVector& WorldLocation) const
{
    const int32 Index = FindClosestBiomeIndex(WorldLocation);
    if (Index == -1)
    {
        // Return a default biome data
        FBiomeData Default;
        Default.BiomeType = DefaultBiomeType;
        return Default;
    }
    return RegisteredBiomes[Index];
}

float ABiomeManager::GetBiomeBlendAlpha(const FVector& WorldLocation, EBiomeType BiomeA, EBiomeType BiomeB) const
{
    const int32 IndexA = FindBiomeIndexByType(BiomeA);
    const int32 IndexB = FindBiomeIndexByType(BiomeB);

    if (IndexA == -1 || IndexB == -1)
    {
        return 0.0f;
    }

    const FBiomeData& A = RegisteredBiomes[IndexA];
    const FBiomeData& B = RegisteredBiomes[IndexB];

    const float DistToA = FVector::Dist2D(WorldLocation, A.WorldCenter);
    const float DistToB = FVector::Dist2D(WorldLocation, B.WorldCenter);

    // Normalised blend: 0 = fully BiomeA, 1 = fully BiomeB
    const float Total = DistToA + DistToB;
    if (Total < KINDA_SMALL_NUMBER)
    {
        return 0.5f;
    }

    return FMath::Clamp(DistToA / Total, 0.0f, 1.0f);
}

float ABiomeManager::GetDangerLevelAtLocation(const FVector& WorldLocation) const
{
    const FBiomeData Data = GetBiomeDataAtLocation(WorldLocation);
    return Data.DangerLevel;
}

float ABiomeManager::GetTemperatureAtLocation(const FVector& WorldLocation) const
{
    const FBiomeData Data = GetBiomeDataAtLocation(WorldLocation);
    return Data.Temperature;
}

TArray<EDinosaurSpecies> ABiomeManager::GetNativeDinosaursAtLocation(const FVector& WorldLocation) const
{
    const FBiomeData Data = GetBiomeDataAtLocation(WorldLocation);
    return Data.NativeDinosaurs;
}

// ============================================================
// Weather
// ============================================================

EWeatherType ABiomeManager::GetWeatherAtLocation(const FVector& WorldLocation) const
{
    const FBiomeData Data = GetBiomeDataAtLocation(WorldLocation);
    return Data.CurrentWeather;
}

void ABiomeManager::SetWeatherForBiome(EBiomeType BiomeType, EWeatherType NewWeather)
{
    for (FBiomeData& Biome : RegisteredBiomes)
    {
        if (Biome.BiomeType == BiomeType)
        {
            Biome.CurrentWeather = NewWeather;
        }
    }
}

// ============================================================
// Default Cretaceous Biome Setup
// ============================================================

void ABiomeManager::SetupDefaultCretaceousBiomes()
{
    ClearAllBiomes();

    // --- 1. Dense Cretaceous Forest (centre of map) ---
    {
        FBiomeData Forest;
        Forest.BiomeType = EBiomeType::Forest;
        Forest.WorldCenter = FVector(0.0f, 0.0f, 0.0f);
        Forest.Radius = 8000.0f;
        Forest.Temperature = 28.0f;
        Forest.Humidity = 0.85f;
        Forest.DangerLevel = 0.5f;
        Forest.CurrentWeather = EWeatherType::Overcast;
        Forest.NativeDinosaurs.Add(EDinosaurSpecies::Velociraptor);
        Forest.NativeDinosaurs.Add(EDinosaurSpecies::Compsognathus);
        Forest.NativeDinosaurs.Add(EDinosaurSpecies::Triceratops);
        RegisterBiome(Forest);
    }

    // --- 2. Open Savanna (north-east) ---
    {
        FBiomeData Savanna;
        Savanna.BiomeType = EBiomeType::Savanna;
        Savanna.WorldCenter = FVector(12000.0f, 8000.0f, 0.0f);
        Savanna.Radius = 10000.0f;
        Savanna.Temperature = 35.0f;
        Savanna.Humidity = 0.3f;
        Savanna.DangerLevel = 0.7f;
        Savanna.CurrentWeather = EWeatherType::Clear;
        Savanna.NativeDinosaurs.Add(EDinosaurSpecies::TyrannosaurusRex);
        Savanna.NativeDinosaurs.Add(EDinosaurSpecies::Brachiosaurus);
        Savanna.NativeDinosaurs.Add(EDinosaurSpecies::Parasaurolophus);
        RegisterBiome(Savanna);
    }

    // --- 3. Swampland (south-west, near water) ---
    {
        FBiomeData Swamp;
        Swamp.BiomeType = EBiomeType::Swampland;
        Swamp.WorldCenter = FVector(-10000.0f, -6000.0f, 0.0f);
        Swamp.Radius = 7000.0f;
        Swamp.Temperature = 32.0f;
        Swamp.Humidity = 0.95f;
        Swamp.DangerLevel = 0.8f;
        Swamp.CurrentWeather = EWeatherType::Fog;
        Swamp.NativeDinosaurs.Add(EDinosaurSpecies::Allosaurus);
        Swamp.NativeDinosaurs.Add(EDinosaurSpecies::Stegosaurus);
        RegisterBiome(Swamp);
    }

    // --- 4. River Valley (east) ---
    {
        FBiomeData River;
        River.BiomeType = EBiomeType::RiverValley;
        River.WorldCenter = FVector(6000.0f, -4000.0f, 0.0f);
        River.Radius = 5000.0f;
        River.Temperature = 26.0f;
        River.Humidity = 0.75f;
        River.DangerLevel = 0.4f;
        River.CurrentWeather = EWeatherType::Clear;
        River.NativeDinosaurs.Add(EDinosaurSpecies::Parasaurolophus);
        River.NativeDinosaurs.Add(EDinosaurSpecies::Ankylosaurus);
        RegisterBiome(River);
    }

    // --- 5. Rocky Mountains (north) ---
    {
        FBiomeData Mountains;
        Mountains.BiomeType = EBiomeType::Mountains;
        Mountains.WorldCenter = FVector(-2000.0f, 14000.0f, 500.0f);
        Mountains.Radius = 9000.0f;
        Mountains.Temperature = 15.0f;
        Mountains.Humidity = 0.4f;
        Mountains.DangerLevel = 0.6f;
        Mountains.CurrentWeather = EWeatherType::Overcast;
        Mountains.NativeDinosaurs.Add(EDinosaurSpecies::Pteranodon);
        Mountains.NativeDinosaurs.Add(EDinosaurSpecies::Velociraptor);
        RegisterBiome(Mountains);
    }

    // --- 6. Coastline (south) ---
    {
        FBiomeData Coast;
        Coast.BiomeType = EBiomeType::Coastline;
        Coast.WorldCenter = FVector(0.0f, -14000.0f, 0.0f);
        Coast.Radius = 6000.0f;
        Coast.Temperature = 30.0f;
        Coast.Humidity = 0.7f;
        Coast.DangerLevel = 0.35f;
        Coast.CurrentWeather = EWeatherType::Clear;
        Coast.NativeDinosaurs.Add(EDinosaurSpecies::Pteranodon);
        Coast.NativeDinosaurs.Add(EDinosaurSpecies::Compsognathus);
        RegisterBiome(Coast);
    }

    UE_LOG(LogTemp, Log, TEXT("ABiomeManager: SetupDefaultCretaceousBiomes — %d biomes registered"), RegisteredBiomes.Num());
}
