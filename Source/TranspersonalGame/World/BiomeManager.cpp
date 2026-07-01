// BiomeManager.cpp
// Prehistoric survival game — biome authority system
// Agent #02 — Engine Architect

#include "BiomeManager.h"
#include "Engine/World.h"

// ============================================================
//  Constructor
// ============================================================
ABiomeManager::ABiomeManager()
{
    PrimaryActorTick.bCanEverTick = false;

    // Default biome: open savanna
    DefaultBiome.BiomeType          = EEng_BiomeType::Savanna;
    DefaultBiome.DisplayName        = TEXT("Savanna");
    DefaultBiome.RadiusCm           = 1000000.0f; // 10 km catch-all
    DefaultBiome.CentreLocation     = FVector::ZeroVector;
    DefaultBiome.AmbientTemperatureC = 30.0f;
    DefaultBiome.Humidity           = 0.3f;
    DefaultBiome.PredatorDensity    = 1.0f;
    DefaultBiome.FoodAvailability   = 0.4f;
    DefaultBiome.FogDensityMultiplier = 0.8f;
    DefaultBiome.GroundTint         = FLinearColor(0.55f, 0.48f, 0.25f, 1.0f);
    DefaultBiome.FoliageDensity     = 0.3f;
}

// ============================================================
//  BeginPlay
// ============================================================
void ABiomeManager::BeginPlay()
{
    Super::BeginPlay();

    if (BiomeZones.Num() == 0)
    {
        InitializeDefaultBiomes();
    }
}

// ============================================================
//  InitializeDefaultBiomes — 8 prehistoric biomes
// ============================================================
void ABiomeManager::InitializeDefaultBiomes()
{
    BiomeZones.Empty();

    // --- Jungle (dense, humid, high predator) ---
    {
        FEng_BiomeData b;
        b.BiomeType             = EEng_BiomeType::Jungle;
        b.DisplayName           = TEXT("Cretaceous Jungle");
        b.RadiusCm              = 80000.0f;
        b.CentreLocation        = FVector(60000.0f, 40000.0f, 0.0f);
        b.AmbientTemperatureC   = 35.0f;
        b.Humidity              = 0.9f;
        b.PredatorDensity       = 1.8f;
        b.FoodAvailability      = 0.8f;
        b.FogDensityMultiplier  = 1.5f;
        b.GroundTint            = FLinearColor(0.1f, 0.3f, 0.05f, 1.0f);
        b.FoliageDensity        = 0.95f;
        BiomeZones.Add(b);
    }

    // --- Savanna (open plains, moderate predator) ---
    {
        FEng_BiomeData b;
        b.BiomeType             = EEng_BiomeType::Savanna;
        b.DisplayName           = TEXT("Open Savanna");
        b.RadiusCm              = 100000.0f;
        b.CentreLocation        = FVector(0.0f, 0.0f, 0.0f);
        b.AmbientTemperatureC   = 30.0f;
        b.Humidity              = 0.3f;
        b.PredatorDensity       = 1.0f;
        b.FoodAvailability      = 0.4f;
        b.FogDensityMultiplier  = 0.7f;
        b.GroundTint            = FLinearColor(0.55f, 0.48f, 0.25f, 1.0f);
        b.FoliageDensity        = 0.3f;
        BiomeZones.Add(b);
    }

    // --- Swamp (slow movement, disease risk) ---
    {
        FEng_BiomeData b;
        b.BiomeType             = EEng_BiomeType::Swamp;
        b.DisplayName           = TEXT("Primordial Swamp");
        b.RadiusCm              = 60000.0f;
        b.CentreLocation        = FVector(-50000.0f, 70000.0f, -500.0f);
        b.AmbientTemperatureC   = 28.0f;
        b.Humidity              = 0.95f;
        b.PredatorDensity       = 1.4f;
        b.FoodAvailability      = 0.6f;
        b.FogDensityMultiplier  = 2.0f;
        b.GroundTint            = FLinearColor(0.15f, 0.2f, 0.08f, 1.0f);
        b.FoliageDensity        = 0.7f;
        BiomeZones.Add(b);
    }

    // --- Volcanic (extreme heat, low food) ---
    {
        FEng_BiomeData b;
        b.BiomeType             = EEng_BiomeType::Volcanic;
        b.DisplayName           = TEXT("Volcanic Badlands");
        b.RadiusCm              = 40000.0f;
        b.CentreLocation        = FVector(90000.0f, -60000.0f, 2000.0f);
        b.AmbientTemperatureC   = 55.0f;
        b.Humidity              = 0.05f;
        b.PredatorDensity       = 0.5f;
        b.FoodAvailability      = 0.1f;
        b.FogDensityMultiplier  = 1.8f;
        b.GroundTint            = FLinearColor(0.3f, 0.1f, 0.05f, 1.0f);
        b.FoliageDensity        = 0.05f;
        BiomeZones.Add(b);
    }

    // --- Coastal Plain (rich resources, amphibious predators) ---
    {
        FEng_BiomeData b;
        b.BiomeType             = EEng_BiomeType::CoastalPlain;
        b.DisplayName           = TEXT("Coastal Plain");
        b.RadiusCm              = 70000.0f;
        b.CentreLocation        = FVector(-80000.0f, -30000.0f, 0.0f);
        b.AmbientTemperatureC   = 26.0f;
        b.Humidity              = 0.65f;
        b.PredatorDensity       = 1.2f;
        b.FoodAvailability      = 0.75f;
        b.FogDensityMultiplier  = 1.1f;
        b.GroundTint            = FLinearColor(0.4f, 0.45f, 0.3f, 1.0f);
        b.FoliageDensity        = 0.5f;
        BiomeZones.Add(b);
    }

    // --- River Delta (fresh water, high food) ---
    {
        FEng_BiomeData b;
        b.BiomeType             = EEng_BiomeType::RiverDelta;
        b.DisplayName           = TEXT("River Delta");
        b.RadiusCm              = 50000.0f;
        b.CentreLocation        = FVector(30000.0f, -80000.0f, -200.0f);
        b.AmbientTemperatureC   = 27.0f;
        b.Humidity              = 0.8f;
        b.PredatorDensity       = 1.3f;
        b.FoodAvailability      = 0.9f;
        b.FogDensityMultiplier  = 1.3f;
        b.GroundTint            = FLinearColor(0.2f, 0.35f, 0.15f, 1.0f);
        b.FoliageDensity        = 0.65f;
        BiomeZones.Add(b);
    }

    // --- Montane Forest (cold, low predator, high stamina drain) ---
    {
        FEng_BiomeData b;
        b.BiomeType             = EEng_BiomeType::MontaneForest;
        b.DisplayName           = TEXT("Montane Forest");
        b.RadiusCm              = 55000.0f;
        b.CentreLocation        = FVector(-20000.0f, -90000.0f, 3000.0f);
        b.AmbientTemperatureC   = 12.0f;
        b.Humidity              = 0.6f;
        b.PredatorDensity       = 0.7f;
        b.FoodAvailability      = 0.5f;
        b.FogDensityMultiplier  = 1.4f;
        b.GroundTint            = FLinearColor(0.2f, 0.28f, 0.12f, 1.0f);
        b.FoliageDensity        = 0.75f;
        BiomeZones.Add(b);
    }

    // --- Badlands (arid, sparse, T-Rex territory) ---
    {
        FEng_BiomeData b;
        b.BiomeType             = EEng_BiomeType::Badlands;
        b.DisplayName           = TEXT("Badlands");
        b.RadiusCm              = 65000.0f;
        b.CentreLocation        = FVector(70000.0f, 80000.0f, 500.0f);
        b.AmbientTemperatureC   = 42.0f;
        b.Humidity              = 0.1f;
        b.PredatorDensity       = 2.0f;   // T-Rex territory
        b.FoodAvailability      = 0.2f;
        b.FogDensityMultiplier  = 0.5f;
        b.GroundTint            = FLinearColor(0.5f, 0.35f, 0.2f, 1.0f);
        b.FoliageDensity        = 0.1f;
        BiomeZones.Add(b);
    }

    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Initialized %d default biomes"), BiomeZones.Num());
}

