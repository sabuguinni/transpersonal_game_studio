// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "PCGComponent.h"
#include "PCGGraph.h"
#include "PCGData.h"
#include "Landscape/Classes/Landscape.h"
#include "Engine/DataTable.h"
#include "Math/UnrealMathUtility.h"
#include "WorldPartition/WorldPartition.h"
#include "WorldPartition/WorldPartitionSubsystem.h"
#include "Components/SplineComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "JurassicWorldGeneratorV2.generated.h"

/**
 * @brief Advanced Jurassic World Generator V2
 * 
 * Generates a living, breathing prehistoric world following Ken Perlin's principle:
 * "A natureza não é aleatória — tem padrões, tem lógica, tem história escrita na terra"
 * 
 * Will Wright's philosophy:
 * "Um mundo que funciona como sistema gera histórias que nenhum designer conseguiu planear"
 * 
 * WORLD SPECIFICATIONS:
 * - Scale: Regional (~200km² explorable terrain)
 * - 5 Distinct Biomes: Forest (spawn), Swamp, Savanna, Desert, Snowy Rockside
 * - Impassable mountain borders (natural boundaries)
 * - Dynamic river system (mountains → center)
 * - Realistic geological formation
 * - Performance: 60fps PC / 30fps console
 * 
 * TECHNICAL FEATURES:
 * - World Partition integration for massive worlds
 * - Hierarchical PCG generation (multiple grid levels)
 * - Runtime generation support
 * - GPU-accelerated terrain generation
 * - Biome transition zones with natural blending
 * - Geological history simulation
 * 
 * @author Procedural World Generator — Agent #5
 * @version 2.0 — March 2026
 */

/** Enhanced biome types with geological context */
UENUM(BlueprintType)
enum class EJurassicBiomeType : uint8
{
    /** Dense tropical forest - player spawn area */
    TropicalForest      UMETA(DisplayName = "Tropical Forest"),
    
    /** Wetland ecosystem with slow-moving water */
    Swampland          UMETA(DisplayName = "Swampland"),
    
    /** Open grassland with scattered trees */
    Savanna            UMETA(DisplayName = "Savanna"),
    
    /** Arid desert with rocky outcrops */
    Desert             UMETA(DisplayName = "Desert"),
    
    /** Cold mountainous region with snow */
    SnowyRockside      UMETA(DisplayName = "Snowy Rockside"),
    
    /** Impassable mountain barriers */
    MountainBarrier    UMETA(DisplayName = "Mountain Barrier"),
    
    /** Smooth transitions between biomes */
    TransitionZone     UMETA(DisplayName = "Transition Zone"),
    
    /** River channels and waterways */
    Riverine           UMETA(DisplayName = "Riverine"),
    
    /** Lake shores and wetlands */
    Lacustrine         UMETA(DisplayName = "Lacustrine")
};

/** Geological formation types affecting terrain generation */
UENUM(BlueprintType)
enum class EGeologicalFormation : uint8
{
    /** Ancient mountain ranges */
    Precambrian        UMETA(DisplayName = "Precambrian Shield"),
    
    /** Sedimentary plains */
    Sedimentary        UMETA(DisplayName = "Sedimentary Basin"),
    
    /** Volcanic activity areas */
    Volcanic           UMETA(DisplayName = "Volcanic Formation"),
    
    /** River-carved valleys */
    Fluvial            UMETA(DisplayName = "Fluvial Valley"),
    
    /** Glacial formations */
    Glacial            UMETA(DisplayName = "Glacial Formation"),
    
    /** Coastal erosion patterns */
    Coastal            UMETA(DisplayName = "Coastal Formation")
};

/** Water system types for realistic hydrology */
UENUM(BlueprintType)
enum class EWaterSystemType : uint8
{
    /** Fast-flowing mountain streams */
    MountainStream     UMETA(DisplayName = "Mountain Stream"),
    
    /** Major rivers through valleys */
    MainRiver          UMETA(DisplayName = "Main River"),
    
    /** Smaller tributary streams */
    Tributary          UMETA(DisplayName = "Tributary"),
    
    /** Slow-moving swamp water */
    SwampChannel       UMETA(DisplayName = "Swamp Channel"),
    
