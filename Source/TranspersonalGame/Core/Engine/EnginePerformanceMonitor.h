#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "../../SharedTypes.h"
#include "EnginePerformanceMonitor.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogEnginePerformance, Log, All);

/**
 * Performance threshold settings
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PerformanceThresholds
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float TargetFrameTime; // In milliseconds

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float MaxGameThreadTime; // In milliseconds

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float MaxRenderThreadTime; // In milliseconds

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    int32 MaxDrawCalls;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    int32 MaxTriangles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float MaxMemoryUsageMB;

    FEng_PerformanceThresholds()
        : TargetFrameTime(16.67f) // 60 FPS
        , MaxGameThreadTime(12.0f)
        , MaxRenderThreadTime(12.0f)
        , MaxDrawCalls(2000)
        , MaxTriangles(1000000)
        , MaxMemoryUsageMB(4096.0f)
    {}
};

/**
 * Performance warning data
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PerformanceWarning
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Warning")
    FString WarningType;

    UPROPERTY(BlueprintReadOnly, Category = "Warning")
    float CurrentValue;

    UPROPERTY(BlueprintReadOnly, Category = "Warning")
    float ThresholdValue;

    UPROPERTY(BlueprintReadOnly, Category = "Warning")
    FString Description;

    UPROPERTY(BlueprintReadOnly, Category = "Warning")
    float Timestamp;

    FEng_PerformanceWarning()
        : CurrentValue(0.0f)
        , ThresholdValue(0.0f)
        , Timestamp(0.0f)
    {}
};

/**
 * Engine Performance Monitor - Real-time performance tracking and alerting
 * Monitors frame rates, memory usage, draw calls, and other critical metrics
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEnginePerformanceMonitor : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEnginePerformanceMonitor();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /**
     * Start performance monitoring
     */
    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    void StartMonitoring();

    /**
     * Stop performance monitoring
     */
    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    void StopMonitoring();

    /**
     * Check if monitoring is active
     */
    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    bool IsMonitoring() const { return bIsMonitoring; }

    /**
     * Get current FPS
     */
    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    float GetCurrentFPS() const;

    /**
     * Get average FPS over last N frames
     */
    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    float GetAverageFPS(int32 FrameCount = 60) const;

    /**
     * Get current frame time in milliseconds
     */
    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    float GetCurrentFrameTime() const;

    /**
     * Get current memory usage in MB
     */
    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    float GetCurrentMemoryUsage() const;

    /**
     * Set performance thresholds
     */
    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    void SetPerformanceThresholds(const FEng_PerformanceThresholds& NewThresholds);

    /**
     * Get current performance thresholds
     */
    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    FEng_PerformanceThresholds GetPerformanceThresholds() const { return Thresholds; }

    /**
     * Check if performance is within acceptable limits
     */
    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    bool IsPerformanceAcceptable() const;

    /**
     * Get current performance warnings
     */
    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    TArray<FEng_PerformanceWarning> GetCurrentWarnings() const;

    /**
     * Clear all performance warnings
     */
    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    void ClearWarnings();

    /**
     * Enable/disable automatic performance logging
     */
    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    void SetAutoLoggingEnabled(bool bEnabled);

    /**
     * Force a performance snapshot to log
     */
    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    void LogPerformanceSnapshot();

protected:
    /**
     * Update performance metrics - called every frame when monitoring
     */
    void UpdatePerformanceMetrics();

    /**
     * Check for performance threshold violations
     */
    void CheckPerformanceThresholds();

    /**
     * Add a performance warning
     */
    void AddPerformanceWarning(const FString& WarningType, float CurrentValue, float ThresholdValue, const FString& Description);

private:
    // Monitoring state
    UPROPERTY()
    bool bIsMonitoring;

    UPROPERTY()
    bool bAutoLoggingEnabled;

    // Performance thresholds
    UPROPERTY()
    FEng_PerformanceThresholds Thresholds;

    // Frame time tracking
    UPROPERTY()
    TArray<float> FrameTimeHistory;

    UPROPERTY()
    int32 MaxFrameHistorySize;

    // Current performance warnings
    UPROPERTY()
    TArray<FEng_PerformanceWarning> CurrentWarnings;

    // Timing
    double LastUpdateTime;
    double LastLogTime;
    float LoggingInterval; // Seconds between automatic logs

    // Tick delegate handle
    FDelegateHandle TickDelegateHandle;
};