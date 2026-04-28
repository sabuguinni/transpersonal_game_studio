// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialParameterCollection.h"
#include "Engine/Texture2D.h"
#include "LandscapeProxy.h"
#include "MaterialSystem.generated.h"

/**
 * @brief Material System for Environment Art
 * 
 * Manages dynamic material blending and variation for the prehistoric world.
 * Creates realistic surface materials that respond to environmental conditions.
 * 
 * Features:
 * - Biome-specific landscape material blending
 * - Weather-responsive material parameters
 * - Procedural wear and aging effects
 * - Dynamic wetness and moisture systems
 * - Seasonal variation support
 * - Performance-optimized material instances
 * 
 * @author Environment Artist — Agent #6
 * @version 1.0 — March 2026
 */

/** Surface material types for the prehistoric world */
UENUM(BlueprintType)
enum class ESurfaceMaterialType : uint8
{
    // Terrain Base
    RichSoil            UMETA(DisplayName = "Rich Soil"),
    ClayDeposits        UMETA(DisplayName = "Clay Deposits"),
    SandyGround         UMETA(DisplayName = "Sandy Ground"),
    RockyTerrain        UMETA(DisplayName = "Rocky Terrain"),
    VolcanicSoil        UMETA(DisplayName = "Volcanic Soil"),
    
    // Vegetation Areas
    ForestFloor         UMETA(DisplayName = "Forest Floor"),
    JungleUndergrowth   UMETA(DisplayName = "Jungle Undergrowth"),
    GrasslandSoil       UMETA(DisplayName = "Grassland Soil"),
    SwampMuck           UMETA(DisplayName = "Swamp Muck"),
    
    // Rock Types
    Limestone           UMETA(DisplayName = "Limestone"),
    Sandstone           UMETA(DisplayName = "Sandstone"),
    Granite             UMETA(DisplayName = "Granite"),
    VolcanicRock        UMETA(DisplayName = "Volcanic Rock"),
    WeatheredStone      UMETA(DisplayName = "Weathered Stone"),
    
    // Water Features
    RiverbedSand        UMETA(DisplayName = "Riverbed Sand"),
    WetMud              UMETA(DisplayName = "Wet Mud"),
    CoastalSand         UMETA(DisplayName = "Coastal Sand"),
    ShallowWater        UMETA(DisplayName = "Shallow Water"),
    
    // Special Surfaces
    CrystalFormations   UMETA(DisplayName = "Crystal Formations"),
    FossilizedSurfaces  UMETA(DisplayName = "Fossilized Surfaces"),
    AmberDeposits       UMETA(DisplayName = "Amber Deposits"),
    CharredGround       UMETA(DisplayName = "Charred Ground")
};

/** Material blend modes for layering */
UENUM(BlueprintType)
enum class EMaterialBlendMode : uint8
{
    HeightBlend         UMETA(DisplayName = "Height-Based Blend"),
    AlphaBlend          UMETA(DisplayName = "Alpha Blend"),
    WeightBlend         UMETA(DisplayName = "Weight Blend"),
    NoiseBlend          UMETA(DisplayName = "Noise-Based Blend"),
    DistanceBlend       UMETA(DisplayName = "Distance-Based Blend")
};

/** Weather conditions affecting materials */
UENUM(BlueprintType)
enum class EWeatherCondition : uint8
{
    Clear               UMETA(DisplayName = "Clear Weather"),
    LightRain           UMETA(DisplayName = "Light Rain"),
    HeavyRain           UMETA(DisplayName = "Heavy Rain"),
    Fog                 UMETA(DisplayName = "Fog"),
    Storm               UMETA(DisplayName = "Storm"),
    Drought             UMETA(DisplayName = "Drought"),
    HighHumidity        UMETA(DisplayName = "High Humidity")
};

/** Material layer data for landscape blending */
USTRUCT(BlueprintType)
struct FMaterialLayerData
{
    GENERATED_BODY()

    /** Layer name for identification */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer Info")
    FString LayerName;

    /** Base material for this layer */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
    TSoftObjectPtr<UMaterialInterface> BaseMaterial;

    /** Normal map for surface detail */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textures")
    TSoftObjectPtr<UTexture2D> NormalMap;

    /** Roughness map */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textures")
    TSoftObjectPtr<UTexture2D> RoughnessMap;

    /** Height map for blending */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textures")
    TSoftObjectPtr<UTexture2D> HeightMap;

    /** Ambient occlusion map */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textures")
    TSoftObjectPtr<UTexture2D> AOMap;

    /** Surface material type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classification")
    ESurfaceMaterialType SurfaceType = ESurfaceMaterialType::RichSoil;

    /** Blend mode for this layer */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blending")
    EMaterialBlendMode BlendMode = EMaterialBlendMode::HeightBlend;

    /** UV tiling scale */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UV Mapping")
    FVector2D UVScale = FVector2D(1.0f, 1.0f);

    /** Material metallic value */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PBR Properties", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Metallic = 0.0f;

    /** Material specular value */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PBR Properties", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Specular = 0.5f;

    /** Base roughness value */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PBR Properties", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float BaseRoughness = 0.5f;

    /** Wetness response strength */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Response", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float WetnessResponse = 1.0f;

    /** Moisture absorption rate */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Response", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MoistureAbsorption = 0.5f;

    /** Drying rate after rain */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Response", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DryingRate = 0.3f;
};

