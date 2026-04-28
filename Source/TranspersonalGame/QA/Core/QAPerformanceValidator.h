#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"
#include "RenderingThread.h"
#include "QAPerformanceValidator.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogQAPerformance, Log, All);

/**
 * Performance validation thresholds for different platforms
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_PerformanceThresholds
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MinFPS_PC = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MinFPS_Console = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxMemoryMB_PC = 8192;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxMemoryMB_Console = 4096;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxFrameTimeMS = 16.67f; // 60fps target

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxDrawCalls = 2000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxTriangles = 2000000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxGPUTimeMS = 16.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxVisibleStaticMeshElements = 5000;
};

/**
 * Performance measurement data structure
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float CurrentFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float FrameTimeMS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float GameThreadTimeMS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float RenderThreadTimeMS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float GPUTimeMS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 MemoryUsageMB = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 DrawCalls = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 Triangles = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 VisibleStaticMeshElements = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    FDateTime Timestamp;
};

/**
 * QA Performance Validator
 * 
 * Provides comprehensive performance monitoring and validation
 * for the Transpersonal Game project. Ensures 60fps on PC and
 * 30fps on console platforms while monitoring memory usage,
 * draw calls, and rendering performance.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQAPerformanceValidator : public UObject
{
    GENERATED_BODY()

public:
    UQAPerformanceValidator();

    /**
     * Initialize the performance validator
     */
    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    void Initialize();

    /**
     * Start performance monitoring session
     */
    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    void StartMonitoring(float Duration = 60.0f);

    /**
     * Stop performance monitoring session
     */
    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    void StopMonitoring();

    /**
     * Get current performance metrics
     */
    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    FQA_PerformanceMetrics GetCurrentMetrics();

    /**
     * Validate current performance against thresholds
     */
    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    bool ValidatePerformance(const FQA_PerformanceThresholds& Thresholds, FString& OutFailureReason);

    /**
     * Generate performance report
     */
    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    FString GeneratePerformanceReport();

    /**
     * Check if monitoring is active
     */
    UFUNCTION(BlueprintPure, Category = "QA Performance")
    bool IsMonitoring() const { return bIsMonitoring; }

    /**
     * Get performance thresholds for current platform
     */
    UFUNCTION(BlueprintPure, Category = "QA Performance")
    FQA_PerformanceThresholds GetPlatformThresholds() const;

    /**
     * Validate frame rate stability
     */
    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    bool ValidateFrameRateStability(float MinFPS, float TestDuration, float AllowedVariance = 5.0f);

    /**
     * Validate memory usage
     */
    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    bool ValidateMemoryUsage(int32 MaxMemoryMB);

    /**
     * Validate rendering performance
     */
    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    bool ValidateRenderingPerformance();

    /**
     * Export performance data to CSV
     */
    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    bool ExportPerformanceData(const FString& FilePath);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FQA_PerformanceThresholds DefaultThresholds;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsMonitoring = false;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float MonitoringDuration = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float ElapsedMonitoringTime = 0.0f;

    // Performance data storage
    TArray<FQA_PerformanceMetrics> PerformanceHistory;
    
    // Timing
    FDateTime MonitoringStartTime;
    FTimerHandle MonitoringTimerHandle;

private:
    /**
     * Update performance metrics
     */
    void UpdateMetrics();

    /**
     * Collect current performance data
     */
    FQA_PerformanceMetrics CollectMetrics();

    /**
     * Get current platform type
     */
    FString GetCurrentPlatform() const;

    /**
     * Calculate average metrics over time period
     */
    FQA_PerformanceMetrics CalculateAverageMetrics(float TimePeriod) const;

    /**
     * Check for performance hitches
     */
    bool DetectPerformanceHitches(float HitchThresholdMS = 33.33f) const;
};