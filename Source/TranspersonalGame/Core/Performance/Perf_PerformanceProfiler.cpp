#include "Perf_PerformanceProfiler.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"
#include "RenderingThread.h"
#include "HAL/IConsoleManager.h"

UPerf_PerformanceProfiler::UPerf_PerformanceProfiler()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    bIsProfilingEnabled = true;
    TargetFPS = 60.0f;
    ProfilingUpdateInterval = 0.1f;
    MaxSamples = 1000;
    LastUpdateTime = 0.0f;
    AccumulatedFrameTime = 0.0f;
    FrameCount = 0;
}

void UPerf_PerformanceProfiler::BeginPlay()
{
    Super::BeginPlay();
    
    LastUpdateTime = GetCurrentTime();
    
    if (bIsProfilingEnabled)
    {
        UE_LOG(LogTemp, Log, TEXT("Performance Profiler started - Target FPS: %.1f"), TargetFPS);
    }
}

void UPerf_PerformanceProfiler::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIsProfilingEnabled)
    {
        return;
    }
    
    AccumulatedFrameTime += DeltaTime;
    FrameCount++;
    
    float CurrentTime = GetCurrentTime();
    if (CurrentTime - LastUpdateTime >= ProfilingUpdateInterval)
    {
        UpdateMetrics();
        LastUpdateTime = CurrentTime;
        AccumulatedFrameTime = 0.0f;
        FrameCount = 0;
    }
}

void UPerf_PerformanceProfiler::StartProfiling()
{
    bIsProfilingEnabled = true;
    ClearSamples();
    LastUpdateTime = GetCurrentTime();
    AccumulatedFrameTime = 0.0f;
    FrameCount = 0;
    
    UE_LOG(LogTemp, Log, TEXT("Performance profiling started"));
}

void UPerf_PerformanceProfiler::StopProfiling()
{
    bIsProfilingEnabled = false;
    UE_LOG(LogTemp, Log, TEXT("Performance profiling stopped"));
}

void UPerf_PerformanceProfiler::BeginSample(const FString& SampleName, EPerf_ProfilerCategory Category)
{
    if (!bIsProfilingEnabled)
    {
        return;
    }
    
    float CurrentTime = GetCurrentTime();
    ActiveSamples.Add(SampleName, CurrentTime);
    
    // Create sample entry
    FPerf_ProfilerSample NewSample;
    NewSample.SampleName = SampleName;
    NewSample.Category = Category;
    NewSample.StartTime = CurrentTime;
    NewSample.EndTime = 0.0f;
    NewSample.Duration = 0.0f;
    
    ProfilerSamples.Add(NewSample);
    
    // Limit sample count
    if (ProfilerSamples.Num() > MaxSamples)
    {
        ProfilerSamples.RemoveAt(0, ProfilerSamples.Num() - MaxSamples);
    }
}

void UPerf_PerformanceProfiler::EndSample(const FString& SampleName)
{
    if (!bIsProfilingEnabled)
    {
        return;
    }
    
    float* StartTimePtr = ActiveSamples.Find(SampleName);
    if (!StartTimePtr)
    {
        UE_LOG(LogTemp, Warning, TEXT("EndSample called for unknown sample: %s"), *SampleName);
        return;
    }
    
    float CurrentTime = GetCurrentTime();
    float Duration = CurrentTime - *StartTimePtr;
    
    // Find the corresponding sample in the array and update it
    for (int32 i = ProfilerSamples.Num() - 1; i >= 0; i--)
    {
        if (ProfilerSamples[i].SampleName == SampleName && ProfilerSamples[i].EndTime == 0.0f)
        {
            ProfilerSamples[i].EndTime = CurrentTime;
            ProfilerSamples[i].Duration = Duration;
            break;
        }
    }
    
    ActiveSamples.Remove(SampleName);
}

FPerf_PerformanceMetrics UPerf_PerformanceProfiler::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

TArray<FPerf_ProfilerSample> UPerf_PerformanceProfiler::GetProfilerSamples() const
{
    return ProfilerSamples;
}

void UPerf_PerformanceProfiler::ClearSamples()
{
    ProfilerSamples.Empty();
    ActiveSamples.Empty();
}

void UPerf_PerformanceProfiler::SetTargetFPS(float NewTargetFPS)
{
    TargetFPS = FMath::Clamp(NewTargetFPS, 15.0f, 120.0f);
    UE_LOG(LogTemp, Log, TEXT("Target FPS set to: %.1f"), TargetFPS);
}

