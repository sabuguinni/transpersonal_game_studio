// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/DataAsset.h"
#include "Materials/MaterialInterface.h"
#include "Components/StaticMeshComponent.h"
#include "Foliage/Public/FoliageType.h"
#include "Landscape/Classes/LandscapeProxy.h"
#include "../WorldGeneration/BiomeSystem.h"
#include "EnvironmentArtSystem.generated.h"

/**
 * @brief Environment Art System for Transpersonal Game Studio
 * 
 * Transforms procedurally generated terrain into a living, breathing prehistoric world.
 * This system populates the world with vegetation, rocks, props, and environmental storytelling
 * elements that make the player feel they've stepped into a real ecosystem.
 * 
 * Core Philosophy:
 * - Every detail tells a story about the world's history
 * - The environment should feel lived-in before the player arrives
 * - Visual composition guides player movement and creates memorable moments
 * - Atmospheric details create immersion and emotional connection
 * 
 * Key Features:
 * - Biome-specific vegetation placement using UE5 Foliage System
 * - Environmental storytelling through prop placement
 * - Landscape material blending for realistic terrain
 * - Atmospheric props (fallen logs, bone piles, abandoned items)
 * - Procedural detail placement with artistic control
 * - Performance-optimized LOD systems
 * 
 * Technical Implementation:
 * - Uses UE5 Foliage Mode for vegetation distribution
 * - Procedural Foliage Tool for large-scale placement
 * - Landscape materials with layer blending
 * - Custom material functions for environmental variation
 * - Nanite support for high-detail props
 * - World Partition integration for streaming
 * 
 * @author Environment Artist — Agent #6
 * @version 1.0 — March 2026
 */

/** Vegetation layer types for different height levels */
UENUM(BlueprintType)
enum class EVegetationLayer : uint8
{
    GroundCover     UMETA(DisplayName = "Ground Cover"),      // Grass, ferns, small plants
    Undergrowth     UMETA(DisplayName = "Undergrowth"),      // Bushes, saplings, medium plants
    Midstory        UMETA(DisplayName = "Midstory"),         // Small trees, large bushes
    Canopy          UMETA(DisplayName = "Canopy"),           // Large trees, primary forest layer
    Emergent        UMETA(DisplayName = "Emergent")          // Giant trees that break through canopy
};

/** Environmental prop categories for storytelling */
UENUM(BlueprintType)
enum class EEnvironmentalPropType : uint8
{
    NaturalFormation    UMETA(DisplayName = "Natural Formation"),    // Rocks, fallen trees, natural debris
    WildlifeRemnants    UMETA(DisplayName = "Wildlife Remnants"),    // Bones, nests, tracks, feeding signs
    WeatherEffects      UMETA(DisplayName = "Weather Effects"),      // Storm damage, erosion, growth patterns
    EcosystemSigns      UMETA(DisplayName = "Ecosystem Signs"),      // Animal paths, territorial markings
    MysterySigns        UMETA(DisplayName = "Mystery Signs"),        // Unexplained formations, ancient traces
    PlayerClues         UMETA(DisplayName = "Player Clues"),         // Hints about the time gem location
    AtmosphericDetail   UMETA(DisplayName = "Atmospheric Detail")    // Mood-setting elements
};

/** Material blend types for landscape painting */
UENUM(BlueprintType)
enum class ELandscapeMaterialType : uint8
{
    BaseGround      UMETA(DisplayName = "Base Ground"),       // Primary soil/dirt
    RichSoil        UMETA(DisplayName = "Rich Soil"),         // Fertile, dark earth
    RockyGround     UMETA(DisplayName = "Rocky Ground"),      // Stone and gravel
    SandySoil       UMETA(DisplayName = "Sandy Soil"),        // Light, sandy terrain
    ClayDeposits    UMETA(DisplayName = "Clay Deposits"),     // Clay-rich areas
    WetMud          UMETA(DisplayName = "Wet Mud"),           // Near water sources
    DriedMud        UMETA(DisplayName = "Dried Mud"),         // Cracked, dry areas
    LeafLitter      UMETA(DisplayName = "Leaf Litter"),       // Forest floor covering
    MossyCover      UMETA(DisplayName = "Mossy Cover"),       // Humid, shaded areas
    VolcanicAsh     UMETA(DisplayName = "Volcanic Ash"),      // Near volcanic regions
    Limestone       UMETA(DisplayName = "Limestone"),         // Cave entrances, cliffs
    Sandstone       UMETA(DisplayName = "Sandstone")          // Weathered rock formations
};

