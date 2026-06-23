// BiomeManager.cpp
// Engine Architect #02 — Transpersonal Game Studio
// Full implementation of the Biome Manager system for the prehistoric survival game.
// Manages 6 biomes: Grassland, Forest, Desert, Swamp, Mountain, Volcanic

#include "BiomeManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

// ─── Constructor ────────────────────────────────────────────────────────────

UBiomeManager::UBiomeManager()
{
    // Default biome definitions — prehistoric survival world
    InitializeDefaultBiomes();
}

// ─── UGameInstanceSubsystem ─────────────────────────────────────────────────

void UBiomeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    InitializeDefaultBiomes();
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Initialized with %d biomes"), RegisteredBiomes.Num());
}

void UBiomeManager::Deinitialize()
{
    RegisteredBiomes.Empty();
    Super::Deinitialize();
}

// ─── Biome Registration ──────────────────────────────────────────────────────

void UBiomeManager::RegisterBiome(const FEng_BiomeDefinition& BiomeDef)
{
    // Prevent duplicate IDs
    for (const FEng_BiomeDefinition& Existing : RegisteredBiomes)
    {
        if (Existing.BiomeID == BiomeDef.BiomeID)
        {
            UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Biome ID '%s' already registered — skipping."), *BiomeDef.BiomeID.ToString());
            return;
        }
    }
    RegisteredBiomes.Add(BiomeDef);
    OnBiomeRegistered.Broadcast(BiomeDef.BiomeID);
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Registered biome '%s'"), *BiomeDef.BiomeName);
}

bool UBiomeManager::GetBiomeByID(FName BiomeID, FEng_BiomeDefinition& OutBiome) const
{
    for (const FEng_BiomeDefinition& Biome : RegisteredBiomes)
    {
        if (Biome.BiomeID == BiomeID)
        {
            OutBiome = Biome;
            return true;
        }
    }
    return false;
}

// ─── World Position Query ────────────────────────────────────────────────────

FName UBiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    // Simple distance-based biome lookup using biome origin points
    FName ClosestBiome = NAME_None;
    float ClosestDist = MAX_FLT;

    for (const FEng_BiomeDefinition& Biome : RegisteredBiomes)
    {
        float Dist = FVector::Dist2D(WorldLocation, Biome.BiomeOrigin);
        if (Dist < Biome.BiomeRadius && Dist < ClosestDist)
        {
            ClosestDist = Dist;
            ClosestBiome = Biome.BiomeID;
        }
    }

    // Fallback: return nearest biome origin regardless of radius
    if (ClosestBiome == NAME_None)
    {
        for (const FEng_BiomeDefinition& Biome : RegisteredBiomes)
        {
            float Dist = FVector::Dist2D(WorldLocation, Biome.BiomeOrigin);
            if (Dist < ClosestDist)
            {
                ClosestDist = Dist;
                ClosestBiome = Biome.BiomeID;
            }
        }
    }

    return ClosestBiome;
}

FEng_BiomeEnvironment UBiomeManager::GetEnvironmentAtLocation(const FVector& WorldLocation) const
{
    FName BiomeID = GetBiomeAtLocation(WorldLocation);
    FEng_BiomeDefinition Def;
    if (GetBiomeByID(BiomeID, Def))
    {
        return Def.Environment;
    }
    // Return default grassland environment
    FEng_BiomeEnvironment Default;
    Default.AmbientTemperatureCelsius = 22.0f;
    Default.HumidityPercent = 60.0f;
    Default.FogDensity = 0.02f;
    Default.WindSpeedKmH = 10.0f;
    return Default;
}

TArray<FName> UBiomeManager::GetAllBiomeIDs() const
{
    TArray<FName> IDs;
    for (const FEng_BiomeDefinition& Biome : RegisteredBiomes)
    {
        IDs.Add(Biome.BiomeID);
    }
    return IDs;
}

// ─── Survival Queries ────────────────────────────────────────────────────────

float UBiomeManager::GetTemperatureAtLocation(const FVector& WorldLocation) const
{
    FEng_BiomeEnvironment Env = GetEnvironmentAtLocation(WorldLocation);
    return Env.AmbientTemperatureCelsius;
}

float UBiomeManager::GetHazardLevelAtLocation(const FVector& WorldLocation) const
{
    FName BiomeID = GetBiomeAtLocation(WorldLocation);
    FEng_BiomeDefinition Def;
    if (GetBiomeByID(BiomeID, Def))
    {
        return Def.HazardLevel;
    }
    return 0.0f;
}

bool UBiomeManager::IsLocationDangerous(const FVector& WorldLocation, float DangerThreshold) const
{
    return GetHazardLevelAtLocation(WorldLocation) >= DangerThreshold;
}

// ─── Default Biome Initialization ───────────────────────────────────────────

