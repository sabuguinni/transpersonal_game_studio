#include "BiomeZoneManager.h"
#include "DrawDebugHelpers.h"

ABiomeZoneManager::ABiomeZoneManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.5f; // Update every 0.5s for performance
    SetupDefaultBiomes();
}

void ABiomeZoneManager::BeginPlay()
{
    Super::BeginPlay();
    if (BiomeZones.Num() == 0)
    {
        SetupDefaultBiomes();
    }
}

void ABiomeZoneManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

#if WITH_EDITOR
    if (bShowDebugBiomeBounds)
    {
        for (const FWorld_BiomeData& Biome : BiomeZones)
        {
            DrawDebugSphere(
                GetWorld(),
                Biome.CenterLocation,
                Biome.Radius,
                32,
                Biome.AmbientColor.ToFColor(true),
                false,
                0.6f
            );
        }
    }
#endif
}

EWorld_BiomeType ABiomeZoneManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    EWorld_BiomeType ClosestBiome = EWorld_BiomeType::None;
    float ClosestDist = MAX_FLT;

    for (const FWorld_BiomeData& Biome : BiomeZones)
    {
        float Dist = FVector::Dist2D(WorldLocation, Biome.CenterLocation);
        if (Dist < Biome.Radius && Dist < ClosestDist)
        {
            ClosestDist = Dist;
            ClosestBiome = Biome.BiomeType;
        }
    }

    return ClosestBiome;
}

FWorld_BiomeData ABiomeZoneManager::GetBiomeData(EWorld_BiomeType BiomeType) const
{
    for (const FWorld_BiomeData& Biome : BiomeZones)
    {
        if (Biome.BiomeType == BiomeType)
        {
            return Biome;
        }
    }
    return FWorld_BiomeData();
}

void ABiomeZoneManager::RegisterBiome(const FWorld_BiomeData& BiomeData)
{
    // Remove existing biome of same type if present
    BiomeZones.RemoveAll([&BiomeData](const FWorld_BiomeData& Existing)
    {
        return Existing.BiomeType == BiomeData.BiomeType;
    });
    BiomeZones.Add(BiomeData);
}

TArray<FWorld_BiomeData> ABiomeZoneManager::GetAllBiomes() const
{
    return BiomeZones;
}

float ABiomeZoneManager::GetBiomeBlendFactor(const FVector& WorldLocation, EWorld_BiomeType BiomeType) const
{
    for (const FWorld_BiomeData& Biome : BiomeZones)
    {
        if (Biome.BiomeType == BiomeType)
        {
            float Dist = FVector::Dist2D(WorldLocation, Biome.CenterLocation);
            if (Dist >= Biome.Radius)
            {
                return 0.0f;
            }
            float BlendStart = Biome.Radius - BiomeBlendRadius;
            if (Dist <= BlendStart)
            {
                return 1.0f;
            }
            // Linear blend in the outer ring
            return 1.0f - ((Dist - BlendStart) / BiomeBlendRadius);
        }
    }
    return 0.0f;
}

void ABiomeZoneManager::InitializeDefaultBiomes()
{
    BiomeZones.Empty();
    SetupDefaultBiomes();
}

void ABiomeZoneManager::SetupDefaultBiomes()
{
    // Dense Jungle — Northwest quadrant
    {
        FWorld_BiomeData Jungle;
        Jungle.BiomeType = EWorld_BiomeType::DenseJungle;
        Jungle.CenterLocation = FVector(-3000.0f, 0.0f, 0.0f);
        Jungle.Radius = 3500.0f;
        Jungle.AmbientColor = FLinearColor(0.1f, 0.9f, 0.2f, 1.0f);
        Jungle.FogDensity = 0.04f;
        Jungle.TemperatureCelsius = 32.0f;
        Jungle.HumidityPercent = 90.0f;
        Jungle.bHasPredators = true;
        Jungle.DinosaurDensity = 1.5f;
        BiomeZones.Add(Jungle);
    }

    // Rocky Highlands — East
    {
        FWorld_BiomeData Rocky;
        Rocky.BiomeType = EWorld_BiomeType::RockyHighlands;
        Rocky.CenterLocation = FVector(3500.0f, 0.0f, 300.0f);
        Rocky.Radius = 3000.0f;
        Rocky.AmbientColor = FLinearColor(0.9f, 0.5f, 0.1f, 1.0f);
        Rocky.FogDensity = 0.01f;
        Rocky.TemperatureCelsius = 22.0f;
        Rocky.HumidityPercent = 30.0f;
        Rocky.bHasPredators = true;
        Rocky.DinosaurDensity = 0.8f;
        BiomeZones.Add(Rocky);
    }

    // River Valley — Center
    {
        FWorld_BiomeData River;
        River.BiomeType = EWorld_BiomeType::RiverValley;
        River.CenterLocation = FVector(0.0f, 0.0f, 0.0f);
        River.Radius = 2000.0f;
        River.AmbientColor = FLinearColor(0.2f, 0.4f, 0.95f, 1.0f);
        River.FogDensity = 0.025f;
        River.TemperatureCelsius = 26.0f;
        River.HumidityPercent = 75.0f;
        River.bHasPredators = false;
        River.DinosaurDensity = 1.2f;
        BiomeZones.Add(River);
    }

    // Open Plains — North and South
    {
        FWorld_BiomeData Plains;
        Plains.BiomeType = EWorld_BiomeType::OpenPlains;
        Plains.CenterLocation = FVector(0.0f, -5000.0f, 0.0f);
        Plains.Radius = 4000.0f;
        Plains.AmbientColor = FLinearColor(0.95f, 0.9f, 0.2f, 1.0f);
        Plains.FogDensity = 0.005f;
        Plains.TemperatureCelsius = 30.0f;
        Plains.HumidityPercent = 40.0f;
        Plains.bHasPredators = true;
        Plains.DinosaurDensity = 2.0f; // Herds on open plains
        BiomeZones.Add(Plains);
    }

    // Volcanic Zone — Far East
    {
        FWorld_BiomeData Volcanic;
        Volcanic.BiomeType = EWorld_BiomeType::VolcanicZone;
        Volcanic.CenterLocation = FVector(6000.0f, 0.0f, 500.0f);
        Volcanic.Radius = 2500.0f;
        Volcanic.AmbientColor = FLinearColor(0.95f, 0.2f, 0.05f, 1.0f);
        Volcanic.FogDensity = 0.06f;
        Volcanic.TemperatureCelsius = 55.0f;
        Volcanic.HumidityPercent = 20.0f;
        Volcanic.bHasPredators = true;
        Volcanic.DinosaurDensity = 0.5f; // Harsh environment
        BiomeZones.Add(Volcanic);
    }

    // Marshland — South delta
    {
        FWorld_BiomeData Marsh;
        Marsh.BiomeType = EWorld_BiomeType::Marshland;
        Marsh.CenterLocation = FVector(0.0f, 5000.0f, -20.0f);
        Marsh.Radius = 2800.0f;
        Marsh.AmbientColor = FLinearColor(0.3f, 0.6f, 0.4f, 1.0f);
        Marsh.FogDensity = 0.05f;
        Marsh.TemperatureCelsius = 29.0f;
        Marsh.HumidityPercent = 95.0f;
        Marsh.bHasPredators = false;
        Marsh.DinosaurDensity = 1.8f; // Aquatic/semi-aquatic dinos
        BiomeZones.Add(Marsh);
    }
}
