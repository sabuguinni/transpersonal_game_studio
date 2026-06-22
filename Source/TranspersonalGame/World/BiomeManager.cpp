// BiomeManager.cpp
// Engine Architect #02 — Transpersonal Game Studio
// Biome system implementation — P1 World Generation priority

#include "BiomeManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"

UBiomeManager::UBiomeManager()
{
    // Default biome configuration
    ActiveBiomeType = EBiomeType::Grassland;
    TransitionBlendRadius = 500.0f;
    bBiomeSystemInitialized = false;
}

void UBiomeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    InitializeBiomeData();
    bBiomeSystemInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Initialized with %d biome definitions"), BiomeDataMap.Num());
}

void UBiomeManager::Deinitialize()
{
    BiomeDataMap.Empty();
    ActiveBiomes.Empty();
    Super::Deinitialize();
}

void UBiomeManager::InitializeBiomeData()
{
    // Grassland biome
    FEng_BiomeData Grassland;
    Grassland.BiomeType = EBiomeType::Grassland;
    Grassland.DisplayName = FText::FromString(TEXT("Prehistoric Grassland"));
    Grassland.TemperatureMin = 18.0f;
    Grassland.TemperatureMax = 32.0f;
    Grassland.Humidity = 0.45f;
    Grassland.FoliageDensity = 0.6f;
    Grassland.bAllowsDinosaurSpawn = true;
    Grassland.DinosaurSpawnWeight = 1.0f;
    BiomeDataMap.Add(EBiomeType::Grassland, Grassland);

    // Forest biome
    FEng_BiomeData Forest;
    Forest.BiomeType = EBiomeType::Forest;
    Forest.DisplayName = FText::FromString(TEXT("Ancient Forest"));
    Forest.TemperatureMin = 14.0f;
    Forest.TemperatureMax = 26.0f;
    Forest.Humidity = 0.75f;
    Forest.FoliageDensity = 0.95f;
    Forest.bAllowsDinosaurSpawn = true;
    Forest.DinosaurSpawnWeight = 0.8f;
    BiomeDataMap.Add(EBiomeType::Forest, Forest);

    // Desert biome
    FEng_BiomeData Desert;
    Desert.BiomeType = EBiomeType::Desert;
    Desert.DisplayName = FText::FromString(TEXT("Arid Badlands"));
    Desert.TemperatureMin = 28.0f;
    Desert.TemperatureMax = 52.0f;
    Desert.Humidity = 0.08f;
    Desert.FoliageDensity = 0.1f;
    Desert.bAllowsDinosaurSpawn = true;
    Desert.DinosaurSpawnWeight = 0.5f;
    BiomeDataMap.Add(EBiomeType::Desert, Desert);

    // Swamp biome
    FEng_BiomeData Swamp;
    Swamp.BiomeType = EBiomeType::Swamp;
    Swamp.DisplayName = FText::FromString(TEXT("Primordial Swamp"));
    Swamp.TemperatureMin = 22.0f;
    Swamp.TemperatureMax = 36.0f;
    Swamp.Humidity = 0.92f;
    Swamp.FoliageDensity = 0.85f;
    Swamp.bAllowsDinosaurSpawn = true;
    Swamp.DinosaurSpawnWeight = 1.2f;
    BiomeDataMap.Add(EBiomeType::Swamp, Swamp);

    // Volcanic biome
    FEng_BiomeData Volcanic;
    Volcanic.BiomeType = EBiomeType::Volcanic;
    Volcanic.DisplayName = FText::FromString(TEXT("Volcanic Highlands"));
    Volcanic.TemperatureMin = 40.0f;
    Volcanic.TemperatureMax = 80.0f;
    Volcanic.Humidity = 0.15f;
    Volcanic.FoliageDensity = 0.05f;
    Volcanic.bAllowsDinosaurSpawn = false;
    Volcanic.DinosaurSpawnWeight = 0.0f;
    BiomeDataMap.Add(EBiomeType::Volcanic, Volcanic);

    // Tundra biome
    FEng_BiomeData Tundra;
    Tundra.BiomeType = EBiomeType::Tundra;
    Tundra.DisplayName = FText::FromString(TEXT("Frozen Tundra"));
    Tundra.TemperatureMin = -20.0f;
    Tundra.TemperatureMax = 5.0f;
    Tundra.Humidity = 0.3f;
    Tundra.FoliageDensity = 0.15f;
    Tundra.bAllowsDinosaurSpawn = true;
    Tundra.DinosaurSpawnWeight = 0.4f;
    BiomeDataMap.Add(EBiomeType::Tundra, Tundra);

    UE_LOG(LogTemp, Log, TEXT("BiomeManager: %d biomes initialized"), BiomeDataMap.Num());
}

