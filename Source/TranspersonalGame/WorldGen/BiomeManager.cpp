// BiomeManager.cpp
// Agent #5 — Procedural World Generator
// Full implementation of 5-biome PCG world zone system

#include "BiomeManager.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

ABiomeManager::ABiomeManager()
{
    PrimaryActorTick.bCanEverTick = false;
    SetupDefaultBiomeZones();
}

void ABiomeManager::BeginPlay()
{
    Super::BeginPlay();
    if (bAutoInitializeOnBeginPlay)
    {
        InitializeBiomes();
    }
}

void ABiomeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ABiomeManager::InitializeBiomes()
{
    if (BiomeZones.Num() == 0)
    {
        SetupDefaultBiomeZones();
    }
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Initialized %d biome zones"), BiomeZones.Num());
}

void ABiomeManager::SetupDefaultBiomeZones()
{
    BiomeZones.Empty();

    // Dense Forest — northwest quadrant
    {
        FWorld_BiomeZone Zone;
        Zone.BiomeType = EWorld_BiomeType::DenseForest;
        Zone.Center = FVector(-2000.0f, -2000.0f, 0.0f);
        Zone.Radius = 1200.0f;
        Zone.HeightOffset = 0.0f;
        Zone.DebugColor = FLinearColor(0.05f, 0.45f, 0.05f, 1.0f);
        Zone.VegetationDensity = 0.95f;
        Zone.Temperature = 22.0f;
        Zone.Humidity = 0.85f;
        BiomeZones.Add(Zone);
    }

    // Open Plains — northeast quadrant
    {
        FWorld_BiomeZone Zone;
        Zone.BiomeType = EWorld_BiomeType::OpenPlains;
        Zone.Center = FVector(2000.0f, -1500.0f, 0.0f);
        Zone.Radius = 1500.0f;
        Zone.HeightOffset = 0.0f;
        Zone.DebugColor = FLinearColor(0.55f, 0.65f, 0.15f, 1.0f);
        Zone.VegetationDensity = 0.30f;
        Zone.Temperature = 28.0f;
        Zone.Humidity = 0.35f;
        BiomeZones.Add(Zone);
    }

    // Rocky Highlands — south-center
    {
        FWorld_BiomeZone Zone;
        Zone.BiomeType = EWorld_BiomeType::RockyHighlands;
        Zone.Center = FVector(-1000.0f, 2500.0f, 200.0f);
        Zone.Radius = 1000.0f;
        Zone.HeightOffset = 200.0f;
        Zone.DebugColor = FLinearColor(0.50f, 0.42f, 0.32f, 1.0f);
        Zone.VegetationDensity = 0.15f;
        Zone.Temperature = 18.0f;
        Zone.Humidity = 0.20f;
        BiomeZones.Add(Zone);
    }

    // River Valley — southeast
    {
        FWorld_BiomeZone Zone;
        Zone.BiomeType = EWorld_BiomeType::RiverValley;
        Zone.Center = FVector(1500.0f, 2000.0f, -50.0f);
        Zone.Radius = 800.0f;
        Zone.HeightOffset = -50.0f;
        Zone.DebugColor = FLinearColor(0.15f, 0.50f, 0.65f, 1.0f);
        Zone.VegetationDensity = 0.75f;
        Zone.Temperature = 24.0f;
        Zone.Humidity = 0.90f;
        BiomeZones.Add(Zone);
    }

    // Volcanic Badlands — west
    {
        FWorld_BiomeZone Zone;
        Zone.BiomeType = EWorld_BiomeType::VolcanicBadlands;
        Zone.Center = FVector(-3000.0f, 500.0f, 150.0f);
        Zone.Radius = 900.0f;
        Zone.HeightOffset = 150.0f;
        Zone.DebugColor = FLinearColor(0.65f, 0.20f, 0.08f, 1.0f);
        Zone.VegetationDensity = 0.05f;
        Zone.Temperature = 45.0f;
        Zone.Humidity = 0.05f;
        BiomeZones.Add(Zone);
    }
}

