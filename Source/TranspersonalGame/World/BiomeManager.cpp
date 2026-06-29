#include "BiomeManager.h"
#include "DrawDebugHelpers.h"

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

ABiomeManager::ABiomeManager()
{
    PrimaryActorTick.bCanEverTick = false; // No per-frame work needed; queries are on-demand
    PrimaryActorTick.bStartWithTickEnabled = false;
}

// ─────────────────────────────────────────────────────────────────────────────
// BeginPlay
// ─────────────────────────────────────────────────────────────────────────────

void ABiomeManager::BeginPlay()
{
    Super::BeginPlay();

    // Prototype fallback: if no biome cells were registered by PCGWorldGenerator,
    // seed the world with a default prehistoric layout for the MinPlayableMap.
    if (BiomeCells.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("ABiomeManager: No biome cells registered — initialising default prehistoric biomes."));
        InitialiseDefaultBiomes();
    }

    UE_LOG(LogTemp, Log, TEXT("ABiomeManager: Ready with %d biome cells."), BiomeCells.Num());
}

// ─────────────────────────────────────────────────────────────────────────────
// Tick (disabled — kept for future weather-driven biome transitions)
// ─────────────────────────────────────────────────────────────────────────────

void ABiomeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

#if WITH_EDITOR
    if (bDrawDebugBiomes)
    {
        for (const FEng_BiomeData& Cell : BiomeCells)
        {
            DrawDebugSphere(GetWorld(), Cell.WorldOrigin, Cell.Radius * 0.1f, 12, FColor::Green, false, -1.0f, 0, 5.0f);
        }
    }
#endif
}

// ─────────────────────────────────────────────────────────────────────────────
// Biome Query API
// ─────────────────────────────────────────────────────────────────────────────

FEng_BiomeData ABiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    const int32 Index = FindNearestBiomeCellIndex(WorldLocation);
    if (Index == -1)
    {
        // Return a safe default — unknown biome, moderate conditions
        FEng_BiomeData Default;
        Default.BiomeType  = EEng_BiomeType::Unknown;
        Default.Temperature = 25.0f;
        Default.Rainfall    = 1000.0f;
        Default.Elevation   = 0.0f;
        Default.DangerLevel = 0.3f;
        Default.WorldOrigin = WorldLocation;
        return Default;
    }
    return BiomeCells[Index];
}

EEng_BiomeType ABiomeManager::GetBiomeTypeAtLocation(const FVector& WorldLocation) const
{
    return GetBiomeAtLocation(WorldLocation).BiomeType;
}

float ABiomeManager::GetDangerLevelAtLocation(const FVector& WorldLocation) const
{
    return GetBiomeAtLocation(WorldLocation).DangerLevel;
}

// ─────────────────────────────────────────────────────────────────────────────
// Biome Registration
// ─────────────────────────────────────────────────────────────────────────────

void ABiomeManager::RegisterBiomeCell(const FEng_BiomeData& BiomeData)
{
    BiomeCells.Add(BiomeData);
}

void ABiomeManager::ClearAllBiomeCells()
{
    BiomeCells.Empty();
    UE_LOG(LogTemp, Log, TEXT("ABiomeManager: All biome cells cleared."));
}

int32 ABiomeManager::GetBiomeCellCount() const
{
    return BiomeCells.Num();
}

// ─────────────────────────────────────────────────────────────────────────────
// Default Biome Initialisation (MinPlayableMap prototype layout)
// ─────────────────────────────────────────────────────────────────────────────

