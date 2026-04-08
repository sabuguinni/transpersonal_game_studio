// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/World.h"
#include "Landscape/Landscape.h"
#include "PCGComponent.h"
#include "PCGGraph.h"
#include "PCGData.h"
#include "PCGSubsystem.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "WorldPartition/WorldPartitionSubsystem.h"
#include "../Performance/PerformanceTargets.h"
#include "ProceduralWorldGeneratorV5.generated.h"

class ALandscape;
class UPCGComponent;
class UPCGGraph;
class AWaterBody;
class UWorldPartitionSubsystem;

/**
 * @brief Procedural World Generator V5 — Transpersonal Game Studio
 * 
 * Advanced procedural world generation system for the prehistoric survival game.
 * Creates a living, breathing Jurassic/Cretaceous world where dinosaurs roam freely.
 * 
 * CORE PHILOSOPHY:
 * - Every valley has a reason to exist
 * - Every river carved its path through geological time
 * - Every forest grew where conditions allowed it
 * - The world existed before the player arrived
 * 
 * TECHNICAL APPROACH:
 * - Uses UE5 PCG Framework for all generation
 * - Integrates with World Partition for seamless streaming
 * - GPU-accelerated where possible for performance
 * - Hierarchical generation from continental scale to grass blade
 * - Respects performance budgets from Performance Optimizer
 * 
 * WORLD SCALE:
 * - 8km x 8km playable region (expandable to 64km x 64km)
 * - Multiple biomes based on realistic climate simulation
 * - River systems that follow realistic hydrology
 * - Geological features based on Mesozoic geology
 * 
 * @author Procedural World Generator — Agent #5
 * @version 5.0 — March 2026
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UProceduralWorldGeneratorV5 : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UProceduralWorldGeneratorV5();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    /** Main world generation entry point */
    UFUNCTION(BlueprintCallable, Category = "World Generation", CallInEditor = true)
    void GeneratePrehistoricWorld(const FWorldGenerationConfig& Config);

    /** Generate specific world layers */
    UFUNCTION(BlueprintCallable, Category = "Terrain", CallInEditor = true)
    void GenerateBaseTerrain(const FTerrainConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Hydrology", CallInEditor = true)
    void GenerateWaterSystems(const FHydrologyConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Climate", CallInEditor = true)
    void GenerateClimateZones(const FClimateConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Biomes", CallInEditor = true)
    void GenerateBiomes(const FBiomeConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Geology", CallInEditor = true)
    void GenerateGeologicalFeatures(const FGeologyConfig& Config);

    /** Performance and optimization */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceBudget(const FPerformanceBudget& Budget);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FWorldGenPerformanceStats GetCurrentPerformanceStats() const;

    /** Runtime generation for dynamic content */
    UFUNCTION(BlueprintCallable, Category = "Runtime")
    void EnableRuntimeGeneration(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Runtime")
    void RegenerateRegion(const FBox& WorldBounds, float Priority = 1.0f);

protected:
    /** World generation configuration */
    USTRUCT(BlueprintType)
    struct FWorldGenerationConfig
    {
        GENERATED_BODY()

        /** World size in Unreal units (default: 8km x 8km) */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scale")
        FVector2D WorldSize = FVector2D(800000.0f, 800000.0f);

        /** Random seed for reproducible generation */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
        int32 Seed = 12345;

        /** Generation quality level */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
        EWorldGenQuality Quality = EWorldGenQuality::High;

        /** Time period (affects flora and geological features) */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting")
        EGeologicalPeriod TimePeriod = EGeologicalPeriod::LateCretaceous;

        /** Geographic region type */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting")
        EGeographicRegion RegionType = EGeographicRegion::TropicalLowlands;

        /** Enable real-time streaming */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
        bool bEnableWorldPartition = true;

        /** World Partition cell size */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
        float PartitionCellSize = 12800.0f; // 128m cells for performance

        /** Use GPU acceleration where possible */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
        bool bUseGPUAcceleration = true;

        /** Enable hierarchical generation */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
        bool bEnableHierarchicalGeneration = true;

        /** Terrain configuration */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
        FTerrainConfig Terrain;

        /** Hydrology configuration */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hydrology")
        FHydrologyConfig Hydrology;

        /** Climate configuration */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
        FClimateConfig Climate;

        /** Biome configuration */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
        FBiomeConfig Biomes;

        /** Geology configuration */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geology")
        FGeologyConfig Geology;
    };

    /** Generation quality levels */
    UENUM(BlueprintType)
    enum class EWorldGenQuality : uint8
    {
        Low         UMETA(DisplayName = "Low - Fast Preview"),
        Medium      UMETA(DisplayName = "Medium - Balanced"),
        High        UMETA(DisplayName = "High - Detailed"),
        Epic        UMETA(DisplayName = "Epic - Maximum Detail"),
        Cinematic   UMETA(DisplayName = "Cinematic - Ultra Quality")
    };

    /** Geological time periods */
    UENUM(BlueprintType)
    enum class EGeologicalPeriod : uint8
    {
        EarlyJurassic       UMETA(DisplayName = "Early Jurassic (201-174 Ma)"),
        MiddleJurassic      UMETA(DisplayName = "Middle Jurassic (174-163 Ma)"),
        LateJurassic        UMETA(DisplayName = "Late Jurassic (163-145 Ma)"),
        EarlyCretaceous     UMETA(DisplayName = "Early Cretaceous (145-100 Ma)"),
        LateCretaceous      UMETA(DisplayName = "Late Cretaceous (100-66 Ma)")
    };

    /** Geographic region types */
    UENUM(BlueprintType)
    enum class EGeographicRegion : uint8
    {
        TropicalLowlands        UMETA(DisplayName = "Tropical Lowlands"),
        TemperateHighlands      UMETA(DisplayName = "Temperate Highlands"),
        CoastalPlains           UMETA(DisplayName = "Coastal Plains"),
        MountainousRegion       UMETA(DisplayName = "Mountainous Region"),
        RiverDelta              UMETA(DisplayName = "River Delta"),
        VolcanicRegion          UMETA(DisplayName = "Volcanic Region"),
        AridUplands             UMETA(DisplayName = "Arid Uplands"),
        SwampyLowlands          UMETA(DisplayName = "Swampy Lowlands")
    };

    /** Terrain generation configuration */
    USTRUCT(BlueprintType)
    struct FTerrainConfig
    {
        GENERATED_BODY()

        /** Heightmap resolution (must be power of 2 + 1) */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resolution")
        int32 HeightmapResolution = 4033; // Optimized for 32x32 components

        /** Maximum elevation in cm */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Elevation")
        float MaxElevation = 300000.0f; // 3km max height

        /** Minimum elevation in cm */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Elevation")
        float MinElevation = -10000.0f; // 100m below sea level

        /** Sea level in cm */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Elevation")
        float SeaLevel = 0.0f;

        /** Continental slope steepness */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Topology", meta = (ClampMin = "0.0", ClampMax = "2.0"))
        float ContinentalSlope = 0.8f;

        /** Mountain formation strength */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Topology", meta = (ClampMin = "0.0", ClampMax = "2.0"))
        float MountainStrength = 1.2f;

        /** Valley carving intensity */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Topology", meta = (ClampMin = "0.0", ClampMax = "2.0"))
        float ValleyIntensity = 0.7f;

        /** Tectonic activity level (higher in Mesozoic) */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geology", meta = (ClampMin = "0.0", ClampMax = "2.0"))
        float TectonicActivity = 1.4f;

        /** Erosion simulation strength */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion", meta = (ClampMin = "0.0", ClampMax = "2.0"))
        float ErosionStrength = 0.6f;

        /** Thermal erosion (cliff formation) */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion", meta = (ClampMin = "0.0", ClampMax = "2.0"))
        float ThermalErosion = 0.4f;

        /** Hydraulic erosion (river carving) */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion", meta = (ClampMin = "0.0", ClampMax = "2.0"))
        float HydraulicErosion = 0.8f;

        /** Noise settings for base terrain */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
        FNoiseSettings BaseNoise;

        /** Noise settings for detail features */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
        FNoiseSettings DetailNoise;

        /** Use realistic geological layering */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Realism")
        bool bUseGeologicalLayers = true;

        /** Use plate tectonics simulation */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Realism")
        bool bUsePlateTectonics = true;

        /** Use climate-based erosion */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Realism")
        bool bUseClimateErosion = true;
    };

    /** Noise generation settings */
    USTRUCT(BlueprintType)
    struct FNoiseSettings
    {
        GENERATED_BODY()

        /** Noise type */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Type")
        ENoiseType NoiseType = ENoiseType::PerlinFBM;

        /** Number of octaves */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters", meta = (ClampMin = "1", ClampMax = "12"))
        int32 Octaves = 6;

        /** Base frequency */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters", meta = (ClampMin = "0.0001", ClampMax = "0.1"))
        float Frequency = 0.002f;

        /** Amplitude multiplier */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters", meta = (ClampMin = "0.0", ClampMax = "5.0"))
        float Amplitude = 1.0f;

        /** Lacunarity (frequency multiplier) */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters", meta = (ClampMin = "1.0", ClampMax = "4.0"))
        float Lacunarity = 2.0f;

        /** Persistence (amplitude multiplier) */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters", meta = (ClampMin = "0.0", ClampMax = "1.0"))
        float Persistence = 0.5f;

        /** Domain warping strength */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced", meta = (ClampMin = "0.0", ClampMax = "2.0"))
        float DomainWarp = 0.3f;

        /** Ridged noise multiplier */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced", meta = (ClampMin = "0.0", ClampMax = "2.0"))
        float RidgedMultiplier = 0.0f;
    };

    /** Noise types available */
    UENUM(BlueprintType)
    enum class ENoiseType : uint8
    {
        Perlin          UMETA(DisplayName = "Perlin Noise"),
        PerlinFBM       UMETA(DisplayName = "Perlin FBM"),
        Simplex         UMETA(DisplayName = "Simplex Noise"),
        SimplexFBM      UMETA(DisplayName = "Simplex FBM"),
        Ridged          UMETA(DisplayName = "Ridged Noise"),
        Voronoi         UMETA(DisplayName = "Voronoi Cells"),
        Worley          UMETA(DisplayName = "Worley Noise")
    };

    /** Hydrology system configuration */
    USTRUCT(BlueprintType)
    struct FHydrologyConfig
    {
        GENERATED_BODY()

        /** Enable river generation */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rivers")
        bool bGenerateRivers = true;

        /** Number of major river systems */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rivers", meta = (ClampMin = "0", ClampMax = "10"))
        int32 MajorRiverCount = 3;

        /** River density (tributaries per major river) */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rivers", meta = (ClampMin = "0.0", ClampMax = "2.0"))
        float RiverDensity = 1.0f;

        /** Minimum river length in meters */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rivers")
        float MinRiverLength = 5000.0f;

        /** Maximum river length in meters */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rivers")
        float MaxRiverLength = 50000.0f;

        /** River meandering strength */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rivers", meta = (ClampMin = "0.0", ClampMax = "2.0"))
        float MeanderingStrength = 0.8f;

        /** Enable lake generation */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lakes")
        bool bGenerateLakes = true;

        /** Number of major lakes */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lakes", meta = (ClampMin = "0", ClampMax = "20"))
        int32 LakeCount = 5;

        /** Minimum lake size in square meters */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lakes")
        float MinLakeSize = 100000.0f; // 100m x 100m

        /** Maximum lake size in square meters */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lakes")
        float MaxLakeSize = 10000000.0f; // 1km x 1km

        /** Enable wetland generation */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wetlands")
        bool bGenerateWetlands = true;

        /** Wetland coverage percentage */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wetlands", meta = (ClampMin = "0.0", ClampMax = "0.3"))
        float WetlandCoverage = 0.15f;

        /** Water table depth variation */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Groundwater", meta = (ClampMin = "0.0", ClampMax = "50.0"))
        float WaterTableVariation = 10.0f; // meters

        /** Use realistic hydrology simulation */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Realism")
        bool bUseRealisticHydrology = true;

        /** Simulate seasonal water level changes */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Realism")
        bool bSimulateSeasonalChanges = true;

        /** Enable water erosion effects on terrain */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Realism")
        bool bEnableWaterErosion = true;
    };

    /** Climate simulation configuration */
    USTRUCT(BlueprintType)
    struct FClimateConfig
    {
        GENERATED_BODY()

        /** Global temperature in Celsius (Cretaceous was warmer) */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temperature")
        float GlobalTemperature = 22.0f;

        /** Temperature variation with latitude */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temperature", meta = (ClampMin = "0.0", ClampMax = "30.0"))
        float LatitudeTemperatureGradient = 15.0f;

        /** Temperature variation with altitude (lapse rate) */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temperature", meta = (ClampMin = "0.0", ClampMax = "0.01"))
        float AltitudeTemperatureGradient = 0.0065f; // 6.5°C per 1000m

        /** Global humidity level */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Humidity", meta = (ClampMin = "0.0", ClampMax = "1.0"))
        float GlobalHumidity = 0.75f;

        /** Seasonal variation strength */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seasons", meta = (ClampMin = "0.0", ClampMax = "2.0"))
        float SeasonalVariation = 0.4f;

        /** Prevailing wind direction */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind")
        FVector PrevailingWindDirection = FVector(1.0f, 0.3f, 0.0f);

        /** Wind strength in m/s */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind", meta = (ClampMin = "0.0", ClampMax = "50.0"))
        float WindStrength = 8.0f;

        /** Annual precipitation in mm */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Precipitation")
        float AnnualPrecipitation = 1200.0f;

        /** Precipitation seasonality */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Precipitation", meta = (ClampMin = "0.0", ClampMax = "2.0"))
        float PrecipitationSeasonality = 0.6f;

        /** CO2 levels in ppm (higher in Mesozoic) */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
        float CO2Level = 1500.0f;

        /** Oxygen levels in percentage (lower in Mesozoic) */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere", meta = (ClampMin = "15.0", ClampMax = "25.0"))
        float OxygenLevel = 18.0f;

        /** Use realistic climate modeling */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Realism")
        bool bUseRealisticClimate = true;

        /** Enable orographic effects (mountains affect weather) */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Realism")
        bool bEnableOrographicEffects = true;

        /** Enable maritime climate effects */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Realism")
        bool bEnableMaritimeEffects = true;
    };

    /** Performance monitoring */
    USTRUCT(BlueprintType)
    struct FWorldGenPerformanceStats
    {
        GENERATED_BODY()

        /** Current frame time for world generation */
        UPROPERTY(BlueprintReadOnly, Category = "Performance")
        float CurrentFrameTime = 0.0f;

        /** Memory usage in MB */
        UPROPERTY(BlueprintReadOnly, Category = "Performance")
        float MemoryUsageMB = 0.0f;

        /** Number of active PCG components */
        UPROPERTY(BlueprintReadOnly, Category = "Performance")
        int32 ActivePCGComponents = 0;

        /** Number of streaming cells loaded */
        UPROPERTY(BlueprintReadOnly, Category = "Performance")
        int32 StreamingCellsLoaded = 0;

        /** GPU utilization percentage */
        UPROPERTY(BlueprintReadOnly, Category = "Performance")
        float GPUUtilization = 0.0f;

        /** Generation quality being achieved */
        UPROPERTY(BlueprintReadOnly, Category = "Performance")
        EWorldGenQuality AchievedQuality = EWorldGenQuality::High;

        /** Performance budget compliance */
        UPROPERTY(BlueprintReadOnly, Category = "Performance")
        bool bWithinPerformanceBudget = true;
    };

private:
    /** Core systems */
    UPROPERTY()
    UPCGSubsystem* PCGSubsystem;

    UPROPERTY()
    UWorldPartitionSubsystem* WorldPartitionSubsystem;

    /** Performance monitoring */
    UPROPERTY()
    FPerformanceBudget CurrentPerformanceBudget;

    UPROPERTY()
    FWorldGenPerformanceStats CurrentStats;

    /** Generation state */
    UPROPERTY()
    bool bIsGenerating;

    UPROPERTY()
    bool bRuntimeGenerationEnabled;

    UPROPERTY()
    int32 CurrentGenerationSeed;

    /** Internal generation functions */
    void InitializePCGSystem();
    void SetupWorldPartition(const FWorldGenerationConfig& Config);
    void GenerateTerrainHeightmap(const FTerrainConfig& Config);
    void SimulateErosion(const FTerrainConfig& Config);
    void GenerateRiverSystems(const FHydrologyConfig& Config);
    void SimulateClimate(const FClimateConfig& Config);
    void GenerateBiomeDistribution(const FBiomeConfig& Config, const FClimateConfig& ClimateConfig);
    void PlaceGeologicalFeatures(const FGeologyConfig& Config);
    
    /** Performance monitoring functions */
    void UpdatePerformanceStats();
    void EnforcePerformanceBudget();
    bool IsWithinPerformanceBudget() const;
    
    /** Utility functions */
    float SampleNoise(const FNoiseSettings& Settings, float X, float Y, int32 Seed) const;
    FVector2D WorldToNormalized(const FVector& WorldLocation, const FVector2D& WorldSize) const;
    FVector NormalizedToWorld(const FVector2D& NormalizedLocation, const FVector2D& WorldSize) const;
};