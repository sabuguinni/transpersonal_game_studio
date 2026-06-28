// BiomeZoneManager.cpp
// Transpersonal Game Studio — Agent #5 Procedural World Generator
// Implementation of biome zone queries and default zone initialization

#include "BiomeZoneManager.h"

ABiomeZoneManager::ABiomeZoneManager()
{
    PrimaryActorTick.bCanEverTick = false;
    // Initialize with 4 default biome zones matching MinPlayableMap layout
    InitializeDefaultBiomes();
}

void ABiomeZoneManager::BeginPlay()
{
    Super::BeginPlay();
    if (BiomeZones.Num() == 0)
    {
        InitializeDefaultBiomes();
    }
}

void ABiomeZoneManager::InitializeDefaultBiomes()
{
    BiomeZones.Empty();

    // Plains biome — west of origin
    FWorld_BiomeZone Plains;
    Plains.BiomeType = EWorld_BiomeType::Plains;
    Plains.CenterLocation = FVector(-2000.0f, 0.0f, 0.0f);
    Plains.Radius = 4000.0f;
    Plains.FoliageDensity = 0.4f;
    Plains.WaterLevel = -200.0f;
    Plains.RainProbability = 0.2f;
    Plains.AmbientTint = FLinearColor(1.0f, 0.95f, 0.7f, 1.0f);
    BiomeZones.Add(Plains);

    // Forest biome — north of origin
    FWorld_BiomeZone Forest;
    Forest.BiomeType = EWorld_BiomeType::Forest;
    Forest.CenterLocation = FVector(0.0f, 2500.0f, 0.0f);
    Forest.Radius = 3500.0f;
    Forest.FoliageDensity = 2.0f;
    Forest.WaterLevel = -100.0f;
    Forest.RainProbability = 0.6f;
    Forest.AmbientTint = FLinearColor(0.6f, 0.9f, 0.5f, 1.0f);
    BiomeZones.Add(Forest);

    // Rocky biome — south-east
    FWorld_BiomeZone Rocky;
    Rocky.BiomeType = EWorld_BiomeType::Rocky;
    Rocky.CenterLocation = FVector(3000.0f, -2000.0f, 100.0f);
    Rocky.Radius = 3000.0f;
    Rocky.FoliageDensity = 0.2f;
    Rocky.WaterLevel = -300.0f;
    Rocky.RainProbability = 0.1f;
    Rocky.AmbientTint = FLinearColor(0.85f, 0.75f, 0.6f, 1.0f);
    BiomeZones.Add(Rocky);

    // River Delta biome — east of origin
    FWorld_BiomeZone River;
    River.BiomeType = EWorld_BiomeType::RiverDelta;
    River.CenterLocation = FVector(2000.0f, 0.0f, -50.0f);
    River.Radius = 3000.0f;
    River.FoliageDensity = 1.5f;
    River.WaterLevel = -50.0f;
    River.RainProbability = 0.7f;
    River.AmbientTint = FLinearColor(0.5f, 0.7f, 1.0f, 1.0f);
    BiomeZones.Add(River);
}

EWorld_BiomeType ABiomeZoneManager::GetBiomeAtLocation(FVector WorldLocation) const
{
    float ClosestDist = TNumericLimits<float>::Max();
    EWorld_BiomeType Result = EWorld_BiomeType::Unknown;

    for (const FWorld_BiomeZone& Zone : BiomeZones)
    {
        float Dist = FVector::Dist2D(WorldLocation, Zone.CenterLocation);
        if (Dist <= Zone.Radius && Dist < ClosestDist)
        {
            ClosestDist = Dist;
            Result = Zone.BiomeType;
        }
    }
    return Result;
}

bool ABiomeZoneManager::GetBiomeZoneAtLocation(FVector WorldLocation, FWorld_BiomeZone& OutZone) const
{
    float ClosestDist = TNumericLimits<float>::Max();
    bool bFound = false;

    for (const FWorld_BiomeZone& Zone : BiomeZones)
    {
        float Dist = FVector::Dist2D(WorldLocation, Zone.CenterLocation);
        if (Dist <= Zone.Radius && Dist < ClosestDist)
        {
            ClosestDist = Dist;
            OutZone = Zone;
            bFound = true;
        }
    }
    return bFound;
}

float ABiomeZoneManager::GetFoliageDensityAtLocation(FVector WorldLocation) const
{
    FWorld_BiomeZone Zone;
    if (GetBiomeZoneAtLocation(WorldLocation, Zone))
    {
        return Zone.FoliageDensity;
    }
    return 1.0f; // default density
}

float ABiomeZoneManager::GetRainProbabilityAtLocation(FVector WorldLocation) const
{
    FWorld_BiomeZone Zone;
    if (GetBiomeZoneAtLocation(WorldLocation, Zone))
    {
        return Zone.RainProbability;
    }
    return 0.3f; // default rain probability
}

int32 ABiomeZoneManager::GetBiomeCount() const
{
    return BiomeZones.Num();
}
