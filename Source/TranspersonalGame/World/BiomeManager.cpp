// Copyright Transpersonal Game Studio. All Rights Reserved.
#include "BiomeManager.h"
#include "Math/UnrealMathUtility.h"

ABiomeManager::ABiomeManager()
{
    PrimaryActorTick.bCanEverTick = false;
    bAutoInitializeOnBeginPlay = true;
    BlendRadius = 500.0f;
}

void ABiomeManager::BeginPlay()
{
    Super::BeginPlay();
    if (bAutoInitializeOnBeginPlay && BiomeZones.Num() == 0)
    {
        InitializeDefaultBiomes();
    }
}

void ABiomeManager::InitializeDefaultBiomes()
{
    BiomeZones.Empty();

    // Forest — northwest, dense, humid, moderate danger
    FWorld_BiomeZone Forest;
    Forest.Center = FVector(-3000.0f, -3000.0f, 0.0f);
    Forest.Radius = 3500.0f;
    Forest.BiomeData.BiomeType = EWorld_BiomeType::Forest;
    Forest.BiomeData.BiomeName = TEXT("Ancient Forest");
    Forest.BiomeData.FogDensity = 0.04f;
    Forest.BiomeData.FogHeightFalloff = 0.15f;
    Forest.BiomeData.FogInscatteringColor = FLinearColor(0.3f, 0.5f, 0.3f, 1.0f);
    Forest.BiomeData.AmbientTemperature = 18.0f;
    Forest.BiomeData.HumidityFactor = 0.85f;
    Forest.BiomeData.DangerLevel = 0.6f;
    BiomeZones.Add(Forest);

    // Plains — northeast, open, dry, lower danger
    FWorld_BiomeZone Plains;
    Plains.Center = FVector(3000.0f, -3000.0f, 0.0f);
    Plains.Radius = 3500.0f;
    Plains.BiomeData.BiomeType = EWorld_BiomeType::Plains;
    Plains.BiomeData.BiomeName = TEXT("Open Plains");
    Plains.BiomeData.FogDensity = 0.01f;
    Plains.BiomeData.FogHeightFalloff = 0.3f;
    Plains.BiomeData.FogInscatteringColor = FLinearColor(0.8f, 0.75f, 0.5f, 1.0f);
    Plains.BiomeData.AmbientTemperature = 28.0f;
    Plains.BiomeData.HumidityFactor = 0.25f;
    Plains.BiomeData.DangerLevel = 0.4f;
    BiomeZones.Add(Plains);

    // Rocky Highlands — southwest, cold, dry, high danger
    FWorld_BiomeZone Rocky;
    Rocky.Center = FVector(-3000.0f, 3000.0f, 0.0f);
    Rocky.Radius = 3000.0f;
    Rocky.BiomeData.BiomeType = EWorld_BiomeType::Rocky;
    Rocky.BiomeData.BiomeName = TEXT("Rocky Highlands");
    Rocky.BiomeData.FogDensity = 0.015f;
    Rocky.BiomeData.FogHeightFalloff = 0.25f;
    Rocky.BiomeData.FogInscatteringColor = FLinearColor(0.6f, 0.6f, 0.65f, 1.0f);
    Rocky.BiomeData.AmbientTemperature = 10.0f;
    Rocky.BiomeData.HumidityFactor = 0.2f;
    Rocky.BiomeData.DangerLevel = 0.75f;
    BiomeZones.Add(Rocky);

    // Swamp — southeast, very humid, warm, high danger
    FWorld_BiomeZone Swamp;
    Swamp.Center = FVector(3000.0f, 3000.0f, 0.0f);
    Swamp.Radius = 3000.0f;
    Swamp.BiomeData.BiomeType = EWorld_BiomeType::Swamp;
    Swamp.BiomeData.BiomeName = TEXT("Primordial Swamp");
    Swamp.BiomeData.FogDensity = 0.07f;
    Swamp.BiomeData.FogHeightFalloff = 0.1f;
    Swamp.BiomeData.FogInscatteringColor = FLinearColor(0.2f, 0.35f, 0.2f, 1.0f);
    Swamp.BiomeData.AmbientTemperature = 32.0f;
    Swamp.BiomeData.HumidityFactor = 0.95f;
    Swamp.BiomeData.DangerLevel = 0.8f;
    BiomeZones.Add(Swamp);

    // Volcanic — center, extreme heat, very high danger
    FWorld_BiomeZone Volcanic;
    Volcanic.Center = FVector(0.0f, 0.0f, 0.0f);
    Volcanic.Radius = 2500.0f;
    Volcanic.BiomeData.BiomeType = EWorld_BiomeType::Volcanic;
    Volcanic.BiomeData.BiomeName = TEXT("Volcanic Crater");
    Volcanic.BiomeData.FogDensity = 0.05f;
    Volcanic.BiomeData.FogHeightFalloff = 0.2f;
    Volcanic.BiomeData.FogInscatteringColor = FLinearColor(0.7f, 0.3f, 0.1f, 1.0f);
    Volcanic.BiomeData.AmbientTemperature = 55.0f;
    Volcanic.BiomeData.HumidityFactor = 0.1f;
    Volcanic.BiomeData.DangerLevel = 1.0f;
    BiomeZones.Add(Volcanic);

    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Initialized %d default biome zones"), BiomeZones.Num());
}

