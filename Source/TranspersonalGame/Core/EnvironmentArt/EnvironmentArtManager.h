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
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Engine/Texture2D.h"
#include "../WorldGeneration/ProceduralWorldGenerator.h"
#include "EnvironmentArtManager.generated.h"

/**
 * @brief Environment Art Manager for the Jurassic Survival Game
 * 
 * Transforms generated terrain into living, breathing environments.
 * Populates the world with vegetation, rocks, props and atmospheric details.
 * Creates the visual narrative that makes players feel they've stepped into a real prehistoric world.
 * 
 * Core Philosophy:
 * - Every detail tells a story about the world before the player arrived
 * - Environmental storytelling through prop placement and wear patterns
 * - Realistic ecosystem distribution based on biome characteristics
 * - Performance-optimized instancing and LOD systems
 * 
 * Key Features:
 * - Biome-specific vegetation distribution using Foliage Mode and PCG
 * - Procedural rock and geological detail placement
 * - Environmental storytelling props (bones, abandoned nests, etc.)
 * - Atmospheric particle systems (pollen, dust, insects)
 * - Dynamic material blending for terrain surfaces
 * - Performance-optimized instancing for high-density vegetation
 * - Nanite-ready asset preparation for ultra-detailed environments
 * 
 * Technical Implementation:
 * - Hierarchical Instanced Static Mesh Components for vegetation
 * - PCG integration for rule-based prop placement
 * - Material layering system for terrain surface variation
 * - LOD chain optimization for performance scaling
 * - Culling and streaming integration with World Partition
 * 
 * @author Environment Artist — Agent #6
 * @version 1.0 — March 2026
 */

/** Vegetation types for prehistoric environments */
UENUM(BlueprintType)
enum class EVegetationType : uint8
{
    // Trees
    ConiferTrees        UMETA(DisplayName = "Conifer Trees"),
    DeciduousTrees      UMETA(DisplayName = "Deciduous Trees"),
    PalmTrees           UMETA(DisplayName = "Palm Trees"),
    CycadTrees          UMETA(DisplayName = "Cycad Trees"),
    FernTrees           UMETA(DisplayName = "Tree Ferns"),
    
    // Large Plants
    GiantFerns          UMETA(DisplayName = "Giant Ferns"),
    Horsetails          UMETA(DisplayName = "Horsetails"),
    Cycads              UMETA(DisplayName = "Cycads"),
    LargeMosses         UMETA(DisplayName = "Large Mosses"),
    
    // Undergrowth
    SmallFerns          UMETA(DisplayName = "Small Ferns"),
    Grasses             UMETA(DisplayName = "Grasses"),
    Shrubs              UMETA(DisplayName = "Shrubs"),
    Vines               UMETA(DisplayName = "Vines"),
    
    // Ground Cover
    Mosses              UMETA(DisplayName = "Mosses"),
    Lichens             UMETA(DisplayName = "Lichens"),
    SmallPlants         UMETA(DisplayName = "Small Plants"),
    Flowers             UMETA(DisplayName = "Flowers"),
    
    // Aquatic
    WaterLilies         UMETA(DisplayName = "Water Lilies"),
    Reeds               UMETA(DisplayName = "Reeds"),
    Algae               UMETA(DisplayName = "Algae"),
    AquaticFerns        UMETA(DisplayName = "Aquatic Ferns")
};

/** Rock and geological feature types */
UENUM(BlueprintType)
enum class ERockType : uint8
{
    // Large Features
    LargeBoulders       UMETA(DisplayName = "Large Boulders"),
    RockOutcrops        UMETA(DisplayName = "Rock Outcrops"),
    CliffFaces          UMETA(DisplayName = "Cliff Faces"),
    RockFormations      UMETA(DisplayName = "Rock Formations"),
    
    // Medium Features
    MediumRocks         UMETA(DisplayName = "Medium Rocks"),
    RockClusters        UMETA(DisplayName = "Rock Clusters"),
    WeatheredStones     UMETA(DisplayName = "Weathered Stones"),
    
    // Small Details
    SmallStones         UMETA(DisplayName = "Small Stones"),
    Pebbles             UMETA(DisplayName = "Pebbles"),
    RockDebris          UMETA(DisplayName = "Rock Debris"),
    
