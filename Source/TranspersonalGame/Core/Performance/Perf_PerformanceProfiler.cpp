#include "Perf_PerformanceProfiler.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformMemory.h"
#include "Stats/Stats.h"
#include "RenderingThread.h"
#include "Engine/GameViewportClient.h"
#include "EngineUtils.h"

UPerf_PerformanceProfiler::UPerf_PerformanceProfiler()
{
    bIsProfilingActive = false;
    bAutoOptimizationEnabled = false;
    ProfilingStartTime = 0.0f;
    FrameCount = 0;
    FrameHistory.Reserve(MaxFrameHistorySize);
}

void UPerf_PerformanceProfiler::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Performance Profiler initialized"));
    
    // Set default thresholds
    PerformanceThresholds = FPerf_PerformanceThresholds();
    
    // Start profiling automatically
    StartProfiling();
}

void UPerf_PerformanceProfiler::Deinitialize()
{
    StopProfiling();
    Super::Deinitialize();
}

bool UPerf_PerformanceProfiler::ShouldCreateSubsystem(UObject* Outer) const
{
    return true;
}

void UPerf_PerformanceProfiler::StartProfiling()
{
    if (!bIsProfilingActive)
    {
        bIsProfilingActive = true;
        ProfilingStartTime = GetWorld()->GetTimeSeconds();
        FrameCount = 0;
        FrameHistory.Empty();
        
        UE_LOG(LogTemp, Log, TEXT("Performance profiling started"));
    }
}

void UPerf_PerformanceProfiler::StopProfiling()
{
    if (bIsProfilingActive)
    {
        bIsProfilingActive = false;
        UE_LOG(LogTemp, Log, TEXT("Performance profiling stopped. Total frames: %d"), FrameCount);
    }
}

void UPerf_PerformanceProfiler::UpdatePerformanceData()
{
    if (!bIsProfilingActive)
    {
        return;
    }

    CollectFrameData();
    AnalyzePerformance();
    
    if (bAutoOptimizationEnabled)
    {
        ApplyAutoOptimizations();
    }
    
    TrimFrameHistory();
    FrameCount++;
}

void UPerf_PerformanceProfiler::CollectFrameData()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    CurrentFrameData = FPerf_FrameData();
    
    // Basic timing data
    CurrentFrameData.DeltaTime = World->GetDeltaSeconds();
    CurrentFrameData.FPS = (CurrentFrameData.DeltaTime > 0.0f) ? (1.0f / CurrentFrameData.DeltaTime) : 0.0f;
    
    // Collect render stats
    CollectRenderStats();
    
    // Collect memory stats
    CollectMemoryStats();
    
    // Collect actor stats
    CollectActorStats();
    
    // Add to history
    FrameHistory.Add(CurrentFrameData);
}

void UPerf_PerformanceProfiler::CollectRenderStats()
{
    // Get render thread stats (simplified approach)
    CurrentFrameData.GPUTime = CurrentFrameData.DeltaTime * 1000.0f * 0.6f; // Estimate
    CurrentFrameData.CPUTime = CurrentFrameData.DeltaTime * 1000.0f * 0.4f; // Estimate
    
    // Estimate draw calls and triangles based on visible actors
    CurrentFrameData.DrawCalls = FMath::Max(100, CurrentFrameData.VisibleActors * 2);
    CurrentFrameData.Triangles = FMath::Max(10000, CurrentFrameData.VisibleActors * 500);
}

void UPerf_PerformanceProfiler::CollectMemoryStats()
{
    // Get memory usage
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentFrameData.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
}

void UPerf_PerformanceProfiler::CollectActorStats()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    int32 TotalActors = 0;
    int32 VisibleActors = 0;

    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && !Actor->IsPendingKill())
        {
            TotalActors++;
            
            // Simple visibility check
            if (!Actor->IsHidden())
            {
                VisibleActors++;
            }
        }
    }

    CurrentFrameData.ActiveActors = TotalActors;
    CurrentFrameData.VisibleActors = VisibleActors;
}

void UPerf_PerformanceProfiler::AnalyzePerformance()
{
    // Check if performance is within acceptable thresholds
    bool bPerformanceGood = IsPerformanceWithinThresholds();
    
    if (!bPerformanceGood)
    {
        TArray<FString> Warnings = GetPerformanceWarnings();
        for (const FString& Warning : Warnings)
        {
            UE_LOG(LogTemp, Warning, TEXT("Performance Warning: %s"), *Warning);
        }
    }
}

