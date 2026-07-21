// PerformanceConfig.h
// Performance Optimizer Agent #4 — Transpersonal Game Studio
// Target: 60fps PC High / 30fps Console
// CYCLE: PROD_CYCLE_AUTO_20260630_005

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PerformanceConfig.generated.h"

// Performance tier enum — Perf_ prefix to avoid conflicts
UENUM(BlueprintType)
enum class EPerf_QualityTier : uint8
{
    Low        UMETA(DisplayName = "Low (Console 30fps)"),
    Medium     UMETA(DisplayName = "Medium (PC 60fps)"),
    High       UMETA(DisplayName = "High (PC 60fps Ultra)"),
    Ultra      UMETA(DisplayName = "Ultra (PC 120fps+)")
};

// Per-actor cull distance config
USTRUCT(BlueprintType)
struct FPerf_CullDistanceConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float VegetationCullDistance = 8000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float RockCullDistance = 12000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PropCullDistance = 6000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float DinosaurCullDistance = 0.0f; // Never cull — gameplay critical

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float NpcCullDistance = 10000.0f;
};

// Shadow quality config
USTRUCT(BlueprintType)
struct FPerf_ShadowConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Shadows")
    int32 MaxCascades = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Shadows")
    int32 MaxShadowResolution = 2048;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Shadows")
    float ShadowDistanceScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Shadows")
    bool bEnableContactShadows = true;
};

// Lumen config
USTRUCT(BlueprintType)
struct FPerf_LumenConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Lumen")
    bool bEnableDiffuseIndirect = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Lumen")
    bool bEnableReflections = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Lumen")
    bool bEnableFastSkyLUT = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Lumen")
    float LumenSceneDetail = 1.0f;
};

// Main performance configuration object
UCLASS(BlueprintType, Blueprintable, Config = Game)
class TRANSPERSONALGAME_API UPerf_PerformanceConfig : public UObject
{
    GENERATED_BODY()

public:
    UPerf_PerformanceConfig();

    // Current quality tier
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", Config)
    EPerf_QualityTier QualityTier = EPerf_QualityTier::High;

    // Draw call budget per frame (60fps target)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", Config)
    int32 DrawCallBudget = 2000;

    // Max actors in scene before streaming kicks in
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", Config)
    int32 MaxActorCount = 500;

    // Cull distance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Culling", Config)
    FPerf_CullDistanceConfig CullDistances;

    // Shadow settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Shadows", Config)
    FPerf_ShadowConfig ShadowConfig;

    // Lumen settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Lumen", Config)
    FPerf_LumenConfig LumenConfig;

    // LOD bias (0 = auto, positive = lower quality LODs)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD", Config)
    float StaticMeshLODDistanceScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD", Config)
    int32 SkeletalMeshLODBias = 0;

    // Foliage density scale (1.0 = full, 0.5 = half density)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Foliage", Config)
    float FoliageLODDistanceScale = 1.5f;

    // Apply settings to the current world via console commands
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void ApplyToWorld();

    // Get estimated FPS tier based on current scene complexity
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FString GetFPSTargetString() const;

    // Validate scene is within budget
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    bool ValidateSceneBudget(int32 ActorCount, int32 EstimatedDrawCalls) const;
};
