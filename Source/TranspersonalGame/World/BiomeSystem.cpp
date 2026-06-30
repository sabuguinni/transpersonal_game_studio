// BiomeSystem.cpp — Biome zone management for Transpersonal Game
// Defines 4 Cretaceous biomes: Forest (pond), Plains (south), Rocky (north), Riverbank

#include "BiomeSystem.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

ABiomeSystem::ABiomeSystem()
{
    PrimaryActorTick.bCanEverTick = false;
    SetActorLabel(TEXT("BiomeSystemManager"));
}

void ABiomeSystem::BeginPlay()
{
    Super::BeginPlay();

    // Auto-register default biomes if none configured
    if (BiomeZones.Num() == 0)
    {
        RegisterDefaultBiomes();
    }
}

void ABiomeSystem::RegisterDefaultBiomes()
{
    BiomeZones.Empty();

    // ── Forest Biome — around pond/water source ──────────────────────────────
    FWorld_BiomeZone Forest;
    Forest.BiomeName = TEXT("Cretaceous Forest");
    Forest.CenterXY = FVector2D(8000.0f, 0.0f);
    Forest.Radius = 2000.0f;
    Forest.VegetationDensity = 2.5f;
    Forest.RockDensity = 0.3f;
    Forest.bHasWater = true;
    Forest.BiomeTint = FLinearColor(0.05f, 0.45f, 0.05f, 1.0f);
    BiomeZones.Add(Forest);

    // ── Plains Biome — south of river ────────────────────────────────────────
    FWorld_BiomeZone Plains;
    Plains.BiomeName = TEXT("Open Plains");
    Plains.CenterXY = FVector2D(5000.0f, -2000.0f);
    Plains.Radius = 3500.0f;
    Plains.VegetationDensity = 0.8f;
    Plains.RockDensity = 0.1f;
    Plains.bHasWater = false;
    Plains.BiomeTint = FLinearColor(0.55f, 0.70f, 0.15f, 1.0f);
    BiomeZones.Add(Plains);

    // ── Rocky Highland — north cluster ───────────────────────────────────────
    FWorld_BiomeZone Rocky;
    Rocky.BiomeName = TEXT("Rocky Highland");
    Rocky.CenterXY = FVector2D(9700.0f, 2600.0f);
    Rocky.Radius = 1800.0f;
    Rocky.VegetationDensity = 0.2f;
    Rocky.RockDensity = 3.0f;
    Rocky.bHasWater = false;
    Rocky.BiomeTint = FLinearColor(0.45f, 0.40f, 0.30f, 1.0f);
    BiomeZones.Add(Rocky);

    // ── Riverbank Biome — along river corridor ───────────────────────────────
    FWorld_BiomeZone Riverbank;
    Riverbank.BiomeName = TEXT("Riverbank");
    Riverbank.CenterXY = FVector2D(4000.0f, 0.0f);
    Riverbank.Radius = 4500.0f;
    Riverbank.VegetationDensity = 1.5f;
    Riverbank.RockDensity = 0.4f;
    Riverbank.bHasWater = true;
    Riverbank.BiomeTint = FLinearColor(0.10f, 0.55f, 0.35f, 1.0f);
    BiomeZones.Add(Riverbank);

    UE_LOG(LogTemp, Log, TEXT("BiomeSystem: Registered %d biomes"), BiomeZones.Num());
}

EWorld_BiomeType ABiomeSystem::GetBiomeAtLocation(FVector WorldLocation) const
{
    FWorld_BiomeZone ClosestZone;
    if (GetBiomeZoneAtLocation(WorldLocation, ClosestZone))
    {
        if (ClosestZone.BiomeName.Contains(TEXT("Forest")))   return EWorld_BiomeType::Forest;
        if (ClosestZone.BiomeName.Contains(TEXT("Plains")))   return EWorld_BiomeType::Plains;
        if (ClosestZone.BiomeName.Contains(TEXT("Rocky")))    return EWorld_BiomeType::Rocky;
        if (ClosestZone.BiomeName.Contains(TEXT("River")))    return EWorld_BiomeType::Riverbank;
        if (ClosestZone.BiomeName.Contains(TEXT("Volcanic"))) return EWorld_BiomeType::Volcanic;
        if (ClosestZone.BiomeName.Contains(TEXT("Swamp")))    return EWorld_BiomeType::Swamp;
    }
    return EWorld_BiomeType::Plains; // Default fallback
}

