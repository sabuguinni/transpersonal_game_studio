#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Core_PhysicsProfiler.generated.h"

/**
 * Physics Profiler - Comprehensive performance monitoring and analysis system
 * Tracks physics performance metrics, generates reports, and provides optimization insights
 * Part of the Core Systems module for prehistoric survival game
 */

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsMetrics
{
    GENERATED_BODY()

    /** Frame time for physics simulation (milliseconds) */
    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float PhysicsFrameTime = 0.0f;

    /** Number of active rigid bodies */
    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 ActiveRigidBodies = 0;

    /** Number of active constraints */
    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 ActiveConstraints = 0;

    /** Number of collision pairs processed */
    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 CollisionPairs = 0;

    /** Memory usage by physics simulation (MB) */
    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float PhysicsMemoryUsage = 0.0f;

    /** CPU usage percentage for physics */
    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float PhysicsCPUUsage = 0.0f;

    /** Timestamp when metrics were captured */
    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float Timestamp = 0.0f;

    FCore_PhysicsMetrics()
    {
        PhysicsFrameTime = 0.0f;
        ActiveRigidBodies = 0;
        ActiveConstraints = 0;
        CollisionPairs = 0;
        PhysicsMemoryUsage = 0.0f;
        PhysicsCPUUsage = 0.0f;
        Timestamp = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PerformanceReport
{
    GENERATED_BODY()

    /** Average frame time over reporting period */
    UPROPERTY(BlueprintReadOnly, Category = "Report")
    float AverageFrameTime = 0.0f;

    /** Peak frame time during reporting period */
    UPROPERTY(BlueprintReadOnly, Category = "Report")
    float PeakFrameTime = 0.0f;

    /** Minimum frame time during reporting period */
    UPROPERTY(BlueprintReadOnly, Category = "Report")
    float MinFrameTime = 0.0f;

    /** Frame time variance (stability metric) */
    UPROPERTY(BlueprintReadOnly, Category = "Report")
    float FrameTimeVariance = 0.0f;

    /** Performance score (0-100) */
    UPROPERTY(BlueprintReadOnly, Category = "Report")
    float PerformanceScore = 0.0f;

    /** Number of performance warnings */
    UPROPERTY(BlueprintReadOnly, Category = "Report")
    int32 WarningCount = 0;

    /** Report generation timestamp */
    UPROPERTY(BlueprintReadOnly, Category = "Report")
    FString ReportTimestamp;

    FCore_PerformanceReport()
    {
        AverageFrameTime = 0.0f;
        PeakFrameTime = 0.0f;
        MinFrameTime = 0.0f;
        FrameTimeVariance = 0.0f;
        PerformanceScore = 0.0f;
        WarningCount = 0;
        ReportTimestamp = TEXT("");
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCore_PhysicsProfiler : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCore_PhysicsProfiler();

    // === SUBSYSTEM INTERFACE ===
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

protected:
    virtual void Tick(float DeltaTime) override;

public:
    // === PROFILING CONTROL ===

    /** Start physics profiling */
    UFUNCTION(BlueprintCallable, Category = "Physics Profiling")
    void StartProfiling();

    /** Stop physics profiling */
    UFUNCTION(BlueprintCallable, Category = "Physics Profiling")
    void StopProfiling();

    /** Reset all profiling data */
    UFUNCTION(BlueprintCallable, Category = "Physics Profiling")
    void ResetProfilingData();

    /** Check if profiling is currently active */
    UFUNCTION(BlueprintCallable, Category = "Physics Profiling")
    bool IsProfilingActive() const { return bIsProfilingActive; }

    // === METRICS COLLECTION ===

    /** Get current physics metrics */
    UFUNCTION(BlueprintCallable, Category = "Physics Profiling")
    FCore_PhysicsMetrics GetCurrentMetrics();

    /** Get metrics history for specified duration (seconds) */
    UFUNCTION(BlueprintCallable, Category = "Physics Profiling")
    TArray<FCore_PhysicsMetrics> GetMetricsHistory(float DurationSeconds = 60.0f);

    /** Get average metrics over specified period */
    UFUNCTION(BlueprintCallable, Category = "Physics Profiling")
    FCore_PhysicsMetrics GetAverageMetrics(float PeriodSeconds = 10.0f);

    // === PERFORMANCE ANALYSIS ===

    /** Generate comprehensive performance report */
    UFUNCTION(BlueprintCallable, Category = "Physics Profiling")
    FCore_PerformanceReport GeneratePerformanceReport(float AnalysisPeriod = 60.0f);

    /** Get performance score (0-100) */
    UFUNCTION(BlueprintCallable, Category = "Physics Profiling")
    float GetPerformanceScore();

    /** Check if performance is within acceptable limits */
    UFUNCTION(BlueprintCallable, Category = "Physics Profiling")
    bool IsPerformanceAcceptable();

    /** Get performance bottlenecks */
    UFUNCTION(BlueprintCallable, Category = "Physics Profiling")
    TArray<FString> GetPerformanceBottlenecks();

    // === REPORTING AND LOGGING ===

    /** Export metrics to CSV file */
    UFUNCTION(BlueprintCallable, Category = "Physics Profiling")
    bool ExportMetricsToCSV(const FString& FilePath);

    /** Export performance report to text file */
    UFUNCTION(BlueprintCallable, Category = "Physics Profiling")
    bool ExportReportToFile(const FString& FilePath);

    /** Log current performance status */
    UFUNCTION(BlueprintCallable, Category = "Physics Profiling")
    void LogPerformanceStatus();

    // === CONFIGURATION ===

    /** Set profiling update frequency (Hz) */
    UFUNCTION(BlueprintCallable, Category = "Physics Profiling")
    void SetProfilingFrequency(float Frequency);

    /** Set maximum history size (number of samples) */
    UFUNCTION(BlueprintCallable, Category = "Physics Profiling")
    void SetMaxHistorySize(int32 MaxSize);

    /** Enable/disable automatic report generation */
    UFUNCTION(BlueprintCallable, Category = "Physics Profiling")
    void SetAutoReportGeneration(bool bEnabled, float IntervalSeconds = 300.0f);

protected:
    // === PROFILING STATE ===

    /** Whether profiling is currently active */
    UPROPERTY(BlueprintReadOnly, Category = "Profiling")
    bool bIsProfilingActive = false;

    /** Profiling update frequency */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profiling")
    float ProfilingFrequency = 10.0f; // 10 Hz

    /** Maximum number of metrics samples to store */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profiling")
    int32 MaxHistorySize = 3600; // 1 hour at 1Hz

    /** Enable automatic report generation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profiling")
    bool bAutoReportGeneration = false;

    /** Auto report generation interval */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profiling")
    float AutoReportInterval = 300.0f; // 5 minutes

    // === PERFORMANCE THRESHOLDS ===

    /** Target physics frame time (milliseconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float TargetFrameTime = 8.33f; // 120 Hz

    /** Warning threshold for frame time */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float WarningFrameTime = 16.67f; // 60 Hz

    /** Critical threshold for frame time */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float CriticalFrameTime = 33.33f; // 30 Hz

    /** Maximum acceptable rigid bodies */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    int32 MaxRigidBodies = 1000;

    /** Maximum acceptable memory usage (MB) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float MaxMemoryUsage = 512.0f;

private:
    // === INTERNAL DATA ===

    /** Metrics history storage */
    TArray<FCore_PhysicsMetrics> MetricsHistory;

    /** Last profiling update time */
    float LastProfilingTime = 0.0f;

    /** Last auto report generation time */
    float LastAutoReportTime = 0.0f;

    /** Current performance warnings */
    TArray<FString> CurrentWarnings;

    // === INTERNAL FUNCTIONS ===

    /** Collect current physics metrics */
    FCore_PhysicsMetrics CollectCurrentMetrics();

    /** Update metrics history */
    void UpdateMetricsHistory(const FCore_PhysicsMetrics& NewMetrics);

    /** Calculate performance score based on metrics */
    float CalculatePerformanceScore(const FCore_PhysicsMetrics& Metrics);

    /** Analyze performance and generate warnings */
    void AnalyzePerformance(const FCore_PhysicsMetrics& Metrics);

    /** Generate automatic report if needed */
    void CheckAutoReportGeneration();

    /** Clean old metrics from history */
    void CleanOldMetrics();

    /** Format timestamp for reports */
    FString FormatTimestamp() const;
};