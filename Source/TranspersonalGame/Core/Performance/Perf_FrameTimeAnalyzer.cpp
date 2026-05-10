#include "Perf_FrameTimeAnalyzer.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"
#include "RenderCore.h"
#include "RHI.h"

UPerf_FrameTimeAnalyzer::UPerf_FrameTimeAnalyzer()
    : MaxFramesToTrack(300)  // 5 seconds at 60fps
    , AnalysisInterval(0.1f)  // Update 10 times per second
    , bIsAnalyzing(false)
    , LastAnalysisTime(0.0)
    , AnalysisStartTime(0.0)
{
    FrameHistory.Reserve(MaxFramesToTrack);
}

void UPerf_FrameTimeAnalyzer::StartAnalysis()
{
    if (bIsAnalyzing)
    {
        UE_LOG(LogTemp, Warning, TEXT("FrameTimeAnalyzer: Analysis already running"));
        return;
    }

    bIsAnalyzing = true;
    AnalysisStartTime = FPlatformTime::Seconds();
    LastAnalysisTime = AnalysisStartTime;
    ClearFrameData();

    UE_LOG(LogTemp, Log, TEXT("FrameTimeAnalyzer: Started frame time analysis"));
}

void UPerf_FrameTimeAnalyzer::StopAnalysis()
{
    if (!bIsAnalyzing)
    {
        UE_LOG(LogTemp, Warning, TEXT("FrameTimeAnalyzer: No analysis running"));
        return;
    }

    bIsAnalyzing = false;
    double AnalysisDuration = FPlatformTime::Seconds() - AnalysisStartTime;
    
    UE_LOG(LogTemp, Log, TEXT("FrameTimeAnalyzer: Stopped analysis after %.2f seconds, %d frames recorded"), 
           AnalysisDuration, FrameHistory.Num());
    
    LogPerformanceReport();
}

void UPerf_FrameTimeAnalyzer::RecordFrame()
{
    if (!bIsAnalyzing)
    {
        return;
    }

    double CurrentTime = FPlatformTime::Seconds();
    if (CurrentTime - LastAnalysisTime < AnalysisInterval)
    {
        return;
    }

    FPerf_FrameTimeData FrameData = CaptureCurrentFrameData();
    FrameHistory.Add(FrameData);
    
    TrimFrameHistory();
    LastAnalysisTime = CurrentTime;
}

FPerf_FrameTimeData UPerf_FrameTimeAnalyzer::CaptureCurrentFrameData() const
{
    FPerf_FrameTimeData FrameData;
    
    // Get delta time from engine
    if (GEngine && GEngine->GetWorldContexts().Num() > 0)
    {
        UWorld* World = GEngine->GetWorldContexts()[0].World();
        if (World)
        {
            FrameData.DeltaTime = World->GetDeltaSeconds();
        }
    }

    // Get timing information from stats
    FrameData.GameThreadTime = FPlatformTime::ToMilliseconds(GGameThreadTime);
    FrameData.RenderThreadTime = FPlatformTime::ToMilliseconds(GRenderThreadTime);
    
    // GPU time approximation
    if (GEngine && GEngine->GetEngineStats().IsValid())
    {
        // Use render thread time as GPU time approximation if GPU stats not available
        FrameData.GPUTime = FrameData.RenderThreadTime;
    }
    else
    {
        FrameData.GPUTime = FrameData.RenderThreadTime;
    }

    FrameData.Timestamp = FPlatformTime::Seconds();
    
    return FrameData;
}

void UPerf_FrameTimeAnalyzer::TrimFrameHistory()
{
    while (FrameHistory.Num() > MaxFramesToTrack)
    {
        FrameHistory.RemoveAt(0);
    }
}

