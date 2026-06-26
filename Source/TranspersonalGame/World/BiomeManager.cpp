// BiomeManager.cpp
// Engine Architect #02 — Transpersonal Game Studio
// Manages biome classification, terrain queries, and biome-driven
// foliage/fauna spawning for the procedural prehistoric world.

#include "BiomeManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UBiomeManager::UBiomeManager()
{
    // Default biome thresholds (altitude-based)
    BiomeAltitudeThresholds.Add(EBiomeType::CoastalShallows,    -200.0f);
    BiomeAltitudeThresholds.Add(EBiomeType::TropicalJungle,      200.0f);
    BiomeAltitudeThresholds.Add(EBiomeType::Savanna,             600.0f);
    BiomeAltitudeThresholds.Add(EBiomeType::TemperateForest,    1200.0f);
    BiomeAltitudeThresholds.Add(EBiomeType::AlpineMeadow,       2000.0f);
    BiomeAltitudeThresholds.Add(EBiomeType::VolcanicBadlands,   3000.0f);

    // Default biome temperatures (Celsius)
    BiomeTemperatures.Add(EBiomeType::CoastalShallows,    28.0f);
    BiomeTemperatures.Add(EBiomeType::TropicalJungle,     35.0f);
    BiomeTemperatures.Add(EBiomeType::Savanna,            32.0f);
    BiomeTemperatures.Add(EBiomeType::TemperateForest,    18.0f);
    BiomeTemperatures.Add(EBiomeType::AlpineMeadow,        8.0f);
    BiomeTemperatures.Add(EBiomeType::VolcanicBadlands,   55.0f);

    // Default biome humidity (0-1)
    BiomeHumidity.Add(EBiomeType::CoastalShallows,    0.9f);
    BiomeHumidity.Add(EBiomeType::TropicalJungle,     0.95f);
    BiomeHumidity.Add(EBiomeType::Savanna,            0.3f);
    BiomeHumidity.Add(EBiomeType::TemperateForest,    0.6f);
    BiomeHumidity.Add(EBiomeType::AlpineMeadow,       0.5f);
    BiomeHumidity.Add(EBiomeType::VolcanicBadlands,   0.1f);

    // Foliage density per biome (0-1)
    BiomeFoliageDensity.Add(EBiomeType::CoastalShallows,    0.4f);
    BiomeFoliageDensity.Add(EBiomeType::TropicalJungle,     1.0f);
    BiomeFoliageDensity.Add(EBiomeType::Savanna,            0.3f);
    BiomeFoliageDensity.Add(EBiomeType::TemperateForest,    0.7f);
    BiomeFoliageDensity.Add(EBiomeType::AlpineMeadow,       0.2f);
    BiomeFoliageDensity.Add(EBiomeType::VolcanicBadlands,   0.05f);
}

void UBiomeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogTemp, Log, TEXT("[BiomeManager] Initialized — %d biomes registered"), BiomeAltitudeThresholds.Num());
}

void UBiomeManager::Deinitialize()
{
    BiomeAltitudeThresholds.Empty();
    BiomeTemperatures.Empty();
    BiomeHumidity.Empty();
    BiomeFoliageDensity.Empty();
    Super::Deinitialize();
}

EBiomeType UBiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    float Altitude = WorldLocation.Z;

    // Walk thresholds from lowest to highest
    EBiomeType Result = EBiomeType::TropicalJungle; // default

    // Coastal if below sea level
    if (Altitude < 0.0f) return EBiomeType::CoastalShallows;

    // Altitude-based classification
    if (Altitude < 200.0f)  return EBiomeType::TropicalJungle;
    if (Altitude < 600.0f)  return EBiomeType::Savanna;
    if (Altitude < 1200.0f) return EBiomeType::TemperateForest;
    if (Altitude < 2000.0f) return EBiomeType::AlpineMeadow;

    return EBiomeType::VolcanicBadlands;
}

