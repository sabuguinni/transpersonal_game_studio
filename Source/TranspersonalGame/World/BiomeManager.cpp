// BiomeManager.cpp
// Transpersonal Game Studio — Engine Architect #02
// Biome system: classifies terrain zones, drives vegetation/fauna density

#include "BiomeManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

// ============================================================
// Constructor
// ============================================================

ABiomeManager::ABiomeManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 2.0f; // Update every 2s — not every frame

    // Default biome grid
    BiomeGridResolution = 16;
    WorldExtentKm = 4.0f;
    bDebugDrawBiomes = false;

    // Default biome parameters
    FEng_BiomeParams ForestParams;
    ForestParams.BiomeType = EEng_BiomeType::TropicalForest;
    ForestParams.VegetationDensity = 0.85f;
    ForestParams.FaunaDensity = 0.7f;
    ForestParams.TemperatureCelsius = 28.0f;
    ForestParams.HumidityPercent = 80.0f;
    ForestParams.DangerLevel = 0.6f;
    BiomeTable.Add(EEng_BiomeType::TropicalForest, ForestParams);

    FEng_BiomeParams SavannaParams;
    SavannaParams.BiomeType = EEng_BiomeType::Savanna;
    SavannaParams.VegetationDensity = 0.35f;
    SavannaParams.FaunaDensity = 0.9f;
    SavannaParams.TemperatureCelsius = 34.0f;
    SavannaParams.HumidityPercent = 30.0f;
    SavannaParams.DangerLevel = 0.8f;
    BiomeTable.Add(EEng_BiomeType::Savanna, SavannaParams);

    FEng_BiomeParams SwampParams;
    SwampParams.BiomeType = EEng_BiomeType::Swamp;
    SwampParams.VegetationDensity = 0.9f;
    SwampParams.FaunaDensity = 0.5f;
    SwampParams.TemperatureCelsius = 26.0f;
    SwampParams.HumidityPercent = 95.0f;
    SwampParams.DangerLevel = 0.7f;
    BiomeTable.Add(EEng_BiomeType::Swamp, SwampParams);

    FEng_BiomeParams VolcanicParams;
    VolcanicParams.BiomeType = EEng_BiomeType::Volcanic;
    VolcanicParams.VegetationDensity = 0.05f;
    VolcanicParams.FaunaDensity = 0.1f;
    VolcanicParams.TemperatureCelsius = 55.0f;
    VolcanicParams.HumidityPercent = 10.0f;
    VolcanicParams.DangerLevel = 1.0f;
    BiomeTable.Add(EEng_BiomeType::Volcanic, VolcanicParams);

    FEng_BiomeParams RiverbankParams;
    RiverbankParams.BiomeType = EEng_BiomeType::Riverbank;
    RiverbankParams.VegetationDensity = 0.75f;
    RiverbankParams.FaunaDensity = 0.65f;
    RiverbankParams.TemperatureCelsius = 24.0f;
    RiverbankParams.HumidityPercent = 70.0f;
    RiverbankParams.DangerLevel = 0.5f;
    BiomeTable.Add(EEng_BiomeType::Riverbank, RiverbankParams);
}

// ============================================================
// BeginPlay
// ============================================================

void ABiomeManager::BeginPlay()
{
    Super::BeginPlay();
    GenerateBiomeGrid();
    UE_LOG(LogTemp, Log, TEXT("[BiomeManager] Initialized — %d biome cells, world %.1fkm²"),
        BiomeGrid.Num(), WorldExtentKm * WorldExtentKm);
}

// ============================================================
// Tick
// ============================================================

void ABiomeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

#if WITH_EDITOR
    if (bDebugDrawBiomes)
    {
        DebugDrawBiomes();
    }
#endif
}

// ============================================================
// GenerateBiomeGrid — Procedural biome assignment using noise
// ============================================================

