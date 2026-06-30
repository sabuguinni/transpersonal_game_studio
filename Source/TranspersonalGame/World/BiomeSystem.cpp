#include "BiomeSystem.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Math/UnrealMathUtility.h"

ABiomeSystem::ABiomeSystem()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ABiomeSystem::BeginPlay()
{
    Super::BeginPlay();
    if (bAutoInitializeOnBeginPlay)
    {
        InitializeBiomes();
    }
}

void ABiomeSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ABiomeSystem::InitializeBiomes()
{
    if (Biomes.Num() > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("BiomeSystem: %d biomes already configured, skipping default init."), Biomes.Num());
        return;
    }

    // Forest biome — northwest quadrant
    FWorld_BiomeData Forest;
    Forest.BiomeType = EWorld_BiomeType::Forest;
    Forest.WorldCenter = FVector2D(-4000.0f, -4000.0f);
    Forest.Radius = 6000.0f;
    Forest.FoliageDensity = 1.8f;
    Forest.Temperature = 22.0f;
    Forest.Humidity = 0.75f;
    Forest.FogColor = FLinearColor(0.4f, 0.7f, 0.4f, 1.0f);
    Forest.FogDensity = 0.03f;
    Biomes.Add(Forest);

    // Open Plains — center
    FWorld_BiomeData Plains;
    Plains.BiomeType = EWorld_BiomeType::OpenPlains;
    Plains.WorldCenter = FVector2D(0.0f, 0.0f);
    Plains.Radius = 7000.0f;
    Plains.FoliageDensity = 0.4f;
    Plains.Temperature = 30.0f;
    Plains.Humidity = 0.3f;
    Plains.FogColor = FLinearColor(0.8f, 0.85f, 0.7f, 1.0f);
    Plains.FogDensity = 0.01f;
    Biomes.Add(Plains);

    // Rocky Highlands — northeast
    FWorld_BiomeData Rocky;
    Rocky.BiomeType = EWorld_BiomeType::RockyHighlands;
    Rocky.WorldCenter = FVector2D(5000.0f, -3000.0f);
    Rocky.Radius = 4000.0f;
    Rocky.FoliageDensity = 0.2f;
    Rocky.Temperature = 18.0f;
    Rocky.Humidity = 0.2f;
    Rocky.FogColor = FLinearColor(0.7f, 0.7f, 0.65f, 1.0f);
    Rocky.FogDensity = 0.015f;
    Biomes.Add(Rocky);

    // River Delta — south
    FWorld_BiomeData River;
    River.BiomeType = EWorld_BiomeType::RiverDelta;
    River.WorldCenter = FVector2D(0.0f, 6000.0f);
    River.Radius = 3500.0f;
    River.FoliageDensity = 1.2f;
    River.Temperature = 28.0f;
    River.Humidity = 0.9f;
    River.FogColor = FLinearColor(0.5f, 0.75f, 0.8f, 1.0f);
    River.FogDensity = 0.04f;
    Biomes.Add(River);

    // Volcanic Zone — far east
    FWorld_BiomeData Volcanic;
    Volcanic.BiomeType = EWorld_BiomeType::VolcanicZone;
    Volcanic.WorldCenter = FVector2D(8000.0f, 2000.0f);
    Volcanic.Radius = 3000.0f;
    Volcanic.FoliageDensity = 0.1f;
    Volcanic.Temperature = 45.0f;
    Volcanic.Humidity = 0.15f;
    Volcanic.FogColor = FLinearColor(0.8f, 0.5f, 0.3f, 1.0f);
    Volcanic.FogDensity = 0.05f;
    Biomes.Add(Volcanic);

    // Swampland — southwest
    FWorld_BiomeData Swamp;
    Swamp.BiomeType = EWorld_BiomeType::Swampland;
    Swamp.WorldCenter = FVector2D(-5000.0f, 5000.0f);
    Swamp.Radius = 4500.0f;
    Swamp.FoliageDensity = 1.5f;
    Swamp.Temperature = 32.0f;
    Swamp.Humidity = 0.95f;
    Swamp.FogColor = FLinearColor(0.45f, 0.6f, 0.45f, 1.0f);
    Swamp.FogDensity = 0.06f;
    Biomes.Add(Swamp);

    UE_LOG(LogTemp, Log, TEXT("BiomeSystem: Initialized %d default biomes."), Biomes.Num());
}

EWorld_BiomeType ABiomeSystem::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    EWorld_BiomeType Dominant = EWorld_BiomeType::OpenPlains;
    float BestWeight = 0.0f;

    for (const FWorld_BiomeData& Biome : Biomes)
    {
        float Weight = GetBlendWeight(WorldLocation, Biome);
        if (Weight > BestWeight)
        {
            BestWeight = Weight;
            Dominant = Biome.BiomeType;
        }
    }

    return Dominant;
}

FWorld_BiomeData ABiomeSystem::GetBiomeData(EWorld_BiomeType BiomeType) const
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

float ABiomeSystem::GetBlendWeight(const FVector& Location, const FWorld_BiomeData& Biome) const
{
    FVector2D Loc2D(Location.X, Location.Y);
    float Distance = FVector2D::Distance(Loc2D, Biome.WorldCenter);

    if (Distance >= Biome.Radius)
    {
        return 0.0f;
    }

    // Smooth falloff using cosine interpolation
    float NormalizedDist = Distance / Biome.Radius;
    float Weight = FMath::Cos(NormalizedDist * PI * 0.5f);
    return FMath::Clamp(Weight, 0.0f, 1.0f);
}

void ABiomeSystem::SpawnBiomeVegetation(const FWorld_BiomeData& BiomeData, int32 Count)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("BiomeSystem: SpawnBiomeVegetation called without valid world."));
        return;
    }

    // Vegetation spawning is handled by FoliageManager in coordination with this system.
    // This stub logs intent and delegates to the foliage pipeline.
    UE_LOG(LogTemp, Log, TEXT("BiomeSystem: SpawnBiomeVegetation requested for biome type %d, count %d."),
        (int32)BiomeData.BiomeType, Count);
}