/** Atmospheric condition effects */
UENUM(BlueprintType)
enum class EAtmosphericCondition : uint8
{
    Clear           UMETA(DisplayName = "Clear"),
    LightFog        UMETA(DisplayName = "Light Fog"),
    DenseFog        UMETA(DisplayName = "Dense Fog"),
    Humidity        UMETA(DisplayName = "High Humidity"),
    Dust            UMETA(DisplayName = "Dust Particles"),
    Pollen          UMETA(DisplayName = "Pollen Clouds"),
    VolcanicHaze    UMETA(DisplayName = "Volcanic Haze"),
    OceanMist       UMETA(DisplayName = "Ocean Mist")
};

/** Vegetation asset configuration */
USTRUCT(BlueprintType)
struct FVegetationAssetConfig
{
    GENERATED_BODY()

    /** Static mesh for this vegetation */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    TSoftObjectPtr<UStaticMesh> VegetationMesh;

    /** Foliage type asset for advanced settings */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    TSoftObjectPtr<UFoliageType> FoliageType;

    /** Material override for seasonal/biome variations */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    TSoftObjectPtr<UMaterialInterface> MaterialOverride;

    /** Vegetation layer this belongs to */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    EVegetationLayer VegetationLayer = EVegetationLayer::GroundCover;

    /** Spawn density per 100m² */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
    float SpawnDensity = 50.0f;

    /** Spawn weight relative to other vegetation in same layer */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float SpawnWeight = 1.0f;

    /** Minimum distance from other instances of same type */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning", meta = (ClampMin = "0.0", ClampMax = "2000.0"))
    float MinSpacing = 100.0f;

    /** Scale variation range */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Variation")
    FVector2D ScaleRange = FVector2D(0.8f, 1.2f);

    /** Rotation variation (degrees) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Variation")
    FVector2D RotationRange = FVector2D(0.0f, 360.0f);

    /** Slope tolerance (0-90 degrees) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement", meta = (ClampMin = "0.0", ClampMax = "90.0"))
    FVector2D SlopeTolerance = FVector2D(0.0f, 45.0f);

    /** Elevation range for spawning (in cm) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    FVector2D ElevationRange = FVector2D(-10000.0f, 10000.0f);

    /** Requires proximity to water sources */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    bool bRequiresWaterProximity = false;

    /** Maximum distance from water if required (in cm) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement", meta = (ClampMin = "0.0", ClampMax = "10000.0"))
    float MaxWaterDistance = 1000.0f;

    /** Avoids other vegetation types */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    TArray<TSoftObjectPtr<UStaticMesh>> AvoidVegetationTypes;

    /** Prefers to spawn near these vegetation types */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    TArray<TSoftObjectPtr<UStaticMesh>> PreferVegetationTypes;

    /** Can be destroyed by large dinosaurs */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
    bool bCanBeDestroyed = true;

    /** Provides food for herbivorous dinosaurs */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
    bool bProvidesFood = false;

    /** Provides hiding spots for player and small creatures */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
    bool bProvidesConcealment = false;

    /** Can be harvested by player for resources */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
    bool bCanBeHarvested = false;

    /** Resource type provided when harvested */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
    FString HarvestResourceType;

    /** Amount of resource provided per harvest */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay", meta = (ClampMin = "0", ClampMax = "100"))
    int32 HarvestAmount = 1;
};

/** Environmental prop configuration */
USTRUCT(BlueprintType)
struct FEnvironmentalPropConfig
{
    GENERATED_BODY()

    /** Static mesh for this prop */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prop")
    TSoftObjectPtr<UStaticMesh> PropMesh;

    /** Material override for weathering/aging effects */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prop")
    TSoftObjectPtr<UMaterialInterface> MaterialOverride;

    /** Prop category for storytelling purposes */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prop")
    EEnvironmentalPropType PropType = EEnvironmentalPropType::NaturalFormation;

    /** Narrative description of what this prop represents */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Storytelling", meta = (MultiLine = true))
    FString StorytellingDescription;

    /** Spawn probability (0.0 - 1.0) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SpawnProbability = 0.1f;

    /** Clustering tendency - props of same type spawn near each other */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ClusteringTendency = 0.3f;

    /** Cluster radius if clustering is enabled */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning", meta = (ClampMin = "100.0", ClampMax = "5000.0"))
    float ClusterRadius = 500.0f;

    /** Scale variation range */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Variation")
    FVector2D ScaleRange = FVector2D(0.7f, 1.3f);

    /** Rotation variation (degrees) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Variation")
    FVector2D RotationRange = FVector2D(0.0f, 360.0f);

    /** Sink into ground percentage */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float GroundSinkPercentage = 0.1f;

    /** Preferred biomes for this prop */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    TArray<EBiomeType> PreferredBiomes;

    /** Avoided biomes for this prop */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    TArray<EBiomeType> AvoidedBiomes;

    /** Requires line of sight to specific landmarks */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    bool bRequiresLandmarkVisibility = false;

    /** Required landmark tags for visibility check */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    TArray<FString> RequiredLandmarkTags;

