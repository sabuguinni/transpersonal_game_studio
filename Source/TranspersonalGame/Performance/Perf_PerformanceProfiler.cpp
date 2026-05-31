#include "Perf_PerformanceProfiler.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Stats/Stats.h"
#include "RenderingThread.h"
#include "HAL/PlatformMemory.h"

UPerf_PerformanceProfiler::UPerf_PerformanceProfiler()
{
    bIsProfilingActive = false;
    TargetFPS = 60.0f;
    AverageFPS = 0.0f;
    MinFPS = 999.0f;
    MaxFPS = 0.0f;
    FrameCount = 0;
    TotalFrameTime = 0.0f;
}

void UPerf_PerformanceProfiler::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Performance Profiler initialized"));
    
    // Initialize default metrics
    PerformanceMetrics.Empty();
    
    FPerf_PerformanceMetric FPSMetric;
    FPSMetric.MetricName = TEXT("FPS");
    FPSMetric.Category = EPerf_ProfilerCategory::Rendering;
    PerformanceMetrics.Add(TEXT("FPS"), FPSMetric);
    
    FPerf_PerformanceMetric FrameTimeMetric;
    FrameTimeMetric.MetricName = TEXT("FrameTime");
    FrameTimeMetric.Category = EPerf_ProfilerCategory::Rendering;
    PerformanceMetrics.Add(TEXT("FrameTime"), FrameTimeMetric);
    
    FPerf_PerformanceMetric MemoryMetric;
    MemoryMetric.MetricName = TEXT("MemoryUsage");
    MemoryMetric.Category = EPerf_ProfilerCategory::Memory;
    PerformanceMetrics.Add(TEXT("MemoryUsage"), MemoryMetric);
    
    FPerf_PerformanceMetric DrawCallMetric;
    DrawCallMetric.MetricName = TEXT("DrawCalls");
    DrawCallMetric.Category = EPerf_ProfilerCategory::Rendering;
    PerformanceMetrics.Add(TEXT("DrawCalls"), DrawCallMetric);
}

void UPerf_PerformanceProfiler::Deinitialize()
{
    StopProfiling();
    Super::Deinitialize();
}

void UPerf_PerformanceProfiler::StartProfiling()
{
    if (bIsProfilingActive)
    {
        return;
    }
    
    bIsProfilingActive = true;
    FrameCount = 0;
    TotalFrameTime = 0.0f;
    AverageFPS = 0.0f;
    MinFPS = 999.0f;
    MaxFPS = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Performance profiling started"));
    
    // Start timer for regular updates
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(ProfilingTimerHandle, this, 
            &UPerf_PerformanceProfiler::UpdateFrameData, 0.1f, true);
    }
}

void UPerf_PerformanceProfiler::StopProfiling()
{
    if (!bIsProfilingActive)
    {
        return;
    }
    
    bIsProfilingActive = false;
    
    UE_LOG(LogTemp, Log, TEXT("Performance profiling stopped"));
    
    // Clear timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(ProfilingTimerHandle);
    }
    
    // Calculate final averages
    CalculateAverages();
}

void UPerf_PerformanceProfiler::ResetMetrics()
{
    FrameCount = 0;
    TotalFrameTime = 0.0f;
    AverageFPS = 0.0f;
    MinFPS = 999.0f;
    MaxFPS = 0.0f;
    
    for (auto& MetricPair : PerformanceMetrics)
    {
        MetricPair.Value.CurrentValue = 0.0f;
        MetricPair.Value.AverageValue = 0.0f;
        MetricPair.Value.MinValue = 0.0f;
        MetricPair.Value.MaxValue = 0.0f;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Performance metrics reset"));
}

TArray<FPerf_PerformanceMetric> UPerf_PerformanceProfiler::GetAllMetrics() const
{
    TArray<FPerf_PerformanceMetric> Metrics;
    for (const auto& MetricPair : PerformanceMetrics)
    {
        Metrics.Add(MetricPair.Value);
    }
    return Metrics;
}

FPerf_PerformanceMetric UPerf_PerformanceProfiler::GetMetric(const FString& MetricName) const
{
    if (const FPerf_PerformanceMetric* Metric = PerformanceMetrics.Find(MetricName))
    {
        return *Metric;
    }
    return FPerf_PerformanceMetric();
}

void UPerf_PerformanceProfiler::AddCustomMetric(const FString& MetricName, float Value, EPerf_ProfilerCategory Category)
{
    FPerf_PerformanceMetric* Metric = PerformanceMetrics.Find(MetricName);
    if (!Metric)
    {
        FPerf_PerformanceMetric NewMetric;
        NewMetric.MetricName = MetricName;
        NewMetric.Category = Category;
        NewMetric.CurrentValue = Value;
        NewMetric.MinValue = Value;
        NewMetric.MaxValue = Value;
        NewMetric.AverageValue = Value;
        NewMetric.LastUpdate = FDateTime::Now();
        PerformanceMetrics.Add(MetricName, NewMetric);
    }
    else
    {
        Metric->CurrentValue = Value;
        Metric->MinValue = FMath::Min(Metric->MinValue, Value);
        Metric->MaxValue = FMath::Max(Metric->MaxValue, Value);
        Metric->LastUpdate = FDateTime::Now();
        
        // Simple running average (could be improved with proper sample window)
        static int32 SampleCount = 0;
        SampleCount++;
        Metric->AverageValue = (Metric->AverageValue * (SampleCount - 1) + Value) / SampleCount;
    }
}

void UPerf_PerformanceProfiler::SaveProfileDataToFile(const FString& FileName)
{
    FString ProfileData = GetProfileDataAsString();
    FString FilePath = FPaths::ProjectSavedDir() / TEXT("Performance") / FileName;
    
    if (!FFileHelper::SaveStringToFile(ProfileData, *FilePath))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to save performance data to file: %s"), *FilePath);
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Performance data saved to: %s"), *FilePath);
    }
}

