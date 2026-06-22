#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PerformanceOptimizer.generated.h"

// ============================================================
// TranspersonalGame — PerformanceOptimizer
// Agent #04 Performance Optimizer — PROD_CYCLE_AUTO_20260622_010
//
// GameInstance subsystem that monitors frame time and applies
// dynamic quality adjustments to maintain 60fps PC / 30fps console.
// Throttles SurvivalComponent tick, manages LOD bias, controls
// shadow cascade count based on real-time frame budget.
// ============================================================

UENUM(BlueprintType)
enum class EPerf_QualityTier : uint8
{
    Epic    UMETA(DisplayName = "Epic (60fps PC)"),
    High    UMETA(DisplayName = "High (50fps PC)"),
    Medium  UMETA(DisplayName = "Medium (30fps Console)"),
    Low     UMETA(DisplayName = "Low (Emergency fallback)")
};

USTRUCT(BlueprintType)
struct FPerf_FrameBudget
{
    GENERATED_BODY()

    /** Target frame time in milliseconds (16.6 = 60fps, 33.3 = 30fps) */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float TargetFrameTimeMs = 16.6f;

    /** Current measured frame time in milliseconds */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFrameTimeMs = 0.0f;

    /** Dynamic resolution screen percentage (70-100) */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float ScreenPercentage = 100.0f;

    /** Number of active shadow cascades (1-4) */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ShadowCascades = 4;

    /** Current quality tier */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    EPerf_QualityTier QualityTier = EPerf_QualityTier::Epic;
};

UCLASS(BlueprintType, meta = (DisplayName = "Performance Optimizer"))
class TRANSPERSONALGAME_API UPerformanceOptimizer : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ---- Public API ----

    /** Returns current frame budget snapshot */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_FrameBudget GetFrameBudget() const;

    /** Force a specific quality tier (for testing / platform detection) */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetQualityTier(EPerf_QualityTier Tier);

    /** Called by SurvivalComponent to get its recommended tick interval */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetSurvivalTickInterval() const;

    /** Called by DinosaurAI to get recommended perception update rate */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetAIUpdateInterval() const;

    /** Returns true if Lumen GI is active (vs baked fallback) */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsLumenActive() const;

    /** Apply all performance CVars for current quality tier */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void ApplyQualitySettings();

    /** Run a one-frame performance snapshot and log results */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void LogPerformanceSnapshot();

private:
    /** Current frame budget state */
    FPerf_FrameBudget CurrentBudget;

    /** Accumulated frame time samples for rolling average */
    TArray<float> FrameTimeSamples;

    /** Max samples for rolling average (default 60 = 1 second at 60fps) */
    static constexpr int32 MaxFrameSamples = 60;

    /** Survival component tick interval per quality tier */
    static constexpr float SurvivalTickIntervals[] = {0.1f, 0.15f, 0.25f, 0.5f};

    /** AI update interval per quality tier */
    static constexpr float AIUpdateIntervals[] = {0.05f, 0.1f, 0.2f, 0.4f};

    /** Apply CVars for Epic tier */
    void ApplyEpicSettings();

    /** Apply CVars for High tier */
    void ApplyHighSettings();

    /** Apply CVars for Medium tier (console) */
    void ApplyMediumSettings();

    /** Apply CVars for Low tier (emergency) */
    void ApplyLowSettings();

    /** Execute a console variable command */
    void SetCVar(const FString& CVarName, float Value);
    void SetCVar(const FString& CVarName, int32 Value);
};