    /** Can be interacted with by player */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
    bool bIsInteractable = false;

    /** Interaction prompt text */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
    FText InteractionPrompt;

    /** Blueprint class to spawn for complex interactions */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
    TSoftClassPtr<AActor> InteractionActorClass;
};

/** Landscape material layer configuration */
USTRUCT(BlueprintType)
struct FLandscapeMaterialLayer
{
    GENERATED_BODY()

    /** Material type identifier */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Material")
    ELandscapeMaterialType MaterialType = ELandscapeMaterialType::BaseGround;

    /** Layer name for landscape painting */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Material")
    FString LayerName;

    /** Material instance for this layer */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Material")
    TSoftObjectPtr<UMaterialInterface> LayerMaterial;

    /** Texture scale for tiling */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Material", meta = (ClampMin = "0.1", ClampMax = "100.0"))
    float TextureScale = 1.0f;

    /** Normal map intensity */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Material", meta = (ClampMin = "0.0", ClampMax = "5.0"))
    float NormalIntensity = 1.0f;

    /** Roughness multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Material", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float RoughnessMultiplier = 1.0f;

    /** Preferred elevation range for this material */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    FVector2D ElevationRange = FVector2D(-1000.0f, 1000.0f);

    /** Preferred slope range (0-90 degrees) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    FVector2D SlopeRange = FVector2D(0.0f, 90.0f);

    /** Moisture preference (0.0 = dry, 1.0 = wet) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    FVector2D MoistureRange = FVector2D(0.0f, 1.0f);

    /** Distance from water for moisture calculation */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement", meta = (ClampMin = "0.0", ClampMax = "10000.0"))
    float WaterInfluenceDistance = 2000.0f;

    /** Biome preference weight */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    TMap<EBiomeType, float> BiomeWeights;
};

/** Atmospheric effect configuration */
USTRUCT(BlueprintType)
struct FAtmosphericEffectConfig
{
    GENERATED_BODY()

    /** Atmospheric condition type */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Atmosphere")
    EAtmosphericCondition ConditionType = EAtmosphericCondition::Clear;

    /** Particle system for the effect */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Atmosphere")
    TSoftObjectPtr<UNiagaraSystem> ParticleEffect;

    /** Effect intensity (0.0 - 1.0) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Atmosphere", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Intensity = 0.5f;

    /** Coverage area radius (in cm) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Atmosphere", meta = (ClampMin = "1000.0", ClampMax = "50000.0"))
    float CoverageRadius = 10000.0f;

    /** Height range for the effect */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Atmosphere")
    FVector2D HeightRange = FVector2D(0.0f, 2000.0f);

    /** Time of day preference (0.0 = midnight, 0.5 = noon, 1.0 = midnight) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Atmosphere")
    FVector2D TimeOfDayRange = FVector2D(0.0f, 1.0f);

    /** Weather condition requirements */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Atmosphere")
    TArray<FString> RequiredWeatherConditions;

    /** Biome preference for this effect */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Atmosphere")
    TArray<EBiomeType> PreferredBiomes;
};