    // Special Types
    VolcanicRock        UMETA(DisplayName = "Volcanic Rock"),
    Limestone           UMETA(DisplayName = "Limestone"),
    Sandstone           UMETA(DisplayName = "Sandstone"),
    Granite             UMETA(DisplayName = "Granite")
};

/** Environmental storytelling prop types */
UENUM(BlueprintType)
enum class EEnvironmentalPropType : uint8
{
    // Dinosaur Evidence
    DinosaurBones       UMETA(DisplayName = "Dinosaur Bones"),
    DinosaurSkull       UMETA(DisplayName = "Dinosaur Skull"),
    DinosaurFootprints  UMETA(DisplayName = "Dinosaur Footprints"),
    DinosaurNests       UMETA(DisplayName = "Dinosaur Nests"),
    DinosaurEggs        UMETA(DisplayName = "Dinosaur Eggs"),
    
    // Natural Debris
    FallenLogs          UMETA(DisplayName = "Fallen Logs"),
    DeadTrees           UMETA(DisplayName = "Dead Trees"),
    BrokenBranches      UMETA(DisplayName = "Broken Branches"),
    RottenStumps        UMETA(DisplayName = "Rotten Stumps"),
    
    // Geological
    CrystalFormations   UMETA(DisplayName = "Crystal Formations"),
    FossilizedWood      UMETA(DisplayName = "Fossilized Wood"),
    AmberDeposits       UMETA(DisplayName = "Amber Deposits"),
    
    // Water Features
    Driftwood           UMETA(DisplayName = "Driftwood"),
    RiverStones         UMETA(DisplayName = "River Stones"),
    WaterloggedBranches UMETA(DisplayName = "Waterlogged Branches"),
    
    // Atmospheric
    SpiderWebs          UMETA(DisplayName = "Spider Webs"),
    InsectSwarms        UMETA(DisplayName = "Insect Swarms"),
    PollenClouds        UMETA(DisplayName = "Pollen Clouds"),
    MistEffects         UMETA(DisplayName = "Mist Effects")
};

/** Vegetation asset data */
USTRUCT(BlueprintType)
struct FVegetationAssetData
{
    GENERATED_BODY()

    /** Static mesh for this vegetation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset")
    TSoftObjectPtr<UStaticMesh> StaticMesh;

    /** Material override (optional) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset")
    TSoftObjectPtr<UMaterialInterface> MaterialOverride;

    /** Vegetation type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classification")
    EVegetationType VegetationType = EVegetationType::SmallFerns;

    /** Size variation range */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variation")
    FVector2D ScaleRange = FVector2D(0.8f, 1.2f);

    /** Rotation variation (degrees) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variation")
    float RotationVariation = 360.0f;

    /** Density per square meter */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution")
    float DensityPerSquareMeter = 1.0f;

    /** Slope tolerance (0-90 degrees) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution")
    FVector2D SlopeRange = FVector2D(0.0f, 45.0f);

    /** Height range for placement */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution")
    FVector2D HeightRange = FVector2D(-1000.0f, 1000.0f);

    /** Distance from water preference */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution")
    FVector2D WaterDistancePreference = FVector2D(0.0f, 1000.0f);

    /** Clustering factor (0 = random, 1 = highly clustered) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution")
    float ClusteringFactor = 0.3f;

    /** LOD distances */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    TArray<float> LODDistances = {500.0f, 1000.0f, 2000.0f};

    /** Culling distance */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CullingDistance = 5000.0f;

    /** Enable Nanite (if supported) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseNanite = false;

    /** Wind response strength */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float WindResponseStrength = 1.0f;
};

/** Rock asset data */
USTRUCT(BlueprintType)
struct FRockAssetData
{
    GENERATED_BODY()

    /** Static mesh for this rock */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset")
    TSoftObjectPtr<UStaticMesh> StaticMesh;

    /** Material variations */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset")
    TArray<TSoftObjectPtr<UMaterialInterface>> MaterialVariations;

    /** Rock type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classification")
    ERockType RockType = ERockType::MediumRocks;

    /** Size variation range */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variation")
    FVector2D ScaleRange = FVector2D(0.7f, 1.5f);

