// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialParameterCollection.h"
#include "Engine/Texture2D.h"
#include "Landscape/Classes/LandscapeProxy.h"
#include "../WorldGeneration/BiomeSystem.h"
#include "LandscapeMaterialSystem.generated.h"

/**
 * @brief Landscape Material System for Prehistoric Environment
 * 
 * Creates realistic, layered landscape materials that blend seamlessly between biomes.
 * Uses physically-based materials with proper roughness, normal maps, and displacement.
 * 
 * Material Layers:
 * - Base Layer: Primary terrain material (dirt, rock, sand)
 * - Detail Layer: Secondary materials (moss, lichen, erosion)
 * - Wetness Layer: Water influence and moisture
 * - Vegetation Layer: Organic matter and decomposition
 * - Weathering Layer: Age and environmental wear
 * 
 * @author Environment Artist — Agent #6
 * @version 1.0 — March 2026
 */

/** Landscape material layer types */
UENUM(BlueprintType)
enum class ELandscapeLayerType : uint8
{
    BaseRock            UMETA(DisplayName = "Base Rock"),
    Dirt                UMETA(DisplayName = "Dirt/Soil"),
    Sand                UMETA(DisplayName = "Sand"),
    Clay                UMETA(DisplayName = "Clay"),
    Gravel              UMETA(DisplayName = "Gravel"),
    Moss                UMETA(DisplayName = "Moss"),
    Lichen              UMETA(DisplayName = "Lichen"),
    MudWet              UMETA(DisplayName = "Wet Mud"),
    MudDry              UMETA(DisplayName = "Dry Mud"),
    Erosion             UMETA(DisplayName = "Erosion Patterns"),
    OrganicMatter       UMETA(DisplayName = "Organic Matter"),
    Weathering          UMETA(DisplayName = "Weathering"),
    Vegetation          UMETA(DisplayName = "Vegetation Blend"),
    Water               UMETA(DisplayName = "Water Influence")
};

/** Material blend mode for landscape layers */
UENUM(BlueprintType)
enum class ELandscapeBlendMode : uint8
{
    WeightBlend         UMETA(DisplayName = "Weight Blend"),
    AlphaBlend          UMETA(DisplayName = "Alpha Blend"),
    HeightBlend         UMETA(DisplayName = "Height Blend"),
    NormalBlend         UMETA(DisplayName = "Normal Blend")
};

/** Texture set for a landscape material layer */
USTRUCT(BlueprintType)
struct FLandscapeTextureSet
{
    GENERATED_BODY()

    /** Diffuse/Albedo texture */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Textures")
    TSoftObjectPtr<UTexture2D> DiffuseTexture;

    /** Normal map texture */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Textures")
    TSoftObjectPtr<UTexture2D> NormalTexture;

    /** Roughness texture (R), Metallic (G), AO (B) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Textures")
    TSoftObjectPtr<UTexture2D> RoughnessMetallicAOTexture;

    /** Height/Displacement texture */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Textures")
    TSoftObjectPtr<UTexture2D> HeightTexture;

    /** Opacity/Mask texture */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Textures")
    TSoftObjectPtr<UTexture2D> OpacityTexture;

    /** Texture tiling scale */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Textures")
    FVector2D TilingScale = FVector2D(1.0f, 1.0f);

    /** Texture offset */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Textures")
    FVector2D TextureOffset = FVector2D(0.0f, 0.0f);

    /** Texture rotation (degrees) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Textures")
    float TextureRotation = 0.0f;
};

/** Material layer configuration */
USTRUCT(BlueprintType)
struct FLandscapeMaterialLayer
{
    GENERATED_BODY()

    /** Layer type identifier */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Layer Info")
    ELandscapeLayerType LayerType = ELandscapeLayerType::BaseRock;

    /** Layer name for identification */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Layer Info")
    FString LayerName = "BaseRock";

    /** Layer description */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Layer Info")
    FString LayerDescription = "Primary rock material";

    /** Texture set for this layer */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Textures")
    FLandscapeTextureSet TextureSet;

    /** Blend mode for this layer */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blending")
    ELandscapeBlendMode BlendMode = ELandscapeBlendMode::WeightBlend;

    /** Blend weight/influence */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blending", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float BlendWeight = 1.0f;

    /** Height blend range */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blending")
    FVector2D HeightBlendRange = FVector2D(0.0f, 1.0f);

    /** Slope blend range (degrees) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blending")
    FVector2D SlopeBlendRange = FVector2D(0.0f, 90.0f);

    /** Material properties */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Material Properties")
    FLinearColor BaseColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Material Properties", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Roughness = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Material Properties", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Metallic = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Material Properties", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Specular = 0.5f;

    /** Normal map intensity */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Material Properties", meta = (ClampMin = "0.0", ClampMax = "5.0"))
    float NormalIntensity = 1.0f;

    /** Displacement intensity */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Material Properties", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float DisplacementIntensity = 1.0f;

