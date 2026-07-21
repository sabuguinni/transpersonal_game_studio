#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "GameFramework/GameModeBase.h"
#include "Subsystems/WorldSubsystem.h"
#include "Containers/Array.h"
#include "Containers/Map.h"
#include "Perf_RealTimeProfiler.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_ProfilerSample
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FString SampleName;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float TimeMS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CPUTimeMS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUTimeMS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float Timestamp;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bIsSpike;

    FPerf_ProfilerSample()
        : SampleName(TEXT("Unknown"))
        , TimeMS(0.0f)
        , CPUTimeMS(0.0f)
        , GPUTimeMS(0.0f)
        , MemoryUsageMB(0)
        , Timestamp(0.0f)
        , bIsSpike(false)
    {
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_SystemMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFrameTimeMS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MinFrameTimeMS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MaxFrameTimeMS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MinFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MaxFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TotalSamples;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 SpikeCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float SpikePercentage;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float TotalProfilingTimeSeconds;

    FPerf_SystemMetrics()
        : AverageFrameTimeMS(0.0f)
        , MinFrameTimeMS(0.0f)
        , MaxFrameTimeMS(0.0f)
        , AverageFPS(0.0f)
        , MinFPS(0.0f)
        , MaxFPS(0.0f)
        , TotalSamples(0)
        , SpikeCount(0)
        , SpikePercentage(0.0f)
        , TotalProfilingTimeSeconds(0.0f)
    {
    }
};

UENUM(BlueprintType)
enum class EPerf_ProfilerMode : uint8
{
    Disabled        UMETA(DisplayName = "Disabled"),
    Basic          UMETA(DisplayName = "Basic Monitoring"),
    Detailed       UMETA(DisplayName = "Detailed Profiling"),
    FullDiagnostic UMETA(DisplayName = "Full Diagnostic Mode")
};

/**
 * Real-time performance profiler that continuously monitors game performance
 * Provides detailed metrics, spike detection, and performance analysis
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_RealTimeProfiler : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UPerf_RealTimeProfiler();

    // Subsystem overrides
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Core profiling functions
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartProfiling(EPerf_ProfilerMode Mode = EPerf_ProfilerMode::Basic);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void PauseProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ResumeProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ResetProfiler();

    // Sample collection
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void CollectSample(const FString& SampleName = TEXT("Frame"));

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void BeginCustomSample(const FString& SampleName);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EndCustomSample(const FString& SampleName);

    // Data access
    UFUNCTION(BlueprintCallable, Category = "Performance")
    TArray<FPerf_ProfilerSample> GetRecentSamples(int32 Count = 60) const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    TArray<FPerf_ProfilerSample> GetSamplesInTimeRange(float StartTime, float EndTime) const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_SystemMetrics GetSystemMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    TArray<FPerf_ProfilerSample> GetPerformanceSpikes(float ThresholdMS = 33.33f) const;

    // Analysis functions
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetAverageFrameTime(int32 SampleCount = 60) const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetAverageFPS(int32 SampleCount = 60) const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool HasPerformanceIssues(float FrameTimeThresholdMS = 33.33f, float SpikeThresholdPercent = 5.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    TArray<FString> GetPerformanceAnalysis() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsCurrentlyProfiling() const;

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetProfilingMode(EPerf_ProfilerMode Mode);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetMaxSamples(int32 MaxSamples);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetSpikeThreshold(float ThresholdMS);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetAutoSaveEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetSampleInterval(float IntervalSeconds);

    // Export and logging
    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool ExportSamplesToCSV(const FString& FilePath) const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void LogPerformanceReport() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FString GetPerformanceReportString() const;

    // Event callbacks
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OnLevelChanged();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OnGameModeChanged();

protected:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    EPerf_ProfilerMode CurrentMode;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    int32 MaxSampleCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float SpikeThresholdMS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float SampleIntervalSeconds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    bool bAutoSaveEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    bool bLogSpikes;

    // State
    UPROPERTY()
    bool bIsProfilingActive;

    UPROPERTY()
    bool bIsPaused;

    UPROPERTY()
    double ProfilingStartTime;

    UPROPERTY()
    double LastSampleTime;

    UPROPERTY()
    double LastAutoSaveTime;

    // Data storage
    UPROPERTY()
    TArray<FPerf_ProfilerSample> ProfilerSamples;

    UPROPERTY()
    TMap<FString, double> CustomSampleStartTimes;

    UPROPERTY()
    FPerf_SystemMetrics CachedMetrics;

    UPROPERTY()
    bool bMetricsCacheValid;

    // Timing
    FTimerHandle SampleTimerHandle;

private:
    // Internal functions
    void TickProfiler();
    FPerf_ProfilerSample CreateSample(const FString& SampleName) const;
    void AddSample(const FPerf_ProfilerSample& Sample);
    void UpdateMetricsCache();
    void TrimOldSamples();
    bool IsSampleSpike(const FPerf_ProfilerSample& Sample) const;
    void LogSpike(const FPerf_ProfilerSample& Sample) const;
    void AutoSaveIfNeeded();
    float GetCurrentCPUTime() const;
    float GetCurrentGPUTime() const;
    int32 GetCurrentMemoryUsage() const;
    void CleanupCustomSamples();
};

/**
 * Global access helper for the real-time profiler
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UPerf_ProfilerBlueprintLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Performance", CallInEditor = true)
    static UPerf_RealTimeProfiler* GetRealTimeProfiler(const UObject* WorldContext);

    UFUNCTION(BlueprintCallable, Category = "Performance", CallInEditor = true)
    static void StartGlobalProfiling(const UObject* WorldContext, EPerf_ProfilerMode Mode = EPerf_ProfilerMode::Basic);

    UFUNCTION(BlueprintCallable, Category = "Performance", CallInEditor = true)
    static void StopGlobalProfiling(const UObject* WorldContext);

    UFUNCTION(BlueprintCallable, Category = "Performance", CallInEditor = true)
    static float GetCurrentFPS(const UObject* WorldContext);

    UFUNCTION(BlueprintCallable, Category = "Performance", CallInEditor = true)
    static float GetCurrentFrameTime(const UObject* WorldContext);

    UFUNCTION(BlueprintCallable, Category = "Performance", CallInEditor = true)
    static bool HasPerformanceProblems(const UObject* WorldContext);
};