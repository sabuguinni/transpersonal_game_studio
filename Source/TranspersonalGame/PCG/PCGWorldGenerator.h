// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "PCGComponent.h"
#include "PCGGraph.h"
#include "PCGData.h"
#include "PCGSettings.h"
#include "Engine/World.h"
#include "Landscape.h"
#include "LandscapeProxy.h"
#include "WorldPartition/WorldPartition.h"
#include "WorldPartition/DataLayer/DataLayer.h"
#include "PCGWorldGenerator.generated.h"

class UPCGGraph;
class UPCGComponent;
class ALandscape;
class ULandscapeComponent;
class UDataLayer;

/**
 * @brief Prehistoric World Generation System using UE5 PCG Framework
 * 
 * Creates vast prehistoric landscapes with:
 * - Procedural terrain generation using Perlin noise and geological simulation
 * - Biome distribution based on climate and elevation
 * - River systems with realistic flow patterns
 * - Vegetation placement following ecological principles
 * - World Partition integration for streaming large worlds
 * 
 * Design Philosophy:
 * - Every hill has a reason - geological history drives terrain
 * - Rivers determine civilization - water sources shape where life thrives
 * - Biomes transition naturally - no hard boundaries in nature
 * - Performance first - beautiful worlds that run smoothly
 * 
 * @author Procedural World Generator — Agent #5
 * @version 1.0 — March 2026
 */

/** Geological formation types for terrain generation */
UENUM(BlueprintType)
enum class EGeologicalFormation : uint8
{
    Plains              UMETA(DisplayName = "Plains"),
    Hills               UMETA(DisplayName = "Hills"),
    Mountains           UMETA(DisplayName = "Mountains"),
    Valleys             UMETA(DisplayName = "Valleys"),
    Plateaus            UMETA(DisplayName = "Plateaus"),
    Canyons             UMETA(DisplayName = "Canyons"),
    CoastalCliffs       UMETA(DisplayName = "Coastal Cliffs"),
    VolcanicCones       UMETA(DisplayName = "Volcanic Cones"),
    RiverDeltas         UMETA(DisplayName = "River Deltas")
};

/** Biome types for prehistoric world */
UENUM(BlueprintType)
enum class EPCG_PrehistoricBiome : uint8
{
    TropicalRainforest  UMETA(DisplayName = "Tropical Rainforest"),
    TemperateForest     UMETA(DisplayName = "Temperate Forest"),
    ConiferousForest    UMETA(DisplayName = "Coniferous Forest"),
    Swampland           UMETA(DisplayName = "Swampland"),
    Grasslands          UMETA(DisplayName = "Grasslands"),
    Scrubland           UMETA(DisplayName = "Scrubland"),
    RiverPlains         UMETA(DisplayName = "River Plains"),
    CoastalMarsh        UMETA(DisplayName = "Coastal Marsh"),
    VolcanicWasteland   UMETA(DisplayName = "Volcanic Wasteland")
};

/** Water body types */
UENUM(BlueprintType)
enum class EPCG_WaterBodyType : uint8
{
    River               UMETA(DisplayName = "River"),
    Stream              UMETA(DisplayName = "Stream"),
    Lake                UMETA(DisplayName = "Lake"),
    Pond                UMETA(DisplayName = "Pond"),
    Swamp               UMETA(DisplayName = "Swamp"),
    CoastalWater        UMETA(DisplayName = "Coastal Water")
};

/** Terrain generation parameters */
USTRUCT(BlueprintType)
struct FPCG_TerrainGenerationParams
{
    GENERATED_BODY()

    /** Base height scale in centimeters */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float BaseHeightScale = 50000.0f;

    /** Primary noise frequency for large features */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
    float PrimaryNoiseFreq = 0.0005f;

    /** Secondary noise frequency for medium features */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
    float SecondaryNoiseFreq = 0.002f;

    /** Detail noise frequency for small features */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
    float DetailNoiseFreq = 0.01f;

    /** Erosion simulation strength */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion")
    float ErosionStrength = 0.3f;

