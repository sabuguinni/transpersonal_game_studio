#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "Containers/Array.h"
#include "Containers/Map.h"
#include "HAL/Platform.h"
#include "Perf_FrameTimeAnalyzer.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_FrameTimeData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float DeltaTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GameThreadTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderThreadTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    double Timestamp;

    FPerf_FrameTimeData()
        : DeltaTime(0.0f)
        , GameThreadTime(0.0f)
        , RenderThreadTime(0.0f)
        , GPUTime(0.0f)
        , Timestamp(0.0)
    {
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_FrameTimeStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MinFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MaxFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTimeVariance;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 FramesBelow30FPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 FramesBelow60FPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PercentFramesBelow30FPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PercentFramesBelow60FPS;

    FPerf_FrameTimeStats()
        : AverageFrameTime(0.0f)
        , MinFrameTime(0.0f)
        , MaxFrameTime(0.0f)
        , FrameTimeVariance(0.0f)
        , FramesBelow30FPS(0)
        , FramesBelow60FPS(0)
        , PercentFramesBelow30FPS(0.0f)
        , PercentFramesBelow60FPS(0.0f)
    {
    }
};

/**
 * Advanced frame time analyzer for performance optimization
 * Tracks frame times, identifies bottlenecks, and provides detailed statistics
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_FrameTimeAnalyzer : public UObject
{
    GENERATED_BODY()

public:
    UPerf_FrameTimeAnalyzer();

    // Core analysis functions
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartAnalysis();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopAnalysis();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void RecordFrame();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_FrameTimeStats GetFrameTimeStats() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    TArray<FPerf_FrameTimeData> GetRecentFrameData(int32 NumFrames = 60) const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ClearFrameData();

    // Analysis functions
    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsFrameRateStable(float ToleranceMS = 2.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentFPS() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetAverageFPS() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsGPUBottlenecked() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsGameThreadBottlenecked() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsRenderThreadBottlenecked() const;

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetMaxFramesToTrack(int32 MaxFrames);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetAnalysisInterval(float IntervalSeconds);

    // Debug output
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void LogPerformanceReport() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FString GetPerformanceReportString() const;

protected:
    // Frame data storage
    UPROPERTY()
    TArray<FPerf_FrameTimeData> FrameHistory;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    int32 MaxFramesToTrack;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float AnalysisInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    bool bIsAnalyzing;

    // Timing
    UPROPERTY()
    double LastAnalysisTime;

    UPROPERTY()
    double AnalysisStartTime;

private:
    // Internal helper functions
    float CalculateVariance(const TArray<float>& Values, float Mean) const;
    FPerf_FrameTimeData CaptureCurrentFrameData() const;
    void TrimFrameHistory();
};