FBiomeData UBiomeManager::GetBiomeData(EBiomeType BiomeType) const
{
    FBiomeData Data;
    Data.BiomeType = BiomeType;

    if (const float* Temp = BiomeTemperatures.Find(BiomeType))
        Data.Temperature = *Temp;
    else
        Data.Temperature = 25.0f;

    if (const float* Hum = BiomeHumidity.Find(BiomeType))
        Data.Humidity = *Hum;
    else
        Data.Humidity = 0.5f;

    if (const float* Density = BiomeFoliageDensity.Find(BiomeType))
        Data.FoliageDensity = *Density;
    else
        Data.FoliageDensity = 0.5f;

    // Biome display names
    switch (BiomeType)
    {
        case EBiomeType::CoastalShallows:  Data.BiomeName = TEXT("Coastal Shallows");  break;
        case EBiomeType::TropicalJungle:   Data.BiomeName = TEXT("Tropical Jungle");   break;
        case EBiomeType::Savanna:          Data.BiomeName = TEXT("Cretaceous Savanna"); break;
        case EBiomeType::TemperateForest:  Data.BiomeName = TEXT("Temperate Forest");  break;
        case EBiomeType::AlpineMeadow:     Data.BiomeName = TEXT("Alpine Meadow");     break;
        case EBiomeType::VolcanicBadlands: Data.BiomeName = TEXT("Volcanic Badlands"); break;
        default:                           Data.BiomeName = TEXT("Unknown Biome");     break;
    }

    return Data;
}

float UBiomeManager::GetTemperatureAtLocation(const FVector& WorldLocation) const
{
    EBiomeType Biome = GetBiomeAtLocation(WorldLocation);
    if (const float* Temp = BiomeTemperatures.Find(Biome))
        return *Temp;
    return 25.0f;
}

float UBiomeManager::GetHumidityAtLocation(const FVector& WorldLocation) const
{
    EBiomeType Biome = GetBiomeAtLocation(WorldLocation);
    if (const float* Hum = BiomeHumidity.Find(Biome))
        return *Hum;
    return 0.5f;
}

float UBiomeManager::GetFoliageDensityAtLocation(const FVector& WorldLocation) const
{
    EBiomeType Biome = GetBiomeAtLocation(WorldLocation);
    if (const float* Density = BiomeFoliageDensity.Find(Biome))
        return *Density;
    return 0.5f;
}

bool UBiomeManager::IsBiomeSuitableForSpecies(EBiomeType Biome, const FName& SpeciesName) const
{
    // Carnivores (T-Rex, Raptor) prefer jungle/savanna
    if (SpeciesName == TEXT("TRex") || SpeciesName == TEXT("Velociraptor"))
    {
        return (Biome == EBiomeType::TropicalJungle || Biome == EBiomeType::Savanna);
    }

    // Herbivores (Brachiosaurus, Triceratops) prefer jungle/savanna/temperate
    if (SpeciesName == TEXT("Brachiosaurus") || SpeciesName == TEXT("Triceratops")
        || SpeciesName == TEXT("Parasaurolophus"))
    {
        return (Biome == EBiomeType::TropicalJungle
             || Biome == EBiomeType::Savanna
             || Biome == EBiomeType::TemperateForest);
    }

    // Default: most biomes are suitable
    return (Biome != EBiomeType::VolcanicBadlands);
}

TArray<EBiomeType> UBiomeManager::GetAllBiomeTypes() const
{
    TArray<EBiomeType> AllBiomes;
    BiomeAltitudeThresholds.GetKeys(AllBiomes);
    return AllBiomes;
}

void UBiomeManager::SetBiomeTemperature(EBiomeType BiomeType, float Temperature)
{
    BiomeTemperatures.Add(BiomeType, Temperature);
}

void UBiomeManager::SetBiomeHumidity(EBiomeType BiomeType, float Humidity)
{
    BiomeHumidity.Add(BiomeType, FMath::Clamp(Humidity, 0.0f, 1.0f));
}

void UBiomeManager::SetBiomeFoliageDensity(EBiomeType BiomeType, float Density)
{
    BiomeFoliageDensity.Add(BiomeType, FMath::Clamp(Density, 0.0f, 1.0f));
}
