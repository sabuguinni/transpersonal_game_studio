// BiomeTerrainGenerator.h
// Procedural World Generator — Agent #05
// Biome-aware terrain generation system for Cretaceous prehistoric world
// Generates terrain height data and biome classification for MinPlayableMap

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "BiomeTerrainGenerator.generated.h"

// ============================================================
// ENUMS — must be at global scope (UHT rule)
// ============================================================

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Plains       UMETA(DisplayName = "Plains"),
    Forest       UMETA(DisplayName = "Forest"),
    Rocky        UMETA(DisplayName = "Rocky Highlands"),
    River        UMETA(DisplayName = "River Valley"),
    Volcanic     UMETA(DisplayName = "Volcanic Region"),
    Swamp        UMETA(DisplayName = "Swamp"),
    COUNT        UMETA(Hidden)
};

// ============================================================
// STRUCTS — must be at global scope (UHT rule)
// ============================================================

USTRUCT(BlueprintType)
struct FWorld_TerrainTileData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "World|Terrain")
    int32 GridX = 0;

    UPROPERTY(BlueprintReadWrite, Category = "World|Terrain")
    int32 GridY = 0;

    UPROPERTY(BlueprintReadWrite, Category = "World|Terrain")
    float HeightOffset = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "World|Terrain")
    EWorld_BiomeType Biome = EWorld_BiomeType::Plains;

    UPROPERTY(BlueprintReadWrite, Category = "World|Terrain")
    float BiomeBlendWeight = 1.0f;
};

USTRUCT(BlueprintType)
struct FWorld_BiomeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Plains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float MinHeight = -500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float MaxHeight = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float VegetationDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float DinosaurSpawnWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    FLinearColor DebugColor = FLinearColor::Green;
};

// ============================================================
// MAIN CLASS
// ============================================================

UCLASS(ClassGroup = (World), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ABiomeTerrainGenerator : public AActor
{
    GENERATED_BODY()

public:
    ABiomeTerrainGenerator();

    // ---- Configuration ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Terrain|Config")
    int32 GridWidth = 7;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Terrain|Config")
    int32 GridHeight = 7;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Terrain|Config")
    float TileSize = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Terrain|Config")
    float MaxHeightVariation = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Terrain|Config")
    float HeightSeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome|Config")
    TArray<FWorld_BiomeConfig> BiomeConfigs;

    // ---- Runtime Data ----

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World|Terrain|Runtime")
    TArray<FWorld_TerrainTileData> TileData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World|Terrain|Runtime")
    int32 TotalTilesGenerated = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World|Terrain|Runtime")
    float GeneratedHeightMin = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World|Terrain|Runtime")
    float GeneratedHeightMax = 0.0f;

    // ---- Public API ----

    /** Generate all terrain tile data (height + biome). Call this before spawning tiles. */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "World|Terrain")
    void GenerateTerrainData();

    /** Get height at a world position using bilinear interpolation */
    UFUNCTION(BlueprintCallable, Category = "World|Terrain")
    float GetHeightAtWorldPosition(float WorldX, float WorldY) const;

    /** Get biome type at a world position */
    UFUNCTION(BlueprintCallable, Category = "World|Terrain")
    EWorld_BiomeType GetBiomeAtWorldPosition(float WorldX, float WorldY) const;

    /** Get all tiles belonging to a specific biome */
    UFUNCTION(BlueprintCallable, Category = "World|Terrain")
    TArray<FWorld_TerrainTileData> GetTilesByBiome(EWorld_BiomeType BiomeType) const;

    /** Returns true if terrain data has been generated */
    UFUNCTION(BlueprintCallable, Category = "World|Terrain")
    bool IsTerrainGenerated() const { return TotalTilesGenerated > 0; }

    /** Get tile data at grid coordinates */
    UFUNCTION(BlueprintCallable, Category = "World|Terrain")
    bool GetTileDataAt(int32 GX, int32 GY, FWorld_TerrainTileData& OutTile) const;

protected:
    virtual void BeginPlay() override;

private:
    /** Compute height using multi-frequency sine waves (deterministic, no random) */
    float ComputeTerrainHeight(float NormX, float NormY) const;

    /** Classify biome based on height and distance from center */
    EWorld_BiomeType ClassifyBiome(float NormX, float NormY, float Height) const;

    bool bTerrainGenerated = false;
};
