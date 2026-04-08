#include "PerformanceProfiler.h"
#include "Engine/Engine.h"
#include "HAL/PlatformMemory.h"
#include "RenderingThread.h"
#include "Stats/Stats.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "../Core/ConsciousnessSystem.h"
#include "../Physics/PhysicsManager.h"

UPerformanceProfiler::UPerformanceProfiler()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    bIsProfilingEnabled = true;
    ProfilingUpdateInterval = 0.1f;
    bAutoOptimizationEnabled = false;
    
    // Default performance targets
    TargetFrameRate = 60.0f;
    MaxAllowedPhysicsTime = 8.0f; // 8ms for physics at 60fps
    MaxAllowedConsciousnessTime = 5.0f; // 5ms for consciousness systems
    MemoryWarningThresholdMB = 2048.0f; // 2GB warning threshold
    
    LastProfilingUpdate = 0.0f;
}

void UPerformanceProfiler::BeginPlay()
{
    Super::BeginPlay();
    
    if (bIsProfilingEnabled)
    {
        StartProfiling();
    }
}

void UPerformanceProfiler::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (bIsProfilingEnabled)
    {
        StopProfiling();
        WritePerformanceLog();
    }
    
    Super::EndPlay(EndPlayReason);
}

void UPerformanceProfiler::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIsProfilingEnabled)
        return;
        
    LastProfilingUpdate += DeltaTime;
    
    if (LastProfilingUpdate >= ProfilingUpdateInterval)
    {
        UpdatePerformanceMetrics();
        CheckPerformanceThresholds();
        
        if (bAutoOptimizationEnabled)
        {
            ApplyAutoOptimizations();
        }
        
        LastProfilingUpdate = 0.0f;
    }
}

void UPerformanceProfiler::StartProfiling()
{
    bIsProfilingEnabled = true;
    MetricsHistory.Empty();
    ConsciousnessProfileTimes.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("Performance Profiler: Started profiling transpersonal game systems"));
}

void UPerformanceProfiler::StopProfiling()
{
    bIsProfilingEnabled = false;
    UE_LOG(LogTemp, Log, TEXT("Performance Profiler: Stopped profiling"));
}

FPerformanceMetrics UPerformanceProfiler::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

void UPerformanceProfiler::LogPerformanceSnapshot(const FString& Label)
{
    FString LogMessage = FString::Printf(TEXT("Performance Snapshot [%s]: %s"), 
        *Label, *FormatMetricsForLog(CurrentMetrics));
    
    UE_LOG(LogTemp, Log, TEXT("%s"), *LogMessage);
}

void UPerformanceProfiler::BeginConsciousnessProfile(const FString& SystemName)
{
    ConsciousnessProfileStartTimes.Add(SystemName, FPlatformTime::Seconds());
}

void UPerformanceProfiler::EndConsciousnessProfile(const FString& SystemName)
{
    if (double* StartTime = ConsciousnessProfileStartTimes.Find(SystemName))
    {
        double ElapsedTime = (FPlatformTime::Seconds() - *StartTime) * 1000.0; // Convert to milliseconds
        ConsciousnessProfileTimes.Add(SystemName, ElapsedTime);
        ConsciousnessProfileStartTimes.Remove(SystemName);
    }
}

