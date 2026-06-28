#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PerformanceConfig.generated.h"

/**
 * Performance tier enumeration for platform-specific quality scaling.
 * Prefix: Perf_ (Agent #4 namespace)
 */
UENUM(BlueprintType)
enum class EPerf_QualityTier : uint8
{
    Console_Low     UMETA(DisplayName = "Console Low (PS4/XBO 30fps)"),
    Console_High    UMETA(DisplayName = "Console High (PS5/XSX 30fps)"),
    PC_Medium       UMETA(DisplayName = "PC Medium (RTX 2070 60fps)"),
    PC_High         UMETA(DisplayName = "PC High (RTX 3080+ 60fps)"),
    PC_Ultra        UMETA(DisplayName = "PC Ultra (RTX 4090 60fps+)")
};

/**
 * Per-system performance budget struct.
 * Defines frame time allocation in milliseconds per subsystem.
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_FrameBudget
{
    GENERATED_BODY()

    /** Total frame budget in ms (16.67ms = 60fps, 33.33ms = 30fps) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float TotalBudgetMs = 16.67f;

    /** CPU game thread budget (ms) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float GameThreadMs = 4.0f;

    /** CPU render thread budget (ms) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float RenderThreadMs = 6.0f;

    /** GPU budget (ms) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float GPUMs = 10.0f;

    /** Dinosaur AI tick budget (ms) — shared across all active dinos */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float DinoAIMs = 2.0f;

    /** Physics simulation budget (ms) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float PhysicsMs = 2.0f;

    /** Particle/VFX budget (ms) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float VFXMs = 1.5f;

    /** Audio budget (ms) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float AudioMs = 0.5f;
};

/**
 * Culling and LOD configuration per quality tier.
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_CullingConfig
{
    GENERATED_BODY()

    /** Max draw distance for dinosaur actors (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Culling")
    float DinoMaxDrawDistance = 15000.0f;

    /** Max draw distance for foliage (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Culling")
    float FoliageMaxDrawDistance = 8000.0f;

    /** Max draw distance for props/rocks (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Culling")
    float PropMaxDrawDistance = 5000.0f;

    /** Max active dinosaur AI agents simultaneously */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Culling")
    int32 MaxActiveDinoAI = 12;

    /** Max simultaneous particle systems */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Culling")
    int32 MaxParticleSystems = 32;

    /** Shadow cascade count (1-4) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Shadows")
    int32 ShadowCascades = 3;

    /** Shadow map resolution (512/1024/2048) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Shadows")
    int32 ShadowMapResolution = 1024;

    /** Shadow draw distance scale (0.5-1.0) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Shadows")
    float ShadowDistanceScale = 0.8f;
};

/**
 * UPerf_PerformanceConfig — Runtime performance configuration manager.
 * Manages quality tier switching, frame budget enforcement, and scalability settings.
 * Owned by GameInstance, accessible globally.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = "TranspersonalGame|Performance")
class TRANSPERSONALGAME_API UPerf_PerformanceConfig : public UObject
{
    GENERATED_BODY()

public:
    UPerf_PerformanceConfig();

    // =========================================================
    // QUALITY TIER
    // =========================================================

    /** Current active quality tier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerf_QualityTier CurrentTier = EPerf_QualityTier::PC_High;

    /** Apply quality tier — sets all scalability CVars */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyQualityTier(EPerf_QualityTier Tier);

    /** Auto-detect best tier based on GPU benchmark */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerf_QualityTier AutoDetectTier();

    // =========================================================
    // FRAME BUDGET
    // =========================================================

    /** Frame budget for current tier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    FPerf_FrameBudget FrameBudget;

    /** Culling config for current tier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Culling")
    FPerf_CullingConfig CullingConfig;

    /** Get frame budget for a specific tier */
    UFUNCTION(BlueprintPure, Category = "Performance|Budget")
    FPerf_FrameBudget GetBudgetForTier(EPerf_QualityTier Tier) const;

    /** Get culling config for a specific tier */
    UFUNCTION(BlueprintPure, Category = "Performance|Culling")
    FPerf_CullingConfig GetCullingConfigForTier(EPerf_QualityTier Tier) const;

    // =========================================================
    // DINOSAUR AI BUDGET
    // =========================================================

    /** Max simultaneous active dinosaur AI agents */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|DinoAI")
    int32 MaxActiveDinoAI = 12;

    /** Dino AI tick rate when player is far (>5000cm) — Hz */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|DinoAI")
    float DinoAITickRateFar = 2.0f;

    /** Dino AI tick rate when player is near (<2000cm) — Hz */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|DinoAI")
    float DinoAITickRateNear = 10.0f;

    /** Dino AI tick rate at medium range — Hz */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|DinoAI")
    float DinoAITickRateMedium = 5.0f;

    /** Distance threshold: near/medium boundary (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|DinoAI")
    float DinoAINearThreshold = 2000.0f;

    /** Distance threshold: medium/far boundary (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|DinoAI")
    float DinoAIFarThreshold = 5000.0f;

    // =========================================================
    // LUMEN CONFIG
    // =========================================================

    /** Enable Lumen GI (disable for low-end) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Lumen")
    bool bLumenGIEnabled = true;

    /** Enable Lumen Reflections */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Lumen")
    bool bLumenReflectionsEnabled = true;

    /** Lumen scene detail (0.25-4.0) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Lumen")
    float LumenSceneDetail = 1.0f;

    /** Apply Lumen settings via console commands */
    UFUNCTION(BlueprintCallable, Category = "Performance|Lumen")
    void ApplyLumenConfig(UObject* WorldContext);

    // =========================================================
    // NANITE CONFIG
    // =========================================================

    /** Nanite max pixels per edge (0.5 = performance, 1.0 = quality, 2.0 = ultra) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Nanite")
    float NaniteMaxPixelsPerEdge = 1.0f;

    /** Apply Nanite settings */
    UFUNCTION(BlueprintCallable, Category = "Performance|Nanite")
    void ApplyNaniteConfig(UObject* WorldContext);

    // =========================================================
    // SCALABILITY HELPERS
    // =========================================================

    /** Apply all performance settings for current tier */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void ApplyAllSettings(UObject* WorldContext);

    /** Get human-readable performance report string */
    UFUNCTION(BlueprintPure, Category = "Performance")
    FString GetPerformanceReport() const;

private:
    /** Internal: apply console commands for a given tier */
    void ApplyConsoleCmdsForTier(EPerf_QualityTier Tier, UObject* WorldContext);
};
