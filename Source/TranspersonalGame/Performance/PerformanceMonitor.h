#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"
#include "PerformanceTypes.h"
#include "PerformanceMonitor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPerformanceThresholdExceeded, EPerf_BudgetCategory, Category);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQualityLevelChanged, EPerf_QualityLevel, NewLevel);

/**
 * Central performance monitoring system
 * Tracks frame time, memory usage, and system performance
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APerf_PerformanceMonitor : public AActor
{
    GENERATED_BODY()

public:
    APerf_PerformanceMonitor();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaTime) override;

public:
    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_MetricsSnapshot GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetMonitoringFrequency(EPerf_MonitoringFrequency Frequency);

    // Budget management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetBudgetAllocation(const FPerf_BudgetAllocation& Allocation);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsBudgetExceeded(EPerf_BudgetCategory Category) const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetBudgetUsage(EPerf_BudgetCategory Category) const;

    // Quality scaling
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetQualityLevel(EPerf_QualityLevel Level);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerf_QualityLevel GetCurrentQualityLevel() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableAdaptiveQuality(bool bEnable);

    // Profiling utilities
    UFUNCTION(BlueprintCallable, Category = "Performance", CallInEditor)
    void DumpPerformanceReport();

    UFUNCTION(BlueprintCallable, Category = "Performance", CallInEditor)
    void ResetPerformanceCounters();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetTargetFPS(float TargetFPS);

protected:
    // Core monitoring
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bIsMonitoring;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerf_MonitoringFrequency MonitoringFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FPerf_OptimizationSettings OptimizationSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    TArray<FPerf_BudgetAllocation> BudgetAllocations;

    // Metrics tracking
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_MetricsSnapshot CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    TArray<FPerf_MetricsSnapshot> MetricsHistory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "10", ClampMax = "1000"))
    int32 MaxHistoryEntries;

    // Performance thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float FrameTimeWarningThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MemoryWarningThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float GPUTimeWarningThreshold;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Performance")
    FOnPerformanceThresholdExceeded OnPerformanceThresholdExceeded;

    UPROPERTY(BlueprintAssignable, Category = "Performance")
    FOnQualityLevelChanged OnQualityLevelChanged;

private:
    // Internal tracking
    float AccumulatedDeltaTime;
    int32 FrameCounter;
    int32 MonitoringFrameInterval;
    
    // Performance history
    float LastFPSCheck;
    float FPSCheckInterval;
    
    // Console variables for runtime adjustment
    TAutoConsoleVariable<float> CVarTargetFPS;
    TAutoConsoleVariable<bool> CVarAdaptiveQuality;
    TAutoConsoleVariable<int32> CVarQualityLevel;

    // Internal methods
    void UpdateMetrics();
    void CheckPerformanceThresholds();
    void UpdateAdaptiveQuality();
    void ApplyQualitySettings(EPerf_QualityLevel Level);
    float GetCurrentFPS() const;
    float GetCurrentFrameTime() const;
    float GetMemoryUsage() const;
    void LogPerformanceMetrics() const;
};