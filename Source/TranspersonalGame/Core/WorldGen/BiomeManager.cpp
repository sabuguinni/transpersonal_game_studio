// BiomeManager.cpp
// Agent #5 — Procedural World Generator
// Implements biome zone management, PCG terrain variation, and river system for MinPlayableMap

#include "BiomeManager.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UBiomeManager::UBiomeManager()
{
    // Default biome configuration
    ActiveBiomeCount = 0;
    bBiomesInitialized = false;
    GlobalWetnessScale = 1.0f;
    GlobalTemperatureScale = 1.0f;
    RiverFlowSpeed = 150.0f;
    bEnableWeatherTransitions = true;
}

void UBiomeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    InitializeBiomeZones();
    unreal::log("BiomeManager: Initialized with 5 biome zones");
}

void UBiomeManager::Deinitialize()
{
    BiomeZones.Empty();
    RiverSegments.Empty();
    Super::Deinitialize();
}

void UBiomeManager::InitializeBiomeZones()
{
    BiomeZones.Empty();

    // Forest Zone — dense prehistoric jungle
    FWorld_BiomeZone Forest;
    Forest.BiomeName = FName("Forest_Zone");
    Forest.BiomeType = EWorld_BiomeType::DenseForest;
    Forest.CenterLocation = FVector(-8000.f, -8000.f, 0.f);
    Forest.RadiusMeters = 4000.f;
    Forest.BaseTemperatureCelsius = 28.f;
    Forest.HumidityPercent = 85.f;
    Forest.VegetationDensity = 0.92f;
    Forest.DinosaurSpawnWeight = 0.75f;
    Forest.GroundColor = FLinearColor(0.02f, 0.18f, 0.02f, 1.f);
    BiomeZones.Add(Forest);

    // Volcanic Plains — ash-covered open terrain
    FWorld_BiomeZone Volcanic;
    Volcanic.BiomeName = FName("Volcanic_Plains");
    Volcanic.BiomeType = EWorld_BiomeType::VolcanicPlains;
    Volcanic.CenterLocation = FVector(8000.f, -4000.f, 0.f);
    Volcanic.RadiusMeters = 3500.f;
    Volcanic.BaseTemperatureCelsius = 52.f;
    Volcanic.HumidityPercent = 12.f;
    Volcanic.VegetationDensity = 0.08f;
    Volcanic.DinosaurSpawnWeight = 0.35f;
    Volcanic.GroundColor = FLinearColor(0.25f, 0.05f, 0.01f, 1.f);
    BiomeZones.Add(Volcanic);

    // River Delta — shallow water, mudflats
    FWorld_BiomeZone RiverDelta;
    RiverDelta.BiomeName = FName("River_Delta");
    RiverDelta.BiomeType = EWorld_BiomeType::RiverDelta;
    RiverDelta.CenterLocation = FVector(0.f, 10000.f, -50.f);
    RiverDelta.RadiusMeters = 5000.f;
    RiverDelta.BaseTemperatureCelsius = 24.f;
    RiverDelta.HumidityPercent = 95.f;
    RiverDelta.VegetationDensity = 0.55f;
    RiverDelta.DinosaurSpawnWeight = 0.90f;
    RiverDelta.GroundColor = FLinearColor(0.01f, 0.08f, 0.22f, 1.f);
    BiomeZones.Add(RiverDelta);

    // Rocky Highlands — cliffs, sparse vegetation
    FWorld_BiomeZone Highlands;
    Highlands.BiomeName = FName("Rocky_Highlands");
    Highlands.BiomeType = EWorld_BiomeType::RockyHighlands;
    Highlands.CenterLocation = FVector(-5000.f, 6000.f, 800.f);
    Highlands.RadiusMeters = 2500.f;
    Highlands.BaseTemperatureCelsius = 15.f;
    Highlands.HumidityPercent = 35.f;
    Highlands.VegetationDensity = 0.22f;
    Highlands.DinosaurSpawnWeight = 0.45f;
    Highlands.GroundColor = FLinearColor(0.28f, 0.22f, 0.15f, 1.f);
    BiomeZones.Add(Highlands);

    // Coastal Swamp — murky water, mangrove-like trees
    FWorld_BiomeZone Swamp;
    Swamp.BiomeName = FName("Coastal_Swamp");
    Swamp.BiomeType = EWorld_BiomeType::CoastalSwamp;
    Swamp.CenterLocation = FVector(6000.f, 8000.f, -100.f);
    Swamp.RadiusMeters = 2800.f;
    Swamp.BaseTemperatureCelsius = 30.f;
    Swamp.HumidityPercent = 98.f;
    Swamp.VegetationDensity = 0.68f;
    Swamp.DinosaurSpawnWeight = 0.80f;
    Swamp.GroundColor = FLinearColor(0.04f, 0.12f, 0.06f, 1.f);
    BiomeZones.Add(Swamp);

    ActiveBiomeCount = BiomeZones.Num();
    bBiomesInitialized = true;

    // Initialize river segments connecting biomes
    InitializeRiverSystem();
}

