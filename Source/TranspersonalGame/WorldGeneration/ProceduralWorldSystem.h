// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/DataAsset.h"
#include "Components/ActorComponent.h"
#include "Landscape/Landscape.h"
#include "PCGComponent.h"
#include "PCGGraph.h"
#include "PCGData.h"
#include "WorldPartition/WorldPartition.h"
#include "WorldPartition/DataLayer/DataLayer.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "ProceduralWorldSystem.generated.h"

/**
 * @brief Procedural World System for Jurassic Survival Game
 * 
 * Generates vast prehistoric landscapes using UE5's PCG Framework and World Partition.
 * Creates biomes, terrains, rivers, vegetation, and geological features based on
 * real-world geological principles and the game's narrative requirements.
 * 
 * Key Features:
 * - Hierarchical terrain generation (macro to micro detail)
 * - Biome-based ecosystem distribution
 * - Geological feature placement (caves, cliffs, valleys)
 * - Water system integration (rivers, lakes, wetlands)
 * - Performance-optimized streaming with World Partition
 * - Narrative location placement (gem locations, points of interest)
 * 
 * Biome Types:
 * - Dense Jungle (primary biome)
 * - River Plains (secondary biome)
 * - Rocky Highlands (tertiary biome)
 * - Wetlands (special areas)
 * - Volcanic Regions (rare, dangerous areas)
 * 
 * @author Procedural World Generator — Agent #5
 * @version 1.0 — March 2026
 */

DECLARE_LOG_CATEGORY_EXTERN(LogProceduralWorld, Log, All);

/** Biome types for the Jurassic world */
UENUM(BlueprintType)
enum class EJurassicBiome : uint8
{
    DenseJungle     UMETA(DisplayName = "Dense Jungle"),
    RiverPlains     UMETA(DisplayName = "River Plains"),
    RockyHighlands  UMETA(DisplayName = "Rocky Highlands"),
    Wetlands        UMETA(DisplayName = "Wetlands"),
    VolcanicRegion  UMETA(DisplayName = "Volcanic Region"),
    CaveSystem      UMETA(DisplayName = "Cave System"),
    Coastline       UMETA(DisplayName = "Coastline")
};

/** Geological feature types */
UENUM(BlueprintType)
enum class EGeologicalFeature : uint8
{
    Valley          UMETA(DisplayName = "Valley"),
    Ridge           UMETA(DisplayName = "Ridge"),
    Plateau         UMETA(DisplayName = "Plateau"),
    Canyon          UMETA(DisplayName = "Canyon"),
    Cave            UMETA(DisplayName = "Cave"),
    Cliff           UMETA(DisplayName = "Cliff"),
    Hill            UMETA(DisplayName = "Hill"),
    Depression      UMETA(DisplayName = "Depression")
};

/** Water feature types */
UENUM(BlueprintType)
enum class EWaterFeature : uint8
{
    River           UMETA(DisplayName = "River"),
    Stream          UMETA(DisplayName = "Stream"),
    Lake            UMETA(DisplayName = "Lake"),
    Pond            UMETA(DisplayName = "Pond"),
    Waterfall       UMETA(DisplayName = "Waterfall"),
    Rapids          UMETA(DisplayName = "Rapids"),
    Swamp           UMETA(DisplayName = "Swamp"),
    Spring          UMETA(DisplayName = "Spring")
};

/** Terrain generation parameters */
USTRUCT(BlueprintType)
struct FTerrainGenerationParams
{
    GENERATED_BODY()

    /** Base terrain scale in Unreal units */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scale")
    float TerrainScale = 100.0f;

    /** Height variation multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Height")
    float HeightMultiplier = 1000.0f;

    /** Maximum terrain height */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Height")
    float MaxHeight = 5000.0f;

    /** Noise octaves for terrain detail */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
    int32 NoiseOctaves = 6;

    /** Noise frequency for macro features */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
    float MacroFrequency = 0.001f;

    /** Noise frequency for micro details */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
    float MicroFrequency = 0.01f;

    /** Erosion simulation strength */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion")
    float ErosionStrength = 0.5f;

    /** River carving strength */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    float RiverCarvingStrength = 2.0f;

    /** Seed for procedural generation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 GenerationSeed = 12345;
};

/** Biome configuration data */
USTRUCT(BlueprintType)
struct FBiomeConfiguration
{
    GENERATED_BODY()

