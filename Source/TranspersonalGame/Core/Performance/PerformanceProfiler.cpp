#include "PerformanceProfiler.h"
#include "Engine/Engine.h"
#include "HAL/PlatformMemory.h"
#include "Stats/Stats.h"
#include "RenderingThread.h"
#include "Engine/World.h"

UPerformanceProfiler::UPerformanceProfiler()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    bIsProfilingActive = false;
    TargetFPS = 60.0f;
    ProfilingStartTime = 0.0f;
    LastUpdateTime = 0.0f;
    FrameCount = 0;
    
    FrameTimeHistory.Reserve(FRAME_HISTORY_SIZE);
}

void UPerformanceProfiler::BeginPlay()
{
    Super::BeginPlay();
    
    // Auto-start profiling
    StartProfiling();
}

void UPerformanceProfiler::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIsProfilingActive)
    {
        return;
    }
    
    UpdateFrameStats(DeltaTime);
    UpdateMemoryStats();
    UpdateRenderStats();
    
    // Analyze performance every second
    if (GetWorld()->GetTimeSeconds() - LastUpdateTime >= 1.0f)
    {
        AnalyzePerformance();
        LastUpdateTime = GetWorld()->GetTimeSeconds();
    }
}

void UPerformanceProfiler::StartProfiling()
{
    if (bIsProfilingActive)
    {
        return;
    }
    
    bIsProfilingActive = true;
    ProfilingStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    ResetStats();
    
    UE_LOG(LogTemp, Log, TEXT("Performance profiling started"));
}

void UPerformanceProfiler::StopProfiling()
{
    if (!bIsProfilingActive)
    {
        return;
    }
    
    bIsProfilingActive = false;
    UE_LOG(LogTemp, Log, TEXT("Performance profiling stopped"));
}

void UPerformanceProfiler::ResetStats()
{
    FrameStats = FPerf_FrameStats();
    MemoryStats = FPerf_MemoryStats();
    RenderStats = FPerf_RenderStats();
    
    FrameTimeHistory.Empty();
    FrameCount = 0;
    
    UE_LOG(LogTemp, Log, TEXT("Performance stats reset"));
}

void UPerformanceProfiler::UpdateFrameStats(float DeltaTime)
{
    if (DeltaTime <= 0.0f)
    {
        return;
    }
    
    // Calculate current FPS
    FrameStats.CurrentFPS = 1.0f / DeltaTime;
    
    // Add to frame time history
    FrameTimeHistory.Add(DeltaTime);
    if (FrameTimeHistory.Num() > FRAME_HISTORY_SIZE)
    {
        FrameTimeHistory.RemoveAt(0);
    }
    
    // Calculate statistics from history
    if (FrameTimeHistory.Num() > 0)
    {
        float TotalFrameTime = 0.0f;
        float MinTime = FrameTimeHistory[0];
        float MaxTime = FrameTimeHistory[0];
        
        for (float FrameTime : FrameTimeHistory)
        {
            TotalFrameTime += FrameTime;
            MinTime = FMath::Min(MinTime, FrameTime);
            MaxTime = FMath::Max(MaxTime, FrameTime);
        }
        
        FrameStats.AverageFrameTime = TotalFrameTime / FrameTimeHistory.Num();
        FrameStats.MinFrameTime = MinTime;
        FrameStats.MaxFrameTime = MaxTime;
        
        // Count dropped frames (frames that took longer than target)
        float TargetFrameTime = 1.0f / TargetFPS;
        FrameStats.DroppedFrames = 0;
        for (float FrameTime : FrameTimeHistory)
        {
            if (FrameTime > TargetFrameTime * 1.5f) // 50% tolerance
            {
                FrameStats.DroppedFrames++;
            }
        }
    }
    
    FrameCount++;
}

void UPerformanceProfiler::UpdateMemoryStats()
{
    // Get platform memory stats
    FPlatformMemoryStats PlatformStats = FPlatformMemory::GetStats();
    
    MemoryStats.UsedPhysicalMemoryMB = PlatformStats.UsedPhysical / (1024.0f * 1024.0f);
    MemoryStats.UsedVirtualMemoryMB = PlatformStats.UsedVirtual / (1024.0f * 1024.0f);
    MemoryStats.AvailablePhysicalMemoryMB = PlatformStats.AvailablePhysical / (1024.0f * 1024.0f);
    
    // Get GC stats
    MemoryStats.GarbageCollectionCount = GGarbageCollectionCount;
    
    // Estimate active objects (simplified)
    MemoryStats.ActiveObjects = GUObjectArray.GetObjectArrayNumMinusAvailable();
}