void UBiomeManager::InitializeRiverSystem()
{
    RiverSegments.Empty();

    // River flows: Rocky Highlands → River Delta → Coastal Swamp
    TArray<FVector> RiverPath = {
        FVector(-5000.f, 5500.f, 750.f),   // Source: Highland lake
        FVector(-2500.f, 6000.f, -20.f),
        FVector(-1000.f, 8000.f, -30.f),
        FVector(500.f,   9500.f, -40.f),
        FVector(2000.f, 10500.f, -50.f),
        FVector(3500.f, 11000.f, -60.f),
        FVector(4500.f, 11500.f, -80.f),   // Mouth: Coastal swamp
    };

    for (int32 i = 0; i < RiverPath.Num() - 1; ++i)
    {
        FWorld_RiverSegment Seg;
        Seg.StartPoint = RiverPath[i];
        Seg.EndPoint = RiverPath[i + 1];
        Seg.WidthMeters = FMath::Lerp(8.f, 40.f, (float)i / (float)(RiverPath.Num() - 1));
        Seg.DepthMeters = FMath::Lerp(1.5f, 6.f, (float)i / (float)(RiverPath.Num() - 1));
        Seg.FlowSpeed = RiverFlowSpeed * (1.f - 0.05f * i); // Slows near delta
        Seg.bIsCrossable = Seg.DepthMeters < 3.f;
        RiverSegments.Add(Seg);
    }
}

EWorld_BiomeType UBiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    if (!bBiomesInitialized) return EWorld_BiomeType::DenseForest;

    float ClosestDist = MAX_FLT;
    EWorld_BiomeType ClosestBiome = EWorld_BiomeType::DenseForest;

    for (const FWorld_BiomeZone& Zone : BiomeZones)
    {
        float Dist = FVector::Dist2D(WorldLocation, Zone.CenterLocation);
        if (Dist < ClosestDist)
        {
            ClosestDist = Dist;
            ClosestBiome = Zone.BiomeType;
        }
    }

    return ClosestBiome;
}

float UBiomeManager::GetTemperatureAtLocation(const FVector& WorldLocation) const
{
    EWorld_BiomeType Biome = GetBiomeAtLocation(WorldLocation);
    for (const FWorld_BiomeZone& Zone : BiomeZones)
    {
        if (Zone.BiomeType == Biome)
        {
            // Altitude modifier: -6.5°C per 1000m
            float AltitudeModifier = (WorldLocation.Z / 100.f) * -0.0065f * 100.f;
            return (Zone.BaseTemperatureCelsius + AltitudeModifier) * GlobalTemperatureScale;
        }
    }
    return 20.f;
}

float UBiomeManager::GetHumidityAtLocation(const FVector& WorldLocation) const
{
    EWorld_BiomeType Biome = GetBiomeAtLocation(WorldLocation);
    for (const FWorld_BiomeZone& Zone : BiomeZones)
    {
        if (Zone.BiomeType == Biome)
        {
            return Zone.HumidityPercent * GlobalWetnessScale;
        }
    }
    return 50.f;
}

float UBiomeManager::GetVegetationDensityAtLocation(const FVector& WorldLocation) const
{
    EWorld_BiomeType Biome = GetBiomeAtLocation(WorldLocation);
    for (const FWorld_BiomeZone& Zone : BiomeZones)
    {
        if (Zone.BiomeType == Biome)
        {
            return Zone.VegetationDensity;
        }
    }
    return 0.5f;
}

bool UBiomeManager::IsLocationInRiver(const FVector& WorldLocation, float Tolerance) const
{
    for (const FWorld_RiverSegment& Seg : RiverSegments)
    {
        // Project point onto river segment
        FVector Dir = (Seg.EndPoint - Seg.StartPoint).GetSafeNormal();
        FVector ToPoint = WorldLocation - Seg.StartPoint;
        float Proj = FVector::DotProduct(ToPoint, Dir);
        float SegLen = FVector::Dist(Seg.StartPoint, Seg.EndPoint);

        if (Proj >= 0.f && Proj <= SegLen)
        {
            FVector ClosestPoint = Seg.StartPoint + Dir * Proj;
            float DistToRiver = FVector::Dist2D(WorldLocation, ClosestPoint);
            if (DistToRiver <= (Seg.WidthMeters * 0.5f + Tolerance))
            {
                return true;
            }
        }
    }
    return false;
}

float UBiomeManager::GetDinosaurSpawnWeightAtLocation(const FVector& WorldLocation) const
{
    EWorld_BiomeType Biome = GetBiomeAtLocation(WorldLocation);
    for (const FWorld_BiomeZone& Zone : BiomeZones)
    {
        if (Zone.BiomeType == Biome)
        {
            return Zone.DinosaurSpawnWeight;
        }
    }
    return 0.5f;
}

const FWorld_BiomeZone* UBiomeManager::GetBiomeZoneByType(EWorld_BiomeType BiomeType) const
{
    for (const FWorld_BiomeZone& Zone : BiomeZones)
    {
        if (Zone.BiomeType == BiomeType)
        {
            return &Zone;
        }
    }
    return nullptr;
}

TArray<FWorld_RiverSegment> UBiomeManager::GetRiverSegmentsNearLocation(const FVector& WorldLocation, float SearchRadius) const
{
    TArray<FWorld_RiverSegment> NearbySegments;
    for (const FWorld_RiverSegment& Seg : RiverSegments)
    {
        FVector MidPoint = (Seg.StartPoint + Seg.EndPoint) * 0.5f;
        if (FVector::Dist(WorldLocation, MidPoint) <= SearchRadius)
        {
            NearbySegments.Add(Seg);
        }
    }
    return NearbySegments;
}
