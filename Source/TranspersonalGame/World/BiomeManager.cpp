// BiomeManager.cpp — Transpersonal Game Studio
// Engine Architect #02 — Cycle PROD_CYCLE_AUTO_20260701_003
// Biome system: classifies terrain into biomes, drives foliage/weather/fauna density

#include "BiomeManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogBiomeManager, Log, All);

// ============================================================
// Constructor
// ============================================================
ABiomeManager::ABiomeManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 2.0f; // Update every 2 seconds — not every frame

    WorldSizeKm = 4.0f;
    BiomeGridResolution = 32;
    bBiomesInitialized = false;

    // Default biome weights for a Cretaceous world
    // 40% forest, 25% plains, 20% swamp, 10% volcanic, 5% coastal
    BiomeWeights.Add(EEng_BiomeType::CretaceousForest, 0.40f);
    BiomeWeights.Add(EEng_BiomeType::OpenPlains, 0.25f);
    BiomeWeights.Add(EEng_BiomeType::SwampDelta, 0.20f);
    BiomeWeights.Add(EEng_BiomeType::VolcanicBadlands, 0.10f);
    BiomeWeights.Add(EEng_BiomeType::CoastalShallows, 0.05f);
}

// ============================================================
// BeginPlay
// ============================================================
void ABiomeManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeBiomeGrid();
}

// ============================================================
// Tick
// ============================================================
void ABiomeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    // Future: update dynamic biome transitions (flooding, volcanic activity)
}

// ============================================================
// InitializeBiomeGrid
// ============================================================
void ABiomeManager::InitializeBiomeGrid()
{
    if (bBiomesInitialized)
    {
        UE_LOG(LogBiomeManager, Warning, TEXT("BiomeGrid already initialized — skipping"));
        return;
    }

    const int32 TotalCells = BiomeGridResolution * BiomeGridResolution;
    BiomeGrid.SetNum(TotalCells);

    UE_LOG(LogBiomeManager, Log, TEXT("Initializing biome grid: %dx%d (%d cells) for %.1fkm world"),
        BiomeGridResolution, BiomeGridResolution, TotalCells, WorldSizeKm);

    // Seed biome grid using deterministic noise-like distribution
    // Cells near center = forest/plains, edges = coastal/volcanic
    const float HalfGrid = BiomeGridResolution * 0.5f;

    for (int32 Y = 0; Y < BiomeGridResolution; Y++)
    {
        for (int32 X = 0; X < BiomeGridResolution; X++)
        {
            const int32 Index = Y * BiomeGridResolution + X;
            const float NormX = (X - HalfGrid) / HalfGrid; // -1..1
            const float NormY = (Y - HalfGrid) / HalfGrid; // -1..1
            const float DistFromCenter = FMath::Sqrt(NormX * NormX + NormY * NormY);

            // Assign biome based on distance + deterministic hash
            EEng_BiomeType AssignedBiome = ClassifyBiomeByPosition(NormX, NormY, DistFromCenter, Index);
            BiomeGrid[Index] = AssignedBiome;
        }
    }

    bBiomesInitialized = true;
    UE_LOG(LogBiomeManager, Log, TEXT("Biome grid initialized successfully"));
}

// ============================================================
// ClassifyBiomeByPosition — deterministic biome assignment
// ============================================================
EEng_BiomeType ABiomeManager::ClassifyBiomeByPosition(float NormX, float NormY, float DistFromCenter, int32 Seed) const
{
    // Coastal ring at edges
    if (DistFromCenter > 0.85f)
    {
        return EEng_BiomeType::CoastalShallows;
    }

    // Volcanic badlands in SW quadrant (NormX < -0.3, NormY < -0.3)
    if (NormX < -0.3f && NormY < -0.3f && DistFromCenter > 0.4f)
    {
        return EEng_BiomeType::VolcanicBadlands;
    }

    // Swamp delta in NE quadrant
    if (NormX > 0.2f && NormY > 0.2f && DistFromCenter > 0.3f)
    {
        return EEng_BiomeType::SwampDelta;
    }

    // Open plains in N strip
    if (NormY < -0.2f && FMath::Abs(NormX) < 0.5f)
    {
        return EEng_BiomeType::OpenPlains;
    }

    // Everything else = dense Cretaceous forest
    return EEng_BiomeType::CretaceousForest;
}

// ============================================================
// GetBiomeAtWorldLocation — query biome for any world position
// ============================================================
EEng_BiomeType ABiomeManager::GetBiomeAtWorldLocation(const FVector& WorldLocation) const
{
    if (!bBiomesInitialized)
    {
        UE_LOG(LogBiomeManager, Warning, TEXT("GetBiomeAtWorldLocation called before initialization"));
        return EEng_BiomeType::CretaceousForest;
    }

    const float WorldSizeCm = WorldSizeKm * 100000.0f; // km to cm
    const float HalfWorld = WorldSizeCm * 0.5f;

    // Clamp to world bounds
    const float ClampedX = FMath::Clamp(WorldLocation.X, -HalfWorld, HalfWorld);
    const float ClampedY = FMath::Clamp(WorldLocation.Y, -HalfWorld, HalfWorld);

    // Map to grid coordinates
    const int32 GridX = FMath::FloorToInt(((ClampedX + HalfWorld) / WorldSizeCm) * BiomeGridResolution);
    const int32 GridY = FMath::FloorToInt(((ClampedY + HalfWorld) / WorldSizeCm) * BiomeGridResolution);

    const int32 SafeX = FMath::Clamp(GridX, 0, BiomeGridResolution - 1);
    const int32 SafeY = FMath::Clamp(GridY, 0, BiomeGridResolution - 1);
    const int32 Index = SafeY * BiomeGridResolution + SafeX;

    if (BiomeGrid.IsValidIndex(Index))
    {
        return BiomeGrid[Index];
    }

    return EEng_BiomeType::CretaceousForest;
}

