#include "PCGBiomeSystem.h"
#include "DrawDebugHelpers.h"

APCGBiomeSystem::APCGBiomeSystem()
{
    PrimaryActorTick.bCanEverTick = false;
    InitializeDefaultZones();
}

void APCGBiomeSystem::BeginPlay()
{
    Super::BeginPlay();
    // Zones already initialized in constructor — nothing else needed at runtime
}

void APCGBiomeSystem::InitializeDefaultZones()
{
    BiomeZones.Empty();

    // --- Forest — NW quadrant ---
    FWorld_BiomeZone Forest;
    Forest.BiomeType = EWorld_BiomeType::Forest;
    Forest.Center = FVector(-2000.0f, -2000.0f, 0.0f);
    Forest.Radius = 2500.0f;
    Forest.AmbientTemperature = 22.0f;
    Forest.Humidity = 0.85f;
    Forest.DangerLevel = 0.4f;
    Forest.FoliageDensity = 2.0f;
    BiomeZones.Add(Forest);

    // --- Plains — NE quadrant ---
    FWorld_BiomeZone Plains;
    Plains.BiomeType = EWorld_BiomeType::Plains;
    Plains.Center = FVector(2000.0f, -2000.0f, 0.0f);
    Plains.Radius = 2500.0f;
    Plains.AmbientTemperature = 32.0f;
    Plains.Humidity = 0.25f;
    Plains.DangerLevel = 0.6f;   // Open ground = more predator exposure
    Plains.FoliageDensity = 0.5f;
    BiomeZones.Add(Plains);

    // --- Highlands — SE quadrant ---
    FWorld_BiomeZone Highlands;
    Highlands.BiomeType = EWorld_BiomeType::Highlands;
    Highlands.Center = FVector(2000.0f, 2000.0f, 0.0f);
    Highlands.Radius = 2500.0f;
    Highlands.AmbientTemperature = 15.0f;
    Highlands.Humidity = 0.30f;
    Highlands.DangerLevel = 0.5f;
    Highlands.FoliageDensity = 0.3f;
    BiomeZones.Add(Highlands);

    // --- Wetlands — SW quadrant ---
    FWorld_BiomeZone Wetlands;
    Wetlands.BiomeType = EWorld_BiomeType::Wetlands;
    Wetlands.Center = FVector(-2000.0f, 2000.0f, 0.0f);
    Wetlands.Radius = 2500.0f;
    Wetlands.AmbientTemperature = 28.0f;
    Wetlands.Humidity = 0.95f;
    Wetlands.DangerLevel = 0.35f;
    Wetlands.FoliageDensity = 1.5f;
    BiomeZones.Add(Wetlands);
}

EWorld_BiomeType APCGBiomeSystem::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    float BestDist = MAX_FLT;
    EWorld_BiomeType BestBiome = EWorld_BiomeType::Undefined;

    for (const FWorld_BiomeZone& Zone : BiomeZones)
    {
        float Dist = FVector::Dist2D(WorldLocation, Zone.Center);
        if (Dist <= Zone.Radius && Dist < BestDist)
        {
            BestDist = Dist;
            BestBiome = Zone.BiomeType;
        }
    }

    return BestBiome;
}

FWorld_BiomeZone APCGBiomeSystem::GetBiomeZoneAtLocation(const FVector& WorldLocation) const
{
    float BestDist = MAX_FLT;
    const FWorld_BiomeZone* BestZone = nullptr;

    for (const FWorld_BiomeZone& Zone : BiomeZones)
    {
        float Dist = FVector::Dist2D(WorldLocation, Zone.Center);
        if (Dist <= Zone.Radius && Dist < BestDist)
        {
            BestDist = Dist;
            BestZone = &Zone;
        }
    }

    return BestZone ? *BestZone : FWorld_BiomeZone();
}

EWorld_BiomeType APCGBiomeSystem::GetNearestBiome(const FVector& WorldLocation) const
{
    float BestDist = MAX_FLT;
    EWorld_BiomeType BestBiome = EWorld_BiomeType::Undefined;

    for (const FWorld_BiomeZone& Zone : BiomeZones)
    {
        float Dist = FVector::Dist2D(WorldLocation, Zone.Center);
        if (Dist < BestDist)
        {
            BestDist = Dist;
            BestBiome = Zone.BiomeType;
        }
    }

    return BestBiome;
}

void APCGBiomeSystem::DrawBiomeDebug()
{
#if WITH_EDITOR
    UWorld* World = GetWorld();
    if (!World) return;

    const TMap<EWorld_BiomeType, FColor> BiomeColors = {
        { EWorld_BiomeType::Forest,    FColor::Green  },
        { EWorld_BiomeType::Plains,    FColor::Yellow },
        { EWorld_BiomeType::Highlands, FColor(139, 90, 43) },  // Brown
        { EWorld_BiomeType::Wetlands,  FColor::Blue   },
    };

    for (const FWorld_BiomeZone& Zone : BiomeZones)
    {
        const FColor* ColorPtr = BiomeColors.Find(Zone.BiomeType);
        FColor Color = ColorPtr ? *ColorPtr : FColor::White;

        // Draw circle at ground level
        DrawDebugCircle(
            World,
            Zone.Center,
            Zone.Radius,
            64,
            Color,
            true,   // persistent
            -1.0f,  // lifetime
            0,
            50.0f,  // thickness
            FVector(1, 0, 0),
            FVector(0, 1, 0)
        );

        // Draw label sphere at center
        DrawDebugSphere(World, Zone.Center + FVector(0, 0, 200), 100.0f, 8, Color, true);
    }
#endif
}