void UBiomeManager::InitializeDefaultBiomes()
{
    RegisteredBiomes.Empty();

    // ── 1. GRASSLAND ──────────────────────────────────────────────────────
    {
        FEng_BiomeDefinition Grassland;
        Grassland.BiomeID = FName("Grassland");
        Grassland.BiomeName = TEXT("Cretaceous Grassland");
        Grassland.BiomeOrigin = FVector(0.0f, 0.0f, 0.0f);
        Grassland.BiomeRadius = 5000.0f;
        Grassland.HazardLevel = 0.2f;
        Grassland.Environment.AmbientTemperatureCelsius = 24.0f;
        Grassland.Environment.HumidityPercent = 55.0f;
        Grassland.Environment.FogDensity = 0.01f;
        Grassland.Environment.WindSpeedKmH = 12.0f;
        Grassland.Environment.SkyColorTint = FLinearColor(0.53f, 0.81f, 0.98f);
        Grassland.Environment.FogColorTint = FLinearColor(0.9f, 0.95f, 1.0f);
        RegisteredBiomes.Add(Grassland);
    }

    // ── 2. FOREST ─────────────────────────────────────────────────────────
    {
        FEng_BiomeDefinition Forest;
        Forest.BiomeID = FName("Forest");
        Forest.BiomeName = TEXT("Prehistoric Dense Forest");
        Forest.BiomeOrigin = FVector(6000.0f, 0.0f, 0.0f);
        Forest.BiomeRadius = 4500.0f;
        Forest.HazardLevel = 0.45f;
        Forest.Environment.AmbientTemperatureCelsius = 20.0f;
        Forest.Environment.HumidityPercent = 80.0f;
        Forest.Environment.FogDensity = 0.04f;
        Forest.Environment.WindSpeedKmH = 4.0f;
        Forest.Environment.SkyColorTint = FLinearColor(0.3f, 0.6f, 0.3f);
        Forest.Environment.FogColorTint = FLinearColor(0.6f, 0.8f, 0.6f);
        RegisteredBiomes.Add(Forest);
    }

    // ── 3. DESERT ─────────────────────────────────────────────────────────
    {
        FEng_BiomeDefinition Desert;
        Desert.BiomeID = FName("Desert");
        Desert.BiomeName = TEXT("Arid Badlands");
        Desert.BiomeOrigin = FVector(-6000.0f, 0.0f, 0.0f);
        Desert.BiomeRadius = 4000.0f;
        Desert.HazardLevel = 0.6f;
        Desert.Environment.AmbientTemperatureCelsius = 42.0f;
        Desert.Environment.HumidityPercent = 10.0f;
        Desert.Environment.FogDensity = 0.005f;
        Desert.Environment.WindSpeedKmH = 25.0f;
        Desert.Environment.SkyColorTint = FLinearColor(0.9f, 0.75f, 0.5f);
        Desert.Environment.FogColorTint = FLinearColor(0.95f, 0.85f, 0.6f);
        RegisteredBiomes.Add(Desert);
    }

    // ── 4. SWAMP ──────────────────────────────────────────────────────────
    {
        FEng_BiomeDefinition Swamp;
        Swamp.BiomeID = FName("Swamp");
        Swamp.BiomeName = TEXT("Primordial Swamp");
        Swamp.BiomeOrigin = FVector(0.0f, 6000.0f, 0.0f);
        Swamp.BiomeRadius = 3500.0f;
        Swamp.HazardLevel = 0.55f;
        Swamp.Environment.AmbientTemperatureCelsius = 30.0f;
        Swamp.Environment.HumidityPercent = 95.0f;
        Swamp.Environment.FogDensity = 0.08f;
        Swamp.Environment.WindSpeedKmH = 2.0f;
        Swamp.Environment.SkyColorTint = FLinearColor(0.4f, 0.5f, 0.3f);
        Swamp.Environment.FogColorTint = FLinearColor(0.5f, 0.6f, 0.4f);
        RegisteredBiomes.Add(Swamp);
    }

    // ── 5. MOUNTAIN ───────────────────────────────────────────────────────
    {
        FEng_BiomeDefinition Mountain;
        Mountain.BiomeID = FName("Mountain");
        Mountain.BiomeName = TEXT("Rocky Highlands");
        Mountain.BiomeOrigin = FVector(0.0f, -6000.0f, 500.0f);
        Mountain.BiomeRadius = 4000.0f;
        Mountain.HazardLevel = 0.5f;
        Mountain.Environment.AmbientTemperatureCelsius = 8.0f;
        Mountain.Environment.HumidityPercent = 40.0f;
        Mountain.Environment.FogDensity = 0.03f;
        Mountain.Environment.WindSpeedKmH = 35.0f;
        Mountain.Environment.SkyColorTint = FLinearColor(0.6f, 0.7f, 0.9f);
        Mountain.Environment.FogColorTint = FLinearColor(0.8f, 0.85f, 0.95f);
        RegisteredBiomes.Add(Mountain);
    }

    // ── 6. VOLCANIC ───────────────────────────────────────────────────────
    {
        FEng_BiomeDefinition Volcanic;
        Volcanic.BiomeID = FName("Volcanic");
        Volcanic.BiomeName = TEXT("Volcanic Caldera");
        Volcanic.BiomeOrigin = FVector(4000.0f, -4000.0f, 200.0f);
        Volcanic.BiomeRadius = 3000.0f;
        Volcanic.HazardLevel = 0.9f;
        Volcanic.Environment.AmbientTemperatureCelsius = 65.0f;
        Volcanic.Environment.HumidityPercent = 20.0f;
        Volcanic.Environment.FogDensity = 0.12f;
        Volcanic.Environment.WindSpeedKmH = 18.0f;
        Volcanic.Environment.SkyColorTint = FLinearColor(0.8f, 0.4f, 0.2f);
        Volcanic.Environment.FogColorTint = FLinearColor(0.7f, 0.3f, 0.1f);
        RegisteredBiomes.Add(Volcanic);
    }

    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Default biomes initialized (%d total)"), RegisteredBiomes.Num());
}
