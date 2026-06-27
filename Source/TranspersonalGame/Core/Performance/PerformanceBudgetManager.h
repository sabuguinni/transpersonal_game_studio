#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PerformanceBudgetManager.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// Enums & Structs — must be at global scope (UHT rule)
// ─────────────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EPerf_QualityTier : uint8
{
    Low      UMETA(DisplayName = "Low (30fps Console)"),
    Medium   UMETA(DisplayName = "Medium (45fps)"),
    High     UMETA(DisplayName = "High (60fps PC)"),
    Ultra    UMETA(DisplayName = "Ultra (120fps)"),
};

UENUM(BlueprintType)
enum class EPerf_BudgetStatus : uint8
{
    OK          UMETA(DisplayName = "Within Budget"),
    Warning     UMETA(DisplayName = "Near Limit"),
    OverBudget  UMETA(DisplayName = "Over Budget"),
};

USTRUCT(BlueprintType)
struct FPerf_FrameBudget
{
    GENERATED_BODY()

    /** Target frame time in milliseconds (16.67ms = 60fps, 33.33ms = 30fps) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameTimeMs = 16.67f;

    /** Maximum dynamic lights allowed in scene */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxDynamicLights = 4;

    /** Maximum draw calls per frame */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxDrawCalls = 2000;

    /** Maximum triangles per frame (millions) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxTrianglesMillion = 3.0f;

    /** Maximum shadow-casting lights */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxShadowCasters = 2;

    /** Minimum LOD screen size threshold */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MinLODScreenSize = 0.0002f;
};

USTRUCT(BlueprintType)
struct FPerf_RuntimeStats
{
    GENERATED_BODY()

    /** Current frame time in ms (smoothed over 30 frames) */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float SmoothedFrameTimeMs = 0.0f;

    /** Current dynamic light count in scene */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DynamicLightCount = 0;

    /** Current visible actor count */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 VisibleActorCount = 0;

    /** Overall budget status */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    EPerf_BudgetStatus BudgetStatus = EPerf_BudgetStatus::OK;

    /** Frames since last budget violation */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 FramesSinceViolation = 0;
};

// ─────────────────────────────────────────────────────────────────────────────
// APerf_BudgetManager — world actor that enforces frame budgets at runtime
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Performance Budget Manager"))
class TRANSPERSONALGAME_API APerf_BudgetManager : public AActor
{
    GENERATED_BODY()

public:
    APerf_BudgetManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ── Budget Configuration ──

    /** Frame budget for current quality tier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    FPerf_FrameBudget FrameBudget;

    /** Active quality tier — drives budget thresholds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    EPerf_QualityTier QualityTier = EPerf_QualityTier::High;

    /** How often (seconds) to run the full budget audit */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float AuditIntervalSeconds = 1.0f;

    // ── Runtime Stats (read-only) ──

    UPROPERTY(BlueprintReadOnly, Category = "Performance|Stats")
    FPerf_RuntimeStats RuntimeStats;

    // ── Public API ──

    /** Set quality tier and update budget thresholds accordingly */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetQualityTier(EPerf_QualityTier NewTier);

    /** Force an immediate budget audit — returns current status */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerf_BudgetStatus RunBudgetAudit();

    /** Returns true if scene is within all budget limits */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance")
    bool IsWithinBudget() const;

    /** Get recommended LOD bias for current performance state */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance")
    float GetRecommendedLODBias() const;

    /** Apply console variable overrides for current quality tier */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void ApplyQualityConsoleVars();

    /** Log current performance stats to output log */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void LogPerformanceStats() const;

private:
    // ── Internal ──

    float TimeSinceLastAudit = 0.0f;

    /** Smoothed frame time accumulator (ring buffer of 30 samples) */
    TArray<float> FrameTimeSamples;
    int32 FrameSampleIndex = 0;
    static constexpr int32 FrameSampleCount = 30;

    void UpdateFrameTimeSamples(float DeltaTime);
    void UpdateDynamicLightCount();
    void ApplyBudgetThresholdsForTier(EPerf_QualityTier Tier, FPerf_FrameBudget& OutBudget) const;
    void ExecuteConsoleCommand(const FString& Command) const;
};
