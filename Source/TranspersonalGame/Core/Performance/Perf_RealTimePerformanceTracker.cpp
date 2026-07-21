#include "Perf_RealTimePerformanceTracker.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"
#include "RenderCore.h"
#include "RHI.h"

APerf_RealTimePerformanceTracker::APerf_RealTimePerformanceTracker()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.0f; // Tick every frame for real-time tracking
    
    // Initialize performance targets
    TargetFPS = 60.0f;
    MaxFrameTimeMS = 16.67f; // 1000ms / 60fps
    MaxGPUTimeMS = 12.0f;
    MaxDrawCalls = 2000;
    MaxMemoryUsageMB = 4096.0f;
    
    // Initialize tracking settings
    MetricsUpdateInterval = 0.1f; // Update metrics 10 times per second
    OptimizationCheckInterval = 1.0f; // Check for optimization needs every second
    bAutoOptimize = true;
    bLogPerformanceWarnings = true;
    
    // Initialize tracking state
    bIsTracking = false;
    LastMetricsUpdateTime = 0.0f;
    LastOptimizationCheckTime = 0.0f;
    FrameCounter = 0;
    AccumulatedFrameTime = 0.0f;
    
    // Initialize metrics
    CurrentMetrics = FPerf_RealTimeMetrics();
    
    // Reserve history arrays
    FPSHistory.Reserve(MaxHistorySize);
    FrameTimeHistory.Reserve(MaxHistorySize);
    MemoryHistory.Reserve(MaxHistorySize);
}

void APerf_RealTimePerformanceTracker::BeginPlay()
{
    Super::BeginPlay();
    
    // Start tracking automatically
    StartPerformanceTracking();
    
    UE_LOG(LogTemp, Warning, TEXT("RealTimePerformanceTracker: Started performance tracking"));
}

void APerf_RealTimePerformanceTracker::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (!bIsTracking)
    {
        return;
    }
    
    // Accumulate frame data
    FrameCounter++;
    AccumulatedFrameTime += DeltaTime;
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Update metrics at specified interval
    if (CurrentTime - LastMetricsUpdateTime >= MetricsUpdateInterval)
    {
        UpdatePerformanceMetrics();
        UpdatePerformanceHistory();
        LastMetricsUpdateTime = CurrentTime;
    }
    
    // Check for optimization needs
    if (bAutoOptimize && CurrentTime - LastOptimizationCheckTime >= OptimizationCheckInterval)
    {
        CheckPerformanceTargets();
        LastOptimizationCheckTime = CurrentTime;
    }
}

void APerf_RealTimePerformanceTracker::StartPerformanceTracking()
{
    bIsTracking = true;
    ResetPerformanceMetrics();
    
    UE_LOG(LogTemp, Log, TEXT("RealTimePerformanceTracker: Performance tracking started"));
}

void APerf_RealTimePerformanceTracker::StopPerformanceTracking()
{
    bIsTracking = false;
    
    UE_LOG(LogTemp, Log, TEXT("RealTimePerformanceTracker: Performance tracking stopped"));
}

void APerf_RealTimePerformanceTracker::ResetPerformanceMetrics()
{
    CurrentMetrics = FPerf_RealTimeMetrics();
    FrameCounter = 0;
    AccumulatedFrameTime = 0.0f;
    LastMetricsUpdateTime = 0.0f;
    LastOptimizationCheckTime = 0.0f;
    
    // Clear history
    FPSHistory.Empty();
    FrameTimeHistory.Empty();
    MemoryHistory.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("RealTimePerformanceTracker: Metrics reset"));
}

FPerf_RealTimeMetrics APerf_RealTimePerformanceTracker::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

void APerf_RealTimePerformanceTracker::SetPerformanceTargets(float InTargetFPS, float InMaxFrameTimeMS)
{
    TargetFPS = InTargetFPS;
    MaxFrameTimeMS = InMaxFrameTimeMS;
    
    UE_LOG(LogTemp, Log, TEXT("RealTimePerformanceTracker: Performance targets set - FPS: %.1f, FrameTime: %.2fms"), 
           TargetFPS, MaxFrameTimeMS);
}

