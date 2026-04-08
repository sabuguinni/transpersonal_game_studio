// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/World.h"
#include "Landscape/Landscape.h"
#include "PCGComponent.h"
#include "PCGGraph.h"
#include "PCGData.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "WorldGenerationSubsystem.generated.h"

class ALandscape;
class UPCGComponent;
class UPCGGraph;
class AWaterBody;
class UWorldPartitionSubsystem;

/**
 * @brief World Generation Subsystem for Transpersonal Game Studio
 * 
 * Manages procedural generation of the prehistoric world using UE5's PCG Framework.
 * Creates terrains, biomes, rivers, vegetation, and geological features that support
 * the survival gameplay loop in a Jurassic/Cretaceous period setting.
 * 
 * Core Features:
 * - Landscape generation with realistic heightmaps
 * - Biome distribution based on climate simulation
 * - River and water system generation
 * - Vegetation placement using PCG
 * - Geological feature placement (caves, cliffs, valleys)
 * - Performance-optimized streaming with World Partition
 * - Hierarchical generation for different detail levels
 * 
 * Technical Implementation:
 * - Uses PCG Framework for all procedural generation
 * - Integrates with World Partition for streaming
 * - Supports Runtime Generation for dynamic content
 * - GPU-accelerated where possible for performance
 * - Respects performance budgets from Performance Optimizer
 * 
 * @author Procedural World Generator — Agent #5
 * @version 1.0 — March 2026
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UWorldGenerationSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UWorldGenerationSubsystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    /** Main world generation functions */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateWorld(const FWorldGenerationSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void RegenerateRegion(const FBox& WorldBounds);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void ClearGeneratedContent();

    /** Landscape generation */
    UFUNCTION(BlueprintCallable, Category = "Landscape")
    ALandscape* GenerateLandscape(const FLandscapeGenerationSettings& Settings);

    /** Biome system */
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    void GenerateBiomes(const FBiomeGenerationSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    EBiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    /** Water system integration */
    UFUNCTION(BlueprintCallable, Category = "Water")
    void GenerateWaterSystems(const FWaterGenerationSettings& Settings);

    /** Vegetation and props */
    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void GenerateVegetation(const FVegetationSettings& Settings);

    /** Geological features */
    UFUNCTION(BlueprintCallable, Category = "Geology")
    void GenerateGeologicalFeatures(const FGeologySettings& Settings);

    /** Performance and streaming */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceBudget(const FWorldGenPerformanceBudget& Budget);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FWorldGenPerformanceStats GetCurrentPerformanceStats() const;

protected:
    /** World generation settings structure */
    USTRUCT(BlueprintType)
    struct FWorldGenerationSettings
    {
        GENERATED_BODY()

        /** World size in Unreal units (default: 8km x 8km) */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FVector2D WorldSize = FVector2D(800000.0f, 800000.0f);

        /** Random seed for generation */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        int32 Seed = 12345;

        /** Generation quality level */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        EWorldGenQuality Quality = EWorldGenQuality::High;

        /** Enable runtime generation */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        bool bEnableRuntimeGeneration = true;

        /** Use GPU acceleration where possible */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        bool bUseGPUAcceleration = true;

        /** World Partition cell size */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float PartitionCellSize = 25600.0f; // 256m cells

        /** Climate settings */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FClimateSettings Climate;

        /** Landscape settings */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FLandscapeGenerationSettings Landscape;

        /** Biome settings */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FBiomeGenerationSettings Biomes;

        /** Water settings */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FWaterGenerationSettings Water;

        /** Vegetation settings */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FVegetationSettings Vegetation;

        /** Geology settings */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FGeologySettings Geology;
    };

    /** Generation quality levels */
    UENUM(BlueprintType)
    enum class EWorldGenQuality : uint8
    {
        Low         UMETA(DisplayName = "Low Quality"),
        Medium      UMETA(DisplayName = "Medium Quality"),
        High        UMETA(DisplayName = "High Quality"),
        Epic        UMETA(DisplayName = "Epic Quality"),
        Cinematic   UMETA(DisplayName = "Cinematic Quality")
    };

    /** Biome types for prehistoric world */
    UENUM(BlueprintType)
    enum class EBiomeType : uint8
    {
        TropicalRainforest      UMETA(DisplayName = "Tropical Rainforest"),
        TemperateForest         UMETA(DisplayName = "Temperate Forest"),
        ConiferousForest        UMETA(DisplayName = "Coniferous Forest"),
        Grassland               UMETA(DisplayName = "Grassland"),
        Savanna                 UMETA(DisplayName = "Savanna"),
        Desert                  UMETA(DisplayName = "Desert"),
        Wetland                 UMETA(DisplayName = "Wetland"),
        CoastalPlain            UMETA(DisplayName = "Coastal Plain"),
        MountainousRegion       UMETA(DisplayName = "Mountainous Region"),
        VolcanicRegion          UMETA(DisplayName = "Volcanic Region"),
        RiverDelta              UMETA(DisplayName = "River Delta"),
        LakeShore               UMETA(DisplayName = "Lake Shore")
    };

    /** Climate settings for prehistoric period */
    USTRUCT(BlueprintType)
    struct FClimateSettings
    {
        GENERATED_BODY()

        /** Global temperature (Cretaceous was warmer) */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float GlobalTemperature = 25.0f; // Celsius

        /** Humidity levels (0-1) */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float GlobalHumidity = 0.7f;

        /** CO2 levels (higher in Cretaceous) */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float CO2Level = 1800.0f; // ppm

        /** Seasonal variation strength */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float SeasonalVariation = 0.3f;

        /** Prevailing wind direction */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FVector WindDirection = FVector(1.0f, 0.0f, 0.0f);

        /** Wind strength */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float WindStrength = 10.0f; // m/s
    };

    /** Landscape generation settings */
    USTRUCT(BlueprintType)
    struct FLandscapeGenerationSettings
    {
        GENERATED_BODY()

        /** Heightmap resolution */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        int32 HeightmapResolution = 4033;

        /** Maximum elevation in cm */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float MaxElevation = 500000.0f; // 5km

        /** Minimum elevation in cm */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float MinElevation = -50000.0f; // 500m below sea level

        /** Terrain roughness (0-1) */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float TerrainRoughness = 0.6f;

        /** Mountain frequency */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float MountainFrequency = 0.3f;

        /** Valley depth multiplier */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float ValleyDepth = 0.8f;

        /** Erosion simulation strength */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float ErosionStrength = 0.5f;

        /** Tectonic activity level */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float TectonicActivity = 0.7f; // Higher in Mesozoic

        /** Use Perlin noise for base terrain */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        bool bUsePerlinNoise = true;

        /** Use fractal brownian motion */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        bool bUseFractalNoise = true;

        /** Noise octaves */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        int32 NoiseOctaves = 6;

        /** Noise frequency */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float NoiseFrequency = 0.001f;

        /** Noise amplitude */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float NoiseAmplitude = 1.0f;
    };

    /** Biome generation settings */
    USTRUCT(BlueprintType)
    struct FBiomeGenerationSettings
    {
        GENERATED_BODY()

        /** Temperature gradient from equator to poles */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float TemperatureGradient = 30.0f;

        /** Precipitation patterns */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float AverageRainfall = 1500.0f; // mm/year

        /** Elevation affects on climate */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float ElevationClimateEffect = 0.6f;

        /** Distance from water affects humidity */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float WaterProximityEffect = 0.8f;

        /** Biome transition smoothness */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float BiomeTransitionSmoothing = 0.3f;

        /** Biome weights for different types */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        TMap<EBiomeType, float> BiomeWeights;

        /** Minimum biome patch size */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float MinBiomePatchSize = 50000.0f; // 500m

        /** Maximum biome patch size */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float MaxBiomePatchSize = 200000.0f; // 2km
    };

    /** Water system generation settings */
    USTRUCT(BlueprintType)
    struct FWaterGenerationSettings
    {
        GENERATED_BODY()

        /** Sea level in cm */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float SeaLevel = 0.0f;

        /** Generate rivers */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        bool bGenerateRivers = true;

        /** Generate lakes */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        bool bGenerateLakes = true;

        /** Generate coastal areas */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        bool bGenerateCoastalAreas = true;

        /** River density (rivers per km²) */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float RiverDensity = 0.1f;

        /** Minimum river length */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float MinRiverLength = 10000.0f; // 100m

        /** Maximum river length */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float MaxRiverLength = 500000.0f; // 5km

        /** River meandering factor */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float RiverMeandering = 0.3f;

        /** Lake size variation */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float LakeSizeVariation = 0.5f;

        /** Water flow simulation */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        bool bSimulateWaterFlow = true;
    };

    /** Vegetation generation settings */
    USTRUCT(BlueprintType)
    struct FVegetationSettings
    {
        GENERATED_BODY()

        /** Overall vegetation density */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float VegetationDensity = 0.8f;

        /** Tree density per biome */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        TMap<EBiomeType, float> TreeDensity;

        /** Undergrowth density per biome */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        TMap<EBiomeType, float> UndergrowthDensity;

        /** Grass coverage per biome */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        TMap<EBiomeType, float> GrassCoverage;

        /** Use hierarchical generation */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        bool bUseHierarchicalGeneration = true;

        /** Enable GPU generation */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        bool bUseGPUGeneration = true;

        /** Vegetation LOD distances */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        TArray<float> LODDistances = {5000.0f, 15000.0f, 50000.0f};

        /** Maximum vegetation instances per cell */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        int32 MaxInstancesPerCell = 10000;

        /** Vegetation variation seed */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        int32 VegetationSeed = 54321;
    };

    /** Geological features settings */
    USTRUCT(BlueprintType)
    struct FGeologySettings
    {
        GENERATED_BODY()

        /** Generate cave systems */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        bool bGenerateCaves = true;

        /** Generate cliff faces */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        bool bGenerateCliffs = true;

        /** Generate rock formations */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        bool bGenerateRockFormations = true;

        /** Generate volcanic features */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        bool bGenerateVolcanicFeatures = true;

        /** Cave system density */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float CaveDensity = 0.05f;

        /** Cave system complexity */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float CaveComplexity = 0.6f;

        /** Cliff generation threshold */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float CliffThreshold = 45.0f; // degrees

        /** Rock formation frequency */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float RockFormationFrequency = 0.1f;

        /** Volcanic activity level */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float VolcanicActivity = 0.3f;
    };

    /** Performance budget for world generation */
    USTRUCT(BlueprintType)
    struct FWorldGenPerformanceBudget
    {
        GENERATED_BODY()

        /** Maximum generation time per frame (ms) */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float MaxGenerationTimePerFrame = 5.0f;

        /** Maximum memory usage for generation (MB) */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float MaxMemoryUsage = 2048.0f;

        /** Maximum concurrent PCG components */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        int32 MaxConcurrentPCGComponents = 16;

        /** Target LOD level for generation */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        int32 TargetLODLevel = 2;

        /** Use GPU acceleration */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        bool bUseGPUAcceleration = true;

        /** Enable streaming generation */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        bool bEnableStreamingGeneration = true;
    };

    /** Performance statistics */
    USTRUCT(BlueprintType)
    struct FWorldGenPerformanceStats
    {
        GENERATED_BODY()

        /** Current generation time per frame */
        UPROPERTY(BlueprintReadOnly)
        float CurrentGenerationTime = 0.0f;

        /** Current memory usage */
        UPROPERTY(BlueprintReadOnly)
        float CurrentMemoryUsage = 0.0f;

        /** Active PCG components */
        UPROPERTY(BlueprintReadOnly)
        int32 ActivePCGComponents = 0;

        /** Generated cells this frame */
        UPROPERTY(BlueprintReadOnly)
        int32 GeneratedCellsThisFrame = 0;

        /** Total generated cells */
        UPROPERTY(BlueprintReadOnly)
        int32 TotalGeneratedCells = 0;

        /** Generation efficiency (0-1) */
        UPROPERTY(BlueprintReadOnly)
        float GenerationEfficiency = 1.0f;
    };

private:
    /** Core generation functions */
    void InitializePCGSystem();
    void SetupWorldPartitionIntegration();
    void CreateMasterPCGGraph();
    void ConfigurePerformanceSettings();

    /** Landscape generation implementation */
    void GenerateHeightmap(const FLandscapeGenerationSettings& Settings);
    void ApplyErosionSimulation(const FLandscapeGenerationSettings& Settings);
    void GenerateTerrainMaterials();

    /** Biome system implementation */
    void CalculateClimateMap(const FClimateSettings& Climate);
    void GenerateBiomeMap(const FBiomeGenerationSettings& Settings);
    void ApplyBiomeTransitions();

    /** Water system implementation */
    void GenerateRiverNetwork(const FWaterGenerationSettings& Settings);
    void GenerateLakeSystems(const FWaterGenerationSettings& Settings);
    void SetupWaterFlow();

    /** Vegetation implementation */
    void SetupVegetationPCG();
    void GenerateTreeDistribution();
    void GenerateUndergrowth();
    void GenerateGrasslands();

    /** Geological features implementation */
    void GenerateCaveSystems(const FGeologySettings& Settings);
    void GenerateCliffFaces(const FGeologySettings& Settings);
    void GenerateRockFormations(const FGeologySettings& Settings);
    void GenerateVolcanicFeatures(const FGeologySettings& Settings);

    /** Performance monitoring */
    void UpdatePerformanceStats();
    void OptimizeGenerationPerformance();
    void ManageMemoryUsage();

    /** PCG system components */
    UPROPERTY()
    TObjectPtr<UPCGComponent> MasterPCGComponent;

    UPROPERTY()
    TObjectPtr<UPCGGraph> MasterPCGGraph;

    UPROPERTY()
    TArray<TObjectPtr<UPCGComponent>> BiomePCGComponents;

    UPROPERTY()
    TArray<TObjectPtr<UPCGComponent>> VegetationPCGComponents;

    /** Generated landscape reference */
    UPROPERTY()
    TObjectPtr<ALandscape> GeneratedLandscape;

    /** Water system references */
    UPROPERTY()
    TArray<TObjectPtr<AWaterBody>> GeneratedWaterBodies;

    /** Current generation settings */
    UPROPERTY()
    FWorldGenerationSettings CurrentSettings;

    /** Performance monitoring */
    UPROPERTY()
    FWorldGenPerformanceBudget PerformanceBudget;

    UPROPERTY()
    FWorldGenPerformanceStats CurrentStats;

    /** Generation state */
    UPROPERTY()
    bool bIsGenerating = false;

    UPROPERTY()
    bool bGenerationComplete = false;

    /** Climate and biome data */
    UPROPERTY()
    TArray<float> ClimateMap;

    UPROPERTY()
    TArray<EBiomeType> BiomeMap;

    /** Performance timer */
    FTimerHandle PerformanceUpdateTimer;

    /** World Partition integration */
    UPROPERTY()
    TObjectPtr<UWorldPartitionSubsystem> WorldPartitionSubsystem;

    /** Generation statistics */
    int32 TotalGenerationCalls = 0;
    float TotalGenerationTime = 0.0f;
    float AverageGenerationTime = 0.0f;
    int32 SuccessfulGenerations = 0;
    int32 FailedGenerations = 0;
};