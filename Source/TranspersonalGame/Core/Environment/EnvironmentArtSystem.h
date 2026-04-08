// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/DataAsset.h"
#include "Materials/MaterialInterface.h"
#include "Components/StaticMeshComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Foliage/Public/FoliageType.h"
#include "Landscape/Classes/LandscapeProxy.h"
#include "../WorldGeneration/BiomeSystem.h"
#include "EnvironmentArtSystem.generated.h"

/**
 * @brief Environment Art System for Transpersonal Game Studio
 * 
 * Transforms the procedurally generated terrain into a living, breathing prehistoric world.
 * This system is the silent author of environmental storytelling — every prop, every cluster
 * of vegetation, every weathered rock tells a story that existed before the player arrived.
 * 
 * Core Philosophy:
 * - Every detail has a reason and tells a story
 * - The world should feel lived-in, not artificially placed
 * - Environmental narrative through subtle details
 * - Composition and lighting guide the player's emotional journey
 * 
 * Features:
 * - Biome-specific vegetation distribution using UE5 Foliage System
 * - Procedural rock and prop placement with narrative purpose
 * - Material blending for realistic terrain surfaces
 * - Environmental storytelling through prop clusters
 * - Performance optimization through LOD and culling systems
 * - Integration with PCG Framework for large-scale generation
 * 
 * Technical Implementation:
 * - Uses Hierarchical Instanced Static Mesh Components for performance
 * - Integrates with UE5 Foliage Mode and Procedural Foliage Tool
 * - Landscape material layering for terrain diversity
 * - Nanite support for high-detail vegetation and rocks
 * - World Partition streaming for massive environments
 * 
 * @author Environment Artist — Agent #6
 * @version 1.0 — March 2026
 */

/** Vegetation placement patterns */
UENUM(BlueprintType)
enum class EVegetationPattern : uint8
{
    Random          UMETA(DisplayName = "Random Distribution"),
    Clustered       UMETA(DisplayName = "Clustered Groups"),
    Linear          UMETA(DisplayName = "Linear Arrangement"),
    Circular        UMETA(DisplayName = "Circular Pattern"),
    EdgeFollowing   UMETA(DisplayName = "Edge Following"),
    Organic         UMETA(DisplayName = "Organic Growth"),
    Sparse          UMETA(DisplayName = "Sparse Scattered"),
    Dense           UMETA(DisplayName = "Dense Coverage")
};

/** Environmental storytelling prop types */
UENUM(BlueprintType)
enum class EStorytellingPropType : uint8
{
    AbandonedCamp       UMETA(DisplayName = "Abandoned Camp"),
    DinosaurRemains     UMETA(DisplayName = "Dinosaur Remains"),
    FallenTree          UMETA(DisplayName = "Fallen Tree"),
    RockFormation       UMETA(DisplayName = "Rock Formation"),
    WaterSource         UMETA(DisplayName = "Water Source"),
    AnimalTrail         UMETA(DisplayName = "Animal Trail"),
    WeatheredStone      UMETA(DisplayName = "Weathered Stone"),
    PlantGrowth         UMETA(DisplayName = "Unique Plant Growth"),
    ErosionPattern      UMETA(DisplayName = "Erosion Pattern"),
    NestingSite         UMETA(DisplayName = "Nesting Site")
};

/** Vegetation asset configuration */
USTRUCT(BlueprintType)
struct FVegetationConfig
{
    GENERATED_BODY()

    /** Vegetation mesh */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    TSoftObjectPtr<UStaticMesh> VegetationMesh;

    /** Foliage type asset for advanced settings */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    TSoftObjectPtr<UFoliageType> FoliageType;

    /** Material variations for this vegetation */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    TArray<TSoftObjectPtr<UMaterialInterface>> MaterialVariations;

    /** Placement pattern */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    EVegetationPattern PlacementPattern = EVegetationPattern::Organic;

    /** Density per square meter */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float DensityPerSquareMeter = 0.5f;

    /** Minimum distance between instances */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement", meta = (ClampMin = "10.0", ClampMax = "1000.0"))
    float MinInstanceDistance = 100.0f;

    /** Scale variation range */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Variation")
    FVector2D ScaleRange = FVector2D(0.8f, 1.2f);

