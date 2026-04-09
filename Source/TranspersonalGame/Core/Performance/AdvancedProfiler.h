// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "AdvancedProfiler.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAdvancedProfiler, Log, All);

/**
 * Profiling session types
 */
UENUM(BlueprintType)
enum class EProfilingSessionType : uint8
{
    Performance     UMETA(DisplayName = "Performance"),
    Memory          UMETA(DisplayName = "Memory"),
    Physics         UMETA(DisplayName = "Physics"),
    Rendering       UMETA(DisplayName = "Rendering"),
    Gameplay        UMETA(DisplayName = "Gameplay"),
    Comprehensive   UMETA(DisplayName = "Comprehensive")
};

/**
 * Profiling data point structure
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FProfilingDataPoint
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Profiling")
    float TimeStamp;

    UPROPERTY(BlueprintReadOnly, Category = "Profiling")
    FString MetricName;

    UPROPERTY(BlueprintReadOnly, Category = "Profiling")
    float Value;

    UPROPERTY(BlueprintReadOnly, Category = "Profiling")
    FString Category;

    UPROPERTY(BlueprintReadOnly, Category = "Profiling")
    FString Context;

    FProfilingDataPoint()
        : TimeStamp(0.0f)
        , MetricName(TEXT(""))
        , Value(0.0f)
        , Category(TEXT(""))
        , Context(TEXT(""))
    {}
};

/**
 * Performance benchmark results
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBenchmarkResults
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Benchmark")
    FString BenchmarkName;

    UPROPERTY(BlueprintReadOnly, Category = "Benchmark")
    float AverageFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Benchmark")
    float MinFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Benchmark")
    float MaxFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Benchmark")
    float AverageFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Benchmark")
    float MinFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Benchmark")
    float MaxFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Benchmark")
    float AverageMemoryMB;

    UPROPERTY(BlueprintReadOnly, Category = "Benchmark")
    float PeakMemoryMB;

    UPROPERTY(BlueprintReadOnly, Category = "Benchmark")
    int32 TotalFrames;

    UPROPERTY(BlueprintReadOnly, Category = "Benchmark")
    float Duration;

    FBenchmarkResults()
        : BenchmarkName(TEXT(""))
        , AverageFrameTime(0.0f)
        , MinFrameTime(0.0f)
        , MaxFrameTime(0.0f)
        , AverageFPS(0.0f)
        , MinFPS(0.0f)
        , MaxFPS(0.0f)
        , AverageMemoryMB(0.0f)
        , PeakMemoryMB(0.0f)
        , TotalFrames(0)
        , Duration(0.0f)
    {}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnProfilingSessionComplete, const FBenchmarkResults&, Results);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPerformanceAlert, const FString&, AlertType, float, Value);

/**
 * Advanced Performance Profiler System
 * Provides comprehensive performance profiling and benchmarking capabilities
 * Integrates with Unreal Insights and custom metrics collection
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAdvancedProfiler : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UAdvancedProfiler();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

public:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableAutomaticProfiling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float ProfilingSampleRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableDetailedLogging;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FString ProfileDataDirectory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    int32 MaxDataPointsPerSession;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnProfilingSessionComplete OnProfilingSessionComplete;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPerformanceAlert OnPerformanceAlert;

    // Public Methods
    UFUNCTION(BlueprintCallable, Category = "Profiling")
    void StartProfilingSession(EProfilingSessionType SessionType, const FString& SessionName);

    UFUNCTION(BlueprintCallable, Category = "Profiling")
    void StopProfilingSession();

    UFUNCTION(BlueprintCallable, Category = "Profiling")
    bool IsProfilingActive() const { return bProfilingActive; }

    UFUNCTION(BlueprintCallable, Category = "Profiling")
    void AddCustomMetric(const FString& MetricName, float Value, const FString& Category = TEXT("Custom"));

    UFUNCTION(BlueprintCallable, Category = "Profiling")
    TArray<FProfilingDataPoint> GetSessionData() const { return CurrentSessionData; }

    UFUNCTION(BlueprintCallable, Category = "Profiling")
    FBenchmarkResults GetCurrentSessionResults() const;

    UFUNCTION(BlueprintCallable, Category = "Benchmarking")
    void StartBenchmark(const FString& BenchmarkName, float Duration = 60.0f);

    UFUNCTION(BlueprintCallable, Category = "Benchmarking")
    void StopBenchmark();

    UFUNCTION(BlueprintCallable, Category = "Benchmarking")
    TArray<FBenchmarkResults> GetBenchmarkHistory() const { return BenchmarkHistory; }

    UFUNCTION(BlueprintCallable, Category = "Export")
    void ExportSessionToCSV(const FString& FilePath);

    UFUNCTION(BlueprintCallable, Category = "Export")
    void ExportSessionToJSON(const FString& FilePath);

    UFUNCTION(BlueprintCallable, Category = "Analysis")
    void AnalyzePerformanceBottlenecks();

    UFUNCTION(BlueprintCallable, Category = "Analysis")
    void GeneratePerformanceReport(const FString& ReportPath);

    // Advanced profiling methods
    UFUNCTION(BlueprintCallable, Category = "Advanced")
    void StartUnrealInsightsTrace();

    UFUNCTION(BlueprintCallable, Category = "Advanced")
    void StopUnrealInsightsTrace();

    UFUNCTION(BlueprintCallable, Category = "Advanced")
    void CaptureMemorySnapshot(const FString& SnapshotName);

    UFUNCTION(BlueprintCallable, Category = "Advanced")
    void ProfilePhysicsPerformance(float Duration = 10.0f);

    UFUNCTION(BlueprintCallable, Category = "Advanced")
    void ProfileRenderingPerformance(float Duration = 10.0f);

    UFUNCTION(BlueprintCallable, Category = "Advanced")
    void ProfileGameplayPerformance(float Duration = 10.0f);

private:
    // Internal state
    bool bProfilingActive;
    EProfilingSessionType CurrentSessionType;
    FString CurrentSessionName;
    float SessionStartTime;
    TArray<FProfilingDataPoint> CurrentSessionData;
    TArray<FBenchmarkResults> BenchmarkHistory;
    
    // Benchmark state
    bool bBenchmarkActive;
    FString CurrentBenchmarkName;
    float BenchmarkStartTime;
    float BenchmarkDuration;
    TArray<float> BenchmarkFrameTimes;
    TArray<float> BenchmarkMemoryUsage;
    
    // Timers
    FTimerHandle ProfilingTimerHandle;
    FTimerHandle BenchmarkTimerHandle;
    
    // File paths
    FString CurrentSessionFilePath;
    FString UnrealInsightsTracePath;
    
    // Internal methods
    void UpdateProfilingData();
    void CollectPerformanceMetrics();
    void CollectMemoryMetrics();
    void CollectPhysicsMetrics();
    void CollectRenderingMetrics();
    void CollectGameplayMetrics();
    
    // Data collection helpers
    float GetFrameTime() const;
    float GetGameThreadTime() const;
    float GetRenderThreadTime() const;
    float GetGPUTime() const;
    float GetPhysicsTime() const;
    float GetMemoryUsage() const;
    int32 GetDrawCalls() const;
    int32 GetTriangles() const;
    int32 GetActivePhysicsObjects() const;
    
    // Analysis methods
    void AnalyzeFrameTimeVariability();
    void AnalyzeMemoryUsagePatterns();
    void AnalyzePhysicsPerformance();
    void AnalyzeRenderingPerformance();
    void DetectPerformanceRegression();
    
    // File I/O
    void SaveSessionData();
    void LoadSessionData(const FString& FilePath);
    void WriteCSVHeader(FString& CSVContent);
    void WriteDataPointToCSV(const FProfilingDataPoint& DataPoint, FString& CSVContent);
    void WriteJSONData(const TArray<FProfilingDataPoint>& Data, FString& JSONContent);
    
    // Unreal Insights integration
    void InitializeUnrealInsights();
    void ConfigureTraceChannels();
    void StartTraceCapture(const FString& TraceName);
    void StopTraceCapture();
    
    // Performance alerts
    void CheckPerformanceThresholds(const FProfilingDataPoint& DataPoint);
    void TriggerPerformanceAlert(const FString& AlertType, float Value);
    
    // Benchmark calculations
    FBenchmarkResults CalculateBenchmarkResults() const;
    void ResetBenchmarkData();
    
    // Utility methods
    FString GenerateSessionFileName() const;
    FString GetTimestampString() const;
    void EnsureDirectoryExists(const FString& DirectoryPath);
    
    // Platform-specific profiling
    void CollectPlatformSpecificMetrics();
    void CollectCPUMetrics();
    void CollectGPUMetrics();
    void CollectIOMetrics();
    
    // Custom metric tracking
    TMap<FString, TArray<float>> CustomMetrics;
    void UpdateCustomMetrics();
    void AnalyzeCustomMetrics();
};