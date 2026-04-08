// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "PCGComponent.h"
#include "PCGGraph.h"
#include "PCGData.h"
#include "PCGSettings.h"
#include "PCGElement.h"
#include "Engine/World.h"
#include "Landscape.h"
#include "LandscapeProxy.h"
#include "WorldPartition/WorldPartition.h"
#include "WorldPartition/DataLayer/DataLayer.h"
#include "Engine/DataTable.h"
#include "Math/UnrealMathUtility.h"
#include "PCGWorldGeneratorV2.generated.h"

class UPCGGraph;
class UPCGComponent;
class ALandscape;
class ULandscapeComponent;
class UDataLayer;

/**
 * @brief Advanced Procedural World Generator for Jurassic Survival Game
 * 
 * Creates a vast prehistoric world following the Geographic Guide specifications:
 * - Regional scale: 200km² of explorable terrain (14.1km x 14.1km)
 * - 5 distinct biomes: Forest (center), Swamp, Savana, Desert, Snowy Rockside
 * - Impassable mountain borders creating natural boundaries
 * - Dynamic river system flowing from mountains to center
 * - Transition zones between biomes for natural blending
 * - World Partition integration for streaming large worlds
 * - Hierarchical PCG generation for performance optimization
 * 
 * Design Philosophy (Ken Perlin + Will Wright):
 * "A natureza não é aleatória — tem padrões, tem lógica, tem história escrita na terra.
 * Um mundo que funciona como sistema gera histórias que nenhum designer conseguiu planear."
 * 
 * @author Procedural World Generator — Agent #5
 * @version 2.0 — March 2026
 */

/** Jurassic biome types following the Geographic Guide */
UENUM(BlueprintType)
enum class EJurassicBiomeType : uint8
{
    Forest          UMETA(DisplayName = "Forest (Tropical Forest - Center)"),
    Swamp           UMETA(DisplayName = "Swamp (Southeast)"),
    Savana          UMETA(DisplayName = "Savana (Southwest)"),
    Desert          UMETA(DisplayName = "Desert (Northwest)"),
    SnowyRockside   UMETA(DisplayName = "Snowy Rockside (Northeast)"),
    MountainBorder  UMETA(DisplayName = "Mountain Border (Impassable)"),
    Transition      UMETA(DisplayName = "Transition Zone"),
    RiverValley     UMETA(DisplayName = "River Valley")
};

/** Geological formation types for realistic terrain generation */
UENUM(BlueprintType)
enum class EGeologicalFormation : uint8
{
    Sedimentary     UMETA(DisplayName = "Sedimentary (Plains, Swamps)"),
    Igneous         UMETA(DisplayName = "Igneous (Mountains, Volcanic)"),
    Metamorphic     UMETA(DisplayName = "Metamorphic (Hills, Ridges)"),
    Alluvial        UMETA(DisplayName = "Alluvial (River Deposits)"),
    Glacial         UMETA(DisplayName = "Glacial (Snowy Regions)"),
    Erosional       UMETA(DisplayName = "Erosional (Canyons, Valleys)")
};

/** Water flow characteristics for different biomes */
UENUM(BlueprintType)
enum class EWaterFlowType : uint8
{
    FastRiver       UMETA(DisplayName = "Fast River (Forest, Mountains)"),
    SlowRiver       UMETA(DisplayName = "Slow River (Swamp)"),
    SeasonalStream  UMETA(DisplayName = "Seasonal Stream (Savana)"),
    DryBed          UMETA(DisplayName = "Dry Riverbed (Desert)"),
    FrozenStream    UMETA(DisplayName = "Frozen Stream (Snowy)"),
    Lake            UMETA(DisplayName = "Lake (Transition Zones)"),
    Wetland         UMETA(DisplayName = "Wetland (Swamp)")
};

/** Terrain generation parameters for each biome */
USTRUCT(BlueprintType)
struct FBiomeTerrainSettings
{
    GENERATED_BODY()

    /** Biome identifier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EJurassicBiomeType BiomeType = EJurassicBiomeType::Forest;

    /** Elevation range for this biome (in cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain", meta = (ClampMin = "0", ClampMax = "500000"))
    FVector2D ElevationRange = FVector2D(0.0f, 10000.0f);

    /** Slope angle range (in degrees) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain", meta = (ClampMin = "0", ClampMax = "90"))
    FVector2D SlopeRange = FVector2D(0.0f, 15.0f);

    /** Geological formation type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geology")
    EGeologicalFormation GeologyType = EGeologicalFormation::Sedimentary;

    /** Primary noise parameters for large-scale features */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise Primary")
    float PrimaryNoiseFreq = 0.0001f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise Primary")
    float PrimaryNoiseAmplitude = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise Primary")
    int32 PrimaryNoiseOctaves = 4;

