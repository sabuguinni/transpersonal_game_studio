// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "PhysicsPerformanceProfiler.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPhysicsProfiler, Log, All);

/**
 * Performance metrics structure for physics systems
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPhysicsPerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PhysicsTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActivePhysicsObjects;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 CollisionChecks;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CPUUsagePercent;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUUsagePercent;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float TimeStamp;

    FPhysicsPerformanceMetrics()
        : FrameTime(0.0f)
        , PhysicsTime(0.0f)
        , ActivePhysicsObjects(0)
        , CollisionChecks(0)
        , MemoryUsageMB(0.0f)
        , CPUUsagePercent(0.0f)
        , GPUUsagePercent(0.0f)
        , DrawCalls(0)
        , TimeStamp(0.0f)
    {}
};

/**
 * Performance alert levels for monitoring
 */
UENUM(BlueprintType)
enum class EPerformanceAlertLevel : uint8
{
    Normal      UMETA(DisplayName = "Normal"),
    Warning     UMETA(DisplayName = "Warning"),
    Critical    UMETA(DisplayName = "Critical"),
    Emergency   UMETA(DisplayName = "Emergency")
};

/**
 * Performance alert structure
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerformanceAlert
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Alert")
    EPerformanceAlertLevel AlertLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Alert")
    FString AlertMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Alert")
    float TimeStamp;

    UPROPERTY(BlueprintReadOnly, Category = "Alert")
    FString MetricName;

    UPROPERTY(BlueprintReadOnly, Category = "Alert")
    float MetricValue;

    UPROPERTY(BlueprintReadOnly, Category = "Alert")
    float ThresholdValue;

    FPerformanceAlert()
        : AlertLevel(EPerformanceAlertLevel::Normal)
        , AlertMessage(TEXT(""))
        , TimeStamp(0.0f)
        , MetricName(TEXT(""))
        , MetricValue(0.0f)
        , ThresholdValue(0.0f)
    {}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPerformanceAlert, const FPerformanceAlert&, Alert);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPerformanceReport, const FPhysicsPerformanceMetrics&, Metrics);

/**
 * Physics Performance Profiler Component
 * Monitors and profiles physics system performance in real-time
 * Provides alerts and detailed metrics for optimization
 */
UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPhysicsPerformanceProfiler : public UActorComponent
{
    GENERATED_BODY()

public:
    UPhysicsPerformanceProfiler();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profiling")
    bool bEnableProfiling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profiling")
    float ProfilingInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profiling")
    bool bEnableAlerts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profiling")
    bool bLogToFile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profiling")
    FString LogFileName;

    // Performance Thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float FrameTimeWarningThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float FrameTimeCriticalThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float MemoryWarningThresholdMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float MemoryCriticalThresholdMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    int32 PhysicsObjectWarningThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    int32 PhysicsObjectCriticalThreshold;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPerformanceAlert OnPerformanceAlert;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPerformanceReport OnPerformanceReport;

    // Public Methods
    UFUNCTION(BlueprintCallable, Category = "Profiling")
    void StartProfiling();

    UFUNCTION(BlueprintCallable, Category = "Profiling")
    void StopProfiling();

    UFUNCTION(BlueprintCallable, Category = "Profiling")
    void ResetMetrics();

    UFUNCTION(BlueprintCallable, Category = "Profiling")
    FPhysicsPerformanceMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Profiling")
    TArray<FPhysicsPerformanceMetrics> GetMetricsHistory() const;

    UFUNCTION(BlueprintCallable, Category = "Profiling")
    TArray<FPerformanceAlert> GetActiveAlerts() const;

    UFUNCTION(BlueprintCallable, Category = "Profiling")
    void ExportMetricsToCSV(const FString& FilePath);

    UFUNCTION(BlueprintCallable, Category = "Profiling")
    void SetCustomThreshold(const FString& MetricName, float WarningThreshold, float CriticalThreshold);

    // Advanced Profiling
    UFUNCTION(BlueprintCallable, Category = "Advanced")
    void StartDetailedProfiling();

    UFUNCTION(BlueprintCallable, Category = "Advanced")
    void StopDetailedProfiling();

    UFUNCTION(BlueprintCallable, Category = "Advanced")
    void GeneratePerformanceReport();

private:
    // Internal state
    bool bProfilingActive;
    float LastProfilingTime;
    TArray<FPhysicsPerformanceMetrics> MetricsHistory;
    TArray<FPerformanceAlert> ActiveAlerts;
    
    // Performance tracking
    float AccumulatedFrameTime;
    float AccumulatedPhysicsTime;
    int32 FrameCount;
    
    // File logging
    FString LogFilePath;
    bool bLogFileInitialized;
    
    // Detailed profiling
    bool bDetailedProfilingActive;
    TMap<FString, float> CustomThresholds;
    
    // Internal methods
    void UpdateMetrics();
    void CollectPerformanceData(FPhysicsPerformanceMetrics& OutMetrics);
    void CheckPerformanceThresholds(const FPhysicsPerformanceMetrics& Metrics);
    void TriggerAlert(EPerformanceAlertLevel Level, const FString& Message, const FString& MetricName, float Value, float Threshold);
    void LogMetricsToFile(const FPhysicsPerformanceMetrics& Metrics);
    void InitializeLogFile();
    
    // Platform-specific performance queries
    float GetCPUUsage() const;
    float GetGPUUsage() const;
    float GetPhysicsMemoryUsage() const;
    int32 GetActivePhysicsObjectCount() const;
    int32 GetCollisionCheckCount() const;
    int32 GetDrawCallCount() const;
    
    // Optimization suggestions
    void AnalyzePerformanceBottlenecks(const FPhysicsPerformanceMetrics& Metrics);
    void SuggestOptimizations();
};