#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "HAL/ThreadManager.h"
#include "HAL/PlatformProcess.h"
#include "Containers/Array.h"
#include "Containers/Map.h"
#include "Perf_ThreadProfiler.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_ThreadData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FString ThreadName;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    uint32 ThreadID;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CPUUsagePercent;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float ExecutionTimeMS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 Priority;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bIsActive;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    double LastUpdateTime;

    FPerf_ThreadData()
        : ThreadName(TEXT("Unknown"))
        , ThreadID(0)
        , CPUUsagePercent(0.0f)
        , ExecutionTimeMS(0.0f)
        , Priority(0)
        , bIsActive(false)
        , LastUpdateTime(0.0)
    {
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_ThreadStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TotalThreadCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveThreadCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float TotalCPUUsage;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GameThreadUsage;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderThreadUsage;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float WorkerThreadsUsage;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageThreadUsage;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 HighUsageThreadCount;

    FPerf_ThreadStats()
        : TotalThreadCount(0)
        , ActiveThreadCount(0)
        , TotalCPUUsage(0.0f)
        , GameThreadUsage(0.0f)
        , RenderThreadUsage(0.0f)
        , WorkerThreadsUsage(0.0f)
        , AverageThreadUsage(0.0f)
        , HighUsageThreadCount(0)
    {
    }
};

/**
 * Thread profiler for monitoring CPU usage across all game threads
 * Tracks thread performance, identifies bottlenecks, and provides optimization insights
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_ThreadProfiler : public UObject
{
    GENERATED_BODY()

public:
    UPerf_ThreadProfiler();

    // Core profiling functions
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateThreadData();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_ThreadStats GetThreadStats() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    TArray<FPerf_ThreadData> GetAllThreadData() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    TArray<FPerf_ThreadData> GetHighUsageThreads(float ThresholdPercent = 50.0f) const;

    // Analysis functions
    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsGameThreadOverloaded(float ThresholdPercent = 80.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsRenderThreadOverloaded(float ThresholdPercent = 80.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool AreWorkerThreadsOverloaded(float ThresholdPercent = 70.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCPUCoreUtilization() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetOptimalWorkerThreadCount() const;

    // Thread management recommendations
    UFUNCTION(BlueprintCallable, Category = "Performance")
    TArray<FString> GetPerformanceRecommendations() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool ShouldReduceThreadLoad() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool ShouldIncreaseThreadCount() const;

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetProfilingInterval(float IntervalSeconds);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetHighUsageThreshold(float ThresholdPercent);

    // Debug output
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void LogThreadReport() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FString GetThreadReportString() const;

protected:
    // Thread data storage
    UPROPERTY()
    TMap<uint32, FPerf_ThreadData> ThreadDataMap;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float ProfilingInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float HighUsageThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    bool bIsProfiling;

    // Timing
    UPROPERTY()
    double LastProfilingTime;

    UPROPERTY()
    double ProfilingStartTime;

    // System info
    UPROPERTY()
    int32 SystemCoreCount;

    UPROPERTY()
    int32 SystemThreadCount;

private:
    // Internal helper functions
    void CaptureThreadData();
    FPerf_ThreadData CreateThreadData(uint32 ThreadID, const FString& ThreadName) const;
    float CalculateThreadCPUUsage(uint32 ThreadID) const;
    void IdentifyKnownThreads();
    bool IsKnownGameThread(uint32 ThreadID) const;
    bool IsKnownRenderThread(uint32 ThreadID) const;
};