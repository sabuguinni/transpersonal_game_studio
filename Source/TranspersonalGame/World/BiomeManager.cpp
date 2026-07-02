// BiomeManager.cpp — Agent #05 Procedural World Generator — PROD_CYCLE_AUTO_20260702_002
// Full implementation of 5-biome prehistoric world system

#include "BiomeManager.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

// ─── Constructor ──────────────────────────────────────────────────────────────
ABiomeManager::ABiomeManager()
{
    PrimaryActorTick.bCanEverTick = false;
    bAutoInitializeBiomes = true;
}

// ─── BeginPlay ────────────────────────────────────────────────────────────────
void ABiomeManager::BeginPlay()
{
    Super::BeginPlay();

    if (bAutoInitializeBiomes && BiomeZones.Num() == 0)
    {
        InitializeDefaultBiomes();
    }
}

// ─── InitializeDefaultBiomes ─────────────────────────────────────────────────
void ABiomeManager::InitializeDefaultBiomes()
{
    BiomeZones.Empty();

    // Forest Zone — dense jungle, high vegetation, moderate danger
    FWorld_BiomeZone Forest;
    Forest.BiomeType        = EWorld_BiomeType::Forest;
    Forest.CenterLocation   = FVector(-8000.0f, -8000.0f, 0.0f);
    Forest.Radius           = 6000.0f;
    Forest.DebugColor       = FLinearColor(0.05f, 0.8f, 0.05f, 1.0f);
    Forest.VegetationDensity = 2.5f;
    Forest.WaterPresence    = 0.3f;
    Forest.DangerLevel      = 0.6f;
    BiomeZones.Add(Forest);

    // Plains Zone — open grassland, low vegetation, low danger
    FWorld_BiomeZone Plains;
    Plains.BiomeType        = EWorld_BiomeType::Plains;
    Plains.CenterLocation   = FVector(8000.0f, 0.0f, 0.0f);
    Plains.Radius           = 7000.0f;
    Plains.DebugColor       = FLinearColor(0.8f, 0.8f, 0.1f, 1.0f);
    Plains.VegetationDensity = 0.8f;
    Plains.WaterPresence    = 0.1f;
    Plains.DangerLevel      = 0.3f;
    BiomeZones.Add(Plains);

    // Rocky Zone — cliffs and boulders, sparse vegetation, moderate danger
    FWorld_BiomeZone Rocky;
    Rocky.BiomeType         = EWorld_BiomeType::Rocky;
    Rocky.CenterLocation    = FVector(0.0f, 8000.0f, 0.0f);
    Rocky.Radius            = 5500.0f;
    Rocky.DebugColor        = FLinearColor(0.5f, 0.35f, 0.2f, 1.0f);
    Rocky.VegetationDensity = 0.3f;
    Rocky.WaterPresence     = 0.05f;
    Rocky.DangerLevel       = 0.7f;
    BiomeZones.Add(Rocky);

    // Swamp Zone — murky wetlands, dense low vegetation, high danger
    FWorld_BiomeZone Swamp;
    Swamp.BiomeType         = EWorld_BiomeType::Swamp;
    Swamp.CenterLocation    = FVector(-8000.0f, 8000.0f, -50.0f);
    Swamp.Radius            = 5000.0f;
    Swamp.DebugColor        = FLinearColor(0.1f, 0.4f, 0.1f, 1.0f);
    Swamp.VegetationDensity = 1.8f;
    Swamp.WaterPresence     = 0.75f;
    Swamp.DangerLevel       = 0.8f;
    BiomeZones.Add(Swamp);

    // Volcanic Zone — lava fields, near-zero vegetation, extreme danger
    FWorld_BiomeZone Volcanic;
    Volcanic.BiomeType         = EWorld_BiomeType::Volcanic;
    Volcanic.CenterLocation    = FVector(0.0f, -10000.0f, 100.0f);
    Volcanic.Radius            = 4500.0f;
    Volcanic.DebugColor        = FLinearColor(1.0f, 0.15f, 0.02f, 1.0f);
    Volcanic.VegetationDensity = 0.05f;
    Volcanic.WaterPresence     = 0.0f;
    Volcanic.DangerLevel       = 1.0f;
    BiomeZones.Add(Volcanic);

    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Initialized %d biome zones"), BiomeZones.Num());
}

