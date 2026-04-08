// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/DataAsset.h"
#include "Materials/MaterialInterface.h"
#include "Components/StaticMeshComponent.h"
#include "FoliageType.h"
#include "Engine/Texture2D.h"
#include "../WorldGeneration/BiomeSystem.h"
#include "EnvironmentArtSystem.generated.h"

/**
 * @brief Environment Art System for Transpersonal Game Studio
 * 
 * Transforms the generated terrain into a living, breathing prehistoric world.
 * Populates biomes with vegetation, rocks, props, and environmental storytelling elements.
 * 
 * Core Philosophy:
 * - Every detail tells a story about the world before the player arrived
 * - Environmental narrative through strategic prop placement
 * - Realistic ecosystem simulation through art placement
 * - Performance-optimized LOD systems for massive vegetation density
 * 
 * Features:
 * - Biome-specific vegetation distribution using UE5 Foliage System
 * - Procedural rock and geological feature placement
 * - Environmental storytelling props (bones, abandoned nests, etc.)
 * - Dynamic weather-responsive materials
 * - Nanite-enabled high-detail vegetation
 * - Performance-optimized instanced rendering
 * 
 * @author Environment Artist — Agent #6
 * @version 1.0 — March 2026
 */

/** Vegetation layer types for different ecosystem levels */
UENUM(BlueprintType)
enum class EVegetationLayer : uint8
{
    Canopy          UMETA(DisplayName = "Canopy Layer"),
    Understory      UMETA(DisplayName = "Understory"),
    Shrub           UMETA(DisplayName = "Shrub Layer"),
    Herbaceous      UMETA(DisplayName = "Herbaceous Layer"),
    GroundCover     UMETA(DisplayName = "Ground Cover"),
    Emergent        UMETA(DisplayName = "Emergent Layer")
};

/** Environmental prop categories for storytelling */
UENUM(BlueprintType)
enum class EEnvironmentalPropType : uint8
{
    DinosaurBones       UMETA(DisplayName = "Dinosaur Bones"),
    AbandonedNest       UMETA(DisplayName = "Abandoned Nest"),
    FallenLog           UMETA(DisplayName = "Fallen Log"),
    BoulderFormation    UMETA(DisplayName = "Boulder Formation"),
    CrystalFormation    UMETA(DisplayName = "Crystal Formation"),
    FossilDeposit       UMETA(DisplayName = "Fossil Deposit"),
    MudPrint            UMETA(DisplayName = "Mud Print"),
    ScratcMarks         UMETA(DisplayName = "Scratch Marks"),
    WaterSource         UMETA(DisplayName = "Water Source"),
    ShelterCave         UMETA(DisplayName = "Shelter Cave")
};

/** Material quality levels for LOD system */
UENUM(BlueprintType)
enum class EMaterialQuality : uint8
{
    Cinematic       UMETA(DisplayName = "Cinematic Quality"),
    High            UMETA(DisplayName = "High Quality"),
    Medium          UMETA(DisplayName = "Medium Quality"),
    Low             UMETA(DisplayName = "Low Quality"),
    Mobile          UMETA(DisplayName = "Mobile Quality")
};

/** Vegetation asset configuration with LOD and performance settings */
USTRUCT(BlueprintType)
struct FVegetationAssetConfig
{
    GENERATED_BODY()

    /** Static mesh for this vegetation */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    TSoftObjectPtr<UStaticMesh> VegetationMesh;

    /** Foliage type for this vegetation */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    TSoftObjectPtr<UFoliageType> FoliageType;

    /** Vegetation layer classification */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    EVegetationLayer VegetationLayer = EVegetationLayer::Shrub;

    /** Base material for this vegetation */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Materials")
    TSoftObjectPtr<UMaterialInterface> BaseMaterial;

    /** Seasonal material variations */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Materials")
    TMap<FString, TSoftObjectPtr<UMaterialInterface>> SeasonalMaterials;

    /** Wind response intensity (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WindResponseIntensity = 0.5f;

    /** Spawn density per 100m² */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Distribution", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
    float SpawnDensity = 10.0f;

