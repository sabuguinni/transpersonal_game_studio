#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PerformanceConfig.generated.h"

// ── Perf_QualityTier ─────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class EPerf_QualityTier : uint8
{
    Low        UMETA(DisplayName = "Low (Console Min)"),
    Medium     UMETA(DisplayName = "Medium (Console Target)"),
    High       UMETA(DisplayName = "High (PC Min)"),
    Epic       UMETA(DisplayName = "Epic (PC Target)"),
    Cinematic  UMETA(DisplayName = "Cinematic (Cutscenes)")
};

// ── Perf_FrameBudget ─────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_FrameBudget
{
    GENERATED_BODY()

    /** Target frame time in milliseconds (16.67ms = 60fps, 33.33ms = 30fps) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameTimeMs = 16.67f;

    /** GPU budget in ms (typically 70% of total frame time) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float GPUBudgetMs = 11.67f;

    /** CPU game thread budget in ms */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CPUGameThreadMs = 8.0f;

    /** CPU render thread budget in ms */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CPURenderThreadMs = 10.0f;

    /** Max draw calls per frame */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxDrawCalls = 2000;

    /** Max triangles per frame (millions) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxTrianglesMillion = 8.0f;
};

// ── Perf_LODConfig ────────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_LODConfig
{
    GENERATED_BODY()

    /** Distance at which LOD0 transitions to LOD1 (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD0Distance = 1500.0f;

    /** Distance at which LOD1 transitions to LOD2 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD1Distance = 3000.0f;

    /** Distance at which LOD2 transitions to LOD3 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD2Distance = 6000.0f;

    /** Cull distance — actors beyond this are not rendered */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float CullDistance = 15000.0f;

    /** LOD bias offset (positive = use lower quality LODs sooner) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    int32 LODBias = 0;

    /** Whether to use screen-size based LOD selection */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    bool bUseScreenSizeLOD = true;
};

// ── Perf_DinosaurBudget ───────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_DinosaurBudget
{
    GENERATED_BODY()

    /** Max simultaneous large dinosaurs (T-Rex class, >scale 2.5) visible */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DinosaurBudget")
    int32 MaxLargeDinosVisible = 4;

    /** Max simultaneous medium dinosaurs (Raptor class, scale 1.5-2.5) visible */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DinosaurBudget")
    int32 MaxMediumDinosVisible = 8;

    /** Max simultaneous small dinosaurs (scale <1.5) visible */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DinosaurBudget")
    int32 MaxSmallDinosVisible = 16;

    /** Distance at which dinosaur animation tick rate drops from full to half */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DinosaurBudget")
    float AnimTickHalfRateDistance = 4000.0f;

    /** Distance at which dinosaur animation is paused entirely */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DinosaurBudget")
    float AnimTickPauseDistance = 8000.0f;
};

/**
 * UPerf_PerformanceConfig
 * Data asset holding all performance configuration for the prehistoric survival game.
 * Targets: 60fps on high-end PC, 30fps on console.
 * Agent #4 — Performance Optimizer
 */
UCLASS(BlueprintType, Blueprintable, DefaultToInstanced, EditInlineNew)
class TRANSPERSONALGAME_API UPerf_PerformanceConfig : public UObject
{
    GENERATED_BODY()

public:
    UPerf_PerformanceConfig();

    // ── Platform Budgets ──────────────────────────────────────────────────────

    /** Frame budget for high-end PC (target: 60fps) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budgets|PC")
    FPerf_FrameBudget PCHighEndBudget;

    /** Frame budget for console (target: 30fps) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budgets|Console")
    FPerf_FrameBudget ConsoleBudget;

    // ── LOD Configuration ─────────────────────────────────────────────────────

    /** LOD config for large dinosaurs (T-Rex, Brachiosaurus) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD|Dinosaurs")
    FPerf_LODConfig LargeDinoLOD;

    /** LOD config for medium dinosaurs (Raptor, Triceratops) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD|Dinosaurs")
    FPerf_LODConfig MediumDinoLOD;

    /** LOD config for foliage (trees, ferns, ground cover) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD|Foliage")
    FPerf_LODConfig FoliageLOD;

    // ── Dinosaur Budget ───────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DinosaurBudget")
    FPerf_DinosaurBudget DinosaurBudget;

    // ── Quality Tier ──────────────────────────────────────────────────────────

    /** Current active quality tier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
    EPerf_QualityTier ActiveQualityTier = EPerf_QualityTier::Epic;

    // ── Blueprint Callable Helpers ────────────────────────────────────────────

    /** Returns the frame budget for the current quality tier */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_FrameBudget GetActiveBudget() const;

    /** Returns true if the given actor count is within budget for the given tier */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsWithinDinosaurBudget(int32 LargeCount, int32 MediumCount, int32 SmallCount) const;

    /** Apply scalability console commands for the given quality tier */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void ApplyQualityTierToConsole();

    /** Get recommended LOD config for a dinosaur scale */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_LODConfig GetLODConfigForScale(float DinoScale) const;
};
