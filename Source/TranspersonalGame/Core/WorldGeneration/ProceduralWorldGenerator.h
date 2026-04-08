// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/DataAsset.h"
#include "Landscape/Classes/Landscape.h"
#include "PCGComponent.h"
#include "PCGGraph.h"
#include "ProceduralWorldGenerator.generated.h"

/**
 * @brief Procedural World Generator for the Jurassic Survival Game
 * 
 * Creates the physical world of the game using UE5's PCG Framework and World Partition.
 * Generates terrains, biomes, rivers, geological structures that support dinosaur ecosystems.
 * 
 * Core Features:
 * - Prehistoric biome generation (forests, plains, swamps, mountains)
 * - Realistic river systems and water flow
 * - Geological formations (caves, cliffs, valleys)
 * - Dinosaur habitat zones with appropriate vegetation
 * - Resource distribution (materials for crafting)
 * - Hidden gem placement for the main quest
 * 
 * Technical Implementation:
 * - Uses PCG Framework for procedural content generation
 * - World Partition for streaming large worlds
 * - Hierarchical Generation for performance optimization
 * - Runtime Generation for dynamic world changes
 * - Landscape system integration for terrain
 * 
 * @author Procedural World Generator — Agent #5
 * @version 1.0 — March 2026
 */

/** Biome types for the prehistoric world */
UENUM(BlueprintType)
enum class EBiomeType : uint8
{
    DenseForest         UMETA(DisplayName = "Dense Forest"),
    OpenWoodland        UMETA(DisplayName = "Open Woodland"),
    GrasslandPlains     UMETA(DisplayName = "Grassland Plains"),
    RiverDelta          UMETA(DisplayName = "River Delta"),
    SwampMarshland      UMETA(DisplayName = "Swamp Marshland"),
    RockyOutcrops       UMETA(DisplayName = "Rocky Outcrops"),
    MountainousRegion   UMETA(DisplayName = "Mountainous Region"),
    CaveSystem          UMETA(DisplayName = "Cave System"),
    CoastalArea         UMETA(DisplayName = "Coastal Area"),
    VolcanicRegion      UMETA(DisplayName = "Volcanic Region")
};

/** Geological feature types */
UENUM(BlueprintType)
enum class EGeologicalFeature : uint8
{
    River               UMETA(DisplayName = "River"),
    Lake                UMETA(DisplayName = "Lake"),
    Waterfall           UMETA(DisplayName = "Waterfall"),
    Cave                UMETA(DisplayName = "Cave"),
    Cliff               UMETA(DisplayName = "Cliff"),
    Valley              UMETA(DisplayName = "Valley"),
    Hill                UMETA(DisplayName = "Hill"),
    Boulder             UMETA(DisplayName = "Boulder"),
    Ravine              UMETA(DisplayName = "Ravine"),
    Plateau             UMETA(DisplayName = "Plateau")
};

/** Resource types scattered throughout the world */
UENUM(BlueprintType)
enum class EResourceType : uint8
{
    Stone               UMETA(DisplayName = "Stone"),
    Wood                UMETA(DisplayName = "Wood"),
    Flint               UMETA(DisplayName = "Flint"),
    Clay                UMETA(DisplayName = "Clay"),
    Fiber               UMETA(DisplayName = "Plant Fiber"),
    Berries             UMETA(DisplayName = "Berries"),
    Herbs               UMETA(DisplayName = "Medicinal Herbs"),
    Water               UMETA(DisplayName = "Fresh Water"),
    Obsidian            UMETA(DisplayName = "Obsidian"),
    Amber               UMETA(DisplayName = "Amber"),
    Crystal             UMETA(DisplayName = "Crystal"),
    TimeGem             UMETA(DisplayName = "Time Gem") // The main quest item
};

