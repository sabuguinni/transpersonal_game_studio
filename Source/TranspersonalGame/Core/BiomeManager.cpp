#include "BiomeManager.h"
#include "Engine/World.h"

ABiomeManager::ABiomeManager()
{
    PrimaryActorTick.bCanEverTick = false;
    InitializeDefaultBiomes();
}

void ABiomeManager::BeginPlay()
{
    Super::BeginPlay();
    if (BiomeDefinitions.Num() == 0)
    {
        InitializeDefaultBiomes();
    }
}

void ABiomeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ABiomeManager::InitializeDefaultBiomes()
{
    BiomeDefinitions.Empty();

    // Jungle biome — dense, hot, humid
    {
        FEng_BiomeData Jungle;
        Jungle.BiomeType = EEng_BiomeType::Jungle;
        Jungle.WorldCenter = FVector(2000.0f, 2000.0f, 0.0f);
        Jungle.Radius = 6000.0f;
        Jungle.Temperature = 35.0f;
        Jungle.Humidity = 0.95f;
        Jungle.VegetationDensity = 1.0f;
        Jungle.DinosaurSpecies = { TEXT("Velociraptor"), TEXT("Parasaurolophus"), TEXT("Compsognathus") };
        BiomeDefinitions.Add(Jungle);
    }

    // Savanna biome — open plains, T-Rex territory
    {
        FEng_BiomeData Savanna;
        Savanna.BiomeType = EEng_BiomeType::Savanna;
        Savanna.WorldCenter = FVector(8000.0f, 2000.0f, 0.0f);
        Savanna.Radius = 8000.0f;
        Savanna.Temperature = 38.0f;
        Savanna.Humidity = 0.3f;
        Savanna.VegetationDensity = 0.3f;
        Savanna.DinosaurSpecies = { TEXT("TRex"), TEXT("Triceratops"), TEXT("Ankylosaurus") };
        BiomeDefinitions.Add(Savanna);
    }

    // Swamp biome — low, wet, dangerous
    {
        FEng_BiomeData Swamp;
        Swamp.BiomeType = EEng_BiomeType::Swamp;
        Swamp.WorldCenter = FVector(2000.0f, 8000.0f, -200.0f);
        Swamp.Radius = 4000.0f;
        Swamp.Temperature = 28.0f;
        Swamp.Humidity = 1.0f;
        Swamp.VegetationDensity = 0.7f;
        Swamp.DinosaurSpecies = { TEXT("Brachiosaurus"), TEXT("Spinosaurus") };
        BiomeDefinitions.Add(Swamp);
    }

    // Volcanic biome — barren, extreme heat
    {
        FEng_BiomeData Volcanic;
        Volcanic.BiomeType = EEng_BiomeType::Volcanic;
        Volcanic.WorldCenter = FVector(12000.0f, 12000.0f, 500.0f);
        Volcanic.Radius = 3000.0f;
        Volcanic.Temperature = 60.0f;
        Volcanic.Humidity = 0.1f;
        Volcanic.VegetationDensity = 0.05f;
        Volcanic.DinosaurSpecies = { TEXT("Pachycephalosaurus") };
        BiomeDefinitions.Add(Volcanic);
    }

    // Coastal Plain biome — near water, diverse fauna
    {
        FEng_BiomeData Coastal;
        Coastal.BiomeType = EEng_BiomeType::CoastalPlain;
        Coastal.WorldCenter = FVector(-4000.0f, 0.0f, 0.0f);
        Coastal.Radius = 5000.0f;
        Coastal.Temperature = 26.0f;
        Coastal.Humidity = 0.7f;
        Coastal.VegetationDensity = 0.6f;
        Coastal.DinosaurSpecies = { TEXT("Pteranodon"), TEXT("Mosasaurus") };
        BiomeDefinitions.Add(Coastal);
    }
}

EEng_BiomeType ABiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    float ClosestDist = TNumericLimits<float>::Max();
    EEng_BiomeType ClosestBiome = EEng_BiomeType::Savanna;

    for (const FEng_BiomeData& Biome : BiomeDefinitions)
    {
        float Dist = FVector::Dist2D(WorldLocation, Biome.WorldCenter);
        if (Dist < ClosestDist)
        {
            ClosestDist = Dist;
            ClosestBiome = Biome.BiomeType;
        }
    }

    return ClosestBiome;
}

FEng_BiomeData ABiomeManager::GetBiomeData(EEng_BiomeType BiomeType) const
{
    for (const FEng_BiomeData& Biome : BiomeDefinitions)
    {
        if (Biome.BiomeType == BiomeType)
        {
            return Biome;
        }
    }
    return FEng_BiomeData();
}

TArray<FName> ABiomeManager::GetDinosaurSpeciesForBiome(EEng_BiomeType BiomeType) const
{
    FEng_BiomeData Data = GetBiomeData(BiomeType);
    return Data.DinosaurSpecies;
}

float ABiomeManager::GetTemperatureAtLocation(const FVector& WorldLocation) const
{
    EEng_BiomeType Biome = GetBiomeAtLocation(WorldLocation);
    FEng_BiomeData Data = GetBiomeData(Biome);
    return Data.Temperature;
}

float ABiomeManager::GetHumidityAtLocation(const FVector& WorldLocation) const
{
    EEng_BiomeType Biome = GetBiomeAtLocation(WorldLocation);
    FEng_BiomeData Data = GetBiomeData(Biome);
    return Data.Humidity;
}

void ABiomeManager::DebugPrintAllBiomes()
{
    UE_LOG(LogTemp, Warning, TEXT("=== BiomeManager: %d biomes registered ==="), BiomeDefinitions.Num());
    for (const FEng_BiomeData& Biome : BiomeDefinitions)
    {
        UE_LOG(LogTemp, Warning, TEXT("  Biome[%d] Center=(%.0f,%.0f) R=%.0f Temp=%.1f Humid=%.2f Dinos=%d"),
            (int32)Biome.BiomeType,
            Biome.WorldCenter.X, Biome.WorldCenter.Y,
            Biome.Radius,
            Biome.Temperature,
            Biome.Humidity,
            Biome.DinosaurSpecies.Num());
    }
}