EWorld_BiomeType ABiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    float ClosestDist = MAX_FLT;
    EWorld_BiomeType ClosestBiome = EWorld_BiomeType::Unknown;

    for (const FWorld_BiomeZone& Zone : BiomeZones)
    {
        float Dist = FVector::Dist2D(WorldLocation, Zone.Center);
        if (Dist < Zone.Radius && Dist < ClosestDist)
        {
            ClosestDist = Dist;
            ClosestBiome = Zone.BiomeType;
        }
    }
    return ClosestBiome;
}

bool ABiomeManager::GetBiomeZoneAtLocation(const FVector& WorldLocation, FWorld_BiomeZone& OutZone) const
{
    float ClosestDist = MAX_FLT;
    int32 ClosestIdx = INDEX_NONE;

    for (int32 i = 0; i < BiomeZones.Num(); ++i)
    {
        float Dist = FVector::Dist2D(WorldLocation, BiomeZones[i].Center);
        if (Dist < BiomeZones[i].Radius && Dist < ClosestDist)
        {
            ClosestDist = Dist;
            ClosestIdx = i;
        }
    }

    if (ClosestIdx != INDEX_NONE)
    {
        OutZone = BiomeZones[ClosestIdx];
        return true;
    }
    return false;
}

TArray<FWorld_BiomeZone> ABiomeManager::GetAllBiomeZones() const
{
    return BiomeZones;
}

float ABiomeManager::GetVegetationDensityAtLocation(const FVector& WorldLocation) const
{
    FWorld_BiomeZone Zone;
    if (GetBiomeZoneAtLocation(WorldLocation, Zone))
    {
        float Dist = FVector::Dist2D(WorldLocation, Zone.Center);
        float BlendFactor = FMath::Clamp(1.0f - (Dist / Zone.Radius), 0.0f, 1.0f);
        return Zone.VegetationDensity * BlendFactor;
    }
    return 0.1f; // Default sparse outside any biome
}

float ABiomeManager::GetTemperatureAtLocation(const FVector& WorldLocation) const
{
    FWorld_BiomeZone Zone;
    if (GetBiomeZoneAtLocation(WorldLocation, Zone))
    {
        return Zone.Temperature;
    }
    return 25.0f; // Default ambient temperature
}

float ABiomeManager::GetHumidityAtLocation(const FVector& WorldLocation) const
{
    FWorld_BiomeZone Zone;
    if (GetBiomeZoneAtLocation(WorldLocation, Zone))
    {
        return Zone.Humidity;
    }
    return 0.4f; // Default ambient humidity
}

void ABiomeManager::DrawBiomeBoundaries()
{
    UWorld* World = GetWorld();
    if (!World) return;

    for (const FWorld_BiomeZone& Zone : BiomeZones)
    {
        FColor DrawColor = Zone.DebugColor.ToFColor(true);
        DrawDebugCircle(
            World,
            Zone.Center,
            Zone.Radius,
            64,
            DrawColor,
            false,
            DebugDrawDuration,
            0,
            15.0f,
            FVector(1, 0, 0),
            FVector(0, 1, 0)
        );
        DrawDebugString(
            World,
            Zone.Center + FVector(0, 0, 200),
            UEnum::GetValueAsString(Zone.BiomeType),
            nullptr,
            DrawColor,
            DebugDrawDuration
        );
    }
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Drew %d biome boundaries"), BiomeZones.Num());
}

float ABiomeManager::CalculateBlendWeight(const FVector& Location, const FWorld_BiomeZone& Zone) const
{
    float Dist = FVector::Dist2D(Location, Zone.Center);
    if (Dist >= Zone.Radius) return 0.0f;
    return FMath::Clamp(1.0f - (Dist / Zone.Radius), 0.0f, 1.0f);
}

int32 ABiomeManager::GetBiomeCount() const
{
    return BiomeZones.Num();
}
