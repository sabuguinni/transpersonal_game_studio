#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "RenderCore.h"
#include "RHI.h"
#include "RHICommandList.h"
#include "Containers/Array.h"
#include "Containers/Map.h"
#include "Perf_GPUProfiler.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_GPUMemoryInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int64 TotalMemoryMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int64 UsedMemoryMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int64 AvailableMemoryMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float UsagePercentage;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int64 TextureMemoryMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int64 BufferMemoryMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int64 RenderTargetMemoryMB;

    FPerf_GPUMemoryInfo()
        : TotalMemoryMB(0)
        , UsedMemoryMB(0)
        , AvailableMemoryMB(0)
        , UsagePercentage(0.0f)
        , TextureMemoryMB(0)
        , BufferMemoryMB(0)
        , RenderTargetMemoryMB(0)
    {
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_GPUTimingData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FString PassName;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float TimeMS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PercentageOfFrame;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 Triangles;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bIsBottleneck;

    FPerf_GPUTimingData()
        : PassName(TEXT("Unknown"))
        , TimeMS(0.0f)
        , PercentageOfFrame(0.0f)
        , DrawCalls(0)
        , Triangles(0)
        , bIsBottleneck(false)
    {
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_GPUStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float TotalFrameTimeMS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TotalDrawCalls;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TotalTriangles;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TotalVertices;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUUtilizationPercent;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryBandwidthMBps;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TextureStreamingPoolMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bIsGPUBound;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bIsMemoryBound;

    FPerf_GPUStats()
        : TotalFrameTimeMS(0.0f)
        , CurrentFPS(0.0f)
        , TotalDrawCalls(0)
        , TotalTriangles(0)
        , TotalVertices(0)
        , GPUUtilizationPercent(0.0f)
        , MemoryBandwidthMBps(0.0f)
        , TextureStreamingPoolMB(0)
        , bIsGPUBound(false)
        , bIsMemoryBound(false)
    {
    }
};

/**
 * GPU profiler for monitoring graphics performance and identifying bottlenecks
 * Tracks render passes, memory usage, and provides optimization recommendations
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_GPUProfiler : public UObject
{
    GENERATED_BODY()

public:
    UPerf_GPUProfiler();

    // Core profiling functions
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateGPUStats();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_GPUStats GetGPUStats() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_GPUMemoryInfo GetGPUMemoryInfo() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    TArray<FPerf_GPUTimingData> GetRenderPassTimings() const;

    // Analysis functions
    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsGPUBound(float ThresholdMS = 16.67f) const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsMemoryBound() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsDrawCallBound(int32 ThresholdDrawCalls = 2000) const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    TArray<FPerf_GPUTimingData> GetBottleneckPasses(float ThresholdPercent = 15.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetTargetResolutionScale() const;

    // Optimization recommendations
    UFUNCTION(BlueprintCallable, Category = "Performance")
    TArray<FString> GetGPUOptimizationRecommendations() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool ShouldReduceResolution() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool ShouldReduceEffects() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool ShouldOptimizeTextures() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool ShouldReduceDrawCalls() const;

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetProfilingEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetTargetFPS(float TargetFPS);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetMemoryWarningThreshold(float ThresholdPercent);

    // Debug output
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void LogGPUReport() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FString GetGPUReportString() const;

    // Render pass tracking
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void BeginRenderPassTracking(const FString& PassName);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EndRenderPassTracking(const FString& PassName);

protected:
    // GPU data storage
    UPROPERTY()
    FPerf_GPUStats CurrentStats;

    UPROPERTY()
    FPerf_GPUMemoryInfo CurrentMemoryInfo;

    UPROPERTY()
    TArray<FPerf_GPUTimingData> RenderPassTimings;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    bool bIsProfilingEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float TargetFrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float MemoryWarningThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float GPUBoundThreshold;

    // Timing tracking
    UPROPERTY()
    TMap<FString, double> PassStartTimes;

    UPROPERTY()
    double LastUpdateTime;

    UPROPERTY()
    double ProfilingStartTime;

    // Frame history for averaging
    UPROPERTY()
    TArray<float> FrameTimeHistory;

    UPROPERTY()
    int32 MaxFrameHistory;

private:
    // Internal helper functions
    void CaptureGPUStats();
    void CaptureMemoryInfo();
    void UpdateRenderPassTimings();
    float CalculateAverageFrameTime() const;
    void AnalyzeBottlenecks();
    bool IsRenderPassBottleneck(const FPerf_GPUTimingData& PassData) const;
    float GetGPUUtilization() const;
    void UpdateFrameHistory(float FrameTime);
    FPerf_GPUTimingData CreateTimingData(const FString& PassName, float TimeMS) const;
};