    /** Thermal erosion iterations */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion")
    int32 ThermalErosionIterations = 5;

    /** Hydraulic erosion iterations */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion")
    int32 HydraulicErosionIterations = 10;

    /** Geological formation influence */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geology")
    TMap<EGeologicalFormation, float> FormationInfluence;
};

/** Biome distribution parameters */
USTRUCT(BlueprintType)
struct FBiomeDistributionParams
{
    GENERATED_BODY()

    /** Temperature gradient (affects biome placement) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float TemperatureGradient = 0.001f;

    /** Moisture distribution noise scale */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float MoistureNoiseScale = 0.0008f;

    /** Elevation influence on biome selection */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float ElevationInfluence = 0.5f;

    /** Biome transition smoothness */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blending")
    float TransitionSmoothness = 1000.0f;

    /** Biome weights by type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution")
    TMap<EPCG_PrehistoricBiome, float> BiomeWeights;
};

/** River generation parameters */
USTRUCT(BlueprintType)
struct FRiverGenerationParams
{
    GENERATED_BODY()

    /** Number of major rivers */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rivers")
    int32 MajorRiverCount = 3;

    /** Number of tributaries per major river */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rivers")
    int32 TributariesPerRiver = 5;

    /** Minimum river length in meters */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rivers")
    float MinRiverLength = 5000.0f;

    /** Maximum river length in meters */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rivers")
    float MaxRiverLength = 20000.0f;

    /** River width variation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rivers")
    FVector2D RiverWidthRange = FVector2D(200.0f, 800.0f);

    /** River depth variation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rivers")
    FVector2D RiverDepthRange = FVector2D(100.0f, 500.0f);

    /** Flow speed range (m/s) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rivers")
    FVector2D FlowSpeedRange = FVector2D(0.5f, 3.0f);
};

/** Vegetation generation parameters */
USTRUCT(BlueprintType)
struct FVegetationGenerationParams
{
    GENERATED_BODY()

    /** Overall vegetation density multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Density")
    float DensityMultiplier = 1.0f;

    /** Tree spacing variation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trees")
    FVector2D TreeSpacingRange = FVector2D(300.0f, 800.0f);

    /** Understory plant density */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Understory")
    float UnderstoryDensity = 0.7f;

    /** Ground cover density */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground Cover")
    float GroundCoverDensity = 1.5f;

    /** Vegetation LOD distances */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    TArray<float> LODDistances = {2000.0f, 5000.0f, 10000.0f};

    /** Maximum vegetation instances per cell */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxInstancesPerCell = 10000;
};

/**
 * @brief PCG-based World Generator for Prehistoric Environments
 * 
 * Manages the complete procedural generation pipeline:
 * 1. Terrain generation with geological simulation
 * 2. Biome distribution based on climate modeling
 * 3. River system generation with realistic flow
 * 4. Vegetation placement following ecological rules
 * 5. World Partition setup for streaming
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APCGWorldGenerator : public AActor
{
    GENERATED_BODY()

public:
    APCGWorldGenerator();

protected:
    virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform& Transform) override;

    /** Main PCG component for world generation */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PCG")
    TObjectPtr<UPCGComponent> MainPCGComponent;

    /** PCG Graph for terrain generation */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PCG Graphs")
    TObjectPtr<UPCGGraph> TerrainGenerationGraph;

    /** PCG Graph for biome distribution */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PCG Graphs")
    TObjectPtr<UPCGGraph> BiomeDistributionGraph;

    /** PCG Graph for river generation */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PCG Graphs")
    TObjectPtr<UPCGGraph> RiverGenerationGraph;

    /** PCG Graph for vegetation placement */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PCG Graphs")
    TObjectPtr<UPCGGraph> VegetationGraph;

    /** World size in kilometers */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
    float WorldSizeKm = 16.0f;

    /** Landscape resolution (must be valid UE5 size) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
    int32 LandscapeResolution = 8129; // 8129x8129 for optimal performance

    /** World Partition cell size in meters */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Partition")
    float PartitionCellSize = 25600.0f; // 256m cells