FPerf_FrameTimeStats UPerf_FrameTimeAnalyzer::GetFrameTimeStats() const
{
    FPerf_FrameTimeStats Stats;
    
    if (FrameHistory.Num() == 0)
    {
        return Stats;
    }

    TArray<float> FrameTimes;
    FrameTimes.Reserve(FrameHistory.Num());
    
    float TotalFrameTime = 0.0f;
    float MinTime = FLT_MAX;
    float MaxTime = 0.0f;
    int32 FramesBelow30 = 0;
    int32 FramesBelow60 = 0;

    for (const FPerf_FrameTimeData& Frame : FrameHistory)
    {
        float FrameTimeMS = Frame.DeltaTime * 1000.0f;
        FrameTimes.Add(FrameTimeMS);
        TotalFrameTime += FrameTimeMS;
        
        MinTime = FMath::Min(MinTime, FrameTimeMS);
        MaxTime = FMath::Max(MaxTime, FrameTimeMS);
        
        // Count frames below target framerates
        if (FrameTimeMS > 33.33f) // Below 30 FPS
        {
            FramesBelow30++;
        }
        if (FrameTimeMS > 16.67f) // Below 60 FPS
        {
            FramesBelow60++;
        }
    }

    int32 NumFrames = FrameHistory.Num();
    Stats.AverageFrameTime = TotalFrameTime / NumFrames;
    Stats.MinFrameTime = MinTime;
    Stats.MaxFrameTime = MaxTime;
    Stats.FrameTimeVariance = CalculateVariance(FrameTimes, Stats.AverageFrameTime);
    Stats.FramesBelow30FPS = FramesBelow30;
    Stats.FramesBelow60FPS = FramesBelow60;
    Stats.PercentFramesBelow30FPS = (float)FramesBelow30 / NumFrames * 100.0f;
    Stats.PercentFramesBelow60FPS = (float)FramesBelow60 / NumFrames * 100.0f;

    return Stats;
}

float UPerf_FrameTimeAnalyzer::CalculateVariance(const TArray<float>& Values, float Mean) const
{
    if (Values.Num() <= 1)
    {
        return 0.0f;
    }

    float SumSquaredDiffs = 0.0f;
    for (float Value : Values)
    {
        float Diff = Value - Mean;
        SumSquaredDiffs += Diff * Diff;
    }

    return SumSquaredDiffs / (Values.Num() - 1);
}

TArray<FPerf_FrameTimeData> UPerf_FrameTimeAnalyzer::GetRecentFrameData(int32 NumFrames) const
{
    TArray<FPerf_FrameTimeData> RecentFrames;
    
    int32 StartIndex = FMath::Max(0, FrameHistory.Num() - NumFrames);
    for (int32 i = StartIndex; i < FrameHistory.Num(); i++)
    {
        RecentFrames.Add(FrameHistory[i]);
    }
    
    return RecentFrames;
}

void UPerf_FrameTimeAnalyzer::ClearFrameData()
{
    FrameHistory.Empty();
    FrameHistory.Reserve(MaxFramesToTrack);
}

bool UPerf_FrameTimeAnalyzer::IsFrameRateStable(float ToleranceMS) const
{
    if (FrameHistory.Num() < 10)
    {
        return false;
    }

    FPerf_FrameTimeStats Stats = GetFrameTimeStats();
    return FMath::Sqrt(Stats.FrameTimeVariance) <= ToleranceMS;
}

float UPerf_FrameTimeAnalyzer::GetCurrentFPS() const
{
    if (FrameHistory.Num() == 0)
    {
        return 0.0f;
    }

    const FPerf_FrameTimeData& LastFrame = FrameHistory.Last();
    if (LastFrame.DeltaTime > 0.0f)
    {
        return 1.0f / LastFrame.DeltaTime;
    }
    
    return 0.0f;
}

float UPerf_FrameTimeAnalyzer::GetAverageFPS() const
{
    FPerf_FrameTimeStats Stats = GetFrameTimeStats();
    if (Stats.AverageFrameTime > 0.0f)
    {
        return 1000.0f / Stats.AverageFrameTime;
    }
    
    return 0.0f;
}

bool UPerf_FrameTimeAnalyzer::IsGPUBottlenecked() const
{
    if (FrameHistory.Num() < 5)
    {
        return false;
    }

    // Check if GPU time is consistently higher than game thread time
    int32 GPUBottleneckFrames = 0;
    for (const FPerf_FrameTimeData& Frame : FrameHistory)
    {
        if (Frame.GPUTime > Frame.GameThreadTime * 1.2f) // 20% threshold
        {
            GPUBottleneckFrames++;
        }
    }

    return (float)GPUBottleneckFrames / FrameHistory.Num() > 0.7f; // 70% of frames
}

