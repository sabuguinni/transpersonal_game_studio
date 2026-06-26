// PCGBiomeSystem.cpp
// Agent #05 — Procedural World Generator
// Biome system for Transpersonal Game Studio dinosaur survival game

#include "PCGBiomeSystem.h"
#include "Math/UnrealMathUtility.h"

APCGBiomeSystem::APCGBiomeSystem()
{
    PrimaryActorTick.bCanEverTick = false;
    bAutoGenerateOnBeginPlay = true;
    WorldRadius = 20000.0f;
}

void APCGBiomeSystem::BeginPlay()
{
    Super::BeginPlay();

    if (Biomes.Num() == 0)
    {
        InitializeDefaultBiomes();
    }

    if (bAutoGenerateOnBeginPlay)
    {
        GenerateBiomeLayout();
    }
}

void APCGBiomeSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void APCGBiomeSystem::InitializeDefaultBiomes()
{
    // Jungle biome — center of map, high humidity
    FWorld_BiomeData Jungle;
    Jungle.BiomeType = EWorld_BiomeType::Jungle;
    Jungle.Center = FVector(2000.0f, 2000.0f, 0.0f);
    Jungle.Radius = 6000.0f;
    Jungle.VegetationDensity = 1.0f;
    Jungle.Temperature = 32.0f;
    Jungle.Humidity = 0.95f;
    Biomes.Add(Jungle);

    // Savanna biome — east side, moderate vegetation
    FWorld_BiomeData Savanna;
    Savanna.BiomeType = EWorld_BiomeType::Savanna;
    Savanna.Center = FVector(6000.0f, 2000.0f, 0.0f);
    Savanna.Radius = 5000.0f;
    Savanna.VegetationDensity = 0.4f;
    Savanna.Temperature = 35.0f;
    Savanna.Humidity = 0.3f;
    Biomes.Add(Savanna);

    // Badlands biome — west side, rocky terrain
    FWorld_BiomeData Badlands;
    Badlands.BiomeType = EWorld_BiomeType::Badlands;
    Badlands.Center = FVector(-2000.0f, 1000.0f, 0.0f);
    Badlands.Radius = 5000.0f;
    Badlands.VegetationDensity = 0.1f;
    Badlands.Temperature = 40.0f;
    Badlands.Humidity = 0.05f;
    Biomes.Add(Badlands);

    // Wetlands biome — south, water features
    FWorld_BiomeData Wetlands;
    Wetlands.BiomeType = EWorld_BiomeType::Wetlands;
    Wetlands.Center = FVector(500.0f, 4000.0f, 0.0f);
    Wetlands.Radius = 4000.0f;
    Wetlands.VegetationDensity = 0.8f;
    Wetlands.Temperature = 28.0f;
    Wetlands.Humidity = 1.0f;
    Biomes.Add(Wetlands);

    // Volcanic biome — north, sparse, dangerous
    FWorld_BiomeData Volcanic;
    Volcanic.BiomeType = EWorld_BiomeType::Volcanic;
    Volcanic.Center = FVector(2000.0f, -4000.0f, 0.0f);
    Volcanic.Radius = 4000.0f;
    Volcanic.VegetationDensity = 0.05f;
    Volcanic.Temperature = 55.0f;
    Volcanic.Humidity = 0.1f;
    Biomes.Add(Volcanic);
}

EWorld_BiomeType APCGBiomeSystem::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    float BestWeight = -1.0f;
    EWorld_BiomeType BestBiome = EWorld_BiomeType::Savanna;

    for (const FWorld_BiomeData& Biome : Biomes)
    {
        float Weight = CalculateBiomeBlend(WorldLocation, Biome);
        if (Weight > BestWeight)
        {
            BestWeight = Weight;
            BestBiome = Biome.BiomeType;
        }
    }

    return BestBiome;
}

FWorld_BiomeData APCGBiomeSystem::GetBiomeData(EWorld_BiomeType BiomeType) const
{
    for (const FWorld_BiomeData& Biome : Biomes)
    {
        if (Biome.BiomeType == BiomeType)
        {
            return Biome;
        }
    }

    // Return default if not found
    FWorld_BiomeData Default;
    Default.BiomeType = BiomeType;
    return Default;
}

void APCGBiomeSystem::RegisterBiome(const FWorld_BiomeData& BiomeData)
{
    // Check if biome of this type already exists — replace it
    for (int32 i = 0; i < Biomes.Num(); ++i)
    {
        if (Biomes[i].BiomeType == BiomeData.BiomeType)
        {
            Biomes[i] = BiomeData;
            return;
        }
    }
    Biomes.Add(BiomeData);
}

void APCGBiomeSystem::GenerateBiomeLayout()
{
    if (Biomes.Num() == 0)
    {
        InitializeDefaultBiomes();
    }

    UE_LOG(LogTemp, Log, TEXT("PCGBiomeSystem: Generated layout with %d biomes"), Biomes.Num());
}

float APCGBiomeSystem::CalculateBiomeBlend(const FVector& Location, const FWorld_BiomeData& Biome) const
{
    float Distance = FVector::Dist2D(Location, Biome.Center);
    if (Distance >= Biome.Radius)
    {
        return 0.0f;
    }

    // Smooth falloff from center to edge
    float NormalizedDist = Distance / Biome.Radius;
    return 1.0f - (NormalizedDist * NormalizedDist);
}
