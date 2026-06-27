#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PerformanceOptimizerComponent.generated.h"

// LOD tier for dynamic quality scaling
UENUM(BlueprintType)
enum class EPerf_QualityTier : uint8
{
    Ultra    UMETA(DisplayName = "Ultra (60fps PC)"),
    High     UMETA(DisplayName = "High (60fps PC)"),
    Medium   UMETA(DisplayName = "Medium (45fps PC / 60fps Console)"),
    Low      UMETA(DisplayName = "Low (30fps Console)"),
    Minimal  UMETA(DisplayName = "Minimal (Emergency)")
};

// Per-frame budget snapshot
USTRUCT(BlueprintType)
struct FPerf_FrameBudget
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float TargetFrameTimeMs = 16.67f;   // 60fps default

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float LastFrameTimeMs = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GameThreadMs = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderThreadMs = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUMs = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bOverBudget = false;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ConsecutiveOverBudgetFrames = 0;
};

/**
 * UPerf_PerformanceOptimizerComponent
 * Attaches to GameState or a dedicated manager actor.
 * Monitors frame time and dynamically scales quality to maintain target FPS.
 * Targets: 60fps on high-end PC, 30fps on console.
 */
UCLASS(ClassGroup = "TranspersonalGame|Performance", meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_PerformanceOptimizerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_PerformanceOptimizerComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // --- Configuration ---

    /** Target frames per second. 60 for PC, 30 for console. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Config")
    float TargetFPS = 60.0f;

    /** How many consecutive over-budget frames before downscaling quality. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Config")
    int32 DownscaleThresholdFrames = 10;

    /** How many consecutive under-budget frames before upscaling quality. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Config")
    int32 UpscaleThresholdFrames = 120;

    /** Enable dynamic resolution scaling (r.ScreenPercentage). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Config")
    bool bDynamicResolutionEnabled = true;

    /** Minimum screen percentage allowed (50 = half resolution). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Config")
    float MinScreenPercentage = 60.0f;

    /** Maximum screen percentage (100 = native). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Config")
    float MaxScreenPercentage = 100.0f;

    // --- Runtime State ---

    UPROPERTY(BlueprintReadOnly, Category = "Performance|State")
    FPerf_FrameBudget CurrentBudget;

    UPROPERTY(BlueprintReadOnly, Category = "Performance|State")
    EPerf_QualityTier CurrentTier = EPerf_QualityTier::High;

    UPROPERTY(BlueprintReadOnly, Category = "Performance|State")
    float CurrentScreenPercentage = 100.0f;

    // --- Blueprint Events ---

    UFUNCTION(BlueprintImplementableEvent, Category = "Performance")
    void OnQualityTierChanged(EPerf_QualityTier NewTier, EPerf_QualityTier OldTier);

    UFUNCTION(BlueprintImplementableEvent, Category = "Performance")
    void OnFrameBudgetExceeded(float OverrunMs);

    // --- Public API ---

    /** Force a specific quality tier immediately. */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetQualityTier(EPerf_QualityTier NewTier);

    /** Get current frame budget snapshot. */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_FrameBudget GetFrameBudget() const { return CurrentBudget; }

    /** Returns true if currently over frame budget. */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsOverBudget() const { return CurrentBudget.bOverBudget; }

    /** Log current performance stats to output log. */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void LogPerformanceStats() const;

private:
    // Rolling average frame time (last 30 frames)
    TArray<float> FrameTimeHistory;
    static constexpr int32 FrameHistorySize = 30;
    int32 FrameHistoryIndex = 0;
    int32 UnderBudgetFrameCount = 0;

    // Tick interval for quality checks (every 0.5s)
    float QualityCheckAccumulator = 0.0f;
    static constexpr float QualityCheckInterval = 0.5f;

    void UpdateFrameBudget(float DeltaTime);
    void EvaluateQualityScaling();
    void ApplyQualityTier(EPerf_QualityTier Tier);
    void ApplyScreenPercentage(float Percentage);
    float GetAverageFrameTimeMs() const;

    // Console command helpers
    void ExecConsoleCommand(const FString& Command) const;
};
