// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataAsset.h"
#include "Materials/MaterialInterface.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Foliage/Public/FoliageType.h"
#include "Foliage/Public/FoliageType_InstancedStaticMesh.h"
#include "../WorldGeneration/BiomeSystem.h"
#include "VegetationSystem.generated.h"

/**
 * @brief Vegetation System for Transpersonal Game Studio
 * 
 * Transforms procedurally generated terrain into a living, breathing prehistoric world.
 * Creates dense forests, undergrowth, ground cover, and scattered props that tell
 * environmental stories without words.
 * 
 * Core Philosophy:
 * - Every plant placement tells a story about the soil, water, and history beneath
 * - Vegetation clusters suggest animal paths, water sources, and shelter areas
 * - Density variations create natural gameplay spaces and sightlines
 * - Seasonal variation and growth states add temporal depth
 * 
 * Technical Features:
 * - Hierarchical Instanced Static Meshes for performance
 * - LOD management for distant vegetation
 * - Wind animation and interaction systems
 * - Destructible vegetation for dinosaur interactions
 * - Procedural variation using vertex painting and material instances
 * 
 * @author Environment Artist — Agent #6
 * @version 1.0 — March 2026
 */

/** Vegetation layer types for hierarchical placement */
UENUM(BlueprintType)
enum class EVegetationLayer : uint8
{
    Canopy          UMETA(DisplayName = "Forest Canopy"),
    Understory      UMETA(DisplayName = "Understory Trees"),
    Shrub           UMETA(DisplayName = "Shrub Layer"),
    Herbaceous      UMETA(DisplayName = "Herbaceous Layer"),
    GroundCover     UMETA(DisplayName = "Ground Cover"),
    Moss            UMETA(DisplayName = "Moss & Lichen"),
    Emergent        UMETA(DisplayName = "Emergent Giants")
};

/** Vegetation growth stages */
UENUM(BlueprintType)
enum class EVegetationGrowthStage : uint8
{
    Seedling        UMETA(DisplayName = "Seedling"),
    Juvenile        UMETA(DisplayName = "Juvenile"),
    Mature          UMETA(DisplayName = "Mature"),
    Ancient         UMETA(DisplayName = "Ancient"),
    Dying           UMETA(DisplayName = "Dying"),
    Dead            UMETA(DisplayName = "Dead/Fallen")
};

/** Vegetation health states */
UENUM(BlueprintType)
enum class EVegetationHealth : uint8
{
    Thriving        UMETA(DisplayName = "Thriving"),
    Healthy         UMETA(DisplayName = "Healthy"),
    Stressed        UMETA(DisplayName = "Stressed"),
    Diseased        UMETA(DisplayName = "Diseased"),
    Dying           UMETA(DisplayName = "Dying"),
    Dead            UMETA(DisplayName = "Dead")
};

/** Seasonal appearance variations */
UENUM(BlueprintType)
enum class ESeasonalVariation : uint8
{
    Spring          UMETA(DisplayName = "Spring Growth"),
    Summer          UMETA(DisplayName = "Summer Lush"),
    Autumn          UMETA(DisplayName = "Autumn Colors"),
    Winter          UMETA(DisplayName = "Winter Dormant"),
    WetSeason       UMETA(DisplayName = "Wet Season"),
    DrySeason       UMETA(DisplayName = "Dry Season")
};

/** Vegetation interaction data */
USTRUCT(BlueprintType)
struct FVegetationInteraction
{
    GENERATED_BODY()

    /** Can dinosaurs eat this vegetation? */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
    bool bEdibleToHerbivores = false;

    /** Can dinosaurs destroy this vegetation by walking through? */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
    bool bDestructibleByMovement = true;

    /** Can dinosaurs use this for shelter/hiding? */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
    bool bProvidesShelter = false;

    /** Can dinosaurs nest in/near this vegetation? */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
    bool bSuitableForNesting = false;

    /** Sound made when destroyed */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
    TSoftObjectPtr<USoundBase> DestructionSound;

    /** Particle effect when destroyed */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
    TSoftObjectPtr<UParticleSystem> DestructionEffect;

    /** Items dropped when destroyed */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
    TMap<FString, int32> DroppedItems;

    /** Health points (0 = indestructible) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (ClampMin = "0", ClampMax = "1000"))
    int32 HealthPoints = 10;

    /** Regrowth time in seconds (0 = no regrowth) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (ClampMin = "0.0", ClampMax = "3600.0"))
    float RegrowthTime = 300.0f;
};

/** Wind response parameters */
USTRUCT(BlueprintType)
struct FVegetationWindResponse
{
    GENERATED_BODY()