    /** Clustering factor (0 = random, 1 = highly clustered) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Distribution", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ClusteringFactor = 0.3f;

    /** Minimum distance from water sources (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Distribution")
    float MinWaterDistance = 0.0f;

    /** Maximum distance from water sources (cm, 0 = no limit) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Distribution")
    float MaxWaterDistance = 0.0f;

    /** Slope tolerance (0-90 degrees) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Distribution")
    FVector2D SlopeTolerance = FVector2D(0.0f, 45.0f);

    /** Elevation preference (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Distribution")
    FVector2D ElevationPreference = FVector2D(-1000.0f, 5000.0f);

    /** Scale variation range */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Variation")
    FVector2D ScaleRange = FVector2D(0.8f, 1.2f);

    /** Rotation randomization */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Variation")
    bool bRandomizeRotation = true;

    /** Color variation intensity */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Variation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ColorVariationIntensity = 0.2f;

    /** Can be destroyed by large dinosaurs */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
    bool bCanBeDestroyed = true;

    /** Provides food for herbivores */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
    bool bProvidesFood = false;

    /** Provides hiding spots for player */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
    bool bProvidesConcealment = false;

    /** Blocks line of sight */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
    bool bBlocksLineOfSight = true;

    /** Nanite enabled for high detail */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    bool bUseNanite = true;

    /** LOD distances for performance optimization */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    TArray<float> LODDistances;

    /** Culling distance */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float CullingDistance = 10000.0f;
};

/** Environmental storytelling prop configuration */
USTRUCT(BlueprintType)
struct FEnvironmentalPropConfig
{
    GENERATED_BODY()

    /** Prop type classification */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prop")
    EEnvironmentalPropType PropType = EEnvironmentalPropType::FallenLog;

    /** Static mesh for this prop */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prop")
    TSoftObjectPtr<UStaticMesh> PropMesh;

    /** Material for this prop */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prop")
    TSoftObjectPtr<UMaterialInterface> PropMaterial;

    /** Spawn probability in suitable locations */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Distribution", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SpawnProbability = 0.1f;

    /** Preferred biomes for this prop */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Distribution")
    TArray<EBiomeType> PreferredBiomes;

    /** Clustering with other props of same type */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Distribution")
    bool bFormClusters = false;

    /** Cluster size range */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Distribution")
    FVector2D ClusterSizeRange = FVector2D(1.0f, 5.0f);

    /** Minimum distance between instances (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Distribution")
    float MinSpacing = 500.0f;

    /** Scale variation */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Variation")
    FVector2D ScaleRange = FVector2D(0.7f, 1.3f);

    /** Weathering intensity (affects material) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Variation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WeatheringIntensity = 0.5f;

    /** Tells a story about past events */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narrative")
    FText NarrativeContext;

    /** Can be interacted with by player */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
    bool bIsInteractable = false;

    /** Provides resources when interacted */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
    bool bProvidesResources = false;
};

/** Rock and geological feature configuration */
USTRUCT(BlueprintType)
struct FGeologicalFeatureConfig
{
    GENERATED_BODY()

    /** Static mesh for this geological feature */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Geology")
    TSoftObjectPtr<UStaticMesh> FeatureMesh;

    /** Material for this feature */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Geology")
    TSoftObjectPtr<UMaterialInterface> FeatureMaterial;

    /** Feature type classification */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Geology")
    FString FeatureType = "Boulder";

    /** Spawn density per km² */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Distribution")
    float SpawnDensityPerKm2 = 5.0f;

    /** Preferred slope range (degrees) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Distribution")
    FVector2D PreferredSlope = FVector2D(15.0f, 60.0f);

    /** Elevation preference (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Distribution")
    FVector2D ElevationRange = FVector2D(0.0f, 10000.0f);

    /** Forms clusters */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Distribution")
    bool bFormsClusters = true;

    /** Cluster density */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Distribution", meta = (ClampMin = "1", ClampMax = "20"))
    int32 ClusterSize = 3;

    /** Scale variation */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Variation")
    FVector2D ScaleRange = FVector2D(0.5f, 2.0f);

    /** Provides climbing opportunities */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
    bool bClimbable = false;

    /** Provides shelter */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
    bool bProvidesShelter = false;

    /** Can be mined for resources */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
    bool bMineable = false;
};

