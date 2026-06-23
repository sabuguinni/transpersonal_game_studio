// BiomeManager.cpp
// Core Systems Programmer — Agent #3
// P1 World Generation: Biome classification and terrain parameter management

#include "BiomeManager.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UBiomeManager::UBiomeManager()
{
    PrimaryComponentTick.bCanEverTick = false;

    // Default biome definitions — prehistoric world
    FCore_BiomeDef Jungle;
    Jungle.BiomeID = ECore_BiomeType::Jungle;
    Jungle.DisplayName = FName("Jungle");
    Jungle.BaseTemperature = 32.0f;
    Jungle.Humidity = 0.9f;
    Jungle.FoliageDensity = 1.0f;
    Jungle.HeightMin = 0.0f;
    Jungle.HeightMax = 400.0f;
    Jungle.bAllowsDinosaurs = true;
    BiomeDefs.Add(ECore_BiomeType::Jungle, Jungle);

    FCore_BiomeDef Plains;
    Plains.BiomeID = ECore_BiomeType::Plains;
    Plains.DisplayName = FName("Plains");
    Plains.BaseTemperature = 24.0f;
    Plains.Humidity = 0.4f;
    Plains.FoliageDensity = 0.3f;
    Plains.HeightMin = 0.0f;
    Plains.HeightMax = 200.0f;
    Plains.bAllowsDinosaurs = true;
    BiomeDefs.Add(ECore_BiomeType::Plains, Plains);

    FCore_BiomeDef Swamp;
    Swamp.BiomeID = ECore_BiomeType::Swamp;
    Swamp.DisplayName = FName("Swamp");
    Swamp.BaseTemperature = 28.0f;
    Swamp.Humidity = 0.95f;
    Swamp.FoliageDensity = 0.7f;
    Swamp.HeightMin = -50.0f;
    Swamp.HeightMax = 50.0f;
    Swamp.bAllowsDinosaurs = true;
    BiomeDefs.Add(ECore_BiomeType::Swamp, Swamp);

    FCore_BiomeDef Volcanic;
    Volcanic.BiomeID = ECore_BiomeType::Volcanic;
    Volcanic.DisplayName = FName("Volcanic");
    Volcanic.BaseTemperature = 55.0f;
    Volcanic.Humidity = 0.1f;
    Volcanic.FoliageDensity = 0.05f;
    Volcanic.HeightMin = 200.0f;
    Volcanic.HeightMax = 1200.0f;
    Volcanic.bAllowsDinosaurs = false;
    BiomeDefs.Add(ECore_BiomeType::Volcanic, Volcanic);

    FCore_BiomeDef Coastal;
    Coastal.BiomeID = ECore_BiomeType::Coastal;
    Coastal.DisplayName = FName("Coastal");
    Coastal.BaseTemperature = 22.0f;
    Coastal.Humidity = 0.7f;
    Coastal.FoliageDensity = 0.4f;
    Coastal.HeightMin = -10.0f;
    Coastal.HeightMax = 80.0f;
    Coastal.bAllowsDinosaurs = true;
    BiomeDefs.Add(ECore_BiomeType::Coastal, Coastal);

    FCore_BiomeDef Forest;
    Forest.BiomeID = ECore_BiomeType::Forest;
    Forest.DisplayName = FName("Forest");
    Forest.BaseTemperature = 18.0f;
    Forest.Humidity = 0.6f;
    Forest.FoliageDensity = 0.85f;
    Forest.HeightMin = 50.0f;
    Forest.HeightMax = 600.0f;
    Forest.bAllowsDinosaurs = true;
    BiomeDefs.Add(ECore_BiomeType::Forest, Forest);
}

void UBiomeManager::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Initialized with %d biome definitions"), BiomeDefs.Num());
}

ECore_BiomeType UBiomeManager::GetBiomeAtLocation(FVector WorldLocation) const
{
    // Classify biome by height and noise-based humidity approximation
    const float Height = WorldLocation.Z;
    const float X = WorldLocation.X;
    const float Y = WorldLocation.Y;

    // Simple deterministic classification using position hash
    // In full implementation this would sample a biome noise map
    const float NoiseVal = FMath::Abs(FMath::Sin(X * 0.0003f) * FMath::Cos(Y * 0.0003f));

    if (Height > 500.0f)
    {
        return ECore_BiomeType::Volcanic;
    }
    if (Height < 20.0f && NoiseVal > 0.7f)
    {
        return ECore_BiomeType::Swamp;
    }
    if (Height < 30.0f)
    {
        return ECore_BiomeType::Coastal;
    }
    if (NoiseVal > 0.6f)
    {
        return ECore_BiomeType::Jungle;
    }
    if (NoiseVal > 0.35f)
    {
        return ECore_BiomeType::Forest;
    }
    return ECore_BiomeType::Plains;
}

FCore_BiomeDef UBiomeManager::GetBiomeDef(ECore_BiomeType BiomeType) const
{
    if (const FCore_BiomeDef* Found = BiomeDefs.Find(BiomeType))
    {
        return *Found;
    }
    // Return default Plains if not found
    FCore_BiomeDef Default;
    Default.BiomeID = ECore_BiomeType::Plains;
    Default.DisplayName = FName("Unknown");
    Default.BaseTemperature = 20.0f;
    Default.Humidity = 0.5f;
    Default.FoliageDensity = 0.3f;
    return Default;
}

float UBiomeManager::GetTemperatureAtLocation(FVector WorldLocation) const
{
    const ECore_BiomeType Biome = GetBiomeAtLocation(WorldLocation);
    const FCore_BiomeDef Def = GetBiomeDef(Biome);

    // Add altitude cooling: -6.5°C per 1000m
    const float AltitudeCooling = FMath::Max(0.0f, WorldLocation.Z / 100.0f) * 0.65f;
    return Def.BaseTemperature - AltitudeCooling;
}

float UBiomeManager::GetHumidityAtLocation(FVector WorldLocation) const
{
    const ECore_BiomeType Biome = GetBiomeAtLocation(WorldLocation);
    const FCore_BiomeDef Def = GetBiomeDef(Biome);
    return Def.Humidity;
}

bool UBiomeManager::IsDinosaurHabitatAt(FVector WorldLocation) const
{
    const ECore_BiomeType Biome = GetBiomeAtLocation(WorldLocation);
    const FCore_BiomeDef Def = GetBiomeDef(Biome);
    return Def.bAllowsDinosaurs;
}

TArray<ECore_BiomeType> UBiomeManager::GetAllBiomeTypes() const
{
    TArray<ECore_BiomeType> Keys;
    BiomeDefs.GetKeys(Keys);
    return Keys;
}