    /** How much this vegetation responds to wind (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wind", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WindSensitivity = 0.5f;

    /** Primary wind direction influence */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wind", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float PrimaryWindInfluence = 0.8f;

    /** Secondary wind turbulence influence */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wind", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float TurbulenceInfluence = 0.3f;

    /** Wind animation speed multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wind", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float AnimationSpeedMultiplier = 1.0f;

    /** Maximum bend angle in degrees */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wind", meta = (ClampMin = "0.0", ClampMax = "45.0"))
    float MaxBendAngle = 15.0f;
};

/** Complete vegetation asset definition */
USTRUCT(BlueprintType)
struct FVegetationAssetData
{
    GENERATED_BODY()

    /** Asset identification */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Asset Info")
    FString AssetName = "Prehistoric Fern";

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Asset Info")
    FString ScientificName = "Cyatheales prehistoricus";

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Asset Info", meta = (MultiLine = true))
    FString Description = "A large prehistoric fern commonly found in humid forest understory.";

    /** Mesh and material data */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rendering")
    TSoftObjectPtr<UStaticMesh> VegetationMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rendering")
    TArray<TSoftObjectPtr<UMaterialInterface>> MaterialVariations;

    /** LOD configuration */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rendering")
    TArray<float> LODDistances = {500.0f, 1500.0f, 3000.0f};

    /** Foliage type for UE5 foliage system */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rendering")
    TSoftObjectPtr<UFoliageType_InstancedStaticMesh> FoliageType;