bool UPerf_PerformanceProfiler::IsPerformanceAcceptable() const
{
    if (AverageFPS < TargetFPS * 0.8f) // 80% of target FPS
    {
        return false;
    }
    
    if (CurrentFrameData.FrameTime > (1.0f / TargetFPS) * 1.5f) // 150% of target frame time
    {
        return false;
    }
    
    return true;
}

void UPerf_PerformanceProfiler::UpdateFrameData()
{
    if (!bIsProfilingActive)
    {
        return;
    }
    
    // Get current frame data
    CurrentFrameData.FrameTime = FApp::GetDeltaTime();
    CurrentFrameData.FPS = 1.0f / FMath::Max(CurrentFrameData.FrameTime, 0.001f);
    
    // Update FPS tracking
    TotalFrameTime += CurrentFrameData.FrameTime;
    FrameCount++;
    
    MinFPS = FMath::Min(MinFPS, CurrentFrameData.FPS);
    MaxFPS = FMath::Max(MaxFPS, CurrentFrameData.FPS);
    
    // Get memory usage
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentFrameData.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    
    // Update metrics
    UpdateMetrics();
}

void UPerf_PerformanceProfiler::UpdateMetrics()
{
    // Update FPS metric
    AddCustomMetric(TEXT("FPS"), CurrentFrameData.FPS, EPerf_ProfilerCategory::Rendering);
    
    // Update Frame Time metric
    AddCustomMetric(TEXT("FrameTime"), CurrentFrameData.FrameTime * 1000.0f, EPerf_ProfilerCategory::Rendering);
    
    // Update Memory metric
    AddCustomMetric(TEXT("MemoryUsage"), CurrentFrameData.MemoryUsageMB, EPerf_ProfilerCategory::Memory);
    
    // Update Draw Calls metric (placeholder - would need render thread access for real data)
    AddCustomMetric(TEXT("DrawCalls"), CurrentFrameData.DrawCalls, EPerf_ProfilerCategory::Rendering);
}

void UPerf_PerformanceProfiler::CalculateAverages()
{
    if (FrameCount > 0)
    {
        AverageFPS = FrameCount / TotalFrameTime;
    }
}

FString UPerf_PerformanceProfiler::GetProfileDataAsString() const
{
    FString ProfileData;
    ProfileData += FString::Printf(TEXT("Performance Profile Data - %s\n"), *FDateTime::Now().ToString());
    ProfileData += FString::Printf(TEXT("Target FPS: %.1f\n"), TargetFPS);
    ProfileData += FString::Printf(TEXT("Average FPS: %.1f\n"), AverageFPS);
    ProfileData += FString::Printf(TEXT("Min FPS: %.1f\n"), MinFPS);
    ProfileData += FString::Printf(TEXT("Max FPS: %.1f\n"), MaxFPS);
    ProfileData += FString::Printf(TEXT("Frame Count: %d\n"), FrameCount);
    ProfileData += FString::Printf(TEXT("Total Time: %.2f seconds\n"), TotalFrameTime);
    ProfileData += TEXT("\nDetailed Metrics:\n");
    
    for (const auto& MetricPair : PerformanceMetrics)
    {
        const FPerf_PerformanceMetric& Metric = MetricPair.Value;
        ProfileData += FString::Printf(TEXT("%s: Current=%.2f, Avg=%.2f, Min=%.2f, Max=%.2f\n"),
            *Metric.MetricName, Metric.CurrentValue, Metric.AverageValue, Metric.MinValue, Metric.MaxValue);
    }
    
    return ProfileData;
}