bool ABiomeSystem::GetBiomeZoneAtLocation(FVector WorldLocation, FWorld_BiomeZone& OutZone) const
{
    if (BiomeZones.Num() == 0)
    {
        return false;
    }

    float ClosestDist = TNumericLimits<float>::Max();
    int32 ClosestIndex = -1;
    FVector2D Loc2D(WorldLocation.X, WorldLocation.Y);

    for (int32 i = 0; i < BiomeZones.Num(); ++i)
    {
        float Dist = FVector2D::Distance(Loc2D, BiomeZones[i].CenterXY);
        if (Dist < ClosestDist && Dist <= BiomeZones[i].Radius)
        {
            ClosestDist = Dist;
            ClosestIndex = i;
        }
    }

    if (ClosestIndex >= 0)
    {
        OutZone = BiomeZones[ClosestIndex];
        return true;
    }

    // Return closest even if outside radius
    for (int32 i = 0; i < BiomeZones.Num(); ++i)
    {
        float Dist = FVector2D::Distance(Loc2D, BiomeZones[i].CenterXY);
        if (Dist < ClosestDist)
        {
            ClosestDist = Dist;
            ClosestIndex = i;
        }
    }

    if (ClosestIndex >= 0)
    {
        OutZone = BiomeZones[ClosestIndex];
        return true;
    }

    return false;
}

float ABiomeSystem::GetVegetationDensityAt(FVector WorldLocation) const
{
    FWorld_BiomeZone Zone;
    if (GetBiomeZoneAtLocation(WorldLocation, Zone))
    {
        // Blend density based on distance from zone center
        FVector2D Loc2D(WorldLocation.X, WorldLocation.Y);
        float Dist = FVector2D::Distance(Loc2D, Zone.CenterXY);
        float BlendFactor = FMath::Clamp(1.0f - (Dist / Zone.Radius), 0.0f, 1.0f);
        return Zone.VegetationDensity * BlendFactor;
    }
    return 0.5f; // Default density
}

bool ABiomeSystem::IsNearWater(FVector WorldLocation, float SearchRadius) const
{
    FVector2D Loc2D(WorldLocation.X, WorldLocation.Y);

    for (const FWorld_BiomeZone& Zone : BiomeZones)
    {
        if (!Zone.bHasWater) continue;

        float Dist = FVector2D::Distance(Loc2D, Zone.CenterXY);
        if (Dist <= SearchRadius)
        {
            return true;
        }
    }
    return false;
}

void ABiomeSystem::DrawBiomeDebug(float Duration) const
{
    UWorld* World = GetWorld();
    if (!World) return;

    for (const FWorld_BiomeZone& Zone : BiomeZones)
    {
        FVector Center(Zone.CenterXY.X, Zone.CenterXY.Y, 100.0f);
        FColor DebugColor = FColor(
            FMath::Clamp((int32)(Zone.BiomeTint.R * 255), 0, 255),
            FMath::Clamp((int32)(Zone.BiomeTint.G * 255), 0, 255),
            FMath::Clamp((int32)(Zone.BiomeTint.B * 255), 0, 255)
        );

        DrawDebugSphere(World, Center, Zone.Radius, 32, DebugColor, false, Duration, 0, 5.0f);
        DrawDebugString(World, Center + FVector(0, 0, 200), Zone.BiomeName, nullptr, DebugColor, Duration);

        UE_LOG(LogTemp, Log, TEXT("BiomeDebug: %s at (%.0f, %.0f) r=%.0f water=%s"),
            *Zone.BiomeName,
            Zone.CenterXY.X, Zone.CenterXY.Y,
            Zone.Radius,
            Zone.bHasWater ? TEXT("YES") : TEXT("NO"));
    }
}