void ABiomeManager::GenerateBiomeGrid()
{
    BiomeGrid.Empty();
    BiomeGrid.Reserve(BiomeGridResolution * BiomeGridResolution);

    const float WorldExtentCm = WorldExtentKm * 100000.0f;
    const float CellSize = WorldExtentCm / BiomeGridResolution;

    for (int32 Y = 0; Y < BiomeGridResolution; ++Y)
    {
        for (int32 X = 0; X < BiomeGridResolution; ++X)
        {
            FEng_BiomeCell Cell;
            Cell.GridX = X;
            Cell.GridY = Y;
            Cell.WorldCenter = FVector(
                (X + 0.5f) * CellSize - WorldExtentCm * 0.5f,
                (Y + 0.5f) * CellSize - WorldExtentCm * 0.5f,
                0.0f
            );
            Cell.BiomeType = ClassifyBiomeAtPosition(Cell.WorldCenter);
            BiomeGrid.Add(Cell);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("[BiomeManager] Generated %d biome cells"), BiomeGrid.Num());
}

// ============================================================
// ClassifyBiomeAtPosition — Simple noise-based classification
// ============================================================

EEng_BiomeType ABiomeManager::ClassifyBiomeAtPosition(const FVector& WorldPos) const
{
    // Normalize to [0,1]
    const float WorldExtentCm = WorldExtentKm * 100000.0f;
    const float NX = (WorldPos.X / WorldExtentCm) + 0.5f;
    const float NY = (WorldPos.Y / WorldExtentCm) + 0.5f;

    // Simple deterministic pseudo-noise using sine waves
    const float Moisture = FMath::Sin(NX * 7.3f) * FMath::Cos(NY * 5.1f) * 0.5f + 0.5f;
    const float Temperature = FMath::Cos(NX * 3.7f + NY * 4.2f) * 0.5f + 0.5f;
    const float Elevation = FMath::Sin(NX * 11.1f) * FMath::Sin(NY * 9.3f) * 0.5f + 0.5f;

    // Classification rules (Whittaker biome model simplified)
    if (Elevation > 0.75f)
    {
        return EEng_BiomeType::Volcanic;
    }
    if (Moisture > 0.65f && Temperature > 0.55f)
    {
        return EEng_BiomeType::TropicalForest;
    }
    if (Moisture > 0.7f && Temperature < 0.5f)
    {
        return EEng_BiomeType::Swamp;
    }
    if (Moisture > 0.5f && Elevation < 0.3f)
    {
        return EEng_BiomeType::Riverbank;
    }

    return EEng_BiomeType::Savanna;
}

// ============================================================
// GetBiomeAtLocation — Query biome for any world position
// ============================================================

EEng_BiomeType ABiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    return ClassifyBiomeAtPosition(WorldLocation);
}

// ============================================================
// GetBiomeParams — Retrieve parameters for a biome type
// ============================================================

FEng_BiomeParams ABiomeManager::GetBiomeParams(EEng_BiomeType BiomeType) const
{
    const FEng_BiomeParams* Found = BiomeTable.Find(BiomeType);
    if (Found)
    {
        return *Found;
    }

    // Return safe defaults if not found
    FEng_BiomeParams Default;
    Default.BiomeType = BiomeType;
    Default.VegetationDensity = 0.5f;
    Default.FaunaDensity = 0.5f;
    Default.TemperatureCelsius = 25.0f;
    Default.HumidityPercent = 50.0f;
    Default.DangerLevel = 0.5f;
    return Default;
}

// ============================================================
// GetVegetationDensityAtLocation
// ============================================================

float ABiomeManager::GetVegetationDensityAtLocation(const FVector& WorldLocation) const
{
    EEng_BiomeType Biome = GetBiomeAtLocation(WorldLocation);
    FEng_BiomeParams Params = GetBiomeParams(Biome);
    return Params.VegetationDensity;
}

// ============================================================
// GetDangerLevelAtLocation
// ============================================================

float ABiomeManager::GetDangerLevelAtLocation(const FVector& WorldLocation) const
{
    EEng_BiomeType Biome = GetBiomeAtLocation(WorldLocation);
    FEng_BiomeParams Params = GetBiomeParams(Biome);
    return Params.DangerLevel;
}

// ============================================================
// DebugDrawBiomes — Editor visualization
// ============================================================

void ABiomeManager::DebugDrawBiomes()
{
#if WITH_EDITOR
    UWorld* World = GetWorld();
    if (!World) return;

    const float WorldExtentCm = WorldExtentKm * 100000.0f;
    const float CellSize = WorldExtentCm / BiomeGridResolution;

    for (const FEng_BiomeCell& Cell : BiomeGrid)
    {
        FColor DebugColor = FColor::White;
        switch (Cell.BiomeType)
        {
            case EEng_BiomeType::TropicalForest: DebugColor = FColor::Green; break;
            case EEng_BiomeType::Savanna:        DebugColor = FColor::Yellow; break;
            case EEng_BiomeType::Swamp:          DebugColor = FColor(0, 100, 50); break;
            case EEng_BiomeType::Volcanic:       DebugColor = FColor::Red; break;
            case EEng_BiomeType::Riverbank:      DebugColor = FColor::Blue; break;
            default: break;
        }

        DrawDebugBox(World,
            Cell.WorldCenter + FVector(0, 0, 100),
            FVector(CellSize * 0.45f, CellSize * 0.45f, 50.0f),
            DebugColor, false, 2.5f, 0, 20.0f);
    }
#endif
}