    /** Biome type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EJurassicBiome BiomeType = EJurassicBiome::DenseJungle;

    /** Height range for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
    FVector2D HeightRange = FVector2D(0.0f, 1000.0f);

    /** Slope range for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
    FVector2D SlopeRange = FVector2D(0.0f, 45.0f);

    /** Moisture requirements */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    FVector2D MoistureRange = FVector2D(0.3f, 1.0f);

    /** Temperature requirements */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    FVector2D TemperatureRange = FVector2D(0.6f, 1.0f);

    /** PCG Graph for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    TSoftObjectPtr<UPCGGraphAsset> BiomePCGGraph;

    /** Landscape material for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TSoftObjectPtr<UMaterialInterface> LandscapeMaterial;

    /** Vegetation density multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float VegetationDensity = 1.0f;

    /** Rock formation density */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geology")
    float RockDensity = 0.3f;

    /** Wildlife spawn probability */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wildlife")
    float WildlifeSpawnRate = 0.1f;

    /** Danger level (affects predator spawns) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    float DangerLevel = 0.5f;
};

/** Point of Interest data for narrative elements */
USTRUCT(BlueprintType)
struct FPointOfInterest
{
    GENERATED_BODY()

    /** Unique identifier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FString POIId;

    /** Display name */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FString DisplayName;

    /** World location */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Location")
    FVector WorldLocation = FVector::ZeroVector;

    /** Required biome for placement */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
    EJurassicBiome RequiredBiome = EJurassicBiome::DenseJungle;

    /** Minimum distance from other POIs */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
    float MinDistanceFromOthers = 5000.0f;

    /** Is this the final gem location? */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bIsFinalGem = false;

    /** Associated actor class to spawn */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    TSoftClassPtr<AActor> ActorClass;

    /** Custom PCG graph for this POI */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    TSoftObjectPtr<UPCGGraphAsset> CustomPCGGraph;
};

/** World generation configuration asset */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UJurassicWorldConfig : public UDataAsset
{
    GENERATED_BODY()

public:
    /** World size in kilometers */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Scale")
    float WorldSizeKm = 16.0f;

    /** World Partition cell size */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Streaming")
    float CellSizeMeters = 1600.0f;

    /** Loading radius for streaming */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Streaming")
    float LoadingRadiusMeters = 3200.0f;

    /** Terrain generation parameters */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain")
    FTerrainGenerationParams TerrainParams;

    /** Biome configurations */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biomes")
    TArray<FBiomeConfiguration> BiomeConfigurations;

    /** Points of interest for narrative */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narrative")
    TArray<FPointOfInterest> PointsOfInterest;

    /** Master PCG graph for world generation */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PCG")
    TSoftObjectPtr<UPCGGraphAsset> MasterPCGGraph;

    /** Landscape heightmap resolution */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Landscape")
    int32 HeightmapResolution = 8129;

    /** Performance quality level */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 QualityLevel = 3; // 0=Low, 1=Medium, 2=High, 3=Epic

    /** Enable runtime generation */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Runtime")
    bool bEnableRuntimeGeneration = true;

    /** Enable hierarchical generation */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Runtime")
    bool bEnableHierarchicalGeneration = true;
};

/** Main Procedural World System */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UProceduralWorldSystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UProceduralWorldSystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

    /** Initialize world generation with configuration */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    bool InitializeWorldGeneration(UJurassicWorldConfig* WorldConfig);

    /** Generate the complete world */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateWorld();

    /** Generate specific region */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateRegion(const FVector& CenterLocation, float Radius);

    /** Get biome at specific location */
    UFUNCTION(BlueprintCallable, Category = "World Query")
    EJurassicBiome GetBiomeAtLocation(const FVector& WorldLocation) const;

    /** Get terrain height at location */
    UFUNCTION(BlueprintCallable, Category = "World Query")
    float GetTerrainHeightAtLocation(const FVector& WorldLocation) const;

    /** Check if location is suitable for POI placement */
    UFUNCTION(BlueprintCallable, Category = "World Query")
    bool IsLocationSuitableForPOI(const FVector& WorldLocation, const FPointOfInterest& POI) const;

    /** Place narrative points of interest */
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlacePointsOfInterest();

    /** Get all placed POIs */
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FPointOfInterest> GetPlacedPointsOfInterest() const;

    /** Enable/disable runtime generation */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetRuntimeGenerationEnabled(bool bEnabled);

    /** Get world generation progress */
    UFUNCTION(BlueprintCallable, Category = "Progress")
    float GetGenerationProgress() const;