    /** Biome compatibility (how well this layer fits each biome) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome Compatibility")
    TMap<EBiomeType, float> BiomeCompatibility;

    /** Environmental conditions that favor this layer */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environmental Conditions")
    FVector2D TemperatureRange = FVector2D(-50.0f, 50.0f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environmental Conditions")
    FVector2D HumidityRange = FVector2D(0.0f, 100.0f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environmental Conditions")
    FVector2D ElevationRange = FVector2D(-1000.0f, 10000.0f);

    /** Weathering and aging properties */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weathering")
    float WeatheringRate = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weathering")
    float ErosionResistance = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weathering")
    float MoistureRetention = 0.5f;
};

/** Biome-specific landscape material configuration */
USTRUCT(BlueprintType)
struct FBiomeLandscapeMaterial
{
    GENERATED_BODY()

    /** Target biome */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::DenseJungle;

    /** Material layers for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Material Layers")
    TArray<FLandscapeMaterialLayer> MaterialLayers;

    /** Transition blend distance with other biomes */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transitions")
    float TransitionBlendDistance = 1000.0f;

    /** Transition falloff curve */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transitions")
    UCurveFloat* TransitionCurve;

    /** Environmental parameters */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment")
    float AverageTemperature = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment")
    float AverageHumidity = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment")
    float RainfallIntensity = 0.5f;

    /** Seasonal variation parameters */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Seasonal")
    float SeasonalTemperatureVariation = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Seasonal")
    float SeasonalHumidityVariation = 20.0f;

    /** Time-of-day lighting adjustments */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lighting")
    FLinearColor DayTimeColorTint = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lighting")
    FLinearColor NightTimeColorTint = FLinearColor(0.2f, 0.3f, 0.5f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lighting")
    float SunlightIntensityMultiplier = 1.0f;
};

/**
 * @brief Landscape Material Configuration Data Asset
 * 
 * Contains all material configurations for different biomes and environmental conditions.
 * Supports dynamic blending, weathering, and seasonal variations.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ULandscapeMaterialConfiguration : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    ULandscapeMaterialConfiguration();

    /** Master landscape material */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Master Material")
    TSoftObjectPtr<UMaterial> MasterLandscapeMaterial;

    /** Material parameter collection for global parameters */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Master Material")
    TSoftObjectPtr<UMaterialParameterCollection> GlobalParameters;

    /** Biome-specific material configurations */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome Materials")
    TArray<FBiomeLandscapeMaterial> BiomeMaterials;

    /** Global material settings */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Global Settings")
    float GlobalRoughnessMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Global Settings")
    float GlobalNormalIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Global Settings")
    float GlobalDisplacementScale = 1.0f;

    /** Texture streaming settings */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 TextureStreamingPoolSize = 1024; // MB

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float TextureLODBias = 0.0f;

    /** Distance-based quality settings */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float HighQualityDistance = 5000.0f; // 50m

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float MediumQualityDistance = 20000.0f; // 200m

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float LowQualityDistance = 50000.0f; // 500m

    /** Get material configuration for specific biome */
    UFUNCTION(BlueprintCallable, Category = "Landscape Material")
    const FBiomeLandscapeMaterial* GetBiomeMaterial(EBiomeType BiomeType) const;

    /** Get blended material parameters for position between biomes */
    UFUNCTION(BlueprintCallable, Category = "Landscape Material")
    FBiomeLandscapeMaterial GetBlendedBiomeMaterial(const TMap<EBiomeType, float>& BiomeWeights) const;

    /** Update global material parameters */
    UFUNCTION(BlueprintCallable, Category = "Landscape Material")
    void UpdateGlobalParameters(float TimeOfDay, float SeasonProgress, float WeatherIntensity);

private:
    /** Cached material instances for performance */
    UPROPERTY(Transient)
    TMap<EBiomeType, UMaterialInstanceDynamic*> CachedMaterialInstances;
};

/**
 * @brief Landscape Material Manager
 * 
 * Manages dynamic material application and blending across the landscape.
 * Handles real-time updates based on environmental conditions.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ULandscapeMaterialManager : public UObject
{
    GENERATED_BODY()

public:
    ULandscapeMaterialManager();

    /** Initialize the material system */
    UFUNCTION(BlueprintCallable, Category = "Landscape Material")
    void Initialize(ULandscapeMaterialConfiguration* Configuration);

    /** Apply materials to landscape based on biome data */
    UFUNCTION(BlueprintCallable, Category = "Landscape Material")
    void ApplyMaterialsToLandscape(ALandscapeProxy* Landscape, const TMap<EBiomeType, float>& BiomeWeights);

    /** Update material parameters for environmental changes */
    UFUNCTION(BlueprintCallable, Category = "Landscape Material")
    void UpdateEnvironmentalParameters(float TimeOfDay, float SeasonProgress, float WeatherIntensity);

    /** Create material instance for specific biome */
    UFUNCTION(BlueprintCallable, Category = "Landscape Material")
    UMaterialInstanceDynamic* CreateBiomeMaterialInstance(EBiomeType BiomeType);

    /** Blend materials between biomes */
    UFUNCTION(BlueprintCallable, Category = "Landscape Material")
    void BlendBiomeMaterials(const TArray<EBiomeType>& BiomeTypes, const TArray<float>& BlendWeights, FVector WorldPosition);

protected:
    /** Material configuration */
    UPROPERTY(BlueprintReadOnly, Category = "Configuration")
    ULandscapeMaterialConfiguration* MaterialConfiguration;

    /** Active material instances */
    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TMap<EBiomeType, UMaterialInstanceDynamic*> ActiveMaterialInstances;

    /** Current environmental parameters */
    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    float CurrentTimeOfDay = 12.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    float CurrentSeasonProgress = 0.5f;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    float CurrentWeatherIntensity = 0.0f;

private:
    /** Update material parameters */
    void UpdateMaterialParameters(UMaterialInstanceDynamic* MaterialInstance, const FBiomeLandscapeMaterial& BiomeMaterial);

    /** Calculate blend weights based on distance */
    float CalculateBlendWeight(float Distance, float BlendRadius) const;
};