    /** Placement parameters */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    EVegetationLayer VegetationLayer = EVegetationLayer::Herbaceous;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    float SpawnWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    FVector2D ScaleRange = FVector2D(0.8f, 1.2f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    float MinSpacing = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    float MaxSpacing = 400.0f;

    /** Environmental requirements */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment")
    FVector2D PreferredElevation = FVector2D(0.0f, 1000.0f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment")
    FVector2D PreferredSlope = FVector2D(0.0f, 30.0f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment")
    FVector2D PreferredMoisture = FVector2D(0.4f, 0.9f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment")
    FVector2D PreferredTemperature = FVector2D(15.0f, 30.0f);

    /** Biome compatibility */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment")
    TMap<EBiomeType, float> BiomeCompatibility;

    /** Growth and lifecycle */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lifecycle")
    EVegetationGrowthStage DefaultGrowthStage = EVegetationGrowthStage::Mature;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lifecycle")
    TMap<EVegetationGrowthStage, TSoftObjectPtr<UStaticMesh>> GrowthStageMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lifecycle")
    float GrowthRate = 1.0f; // Units per day

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lifecycle")
    float Lifespan = 365.0f; // Days

    /** Interaction data */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
    FVegetationInteraction InteractionData;

    /** Wind response */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
    FVegetationWindResponse WindResponse;

    /** Seasonal variations */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Seasonal")
    TMap<ESeasonalVariation, TSoftObjectPtr<UMaterialInterface>> SeasonalMaterials;

    /** Audio properties */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    TSoftObjectPtr<USoundBase> AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    TSoftObjectPtr<USoundBase> RustleSound;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    float AudioRange = 500.0f;
};

/** Vegetation cluster definition for natural grouping */
USTRUCT(BlueprintType)
struct FVegetationCluster
{
    GENERATED_BODY()

    /** Cluster identification */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cluster Info")
    FString ClusterName = "Forest Clearing Edge";

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cluster Info", meta = (MultiLine = true))
    FString ClusterDescription = "Vegetation pattern found at the edge of forest clearings.";

    /** Primary vegetation in this cluster */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Composition")
    TArray<FVegetationAssetData> PrimaryVegetation;

    /** Secondary vegetation (fills gaps) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Composition")
    TArray<FVegetationAssetData> SecondaryVegetation;

    /** Accent vegetation (rare, distinctive) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Composition")
    TArray<FVegetationAssetData> AccentVegetation;

    /** Cluster size range */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    FVector2D ClusterSizeRange = FVector2D(500.0f, 1500.0f);

    /** Cluster density (instances per square meter) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    FVector2D DensityRange = FVector2D(0.1f, 0.8f);

    /** Edge falloff distance */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    float EdgeFalloffDistance = 200.0f;

    /** Environmental story this cluster tells */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narrative")
    FString EnvironmentalStory = "Recent disturbance created this clearing, now slowly being reclaimed.";

    /** Associated props (fallen logs, rocks, etc.) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Props")
    TArray<TSoftObjectPtr<UStaticMesh>> AssociatedProps;

    /** Spawn probability for associated props */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Props")
    TArray<float> PropSpawnProbabilities;
};

/**
 * @brief Vegetation System Component
 * 
 * Manages vegetation placement, growth, interaction, and environmental storytelling.
 * Works in conjunction with the PCG system to create believable prehistoric ecosystems.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Environment), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVegetationSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UVegetationSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    /** Initialize vegetation system for a biome */
    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void InitializeForBiome(EBiomeType BiomeType, const FVector& WorldLocation, const FVector& WorldSize);

    /** Place vegetation in a specific area */
    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void PlaceVegetationInArea(const FVector& Center, float Radius, float Density);

    /** Place a specific vegetation cluster */
    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void PlaceVegetationCluster(const FVegetationCluster& Cluster, const FVector& Location);

    /** Remove vegetation in an area (for dinosaur destruction) */
    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void RemoveVegetationInArea(const FVector& Center, float Radius, bool bCreateDebris = true);

    /** Update seasonal appearance */
    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void UpdateSeasonalAppearance(ESeasonalVariation Season);

    /** Apply wind effects to vegetation */
    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void ApplyWindEffects(const FVector& WindDirection, float WindStrength);

    /** Get vegetation density at a location */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Vegetation")
    float GetVegetationDensityAtLocation(const FVector& Location) const;

    /** Check if location is suitable for specific vegetation */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Vegetation")
    bool IsLocationSuitableForVegetation(const FVegetationAssetData& VegetationData, const FVector& Location) const;

    /** Get environmental story at location */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Vegetation")
    FString GetEnvironmentalStoryAtLocation(const FVector& Location) const;

protected:
    /** Vegetation asset database */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    TMap<EBiomeType, TArray<FVegetationAssetData>> BiomeVegetationDatabase;

    /** Vegetation cluster definitions */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    TArray<FVegetationCluster> VegetationClusters;

    /** Current biome being managed */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    EBiomeType CurrentBiome = EBiomeType::DenseJungle;

    /** Current seasonal variation */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    ESeasonalVariation CurrentSeason = ESeasonalVariation::Summer;

    /** Hierarchical instanced mesh components for performance */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    TMap<FString, UHierarchicalInstancedStaticMeshComponent*> VegetationInstances;

    /** Wind system reference */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    TWeakObjectPtr<class UWindSystem> WindSystem;

    /** Performance optimization settings */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 MaxInstancesPerComponent = 10000;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float CullingDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 LODLevels = 4;

private:
    /** Internal vegetation placement logic */
    void PlaceVegetationInternal(const FVegetationAssetData& VegetationData, const FVector& Location, const FRotator& Rotation, const FVector& Scale);
    
    /** Calculate environmental suitability */
    float CalculateEnvironmentalSuitability(const FVegetationAssetData& VegetationData, const FVector& Location) const;
    
    /** Generate natural variation */
    FVector GenerateNaturalScale(const FVegetationAssetData& VegetationData) const;
    FRotator GenerateNaturalRotation() const;
    
    /** Update LOD levels based on distance */
    void UpdateLODLevels();
    
    /** Manage instance culling for performance */
    void UpdateInstanceCulling();
};

/**
 * @brief Vegetation Database Asset
 * 
 * Central repository of all vegetation assets organized by biome and type.
 * Used by the Environment Artist to define the visual palette of each biome.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UVegetationDatabase : public UDataAsset
{
    GENERATED_BODY()

public:
    /** Complete vegetation database organized by biome */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Database")
    TMap<EBiomeType, TArray<FVegetationAssetData>> VegetationByBiome;

    /** Predefined vegetation clusters */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Database")
    TArray<FVegetationCluster> VegetationClusters;

    /** Global vegetation settings */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
    float GlobalDensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
    float GlobalScaleVariation = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
    bool bEnableSeasonalVariation = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
    bool bEnableWindAnimation = true;

    /** Get vegetation assets for a specific biome */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Vegetation Database")
    TArray<FVegetationAssetData> GetVegetationForBiome(EBiomeType BiomeType) const;

    /** Get vegetation assets by layer */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Vegetation Database")
    TArray<FVegetationAssetData> GetVegetationByLayer(EBiomeType BiomeType, EVegetationLayer Layer) const;

    /** Get suitable vegetation clusters for a biome */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Vegetation Database")
    TArray<FVegetationCluster> GetClustersForBiome(EBiomeType BiomeType) const;
};