/** Biome-specific environment art configuration */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEnvironmentArtData : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UEnvironmentArtData();

    /** Target biome for this configuration */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome")
    EBiomeType TargetBiome = EBiomeType::DenseJungle;

    /** Vegetation configurations for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    TArray<FVegetationAssetConfig> VegetationConfigs;

    /** Environmental props for storytelling */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Props")
    TArray<FEnvironmentalPropConfig> EnvironmentalProps;

    /** Geological features for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Geology")
    TArray<FGeologicalFeatureConfig> GeologicalFeatures;

    /** Base landscape material */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Materials")
    TSoftObjectPtr<UMaterialInterface> BaseLandscapeMaterial;

    /** Material layers for landscape blending */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Materials")
    TMap<FString, TSoftObjectPtr<UMaterialInterface>> MaterialLayers;

    /** Ambient lighting color tint */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lighting")
    FLinearColor AmbientLightTint = FLinearColor::White;

    /** Fog density multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Atmosphere")
    float FogDensityMultiplier = 1.0f;

    /** Fog color tint */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Atmosphere")
    FLinearColor FogColorTint = FLinearColor::White;

    /** Wind intensity for vegetation animation */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
    float WindIntensity = 0.5f;

    /** Wind direction variance */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
    float WindDirectionVariance = 30.0f;

    /** Overall art style intensity */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Style", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float ArtStyleIntensity = 1.0f;
};

/** Main Environment Art System */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEnvironmentArtSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEnvironmentArtSystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /** Populate a biome region with environment art */
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void PopulateBiomeRegion(const FVector& RegionCenter, float RegionRadius, EBiomeType BiomeType);

    /** Generate vegetation for a specific area */
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void GenerateVegetationInArea(const FVector& AreaCenter, float AreaRadius, const FVegetationAssetConfig& VegConfig);

    /** Place environmental storytelling props */
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void PlaceEnvironmentalProps(const FVector& AreaCenter, float AreaRadius, EBiomeType BiomeType);

    /** Generate geological features */
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void GenerateGeologicalFeatures(const FVector& AreaCenter, float AreaRadius, EBiomeType BiomeType);

    /** Apply biome-specific materials to landscape */
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void ApplyBiomeMaterials(ALandscape* Landscape, EBiomeType BiomeType);

    /** Update vegetation LOD based on player distance */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateVegetationLOD(const FVector& PlayerLocation);

    /** Get environment art data for biome */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Environment Art")
    UEnvironmentArtData* GetEnvironmentArtData(EBiomeType BiomeType) const;

    /** Set material quality level */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetMaterialQuality(EMaterialQuality Quality);

    /** Enable/disable Nanite for vegetation */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetVegetationNaniteEnabled(bool bEnabled);

protected:
    /** Environment art data assets for each biome */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    TMap<EBiomeType, TSoftObjectPtr<UEnvironmentArtData>> BiomeArtData;

    /** Current material quality setting */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    EMaterialQuality CurrentMaterialQuality = EMaterialQuality::High;

    /** Maximum vegetation instances per region */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 MaxVegetationInstancesPerRegion = 50000;

    /** LOD update frequency (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float LODUpdateFrequency = 1.0f;

    /** Vegetation instance tracking */
    UPROPERTY()
    TMap<FVector, TArray<AActor*>> VegetationInstances;

    /** Environmental prop tracking */
    UPROPERTY()
    TMap<FVector, TArray<AActor*>> PropInstances;

    /** Geological feature tracking */
    UPROPERTY()
    TMap<FVector, TArray<AActor*>> GeologicalInstances;

private:
    /** Internal vegetation placement logic */
    void PlaceVegetationInstance(const FVector& Location, const FVegetationAssetConfig& Config);

    /** Internal prop placement logic */
    void PlaceEnvironmentalProp(const FVector& Location, const FEnvironmentalPropConfig& Config);

    /** Internal geological feature placement */
    void PlaceGeologicalFeature(const FVector& Location, const FGeologicalFeatureConfig& Config);

    /** Calculate vegetation density based on environmental factors */
    float CalculateVegetationDensity(const FVector& Location, EBiomeType BiomeType);

    /** Check if location is suitable for specific vegetation */
    bool IsLocationSuitableForVegetation(const FVector& Location, const FVegetationAssetConfig& Config);

    /** Generate clustered placement pattern */
    TArray<FVector> GenerateClusteredPlacement(const FVector& Center, float Radius, int32 Count, float ClusterFactor);

    /** Apply weathering effects to materials */
    void ApplyWeatheringEffects(UMaterialInstanceDynamic* Material, float WeatheringIntensity);

    /** LOD update timer */
    FTimerHandle LODUpdateTimer;

    /** Performance monitoring */
    int32 CurrentVegetationInstanceCount = 0;
    int32 CurrentPropInstanceCount = 0;
    int32 CurrentGeologicalInstanceCount = 0;
};