    /** Secondary noise for medium-scale features */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise Secondary")
    float SecondaryNoiseFreq = 0.001f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise Secondary")
    float SecondaryNoiseAmplitude = 0.5f;

    /** Detail noise for small-scale features */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise Detail")
    float DetailNoiseFreq = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise Detail")
    float DetailNoiseAmplitude = 0.2f;

    /** Erosion simulation parameters */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion")
    float ThermalErosionRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion")
    float HydraulicErosionRate = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion")
    int32 ErosionIterations = 5;

    /** Water characteristics */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    EWaterFlowType WaterType = EWaterFlowType::FastRiver;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    float WaterDensity = 0.3f; // 0-1 range

    /** Climate parameters affecting generation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float Temperature = 25.0f; // Celsius

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float Humidity = 0.6f; // 0-1 range

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float Rainfall = 150.0f; // cm/year

    /** Transition zone settings */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transitions")
    float TransitionWidth = 200000.0f; // 2km transition zones

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transitions")
    float TransitionSmoothness = 0.7f; // How smooth the blend is

    /** PCG performance settings */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 PCGGridSize = 12800; // cm, for hierarchical generation

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float StreamingDistance = 640000.0f; // 6.4km streaming distance

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseGPUGeneration = true; // Use GPU for point processing

    /** PCG Graph reference for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG")
    TSoftObjectPtr<UPCGGraph> BiomePCGGraph;
};

/** River system configuration */
USTRUCT(BlueprintType)
struct FRiverSystemConfig
{
    GENERATED_BODY()

    /** River source points (mountain peaks) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sources")
    TArray<FVector> RiverSources;

    /** Main river count flowing to center */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rivers")
    int32 MainRiverCount = 4;

    /** Tributary rivers per main river */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rivers")
    int32 TributaryCount = 8;

    /** River width range (in cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dimensions")
    FVector2D RiverWidthRange = FVector2D(500.0f, 2000.0f); // 5-20m

    /** River depth range (in cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dimensions")
    FVector2D RiverDepthRange = FVector2D(200.0f, 800.0f); // 2-8m

    /** Flow speed range (cm/s) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flow")
    FVector2D FlowSpeedRange = FVector2D(50.0f, 300.0f); // 0.5-3 m/s

    /** Meandering factor (how curvy rivers are) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shape")
    float MeanderingFactor = 0.3f;

    /** Erosion influence on terrain */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion")
    float RiverErosionStrength = 0.5f;

    /** Lake generation in transition zones */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lakes")
    int32 LakeCount = 6;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lakes")
    FVector2D LakeSizeRange = FVector2D(100000.0f, 500000.0f); // 1-5km diameter
};

/** World generation master configuration */
USTRUCT(BlueprintType)
struct FWorldGenerationMasterConfig
{
    GENERATED_BODY()

    /** World dimensions following UE5 best practices */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Size")
    int32 LandscapeSize = 8129; // 8129x8129 vertices for optimal performance

    /** Scale: 25m per landscape unit = ~200km² total area */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Size")
    float WorldScale = 2500.0f; // cm per landscape unit

    /** Maximum elevation (mountain peaks) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Size")
    float MaxElevation = 400000.0f; // 4km mountain peaks

    /** Impassable mountain border configuration */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mountain Border")
    float BorderWidth = 1500000.0f; // 15km impassable border

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mountain Border")
    float BorderHeight = 350000.0f; // 3.5km high mountains

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mountain Border")
    float BorderSteepness = 0.85f; // Very steep, impassable

    /** Biome distribution settings */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Distribution")
    float ForestCenterRadius = 3000000.0f; // 30km forest in center

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Distribution")
    float BiomeTransitionSmoothness = 0.8f;

    /** Player spawn location (always in Forest biome center) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Spawn")
    FVector PlayerSpawnLocation = FVector(0.0f, 0.0f, 50000.0f); // Forest center, 500m elevation

    /** World Partition configuration */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Partition")
    bool bUseWorldPartition = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Partition")
    int32 WorldPartitionCellSize = 128000; // 1.28km cells

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Partition")
    float StreamingRange = 768000.0f; // 7.68km streaming range

    /** Hierarchical generation settings */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hierarchical Generation")
    bool bUseHierarchicalGeneration = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hierarchical Generation")
    TArray<int32> HierarchicalGridSizes = {25600, 12800, 6400, 3200}; // Multi-scale generation

    /** Performance optimization */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseGPUGeneration = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxConcurrentGenerations = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameTime = 16.67f; // 60fps target

