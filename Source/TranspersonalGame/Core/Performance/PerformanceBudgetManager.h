#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "PerformanceBudgetManager.generated.h"

/**
 * FPerf_BudgetSnapshot — per-frame performance snapshot
 * Prefix: Perf_ (Agent #04 namespace — prevents ODR collision)
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_BudgetSnapshot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTimeMs = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GameThreadMs = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderThreadMs = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUTimeMs = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCallCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TriangleCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float TextureMemoryMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bIsOverBudget = false;
};

/**
 * EPerf_QualityTier — scalability quality tiers
 * Prefix: Perf_ (Agent #04 namespace)
 */
UENUM(BlueprintType)
enum class EPerf_QualityTier : uint8
{
    Low        UMETA(DisplayName = "Low (Console 30fps)"),
    Medium     UMETA(DisplayName = "Medium (Console 60fps)"),
    High       UMETA(DisplayName = "High (PC 60fps)"),
    Ultra      UMETA(DisplayName = "Ultra (PC 60fps+ High-End)"),
    Cinematic  UMETA(DisplayName = "Cinematic (Offline/Screenshot)")
};

/**
 * UPerformanceBudgetManager — World Subsystem
 * 
 * Monitors frame budget and dynamically adjusts quality settings
 * to maintain target FPS (60 PC / 30 console).
 * 
 * Frame budgets:
 *   60fps target = 16.67ms total
 *   30fps target = 33.33ms total
 *   Game thread budget: 8ms (60fps) / 16ms (30fps)
 *   Render thread budget: 8ms (60fps) / 16ms (30fps)
 *   GPU budget: 14ms (60fps) / 28ms (30fps)
 */
UCLASS()
class TRANSPERSONALGAME_API UPerformanceBudgetManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // UWorldSubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

    // --- Frame Budget Queries ---

    /** Returns the most recent performance snapshot */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_BudgetSnapshot GetCurrentSnapshot() const;

    /** Returns true if the last frame exceeded the target frame time */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsOverBudget() const;

    /** Returns current FPS (smoothed over 30 frames) */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetSmoothedFPS() const;

    /** Returns target frame time in ms (16.67 for 60fps, 33.33 for 30fps) */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetTargetFrameTimeMs() const;

    // --- Quality Tier Management ---

    /** Set the active quality tier — applies console commands immediately */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetQualityTier(EPerf_QualityTier NewTier);

    /** Returns the currently active quality tier */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerf_QualityTier GetCurrentQualityTier() const { return CurrentQualityTier; }

    /** Enable/disable dynamic quality scaling (auto-adjusts tier based on FPS) */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetDynamicQualityEnabled(bool bEnabled);

    // --- Budget Configuration ---

    /** Set target FPS (default: 60 on PC, 30 on console) */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetTargetFPS(int32 InTargetFPS);

    /** Returns target FPS */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetTargetFPS() const { return TargetFPS; }

    // --- Diagnostics ---

    /** Log full performance report to output log */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void LogPerformanceReport() const;

    /** Apply all performance console commands for current tier */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void ApplyQualityTierCommands();

private:
    // Current state
    FPerf_BudgetSnapshot CurrentSnapshot;
    EPerf_QualityTier CurrentQualityTier = EPerf_QualityTier::High;
    int32 TargetFPS = 60;
    bool bDynamicQualityEnabled = true;

    // FPS smoothing ring buffer
    static constexpr int32 FPS_SAMPLE_COUNT = 30;
    TArray<float> FPSSamples;
    int32 FPSSampleIndex = 0;

    // Timer handle for periodic budget checks
    FTimerHandle BudgetCheckTimer;

    // Internal methods
    void TickBudgetCheck();
    void UpdateFPSSamples(float DeltaTime);
    void AutoAdjustQuality();
    void ApplyLowQualitySettings();
    void ApplyMediumQualitySettings();
    void ApplyHighQualitySettings();
    void ApplyUltraQualitySettings();
    void ExecuteConsoleCommand(const FString& Command) const;
};
