// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Landscape/Classes/LandscapeProxy.h"
#include "FoliageType_InstancedStaticMesh.h"
#include "InstancedFoliageActor.h"
#include "../WorldGeneration/BiomeSystem.h"
#include "EnvironmentArtSystem.generated.h"

/**
 * @brief Environment Art System for Transpersonal Game Studio
 * 
 * Transforms procedurally generated terrain into a living, breathing prehistoric world.
 * Inspired by Roger Deakins' philosophy: "Light and composition tell the story before any character speaks"
 * and RDR2's environmental design: "Every detail exists to create an illusion of history"
 * 
 * Core Philosophy:
 * - A player should never feel they're in a game environment
 * - Every prop tells a story of what happened before the player arrived
 * - Environmental narrative is written through placement, not dialogue
 * 
 * Features:
 * - Biome-specific vegetation distribution using UE5 Foliage system
 * - Procedural rock and prop placement with narrative context
 * - Dynamic material blending based on environmental conditions
 * - Atmospheric storytelling through strategic asset placement
 * - Performance-optimized instancing for massive vegetation density
 * 
 * @author Environment Artist — Agent #6
 * @version 1.0 — March 2026
 */

/** Vegetation placement strategy */
UENUM(BlueprintType)
enum class EVegetationPlacementType : uint8
{
    Random          UMETA(DisplayName = "Random Distribution"),
    Clustered       UMETA(DisplayName = "Natural Clusters"),
    Linear          UMETA(DisplayName = "Linear Patterns"),
    Scattered       UMETA(DisplayName = "Scattered Placement"),
    EdgeBased       UMETA(DisplayName = "Edge-Based Placement"),
    HeightBased     UMETA(DisplayName = "Height-Based Placement")
};

/** Environmental storytelling prop types */
UENUM(BlueprintType)
enum class EStorytellingPropType : uint8
{
    AbandonedCampfire   UMETA(DisplayName = "Abandoned Campfire"),
    BrokenWeapons       UMETA(DisplayName = "Broken Weapons"),
    AnimalBones         UMETA(DisplayName = "Animal Bones"),
    DinosaurNests       UMETA(DisplayName = "Dinosaur Nests"),
    AncientRuins        UMETA(DisplayName = "Ancient Ruins"),
    FossilizedRemains   UMETA(DisplayName = "Fossilized Remains"),
    CaveDrawings        UMETA(DisplayName = "Cave Drawings"),
    MysteriousArtifacts UMETA(DisplayName = "Mysterious Artifacts"),
    NaturalLandmarks    UMETA(DisplayName = "Natural Landmarks"),
    WaterSources        UMETA(DisplayName = "Water Sources")
};

/** Material blend layers for landscape */
UENUM(BlueprintType)
enum class ELandscapeMaterialLayer : uint8
{
    BaseRock        UMETA(DisplayName = "Base Rock"),
    Soil            UMETA(DisplayName = "Fertile Soil"),
    Sand            UMETA(DisplayName = "Sand"),
    Mud             UMETA(DisplayName = "Mud"),
    Grass           UMETA(DisplayName = "Grass"),
    Moss            UMETA(DisplayName = "Moss"),
    Snow            UMETA(DisplayName = "Snow"),
    Lava            UMETA(DisplayName = "Volcanic Lava"),
    Water           UMETA(DisplayName = "Water Surface"),
    Vegetation      UMETA(DisplayName = "Dense Vegetation")
};

/** Vegetation instance data for performance optimization */
USTRUCT(BlueprintType)
struct FVegetationInstanceData
{
    GENERATED_BODY()

    /** World location of this vegetation instance */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instance")
    FVector Location = FVector::ZeroVector;

    /** Rotation of this instance */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instance")
    FRotator Rotation = FRotator::ZeroRotator;

    /** Scale of this instance */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instance")
    FVector Scale = FVector::OneVector;

    /** Age of this vegetation (affects appearance) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Age = 0.5f;

    /** Health state (affects material properties) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Health = 1.0f;

    /** Wind influence factor */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instance", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float WindInfluence = 1.0f;

    /** Seasonal variation factor */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SeasonalVariation = 0.0f;
};

/** Environmental prop placement data */
USTRUCT(BlueprintType)
struct FEnvironmentalProp
{
    GENERATED_BODY()