bool UPerf_FrameTimeAnalyzer::IsGameThreadBottlenecked() const
{
    if (FrameHistory.Num() < 5)
    {
        return false;
    }

    // Check if game thread time is consistently higher than render thread time
    int32 GameThreadBottleneckFrames = 0;
    for (const FPerf_FrameTimeData& Frame : FrameHistory)
    {
        if (Frame.GameThreadTime > Frame.RenderThreadTime * 1.2f && 
            Frame.GameThreadTime > Frame.GPUTime * 1.2f)
        {
            GameThreadBottleneckFrames++;
        }
    }

    return (float)GameThreadBottleneckFrames / FrameHistory.Num() > 0.7f;
}

bool UPerf_FrameTimeAnalyzer::IsRenderThreadBottlenecked() const
{
    if (FrameHistory.Num() < 5)
    {
        return false;
    }

    // Check if render thread time is consistently higher than game thread time
    int32 RenderThreadBottleneckFrames = 0;
    for (const FPerf_FrameTimeData& Frame : FrameHistory)
    {
        if (Frame.RenderThreadTime > Frame.GameThreadTime * 1.2f && 
            Frame.RenderThreadTime > Frame.GPUTime * 0.8f) // Render thread often close to GPU
        {
            RenderThreadBottleneckFrames++;
        }
    }

    return (float)RenderThreadBottleneckFrames / FrameHistory.Num() > 0.7f;
}

void UPerf_FrameTimeAnalyzer::SetMaxFramesToTrack(int32 MaxFrames)
{
    MaxFramesToTrack = FMath::Max(1, MaxFrames);
    TrimFrameHistory();
}

void UPerf_FrameTimeAnalyzer::SetAnalysisInterval(float IntervalSeconds)
{
    AnalysisInterval = FMath::Max(0.01f, IntervalSeconds);
}

void UPerf_FrameTimeAnalyzer::LogPerformanceReport() const
{
    FString Report = GetPerformanceReportString();
    UE_LOG(LogTemp, Log, TEXT("Performance Report:\n%s"), *Report);
}

FString UPerf_FrameTimeAnalyzer::GetPerformanceReportString() const
{
    FPerf_FrameTimeStats Stats = GetFrameTimeStats();
    
    FString Report = FString::Printf(TEXT(
        "=== FRAME TIME ANALYSIS REPORT ===\n"
        "Frames Analyzed: %d\n"
        "Average FPS: %.1f\n"
        "Average Frame Time: %.2f ms\n"
        "Min Frame Time: %.2f ms\n"
        "Max Frame Time: %.2f ms\n"
        "Frame Time Variance: %.2f\n"
        "Frame Rate Stability: %s\n"
        "Frames Below 30 FPS: %d (%.1f%%)\n"
        "Frames Below 60 FPS: %d (%.1f%%)\n"
        "\n=== BOTTLENECK ANALYSIS ===\n"
        "GPU Bottlenecked: %s\n"
        "Game Thread Bottlenecked: %s\n"
        "Render Thread Bottlenecked: %s\n"
        "Current FPS: %.1f\n"),
        FrameHistory.Num(),
        GetAverageFPS(),
        Stats.AverageFrameTime,
        Stats.MinFrameTime,
        Stats.MaxFrameTime,
        Stats.FrameTimeVariance,
        IsFrameRateStable() ? TEXT("STABLE") : TEXT("UNSTABLE"),
        Stats.FramesBelow30FPS,
        Stats.PercentFramesBelow30FPS,
        Stats.FramesBelow60FPS,
        Stats.PercentFramesBelow60FPS,
        IsGPUBottlenecked() ? TEXT("YES") : TEXT("NO"),
        IsGameThreadBottlenecked() ? TEXT("YES") : TEXT("NO"),
        IsRenderThreadBottlenecked() ? TEXT("YES") : TEXT("NO"),
        GetCurrentFPS()
    );

    return Report;
}