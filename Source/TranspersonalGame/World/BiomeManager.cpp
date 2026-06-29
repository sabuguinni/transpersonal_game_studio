#include "BiomeManager.h"
#include "DrawDebugHelpers.h"
#include "Math/UnrealMathUtility.h"

ABiomeManager::ABiomeManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f; // Check biomes once per second max
}

void ABiomeManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeBiomeData();
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Initialized with %d biome zones, %d biome types"),
        BiomeZones.Num(), BiomeDataMap.Num());
}

void ABiomeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bShowDebugBiomes)
    {
        DrawBiomeBoundaries();
    }
}

void ABiomeManager::InitializeBiomeData()
{
    // Forest biome — dense Cretaceous jungle
    FEng_BiomeData ForestData;
    ForestData.BiomeType = EEng_BiomeType::Forest;
    ForestData.BiomeName = TEXT("Cretaceous Forest");
    ForestData.Temperature = 28.0f;
    ForestData.Humidity = 0.85f;
    ForestData.FoliageDensity = 0.9f;
    ForestData.DinosaurSpawnWeight = 1.2f;
    ForestData.BiomeDebugColor = FLinearColor(0.0f, 0.8f, 0.2f, 1.0f);
    BiomeDataMap.Add(EEng_BiomeType::Forest, ForestData);

    // Savanna biome — open grassland with scattered trees
    FEng_BiomeData SavannaData;
    SavannaData.BiomeType = EEng_BiomeType::Savanna;
    SavannaData.BiomeName = TEXT("Open Savanna");
    SavannaData.Temperature = 35.0f;
    SavannaData.Humidity = 0.3f;
    SavannaData.FoliageDensity = 0.3f;
    SavannaData.DinosaurSpawnWeight = 1.5f;
    SavannaData.BiomeDebugColor = FLinearColor(0.8f, 0.6f, 0.1f, 1.0f);
    BiomeDataMap.Add(EEng_BiomeType::Savanna, SavannaData);

    // Wetland biome — river delta, marshes
    FEng_BiomeData WetlandData;
    WetlandData.BiomeType = EEng_BiomeType::Wetland;
    WetlandData.BiomeName = TEXT("River Wetland");
    WetlandData.Temperature = 26.0f;
    WetlandData.Humidity = 0.95f;
    WetlandData.FoliageDensity = 0.7f;
    WetlandData.DinosaurSpawnWeight = 0.8f;
    WetlandData.BiomeDebugColor = FLinearColor(0.2f, 0.5f, 0.9f, 1.0f);
    BiomeDataMap.Add(EEng_BiomeType::Wetland, WetlandData);

    // Volcanic biome — ash plains, lava fields
    FEng_BiomeData VolcanicData;
    VolcanicData.BiomeType = EEng_BiomeType::Volcanic;
    VolcanicData.BiomeName = TEXT("Volcanic Badlands");
    VolcanicData.Temperature = 55.0f;
    VolcanicData.Humidity = 0.05f;
    VolcanicData.FoliageDensity = 0.05f;
    VolcanicData.DinosaurSpawnWeight = 0.4f;
    VolcanicData.BiomeDebugColor = FLinearColor(0.6f, 0.3f, 0.1f, 1.0f);
    BiomeDataMap.Add(EEng_BiomeType::Volcanic, VolcanicData);

    // Plains biome — open flat terrain, high visibility
    FEng_BiomeData PlainsData;
    PlainsData.BiomeType = EEng_BiomeType::Plains;
    PlainsData.BiomeName = TEXT("Open Plains");
    PlainsData.Temperature = 30.0f;
    PlainsData.Humidity = 0.4f;
    PlainsData.FoliageDensity = 0.2f;
    PlainsData.DinosaurSpawnWeight = 2.0f; // High dinosaur density on plains
    PlainsData.BiomeDebugColor = FLinearColor(0.9f, 0.9f, 0.8f, 1.0f);
    BiomeDataMap.Add(EEng_BiomeType::Plains, PlainsData);

    // Coastal biome — shoreline, tidal flats
    FEng_BiomeData CoastalData;
    CoastalData.BiomeType = EEng_BiomeType::Coastal;
    CoastalData.BiomeName = TEXT("Coastal Shore");
    CoastalData.Temperature = 24.0f;
    CoastalData.Humidity = 0.7f;
    CoastalData.FoliageDensity = 0.4f;
    CoastalData.DinosaurSpawnWeight = 0.6f;
    CoastalData.BiomeDebugColor = FLinearColor(0.3f, 0.7f, 1.0f, 1.0f);
    BiomeDataMap.Add(EEng_BiomeType::Coastal, CoastalData);
}