    /** Enable hierarchical generation for performance */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableHierarchicalGeneration = true;

    /** Enable runtime generation for streaming */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableRuntimeGeneration = true;

    /** Generation parameters */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Parameters")
    FPCG_TerrainGenerationParams TerrainParams;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Parameters")
    FBiomeDistributionParams BiomeParams;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Parameters")
    FRiverGenerationParams RiverParams;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Parameters")
    FVegetationGenerationParams VegetationParams;

    /** Data Layers for organization */
// [UHT-FIX]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Layers")
    TObjectPtr<UDataLayer> TerrainDataLayer;

// [UHT-FIX]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Layers")
    TObjectPtr<UDataLayer> VegetationDataLayer;

// [UHT-FIX]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Layers")
    TObjectPtr<UDataLayer> WaterDataLayer;

// [UHT-FIX]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Layers")
    TObjectPtr<UDataLayer> GeologyDataLayer;

public:
    /** Generate the complete prehistoric world */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateWorld();

    /** Generate only the base terrain */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateTerrain();

    /** Generate biome distribution */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateBiomes();

    /** Generate river systems */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateRivers();

    /** Generate vegetation for all biomes */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateVegetation();

    /** Clear all generated content */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void ClearGeneratedContent();

    /** Setup World Partition for streaming */
    UFUNCTION(BlueprintCallable, Category = "World Partition")
    void SetupWorldPartition();

    /** Get biome type at world location */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "World Query")
    EPCG_PrehistoricBiome GetBiomeAtLocation(const FVector& WorldLocation) const;

    /** Get terrain height at location */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "World Query")
    float GetTerrainHeightAtLocation(const FVector& WorldLocation) const;

    /** Check if location is near water */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "World Query")
    bool IsLocationNearWater(const FVector& WorldLocation, float MaxDistance = 1000.0f) const;

    /** Find suitable locations for player base */
    UFUNCTION(BlueprintCallable, Category = "World Query")
    TArray<FVector> FindBaseBuildingLocations(int32 MaxLocations = 10) const;

    /** Find locations suitable for dinosaur spawning */
    UFUNCTION(BlueprintCallable, Category = "World Query")
    TArray<FVector> FindDinosaurSpawnLocations(EPCG_PrehistoricBiome PreferredBiome, int32 MaxLocations = 20) const;

    /** Get generation progress (0-1) */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "World Generation")
    float GetGenerationProgress() const;

    /** Check if world generation is complete */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "World Generation")
    bool IsWorldGenerationComplete() const;

protected:
    /** Initialize PCG components and graphs */
    void InitializePCGSystem();

    /** Create and configure landscape */
    void CreateLandscape();

    /** Apply geological formations */
    void ApplyGeologicalFormations();

    /** Calculate biome weights at location */
    TMap<EPCG_PrehistoricBiome, float> CalculateBiomeWeights(const FVector& Location) const;

    /** Generate noise for terrain */
    float GenerateTerrainNoise(const FVector2D& Location) const;

    /** Generate moisture map */
    float GenerateMoistureNoise(const FVector2D& Location) const;

    /** Generate temperature map */
    float GenerateTemperatureNoise(const FVector2D& Location) const;

    /** Create river spline from heightmap analysis */
    TArray<FVector> GenerateRiverPath(const FVector& StartLocation, const FVector& EndLocation) const;

    /** Apply erosion effects to heightmap */
    void ApplyErosionEffects();

    /** Configure PCG for performance */
    void ConfigurePCGPerformance();

    /** Setup data layers for organization */
    void SetupDataLayers();

private:
    /** Generated landscape reference */
    UPROPERTY()
    TObjectPtr<ALandscape> GeneratedLandscape;

    /** Generation progress tracking */
    float GenerationProgress = 0.0f;

    /** Generation completion flag */
    bool bGenerationComplete = false;

    /** Cached heightmap data for queries */
    TArray<float> CachedHeightData;

    /** Cached biome data for queries */
    TArray<uint8> CachedBiomeData;

    /** Random seed for consistent generation */
    int32 GenerationSeed = 12345;
};