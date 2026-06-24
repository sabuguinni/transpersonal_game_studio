// PCGBiomeSystem.cpp
// Agent #05 — Procedural World Generator
// Biome classification and environmental data system for Cretaceous world

#include "PCGBiomeSystem.h"
#include "Math/UnrealMathUtility.h"

// ─── Constructor ─────────────────────────────────────────────────────────────

APCGBiomeSystem::APCGBiomeSystem()
{
    PrimaryActorTick.bCanEverTick = false;

    // Initialize with default Cretaceous biomes at construction
    // (full initialization happens in BeginPlay or via InitializeDefaultBiomes)
}

// ─── BeginPlay ────────────────────────────────────────────────────────────────

void APCGBiomeSystem::BeginPlay()
{
    Super::BeginPlay();

    if (BiomeZones.Num() == 0)
    {
        InitializeDefaultBiomes();
    }
}

// ─── InitializeDefaultBiomes ─────────────────────────────────────────────────

void APCGBiomeSystem::InitializeDefaultBiomes()
{
    BiomeZones.Empty();

    // ── Forest Biome (center-north) ──────────────────────────────────────────
    {
        FWorld_BiomeData Forest;
        Forest.BiomeType        = EWorld_BiomeType::Forest;
        Forest.BiomeName        = TEXT("Cretaceous Forest");
        Forest.MinTemperature   = 22.0f;
        Forest.MaxTemperature   = 32.0f;
        Forest.Humidity         = 0.85f;
        Forest.VegetationDensity = 0.95f;
        Forest.FogDensity       = 0.35f;
        Forest.FogColor         = FLinearColor(0.4f, 0.7f, 0.4f, 1.0f);
        Forest.AmbientColor     = FLinearColor(0.2f, 0.4f, 0.2f, 1.0f);
        Forest.DangerLevel      = 0.6f;
        Forest.ResourceRichness = 0.9f;
        Forest.ZoneCenter       = FVector(0.0f, 0.0f, 0.0f);
        Forest.ZoneRadius       = 6000.0f;
        BiomeZones.Add(Forest);
    }

    // ── Savanna Biome (east) ─────────────────────────────────────────────────
    {
        FWorld_BiomeData Savanna;
        Savanna.BiomeType        = EWorld_BiomeType::Savanna;
        Savanna.BiomeName        = TEXT("Open Savanna");
        Savanna.MinTemperature   = 28.0f;
        Savanna.MaxTemperature   = 42.0f;
        Savanna.Humidity         = 0.30f;
        Savanna.VegetationDensity = 0.35f;
        Savanna.FogDensity       = 0.05f;
        Savanna.FogColor         = FLinearColor(0.8f, 0.7f, 0.4f, 1.0f);
        Savanna.AmbientColor     = FLinearColor(0.6f, 0.5f, 0.2f, 1.0f);
        Savanna.DangerLevel      = 0.75f;
        Savanna.ResourceRichness = 0.5f;
        Savanna.ZoneCenter       = FVector(3000.0f, 0.0f, 0.0f);
        Savanna.ZoneRadius       = 5000.0f;
        BiomeZones.Add(Savanna);
    }

    // ── Swamp Biome (south) ──────────────────────────────────────────────────
    {
        FWorld_BiomeData Swamp;
        Swamp.BiomeType        = EWorld_BiomeType::Swamp;
        Swamp.BiomeName        = TEXT("Coastal Swamp");
        Swamp.MinTemperature   = 24.0f;
        Swamp.MaxTemperature   = 30.0f;
        Swamp.Humidity         = 0.95f;
        Swamp.VegetationDensity = 0.70f;
        Swamp.FogDensity       = 0.55f;
        Swamp.FogColor         = FLinearColor(0.3f, 0.5f, 0.35f, 1.0f);
        Swamp.AmbientColor     = FLinearColor(0.15f, 0.3f, 0.2f, 1.0f);
        Swamp.DangerLevel      = 0.65f;
        Swamp.ResourceRichness = 0.75f;
        Swamp.ZoneCenter       = FVector(0.0f, 3000.0f, 0.0f);
        Swamp.ZoneRadius       = 4500.0f;
        BiomeZones.Add(Swamp);
    }

    // ── Rocky Highlands (west) ───────────────────────────────────────────────
    {
        FWorld_BiomeData Rocky;
        Rocky.BiomeType        = EWorld_BiomeType::Rocky;
        Rocky.BiomeName        = TEXT("Rocky Highlands");
        Rocky.MinTemperature   = 15.0f;
        Rocky.MaxTemperature   = 28.0f;
        Rocky.Humidity         = 0.25f;
        Rocky.VegetationDensity = 0.20f;
        Rocky.FogDensity       = 0.10f;
        Rocky.FogColor         = FLinearColor(0.6f, 0.55f, 0.45f, 1.0f);
        Rocky.AmbientColor     = FLinearColor(0.4f, 0.35f, 0.25f, 1.0f);
        Rocky.DangerLevel      = 0.50f;
        Rocky.ResourceRichness = 0.60f;
        Rocky.ZoneCenter       = FVector(-2000.0f, 1500.0f, 500.0f);
        Rocky.ZoneRadius       = 4000.0f;
        BiomeZones.Add(Rocky);
    }

    // ── Volcanic Badlands (south-east) ───────────────────────────────────────
    {
        FWorld_BiomeData Volcanic;
        Volcanic.BiomeType        = EWorld_BiomeType::Volcanic;
        Volcanic.BiomeName        = TEXT("Volcanic Badlands");
        Volcanic.MinTemperature   = 35.0f;
        Volcanic.MaxTemperature   = 60.0f;
        Volcanic.Humidity         = 0.15f;
        Volcanic.VegetationDensity = 0.05f;
        Volcanic.FogDensity       = 0.40f;
        Volcanic.FogColor         = FLinearColor(0.7f, 0.3f, 0.1f, 1.0f);
        Volcanic.AmbientColor     = FLinearColor(0.5f, 0.15f, 0.05f, 1.0f);
        Volcanic.DangerLevel      = 0.95f;
        Volcanic.ResourceRichness = 0.30f;
        Volcanic.ZoneCenter       = FVector(2000.0f, -2000.0f, 0.0f);
        Volcanic.ZoneRadius       = 3500.0f;
        BiomeZones.Add(Volcanic);
    }

    // ── River Delta (center) ─────────────────────────────────────────────────
    {
        FWorld_BiomeData River;
        River.BiomeType        = EWorld_BiomeType::River;
        River.BiomeName        = TEXT("River Delta");
        River.MinTemperature   = 20.0f;
        River.MaxTemperature   = 28.0f;
        River.Humidity         = 0.90f;
        River.VegetationDensity = 0.80f;
        River.FogDensity       = 0.25f;
        River.FogColor         = FLinearColor(0.4f, 0.6f, 0.7f, 1.0f);
        River.AmbientColor     = FLinearColor(0.2f, 0.4f, 0.5f, 1.0f);
        River.DangerLevel      = 0.45f;
        River.ResourceRichness = 0.95f;
        River.ZoneCenter       = FVector(1000.0f, 1000.0f, 0.0f);
        River.ZoneRadius       = 2500.0f;
        BiomeZones.Add(River);
    }

    UE_LOG(LogTemp, Log, TEXT("PCGBiomeSystem: Initialized %d Cretaceous biome zones"), BiomeZones.Num());
}

