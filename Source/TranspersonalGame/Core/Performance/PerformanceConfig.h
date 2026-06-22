// PerformanceConfig.h
// Performance Optimizer — Agent #04
// Runtime performance configuration for 60fps PC / 30fps console targets.
// Encodes validated CVar defaults from PROD_CYCLE_AUTO_20260622_007.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PerformanceConfig.generated.h"

UENUM(BlueprintType)
enum class EPerf_QualityPreset : uint8
{
    Low        UMETA(DisplayName = "Low (30fps Console)"),
    Medium     UMETA(DisplayName = "Medium (60fps Mid-PC)"),
    High       UMETA(DisplayName = "High (60fps High-PC)"),
    Ultra      UMETA(DisplayName = "Ultra (Uncapped)"),
};

USTRUCT(BlueprintType)
struct FPerf_ShadowSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Shadows")
    int32 MaxResolution = 2048;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Shadows")
    float RadiusThreshold = 0.03f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Shadows")
    int32 ShadowQuality = 3;
};

USTRUCT(BlueprintType)
struct FPerf_LumenSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Lumen")
    bool bAllowGI = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Lumen")
    bool bAllowReflections = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Lumen")
    float MaxTraceDistance = 8000.0f;
};

USTRUCT(BlueprintType)
struct FPerf_FoliageSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Foliage")
    float LODDistanceScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Foliage")
    float CullDistanceScale = 1.0f;
};

USTRUCT(BlueprintType)
struct FPerf_VolumetricFogSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|VolumetricFog")
    bool bEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|VolumetricFog")
    int32 GridPixelSize = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|VolumetricFog")
    int32 GridSizeZ = 64;
};

/**
 * UPerf_PerformanceConfig
 * Runtime-queryable performance configuration object.
 * Stores validated CVar defaults and applies them on demand.
 * Designed for use as a GameInstance subsystem or standalone UObject.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_PerformanceConfig : public UObject
{
    GENERATED_BODY()

public:
    UPerf_PerformanceConfig();

    // Active quality preset
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerf_QualityPreset ActivePreset = EPerf_QualityPreset::High;

    // Shadow configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Shadows")
    FPerf_ShadowSettings ShadowSettings;

    // Lumen GI/Reflections configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Lumen")
    FPerf_LumenSettings LumenSettings;

    // Foliage LOD configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Foliage")
    FPerf_FoliageSettings FoliageSettings;

    // Volumetric fog configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|VolumetricFog")
    FPerf_VolumetricFogSettings VolumetricFogSettings;

    // Screen percentage (100 = native, 75 = DLSS Quality equivalent)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "50", ClampMax = "200"))
    int32 ScreenPercentage = 100;

    // Target frame rate (60 PC / 30 console)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 TargetFPS = 60;

    // Apply all settings to the engine via console commands
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplySettings();

    // Apply a named preset (Low/Medium/High/Ultra)
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyPreset(EPerf_QualityPreset Preset);

    // Get recommended preset for current hardware (stub — returns High by default)
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance")
    EPerf_QualityPreset GetRecommendedPreset() const;

    // Returns true if current settings are within 60fps budget
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance")
    bool IsWithin60fpsBudget() const;
};