    /** Type of storytelling prop */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prop")
    EStorytellingPropType PropType = EStorytellingPropType::AnimalBones;

    /** Static mesh for this prop */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prop")
    TSoftObjectPtr<UStaticMesh> PropMesh;

    /** Material for this prop */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prop")
    TSoftObjectPtr<UMaterialInterface> PropMaterial;

    /** Spawn probability in biome (0.0 - 1.0) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prop", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SpawnProbability = 0.1f;

    /** Minimum distance from player spawn points */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prop", meta = (ClampMin = "0.0", ClampMax = "10000.0"))
    float MinDistanceFromSpawn = 500.0f;

    /** Preferred elevation range for spawning */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prop")
    FVector2D PreferredElevation = FVector2D(0.0f, 1000.0f);

    /** Scale variation range */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prop")
    FVector2D ScaleRange = FVector2D(0.8f, 1.2f);

    /** Narrative context description */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prop")
    FText NarrativeContext;

    /** Can this prop be interacted with? */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prop")
    bool bIsInteractable = false;

    /** Does this prop provide gameplay functionality? */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prop")
    bool bProvidesGameplayFunction = false;
};

/** Atmospheric lighting configuration */
USTRUCT(BlueprintType)
struct FAtmosphericSettings
{
    GENERATED_BODY()

    /** Base fog density */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FogDensity = 0.3f;

    /** Fog color tint */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);

    /** Volumetric fog intensity */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float VolumetricFogIntensity = 0.5f;

    /** God ray intensity */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float GodRayIntensity = 0.8f;

    /** Ambient sound attenuation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AmbientSoundAttenuation = 0.7f;

    /** Wind strength affecting vegetation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float WindStrength = 0.6f;

    /** Wind direction (normalized) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FVector WindDirection = FVector(1.0f, 0.0f, 0.0f);
};

/** Material parameter set for dynamic blending */
USTRUCT(BlueprintType)
struct FMaterialParameterSet
{
    GENERATED_BODY()

    /** Base material to modify */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
    TSoftObjectPtr<UMaterialInterface> BaseMaterial;

    /** Scalar parameters to set */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
    TMap<FName, float> ScalarParameters;

    /** Vector parameters to set */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
    TMap<FName, FLinearColor> VectorParameters;

    /** Texture parameters to set */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
    TMap<FName, TSoftObjectPtr<UTexture>> TextureParameters;
};

/**
 * @brief Environment Art Manager
 * 
 * Central system that coordinates all environmental art placement and management.
 * Works closely with the BiomeSystem to create visually stunning and narratively rich environments.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvironmentArtManager : public AActor
{
    GENERATED_BODY()

public:
    AEnvironmentArtManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    /** Initialize the environment art system */
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void InitializeEnvironmentSystem();

    /** Generate vegetation for a specific biome area */
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void GenerateVegetationForBiome(const UBiomeData* BiomeData, const FVector& CenterLocation, float Radius);

    /** Place environmental storytelling props */
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void PlaceStorytellingProps(const UBiomeData* BiomeData, const FVector& CenterLocation, float Radius);

    /** Update material parameters based on environmental conditions */
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void UpdateEnvironmentalMaterials(const FVector& PlayerLocation);

    /** Apply atmospheric settings to the world */
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void ApplyAtmosphericSettings(const FAtmosphericSettings& Settings);

    /** Get vegetation density at world location */
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    float GetVegetationDensityAtLocation(const FVector& WorldLocation) const;

    /** Check if location is suitable for prop placement */
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    bool IsLocationSuitableForProp(const FVector& Location, const FEnvironmentalProp& PropData) const;

    /** Create dynamic material instance for environmental blending */
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    UMaterialInstanceDynamic* CreateEnvironmentalMaterial(UMaterialInterface* BaseMaterial, const FMaterialParameterSet& Parameters);