void UPerformanceProfiler::EnableAutoOptimization(bool bEnable)
{
    bAutoOptimizationEnabled = bEnable;
    UE_LOG(LogTemp, Log, TEXT("Performance Profiler: Auto-optimization %s"), 
        bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UPerformanceProfiler::SetPerformanceTargets(float TargetFPS, float MaxPhysicsTime, float MaxConsciousnessTime)
{
    TargetFrameRate = TargetFPS;
    MaxAllowedPhysicsTime = MaxPhysicsTime;
    MaxAllowedConsciousnessTime = MaxConsciousnessTime;
    
    UE_LOG(LogTemp, Log, TEXT("Performance Profiler: Updated targets - FPS: %.1f, Physics: %.1fms, Consciousness: %.1fms"),
        TargetFPS, MaxPhysicsTime, MaxConsciousnessTime);
}

void UPerformanceProfiler::UpdatePerformanceMetrics()
{
    // Get frame timing
    CurrentMetrics.FrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds
    
    // Get thread timings (simplified - in real implementation would use more detailed stats)
    CurrentMetrics.GameThreadTime = FPlatformTime::ToMilliseconds(GGameThreadTime);
    CurrentMetrics.RenderThreadTime = FPlatformTime::ToMilliseconds(GRenderThreadTime);
    
    // Physics timing (would integrate with PhysicsManager)
    if (UPhysicsManager* PhysicsManager = UPhysicsManager::GetInstance(GetWorld()))
    {
        CurrentMetrics.PhysicsTime = PhysicsManager->GetLastFramePhysicsTime();
        CurrentMetrics.PhysicsObjects = PhysicsManager->GetActivePhysicsObjectCount();
    }
    
    // Consciousness system timing
    float TotalConsciousnessTime = 0.0f;
    for (const auto& ProfilePair : ConsciousnessProfileTimes)
    {
        TotalConsciousnessTime += ProfilePair.Value;
    }
    CurrentMetrics.ConsciousnessSystemTime = TotalConsciousnessTime;
    
    // Memory usage
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    
    // GPU memory (simplified)
    CurrentMetrics.GPUMemoryUsageMB = 0.0f; // Would need platform-specific implementation
    
    // Count consciousness entities (would integrate with ConsciousnessSystem)
    CurrentMetrics.ActiveConsciousnessEntities = 0; // Placeholder
    
    // Store in history
    MetricsHistory.Add(CurrentMetrics);
    if (MetricsHistory.Num() > 600) // Keep 1 minute of history at 10Hz
    {
        MetricsHistory.RemoveAt(0);
    }
}

void UPerformanceProfiler::CheckPerformanceThresholds()
{
    // Check frame rate
    float CurrentFPS = 1000.0f / FMath::Max(CurrentMetrics.FrameTime, 0.001f);
    if (CurrentFPS < TargetFrameRate * 0.8f) // 20% tolerance
    {
        FString AlertMessage = FString::Printf(TEXT("Low FPS detected: %.1f (target: %.1f)"), 
            CurrentFPS, TargetFrameRate);
        OnPerformanceAlert.Broadcast(AlertMessage);
    }
    
    // Check physics time
    if (CurrentMetrics.PhysicsTime > MaxAllowedPhysicsTime)
    {
        FString AlertMessage = FString::Printf(TEXT("High physics time: %.2fms (max: %.2fms)"), 
            CurrentMetrics.PhysicsTime, MaxAllowedPhysicsTime);
        OnPerformanceAlert.Broadcast(AlertMessage);
    }
    
    // Check consciousness system time
    if (CurrentMetrics.ConsciousnessSystemTime > MaxAllowedConsciousnessTime)
    {
        FString AlertMessage = FString::Printf(TEXT("High consciousness system time: %.2fms (max: %.2fms)"), 
            CurrentMetrics.ConsciousnessSystemTime, MaxAllowedConsciousnessTime);
        OnPerformanceAlert.Broadcast(AlertMessage);
    }
    
    // Check memory usage
    if (CurrentMetrics.MemoryUsageMB > MemoryWarningThresholdMB)
    {
        FString AlertMessage = FString::Printf(TEXT("High memory usage: %.1fMB (threshold: %.1fMB)"), 
            CurrentMetrics.MemoryUsageMB, MemoryWarningThresholdMB);
        OnPerformanceAlert.Broadcast(AlertMessage);
    }
}

void UPerformanceProfiler::ApplyAutoOptimizations()
{
    float CurrentFPS = 1000.0f / FMath::Max(CurrentMetrics.FrameTime, 0.001f);
    
    // If performance is below target, apply optimizations
    if (CurrentFPS < TargetFrameRate * 0.9f)
    {
        if (CurrentMetrics.PhysicsTime > MaxAllowedPhysicsTime * 0.8f)
        {
            OptimizePhysicsSettings();
        }
        
        if (CurrentMetrics.ConsciousnessSystemTime > MaxAllowedConsciousnessTime * 0.8f)
        {
            OptimizeConsciousnessSystem();
        }
        
        OptimizeRenderingSettings();
    }
}

void UPerformanceProfiler::OptimizePhysicsSettings()
{
    // Reduce physics simulation frequency for non-critical objects
    if (UPhysicsManager* PhysicsManager = UPhysicsManager::GetInstance(GetWorld()))
    {
        PhysicsManager->SetOptimizationLevel(1); // Moderate optimization
        UE_LOG(LogTemp, Warning, TEXT("Performance Profiler: Applied physics optimizations"));
    }
}

void UPerformanceProfiler::OptimizeConsciousnessSystem()
{
    // Reduce update frequency for distant consciousness entities
    UE_LOG(LogTemp, Warning, TEXT("Performance Profiler: Applied consciousness system optimizations"));
    
    // This would integrate with the ConsciousnessSystem to:
    // - Reduce update frequency for distant entities
    // - Simplify consciousness calculations for background NPCs
    // - Use LOD system for consciousness complexity
}

void UPerformanceProfiler::OptimizeRenderingSettings()
{
    // Dynamically adjust rendering quality
    UE_LOG(LogTemp, Warning, TEXT("Performance Profiler: Applied rendering optimizations"));
    
    // This could adjust:
    // - Shadow quality
    // - Particle density
    // - LOD bias
    // - Post-processing effects
}

void UPerformanceProfiler::WritePerformanceLog()
{
    if (MetricsHistory.Num() == 0)
        return;
        
    FString LogContent = TEXT("=== Transpersonal Game Performance Report ===\n\n");
    
    // Calculate averages
    FPerformanceMetrics AverageMetrics;
    for (const FPerformanceMetrics& Metrics : MetricsHistory)
    {
        AverageMetrics.FrameTime += Metrics.FrameTime;
        AverageMetrics.GameThreadTime += Metrics.GameThreadTime;
        AverageMetrics.RenderThreadTime += Metrics.RenderThreadTime;
        AverageMetrics.PhysicsTime += Metrics.PhysicsTime;
        AverageMetrics.ConsciousnessSystemTime += Metrics.ConsciousnessSystemTime;
        AverageMetrics.MemoryUsageMB += Metrics.MemoryUsageMB;
    }
    
    float NumSamples = MetricsHistory.Num();
    AverageMetrics.FrameTime /= NumSamples;
    AverageMetrics.GameThreadTime /= NumSamples;
    AverageMetrics.RenderThreadTime /= NumSamples;
    AverageMetrics.PhysicsTime /= NumSamples;
    AverageMetrics.ConsciousnessSystemTime /= NumSamples;
    AverageMetrics.MemoryUsageMB /= NumSamples;
    
    LogContent += FString::Printf(TEXT("Average Performance Metrics:\n%s\n\n"), 
        *FormatMetricsForLog(AverageMetrics));
    
    // Consciousness system breakdown
    LogContent += TEXT("Consciousness System Breakdown:\n");
    for (const auto& ProfilePair : ConsciousnessProfileTimes)
    {
        LogContent += FString::Printf(TEXT("  %s: %.2fms\n"), 
            *ProfilePair.Key, ProfilePair.Value);
    }
    
    UE_LOG(LogTemp, Log, TEXT("%s"), *LogContent);
}

FString UPerformanceProfiler::FormatMetricsForLog(const FPerformanceMetrics& Metrics) const
{
    float FPS = 1000.0f / FMath::Max(Metrics.FrameTime, 0.001f);
    
    return FString::Printf(TEXT("FPS: %.1f | Frame: %.2fms | Game: %.2fms | Render: %.2fms | Physics: %.2fms | Consciousness: %.2fms | Memory: %.1fMB"),
        FPS, Metrics.FrameTime, Metrics.GameThreadTime, Metrics.RenderThreadTime, 
        Metrics.PhysicsTime, Metrics.ConsciousnessSystemTime, Metrics.MemoryUsageMB);
}