    /** Seasonal desert streams */
    SeasonalStream     UMETA(DisplayName = "Seasonal Stream"),
    
    /** Permanent lakes */
    Lake               UMETA(DisplayName = "Lake"),
    
    /** Temporary water bodies */
    SeasonalLake       UMETA(DisplayName = "Seasonal Lake")
};

/** Advanced biome configuration with geological context */
USTRUCT(BlueprintType)
struct FAdvancedBiomeSettings
{
    GENERATED_BODY()

    /** Biome identifier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EJurassicBiomeType BiomeType = EJurassicBiomeType::TropicalForest;

    /** Geological foundation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geology")
    EGeologicalFormation GeologicalBase = EGeologicalFormation::Sedimentary;

    /** Elevation constraints (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector2D ElevationRange = FVector2D(0.0f, 50000.0f);

    /** Slope angle preferences (degrees) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector2D OptimalSlope = FVector2D(0.0f, 15.0f);

    /** Distance from water sources (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hydrology")
    FVector2D WaterProximity = FVector2D(0.0f, 100000.0f);

    /** Climate parameters */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float Temperature = 25.0f; // Celsius

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float Precipitation = 1500.0f; // mm/year

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float Humidity = 0.6f; // 0-1

    /** Terrain generation parameters */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float BaseNoiseFrequency = 0.001f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float NoiseAmplitude = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 NoiseOctaves = 6;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float RidgeNoise = 0.3f;

    /** Transition zone settings */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transitions")
    float TransitionWidth = 200000.0f; // 2km

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transitions")
    float BlendSmoothness = 0.7f;

    /** PCG configuration */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG")
    TSoftObjectPtr<class UPCGGraph> BiomePCGGraph;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG")
    int32 PCGGridSize = 12800; // cm

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG")
    bool bUseGPUGeneration = true;

    /** Performance settings */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float StreamingDistance = 500000.0f; // 5km

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxInstancesPerCell = 10000;
};

/** River system configuration */
USTRUCT(BlueprintType)
struct FRiverSystemConfig
{
    GENERATED_BODY()

    /** River type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    EWaterSystemType WaterType = EWaterSystemType::MainRiver;

    /** Source elevation (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    float SourceElevation = 200000.0f;

    /** Target elevation (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    float TargetElevation = 0.0f;

    /** River width (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    float RiverWidth = 5000.0f;

    /** River depth (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    float RiverDepth = 500.0f;

    /** Flow velocity (cm/s) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    float FlowVelocity = 100.0f;

    /** Meandering factor (0 = straight, 1 = very curvy) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    float MeanderingFactor = 0.3f;

    /** Erosion strength */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    float ErosionStrength = 0.5f;

    /** Tributary generation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tributaries")
    int32 TributaryCount = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tributaries")
    float TributarySpacing = 50000.0f; // 500m
};

/** Master world generation configuration */
USTRUCT(BlueprintType)
struct FJurassicWorldConfig
{
    GENERATED_BODY()

    /** World dimensions */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Size")
    int32 LandscapeResolution = 8129; // UE5 recommended for large worlds

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Size")
    float WorldScale = 2500.0f; // 25m per unit = ~200km²

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Size")
    float MaxElevation = 300000.0f; // 3km peaks

    /** Mountain barrier configuration */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mountain Barrier")
    float BarrierWidth = 1000000.0f; // 10km impassable border

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mountain Barrier")
    float BarrierHeight = 250000.0f; // 2.5km high

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mountain Barrier")
    float BarrierSteepness = 0.9f; // Nearly vertical

    /** River system */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hydrology")
    TArray<FRiverSystemConfig> MajorRivers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hydrology")
    int32 LakeCount = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hydrology")
    FVector2D LakeSizeRange = FVector2D(50000.0f, 200000.0f);

    /** Biome distribution */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TMap<EJurassicBiomeType, FAdvancedBiomeSettings> BiomeSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    float ForestCenterRadius = 2000000.0f; // 20km forest center

    /** Player spawn */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
    FVector PlayerSpawnLocation = FVector::ZeroVector;

    /** Generation parameters */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 WorldSeed = 42; // Reproducible generation

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    bool bUseGeologicalHistory = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float GeologicalTimeScale = 1000000.0f; // Simulated years