protected:
    /** Reference to the world's biome system */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Environment Art")
    TObjectPtr<class UBiomeSystem> BiomeSystem;

    /** Foliage actor for managing vegetation instances */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Environment Art")
    TObjectPtr<AInstancedFoliageActor> FoliageActor;

    /** Hierarchical instanced static mesh components for different vegetation types */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Environment Art")
    TMap<FString, UHierarchicalInstancedStaticMeshComponent*> VegetationComponents;

    /** Environmental props placed in the world */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Environment Art")
    TArray<AActor*> PlacedEnvironmentalProps;

    /** Dynamic material instances for environmental blending */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Environment Art")
    TArray<UMaterialInstanceDynamic*> DynamicMaterials;

    /** Current atmospheric settings */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Art")
    FAtmosphericSettings CurrentAtmosphericSettings;

    /** Performance settings */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxVegetationInstancesPerChunk = 10000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float VegetationCullingDistance = 15000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxEnvironmentalPropsPerBiome = 500;

private:
    /** Generate vegetation cluster at specific location */
    void GenerateVegetationCluster(const FVegetationAsset& VegetationAsset, const FVector& ClusterCenter, float ClusterRadius, int32 InstanceCount);

    /** Calculate optimal vegetation placement using Poisson disk sampling */
    TArray<FVector> GeneratePoissonDiskSampling(const FVector& CenterLocation, float Radius, float MinDistance, int32 MaxAttempts = 30);

    /** Apply environmental storytelling through prop placement */
    void CreateNarrativeScene(const FVector& Location, EStorytellingPropType SceneType);

    /** Update vegetation materials based on seasonal and environmental factors */
    void UpdateVegetationMaterials(float DeltaTime);

    /** Performance optimization: cull distant vegetation instances */
    void CullDistantVegetation(const FVector& ViewerLocation);

    /** Blend landscape materials based on biome transitions */
    void BlendLandscapeMaterials(ALandscapeProxy* Landscape, const TArray<UBiomeData*>& NearbyBiomes);
};

/**
 * @brief Vegetation Asset Library
 * 
 * Data asset containing all vegetation meshes and materials for the prehistoric world.
 * Organized by biome type and vegetation category for efficient lookup and spawning.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UVegetationAssetLibrary : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UVegetationAssetLibrary();

    /** Get vegetation assets for a specific biome */
    UFUNCTION(BlueprintCallable, Category = "Vegetation Library")
    TArray<FVegetationAsset> GetVegetationForBiome(EBiomeType BiomeType) const;

    /** Get random vegetation asset from biome with weighted selection */
    UFUNCTION(BlueprintCallable, Category = "Vegetation Library")
    FVegetationAsset GetRandomVegetationAsset(EBiomeType BiomeType) const;

    /** Check if vegetation asset exists for biome */
    UFUNCTION(BlueprintCallable, Category = "Vegetation Library")
    bool HasVegetationForBiome(EBiomeType BiomeType) const;

protected:
    /** Vegetation assets organized by biome type */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation Library")
    TMap<EBiomeType, TArray<FVegetationAsset>> BiomeVegetationMap;

    /** Default vegetation for fallback */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation Library")
    TArray<FVegetationAsset> DefaultVegetation;
};

/**
 * @brief Environmental Prop Library
 * 
 * Data asset containing all environmental storytelling props for creating narrative scenes.
 * Each prop is designed to tell a story about the prehistoric world and its inhabitants.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEnvironmentalPropLibrary : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UEnvironmentalPropLibrary();

    /** Get props suitable for a specific biome */
    UFUNCTION(BlueprintCallable, Category = "Prop Library")
    TArray<FEnvironmentalProp> GetPropsForBiome(EBiomeType BiomeType) const;

    /** Get props of a specific storytelling type */
    UFUNCTION(BlueprintCallable, Category = "Prop Library")
    TArray<FEnvironmentalProp> GetPropsByType(EStorytellingPropType PropType) const;

    /** Get random prop for creating narrative scenes */
    UFUNCTION(BlueprintCallable, Category = "Prop Library")
    FEnvironmentalProp GetRandomProp(EBiomeType BiomeType, EStorytellingPropType PreferredType = EStorytellingPropType::AnimalBones) const;

protected:
    /** Environmental props organized by biome compatibility */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prop Library")
    TMap<EBiomeType, TArray<FEnvironmentalProp>> BiomePropMap;

    /** Props organized by storytelling function */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prop Library")
    TMap<EStorytellingPropType, TArray<FEnvironmentalProp>> StorytellingPropMap;
};