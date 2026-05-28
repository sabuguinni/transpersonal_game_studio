#include "EngArch_PerformanceProfiler.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformMemory.h"
#include "Stats/Stats.h"
#include "RenderingThread.h"
#include "Misc/DateTime.h"

UEngArch_PerformanceProfiler::UEngArch_PerformanceProfiler()
{
    bIsProfilingActive = false;
    TargetFrameRate = 60.0f;
    ProfilingInterval = 1.0f;
    LastProfilingTime = 0.0f;
    MaxHistorySize = 300;
}

void UEngArch_PerformanceProfiler::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("EngArch_PerformanceProfiler: Initialized"));
    
    // Start profiling by default
    StartProfiling();
}

void UEngArch_PerformanceProfiler::Deinitialize()
{
    StopProfiling();
    PerformanceHistory.Empty();
    CurrentMetrics.Empty();
    
    Super::Deinitialize();
}

void UEngArch_PerformanceProfiler::Tick(float DeltaTime)
{
    if (!bIsProfilingActive)
    {
        return;
    }
    
    float CurrentTime = FPlatformTime::Seconds();
    if (CurrentTime - LastProfilingTime >= ProfilingInterval)
    {
        UpdatePerformanceMetrics();
        AnalyzePerformance();
        CheckPerformanceThresholds();
        LastProfilingTime = CurrentTime;
    }
}

void UEngArch_PerformanceProfiler::StartProfiling()
{
    if (bIsProfilingActive)
    {
        return;
    }
    
    bIsProfilingActive = true;
    LastProfilingTime = FPlatformTime::Seconds();
    PerformanceHistory.Empty();
    CurrentMetrics.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("EngArch_PerformanceProfiler: Started profiling"));
}

void UEngArch_PerformanceProfiler::StopProfiling()
{
    if (!bIsProfilingActive)
    {
        return;
    }
    
    bIsProfilingActive = false;
    
    UE_LOG(LogTemp, Log, TEXT("EngArch_PerformanceProfiler: Stopped profiling"));
}

FEng_PerformanceSnapshot UEngArch_PerformanceProfiler::GetCurrentSnapshot()
{
    FEng_PerformanceSnapshot Snapshot;
    Snapshot.Timestamp = FPlatformTime::Seconds();
    
    // Get frame rate and timing
    if (GEngine && GEngine->GetGameViewport())
    {
        Snapshot.FrameRate = 1.0f / FApp::GetDeltaTime();
        Snapshot.FrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds
    }
    
    // Get CPU time (approximate)
    Snapshot.CPUTime = FApp::GetDeltaTime() * 1000.0f;
    
    // Get GPU time (if available)
    Snapshot.GPUTime = 0.0f; // Would need RHI integration for accurate GPU timing
    
    // Get rendering stats (approximate)
    Snapshot.DrawCalls = 0; // Would need rendering thread integration
    Snapshot.Triangles = 0; // Would need rendering thread integration
    
    // Get memory usage
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    Snapshot.MemoryUsage = MemStats.UsedPhysical / (1024.0f * 1024.0f); // Convert to MB
    
    return Snapshot;
}

TArray<FEng_PerformanceMetric> UEngArch_PerformanceProfiler::GetPerformanceMetrics()
{
    return CurrentMetrics;
}

float UEngArch_PerformanceProfiler::GetAverageFrameRate() const
{
    return CalculateAverageFromHistory([](const FEng_PerformanceSnapshot& Snapshot) { return Snapshot.FrameRate; });
}

float UEngArch_PerformanceProfiler::GetAverageFrameTime() const
{
    return CalculateAverageFromHistory([](const FEng_PerformanceSnapshot& Snapshot) { return Snapshot.FrameTime; });
}

bool UEngArch_PerformanceProfiler::IsPerformanceAcceptable() const
{
    float AvgFrameRate = GetAverageFrameRate();
    float AvgFrameTime = GetAverageFrameTime();
    
    // Check if we're meeting target frame rate (within 10% tolerance)
    bool bFrameRateOK = AvgFrameRate >= (TargetFrameRate * 0.9f);
    
    // Check if frame time is reasonable (less than 20ms for 50+ FPS)
    bool bFrameTimeOK = AvgFrameTime <= 20.0f;
    
    return bFrameRateOK && bFrameTimeOK;
}