bool UPerf_PerformanceProfiler::IsPerformanceWithinTarget() const
{
    return CurrentMetrics.FPS >= (TargetFPS * 0.9f); // Allow 10% tolerance
}

void UPerf_PerformanceProfiler::LogPerformanceReport()
{
    if (!bIsProfilingEnabled)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("=== PERFORMANCE REPORT ==="));
    UE_LOG(LogTemp, Log, TEXT("FPS: %.1f (Target: %.1f)"), CurrentMetrics.FPS, TargetFPS);
    UE_LOG(LogTemp, Log, TEXT("Frame Time: %.2f ms"), CurrentMetrics.FrameTime * 1000.0f);
    UE_LOG(LogTemp, Log, TEXT("CPU Time: %.2f ms"), CurrentMetrics.CPUTime * 1000.0f);
    UE_LOG(LogTemp, Log, TEXT("GPU Time: %.2f ms"), CurrentMetrics.GPUTime * 1000.0f);
    UE_LOG(LogTemp, Log, TEXT("Memory Usage: %.1f MB"), CurrentMetrics.MemoryUsage);
    UE_LOG(LogTemp, Log, TEXT("Draw Calls: %d"), CurrentMetrics.DrawCalls);
    UE_LOG(LogTemp, Log, TEXT("Triangles: %d"), CurrentMetrics.Triangles);
    UE_LOG(LogTemp, Log, TEXT("Performance Within Target: %s"), IsPerformanceWithinTarget() ? TEXT("YES") : TEXT("NO"));
    
    // Log top 10 slowest samples
    TArray<FPerf_ProfilerSample> SortedSamples = ProfilerSamples;
    SortedSamples.Sort([](const FPerf_ProfilerSample& A, const FPerf_ProfilerSample& B) {
        return A.Duration > B.Duration;
    });
    
    UE_LOG(LogTemp, Log, TEXT("=== TOP PERFORMANCE SAMPLES ==="));
    for (int32 i = 0; i < FMath::Min(10, SortedSamples.Num()); i++)
    {
        const FPerf_ProfilerSample& Sample = SortedSamples[i];
        UE_LOG(LogTemp, Log, TEXT("%s: %.2f ms"), *Sample.SampleName, Sample.Duration * 1000.0f);
    }
}

void UPerf_PerformanceProfiler::UpdateMetrics()
{
    if (FrameCount == 0)
    {
        return;
    }
    
    // Calculate average frame time and FPS
    float AverageFrameTime = AccumulatedFrameTime / FrameCount;
    CurrentMetrics.FrameTime = AverageFrameTime;
    CurrentMetrics.FPS = (AverageFrameTime > 0.0f) ? (1.0f / AverageFrameTime) : 0.0f;
    
    // Collect rendering and memory stats
    CollectRenderingStats();
    CollectMemoryStats();
    
    // Log performance warnings
    if (CurrentMetrics.FPS < TargetFPS * 0.8f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Performance below target: %.1f FPS (Target: %.1f)"), 
               CurrentMetrics.FPS, TargetFPS);
    }
}

void UPerf_PerformanceProfiler::CollectRenderingStats()
{
    // Get basic rendering stats
    if (GEngine && GEngine->GetWorld())
    {
        // These are approximate values - in a real implementation you'd use proper stat collection
        CurrentMetrics.CPUTime = CurrentMetrics.FrameTime * 0.6f; // Estimate CPU time as 60% of frame time
        CurrentMetrics.GPUTime = CurrentMetrics.FrameTime * 0.4f; // Estimate GPU time as 40% of frame time
        
        // Estimate draw calls and triangles based on world complexity
        UWorld* World = GEngine->GetWorld();
        if (World)
        {
            int32 ActorCount = World->GetCurrentLevel()->Actors.Num();
            CurrentMetrics.DrawCalls = FMath::Clamp(ActorCount * 2, 100, 5000);
            CurrentMetrics.Triangles = FMath::Clamp(ActorCount * 500, 10000, 1000000);
        }
    }
}

void UPerf_PerformanceProfiler::CollectMemoryStats()
{
    // Get memory usage in MB
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.MemoryUsage = MemStats.UsedPhysical / (1024.0f * 1024.0f);
}

float UPerf_PerformanceProfiler::GetCurrentTime() const
{
    return FPlatformTime::Seconds();
}