/**
 * @brief Biome Environment Configuration
 * 
 * Defines all environmental art elements for a specific biome.
 * This includes vegetation layers, prop placement, material distribution,
 * and atmospheric effects that create the biome's unique character.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBiomeEnvironmentData : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UBiomeEnvironmentData();

    /** Biome this configuration applies to */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome Info")
    EBiomeType TargetBiome = EBiomeType::DenseJungle;

    /** Human-readable name for this environment configuration */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome Info")
    FText EnvironmentName;

    /** Artistic vision description */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome Info", meta = (MultiLine = true))
    FText ArtisticVision;

    /** Color palette for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome Info")
    TArray<FLinearColor> ColorPalette;

    /** Vegetation configurations for all layers */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    TArray<FVegetationAssetConfig> VegetationConfigs;

    /** Environmental props for storytelling */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Props")
    TArray<FEnvironmentalPropConfig> EnvironmentalProps;

    /** Landscape material layers */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Materials")
    TArray<FLandscapeMaterialLayer> MaterialLayers;

    /** Master landscape material for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Materials")
    TSoftObjectPtr<UMaterialInterface> MasterLandscapeMaterial;

    /** Atmospheric effects */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Atmosphere")
    TArray<FAtmosphericEffectConfig> AtmosphericEffects;

    /** Ambient sound configuration */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    TSoftObjectPtr<USoundBase> AmbientSound;

    /** Sound attenuation settings */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    TSoftObjectPtr<USoundAttenuation> SoundAttenuation;

    /** Overall vegetation density multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Density Control", meta = (ClampMin = "0.0", ClampMax = "5.0"))
    float VegetationDensityMultiplier = 1.0f;

    /** Overall prop density multiplier */
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
 * @brief Environment Art System - Main Controller
 * 
 * Manages the placement and configuration of all environmental art elements
 * across the procedurally generated world. Coordinates with the Procedural
 * World Generator to ensure artistic vision is maintained while respecting
 * performance constraints.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEnvironmentArtSystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UEnvironmentArtSystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

    /** Initialize environment art for a specific biome region */
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void InitializeBiomeEnvironment(EBiomeType BiomeType, const FVector& RegionCenter, float RegionRadius);

    /** Populate vegetation in a specific area */
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void PopulateVegetationInArea(const FVector& AreaCenter, float AreaRadius, EBiomeType BiomeType);

    /** Place environmental props for storytelling */
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void PlaceEnvironmentalProps(const FVector& AreaCenter, float AreaRadius, EBiomeType BiomeType);

    /** Apply landscape materials to terrain */
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void ApplyLandscapeMaterials(ALandscapeProxy* Landscape, EBiomeType BiomeType);

    /** Create atmospheric effects for the biome */
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void CreateAtmosphericEffects(const FVector& AreaCenter, float AreaRadius, EBiomeType BiomeType);

    /** Get biome environment configuration */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Environment Art")
    UBiomeEnvironmentData* GetBiomeEnvironmentData(EBiomeType BiomeType) const;

    /** Register a new biome environment configuration */
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void RegisterBiomeEnvironmentData(EBiomeType BiomeType, UBiomeEnvironmentData* EnvironmentData);

    /** Update vegetation density based on performance metrics */
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void UpdateVegetationDensity(float DensityMultiplier);

    /** Clear all environment art in a specific area */
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void ClearEnvironmentArt(const FVector& AreaCenter, float AreaRadius);

    /** Regenerate environment art after world changes */
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void RegenerateEnvironmentArt(const FVector& AreaCenter, float AreaRadius);

protected:
    /** Biome environment configurations */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    TMap<EBiomeType, TSoftObjectPtr<UBiomeEnvironmentData>> BiomeEnvironmentConfigs;

    /** Runtime biome environment data cache */
    UPROPERTY(Transient)
    TMap<EBiomeType, UBiomeEnvironmentData*> LoadedEnvironmentData;

    /** Active foliage instances for management */
    UPROPERTY(Transient)
    TArray<TWeakObjectPtr<UStaticMeshComponent>> ActiveFoliageInstances;

    /** Active prop instances for management */
    UPROPERTY(Transient)
    TArray<TWeakObjectPtr<AActor>> ActivePropInstances;

    /** Active atmospheric effects */
    UPROPERTY(Transient)
    TArray<TWeakObjectPtr<UNiagaraComponent>> ActiveAtmosphericEffects;

    /** Current vegetation density multiplier */
    UPROPERTY(Transient)
    float CurrentVegetationDensity = 1.0f;

    /** Current prop density multiplier */
    UPROPERTY(Transient)
    float CurrentPropDensity = 1.0f;

    /** Performance monitoring */
    UPROPERTY(Transient)
    float LastFrameTime = 0.0f;

    UPROPERTY(Transient)
    int32 ActiveInstanceCount = 0;

private:
    /** Internal helper functions */
    void LoadBiomeEnvironmentData(EBiomeType BiomeType);
    void PlaceVegetationLayer(const FVegetationAssetConfig& VegConfig, const FVector& AreaCenter, float AreaRadius);
    void PlaceSingleProp(const FEnvironmentalPropConfig& PropConfig, const FVector& Location, const FRotator& Rotation);
    bool IsValidPlacementLocation(const FVector& Location, const FVegetationAssetConfig& VegConfig) const;
    bool IsValidPropLocation(const FVector& Location, const FEnvironmentalPropConfig& PropConfig) const;
    FVector GetGroundLocation(const FVector& WorldLocation) const;
    float GetSlopeAngle(const FVector& Location) const;
    float GetMoistureLevel(const FVector& Location) const;
    void OptimizeInstancesForPerformance();
    void UpdateLODDistances();
};

/**
 * @brief Environment Art Component
 * 
 * Component that can be attached to actors to provide environment art
 * functionality and integration with the main Environment Art System.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEnvironmentArtComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UEnvironmentArtComponent();

    /** Initialize component with biome data */
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void InitializeWithBiome(EBiomeType BiomeType);

    /** Get the environment art system */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Environment Art")
    UEnvironmentArtSystem* GetEnvironmentArtSystem() const;

protected:
    /** Biome type this component represents */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment Art")
    EBiomeType AssignedBiome = EBiomeType::DenseJungle;

    /** Auto-initialize on begin play */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment Art")
    bool bAutoInitialize = true;

    /** Influence radius for environment art */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment Art", meta = (ClampMin = "100.0", ClampMax = "50000.0"))
    float InfluenceRadius = 5000.0f;

    // UActorComponent interface
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};