// ─── ComputeBlendWeight ───────────────────────────────────────────────────────

float APCGBiomeSystem::ComputeBlendWeight(float Distance, float ZoneRadius) const
{
    if (Distance >= ZoneRadius) return 0.0f;
    if (Distance <= 0.0f) return 1.0f;

    // Smooth falloff using cosine interpolation
    const float NormalizedDist = Distance / ZoneRadius;
    return FMath::Clamp(0.5f * (1.0f + FMath::Cos(NormalizedDist * PI)), 0.0f, 1.0f);
}

// ─── FindTwoClosestBiomes ─────────────────────────────────────────────────────

void APCGBiomeSystem::FindTwoClosestBiomes(const FVector& Location, int32& OutPrimary, int32& OutSecondary, float& OutBlend) const
{
    OutPrimary   = 0;
    OutSecondary = 0;
    OutBlend     = 0.0f;

    if (BiomeZones.Num() == 0) return;

    float BestWeight  = -1.0f;
    float SecondWeight = -1.0f;

    for (int32 i = 0; i < BiomeZones.Num(); ++i)
    {
        const float Dist   = FVector::Dist(Location, BiomeZones[i].ZoneCenter);
        const float Weight = ComputeBlendWeight(Dist, BiomeZones[i].ZoneRadius);

        if (Weight > BestWeight)
        {
            SecondWeight = BestWeight;
            OutSecondary = OutPrimary;
            BestWeight   = Weight;
            OutPrimary   = i;
        }
        else if (Weight > SecondWeight)
        {
            SecondWeight = Weight;
            OutSecondary = i;
        }
    }

    const float TotalWeight = BestWeight + SecondWeight;
    OutBlend = (TotalWeight > KINDA_SMALL_NUMBER) ? (SecondWeight / TotalWeight) : 0.0f;
}