EBiomeType UBiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    if (!bBiomeSystemInitialized)
    {
        return EBiomeType::Grassland;
    }

    // Simplified biome lookup using world coordinates
    // In full implementation this queries the PCG biome map texture
    // For MinPlayableMap: use distance-based zones from origin
    float DistFromOrigin = FVector2D(WorldLocation.X, WorldLocation.Y).Size();
    float Angle = FMath::Atan2(WorldLocation.Y, WorldLocation.X);

    if (DistFromOrigin < 1000.0f)
    {
        return EBiomeType::Grassland;
    }
    else if (DistFromOrigin < 2500.0f)
    {
        // Quadrant-based biome assignment
        if (Angle > 0.0f && Angle < HALF_PI)
            return EBiomeType::Forest;
        else if (Angle >= HALF_PI || Angle < -HALF_PI)
            return EBiomeType::Swamp;
        else if (Angle >= -HALF_PI && Angle < 0.0f)
            return EBiomeType::Desert;
        else
            return EBiomeType::Grassland;
    }
    else if (DistFromOrigin < 5000.0f)
    {
        return EBiomeType::Tundra;
    }
    else
    {
        return EBiomeType::Volcanic;
    }
}

bool UBiomeManager::GetBiomeData(EBiomeType BiomeType, FEng_BiomeData& OutBiomeData) const
{
    if (const FEng_BiomeData* Found = BiomeDataMap.Find(BiomeType))
    {
        OutBiomeData = *Found;
        return true;
    }
    return false;
}

float UBiomeManager::GetTemperatureAtLocation(const FVector& WorldLocation) const
{
    EBiomeType Biome = GetBiomeAtLocation(WorldLocation);
    FEng_BiomeData Data;
    if (GetBiomeData(Biome, Data))
    {
        // Interpolate based on altitude
        float AltitudeFactor = FMath::Clamp(WorldLocation.Z / 5000.0f, 0.0f, 1.0f);
        float BaseTemp = FMath::Lerp(Data.TemperatureMax, Data.TemperatureMin, AltitudeFactor);
        return BaseTemp;
    }
    return 20.0f; // Default temperature
}

float UBiomeManager::GetHumidityAtLocation(const FVector& WorldLocation) const
{
    EBiomeType Biome = GetBiomeAtLocation(WorldLocation);
    FEng_BiomeData Data;
    if (GetBiomeData(Biome, Data))
    {
        return Data.Humidity;
    }
    return 0.5f;
}

bool UBiomeManager::CanDinosaursSpawnAtLocation(const FVector& WorldLocation) const
{
    EBiomeType Biome = GetBiomeAtLocation(WorldLocation);
    FEng_BiomeData Data;
    if (GetBiomeData(Biome, Data))
    {
        return Data.bAllowsDinosaurSpawn;
    }
    return false;
}

float UBiomeManager::GetDinosaurSpawnWeightAtLocation(const FVector& WorldLocation) const
{
    EBiomeType Biome = GetBiomeAtLocation(WorldLocation);
    FEng_BiomeData Data;
    if (GetBiomeData(Biome, Data))
    {
        return Data.DinosaurSpawnWeight;
    }
    return 0.0f;
}

TArray<EBiomeType> UBiomeManager::GetActiveBiomes() const
{
    return ActiveBiomes;
}

void UBiomeManager::RegisterActiveBiome(EBiomeType BiomeType)
{
    if (!ActiveBiomes.Contains(BiomeType))
    {
        ActiveBiomes.Add(BiomeType);
        UE_LOG(LogTemp, Log, TEXT("BiomeManager: Registered active biome %d"), (int32)BiomeType);
    }
}
