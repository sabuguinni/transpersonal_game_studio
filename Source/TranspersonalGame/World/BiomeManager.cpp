// BiomeManager.cpp
// Engine Architect #02 — Transpersonal Game Studio
// Full implementation of the Biome system for the prehistoric survival game.
// Cycle: PROD_CYCLE_AUTO_20260623_009

#include "BiomeManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

// ============================================================
// Constructor
// ============================================================

UBiomeManager::UBiomeManager()
{
    // Default biome definitions — prehistoric Cretaceous world
    FEng_BiomeDefinition Grassland;
    Grassland.BiomeID        = EEng_BiomeType::Grassland;
    Grassland.BiomeName      = FName("Grassland");
    Grassland.MinTemperature = 15.0f;
    Grassland.MaxTemperature = 35.0f;
    Grassland.Humidity       = 0.4f;
    Grassland.FoliageDensity = 0.5f;
    Grassland.bHasPredators  = true;
    RegisteredBiomes.Add(Grassland);

    FEng_BiomeDefinition Forest;
    Forest.BiomeID        = EEng_BiomeType::Forest;
    Forest.BiomeName      = FName("Forest");
    Forest.MinTemperature = 18.0f;
    Forest.MaxTemperature = 28.0f;
    Forest.Humidity       = 0.75f;
    Forest.FoliageDensity = 0.9f;
    Forest.bHasPredators  = true;
    RegisteredBiomes.Add(Forest);

    FEng_BiomeDefinition Desert;
    Desert.BiomeID        = EEng_BiomeType::Desert;
    Desert.BiomeName      = FName("Desert");
    Desert.MinTemperature = 30.0f;
    Desert.MaxTemperature = 55.0f;
    Desert.Humidity       = 0.05f;
    Desert.FoliageDensity = 0.1f;
    Desert.bHasPredators  = false;
    RegisteredBiomes.Add(Desert);

    FEng_BiomeDefinition Swamp;
    Swamp.BiomeID        = EEng_BiomeType::Swamp;
    Swamp.BiomeName      = FName("Swamp");
    Swamp.MinTemperature = 22.0f;
    Swamp.MaxTemperature = 32.0f;
    Swamp.Humidity       = 0.95f;
    Swamp.FoliageDensity = 0.8f;
    Swamp.bHasPredators  = true;
    RegisteredBiomes.Add(Swamp);

    FEng_BiomeDefinition Volcanic;
    Volcanic.BiomeID        = EEng_BiomeType::Volcanic;
    Volcanic.BiomeName      = FName("Volcanic");
    Volcanic.MinTemperature = 40.0f;
    Volcanic.MaxTemperature = 80.0f;
    Volcanic.Humidity       = 0.1f;
    Volcanic.FoliageDensity = 0.05f;
    Volcanic.bHasPredators  = false;
    RegisteredBiomes.Add(Volcanic);

    FEng_BiomeDefinition Coastal;
    Coastal.BiomeID        = EEng_BiomeType::Coastal;
    Coastal.BiomeName      = FName("Coastal");
    Coastal.MinTemperature = 20.0f;
    Coastal.MaxTemperature = 30.0f;
    Coastal.Humidity       = 0.65f;
    Coastal.FoliageDensity = 0.4f;
    Coastal.bHasPredators  = true;
    RegisteredBiomes.Add(Coastal);

    ActiveBiome = EEng_BiomeType::Grassland;
}

// ============================================================
// Initialization
// ============================================================

void UBiomeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Initialized with %d biomes"), RegisteredBiomes.Num());
}

void UBiomeManager::Deinitialize()
{
    RegisteredBiomes.Empty();
    Super::Deinitialize();
}

// ============================================================
// Core API
// ============================================================

EEng_BiomeType UBiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    // Simple sector-based biome assignment using world coordinates.
    // In production this will be driven by a noise-based biome map.
    const float X = WorldLocation.X;
    const float Y = WorldLocation.Y;

    if (X > 5000.0f)  return EEng_BiomeType::Volcanic;
    if (X < -5000.0f) return EEng_BiomeType::Desert;
    if (Y > 5000.0f)  return EEng_BiomeType::Coastal;
    if (Y < -5000.0f) return EEng_BiomeType::Swamp;
    if (FMath::Abs(X) < 2000.0f && FMath::Abs(Y) < 2000.0f) return EEng_BiomeType::Forest;

    return EEng_BiomeType::Grassland;
}

FEng_BiomeDefinition UBiomeManager::GetBiomeDefinition(EEng_BiomeType BiomeType) const
{
    for (const FEng_BiomeDefinition& Def : RegisteredBiomes)
    {
        if (Def.BiomeID == BiomeType)
        {
            return Def;
        }
    }

    // Return default (Grassland) if not found
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: BiomeType %d not found, returning default"), (int32)BiomeType);
    return RegisteredBiomes.Num() > 0 ? RegisteredBiomes[0] : FEng_BiomeDefinition();
}

void UBiomeManager::SetActiveBiome(EEng_BiomeType NewBiome)
{
    if (ActiveBiome != NewBiome)
    {
        EEng_BiomeType OldBiome = ActiveBiome;
        ActiveBiome = NewBiome;

        UE_LOG(LogTemp, Log, TEXT("BiomeManager: Transition %d -> %d"), (int32)OldBiome, (int32)NewBiome);
        OnBiomeChanged.Broadcast(OldBiome, NewBiome);
    }
}

float UBiomeManager::GetTemperatureAtLocation(const FVector& WorldLocation) const
{
    EEng_BiomeType Biome = GetBiomeAtLocation(WorldLocation);
    FEng_BiomeDefinition Def = GetBiomeDefinition(Biome);

    // Interpolate temperature based on altitude (Z)
    const float AltitudeFactor = FMath::Clamp(WorldLocation.Z / 5000.0f, 0.0f, 1.0f);
    const float BaseTemp = FMath::Lerp(Def.MaxTemperature, Def.MinTemperature, AltitudeFactor);

    // Add small noise variation
    const float Noise = FMath::Sin(WorldLocation.X * 0.001f) * FMath::Cos(WorldLocation.Y * 0.001f) * 2.0f;
    return BaseTemp + Noise;
}

float UBiomeManager::GetHumidityAtLocation(const FVector& WorldLocation) const
{
    EEng_BiomeType Biome = GetBiomeAtLocation(WorldLocation);
    FEng_BiomeDefinition Def = GetBiomeDefinition(Biome);
    return Def.Humidity;
}

bool UBiomeManager::IsDangerousZone(const FVector& WorldLocation) const
{
    EEng_BiomeType Biome = GetBiomeAtLocation(WorldLocation);
    FEng_BiomeDefinition Def = GetBiomeDefinition(Biome);
    return Def.bHasPredators;
}

void UBiomeManager::RegisterBiome(const FEng_BiomeDefinition& BiomeDef)
{
    // Remove existing entry for this biome type if present
    RegisteredBiomes.RemoveAll([&](const FEng_BiomeDefinition& Existing)
    {
        return Existing.BiomeID == BiomeDef.BiomeID;
    });

    RegisteredBiomes.Add(BiomeDef);
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Registered biome '%s'"), *BiomeDef.BiomeName.ToString());
}