    /** Rotation variation (degrees) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Variation", meta = (ClampMin = "0.0", ClampMax = "360.0"))
    float RotationVariation = 360.0f;

    /** Slope tolerance (degrees) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain")
    FVector2D SlopeRange = FVector2D(0.0f, 45.0f);

    /** Altitude preference range (meters) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain")
    FVector2D AltitudeRange = FVector2D(0.0f, 1000.0f);

    /** Moisture preference (0-1, dry to wet) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    FVector2D MoistureRange = FVector2D(0.3f, 0.8f);

    /** Temperature preference (Celsius) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment")
    FVector2D TemperatureRange = FVector2D(15.0f, 30.0f);

    /** Provides food for herbivores */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
    bool bProvidesFood = false;

    /** Provides shelter/hiding spots */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
    bool bProvidesShelter = false;

    /** Can be destroyed by large dinosaurs */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
    bool bCanBeDestroyed = true;

    /** Regrowth time after destruction (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay", meta = (ClampMin = "0.0"))
    float RegrowthTime = 300.0f;
};

/** Environmental storytelling prop configuration */
USTRUCT(BlueprintType)
struct FStorytellingPropConfig
{
    GENERATED_BODY()

    /** Prop type */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Storytelling")
    EStorytellingPropType PropType = EStorytellingPropType::RockFormation;

    /** Main prop mesh */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Storytelling")
    TSoftObjectPtr<UStaticMesh> MainPropMesh;

    /** Additional detail meshes */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Storytelling")
    TArray<TSoftObjectPtr<UStaticMesh>> DetailMeshes;

    /** Narrative description (for designers) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Storytelling", meta = (MultiLine = true))
    FString NarrativeDescription;

    /** Spawn probability (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SpawnProbability = 0.1f;

    /** Minimum distance from player spawn */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement", meta = (ClampMin = "0.0"))
    float MinDistanceFromSpawn = 500.0f;

    /** Preferred biomes for this prop */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    TArray<EBiomeType> PreferredBiomes;

    /** Cluster size (number of related props) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Clustering", meta = (ClampMin = "1", ClampMax = "20"))
    int32 ClusterSize = 1;

    /** Cluster spread radius */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Clustering", meta = (ClampMin = "100.0", ClampMax = "2000.0"))
    float ClusterRadius = 500.0f;

    /** Interaction type for gameplay */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
    FString InteractionType;

    /** Provides resources when interacted with */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
    TMap<FString, int32> ResourceRewards;
};

/** Terrain material layer configuration */
USTRUCT(BlueprintType)
struct FTerrainMaterialLayer
{
    GENERATED_BODY()

    /** Layer name */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Material")
    FString LayerName;

    /** Material for this layer */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Material")
    TSoftObjectPtr<UMaterialInterface> LayerMaterial;

    /** Blend weight (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Material", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float BlendWeight = 1.0f;

    /** Slope preference for this layer */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    FVector2D SlopePreference = FVector2D(0.0f, 90.0f);

    /** Altitude preference for this layer */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    FVector2D AltitudePreference = FVector2D(0.0f, 1000.0f);

    /** Moisture preference for this layer */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    FVector2D MoisturePreference = FVector2D(0.0f, 1.0f);

    /** Tiling scale */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Material", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float TilingScale = 1.0f;

    /** Normal map intensity */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Material", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float NormalIntensity = 1.0f;

    /** Roughness multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Material", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float RoughnessMultiplier = 1.0f;
};

/** Biome-specific environment art configuration */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEnvironmentArtData : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UEnvironmentArtData();

    /** Associated biome type */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::DenseJungle;

    /** Biome display name */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome")
    FText BiomeName;

    /** Visual description for artists */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome", meta = (MultiLine = true))
    FText ArtisticDescription;

    /** Color palette for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual Style")
    TArray<FLinearColor> ColorPalette;

    /** Lighting mood (warm/cool bias) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual Style", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
    float LightingMoodBias = 0.0f;

    /** Terrain material layers */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain Materials")
    TArray<FTerrainMaterialLayer> TerrainLayers;

    /** Tree configurations */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    TArray<FVegetationConfig> TreeConfigurations;

    /** Undergrowth configurations */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    TArray<FVegetationConfig> UndergrowthConfigurations;

    /** Ground cover configurations */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    TArray<FVegetationConfig> GroundCoverConfigurations;

    /** Rock and geological feature configurations */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Geological Features")
    TArray<FVegetationConfig> RockConfigurations;

    /** Environmental storytelling props */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Storytelling")
    TArray<FStorytellingPropConfig> StorytellingProps;