/** Biome configuration data asset */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UBiomeData : public UDataAsset
{
    GENERATED_BODY()

public:
    /** Biome type identifier */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::DenseForest;

    /** Biome display name */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome")
    FString BiomeName = "Dense Forest";

    /** Biome description */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome", meta = (MultiLine = true))
    FString BiomeDescription = "A thick prehistoric forest with towering conifers and dense undergrowth.";

    /** Terrain height range for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain")
    FVector2D HeightRange = FVector2D(0.0f, 500.0f);

    /** Terrain slope range (0-90 degrees) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain")
    FVector2D SlopeRange = FVector2D(0.0f, 30.0f);

    /** Temperature range (Celsius) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Climate")
    FVector2D TemperatureRange = FVector2D(15.0f, 25.0f);

    /** Humidity percentage */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Climate")
    FVector2D HumidityRange = FVector2D(60.0f, 90.0f);

    /** Vegetation density (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    float VegetationDensity = 0.8f;

    /** Tree types for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    TArray<TSoftObjectPtr<UStaticMesh>> TreeMeshes;

    /** Undergrowth vegetation */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    TArray<TSoftObjectPtr<UStaticMesh>> UndergrowthMeshes;

    /** Ground cover vegetation */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    TArray<TSoftObjectPtr<UStaticMesh>> GroundCoverMeshes;

    /** Rock and geological features */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Geology")
    TArray<TSoftObjectPtr<UStaticMesh>> RockMeshes;

    /** Resources available in this biome */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resources")
    TMap<EResourceType, float> ResourceAvailability;

    /** Dinosaur habitat suitability (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wildlife")
    TMap<FString, float> DinosaurHabitatSuitability;

    /** PCG Graph for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation")
    TSoftObjectPtr<UPCGGraph> BiomePCGGraph;

    /** Biome transition blend distance */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation")
    float TransitionBlendDistance = 1000.0f;
};

/** World generation parameters */
USTRUCT(BlueprintType)
struct FWorldGenerationParams
{
    GENERATED_BODY()

    /** World size in Unreal units */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Size")
    FVector2D WorldSize = FVector2D(20480.0f, 20480.0f); // 20km x 20km

    /** Landscape resolution */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    int32 LandscapeResolution = 4033; // 4033x4033 vertices

    /** Height scale multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float HeightScale = 100.0f;

    /** Number of major biomes */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    int32 MajorBiomeCount = 5;

    /** Number of minor biome variations */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    int32 MinorBiomeCount = 12;

    /** River system complexity (1-10) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Systems")
    int32 RiverComplexity = 6;

    /** Number of major rivers */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Systems")
    int32 MajorRiverCount = 3;

    /** Lake generation probability */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Systems")
    float LakeGenerationProbability = 0.3f;

    /** Cave system density */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geological Features")
    float CaveSystemDensity = 0.15f;

    /** Resource distribution density */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
    float ResourceDensity = 1.0f;

    /** Time gem spawn locations count */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Items")
    int32 TimeGemLocations = 1;

    /** Random seed for generation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 WorldSeed = 12345;
};

/** River system data */
USTRUCT(BlueprintType)
struct FRiverSystemData
{
    GENERATED_BODY()

    /** River spline points */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    TArray<FVector> SplinePoints;

    /** River width at each point */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    TArray<float> WidthAtPoints;

    /** River depth at each point */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    TArray<float> DepthAtPoints;

    /** Flow velocity */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    float FlowVelocity = 100.0f;

    /** River type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    FString RiverType = "Meandering";

    /** Connected lakes */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    TArray<FVector> ConnectedLakes;
};

/** Main Procedural World Generator class */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UProceduralWorldGenerator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UProceduralWorldGenerator();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /** Generate the complete world */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateWorld(const FWorldGenerationParams& Params);

    /** Generate specific biome at location */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateBiomeAtLocation(FVector Location, EBiomeType BiomeType, float Radius = 5000.0f);

    /** Generate river system */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateRiverSystem(const FRiverSystemData& RiverData);

    /** Place geological features */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void PlaceGeologicalFeatures(EGeologicalFeature FeatureType, TArray<FVector> Locations);

    /** Distribute resources across the world */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void DistributeResources();

    /** Place the time gem (main quest item) */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void PlaceTimeGem(FVector Location);

    /** Get biome type at world location */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    EBiomeType GetBiomeAtLocation(FVector WorldLocation) const;

    /** Get available resources at location */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    TArray<EResourceType> GetResourcesAtLocation(FVector WorldLocation, float SearchRadius = 1000.0f) const;