TArray<FString> UEngArch_PerformanceProfiler::GetPerformanceWarnings()
{
    TArray<FString> Warnings;
    
    float AvgFrameRate = GetAverageFrameRate();
    float AvgFrameTime = GetAverageFrameTime();
    
    if (AvgFrameRate < TargetFrameRate * 0.8f)
    {
        Warnings.Add(FString::Printf(TEXT("Frame rate below target: %.1f FPS (target: %.1f FPS)"), AvgFrameRate, TargetFrameRate));
    }
    
    if (AvgFrameTime > 33.0f) // 30 FPS threshold
    {
        Warnings.Add(FString::Printf(TEXT("High frame time: %.2f ms"), AvgFrameTime));
    }
    
    // Check memory usage
    if (PerformanceHistory.Num() > 0)
    {
        float MemoryUsage = PerformanceHistory.Last().MemoryUsage;
        if (MemoryUsage > 4096.0f) // 4GB threshold
        {
            Warnings.Add(FString::Printf(TEXT("High memory usage: %.1f MB"), MemoryUsage));
        }
    }
    
    return Warnings;
}

void UEngArch_PerformanceProfiler::LogPerformanceReport()
{
    UE_LOG(LogTemp, Log, TEXT("=== PERFORMANCE PROFILER REPORT ==="));
    UE_LOG(LogTemp, Log, TEXT("Profiling Active: %s"), bIsProfilingActive ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Log, TEXT("Target Frame Rate: %.1f FPS"), TargetFrameRate);
    UE_LOG(LogTemp, Log, TEXT("Profiling Interval: %.2f seconds"), ProfilingInterval);
    UE_LOG(LogTemp, Log, TEXT("History Size: %d snapshots"), PerformanceHistory.Num());
    
    if (PerformanceHistory.Num() > 0)
    {
        float AvgFrameRate = GetAverageFrameRate();
        float AvgFrameTime = GetAverageFrameTime();
        
        UE_LOG(LogTemp, Log, TEXT("Average Frame Rate: %.2f FPS"), AvgFrameRate);
        UE_LOG(LogTemp, Log, TEXT("Average Frame Time: %.2f ms"), AvgFrameTime);
        
        FEng_PerformanceSnapshot LastSnapshot = PerformanceHistory.Last();
        UE_LOG(LogTemp, Log, TEXT("Current Memory Usage: %.1f MB"), LastSnapshot.MemoryUsage);
        
        bool bAcceptable = IsPerformanceAcceptable();
        UE_LOG(LogTemp, Log, TEXT("Performance Acceptable: %s"), bAcceptable ? TEXT("YES") : TEXT("NO"));
        
        TArray<FString> Warnings = GetPerformanceWarnings();
        if (Warnings.Num() > 0)
        {
            UE_LOG(LogTemp, Warning, TEXT("Performance Warnings:"));
            for (const FString& Warning : Warnings)
            {
                UE_LOG(LogTemp, Warning, TEXT("  %s"), *Warning);
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Current Metrics: %d"), CurrentMetrics.Num());
    for (const FEng_PerformanceMetric& Metric : CurrentMetrics)
    {
        UE_LOG(LogTemp, Log, TEXT("  %s: %.2f %s"), *Metric.MetricName, Metric.CurrentValue, *Metric.Unit);
    }
    
    UE_LOG(LogTemp, Log, TEXT("==================================="));
}

void UEngArch_PerformanceProfiler::SetTargetFrameRate(float TargetFPS)
{
    TargetFrameRate = FMath::Clamp(TargetFPS, 30.0f, 120.0f);
    UE_LOG(LogTemp, Log, TEXT("EngArch_PerformanceProfiler: Target frame rate set to %.1f FPS"), TargetFrameRate);
}

void UEngArch_PerformanceProfiler::SetProfilingInterval(float IntervalSeconds)
{
    ProfilingInterval = FMath::Clamp(IntervalSeconds, 0.1f, 10.0f);
    UE_LOG(LogTemp, Log, TEXT("EngArch_PerformanceProfiler: Profiling interval set to %.2f seconds"), ProfilingInterval);
}

void UEngArch_PerformanceProfiler::UpdatePerformanceMetrics()
{
    FEng_PerformanceSnapshot Snapshot = GetCurrentSnapshot();
    AddPerformanceSnapshot(Snapshot);
    
    // Update current metrics
    CurrentMetrics.Empty();
    
    CurrentMetrics.Add(CreateMetric(EEng_PerformanceCategory::Rendering, TEXT("Frame Rate"), Snapshot.FrameRate, TEXT("FPS")));
    CurrentMetrics.Add(CreateMetric(EEng_PerformanceCategory::Rendering, TEXT("Frame Time"), Snapshot.FrameTime, TEXT("ms")));
    CurrentMetrics.Add(CreateMetric(EEng_PerformanceCategory::CPU, TEXT("CPU Time"), Snapshot.CPUTime, TEXT("ms")));
    CurrentMetrics.Add(CreateMetric(EEng_PerformanceCategory::Memory, TEXT("Memory Usage"), Snapshot.MemoryUsage, TEXT("MB")));
}

void UEngArch_PerformanceProfiler::AnalyzePerformance()
{
    // This method can be expanded for more sophisticated performance analysis
    // For now, it just updates the metrics
}

void UEngArch_PerformanceProfiler::CheckPerformanceThresholds()
{
    TArray<FString> Warnings = GetPerformanceWarnings();
    
    for (const FString& Warning : Warnings)
    {
        OnPerformanceWarning.Broadcast(Warning);
        
        // Check for critical performance issues
        if (Warning.Contains(TEXT("Frame rate below")) && GetAverageFrameRate() < TargetFrameRate * 0.5f)
        {
            OnPerformanceCritical.Broadcast(FString::Printf(TEXT("CRITICAL: %s"), *Warning));
        }
    }
}

FEng_PerformanceMetric UEngArch_PerformanceProfiler::CreateMetric(EEng_PerformanceCategory Category, const FString& Name, float Value, const FString& Unit)
{
    FEng_PerformanceMetric Metric;
    Metric.Category = Category;
    Metric.MetricName = Name;
    Metric.CurrentValue = Value;
    Metric.Unit = Unit;
    
    // Calculate average from history
    if (PerformanceHistory.Num() > 1)
    {
        if (Name == TEXT("Frame Rate"))
        {
            Metric.AverageValue = GetAverageFrameRate();
        }
        else if (Name == TEXT("Frame Time"))
        {
            Metric.AverageValue = GetAverageFrameTime();
        }
        else
        {
            Metric.AverageValue = Value; // Fallback to current value
        }
    }
    else
    {
        Metric.AverageValue = Value;
    }
    
    // Set warning/critical flags
    if (Category == EEng_PerformanceCategory::Rendering && Name == TEXT("Frame Rate"))
    {
        Metric.bIsWarning = Value < TargetFrameRate * 0.8f;
        Metric.bIsCritical = Value < TargetFrameRate * 0.5f;
    }
    else if (Category == EEng_PerformanceCategory::Rendering && Name == TEXT("Frame Time"))
    {
        Metric.bIsWarning = Value > 20.0f;
        Metric.bIsCritical = Value > 50.0f;
    }
    
    return Metric;
}

void UEngArch_PerformanceProfiler::AddPerformanceSnapshot(const FEng_PerformanceSnapshot& Snapshot)
{
    PerformanceHistory.Add(Snapshot);
    
    // Maintain history size limit
    if (PerformanceHistory.Num() > MaxHistorySize)
    {
        PerformanceHistory.RemoveAt(0);
    }
}

float UEngArch_PerformanceProfiler::CalculateAverageFromHistory(TFunction<float(const FEng_PerformanceSnapshot&)> Extractor) const
{
    if (PerformanceHistory.Num() == 0)
    {
        return 0.0f;
    }
    
    float Sum = 0.0f;
    for (const FEng_PerformanceSnapshot& Snapshot : PerformanceHistory)
    {
        Sum += Extractor(Snapshot);
    }
    
    return Sum / PerformanceHistory.Num();
}