bool APerf_RealTimePerformanceTracker::IsPerformanceWithinTargets() const
{
    return CurrentMetrics.bIsPerformanceGood;
}

void APerf_RealTimePerformanceTracker::LogPerformanceReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PERFORMANCE REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Current FPS: %.1f (Target: %.1f)"), CurrentMetrics.CurrentFPS, TargetFPS);
    UE_LOG(LogTemp, Warning, TEXT("Frame Time: %.2fms (Max: %.2fms)"), CurrentMetrics.FrameTimeMS, MaxFrameTimeMS);
    UE_LOG(LogTemp, Warning, TEXT("Game Thread: %.2fms"), CurrentMetrics.GameThreadTimeMS);
    UE_LOG(LogTemp, Warning, TEXT("Render Thread: %.2fms"), CurrentMetrics.RenderThreadTimeMS);
    UE_LOG(LogTemp, Warning, TEXT("GPU Time: %.2fms (Max: %.2fms)"), CurrentMetrics.GPUTimeMS, MaxGPUTimeMS);
    UE_LOG(LogTemp, Warning, TEXT("Draw Calls: %d (Max: %d)"), CurrentMetrics.DrawCalls, MaxDrawCalls);
    UE_LOG(LogTemp, Warning, TEXT("Triangles: %d"), CurrentMetrics.Triangles);
    UE_LOG(LogTemp, Warning, TEXT("Memory Usage: %.1fMB (Max: %.1fMB)"), CurrentMetrics.MemoryUsageMB, MaxMemoryUsageMB);
    UE_LOG(LogTemp, Warning, TEXT("Active Actors: %d"), CurrentMetrics.ActiveActors);
    UE_LOG(LogTemp, Warning, TEXT("Performance Good: %s"), CurrentMetrics.bIsPerformanceGood ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("========================"));
}

void APerf_RealTimePerformanceTracker::TriggerLODOptimization()
{
    UE_LOG(LogTemp, Warning, TEXT("RealTimePerformanceTracker: Triggering LOD optimization"));
    
    // Execute LOD optimization console commands
    if (UWorld* World = GetWorld())
    {
        if (GEngine)
        {
            GEngine->Exec(World, TEXT("r.SkeletalMeshLODBias 1"));
            GEngine->Exec(World, TEXT("r.StaticMeshLODDistanceScale 1.5"));
            GEngine->Exec(World, TEXT("r.ForceLOD 1"));
        }
    }
}

void APerf_RealTimePerformanceTracker::TriggerCullingOptimization()
{
    UE_LOG(LogTemp, Warning, TEXT("RealTimePerformanceTracker: Triggering culling optimization"));
    
    // Execute culling optimization console commands
    if (UWorld* World = GetWorld())
    {
        if (GEngine)
        {
            GEngine->Exec(World, TEXT("r.CullDistanceQuality 3"));
            GEngine->Exec(World, TEXT("r.ViewDistanceScale 0.7"));
            GEngine->Exec(World, TEXT("r.OcclusionCulling 1"));
        }
    }
}

void APerf_RealTimePerformanceTracker::TriggerMemoryCleanup()
{
    UE_LOG(LogTemp, Warning, TEXT("RealTimePerformanceTracker: Triggering memory cleanup"));
    
    // Execute memory cleanup console commands
    if (UWorld* World = GetWorld())
    {
        if (GEngine)
        {
            GEngine->Exec(World, TEXT("gc.CollectGarbageEveryFrame 1"));
            GEngine->Exec(World, TEXT("r.Streaming.PoolSize 1500"));
            GEngine->Exec(World, TEXT("r.Streaming.MaxTempMemoryAllowed 50"));
        }
    }
}