    /** Rotation variation (degrees) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variation")
    float RotationVariation = 360.0f;

    /** Sink into ground percentage */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
    FVector2D SinkRange = FVector2D(0.0f, 0.3f);

    /** Density per square meter */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution")
    float DensityPerSquareMeter = 0.1f;

    /** Slope preference */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution")
    FVector2D SlopePreference = FVector2D(15.0f, 60.0f);

    /** Clustering factor */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution")
    float ClusteringFactor = 0.7f;

    /** Enable collision */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bEnableCollision = true;

    /** Enable Nanite */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseNanite = true;
};

/** Environmental prop asset data */
USTRUCT(BlueprintType)
struct FEnvironmentalPropData
{
    GENERATED_BODY()

    /** Static mesh for this prop */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset")
    TSoftObjectPtr<UStaticMesh> StaticMesh;

    /** Prop type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classification")
    EEnvironmentalPropType PropType = EEnvironmentalPropType::FallenLogs;

    /** Story weight (higher = more narrative importance) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float StoryWeight = 1.0f;

    /** Rarity (0 = common, 1 = very rare) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution")
    float Rarity = 0.5f;

    /** Biome preferences */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution")
    TMap<EBiomeType, float> BiomePreferences;

    /** Size variation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variation")
    FVector2D ScaleRange = FVector2D(0.8f, 1.2f);

    /** Weathering level (0 = new, 1 = heavily weathered) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FVector2D WeatheringRange = FVector2D(0.2f, 0.8f);

    /** Associated particle effects */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    TArray<TSoftObjectPtr<UNiagaraSystem>> ParticleEffects;
};

/** Biome art configuration */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UBiomeArtData : public UDataAsset
{
    GENERATED_BODY()

public:
    /** Biome type this art data applies to */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::DenseForest;

    /** Vegetation assets for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    TArray<FVegetationAssetData> VegetationAssets;

    /** Rock assets for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rocks")
    TArray<FRockAssetData> RockAssets;

    /** Environmental props for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Props")
    TArray<FEnvironmentalPropData> EnvironmentalProps;

    /** Terrain materials for layered blending */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Materials")
    TArray<TSoftObjectPtr<UMaterialInterface>> TerrainMaterials;

    /** Landscape material for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Materials")
    TSoftObjectPtr<UMaterialInterface> LandscapeMaterial;

    /** Atmospheric particle systems */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Atmosphere")
    TArray<TSoftObjectPtr<UNiagaraSystem>> AtmosphericEffects;

    /** Ambient sound cues */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    TArray<TSoftObjectPtr<USoundCue>> AmbientSounds;

    /** PCG graph for art placement */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation")
    TSoftObjectPtr<UPCGGraph> ArtPlacementGraph;

    /** Overall vegetation density multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Density")
    float VegetationDensityMultiplier = 1.0f;

    /** Overall rock density multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Density")
    float RockDensityMultiplier = 1.0f;

    /** Environmental storytelling density */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Density")
    float StorytellingDensity = 1.0f;
};

/** Environment art generation parameters */
USTRUCT(BlueprintType)
struct FEnvironmentArtParams
{
    GENERATED_BODY()

    /** Overall detail level (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
    float DetailLevel = 1.0f;

    /** Performance target (0 = max quality, 1 = max performance) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PerformanceTarget = 0.5f;

    /** Maximum instances per chunk */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxInstancesPerChunk = 10000;

    /** Culling distance multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CullingDistanceMultiplier = 1.0f;

    /** Enable Nanite where possible */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableNanite = true;

    /** Enable instancing optimization */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableInstancing = true;

    /** Vegetation variation seed */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 VegetationSeed = 54321;

    /** Rock placement seed */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 RockSeed = 98765;

    /** Storytelling seed */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 StorytellingeSeed = 13579;
};

