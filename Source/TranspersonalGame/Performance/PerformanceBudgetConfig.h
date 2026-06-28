#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PerformanceBudgetConfig.generated.h"

/**
 * Performance budget configuration for Transpersonal Game Studio.
 * Targets: 60fps on high-end PC (RTX 3070+, i7-10th gen+, 16GB RAM)
 *          30fps on console baseline (PS5/XSX equivalent)
 *
 * Agent #4 — Performance Optimizer
 */

UENUM(BlueprintType)
enum class EPerf_QualityTier : uint8
{
    Low         UMETA(DisplayName = "Low (30fps Console)"),
    Medium      UMETA(DisplayName = "Medium (45fps Mid PC)"),
    High        UMETA(DisplayName = "High (60fps PC)"),
    Ultra       UMETA(DisplayName = "Ultra (60fps+ High-End PC)")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_FrameBudget
{
    GENERATED_BODY()

    /** Target frames per second */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float TargetFPS = 60.0f;

    /** Max CPU game thread time in milliseconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float MaxGameThreadMs = 6.0f;

    /** Max CPU render thread time in milliseconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float MaxRenderThreadMs = 5.0f;

    /** Max GPU time in milliseconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float MaxGPUMs = 10.0f;

    /** Max draw calls per frame */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    int32 MaxDrawCalls = 2000;

    /** Max triangle count per frame (millions) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float MaxTrianglesM = 8.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_ShadowBudget
{
    GENERATED_BODY()

    /** Max CSM cascade count */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Shadows")
    int32 MaxCSMCascades = 3;

    /** Max shadow map resolution */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Shadows")
    int32 MaxShadowResolution = 2048;

    /** Shadow distance scale */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Shadows")
    float ShadowDistanceScale = 1.0f;

    /** Radius threshold for shadow casting (smaller = fewer shadow casters) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Shadows")
    float RadiusThreshold = 0.03f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_LumenBudget
{
    GENERATED_BODY()

    /** Enable Lumen Global Illumination */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Lumen")
    bool bEnableLumenGI = true;

    /** Enable Lumen Reflections */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Lumen")
    bool bEnableLumenReflections = true;

    /** Max GI trace distance in cm */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Lumen")
    float MaxGITraceDistance = 8000.0f;

    /** Max roughness for reflection traces (lower = fewer traces) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Lumen")
    float MaxReflectionRoughness = 0.4f;

    /** Enable FastSkyLUT (significant GPU savings) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Lumen")
    bool bFastSkyLUT = true;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_ParticleBudget
{
    GENERATED_BODY()

    /** Max CPU particles per emitter */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Particles")
    int32 MaxCPUParticlesPerEmitter = 500;

    /** Max GPU particles spawned per frame */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Particles")
    int32 MaxGPUParticlesPerFrame = 2000;

    /** Niagara system LOD bias */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Particles")
    float NiagaraLODBias = 0.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_StreamingBudget
{
    GENERATED_BODY()

    /** Texture streaming pool size in MB */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Streaming")
    int32 TexturePoolSizeMB = 2048;

    /** Max temp memory for streaming in MB */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Streaming")
    int32 MaxTempMemoryMB = 50;

    /** View distance scale (1.0 = default, lower = less streaming) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Streaming")
    float ViewDistanceScale = 1.0f;

    /** Static mesh LOD distance scale */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Streaming")
    float StaticMeshLODDistanceScale = 1.0f;

    /** Foliage LOD distance scale */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Streaming")
    float FoliageLODDistanceScale = 1.5f;
};

/**
 * UPerf_BudgetConfig — Runtime performance budget manager.
 * Stores and applies per-quality-tier performance settings.
 * Callable from Blueprint to switch quality tiers at runtime.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = "Performance")
class TRANSPERSONALGAME_API UPerf_BudgetConfig : public UObject
{
    GENERATED_BODY()

public:
    UPerf_BudgetConfig();

    /** Current quality tier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerf_QualityTier QualityTier = EPerf_QualityTier::High;

    /** Frame budget for current tier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    FPerf_FrameBudget FrameBudget;

    /** Shadow budget for current tier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Shadows")
    FPerf_ShadowBudget ShadowBudget;

    /** Lumen budget for current tier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Lumen")
    FPerf_LumenBudget LumenBudget;

    /** Particle budget for current tier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Particles")
    FPerf_ParticleBudget ParticleBudget;

    /** Streaming budget for current tier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Streaming")
    FPerf_StreamingBudget StreamingBudget;

    /**
     * Apply all budget settings as console variable commands.
     * Call this after changing QualityTier to push settings to the renderer.
     */
    UFUNCTION(BlueprintCallable, Category = "Performance", CallInEditor)
    void ApplyBudgetSettings();

    /**
     * Switch to a new quality tier and immediately apply settings.
     * @param NewTier — Target quality tier
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetQualityTier(EPerf_QualityTier NewTier);

    /**
     * Get the frame budget for a specific quality tier (does not apply).
     * @param Tier — Quality tier to query
     * @param OutBudget — Output frame budget struct
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void GetFrameBudgetForTier(EPerf_QualityTier Tier, FPerf_FrameBudget& OutBudget) const;

    /**
     * Check if current frame time is within budget.
     * @param GameThreadMs — Measured game thread time
     * @param RenderThreadMs — Measured render thread time
     * @param GPUMs — Measured GPU time
     * @return True if all times are within budget
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance")
    bool IsWithinBudget(float GameThreadMs, float RenderThreadMs, float GPUMs) const;

private:
    /** Configure budget structs for the given tier */
    void ConfigureTierBudgets(EPerf_QualityTier Tier);
};