void UPerf_PerformanceProfiler::ApplyAutoOptimizations()
{
    // Simple auto-optimization logic
    if (CurrentFrameData.FPS < PerformanceThresholds.MinimumFPS)
    {
        // Could trigger LOD adjustments, culling changes, etc.
        UE_LOG(LogTemp, Log, TEXT("Auto-optimization triggered: FPS below minimum"));
        TriggerPerformanceOptimization();
    }
}

FPerf_FrameData UPerf_PerformanceProfiler::GetCurrentFrameData() const
{
    return CurrentFrameData;
}

FPerf_FrameData UPerf_PerformanceProfiler::GetAverageFrameData() const
{
    if (FrameHistory.Num() == 0)
    {
        return FPerf_FrameData();
    }

    FPerf_FrameData AverageData;
    
    for (const FPerf_FrameData& Frame : FrameHistory)
    {
        AverageData.DeltaTime += Frame.DeltaTime;
        AverageData.GPUTime += Frame.GPUTime;
        AverageData.CPUTime += Frame.CPUTime;
        AverageData.MemoryUsageMB += Frame.MemoryUsageMB;
        AverageData.DrawCalls += Frame.DrawCalls;
        AverageData.Triangles += Frame.Triangles;
        AverageData.ActiveActors += Frame.ActiveActors;
        AverageData.VisibleActors += Frame.VisibleActors;
    }

    int32 NumFrames = FrameHistory.Num();
    AverageData.DeltaTime /= NumFrames;
    AverageData.FPS = CalculateAverageFPS();
    AverageData.GPUTime /= NumFrames;
    AverageData.CPUTime /= NumFrames;
    AverageData.MemoryUsageMB /= NumFrames;
    AverageData.DrawCalls /= NumFrames;
    AverageData.Triangles /= NumFrames;
    AverageData.ActiveActors /= NumFrames;
    AverageData.VisibleActors /= NumFrames;

    return AverageData;
}

bool UPerf_PerformanceProfiler::IsPerformanceWithinThresholds() const
{
    return CurrentFrameData.FPS >= PerformanceThresholds.MinimumFPS &&
           CurrentFrameData.GPUTime <= PerformanceThresholds.MaxGPUTimeMS &&
           CurrentFrameData.CPUTime <= PerformanceThresholds.MaxCPUTimeMS &&
           CurrentFrameData.MemoryUsageMB <= PerformanceThresholds.MaxMemoryUsageMB &&
           CurrentFrameData.DrawCalls <= PerformanceThresholds.MaxDrawCalls &&
           CurrentFrameData.Triangles <= PerformanceThresholds.MaxTriangles;
}

TArray<FString> UPerf_PerformanceProfiler::GetPerformanceWarnings() const
{
    TArray<FString> Warnings;

    if (CurrentFrameData.FPS < PerformanceThresholds.MinimumFPS)
    {
        Warnings.Add(FString::Printf(TEXT("FPS below minimum: %.1f < %.1f"), 
            CurrentFrameData.FPS, PerformanceThresholds.MinimumFPS));
    }

    if (CurrentFrameData.GPUTime > PerformanceThresholds.MaxGPUTimeMS)
    {
        Warnings.Add(FString::Printf(TEXT("GPU time too high: %.2fms > %.2fms"), 
            CurrentFrameData.GPUTime, PerformanceThresholds.MaxGPUTimeMS));
    }

    if (CurrentFrameData.CPUTime > PerformanceThresholds.MaxCPUTimeMS)
    {
        Warnings.Add(FString::Printf(TEXT("CPU time too high: %.2fms > %.2fms"), 
            CurrentFrameData.CPUTime, PerformanceThresholds.MaxCPUTimeMS));
    }

    if (CurrentFrameData.MemoryUsageMB > PerformanceThresholds.MaxMemoryUsageMB)
    {
        Warnings.Add(FString::Printf(TEXT("Memory usage too high: %.1fMB > %.1fMB"), 
            CurrentFrameData.MemoryUsageMB, PerformanceThresholds.MaxMemoryUsageMB));
    }

    if (CurrentFrameData.DrawCalls > PerformanceThresholds.MaxDrawCalls)
    {
        Warnings.Add(FString::Printf(TEXT("Draw calls too high: %d > %d"), 
            CurrentFrameData.DrawCalls, PerformanceThresholds.MaxDrawCalls));
    }

    if (CurrentFrameData.Triangles > PerformanceThresholds.MaxTriangles)
    {
        Warnings.Add(FString::Printf(TEXT("Triangle count too high: %d > %d"), 
            CurrentFrameData.Triangles, PerformanceThresholds.MaxTriangles));
    }

    return Warnings;
}