// ─── RegisterBiomeZone ────────────────────────────────────────────────────────
void ABiomeManager::RegisterBiomeZone(const FWorld_BiomeZone& Zone)
{
    // Remove existing zone of same type if present
    BiomeZones.RemoveAll([&Zone](const FWorld_BiomeZone& Existing)
    {
        return Existing.BiomeType == Zone.BiomeType;
    });
    BiomeZones.Add(Zone);
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Registered biome zone type=%d"), (int32)Zone.BiomeType);
}

// ─── FindClosestBiomeIndex ────────────────────────────────────────────────────
int32 ABiomeManager::FindClosestBiomeIndex(const FVector& WorldLocation) const
{
    int32 ClosestIndex = -1;
    float ClosestDistSq = FLT_MAX;

    for (int32 i = 0; i < BiomeZones.Num(); ++i)
    {
        const float DistSq = FVector::DistSquaredXY(WorldLocation, BiomeZones[i].CenterLocation);
        if (DistSq < ClosestDistSq)
        {
            ClosestDistSq = DistSq;
            ClosestIndex  = i;
        }
    }
    return ClosestIndex;
}

// ─── GetBiomeAtLocation ───────────────────────────────────────────────────────
EWorld_BiomeType ABiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    if (BiomeZones.Num() == 0) return EWorld_BiomeType::Unknown;

    // First check if location is within any biome radius
    for (const FWorld_BiomeZone& Zone : BiomeZones)
    {
        const float DistXY = FVector::DistXY(WorldLocation, Zone.CenterLocation);
        if (DistXY <= Zone.Radius)
        {
            return Zone.BiomeType;
        }
    }

    // Fallback: return closest biome type
    const int32 Idx = FindClosestBiomeIndex(WorldLocation);
    return (Idx >= 0) ? BiomeZones[Idx].BiomeType : EWorld_BiomeType::Unknown;
}

// ─── GetBiomeZoneData ─────────────────────────────────────────────────────────
FWorld_BiomeZone ABiomeManager::GetBiomeZoneData(EWorld_BiomeType BiomeType) const
{
    for (const FWorld_BiomeZone& Zone : BiomeZones)
    {
        if (Zone.BiomeType == BiomeType) return Zone;
    }
    return FWorld_BiomeZone(); // Empty/default
}

// ─── GetDangerLevelAtLocation ─────────────────────────────────────────────────
float ABiomeManager::GetDangerLevelAtLocation(const FVector& WorldLocation) const
{
    const EWorld_BiomeType Biome = GetBiomeAtLocation(WorldLocation);
    const FWorld_BiomeZone Zone  = GetBiomeZoneData(Biome);
    return Zone.DangerLevel;
}

// ─── GetVegetationDensityAtLocation ──────────────────────────────────────────
float ABiomeManager::GetVegetationDensityAtLocation(const FVector& WorldLocation) const
{
    const EWorld_BiomeType Biome = GetBiomeAtLocation(WorldLocation);
    const FWorld_BiomeZone Zone  = GetBiomeZoneData(Biome);
    return Zone.VegetationDensity;
}

// ─── IsLocationInWater ────────────────────────────────────────────────────────
bool ABiomeManager::IsLocationInWater(const FVector& WorldLocation) const
{
    const EWorld_BiomeType Biome = GetBiomeAtLocation(WorldLocation);
    const FWorld_BiomeZone Zone  = GetBiomeZoneData(Biome);
    // Consider "in water" if water presence > 0.5 and location is below zone center Z
    return (Zone.WaterPresence > 0.5f && WorldLocation.Z <= Zone.CenterLocation.Z + 10.0f);
}

// ─── DrawBiomeDebugSpheres ────────────────────────────────────────────────────
void ABiomeManager::DrawBiomeDebugSpheres(float Duration)
{
    UWorld* World = GetWorld();
    if (!World) return;

    for (const FWorld_BiomeZone& Zone : BiomeZones)
    {
        const FColor Color = Zone.DebugColor.ToFColor(true);
        DrawDebugSphere(World, Zone.CenterLocation, Zone.Radius, 24, Color, false, Duration, 0, 50.0f);
        DrawDebugString(World, Zone.CenterLocation + FVector(0, 0, Zone.Radius + 200.0f),
            FString::Printf(TEXT("Biome: %d | Danger: %.1f | Veg: %.1f"),
                (int32)Zone.BiomeType, Zone.DangerLevel, Zone.VegetationDensity),
            nullptr, Color, Duration);
    }
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Drew debug spheres for %d zones"), BiomeZones.Num());
}