/** Dynamic material parameter data */
USTRUCT(BlueprintType)
struct FDynamicMaterialParams
{
    GENERATED_BODY()

    /** Current wetness level (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WetnessLevel = 0.0f;

    /** Current moisture level (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float MoistureLevel = 0.5f;

    /** Temperature factor (-1 to 1, cold to hot) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Temperature = 0.0f;

    /** Wind strength (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WindStrength = 0.3f;

    /** Time of day (0-1, midnight to midnight) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float TimeOfDay = 0.5f;

    /** Season factor (0-1, winter to summer) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float SeasonFactor = 0.5f;

    /** Wear and aging factor (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aging")
    float WearFactor = 0.2f;

    /** Vegetation growth factor (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Growth")
    float VegetationGrowth = 0.7f;
};

/**
 * @brief Material Configuration Data Asset
 * 
 * Defines material blending rules and dynamic parameters for environment surfaces.
 * Supports complex material layering with weather and time-based variations.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UMaterialConfigData : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UMaterialConfigData();

    /** Configuration name */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config Info")
    FText ConfigName;

    /** Description of this material configuration */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config Info")
    FText ConfigDescription;

    /** Master landscape material */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Material")
    TSoftObjectPtr<UMaterialInterface> MasterLandscapeMaterial;

    /** Material parameter collection for dynamic updates */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dynamic Parameters")
    TSoftObjectPtr<UMaterialParameterCollection> ParameterCollection;

    /** Material layers for blending */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Material Layers")
    TArray<FMaterialLayerData> MaterialLayers;

    /** Landscape layer info objects */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Landscape Layers")
    TArray<TSoftObjectPtr<ULandscapeLayerInfoObject>> LandscapeLayerInfos;

    /** Default dynamic parameters */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Default Parameters")
    FDynamicMaterialParams DefaultParameters;

    /** Weather response curves */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather Response")
    TMap<EWeatherCondition, FDynamicMaterialParams> WeatherResponses;

    /** Update frequency for dynamic parameters (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float UpdateFrequency = 1.0f;

    /** Enable real-time parameter updates */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    bool bEnableRealTimeUpdates = true;

    /** Distance-based LOD for material complexity */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    TArray<float> MaterialLODDistances = {1000.0f, 2500.0f, 5000.0f};
};

/**
 * @brief Material System Manager
 * 
 * Handles dynamic material blending, weather responses, and performance optimization
 * for all environment materials in the prehistoric world.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UMaterialSystemManager : public UObject
{
    GENERATED_BODY()

public:
    UMaterialSystemManager();

    /** Initialize the material system */
    UFUNCTION(BlueprintCallable, Category = "Material System")
    void InitializeMaterialSystem();

    /** Update dynamic material parameters */
    UFUNCTION(BlueprintCallable, Category = "Material System")
    void UpdateDynamicParameters(const FDynamicMaterialParams& NewParameters);

    /** Set weather condition */
    UFUNCTION(BlueprintCallable, Category = "Material System")
    void SetWeatherCondition(EWeatherCondition WeatherCondition, float Intensity = 1.0f);

    /** Apply material configuration to landscape */
    UFUNCTION(BlueprintCallable, Category = "Material System")
    bool ApplyMaterialConfigToLandscape(class ALandscape* Landscape, UMaterialConfigData* ConfigData);

    /** Create dynamic material instance */
    UFUNCTION(BlueprintCallable, Category = "Material System")
    class UMaterialInstanceDynamic* CreateDynamicMaterialInstance(UMaterialInterface* BaseMaterial);

    /** Update material parameter collection */
    UFUNCTION(BlueprintCallable, Category = "Material System")
    void UpdateParameterCollection(UMaterialParameterCollection* Collection, const FDynamicMaterialParams& Parameters);

    /** Get current dynamic parameters */
    UFUNCTION(BlueprintPure, Category = "Material System")
    FDynamicMaterialParams GetCurrentDynamicParameters() const { return CurrentDynamicParameters; }

    /** Get current weather condition */
    UFUNCTION(BlueprintPure, Category = "Material System")
    EWeatherCondition GetCurrentWeatherCondition() const { return CurrentWeatherCondition; }

protected:
    /** Current dynamic parameters */
    UPROPERTY(BlueprintReadOnly, Category = "State")
    FDynamicMaterialParams CurrentDynamicParameters;

    /** Current weather condition */
    UPROPERTY(BlueprintReadOnly, Category = "State")
    EWeatherCondition CurrentWeatherCondition = EWeatherCondition::Clear;

    /** Current weather intensity */
    UPROPERTY(BlueprintReadOnly, Category = "State")
    float CurrentWeatherIntensity = 0.0f;

    /** Active material configurations */
    UPROPERTY(BlueprintReadOnly, Category = "State")
    TArray<UMaterialConfigData*> ActiveConfigurations;

    /** Dynamic material instances cache */
    UPROPERTY()
    TMap<UMaterialInterface*, UMaterialInstanceDynamic*> DynamicMaterialCache;

    /** Update timer handle */
    FTimerHandle UpdateTimerHandle;

    /** Internal update function */
    void InternalUpdateMaterials();

    /** Blend weather parameters */
    FDynamicMaterialParams BlendWeatherParameters(const FDynamicMaterialParams& Base, const FDynamicMaterialParams& Weather, float Intensity);
};