    /** Check if location is suitable for dinosaur habitat */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    float GetDinosaurHabitatSuitability(FVector WorldLocation, const FString& DinosaurSpecies) const;

protected:
    /** Initialize PCG system for world generation */
    void InitializePCGSystem();

    /** Create base landscape */
    void CreateBaseLandscape(const FWorldGenerationParams& Params);

    /** Generate height maps using noise functions */
    void GenerateHeightMaps(const FWorldGenerationParams& Params);

    /** Create biome distribution map */
    void CreateBiomeDistribution(const FWorldGenerationParams& Params);

    /** Generate water systems (rivers, lakes) */
    void GenerateWaterSystems(const FWorldGenerationParams& Params);

    /** Place vegetation using PCG */
    void PlaceVegetation();

    /** Generate geological features */
    void GenerateGeologicalFeatures(const FWorldGenerationParams& Params);

    /** Create cave systems */
    void GenerateCaveSystems(const FWorldGenerationParams& Params);

    /** Setup World Partition streaming */
    void SetupWorldPartition();

    /** Apply performance optimizations */
    void ApplyPerformanceOptimizations();

private:
    /** Available biome data assets */
    UPROPERTY(EditAnywhere, Category = "Biomes")
    TArray<TSoftObjectPtr<UBiomeData>> AvailableBiomes;

    /** Current world generation parameters */
    UPROPERTY()
    FWorldGenerationParams CurrentWorldParams;

    /** Generated landscape reference */
    UPROPERTY()
    TObjectPtr<ALandscape> GeneratedLandscape;

    /** PCG World Actor for procedural generation */
    UPROPERTY()
    TObjectPtr<class APCGWorldActor> PCGWorldActor;

    /** Biome distribution texture */
    UPROPERTY()
    TObjectPtr<UTexture2D> BiomeDistributionTexture;

    /** Height map texture */
    UPROPERTY()
    TObjectPtr<UTexture2D> HeightMapTexture;

    /** Water flow texture */
    UPROPERTY()
    TObjectPtr<UTexture2D> WaterFlowTexture;

    /** Resource distribution map */
    UPROPERTY()
    TMap<EResourceType, TArray<FVector>> ResourceDistributionMap;

    /** Generated river systems */
    UPROPERTY()
    TArray<FRiverSystemData> GeneratedRivers;

    /** Time gem location (main quest) */
    UPROPERTY()
    FVector TimeGemLocation;

    /** World generation seed */
    int32 GenerationSeed = 12345;

    /** Performance metrics during generation */
    float GenerationStartTime = 0.0f;
    int32 GeneratedActorCount = 0;
    float TotalGenerationTime = 0.0f;

    /** Utility functions */
    float GeneratePerlinNoise(float X, float Y, float Scale, int32 Octaves = 4) const;
    float GenerateRidgedNoise(float X, float Y, float Scale) const;
    FVector2D GenerateGradientNoise(float X, float Y) const;
    EBiomeType SelectBiomeBasedOnConditions(float Height, float Temperature, float Humidity, float Slope) const;
    bool IsLocationSuitableForWater(FVector Location) const;
    TArray<FVector> GenerateRiverPath(FVector StartLocation, FVector EndLocation) const;
    void BlendBiomeTransitions(FVector Location, float BlendRadius) const;
};

/** PCG-based biome generator component */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UBiomeGeneratorComponent : public UPCGComponent
{
    GENERATED_BODY()

public:
    UBiomeGeneratorComponent();

    /** Set biome data for generation */
    UFUNCTION(BlueprintCallable, Category = "Biome Generation")
    void SetBiomeData(UBiomeData* BiomeData);

    /** Generate biome content at location */
    UFUNCTION(BlueprintCallable, Category = "Biome Generation")
    void GenerateBiomeContent(FVector CenterLocation, float Radius);

protected:
    /** Biome data reference */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome")
    TObjectPtr<UBiomeData> BiomeData;

    /** Generation parameters */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation")
    float VegetationDensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation")
    float ResourceDensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation")
    bool bEnableRuntimeGeneration = true;
};