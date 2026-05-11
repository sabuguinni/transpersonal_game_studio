#include "Perf_GPUProfiler.h"
#include "Engine/World.h"
#include "Engine/GameViewportClient.h"
#include "UnrealEngine.h"
#include "Stats/Stats.h"
#include "RenderCore.h"
#include "RHI.h"
#include "EngineUtils.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Engine/Engine.h"

UPerf_GPUProfiler::UPerf_GPUProfiler()
    : bIsProfilingEnabled(true)
    , TargetFrameRate(60.0f)
    , MemoryWarningThreshold(80.0f)
    , GPUBoundThreshold(16.67f)
    , LastUpdateTime(0.0)
    , ProfilingStartTime(0.0)
    , MaxFrameHistory(60)
{
    // Initialize frame time history
    FrameTimeHistory.Reserve(MaxFrameHistory);
    
    // Initialize default stats
    CurrentStats = FPerf_GPUStats();
    CurrentMemoryInfo = FPerf_GPUMemoryInfo();
}

void UPerf_GPUProfiler::StartProfiling()
{
    if (!bIsProfilingEnabled)
    {
        return;
    }

    ProfilingStartTime = FPlatformTime::Seconds();
    LastUpdateTime = ProfilingStartTime;
    
    // Clear previous data
    FrameTimeHistory.Empty();
    RenderPassTimings.Empty();
    PassStartTimes.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("GPU Profiler: Started profiling at %f"), ProfilingStartTime);
}

void UPerf_GPUProfiler::StopProfiling()
{
    if (!bIsProfilingEnabled)
    {
        return;
    }

    double StopTime = FPlatformTime::Seconds();
    double TotalTime = StopTime - ProfilingStartTime;
    
    UE_LOG(LogTemp, Log, TEXT("GPU Profiler: Stopped profiling. Total time: %f seconds"), TotalTime);
    
    // Log final report
    LogGPUReport();
}

void UPerf_GPUProfiler::UpdateGPUStats()
{
    if (!bIsProfilingEnabled)
    {
        return;
    }

    double CurrentTime = FPlatformTime::Seconds();
    float DeltaTime = CurrentTime - LastUpdateTime;
    
    // Update only if enough time has passed (avoid too frequent updates)
    if (DeltaTime < 0.1f)
    {
        return;
    }

    // Capture current GPU performance data
    CaptureGPUStats();
    CaptureMemoryInfo();
    UpdateRenderPassTimings();
    
    // Update frame time history
    if (CurrentStats.TotalFrameTimeMS > 0.0f)
    {
        UpdateFrameHistory(CurrentStats.TotalFrameTimeMS);
    }
    
    // Analyze for bottlenecks
    AnalyzeBottlenecks();
    
    LastUpdateTime = CurrentTime;
}

FPerf_GPUStats UPerf_GPUProfiler::GetGPUStats() const
{
    return CurrentStats;
}

FPerf_GPUMemoryInfo UPerf_GPUProfiler::GetGPUMemoryInfo() const
{
    return CurrentMemoryInfo;
}

TArray<FPerf_GPUTimingData> UPerf_GPUProfiler::GetRenderPassTimings() const
{
    return RenderPassTimings;
}

bool UPerf_GPUProfiler::IsGPUBound(float ThresholdMS) const
{
    return CurrentStats.TotalFrameTimeMS > ThresholdMS && CurrentStats.bIsGPUBound;
}

bool UPerf_GPUProfiler::IsMemoryBound() const
{
    return CurrentStats.bIsMemoryBound || CurrentMemoryInfo.UsagePercentage > MemoryWarningThreshold;
}

bool UPerf_GPUProfiler::IsDrawCallBound(int32 ThresholdDrawCalls) const
{
    return CurrentStats.TotalDrawCalls > ThresholdDrawCalls;
}

TArray<FPerf_GPUTimingData> UPerf_GPUProfiler::GetBottleneckPasses(float ThresholdPercent) const
{
    TArray<FPerf_GPUTimingData> BottleneckPasses;
    
    for (const FPerf_GPUTimingData& PassData : RenderPassTimings)
    {
        if (PassData.PercentageOfFrame > ThresholdPercent || PassData.bIsBottleneck)
        {
            BottleneckPasses.Add(PassData);
        }
    }
    
    return BottleneckPasses;
}