    /** Get generation status */
    UFUNCTION(BlueprintCallable, Category = "Progress")
    FString GetGenerationStatus() const;

protected:
    /** Initialize World Partition system */
    void InitializeWorldPartition();

    /** Setup PCG system */
    void SetupPCGSystem();

    /** Generate base landscape */
    void GenerateBaseLandscape();

    /** Generate biome distribution */
    void GenerateBiomeDistribution();

    /** Generate water features */
    void GenerateWaterFeatures();

    /** Generate geological features */
    void GenerateGeologicalFeatures();

    /** Apply erosion simulation */
    void ApplyErosionSimulation();

    /** Generate vegetation using PCG */
    void GenerateVegetation();

    /** Generate rock formations */
    void GenerateRockFormations();

    /** Setup streaming and LOD */
    void SetupStreamingAndLOD();

    /** Performance optimization */
    void OptimizeForPerformance();

    /** Biome calculation functions */
    float CalculateMoisture(const FVector& Location) const;
    float CalculateTemperature(const FVector& Location) const;
    float CalculateElevation(const FVector& Location) const;
    float CalculateSlope(const FVector& Location) const;

    /** Noise generation functions */
    float GeneratePerlinNoise(const FVector& Location, float Frequency, int32 Octaves) const;
    float GenerateRidgedNoise(const FVector& Location, float Frequency) const;
    float GenerateBillowNoise(const FVector& Location, float Frequency) const;

private:
    /** World configuration */
    UPROPERTY()
    UJurassicWorldConfig* CurrentWorldConfig;

    /** Main landscape actor */
    UPROPERTY()
    ALandscape* MainLandscape;

    /** PCG World Actor */
    UPROPERTY()
    class APCGWorldActor* PCGWorldActor;

    /** World Partition instance */
    UPROPERTY()
    UWorldPartition* WorldPartition;

    /** Generated biome map */
    UPROPERTY()
    TMap<FIntPoint, EJurassicBiome> BiomeMap;

    /** Placed points of interest */
    UPROPERTY()
    TArray<FPointOfInterest> PlacedPOIs;

    /** Generation progress tracking */
    UPROPERTY()
    float GenerationProgress = 0.0f;

    /** Generation status */
    UPROPERTY()
    FString GenerationStatus = TEXT("Not Started");

    /** Is generation in progress */
    UPROPERTY()
    bool bGenerationInProgress = false;

    /** Performance monitoring */
    UPROPERTY()
    class UPerformanceOptimizer* PerformanceOptimizer;

    /** Heightmap data cache */
    TArray<float> HeightmapData;

    /** Biome weight maps */
    TMap<EJurassicBiome, TArray<float>> BiomeWeightMaps;

    /** Water flow map for river generation */
    TArray<FVector2D> WaterFlowMap;

    /** Erosion map for terrain modification */
    TArray<float> ErosionMap;

    /** Generation seed */
    int32 CurrentSeed = 12345;

    /** World bounds */
    FBox WorldBounds;

    /** Cell grid for World Partition */
    FIntPoint GridSize;

    /** Async generation task handle */
    TSharedPtr<class FAsyncTask<class FWorldGenerationTask>> GenerationTask;
};

/** Async world generation task */
class FWorldGenerationTask : public FNonAbandonableTask
{
    friend class FAsyncTask<FWorldGenerationTask>;

public:
    FWorldGenerationTask(UProceduralWorldSystem* InWorldSystem, UJurassicWorldConfig* InConfig);

    void DoWork();

    FORCEINLINE TStatId GetStatId() const
    {
        RETURN_QUICK_DECLARE_CYCLE_STAT(FWorldGenerationTask, STATGROUP_ThreadPoolAsyncTasks);
    }

private:
    TWeakObjectPtr<UProceduralWorldSystem> WorldSystem;
    TWeakObjectPtr<UJurassicWorldConfig> WorldConfig;
};

/** PCG-based biome generator component */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(PCG))
class TRANSPERSONALGAME_API UJurassicBiomeGenerator : public UPCGComponent
{
    GENERATED_BODY()

public:
    UJurassicBiomeGenerator();

    /** Target biome for this generator */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EJurassicBiome TargetBiome = EJurassicBiome::DenseJungle;

    /** Biome configuration */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FBiomeConfiguration BiomeConfig;

    /** Generate biome content */
    UFUNCTION(BlueprintCallable, Category = "Generation")
    void GenerateBiomeContent();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};