    /** Generation seed for reproducible worlds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 WorldSeed = 42; // Answer to everything

    /** Debug visualization settings */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowBiomeDebugColors = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowRiverDebugLines = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowElevationContours = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowPerformanceStats = false;
};

/**
 * @brief Advanced PCG World Generator V2
 * 
 * Manages the complete procedural generation pipeline using UE5 PCG Framework:
 * 1. Landscape heightfield generation with geological simulation
 * 2. Biome distribution based on climate and elevation
 * 3. River system generation with realistic flow patterns
 * 4. World Partition setup for streaming
 * 5. Hierarchical PCG generation for performance
 * 6. GPU-accelerated point processing
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APCGWorldGeneratorV2 : public AActor
{
    GENERATED_BODY()

public:
    APCGWorldGeneratorV2();

protected:
    virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform& Transform) override;

public:
    /** Generate the complete world */
    UFUNCTION(BlueprintCallable, Category = "World Generation", CallInEditor = true)
    void GenerateWorld();

    /** Generate only terrain heightfield */
    UFUNCTION(BlueprintCallable, Category = "World Generation", CallInEditor = true)
    void GenerateTerrain();

    /** Generate biome distribution */
    UFUNCTION(BlueprintCallable, Category = "World Generation", CallInEditor = true)
    void GenerateBiomes();

    /** Generate river system */
    UFUNCTION(BlueprintCallable, Category = "World Generation", CallInEditor = true)
    void GenerateRivers();

    /** Setup World Partition */
    UFUNCTION(BlueprintCallable, Category = "World Generation", CallInEditor = true)
    void SetupWorldPartition();

    /** Clear all generated content */
    UFUNCTION(BlueprintCallable, Category = "World Generation", CallInEditor = true)
    void ClearWorld();

    /** Get biome type at world location */
    UFUNCTION(BlueprintCallable, Category = "World Query")
    EJurassicBiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    /** Get elevation at world location */
    UFUNCTION(BlueprintCallable, Category = "World Query")
    float GetElevationAtLocation(const FVector& WorldLocation) const;

    /** Get water flow data at location */
    UFUNCTION(BlueprintCallable, Category = "World Query")
    EWaterFlowType GetWaterFlowAtLocation(const FVector& WorldLocation) const;

protected:
    /** Main PCG component for world generation */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PCG")
    TObjectPtr<UPCGComponent> MainPCGComponent;

    /** PCG component for terrain generation */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PCG")
    TObjectPtr<UPCGComponent> TerrainPCGComponent;

    /** PCG component for biome distribution */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PCG")
    TObjectPtr<UPCGComponent> BiomePCGComponent;

    /** PCG component for river generation */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PCG")
    TObjectPtr<UPCGComponent> RiverPCGComponent;

    /** Master world generation configuration */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Configuration")
    FWorldGenerationMasterConfig WorldConfig;

    /** Biome terrain settings for each biome type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Configuration")
    TMap<EJurassicBiomeType, FBiomeTerrainSettings> BiomeSettings;

    /** River system configuration */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Configuration")
    FRiverSystemConfig RiverConfig;

    /** PCG Graphs for different generation phases */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PCG Graphs")
    TObjectPtr<UPCGGraph> TerrainGenerationGraph;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PCG Graphs")
    TObjectPtr<UPCGGraph> BiomeDistributionGraph;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PCG Graphs")
    TObjectPtr<UPCGGraph> RiverGenerationGraph;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PCG Graphs")
    TObjectPtr<UPCGGraph> MountainBorderGraph;

    /** Generated landscape reference */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Generated Content")
    TObjectPtr<ALandscape> GeneratedLandscape;

    /** Data layers for World Partition */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World Partition")
    TArray<TObjectPtr<UDataLayer>> BiomeDataLayers;

private:
    /** Internal generation methods */
    void InitializePCGComponents();
    void InitializeBiomeSettings();
    void InitializeRiverConfig();
    void CreateLandscape();
    void GenerateHeightfield();
    void DistributeBiomes();
    void CreateRiverSystem();
    void CreateMountainBorders();
    void SetupDataLayers();
    void OptimizePerformance();

    /** Utility methods */
    FVector2D WorldLocationToBiomeCoordinate(const FVector& WorldLocation) const;
    float CalculateDistanceFromCenter(const FVector& WorldLocation) const;
    float CalculateDistanceFromBorder(const FVector& WorldLocation) const;
    EJurassicBiomeType DetermineBiomeFromLocation(const FVector& WorldLocation) const;

    /** Performance monitoring */
    void StartPerformanceTimer(const FString& OperationName);
    void EndPerformanceTimer(const FString& OperationName);
    TMap<FString, double> PerformanceTimers;
};