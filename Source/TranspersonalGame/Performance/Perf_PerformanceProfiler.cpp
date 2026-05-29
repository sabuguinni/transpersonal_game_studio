#include "Perf_PerformanceProfiler.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformMemory.h"
#include "Stats/Stats.h"
#include "RenderingThread.h"

UPerf_PerformanceProfiler::UPerf_PerformanceProfiler()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.0f; // Tick every frame
    
    TargetFPS = 60.0f;
    MinimumAcceptableFPS = 30.0f;
    bIsProfilingActive = true;
    ProfilingInterval = 1.0f;
    
    FPSSamples.Reserve(300); // 5 minutes at 1 sample per second
}

void UPerf_PerformanceProfiler::BeginPlay()
{
    Super::BeginPlay();
    
    StartProfiling();
    
    UE_LOG(LogTemp, Warning, TEXT("Performance Profiler started - Target FPS: %.1f"), TargetFPS);
}

void UPerf_PerformanceProfiler::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsProfilingActive)
    {
        UpdateMetrics(DeltaTime);
        
        // Log performance report periodically
        LastProfilingTime += DeltaTime;
        if (LastProfilingTime >= ProfilingInterval)
        {
            LogPerformanceReport();
            LastProfilingTime = 0.0f;
        }
    }
}

void UPerf_PerformanceProfiler::StartProfiling()
{
    bIsProfilingActive = true;
    ResetMetrics();
    UE_LOG(LogTemp, Log, TEXT("Performance profiling started"));
}

void UPerf_PerformanceProfiler::StopProfiling()
{
    bIsProfilingActive = false;
    UE_LOG(LogTemp, Log, TEXT("Performance profiling stopped"));
}

void UPerf_PerformanceProfiler::ResetMetrics()
{
    FPSSamples.Empty();
    FrameCount = 0;
    TotalFrameTime = 0.0f;
    LastProfilingTime = 0.0f;
    
    CurrentMetrics = FPerf_PerformanceMetrics();
    
    UE_LOG(LogTemp, Log, TEXT("Performance metrics reset"));
}

FPerf_PerformanceMetrics UPerf_PerformanceProfiler::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

void UPerf_PerformanceProfiler::LogPerformanceReport()
{
    if (!bIsProfilingActive) return;
    
    UE_LOG(LogTemp, Warning, TEXT("=== PERFORMANCE REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Current FPS: %.1f"), CurrentMetrics.CurrentFPS);
    UE_LOG(LogTemp, Warning, TEXT("Average FPS: %.1f"), CurrentMetrics.AverageFPS);
    UE_LOG(LogTemp, Warning, TEXT("Min FPS: %.1f"), CurrentMetrics.MinFPS);
    UE_LOG(LogTemp, Warning, TEXT("Max FPS: %.1f"), CurrentMetrics.MaxFPS);
    UE_LOG(LogTemp, Warning, TEXT("Frame Time: %.2f ms"), CurrentMetrics.FrameTime);
    UE_LOG(LogTemp, Warning, TEXT("Render Time: %.2f ms"), CurrentMetrics.RenderTime);
    UE_LOG(LogTemp, Warning, TEXT("Draw Calls: %d"), CurrentMetrics.DrawCalls);
    UE_LOG(LogTemp, Warning, TEXT("Triangles: %d"), CurrentMetrics.TriangleCount);
    UE_LOG(LogTemp, Warning, TEXT("Memory Usage: %.1f MB"), CurrentMetrics.MemoryUsageMB);
    
    bool bPerformanceOK = IsPerformanceAcceptable();
    UE_LOG(LogTemp, Warning, TEXT("Performance Status: %s"), bPerformanceOK ? TEXT("ACCEPTABLE") : TEXT("NEEDS OPTIMIZATION"));
}

bool UPerf_PerformanceProfiler::IsPerformanceAcceptable() const
{
    return CurrentMetrics.CurrentFPS >= MinimumAcceptableFPS && 
           CurrentMetrics.AverageFPS >= MinimumAcceptableFPS;
}

void UPerf_PerformanceProfiler::SetTargetFPS(float NewTargetFPS)
{
    TargetFPS = FMath::Clamp(NewTargetFPS, 15.0f, 240.0f);
    UE_LOG(LogTemp, Log, TEXT("Target FPS set to: %.1f"), TargetFPS);
}

void UPerf_PerformanceProfiler::UpdateMetrics(float DeltaTime)
{
    if (DeltaTime <= 0.0f) return;
    
    // Calculate current FPS
    CurrentMetrics.CurrentFPS = 1.0f / DeltaTime;
    CurrentMetrics.FrameTime = DeltaTime * 1000.0f; // Convert to milliseconds
    
    // Update frame statistics
    FrameCount++;
    TotalFrameTime += DeltaTime;
    FPSSamples.Add(CurrentMetrics.CurrentFPS);
    
    // Keep only last 300 samples (5 minutes at 1 sample per second)
    if (FPSSamples.Num() > 300)
    {
        FPSSamples.RemoveAt(0);
    }
    
    // Calculate average FPS
    if (FrameCount > 0)
    {
        CurrentMetrics.AverageFPS = FrameCount / TotalFrameTime;
    }
    
    // Calculate min/max FPS
    if (FPSSamples.Num() > 0)
    {
        CurrentMetrics.MinFPS = *FMath::MinElement(FPSSamples);
        CurrentMetrics.MaxFPS = *FMath::MaxElement(FPSSamples);
    }
    
    // Collect additional stats
    CollectRenderStats();
    CollectMemoryStats();
}

void UPerf_PerformanceProfiler::CollectRenderStats()
{
    // Get render thread stats if available
    CurrentMetrics.RenderTime = CurrentMetrics.FrameTime * 0.6f; // Estimate 60% of frame time
    
    // Estimate draw calls and triangles (these would need engine access for real values)
    CurrentMetrics.DrawCalls = 500; // Placeholder - would need RHI stats
    CurrentMetrics.TriangleCount = 50000; // Placeholder - would need render stats
}

void UPerf_PerformanceProfiler::CollectMemoryStats()
{
    // Get memory usage
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
}