void ABiomeManager::InitialiseDefaultBiomes()
{
    ClearAllBiomeCells();

    // Layout: 7 biome cells arranged around the MinPlayableMap origin (0,0,0)
    // Each cell covers a 500m radius (50,000 cm) — overlapping Voronoi-style

    // 1. Central Plains — safe starting area around PlayerStart
    {
        FEng_BiomeData Cell;
        Cell.BiomeType   = EEng_BiomeType::Plains;
        Cell.Temperature = 28.0f;
        Cell.Rainfall    = 900.0f;
        Cell.Elevation   = 50.0f;
        Cell.DangerLevel = 0.2f;
        Cell.WorldOrigin = FVector(0.0f, 0.0f, 0.0f);
        Cell.Radius      = 50000.0f;
        BiomeCells.Add(Cell);
    }

    // 2. Northern Jungle — dense vegetation, medium danger (Raptors)
    {
        FEng_BiomeData Cell;
        Cell.BiomeType   = EEng_BiomeType::Jungle;
        Cell.Temperature = 32.0f;
        Cell.Rainfall    = 2200.0f;
        Cell.Elevation   = 80.0f;
        Cell.DangerLevel = 0.65f;
        Cell.WorldOrigin = FVector(0.0f, 80000.0f, 0.0f);
        Cell.Radius      = 60000.0f;
        BiomeCells.Add(Cell);
    }

    // 3. Eastern Savanna — open grassland, T-Rex territory (high danger)
    {
        FEng_BiomeData Cell;
        Cell.BiomeType   = EEng_BiomeType::Savanna;
        Cell.Temperature = 35.0f;
        Cell.Rainfall    = 600.0f;
        Cell.Elevation   = 30.0f;
        Cell.DangerLevel = 0.85f;
        Cell.WorldOrigin = FVector(80000.0f, 0.0f, 0.0f);
        Cell.Radius      = 55000.0f;
        BiomeCells.Add(Cell);
    }

    // 4. Western Swamp — slow movement, ambush predators
    {
        FEng_BiomeData Cell;
        Cell.BiomeType   = EEng_BiomeType::Swamp;
        Cell.Temperature = 30.0f;
        Cell.Rainfall    = 1800.0f;
        Cell.Elevation   = 5.0f;
        Cell.DangerLevel = 0.70f;
        Cell.WorldOrigin = FVector(-80000.0f, 0.0f, 0.0f);
        Cell.Radius      = 55000.0f;
        BiomeCells.Add(Cell);
    }

    // 5. Southern Coastal — Brachiosaurus grazing, low danger
    {
        FEng_BiomeData Cell;
        Cell.BiomeType   = EEng_BiomeType::Coastal;
        Cell.Temperature = 26.0f;
        Cell.Rainfall    = 1100.0f;
        Cell.Elevation   = 2.0f;
        Cell.DangerLevel = 0.30f;
        Cell.WorldOrigin = FVector(0.0f, -80000.0f, 0.0f);
        Cell.Radius      = 50000.0f;
        BiomeCells.Add(Cell);
    }

    // 6. North-East Volcanic — extreme danger, rare resources
    {
        FEng_BiomeData Cell;
        Cell.BiomeType   = EEng_BiomeType::Volcanic;
        Cell.Temperature = 55.0f;
        Cell.Rainfall    = 200.0f;
        Cell.Elevation   = 400.0f;
        Cell.DangerLevel = 0.95f;
        Cell.WorldOrigin = FVector(70000.0f, 70000.0f, 0.0f);
        Cell.Radius      = 45000.0f;
        BiomeCells.Add(Cell);
    }

    // 7. North-West Forest — medium danger, good crafting resources
    {
        FEng_BiomeData Cell;
        Cell.BiomeType   = EEng_BiomeType::Forest;
        Cell.Temperature = 22.0f;
        Cell.Rainfall    = 1400.0f;
        Cell.Elevation   = 120.0f;
        Cell.DangerLevel = 0.45f;
        Cell.WorldOrigin = FVector(-70000.0f, 70000.0f, 0.0f);
        Cell.Radius      = 50000.0f;
        BiomeCells.Add(Cell);
    }

    UE_LOG(LogTemp, Log, TEXT("ABiomeManager: Initialised %d default prehistoric biome cells."), BiomeCells.Num());
}

// ─────────────────────────────────────────────────────────────────────────────
// Internal — Nearest Cell Search (linear scan; acceptable for ≤100 cells)
// ─────────────────────────────────────────────────────────────────────────────

int32 ABiomeManager::FindNearestBiomeCellIndex(const FVector& WorldLocation) const
{
    if (BiomeCells.Num() == 0)
    {
        return -1;
    }

    int32   NearestIndex    = 0;
    float   NearestDistSq   = FVector::DistSquared(WorldLocation, BiomeCells[0].WorldOrigin);

    for (int32 i = 1; i < BiomeCells.Num(); ++i)
    {
        const float DistSq = FVector::DistSquared(WorldLocation, BiomeCells[i].WorldOrigin);
        if (DistSq < NearestDistSq)
        {
            NearestDistSq = DistSq;
            NearestIndex  = i;
        }
    }

    return NearestIndex;
}