void APerf_RealTimePerformanceTracker::UpdatePerformanceMetrics()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Calculate FPS
    if (AccumulatedFrameTime > 0.0f && FrameCounter > 0)
    {
        CurrentMetrics.CurrentFPS = FrameCounter / AccumulatedFrameTime;
        CurrentMetrics.FrameTimeMS = (AccumulatedFrameTime / FrameCounter) * 1000.0f;
    }
    
    // Get engine stats (simplified approach)
    CurrentMetrics.GameThreadTimeMS = FPlatformTime::ToMilliseconds(GGameThreadTime);
    CurrentMetrics.RenderThreadTimeMS = FPlatformTime::ToMilliseconds(GRenderThreadTime);
    CurrentMetrics.GPUTimeMS = FPlatformTime::ToMilliseconds(GGPUFrameTime);
    
    // Get memory usage (simplified)
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    
    // Count active actors
    CurrentMetrics.ActiveActors = GetWorld()->GetActorCount();
    
    // Estimate draw calls and triangles (simplified)
    CurrentMetrics.DrawCalls = FMath::RandRange(800, 1200); // Placeholder
    CurrentMetrics.Triangles = FMath::RandRange(50000, 150000); // Placeholder
    
    // Check if performance is within targets
    CurrentMetrics.bIsPerformanceGood = 
        CurrentMetrics.CurrentFPS >= TargetFPS * 0.9f && // Allow 10% tolerance
        CurrentMetrics.FrameTimeMS <= MaxFrameTimeMS * 1.1f &&
        CurrentMetrics.GPUTimeMS <= MaxGPUTimeMS &&
        CurrentMetrics.MemoryUsageMB <= MaxMemoryUsageMB;
    
    // Reset frame counters
    FrameCounter = 0;
    AccumulatedFrameTime = 0.0f;
}

void APerf_RealTimePerformanceTracker::CheckPerformanceTargets()
{
    if (!CurrentMetrics.bIsPerformanceGood)
    {
        if (bLogPerformanceWarnings)
        {
            UE_LOG(LogTemp, Warning, TEXT("RealTimePerformanceTracker: Performance below targets!"));
            LogPerformanceReport();
        }
        
        if (bAutoOptimize)
        {
            ExecuteAutoOptimization();
        }
    }
}

void APerf_RealTimePerformanceTracker::UpdatePerformanceHistory()
{
    // Add current metrics to history
    FPSHistory.Add(CurrentMetrics.CurrentFPS);
    FrameTimeHistory.Add(CurrentMetrics.FrameTimeMS);
    MemoryHistory.Add(CurrentMetrics.MemoryUsageMB);
    
    // Maintain history size
    if (FPSHistory.Num() > MaxHistorySize)
    {
        FPSHistory.RemoveAt(0);
        FrameTimeHistory.RemoveAt(0);
        MemoryHistory.RemoveAt(0);
    }
}

float APerf_RealTimePerformanceTracker::CalculateAverageFPS() const
{
    if (FPSHistory.Num() == 0)
    {
        return 0.0f;
    }
    
    float Sum = 0.0f;
    for (float FPS : FPSHistory)
    {
        Sum += FPS;
    }
    
    return Sum / FPSHistory.Num();
}

float APerf_RealTimePerformanceTracker::CalculateAverageFrameTime() const
{
    if (FrameTimeHistory.Num() == 0)
    {
        return 0.0f;
    }
    
    float Sum = 0.0f;
    for (float FrameTime : FrameTimeHistory)
    {
        Sum += FrameTime;
    }
    
    return Sum / FrameTimeHistory.Num();
}

bool APerf_RealTimePerformanceTracker::ShouldTriggerOptimization() const
{
    // Check if performance has been consistently poor
    if (FPSHistory.Num() < 10)
    {
        return false; // Not enough data
    }
    
    float AverageFPS = CalculateAverageFPS();
    float AverageFrameTime = CalculateAverageFrameTime();
    
    return AverageFPS < TargetFPS * 0.8f || AverageFrameTime > MaxFrameTimeMS * 1.2f;
}

void APerf_RealTimePerformanceTracker::ExecuteAutoOptimization()
{
    UE_LOG(LogTemp, Warning, TEXT("RealTimePerformanceTracker: Executing auto-optimization"));
    
    // Determine which optimization to apply based on bottleneck
    if (CurrentMetrics.GPUTimeMS > MaxGPUTimeMS)
    {
        TriggerLODOptimization();
        TriggerCullingOptimization();
    }
    
    if (CurrentMetrics.MemoryUsageMB > MaxMemoryUsageMB * 0.9f)
    {
        TriggerMemoryCleanup();
    }
    
    if (CurrentMetrics.FrameTimeMS > MaxFrameTimeMS)
    {
        TriggerCullingOptimization();
    }
}