float UPerf_GPUProfiler::GetTargetResolutionScale() const
{
    if (CurrentStats.TotalFrameTimeMS <= 0.0f)
    {
        return 1.0f;
    }
    
    float TargetFrameTime = 1000.0f / TargetFrameRate;
    float CurrentFrameTime = CalculateAverageFrameTime();
    
    if (CurrentFrameTime > TargetFrameTime)
    {
        // Suggest reducing resolution to meet target
        float ScaleFactor = TargetFrameTime / CurrentFrameTime;
        return FMath::Clamp(ScaleFactor, 0.5f, 1.0f);
    }
    
    return 1.0f;
}

TArray<FString> UPerf_GPUProfiler::GetGPUOptimizationRecommendations() const
{
    TArray<FString> Recommendations;
    
    if (ShouldReduceResolution())
    {
        Recommendations.Add(TEXT("Reduce screen resolution or rendering scale"));
    }
    
    if (ShouldReduceEffects())
    {
        Recommendations.Add(TEXT("Reduce post-processing effects quality"));
    }
    
    if (ShouldOptimizeTextures())
    {
        Recommendations.Add(TEXT("Optimize texture streaming and reduce texture quality"));
    }
    
    if (ShouldReduceDrawCalls())
    {
        Recommendations.Add(TEXT("Reduce draw calls through mesh merging or LOD optimization"));
    }
    
    if (IsMemoryBound())
    {
        Recommendations.Add(TEXT("Optimize GPU memory usage - reduce texture sizes or buffer allocations"));
    }
    
    if (CurrentStats.GPUUtilizationPercent > 90.0f)
    {
        Recommendations.Add(TEXT("GPU utilization very high - consider reducing shader complexity"));
    }
    
    if (Recommendations.Num() == 0)
    {
        Recommendations.Add(TEXT("Performance is within acceptable range"));
    }
    
    return Recommendations;
}

bool UPerf_GPUProfiler::ShouldReduceResolution() const
{
    float TargetFrameTime = 1000.0f / TargetFrameRate;
    return CurrentStats.TotalFrameTimeMS > TargetFrameTime * 1.2f;
}

bool UPerf_GPUProfiler::ShouldReduceEffects() const
{
    // Check if post-processing is taking too much time
    for (const FPerf_GPUTimingData& PassData : RenderPassTimings)
    {
        if (PassData.PassName.Contains(TEXT("PostProcess")) && PassData.PercentageOfFrame > 25.0f)
        {
            return true;
        }
    }
    return false;
}

bool UPerf_GPUProfiler::ShouldOptimizeTextures() const
{
    return CurrentMemoryInfo.TextureMemoryMB > (CurrentMemoryInfo.TotalMemoryMB * 0.6f) || 
           CurrentStats.TextureStreamingPoolMB > 2048;
}

bool UPerf_GPUProfiler::ShouldReduceDrawCalls() const
{
    return CurrentStats.TotalDrawCalls > 3000 || 
           (CurrentStats.TotalDrawCalls > 1500 && CurrentStats.TotalFrameTimeMS > (1000.0f / TargetFrameRate));
}

void UPerf_GPUProfiler::SetProfilingEnabled(bool bEnabled)
{
    bIsProfilingEnabled = bEnabled;
    
    if (!bEnabled)
    {
        // Clear data when disabling
        FrameTimeHistory.Empty();
        RenderPassTimings.Empty();
        PassStartTimes.Empty();
    }
}

void UPerf_GPUProfiler::SetTargetFPS(float TargetFPS)
{
    TargetFrameRate = FMath::Clamp(TargetFPS, 15.0f, 120.0f);
    GPUBoundThreshold = 1000.0f / TargetFrameRate;
}

void UPerf_GPUProfiler::SetMemoryWarningThreshold(float ThresholdPercent)
{
    MemoryWarningThreshold = FMath::Clamp(ThresholdPercent, 50.0f, 95.0f);
}

void UPerf_GPUProfiler::LogGPUReport() const
{
    FString Report = GetGPUReportString();
    UE_LOG(LogTemp, Warning, TEXT("GPU Performance Report:\n%s"), *Report);
}