EEng_BiomeType ABiomeManager::GetBiomeAtLocation(FVector WorldLocation) const
{
    if (BiomeZones.Num() == 0)
    {
        // Fallback: classify by noise/position when no zones registered
        return ClassifyLocationByNoise(WorldLocation);
    }

    float ClosestDist = FLT_MAX;
    EEng_BiomeType ClosestBiome = EEng_BiomeType::Plains;

    for (const FEng_BiomeZone& Zone : BiomeZones)
    {
        float Dist = FVector::Dist2D(WorldLocation, Zone.Center);
        if (Dist <= Zone.Radius && Dist < ClosestDist)
        {
            ClosestDist = Dist;
            ClosestBiome = Zone.BiomeType;
        }
    }

    return ClosestBiome;
}

FEng_BiomeData ABiomeManager::GetBiomeDataAtLocation(FVector WorldLocation) const
{
    EEng_BiomeType BiomeType = GetBiomeAtLocation(WorldLocation);
    return GetBiomeDataByType(BiomeType);
}

FEng_BiomeData ABiomeManager::GetBiomeDataByType(EEng_BiomeType BiomeType) const
{
    const FEng_BiomeData* Data = BiomeDataMap.Find(BiomeType);
    if (Data)
    {
        return *Data;
    }

    // Return default Plains data if type not found
    FEng_BiomeData DefaultData;
    DefaultData.BiomeType = EEng_BiomeType::Plains;
    DefaultData.BiomeName = TEXT("Default Plains");
    return DefaultData;
}

void ABiomeManager::RegisterBiomeZone(FVector Center, float Radius, EEng_BiomeType BiomeType)
{
    FEng_BiomeZone NewZone;
    NewZone.Center = Center;
    NewZone.Radius = Radius;
    NewZone.BiomeType = BiomeType;
    BiomeZones.Add(NewZone);

    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Registered zone '%s' at (%.0f, %.0f) radius=%.0f"),
        *UEnum::GetValueAsString(BiomeType), Center.X, Center.Y, Radius);
}

int32 ABiomeManager::GetBiomeZoneCount() const
{
    return BiomeZones.Num();
}

void ABiomeManager::DrawBiomeBoundaries()
{
    UWorld* World = GetWorld();
    if (!World) return;

    for (const FEng_BiomeZone& Zone : BiomeZones)
    {
        const FEng_BiomeData* Data = BiomeDataMap.Find(Zone.BiomeType);
        FColor DebugColor = FColor::White;
        if (Data)
        {
            DebugColor = FColor(
                (uint8)(Data->BiomeDebugColor.R * 255),
                (uint8)(Data->BiomeDebugColor.G * 255),
                (uint8)(Data->BiomeDebugColor.B * 255),
                255
            );
        }

        DrawDebugCircle(World, Zone.Center, Zone.Radius, 32, DebugColor,
            false, 2.0f, 0, 5.0f, FVector(0,0,1), FVector(1,0,0));
    }
}

EEng_BiomeType ABiomeManager::ClassifyLocationByNoise(FVector WorldLocation) const
{
    // Simple deterministic classification based on world position
    // Used when no zones are registered (fallback for early development)
    float X = WorldLocation.X;
    float Y = WorldLocation.Y;

    // Quadrant-based classification for MinPlayableMap
    if (X > 300.0f && Y > -200.0f)  return EEng_BiomeType::Forest;
    if (X < -300.0f && Y > 200.0f)  return EEng_BiomeType::Savanna;
    if (FMath::Abs(Y) > 600.0f)     return EEng_BiomeType::Wetland;
    if (X > 600.0f && Y < -200.0f)  return EEng_BiomeType::Volcanic;
    if (X < -200.0f && Y < -400.0f) return EEng_BiomeType::Coastal;
    return EEng_BiomeType::Plains;
}