/** Main Environment Art Manager class */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEnvironmentArtManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEnvironmentArtManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /** Generate environment art for the world */
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void GenerateEnvironmentArt(const FEnvironmentArtParams& Params);

    /** Populate a specific biome with art assets */
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void PopulateBiome(EBiomeType BiomeType, const FVector& Center, float Radius);

    /** Add vegetation to a specific area */
    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void PlaceVegetationInArea(const FVector& Center, float Radius, const TArray<FVegetationAssetData>& VegetationData);

    /** Add rocks and geological features */
    UFUNCTION(BlueprintCallable, Category = "Geology")
    void PlaceRocksInArea(const FVector& Center, float Radius, const TArray<FRockAssetData>& RockData);

    /** Add environmental storytelling props */
    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void PlaceStorytellingProps(const FVector& Center, float Radius, const TArray<FEnvironmentalPropData>& PropData);

    /** Apply terrain materials to landscape */
    UFUNCTION(BlueprintCallable, Category = "Materials")
    void ApplyTerrainMaterials(ALandscape* Landscape, EBiomeType BiomeType);

    /** Create atmospheric effects for biome */
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void CreateAtmosphericEffects(const FVector& Center, float Radius, EBiomeType BiomeType);

    /** Optimize environment art for performance */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeEnvironmentArt(float PerformanceTarget);

    /** Get biome art data */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Environment Art")
    UBiomeArtData* GetBiomeArtData(EBiomeType BiomeType) const;

    /** Register biome art data */
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void RegisterBiomeArtData(EBiomeType BiomeType, UBiomeArtData* ArtData);

    /** Clear all environment art */
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void ClearEnvironmentArt();

    /** Get vegetation density at location */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Environment Art")
    float GetVegetationDensityAtLocation(const FVector& Location) const;

    /** Check if location is suitable for prop placement */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Environment Art")
    bool IsLocationSuitableForProp(const FVector& Location, EEnvironmentalPropType PropType) const;

protected:
    /** Biome art data registry */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    TMap<EBiomeType, TSoftObjectPtr<UBiomeArtData>> BiomeArtDataMap;

    /** Current environment art parameters */
    UPROPERTY(BlueprintReadOnly, Category = "State")
    FEnvironmentArtParams CurrentArtParams;

    /** Instanced mesh components for vegetation */
    UPROPERTY()
    TMap<UStaticMesh*, UHierarchicalInstancedStaticMeshComponent*> VegetationInstances;

    /** Instanced mesh components for rocks */
    UPROPERTY()
    TMap<UStaticMesh*, UHierarchicalInstancedStaticMeshComponent*> RockInstances;

    /** Environmental prop instances */
    UPROPERTY()
    TArray<AActor*> EnvironmentalPropActors;

    /** Reference to world generator */
    UPROPERTY()
    UProceduralWorldGenerator* WorldGenerator;

private:
    /** Initialize default biome art data */
    void InitializeDefaultBiomeArtData();

    /** Create instanced mesh component for asset */
    UHierarchicalInstancedStaticMeshComponent* CreateInstancedMeshComponent(UStaticMesh* Mesh, UMaterialInterface* Material = nullptr);

    /** Calculate placement density based on biome and location */
    float CalculatePlacementDensity(const FVector& Location, EBiomeType BiomeType, EVegetationType VegetationType) const;

    /** Generate placement points using Poisson disc sampling */
    TArray<FVector> GeneratePoissonDiscSampling(const FVector& Center, float Radius, float MinDistance, int32 MaxAttempts = 30) const;

    /** Apply environmental storytelling rules */
    void ApplyStorytellingRules(const FVector& Location, TArray<FEnvironmentalPropData>& Props) const;

    /** Optimize instance rendering */
    void OptimizeInstanceRendering();

    /** Update LOD settings based on performance target */
    void UpdateLODSettings(float PerformanceTarget);
};

/**
 * @brief Environment Art Component for actors that need custom art placement
 */
UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEnvironmentArtComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UEnvironmentArtComponent();

    /** Art generation radius around this component */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Art")
    float ArtRadius = 1000.0f;

    /** Biome type to use for art generation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Art")
    EBiomeType BiomeType = EBiomeType::DenseForest;

    /** Custom art parameters */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Art")
    FEnvironmentArtParams CustomArtParams;

    /** Generate art around this component */
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void GenerateArt();

    /** Clear art around this component */
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void ClearArt();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    /** Generated art instances */
    UPROPERTY()
    TArray<UInstancedStaticMeshComponent*> GeneratedInstances;
};