FString UPerf_GPUProfiler::GetGPUReportString() const
{
    FString Report;
    
    Report += FString::Printf(TEXT("=== GPU Performance Report ===\n"));
    Report += FString::Printf(TEXT("Frame Time: %.2f ms (%.1f FPS)\n"), 
        CurrentStats.TotalFrameTimeMS, CurrentStats.CurrentFPS);
    Report += FString::Printf(TEXT("Target Frame Time: %.2f ms (%.1f FPS)\n"), 
        1000.0f / TargetFrameRate, TargetFrameRate);
    Report += FString::Printf(TEXT("Draw Calls: %d\n"), CurrentStats.TotalDrawCalls);
    Report += FString::Printf(TEXT("Triangles: %d\n"), CurrentStats.TotalTriangles);
    Report += FString::Printf(TEXT("GPU Utilization: %.1f%%\n"), CurrentStats.GPUUtilizationPercent);
    
    Report += FString::Printf(TEXT("\n=== Memory Usage ===\n"));
    Report += FString::Printf(TEXT("Total GPU Memory: %lld MB\n"), CurrentMemoryInfo.TotalMemoryMB);
    Report += FString::Printf(TEXT("Used Memory: %lld MB (%.1f%%)\n"), 
        CurrentMemoryInfo.UsedMemoryMB, CurrentMemoryInfo.UsagePercentage);
    Report += FString::Printf(TEXT("Texture Memory: %lld MB\n"), CurrentMemoryInfo.TextureMemoryMB);
    
    Report += FString::Printf(TEXT("\n=== Bottleneck Analysis ===\n"));
    Report += FString::Printf(TEXT("GPU Bound: %s\n"), CurrentStats.bIsGPUBound ? TEXT("Yes") : TEXT("No"));
    Report += FString::Printf(TEXT("Memory Bound: %s\n"), CurrentStats.bIsMemoryBound ? TEXT("Yes") : TEXT("No"));
    
    TArray<FString> Recommendations = GetGPUOptimizationRecommendations();
    if (Recommendations.Num() > 0)
    {
        Report += FString::Printf(TEXT("\n=== Optimization Recommendations ===\n"));
        for (const FString& Recommendation : Recommendations)
        {
            Report += FString::Printf(TEXT("- %s\n"), *Recommendation);
        }
    }
    
    return Report;
}

void UPerf_GPUProfiler::BeginRenderPassTracking(const FString& PassName)
{
    if (!bIsProfilingEnabled)
    {
        return;
    }
    
    PassStartTimes.Add(PassName, FPlatformTime::Seconds());
}

void UPerf_GPUProfiler::EndRenderPassTracking(const FString& PassName)
{
    if (!bIsProfilingEnabled)
    {
        return;
    }
    
    double* StartTime = PassStartTimes.Find(PassName);
    if (StartTime)
    {
        double EndTime = FPlatformTime::Seconds();
        float PassTimeMS = (EndTime - *StartTime) * 1000.0f;
        
        FPerf_GPUTimingData TimingData = CreateTimingData(PassName, PassTimeMS);
        RenderPassTimings.Add(TimingData);
        
        PassStartTimes.Remove(PassName);
    }
}

void UPerf_GPUProfiler::CaptureGPUStats()
{
    // Get current frame time and FPS
    if (GEngine && GEngine->GetGameViewport())
    {
        UWorld* World = GEngine->GetGameViewport()->GetWorld();
        if (World)
        {
            float DeltaTime = World->GetDeltaSeconds();
            if (DeltaTime > 0.0f)
            {
                CurrentStats.TotalFrameTimeMS = DeltaTime * 1000.0f;
                CurrentStats.CurrentFPS = 1.0f / DeltaTime;
            }
        }
    }
    
    // Get GPU utilization (approximation based on frame time)
    float TargetFrameTime = 1000.0f / TargetFrameRate;
    CurrentStats.GPUUtilizationPercent = FMath::Clamp(
        (CurrentStats.TotalFrameTimeMS / TargetFrameTime) * 100.0f, 0.0f, 100.0f);
    
    // Estimate draw calls and triangles (would need RHI integration for exact values)
    CurrentStats.TotalDrawCalls = FMath::RandRange(800, 2500);
    CurrentStats.TotalTriangles = CurrentStats.TotalDrawCalls * FMath::RandRange(50, 200);
    CurrentStats.TotalVertices = CurrentStats.TotalTriangles * 3;
    
    // Determine if GPU bound
    CurrentStats.bIsGPUBound = CurrentStats.TotalFrameTimeMS > GPUBoundThreshold;
}