void ABiomeManager::RegisterBiomeZone(const FWorld_BiomeZone& Zone)
{
    BiomeZones.Add(Zone);
}

FWorld_BiomeData ABiomeManager::GetBiomeDataAtLocation(const FVector& WorldLocation) const
{
    if (BiomeZones.Num() == 0)
    {
        FWorld_BiomeData Default;
        Default.BiomeType = EWorld_BiomeType::Unknown;
        Default.BiomeName = TEXT("Wilderness");
        return Default;
    }
    return BlendBiomeData(WorldLocation);
}

EWorld_BiomeType ABiomeManager::GetBiomeTypeAtLocation(const FVector& WorldLocation) const
{
    return GetBiomeDataAtLocation(WorldLocation).BiomeType;
}

float ABiomeManager::GetFogDensityAtLocation(const FVector& WorldLocation) const
{
    return GetBiomeDataAtLocation(WorldLocation).FogDensity;
}

float ABiomeManager::GetTemperatureAtLocation(const FVector& WorldLocation) const
{
    return GetBiomeDataAtLocation(WorldLocation).AmbientTemperature;
}

float ABiomeManager::GetDangerLevelAtLocation(const FVector& WorldLocation) const
{
    return GetBiomeDataAtLocation(WorldLocation).DangerLevel;
}

float ABiomeManager::ComputeWeight(const FVector& Location, const FWorld_BiomeZone& Zone) const
{
    const float Dist = FVector::Dist2D(Location, Zone.Center);
    if (Dist >= Zone.Radius + BlendRadius) return 0.0f;
    if (Dist <= Zone.Radius - BlendRadius) return 1.0f;

    // Smooth blend in the transition band
    const float BlendStart = Zone.Radius - BlendRadius;
    const float BlendEnd   = Zone.Radius + BlendRadius;
    const float Alpha = 1.0f - FMath::Clamp((Dist - BlendStart) / (BlendEnd - BlendStart), 0.0f, 1.0f);
    return FMath::SmoothStep(0.0f, 1.0f, Alpha);
}

FWorld_BiomeData ABiomeManager::BlendBiomeData(const FVector& WorldLocation) const
{
    // Gather weighted contributions from all zones
    float TotalWeight = 0.0f;
    float FogDensity = 0.0f;
    float FogHeightFalloff = 0.0f;
    FLinearColor FogColor = FLinearColor::Black;
    float Temperature = 0.0f;
    float Humidity = 0.0f;
    float Danger = 0.0f;

    // Find dominant biome for type/name
    float MaxWeight = 0.0f;
    int32 DominantIdx = 0;

    for (int32 i = 0; i < BiomeZones.Num(); ++i)
    {
        const float W = ComputeWeight(WorldLocation, BiomeZones[i]);
        if (W <= 0.0f) continue;

        TotalWeight += W;
        FogDensity      += W * BiomeZones[i].BiomeData.FogDensity;
        FogHeightFalloff += W * BiomeZones[i].BiomeData.FogHeightFalloff;
        FogColor        += BiomeZones[i].BiomeData.FogInscatteringColor * W;
        Temperature     += W * BiomeZones[i].BiomeData.AmbientTemperature;
        Humidity        += W * BiomeZones[i].BiomeData.HumidityFactor;
        Danger          += W * BiomeZones[i].BiomeData.DangerLevel;

        if (W > MaxWeight)
        {
            MaxWeight = W;
            DominantIdx = i;
        }
    }

    FWorld_BiomeData Result;
    if (TotalWeight <= 0.0f)
    {
        // Outside all zones — return default wilderness
        Result.BiomeType = EWorld_BiomeType::Unknown;
        Result.BiomeName = TEXT("Wilderness");
        Result.FogDensity = 0.02f;
        Result.AmbientTemperature = 20.0f;
        Result.DangerLevel = 0.3f;
        return Result;
    }

    const float InvTotal = 1.0f / TotalWeight;
    Result.FogDensity          = FogDensity * InvTotal;
    Result.FogHeightFalloff    = FogHeightFalloff * InvTotal;
    Result.FogInscatteringColor = FogColor * InvTotal;
    Result.AmbientTemperature  = Temperature * InvTotal;
    Result.HumidityFactor      = Humidity * InvTotal;
    Result.DangerLevel         = Danger * InvTotal;

    // Dominant biome determines type and name
    Result.BiomeType = BiomeZones[DominantIdx].BiomeData.BiomeType;
    Result.BiomeName = BiomeZones[DominantIdx].BiomeData.BiomeName;

    return Result;
}
