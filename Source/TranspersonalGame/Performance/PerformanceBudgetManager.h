// PerformanceBudgetManager.h
// Agent #04 — Performance Optimizer | PROD_CYCLE_AUTO_20260620_003
// Manages per-system frame budgets and dynamic quality scaling for 60fps PC / 30fps console.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PerformanceBudgetManager.generated.h"

// ─── Enums (global scope — UHT requirement) ───────────────────────────────────

UENUM(BlueprintType)
enum class EPerf_QualityTier : uint8
{
    Ultra    UMETA(DisplayName = "Ultra"),
    High     UMETA(DisplayName = "High"),
    Medium   UMETA(DisplayName = "Medium"),
    Low      UMETA(DisplayName = "Low"),
    Potato   UMETA(DisplayName = "Potato")
};

UENUM(BlueprintType)
enum class EPerf_BudgetSystem : uint8
{
    Rendering    UMETA(DisplayName = "Rendering"),
    Physics      UMETA(DisplayName = "Physics"),
    AI           UMETA(DisplayName = "AI"),
    Animation    UMETA(DisplayName = "Animation"),
    Audio        UMETA(DisplayName = "Audio"),
    Streaming    UMETA(DisplayName = "Streaming")
};

// ─── Structs ──────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FPerf_SystemBudget
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerf_BudgetSystem System = EPerf_BudgetSystem::Rendering;

    /** Budget in milliseconds per frame (16.67ms = 60fps target) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float BudgetMs = 8.0f;

    /** Last measured time for this system */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float LastMeasuredMs = 0.0f;

    /** True if this system exceeded its budget last frame */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bOverBudget = false;
};

USTRUCT(BlueprintType)
struct FPerf_QualitySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerf_QualityTier Tier = EPerf_QualityTier::High;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float ShadowDistanceScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxShadowResolution = 2048;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float ScreenPercentage = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bLumenGI = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bLumenReflections = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxDinoLOD = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveDinosaurs = 20;
};

// ─── Class ────────────────────────────────────────────────────────────────────

UCLASS(ClassGroup = (TranspersonalGame), meta = (DisplayName = "Performance Budget Manager"))
class TRANSPERSONALGAME_API APerformanceBudgetManager : public AActor
{
    GENERATED_BODY()

public:
    APerformanceBudgetManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // ── Configuration ────────────────────────────────────────────────────────

    /** Target FPS — 60 for PC, 30 for console */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Targets")
    float TargetFPS = 60.0f;

    /** Total frame budget in ms (1000 / TargetFPS) */
    UPROPERTY(BlueprintReadOnly, Category = "Performance|Targets")
    float TotalFrameBudgetMs = 16.67f;

    /** Current quality tier applied to the scene */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Quality")
    EPerf_QualityTier CurrentQualityTier = EPerf_QualityTier::High;

    /** Per-system budgets */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budgets")
    TArray<FPerf_SystemBudget> SystemBudgets;

    /** Quality presets for each tier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Quality")
    TArray<FPerf_QualitySettings> QualityPresets;

    /** Enable automatic dynamic quality scaling */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|DynamicScaling")
    bool bEnableDynamicScaling = true;

    /** FPS threshold below which quality is reduced */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|DynamicScaling")
    float ScaleDownThresholdFPS = 50.0f;

    /** FPS threshold above which quality can be increased */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|DynamicScaling")
    float ScaleUpThresholdFPS = 65.0f;

    /** Seconds to wait before scaling quality up (hysteresis) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|DynamicScaling")
    float ScaleUpHysteresisSeconds = 5.0f;

    // ── Runtime State ─────────────────────────────────────────────────────────

    /** Smoothed FPS over last 60 frames */
    UPROPERTY(BlueprintReadOnly, Category = "Performance|Runtime")
    float SmoothedFPS = 60.0f;

    /** Current frame time in ms */
    UPROPERTY(BlueprintReadOnly, Category = "Performance|Runtime")
    float CurrentFrameMs = 16.67f;

    /** Number of consecutive frames over budget */
    UPROPERTY(BlueprintReadOnly, Category = "Performance|Runtime")
    int32 OverBudgetFrameCount = 0;

    /** Seconds since last quality scale-up */
    UPROPERTY(BlueprintReadOnly, Category = "Performance|Runtime")
    float TimeSinceLastScaleUp = 0.0f;

    // ── Public API ────────────────────────────────────────────────────────────

    /** Apply a specific quality tier immediately */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyQualityTier(EPerf_QualityTier NewTier);

    /** Get current quality settings for a tier */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_QualitySettings GetQualitySettings(EPerf_QualityTier Tier) const;

    /** Force a performance audit — logs all system budgets */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void RunPerformanceAudit();

    /** Get budget usage ratio (0-1) for a specific system */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetBudgetUsageRatio(EPerf_BudgetSystem System) const;

    /** Register a system's measured frame time */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ReportSystemTime(EPerf_BudgetSystem System, float MeasuredMs);

private:
    void InitializeDefaultBudgets();
    void InitializeQualityPresets();
    void UpdateFPSSmoothing(float DeltaTime);
    void CheckDynamicScaling();
    void ApplyCVarsForTier(const FPerf_QualitySettings& Settings);

    TArray<float> FPSHistory;
    static constexpr int32 FPS_HISTORY_SIZE = 60;
};
