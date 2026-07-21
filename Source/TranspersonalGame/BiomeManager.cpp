// BiomeManager.cpp — Biome system implementation
// Engine Architect #02 — Transpersonal Game Studio
// Cycle: PROD_CYCLE_AUTO_20260626_012

#include "BiomeManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

// ============================================================
// UBiomeDataAsset
// ============================================================

UBiomeDataAsset::UBiomeDataAsset()
{
    BiomeName           = TEXT("Unknown Biome");
    BiomeType           = EBiomeType::Grassland;
    MinTemperature      = 15.0f;
    MaxTemperature      = 30.0f;
    Humidity            = 0.5f;
    VegetationDensity   = 0.5f;
    FoliageColor        = FLinearColor(0.1f, 0.4f, 0.05f, 1.0f);
    FogDensity          = 0.02f;
    bHasPredators       = true;
    bHasWaterSources    = true;
    ResourceAbundance   = 0.5f;
}

// ============================================================
// ABiomeManager
// ============================================================

ABiomeManager::ABiomeManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f; // Update once per second

    WorldSizeKm         = 4.0f;
    BiomeBlendRadius    = 500.0f;
    bDebugDrawBiomes    = false;
    bBiomesInitialized  = false;
    CurrentDayTime      = 12.0f; // Start at noon
    DayDurationSeconds  = 600.0f; // 10-minute day cycle
}

void ABiomeManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeBiomes();
}

void ABiomeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Advance day/night cycle
    CurrentDayTime += DeltaTime * (24.0f / DayDurationSeconds);
    if (CurrentDayTime >= 24.0f)
    {
        CurrentDayTime -= 24.0f;
    }

    // Debug draw biome boundaries
    if (bDebugDrawBiomes && GetWorld())
    {
        for (const FBiomeZone& Zone : BiomeZones)
        {
            DrawDebugSphere(GetWorld(), Zone.WorldCenter, Zone.Radius, 16,
                FColor::Green, false, 1.1f);
        }
    }
}

void ABiomeManager::InitializeBiomes()
{
    if (bBiomesInitialized) return;

    // Generate default biome layout if none provided via data assets
    if (BiomeZones.Num() == 0)
    {
        GenerateDefaultBiomeLayout();
    }

    bBiomesInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Initialized %d biome zones"), BiomeZones.Num());
}

void ABiomeManager::GenerateDefaultBiomeLayout()
{
    const float HalfWorld = (WorldSizeKm * 1000.0f) * 0.5f;

    // Create 6 distinct biome zones across the world
    struct FBiomePreset
    {
        FVector2D Offset;
        float     Radius;
        EBiomeType Type;
        FString   Name;
    };

    TArray<FBiomePreset> Presets = {
        { FVector2D(0.0f,      0.0f),      1800.0f, EBiomeType::Jungle,      TEXT("Central Jungle")     },
        { FVector2D(2500.0f,   0.0f),      1400.0f, EBiomeType::Grassland,   TEXT("Eastern Savanna")    },
        { FVector2D(-2500.0f,  0.0f),      1400.0f, EBiomeType::Forest,      TEXT("Western Forest")     },
        { FVector2D(0.0f,      2500.0f),   1200.0f, EBiomeType::Desert,      TEXT("Northern Badlands")  },
        { FVector2D(0.0f,     -2500.0f),   1200.0f, EBiomeType::Swamp,       TEXT("Southern Swamp")     },
        { FVector2D(2000.0f,  -2000.0f),   1000.0f, EBiomeType::Mountain,    TEXT("Volcanic Ridge")     },
    };

    for (const FBiomePreset& Preset : Presets)
    {
        FBiomeZone Zone;
        Zone.WorldCenter = FVector(Preset.Offset.X, Preset.Offset.Y, 0.0f);
        Zone.Radius      = Preset.Radius;
        Zone.BiomeType   = Preset.Type;
        Zone.BiomeName   = Preset.Name;
        Zone.BlendWeight = 1.0f;
        BiomeZones.Add(Zone);
    }
}

EBiomeType ABiomeManager::GetBiomeAtLocation(FVector WorldLocation) const
{
    if (BiomeZones.Num() == 0) return EBiomeType::Grassland;

    float      BestWeight = -1.0f;
    EBiomeType BestBiome  = EBiomeType::Grassland;

    for (const FBiomeZone& Zone : BiomeZones)
    {
        const float Dist   = FVector::Dist2D(WorldLocation, Zone.WorldCenter);
        const float Weight = FMath::Max(0.0f, 1.0f - (Dist / Zone.Radius));

        if (Weight > BestWeight)
        {
            BestWeight = Weight;
            BestBiome  = Zone.BiomeType;
        }
    }

    return BestBiome;
}

FBiomeZone ABiomeManager::GetBiomeZoneAtLocation(FVector WorldLocation) const
{
    if (BiomeZones.Num() == 0) return FBiomeZone();

    float      BestWeight = -1.0f;
    int32      BestIndex  = 0;

    for (int32 i = 0; i < BiomeZones.Num(); ++i)
    {
        const FBiomeZone& Zone = BiomeZones[i];
        const float Dist   = FVector::Dist2D(WorldLocation, Zone.WorldCenter);
        const float Weight = FMath::Max(0.0f, 1.0f - (Dist / Zone.Radius));

        if (Weight > BestWeight)
        {
            BestWeight = Weight;
            BestIndex  = i;
        }
    }

    return BiomeZones[BestIndex];
}

float ABiomeManager::GetTemperatureAtLocation(FVector WorldLocation) const
{
    const EBiomeType Biome = GetBiomeAtLocation(WorldLocation);

    // Base temperature per biome type
    switch (Biome)
    {
    case EBiomeType::Desert:    return 42.0f;
    case EBiomeType::Jungle:    return 32.0f;
    case EBiomeType::Swamp:     return 28.0f;
    case EBiomeType::Grassland: return 25.0f;
    case EBiomeType::Forest:    return 20.0f;
    case EBiomeType::Mountain:  return 8.0f;
    case EBiomeType::Coastal:   return 22.0f;
    default:                    return 20.0f;
    }
}

float ABiomeManager::GetHumidityAtLocation(FVector WorldLocation) const
{
    const EBiomeType Biome = GetBiomeAtLocation(WorldLocation);

    switch (Biome)
    {
    case EBiomeType::Swamp:     return 0.95f;
    case EBiomeType::Jungle:    return 0.85f;
    case EBiomeType::Coastal:   return 0.70f;
    case EBiomeType::Forest:    return 0.60f;
    case EBiomeType::Grassland: return 0.40f;
    case EBiomeType::Mountain:  return 0.35f;
    case EBiomeType::Desert:    return 0.05f;
    default:                    return 0.40f;
    }
}

bool ABiomeManager::IsLocationInBiome(FVector WorldLocation, EBiomeType BiomeType) const
{
    return GetBiomeAtLocation(WorldLocation) == BiomeType;
}

TArray<FBiomeZone> ABiomeManager::GetAllBiomeZones() const
{
    return BiomeZones;
}

float ABiomeManager::GetCurrentDayTime() const
{
    return CurrentDayTime;
}

bool ABiomeManager::IsDaytime() const
{
    return CurrentDayTime >= 6.0f && CurrentDayTime < 20.0f;
}