    /** Performance optimization */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseWorldPartition = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseHierarchicalGeneration = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseGPUGeneration = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 WorldPartitionCellSize = 128000; // 1.28km cells

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float StreamingRange = 640000.0f; // 6.4km

    /** Debug visualization */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowBiomeDebugOverlay = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowRiverDebugSplines = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowGeologicalLayers = false;
};

/**
 * @brief Advanced Jurassic World Generator Subsystem
 * 
 * Orchestrates the complete world generation process:
 * 1. Geological foundation simulation
 * 2. Terrain heightfield generation
 * 3. Hydrological system creation
 * 4. Biome placement and blending
 * 5. PCG content population
 * 6. World Partition setup
 * 7. Performance optimization
 */
UCLASS()
class TRANSPERSONALGAME_API UJurassicWorldGeneratorV2 : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UJurassicWorldGeneratorV2();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /** Main world generation entry point */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateJurassicWorld(const FJurassicWorldConfig& Config);

    /** Generate specific components */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateTerrainHeightfield();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateRiverSystem();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateBiomeDistribution();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void PopulateBiomesWithPCG();

    /** World Partition setup */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void SetupWorldPartition();

    /** Utility functions */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    EJurassicBiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    float GetElevationAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    float GetDistanceToWater(const FVector& WorldLocation) const;

    /** Debug and visualization */
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void ToggleBiomeDebugVisualization();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void ExportHeightmapToTexture(const FString& FilePath);

protected:
    /** Core generation methods */
    void InitializeGeologicalFoundation();
    void GenerateBaseHeightfield();
    void ApplyGeologicalProcesses();
    void CreateRiverNetworks();
    void CalculateBiomeBoundaries();
    void BlendBiomeTransitions();
    void SetupPCGComponents();
    void OptimizeForPerformance();

    /** Geological simulation */
    void SimulateErosion(int32 Iterations = 100);
    void SimulateTectonicActivity();
    void SimulateWeatheringProcesses();

    /** Hydrological simulation */
    void CalculateWaterFlow();
    void GenerateRiverSplines();
    void CreateLakeSystems();
    void SimulateSeasonalVariation();

    /** Biome logic */
    float CalculateBiomeSuitability(const FVector& Location, EJurassicBiomeType BiomeType) const;
    void ApplyBiomeInfluence(const FVector& Location, EJurassicBiomeType BiomeType, float Influence);

    /** PCG integration */
    void CreatePCGGraphForBiome(EJurassicBiomeType BiomeType);
    void ExecutePCGGeneration();
    void OptimizePCGPerformance();

    /** Performance optimization */
    void SetupHierarchicalGeneration();
    void ConfigureGPUGeneration();
    void SetupRuntimeGeneration();

private:
    /** Current world configuration */
    UPROPERTY()
    FJurassicWorldConfig CurrentConfig;

    /** Generated landscape reference */
    UPROPERTY()
    TObjectPtr<ALandscape> GeneratedLandscape;

    /** Heightfield data */
    UPROPERTY()
    TArray<float> HeightfieldData;

    /** Biome distribution map */
    UPROPERTY()
    TArray<uint8> BiomeMap;

    /** Water distance map */
    UPROPERTY()
    TArray<float> WaterDistanceMap;

    /** River spline components */
    UPROPERTY()
    TArray<TObjectPtr<USplineComponent>> RiverSplines;

    /** PCG components per biome */
    UPROPERTY()
    TMap<EJurassicBiomeType, TObjectPtr<UPCGComponent>> BiomePCGComponents;

    /** Render targets for data visualization */
    UPROPERTY()
    TObjectPtr<UTextureRenderTarget2D> HeightmapRenderTarget;

    UPROPERTY()
    TObjectPtr<UTextureRenderTarget2D> BiomeMapRenderTarget;

    /** Generation state */
    bool bIsGenerating = false;
    bool bGenerationComplete = false;
    float GenerationProgress = 0.0f;

    /** Random number generator with seed */
    FRandomStream RandomGenerator;

    /** Performance monitoring */
    double GenerationStartTime = 0.0;
    double LastPhaseTime = 0.0;

    /** Debug visualization */
    bool bDebugVisualizationEnabled = false;
    TArray<TObjectPtr<class UStaticMeshComponent>> DebugMeshComponents;
};