// ============================================================
// GetBiomeDisplayName — human-readable biome name
// ============================================================
FString ABiomeManager::GetBiomeDisplayName(EEng_BiomeType BiomeType) const
{
    switch (BiomeType)
    {
        case EEng_BiomeType::CretaceousForest:   return TEXT("Cretaceous Forest");
        case EEng_BiomeType::OpenPlains:          return TEXT("Open Plains");
        case EEng_BiomeType::SwampDelta:          return TEXT("Swamp Delta");
        case EEng_BiomeType::VolcanicBadlands:    return TEXT("Volcanic Badlands");
        case EEng_BiomeType::CoastalShallows:     return TEXT("Coastal Shallows");
        default:                                   return TEXT("Unknown Biome");
    }
}

// ============================================================
// GetFoliageDensityForBiome — drives FoliageManager density
// ============================================================
float ABiomeManager::GetFoliageDensityForBiome(EEng_BiomeType BiomeType) const
{
    switch (BiomeType)
    {
        case EEng_BiomeType::CretaceousForest:   return 1.0f;   // Maximum density
        case EEng_BiomeType::SwampDelta:          return 0.8f;   // Dense but water-logged
        case EEng_BiomeType::OpenPlains:          return 0.3f;   // Sparse ground cover
        case EEng_BiomeType::CoastalShallows:     return 0.2f;   // Beach vegetation
        case EEng_BiomeType::VolcanicBadlands:    return 0.05f;  // Almost barren
        default:                                   return 0.5f;
    }
}

// ============================================================
// GetDinosaurDensityForBiome — drives DinosaurSpawner density
// ============================================================
float ABiomeManager::GetDinosaurDensityForBiome(EEng_BiomeType BiomeType) const
{
    switch (BiomeType)
    {
        case EEng_BiomeType::OpenPlains:          return 1.0f;   // Herds roam plains
        case EEng_BiomeType::CretaceousForest:    return 0.7f;   // Predators in forest
        case EEng_BiomeType::SwampDelta:          return 0.5f;   // Aquatic species
        case EEng_BiomeType::CoastalShallows:     return 0.3f;   // Coastal species
        case EEng_BiomeType::VolcanicBadlands:    return 0.1f;   // Extreme survivors only
        default:                                   return 0.5f;
    }
}

// ============================================================
// GetWeatherIntensityForBiome — drives Weather system
// ============================================================
float ABiomeManager::GetWeatherIntensityForBiome(EEng_BiomeType BiomeType) const
{
    switch (BiomeType)
    {
        case EEng_BiomeType::SwampDelta:          return 0.9f;   // Constant rain
        case EEng_BiomeType::CoastalShallows:     return 0.7f;   // Coastal storms
        case EEng_BiomeType::CretaceousForest:    return 0.5f;   // Tropical showers
        case EEng_BiomeType::OpenPlains:          return 0.3f;   // Dry season
        case EEng_BiomeType::VolcanicBadlands:    return 0.1f;   // Ash clouds only
        default:                                   return 0.4f;
    }
}

// ============================================================
// DebugDrawBiomeGrid — visualize biome boundaries in editor
// ============================================================
void ABiomeManager::DebugDrawBiomeGrid()
{
    if (!bBiomesInitialized)
    {
        InitializeBiomeGrid();
    }

    UWorld* World = GetWorld();
    if (!World) return;

    const float WorldSizeCm = WorldSizeKm * 100000.0f;
    const float CellSize = WorldSizeCm / BiomeGridResolution;
    const float HalfWorld = WorldSizeCm * 0.5f;
    const float DrawZ = 200.0f;
    const float DrawDuration = 30.0f;

    // Color map per biome
    TMap<EEng_BiomeType, FColor> BiomeColors;
    BiomeColors.Add(EEng_BiomeType::CretaceousForest,  FColor(34, 139, 34));    // Forest green
    BiomeColors.Add(EEng_BiomeType::OpenPlains,         FColor(210, 180, 140));  // Tan
    BiomeColors.Add(EEng_BiomeType::SwampDelta,         FColor(0, 100, 80));     // Dark teal
    BiomeColors.Add(EEng_BiomeType::VolcanicBadlands,   FColor(180, 60, 20));    // Dark red
    BiomeColors.Add(EEng_BiomeType::CoastalShallows,    FColor(64, 164, 223));   // Ocean blue

    for (int32 Y = 0; Y < BiomeGridResolution; Y++)
    {
        for (int32 X = 0; X < BiomeGridResolution; X++)
        {
            const int32 Index = Y * BiomeGridResolution + X;
            if (!BiomeGrid.IsValidIndex(Index)) continue;

            const EEng_BiomeType Biome = BiomeGrid[Index];
            const FColor* ColorPtr = BiomeColors.Find(Biome);
            const FColor Color = ColorPtr ? *ColorPtr : FColor::White;

            const float WorldX = -HalfWorld + (X + 0.5f) * CellSize;
            const float WorldY = -HalfWorld + (Y + 0.5f) * CellSize;
            const FVector Center(WorldX, WorldY, DrawZ);

            DrawDebugBox(World, Center, FVector(CellSize * 0.45f, CellSize * 0.45f, 50.0f),
                Color, false, DrawDuration, 0, 10.0f);
        }
    }

    UE_LOG(LogBiomeManager, Log, TEXT("Biome grid debug draw complete (%d cells)"), BiomeGrid.Num());
}