// ─── FindNearestBiomeIndex ────────────────────────────────────────────────────

int32 APCGBiomeSystem::FindNearestBiomeIndex(const FVector& WorldLocation) const
{
    int32 NearestIndex = 0;
    float NearestDist  = TNumericLimits<float>::Max();

    for (int32 i = 0; i < BiomeZones.Num(); ++i)
    {
        const float Dist = FVector::Dist(WorldLocation, BiomeZones[i].ZoneCenter);
        if (Dist < NearestDist)
        {
            NearestDist  = Dist;
            NearestIndex = i;
        }
    }

    return NearestIndex;
}

// ─── QueryBiomeAtLocation ─────────────────────────────────────────────────────

FWorld_BiomeQueryResult APCGBiomeSystem::QueryBiomeAtLocation(const FVector& WorldLocation) const
{
    FWorld_BiomeQueryResult Result;

    if (BiomeZones.Num() == 0)
    {
        Result.bIsValid = false;
        return Result;
    }

    int32 PrimaryIdx   = 0;
    int32 SecondaryIdx = 0;
    float BlendFactor  = 0.0f;
    FindTwoClosestBiomes(WorldLocation, PrimaryIdx, SecondaryIdx, BlendFactor);

    const FWorld_BiomeData& Primary   = BiomeZones[PrimaryIdx];
    const FWorld_BiomeData& Secondary = BiomeZones[SecondaryIdx];

    Result.PrimaryBiome   = Primary.BiomeType;
    Result.SecondaryBiome = Secondary.BiomeType;
    Result.BlendFactor    = BlendFactor;

    // Blend environmental values
    const float PrimaryWeight   = 1.0f - BlendFactor;
    const float SecondaryWeight = BlendFactor;

    const float MidTemp = (Primary.MinTemperature + Primary.MaxTemperature) * 0.5f;
    const float SecMidTemp = (Secondary.MinTemperature + Secondary.MaxTemperature) * 0.5f;
    Result.Temperature  = MidTemp * PrimaryWeight + SecMidTemp * SecondaryWeight;
    Result.Humidity     = Primary.Humidity * PrimaryWeight + Secondary.Humidity * SecondaryWeight;
    Result.DangerLevel  = Primary.DangerLevel * PrimaryWeight + Secondary.DangerLevel * SecondaryWeight;
    Result.bIsValid     = true;

    return Result;
}

// ─── GetBiomeTypeAtLocation ───────────────────────────────────────────────────

EWorld_BiomeType APCGBiomeSystem::GetBiomeTypeAtLocation(const FVector& WorldLocation) const
{
    if (BiomeZones.Num() == 0) return EWorld_BiomeType::Plains;
    const int32 Idx = FindNearestBiomeIndex(WorldLocation);
    return BiomeZones[Idx].BiomeType;
}

// ─── GetTemperatureAtLocation ─────────────────────────────────────────────────

float APCGBiomeSystem::GetTemperatureAtLocation(const FVector& WorldLocation) const
{
    const FWorld_BiomeQueryResult Result = QueryBiomeAtLocation(WorldLocation);
    return Result.bIsValid ? Result.Temperature : 28.0f;
}

// ─── GetDangerLevelAtLocation ─────────────────────────────────────────────────

float APCGBiomeSystem::GetDangerLevelAtLocation(const FVector& WorldLocation) const
{
    const FWorld_BiomeQueryResult Result = QueryBiomeAtLocation(WorldLocation);
    return Result.bIsValid ? Result.DangerLevel : 0.3f;
}

// ─── GetVegetationDensityAtLocation ──────────────────────────────────────────

float APCGBiomeSystem::GetVegetationDensityAtLocation(const FVector& WorldLocation) const
{
    if (BiomeZones.Num() == 0) return 0.5f;

    int32 PrimaryIdx   = 0;
    int32 SecondaryIdx = 0;
    float BlendFactor  = 0.0f;
    FindTwoClosestBiomes(WorldLocation, PrimaryIdx, SecondaryIdx, BlendFactor);

    const float PrimaryDensity   = BiomeZones[PrimaryIdx].VegetationDensity;
    const float SecondaryDensity = BiomeZones[SecondaryIdx].VegetationDensity;
    return FMath::Lerp(PrimaryDensity, SecondaryDensity, BlendFactor);
}
