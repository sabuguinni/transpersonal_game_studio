// BiomeManager.cpp — Biome classification and terrain query system
// Engine Architect #02 — Transpersonal Game Studio

#include "BiomeManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UBiomeManager::UBiomeManager()
{
    // Initialize biome definitions
    InitializeBiomeDefinitions();
}

void UBiomeManager::InitializeBiomeDefinitions()
{
    BiomeDefinitions.Empty();

    // Jungle biome
    FEng_BiomeDefinition Jungle;
    Jungle.BiomeType = EEng_BiomeType::Jungle;
    Jungle.DisplayName = FText::FromString(TEXT("Jungle"));
    Jungle.TemperatureMin = 25.f;
    Jungle.TemperatureMax = 38.f;
    Jungle.HumidityMin = 0.7f;
    Jungle.HumidityMax = 1.0f;
    Jungle.ElevationMin = 0.f;
    Jungle.ElevationMax = 300.f;
    Jungle.VegetationDensity = 0.9f;
    Jungle.FoliageColor = FLinearColor(0.05f, 0.35f, 0.05f, 1.f);
    BiomeDefinitions.Add(EEng_BiomeType::Jungle, Jungle);

    // Savanna biome
    FEng_BiomeDefinition Savanna;
    Savanna.BiomeType = EEng_BiomeType::Savanna;
    Savanna.DisplayName = FText::FromString(TEXT("Savanna"));
    Savanna.TemperatureMin = 20.f;
    Savanna.TemperatureMax = 40.f;
    Savanna.HumidityMin = 0.2f;
    Savanna.HumidityMax = 0.5f;
    Savanna.ElevationMin = 0.f;
    Savanna.ElevationMax = 200.f;
    Savanna.VegetationDensity = 0.3f;
    Savanna.FoliageColor = FLinearColor(0.6f, 0.5f, 0.1f, 1.f);
    BiomeDefinitions.Add(EEng_BiomeType::Savanna, Savanna);

    // Swamp biome
    FEng_BiomeDefinition Swamp;
    Swamp.BiomeType = EEng_BiomeType::Swamp;
    Swamp.DisplayName = FText::FromString(TEXT("Swamp"));
    Swamp.TemperatureMin = 18.f;
    Swamp.TemperatureMax = 30.f;
    Swamp.HumidityMin = 0.8f;
    Swamp.HumidityMax = 1.0f;
    Swamp.ElevationMin = -10.f;
    Swamp.ElevationMax = 50.f;
    Swamp.VegetationDensity = 0.7f;
    Swamp.FoliageColor = FLinearColor(0.1f, 0.25f, 0.1f, 1.f);
    BiomeDefinitions.Add(EEng_BiomeType::Swamp, Swamp);

    // Volcanic biome
    FEng_BiomeDefinition Volcanic;
    Volcanic.BiomeType = EEng_BiomeType::Volcanic;
    Volcanic.DisplayName = FText::FromString(TEXT("Volcanic Highlands"));
    Volcanic.TemperatureMin = 30.f;
    Volcanic.TemperatureMax = 80.f;
    Volcanic.HumidityMin = 0.0f;
    Volcanic.HumidityMax = 0.2f;
    Volcanic.ElevationMin = 400.f;
    Volcanic.ElevationMax = 2000.f;
    Volcanic.VegetationDensity = 0.05f;
    Volcanic.FoliageColor = FLinearColor(0.3f, 0.1f, 0.0f, 1.f);
    BiomeDefinitions.Add(EEng_BiomeType::Volcanic, Volcanic);

    // Coastal biome
    FEng_BiomeDefinition Coastal;
    Coastal.BiomeType = EEng_BiomeType::Coastal;
    Coastal.DisplayName = FText::FromString(TEXT("Coastal"));
    Coastal.TemperatureMin = 15.f;
    Coastal.TemperatureMax = 28.f;
    Coastal.HumidityMin = 0.5f;
    Coastal.HumidityMax = 0.8f;
    Coastal.ElevationMin = -5.f;
    Coastal.ElevationMax = 80.f;
    Coastal.VegetationDensity = 0.4f;
    Coastal.FoliageColor = FLinearColor(0.2f, 0.45f, 0.15f, 1.f);
    BiomeDefinitions.Add(EEng_BiomeType::Coastal, Coastal);
}

EEng_BiomeType UBiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    float Elevation = WorldLocation.Z;
    float NormalizedX = FMath::Frac(FMath::Abs(WorldLocation.X) / 10000.f);
    float NormalizedY = FMath::Frac(FMath::Abs(WorldLocation.Y) / 10000.f);

    // Simple noise-based biome assignment using position
    float HumidityEstimate = FMath::Sin(WorldLocation.X * 0.0001f) * 0.5f + 0.5f;
    float TempEstimate = FMath::Cos(WorldLocation.Y * 0.0001f) * 0.5f + 0.5f;

    if (Elevation > 400.f)
        return EEng_BiomeType::Volcanic;
    if (Elevation < 20.f && HumidityEstimate > 0.6f)
        return EEng_BiomeType::Coastal;
    if (HumidityEstimate > 0.7f && Elevation < 300.f)
        return (Elevation < 50.f) ? EEng_BiomeType::Swamp : EEng_BiomeType::Jungle;
    if (HumidityEstimate < 0.35f)
        return EEng_BiomeType::Savanna;

    return EEng_BiomeType::Jungle;
}

FEng_BiomeDefinition UBiomeManager::GetBiomeDefinition(EEng_BiomeType BiomeType) const
{
    const FEng_BiomeDefinition* Found = BiomeDefinitions.Find(BiomeType);
    if (Found)
        return *Found;
    return FEng_BiomeDefinition();
}

float UBiomeManager::GetVegetationDensityAt(const FVector& WorldLocation) const
{
    EEng_BiomeType Biome = GetBiomeAtLocation(WorldLocation);
    return GetBiomeDefinition(Biome).VegetationDensity;
}

FLinearColor UBiomeManager::GetFoliageColorAt(const FVector& WorldLocation) const
{
    EEng_BiomeType Biome = GetBiomeAtLocation(WorldLocation);
    return GetBiomeDefinition(Biome).FoliageColor;
}

TArray<EEng_BiomeType> UBiomeManager::GetAllBiomeTypes() const
{
    TArray<EEng_BiomeType> Result;
    BiomeDefinitions.GetKeys(Result);
    return Result;
}