// ============================================================
//  FindClosestBiome — internal helper
// ============================================================
const FEng_BiomeData* ABiomeManager::FindClosestBiome(const FVector& WorldLocation) const
{
    const FEng_BiomeData* Closest = nullptr;
    float ClosestDist = FLT_MAX;

    for (const FEng_BiomeData& Biome : BiomeZones)
    {
        const float Dist = FVector::Dist2D(WorldLocation, Biome.CentreLocation);
        if (Dist < Biome.RadiusCm && Dist < ClosestDist)
        {
            ClosestDist = Dist;
            Closest = &Biome;
        }
    }

    return Closest;
}

// ============================================================
//  Public API
// ============================================================
EEng_BiomeType ABiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    const FEng_BiomeData* Found = FindClosestBiome(WorldLocation);
    return Found ? Found->BiomeType : DefaultBiome.BiomeType;
}

FEng_BiomeData ABiomeManager::GetBiomeDataAtLocation(const FVector& WorldLocation) const
{
    const FEng_BiomeData* Found = FindClosestBiome(WorldLocation);
    return Found ? *Found : DefaultBiome;
}

bool ABiomeManager::IsLocationInBiome(const FVector& WorldLocation, EEng_BiomeType BiomeType) const
{
    return GetBiomeAtLocation(WorldLocation) == BiomeType;
}

float ABiomeManager::GetPredatorDensityAt(const FVector& WorldLocation) const
{
    const FEng_BiomeData* Found = FindClosestBiome(WorldLocation);
    return Found ? Found->PredatorDensity : DefaultBiome.PredatorDensity;
}

float ABiomeManager::GetTemperatureAt(const FVector& WorldLocation) const
{
    const FEng_BiomeData* Found = FindClosestBiome(WorldLocation);
    return Found ? Found->AmbientTemperatureC : DefaultBiome.AmbientTemperatureC;
}

float ABiomeManager::GetFoodAvailabilityAt(const FVector& WorldLocation) const
{
    const FEng_BiomeData* Found = FindClosestBiome(WorldLocation);
    return Found ? Found->FoodAvailability : DefaultBiome.FoodAvailability;
}

void ABiomeManager::RegisterBiome(const FEng_BiomeData& BiomeData)
{
    // Replace existing biome of same type if present
    for (FEng_BiomeData& Existing : BiomeZones)
    {
        if (Existing.BiomeType == BiomeData.BiomeType &&
            FVector::Dist(Existing.CentreLocation, BiomeData.CentreLocation) < 1000.0f)
        {
            Existing = BiomeData;
            return;
        }
    }
    BiomeZones.Add(BiomeData);
}

TArray<FEng_BiomeData> ABiomeManager::GetAllBiomes() const
{
    return BiomeZones;
}

void ABiomeManager::RebuildDefaultBiomes()
{
    BiomeZones.Empty();
    InitializeDefaultBiomes();
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Rebuilt %d default biomes"), BiomeZones.Num());
}