    /** Overall vegetation density multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Density Control", meta = (ClampMin = "0.0", ClampMax = "5.0"))
    float VegetationDensityMultiplier = 1.0f;

    /** Rock density multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Density Control", meta = (ClampMin = "0.0", ClampMax = "5.0"))
    float RockDensityMultiplier = 1.0f;

    /** Storytelling prop density multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Density Control", meta = (ClampMin = "0.0", ClampMax = "5.0"))
    float PropDensityMultiplier = 1.0f;

    /** Performance LOD settings */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    TMap<FString, float> LODDistances;

    /** Culling distances for different asset types */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    TMap<FString, float> CullingDistances;
};

/**
 * @brief Main Environment Art System
 * 
 * Orchestrates the transformation of procedural terrain into artistic environments.
 * This system thinks like Roger Deakins — every placement serves the narrative,
 * every composition guides the player's emotional journey through the prehistoric world.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEnvironmentArtSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEnvironmentArtSystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /** Generate environment art for a specific biome area */
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void GenerateEnvironmentArt(const FVector& WorldLocation, float Radius, EBiomeType BiomeType);

    /** Populate vegetation in a specific area */
    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void PopulateVegetation(const FVector& Center, float Radius, const UEnvironmentArtData* ArtData);

    /** Place storytelling props in an area */
    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void PlaceStorytellingProps(const FVector& Center, float Radius, const UEnvironmentArtData* ArtData);

    /** Apply terrain materials to landscape */
    UFUNCTION(BlueprintCallable, Category = "Materials")
    void ApplyTerrainMaterials(ALandscapeProxy* Landscape, const UEnvironmentArtData* ArtData);

    /** Create a narrative cluster of props */
    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void CreateNarrativeCluster(const FVector& Location, EStorytellingPropType PropType, const UEnvironmentArtData* ArtData);

    /** Optimize environment art for performance */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeEnvironmentArt(const FVector& PlayerLocation, float OptimizationRadius);

    /** Get environment art data for a biome */
    UFUNCTION(BlueprintCallable, Category = "Data")
    const UEnvironmentArtData* GetEnvironmentArtData(EBiomeType BiomeType) const;

    /** Register environment art data asset */
    UFUNCTION(BlueprintCallable, Category = "Data")
    void RegisterEnvironmentArtData(UEnvironmentArtData* ArtData);

    /** Event called when biome generation is complete */
    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnBiomeArtGenerationComplete(EBiomeType BiomeType, const FVector& Location, float Radius);

protected:
    /** Environment art data for each biome type */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    TMap<EBiomeType, TSoftObjectPtr<UEnvironmentArtData>> BiomeArtData;

    /** Active vegetation instances */
    UPROPERTY(Transient)
    TMap<FString, UHierarchicalInstancedStaticMeshComponent*> VegetationInstances;

    /** Active prop instances */
    UPROPERTY(Transient)
    TMap<FString, UHierarchicalInstancedStaticMeshComponent*> PropInstances;

    /** Performance monitoring */
    UPROPERTY(Transient)
    TMap<FString, float> PerformanceMetrics;

    /** Random number generator for consistent results */
    UPROPERTY(Transient)
    FRandomStream RandomGenerator;

private:
    /** Place individual vegetation instance */
    void PlaceVegetationInstance(const FVegetationConfig& Config, const FVector& Location, const FRotator& Rotation, float Scale);

    /** Calculate vegetation density based on environmental factors */
    float CalculateVegetationDensity(const FVector& Location, const FVegetationConfig& Config, const UEnvironmentArtData* ArtData);

    /** Generate organic placement pattern */
    TArray<FVector> GenerateOrganicPattern(const FVector& Center, float Radius, int32 InstanceCount, float MinDistance);

    /** Generate clustered placement pattern */
    TArray<FVector> GenerateClusteredPattern(const FVector& Center, float Radius, int32 ClusterCount, int32 InstancesPerCluster);

    /** Check if location is suitable for vegetation */
    bool IsLocationSuitableForVegetation(const FVector& Location, const FVegetationConfig& Config);

    /** Create material instance for terrain layer */
    UMaterialInstanceDynamic* CreateTerrainMaterialInstance(const FTerrainMaterialLayer& Layer);

    /** Update LOD levels based on distance */
    void UpdateLODLevels(const FVector& ViewerLocation);

    /** Cull distant instances for performance */
    void CullDistantInstances(const FVector& ViewerLocation);

    /** Log environment art statistics */
    void LogEnvironmentStats(const FString& BiomeName, int32 VegetationCount, int32 PropCount);
};