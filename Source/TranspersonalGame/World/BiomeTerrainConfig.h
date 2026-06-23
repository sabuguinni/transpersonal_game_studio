// BiomeTerrainConfig.h
// Agent #05 — Procedural World Generator
// PROD_CYCLE_AUTO_20260623_007
// Biome terrain configuration constants for the Cretaceous world

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "BiomeTerrainConfig.generated.h"

// ============================================================
// Biome type enum — must be global scope (UHT rule)
// ============================================================
UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest          UMETA(DisplayName = "Cretaceous Forest"),
    Plains          UMETA(DisplayName = "Open Plains"),
    RockyRidge      UMETA(DisplayName = "Rocky Ridge"),
    RiverValley     UMETA(DisplayName = "River Valley"),
    VolcanicHills   UMETA(DisplayName = "Volcanic Hills"),
    Swamp           UMETA(DisplayName = "Swamp"),
    COUNT           UMETA(Hidden)
};

// ============================================================
// Per-biome terrain parameters
// ============================================================
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeTerrainParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Plains;

    /** Base height offset for this biome (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float BaseHeightOffset = 0.0f;

    /** Maximum height variation within biome (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float HeightVariation = 500.0f;

    /** Noise frequency multiplier for terrain detail */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float NoiseFrequency = 1.0f;

    /** Foliage density 0-1 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float FoliageDensity = 0.5f;

    /** Rock density 0-1 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float RockDensity = 0.3f;

    /** Water presence 0-1 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float WaterPresence = 0.0f;

    FWorld_BiomeTerrainParams() = default;
};

// ============================================================
// World terrain configuration — compile-time constants
// ============================================================
namespace WorldTerrainConfig
{
    // World size
    static constexpr float WORLD_EXTENT_CM         = 28000.0f;  // 280m radius
    static constexpr float TERRAIN_TILE_SIZE_CM     = 4000.0f;  // 40m tiles
    static constexpr int32 TERRAIN_GRID_SIZE        = 7;         // 7x7 grid

    // Height range
    static constexpr float TERRAIN_MIN_HEIGHT_CM    = -200.0f;
    static constexpr float TERRAIN_MAX_HEIGHT_CM    = 1800.0f;
    static constexpr float TERRAIN_HEIGHT_RANGE_CM  = TERRAIN_MAX_HEIGHT_CM - TERRAIN_MIN_HEIGHT_CM;

    // Biome radii (from world center, cm)
    static constexpr float BIOME_FOREST_RADIUS      = 12000.0f;
    static constexpr float BIOME_PLAINS_RADIUS      = 8000.0f;
    static constexpr float BIOME_ROCKY_RADIUS       = 6000.0f;
    static constexpr float BIOME_RIVER_WIDTH        = 800.0f;
    static constexpr float BIOME_VOLCANO_RADIUS     = 3000.0f;

    // River path (diagonal NW to SE)
    static constexpr float RIVER_START_X           = -12000.0f;
    static constexpr float RIVER_START_Y           = -8000.0f;
    static constexpr float RIVER_END_X             = 12000.0f;
    static constexpr float RIVER_END_Y             = 8000.0f;
    static constexpr float RIVER_DEPTH_CM          = 120.0f;

    // Volcano center
    static constexpr float VOLCANO_CENTER_X        = 8000.0f;
    static constexpr float VOLCANO_CENTER_Y        = 8000.0f;
    static constexpr float VOLCANO_PEAK_HEIGHT     = 1200.0f;

    // Performance (from PerformanceBudgetConfig.h)
    static constexpr float PCG_CULL_DISTANCE_CM    = 20000.0f;
    static constexpr float PCG_LOD3_DISTANCE_CM    = 8000.0f;
    static constexpr int32 MAX_ACTORS_PER_CELL     = 500;
    static constexpr float DINO_SLEEP_DISTANCE_CM  = 15000.0f;
}

// ============================================================
// UObject config asset for Blueprint-editable terrain params
// ============================================================
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UWorld_BiomeTerrainConfig : public UObject
{
    GENERATED_BODY()

public:
    UWorld_BiomeTerrainConfig();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Terrain")
    TArray<FWorld_BiomeTerrainParams> BiomeParams;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Terrain")
    float GlobalHeightScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Terrain")
    float GlobalNoiseScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Terrain")
    bool bEnableRiverCarving = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Terrain")
    bool bEnableVolcanicFeatures = true;

    /** Returns default biome params for a given biome type */
    UFUNCTION(BlueprintCallable, Category = "World|Terrain")
    FWorld_BiomeTerrainParams GetBiomeParams(EWorld_BiomeType BiomeType) const;

    /** Returns the biome type at a given world XY position */
    UFUNCTION(BlueprintCallable, Category = "World|Terrain")
    EWorld_BiomeType GetBiomeAtLocation(float WorldX, float WorldY) const;
};