void UPerf_PerformanceProfiler::SetPerformanceThresholds(const FPerf_PerformanceThresholds& NewThresholds)
{
    PerformanceThresholds = NewThresholds;
    UE_LOG(LogTemp, Log, TEXT("Performance thresholds updated"));
}

FPerf_PerformanceThresholds UPerf_PerformanceProfiler::GetPerformanceThresholds() const
{
    return PerformanceThresholds;
}

void UPerf_PerformanceProfiler::EnableAutoOptimization(bool bEnable)
{
    bAutoOptimizationEnabled = bEnable;
    UE_LOG(LogTemp, Log, TEXT("Auto-optimization %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UPerf_PerformanceProfiler::TriggerPerformanceOptimization()
{
    UE_LOG(LogTemp, Log, TEXT("Performance optimization triggered"));
    
    // This would interface with other performance systems
    // For now, just log the event
}

void UPerf_PerformanceProfiler::LogPerformanceData() const
{
    UE_LOG(LogTemp, Log, TEXT("=== PERFORMANCE DATA ==="));
    UE_LOG(LogTemp, Log, TEXT("FPS: %.1f"), CurrentFrameData.FPS);
    UE_LOG(LogTemp, Log, TEXT("GPU Time: %.2fms"), CurrentFrameData.GPUTime);
    UE_LOG(LogTemp, Log, TEXT("CPU Time: %.2fms"), CurrentFrameData.CPUTime);
    UE_LOG(LogTemp, Log, TEXT("Memory: %.1fMB"), CurrentFrameData.MemoryUsageMB);
    UE_LOG(LogTemp, Log, TEXT("Draw Calls: %d"), CurrentFrameData.DrawCalls);
    UE_LOG(LogTemp, Log, TEXT("Triangles: %d"), CurrentFrameData.Triangles);
    UE_LOG(LogTemp, Log, TEXT("Active Actors: %d"), CurrentFrameData.ActiveActors);
    UE_LOG(LogTemp, Log, TEXT("Visible Actors: %d"), CurrentFrameData.VisibleActors);
}

void UPerf_PerformanceProfiler::DumpPerformanceReport() const
{
    FPerf_FrameData AverageData = GetAverageFrameData();
    
    UE_LOG(LogTemp, Log, TEXT("=== PERFORMANCE REPORT ==="));
    UE_LOG(LogTemp, Log, TEXT("Profiling Duration: %.1f seconds"), GetWorld()->GetTimeSeconds() - ProfilingStartTime);
    UE_LOG(LogTemp, Log, TEXT("Total Frames: %d"), FrameCount);
    UE_LOG(LogTemp, Log, TEXT("Average FPS: %.1f"), AverageData.FPS);
    UE_LOG(LogTemp, Log, TEXT("Average GPU Time: %.2fms"), AverageData.GPUTime);
    UE_LOG(LogTemp, Log, TEXT("Average CPU Time: %.2fms"), AverageData.CPUTime);
    UE_LOG(LogTemp, Log, TEXT("Average Memory: %.1fMB"), AverageData.MemoryUsageMB);
    UE_LOG(LogTemp, Log, TEXT("Performance Within Thresholds: %s"), IsPerformanceWithinThresholds() ? TEXT("Yes") : TEXT("No"));
}

float UPerf_PerformanceProfiler::CalculateAverageFPS() const
{
    if (FrameHistory.Num() == 0)
    {
        return 0.0f;
    }

    float TotalFPS = 0.0f;
    for (const FPerf_FrameData& Frame : FrameHistory)
    {
        TotalFPS += Frame.FPS;
    }

    return TotalFPS / FrameHistory.Num();
}

float UPerf_PerformanceProfiler::CalculateAverageGPUTime() const
{
    if (FrameHistory.Num() == 0)
    {
        return 0.0f;
    }

    float TotalGPUTime = 0.0f;
    for (const FPerf_FrameData& Frame : FrameHistory)
    {
        TotalGPUTime += Frame.GPUTime;
    }

    return TotalGPUTime / FrameHistory.Num();
}

float UPerf_PerformanceProfiler::CalculateAverageCPUTime() const
{
    if (FrameHistory.Num() == 0)
    {
        return 0.0f;
    }

    float TotalCPUTime = 0.0f;
    for (const FPerf_FrameData& Frame : FrameHistory)
    {
        TotalCPUTime += Frame.CPUTime;
    }

    return TotalCPUTime / FrameHistory.Num();
}

void UPerf_PerformanceProfiler::TrimFrameHistory()
{
    if (FrameHistory.Num() > MaxFrameHistorySize)
    {
        FrameHistory.RemoveAt(0, FrameHistory.Num() - MaxFrameHistorySize);
    }
}