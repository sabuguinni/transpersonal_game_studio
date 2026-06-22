// BiomeSystem.cpp
// Agent #05 — Procedural World Generator | PROD_CYCLE_AUTO_20260622_005
// Implements 4-zone biome system matching MinPlayableMap actor layout

#include "BiomeSystem.h"
#include "Math/UnrealMathUtility.h"

ABiomeSystem::ABiomeSystem()
{
    PrimaryActorTick.bCanEverTick = false;

    // Safe defaults — no world queries in constructor (CDO safety)
    TransitionBlendRadius = 1500.0f;
    WaterZoneRadius = 2000.0f;
    WaterSourceLocation = FVector(8000.0f, 0.0f, 50.0f);
}

void ABiomeSystem::BeginPlay()
{
    Super::BeginPlay();

    if (BiomeZones.Num() == 0)
    {
        SetupDefaultBiomes();
    }
}

void ABiomeSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ABiomeSystem::InitialiseBiomeZones()
{
    BiomeZones.Empty();
    SetupDefaultBiomes();
    UE_LOG(LogTemp, Log, TEXT("BiomeSystem: Initialised %d biome zones"), BiomeZones.Num());
}

void ABiomeSystem::SetupDefaultBiomes()
{
    // Matches MinPlayableMap BiomeZone marker positions

    // NW — Jungle (-8000, -8000)
    FWorld_BiomeZone Jungle;
    Jungle.BiomeType       = EWorld_BiomeType::Jungle;
    Jungle.CentreLocation  = FVector(-8000.0f, -8000.0f, 200.0f);
    Jungle.Radius          = 6000.0f;
    Jungle.BaseElevation   = 200.0f;
    Jungle.FoliageDensity  = 2.5f;
    Jungle.Temperature     = 32.0f;
    Jungle.Humidity        = 0.85f;
    BiomeZones.Add(Jungle);

    // NE — Plains (8000, -8000)
    FWorld_BiomeZone Plains;
    Plains.BiomeType       = EWorld_BiomeType::Plains;
    Plains.CentreLocation  = FVector(8000.0f, -8000.0f, 0.0f);
    Plains.Radius          = 6000.0f;
    Plains.BaseElevation   = 0.0f;
    Plains.FoliageDensity  = 0.6f;
    Plains.Temperature     = 28.0f;
    Plains.Humidity        = 0.35f;
    BiomeZones.Add(Plains);

    // SW — Rocky (-8000, 8000)
    FWorld_BiomeZone Rocky;
    Rocky.BiomeType        = EWorld_BiomeType::Rocky;
    Rocky.CentreLocation   = FVector(-8000.0f, 8000.0f, 400.0f);
    Rocky.Radius           = 6000.0f;
    Rocky.BaseElevation    = 400.0f;
    Rocky.FoliageDensity   = 0.3f;
    Rocky.Temperature      = 22.0f;
    Rocky.Humidity         = 0.2f;
    BiomeZones.Add(Rocky);

    // SE — Wetlands (8000, 8000)
    FWorld_BiomeZone Wetlands;
    Wetlands.BiomeType       = EWorld_BiomeType::Wetlands;
    Wetlands.CentreLocation  = FVector(8000.0f, 8000.0f, -50.0f);
    Wetlands.Radius          = 6000.0f;
    Wetlands.BaseElevation   = -50.0f;
    Wetlands.FoliageDensity  = 1.8f;
    Wetlands.Temperature     = 26.0f;
    Wetlands.Humidity        = 0.90f;
    BiomeZones.Add(Wetlands);
}

EWorld_BiomeType ABiomeSystem::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    if (BiomeZones.Num() == 0)
    {
        return EWorld_BiomeType::Plains;
    }

    float ClosestDist = TNumericLimits<float>::Max();
    EWorld_BiomeType ClosestBiome = EWorld_BiomeType::Plains;

    for (const FWorld_BiomeZone& Zone : BiomeZones)
    {
        float Dist = FVector::Dist2D(WorldLocation, Zone.CentreLocation);
        if (Dist < ClosestDist)
        {
            ClosestDist = Dist;
            ClosestBiome = Zone.BiomeType;
        }
    }

    return ClosestBiome;
}

FWorld_BiomeZone ABiomeSystem::GetBiomeZoneData(EWorld_BiomeType BiomeType) const
{
    for (const FWorld_BiomeZone& Zone : BiomeZones)
    {
        if (Zone.BiomeType == BiomeType)
        {
            return Zone;
        }
    }

    // Return default plains zone if not found
    FWorld_BiomeZone Default;
    Default.BiomeType = EWorld_BiomeType::Plains;
    return Default;
}

float ABiomeSystem::GetTemperatureAtLocation(const FVector& WorldLocation) const
{
    if (BiomeZones.Num() == 0)
    {
        return 25.0f;
    }

    // Weighted blend of nearest two biomes within transition radius
    float TotalWeight = 0.0f;
    float WeightedTemp = 0.0f;

    for (const FWorld_BiomeZone& Zone : BiomeZones)
    {
        float Dist = FVector::Dist2D(WorldLocation, Zone.CentreLocation);
        if (Dist < Zone.Radius + TransitionBlendRadius)
        {
            float Weight = FMath::Max(0.0f, 1.0f - (Dist / (Zone.Radius + TransitionBlendRadius)));
            WeightedTemp += Zone.Temperature * Weight;
            TotalWeight += Weight;
        }
    }

    if (TotalWeight > 0.0f)
    {
        return WeightedTemp / TotalWeight;
    }

    return 25.0f;
}

float ABiomeSystem::GetHumidityAtLocation(const FVector& WorldLocation) const
{
    if (BiomeZones.Num() == 0)
    {
        return 0.5f;
    }

    float TotalWeight = 0.0f;
    float WeightedHumidity = 0.0f;

    for (const FWorld_BiomeZone& Zone : BiomeZones)
    {
        float Dist = FVector::Dist2D(WorldLocation, Zone.CentreLocation);
        if (Dist < Zone.Radius + TransitionBlendRadius)
        {
            float Weight = FMath::Max(0.0f, 1.0f - (Dist / (Zone.Radius + TransitionBlendRadius)));
            WeightedHumidity += Zone.Humidity * Weight;
            TotalWeight += Weight;
        }
    }

    if (TotalWeight > 0.0f)
    {
        return FMath::Clamp(WeightedHumidity / TotalWeight, 0.0f, 1.0f);
    }

    return 0.5f;
}

bool ABiomeSystem::IsInWaterZone(const FVector& WorldLocation) const
{
    float DistToWater = FVector::Dist2D(WorldLocation, WaterSourceLocation);
    return DistToWater <= WaterZoneRadius;
}