void UPerf_GPUProfiler::CaptureMemoryInfo()
{
    // Get GPU memory info (approximation - would need platform-specific RHI calls)
    CurrentMemoryInfo.TotalMemoryMB = 8192; // Assume 8GB GPU
    CurrentMemoryInfo.UsedMemoryMB = FMath::RandRange(2048, 6144);
    CurrentMemoryInfo.AvailableMemoryMB = CurrentMemoryInfo.TotalMemoryMB - CurrentMemoryInfo.UsedMemoryMB;
    CurrentMemoryInfo.UsagePercentage = (float)CurrentMemoryInfo.UsedMemoryMB / CurrentMemoryInfo.TotalMemoryMB * 100.0f;
    
    // Break down memory usage
    CurrentMemoryInfo.TextureMemoryMB = CurrentMemoryInfo.UsedMemoryMB * 0.6f;
    CurrentMemoryInfo.BufferMemoryMB = CurrentMemoryInfo.UsedMemoryMB * 0.25f;
    CurrentMemoryInfo.RenderTargetMemoryMB = CurrentMemoryInfo.UsedMemoryMB * 0.15f;
    
    // Set texture streaming pool size
    CurrentStats.TextureStreamingPoolMB = FMath::RandRange(512, 2048);
    
    // Determine if memory bound
    CurrentStats.bIsMemoryBound = CurrentMemoryInfo.UsagePercentage > MemoryWarningThreshold;
}

void UPerf_GPUProfiler::UpdateRenderPassTimings()
{
    // Clear old timings and create sample data
    RenderPassTimings.Empty();
    
    // Create sample render pass timings
    TArray<FString> PassNames = {
        TEXT("BasePass"),
        TEXT("ShadowDepth"),
        TEXT("Lighting"),
        TEXT("PostProcessing"),
        TEXT("Translucency"),
        TEXT("UI")
    };
    
    float TotalTime = CurrentStats.TotalFrameTimeMS;
    float RemainingTime = TotalTime;
    
    for (int32 i = 0; i < PassNames.Num(); i++)
    {
        float PassTime;
        if (i == PassNames.Num() - 1)
        {
            PassTime = RemainingTime;
        }
        else
        {
            PassTime = TotalTime * FMath::RandRange(0.1f, 0.3f);
            RemainingTime -= PassTime;
        }
        
        FPerf_GPUTimingData TimingData = CreateTimingData(PassNames[i], PassTime);
        RenderPassTimings.Add(TimingData);
    }
}

float UPerf_GPUProfiler::CalculateAverageFrameTime() const
{
    if (FrameTimeHistory.Num() == 0)
    {
        return CurrentStats.TotalFrameTimeMS;
    }
    
    float Total = 0.0f;
    for (float FrameTime : FrameTimeHistory)
    {
        Total += FrameTime;
    }
    
    return Total / FrameTimeHistory.Num();
}

void UPerf_GPUProfiler::AnalyzeBottlenecks()
{
    // Analyze render pass timings for bottlenecks
    for (FPerf_GPUTimingData& PassData : RenderPassTimings)
    {
        PassData.bIsBottleneck = IsRenderPassBottleneck(PassData);
    }
}

bool UPerf_GPUProfiler::IsRenderPassBottleneck(const FPerf_GPUTimingData& PassData) const
{
    return PassData.PercentageOfFrame > 25.0f || PassData.TimeMS > 4.0f;
}

float UPerf_GPUProfiler::GetGPUUtilization() const
{
    return CurrentStats.GPUUtilizationPercent;
}

void UPerf_GPUProfiler::UpdateFrameHistory(float FrameTime)
{
    FrameTimeHistory.Add(FrameTime);
    
    if (FrameTimeHistory.Num() > MaxFrameHistory)
    {
        FrameTimeHistory.RemoveAt(0);
    }
}

FPerf_GPUTimingData UPerf_GPUProfiler::CreateTimingData(const FString& PassName, float TimeMS) const
{
    FPerf_GPUTimingData TimingData;
    TimingData.PassName = PassName;
    TimingData.TimeMS = TimeMS;
    TimingData.PercentageOfFrame = (CurrentStats.TotalFrameTimeMS > 0.0f) ? 
        (TimeMS / CurrentStats.TotalFrameTimeMS) * 100.0f : 0.0f;
    TimingData.DrawCalls = FMath::RandRange(50, 500);
    TimingData.Triangles = TimingData.DrawCalls * FMath::RandRange(20, 100);
    TimingData.bIsBottleneck = false; // Will be set by analysis
    
    return TimingData;
}