void UPerformanceProfiler::UpdateRenderStats()
{
    // Note: These are simplified estimates. Real render stats would require engine modifications
    // or access to internal rendering statistics
    
    if (GetWorld() && GetWorld()->Scene)
    {
        // Estimate visible primitives based on scene complexity
        MemoryStats.ActiveObjects = GUObjectArray.GetObjectArrayNumMinusAvailable();
        RenderStats.VisiblePrimitives = FMath::Min(MemoryStats.ActiveObjects / 10, 10000); // Rough estimate
        
        // Estimate draw calls and triangles based on visible primitives
        RenderStats.DrawCalls = RenderStats.VisiblePrimitives / 2; // Rough estimate
        RenderStats.Triangles = RenderStats.VisiblePrimitives * 100; // Rough estimate
        
        // GPU frame time approximation
        RenderStats.GPUFrameTime = FrameStats.AverageFrameTime * 0.7f; // Assume 70% GPU bound
        RenderStats.RenderThreadTime = FrameStats.AverageFrameTime * 0.5f; // Rough estimate
    }
}

EPerf_PerformanceLevel UPerformanceProfiler::GetCurrentPerformanceLevel() const
{
    float CurrentFPS = FrameStats.CurrentFPS;
    
    if (CurrentFPS >= HIGH_PERFORMANCE_THRESHOLD)
    {
        return EPerf_PerformanceLevel::Ultra;
    }
    else if (CurrentFPS >= MEDIUM_PERFORMANCE_THRESHOLD)
    {
        return EPerf_PerformanceLevel::High;
    }
    else if (CurrentFPS >= LOW_PERFORMANCE_THRESHOLD)
    {
        return EPerf_PerformanceLevel::Medium;
    }
    else
    {
        return EPerf_PerformanceLevel::Low;
    }
}

TArray<FString> UPerformanceProfiler::GetOptimizationRecommendations() const
{
    TArray<FString> Recommendations;
    
    // FPS-based recommendations
    if (FrameStats.CurrentFPS < TargetFPS * 0.8f)
    {
        Recommendations.Add(TEXT("Consider reducing graphics quality settings"));
        Recommendations.Add(TEXT("Enable LOD (Level of Detail) for distant objects"));
        Recommendations.Add(TEXT("Reduce shadow quality or distance"));
    }
    
    // Memory-based recommendations
    if (MemoryStats.UsedPhysicalMemoryMB > 4000.0f) // 4GB threshold
    {
        Recommendations.Add(TEXT("High memory usage detected - consider texture streaming"));
        Recommendations.Add(TEXT("Enable garbage collection more frequently"));
    }
    
    // Rendering-based recommendations
    if (RenderStats.DrawCalls > 2000)
    {
        Recommendations.Add(TEXT("High draw call count - consider mesh batching"));
        Recommendations.Add(TEXT("Use instanced rendering for repeated objects"));
    }
    
    if (RenderStats.Triangles > 1000000) // 1M triangles
    {
        Recommendations.Add(TEXT("High triangle count - optimize mesh complexity"));
        Recommendations.Add(TEXT("Use LOD models for distant objects"));
    }
    
    // Frame time consistency recommendations
    if (FrameStats.MaxFrameTime > FrameStats.AverageFrameTime * 2.0f)
    {
        Recommendations.Add(TEXT("Inconsistent frame times - check for performance spikes"));
        Recommendations.Add(TEXT("Consider spreading heavy operations across multiple frames"));
    }
    
    // If performance is good, suggest quality improvements
    if (FrameStats.CurrentFPS > TargetFPS * 1.2f)
    {
        Recommendations.Add(TEXT("Performance headroom available - consider increasing quality"));
        Recommendations.Add(TEXT("Enable additional visual effects or higher resolution"));
    }
    
    return Recommendations;
}

bool UPerformanceProfiler::IsPerformanceTargetMet() const
{
    return FrameStats.CurrentFPS >= TargetFPS * 0.9f; // 10% tolerance
}

void UPerformanceProfiler::AnalyzePerformance()
{
    // Log current performance state
    UE_LOG(LogTemp, Log, TEXT("Performance Analysis - FPS: %.1f, Memory: %.1f MB, Draw Calls: %d"), 
           FrameStats.CurrentFPS, MemoryStats.UsedPhysicalMemoryMB, RenderStats.DrawCalls);
    
    // Check for performance issues
    if (FrameStats.CurrentFPS < LOW_PERFORMANCE_THRESHOLD)
    {
        UE_LOG(LogTemp, Warning, TEXT("Low performance detected: %.1f FPS"), FrameStats.CurrentFPS);
    }
    
    if (MemoryStats.UsedPhysicalMemoryMB > 6000.0f) // 6GB warning threshold
    {
        UE_LOG(LogTemp, Warning, TEXT("High memory usage: %.1f MB"), MemoryStats.UsedPhysicalMemoryMB);
    }
}

void UPerformanceProfiler::GenerateRecommendations()
{
    TArray<FString> Recommendations = GetOptimizationRecommendations();
    
    for (const FString& Recommendation : Recommendations)
    {
        UE_LOG(LogTemp, Log, TEXT("Performance Recommendation: %s"), *Recommendation);
    }
}