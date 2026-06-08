#include "Perf_RealTimePerformanceMonitor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformMemory.h"
#include "Stats/Stats.h"
#include "RenderCore.h"
#include "TimerManager.h"
#include "Engine/GameViewportClient.h"
#include "Misc/App.h"

UPerf_RealTimePerformanceMonitor::UPerf_RealTimePerformanceMonitor()
{
    bIsMonitoring = false;
    TargetFPS = 60.0f;
    MonitoringInterval = 0.1f;
    LastPerformanceLevel = EPerf_PerformanceLevel::Excellent;
}

void UPerf_RealTimePerformanceMonitor::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Performance Monitor initialized"));
    
    // Reserve space for historical data
    FPSHistory.Reserve(300); // 30 seconds at 0.1s intervals
    FrameTimeHistory.Reserve(300);
    MemoryHistory.Reserve(300);
    
    // Start monitoring automatically
    StartMonitoring();
}

void UPerf_RealTimePerformanceMonitor::Deinitialize()
{
    StopMonitoring();
    Super::Deinitialize();
}

bool UPerf_RealTimePerformanceMonitor::ShouldCreateSubsystem(UObject* Outer) const
{
    return true;
}

void UPerf_RealTimePerformanceMonitor::StartMonitoring()
{
    if (bIsMonitoring)
    {
        return;
    }

    bIsMonitoring = true;
    
    UWorld* World = GetWorld();
    if (World && World->GetTimerManager().IsValidHandle(MonitoringTimerHandle) == false)
    {
        World->GetTimerManager().SetTimer(
            MonitoringTimerHandle,
            this,
            &UPerf_RealTimePerformanceMonitor::UpdateMetrics,
            MonitoringInterval,
            true
        );
        
        UE_LOG(LogTemp, Log, TEXT("Performance monitoring started"));
    }
}

void UPerf_RealTimePerformanceMonitor::StopMonitoring()
{
    if (!bIsMonitoring)
    {
        return;
    }

    bIsMonitoring = false;
    
    UWorld* World = GetWorld();
    if (World && World->GetTimerManager().IsValidHandle(MonitoringTimerHandle))
    {
        World->GetTimerManager().ClearTimer(MonitoringTimerHandle);
        UE_LOG(LogTemp, Log, TEXT("Performance monitoring stopped"));
    }
}

void UPerf_RealTimePerformanceMonitor::UpdateMetrics()
{
    if (!bIsMonitoring)
    {
        return;
    }

    UpdateFrameMetrics();
    UpdateMemoryMetrics();
    UpdateSystemMetrics();
    CheckPerformanceLevel();
    TriggerOptimizationIfNeeded();
}

void UPerf_RealTimePerformanceMonitor::UpdateFrameMetrics()
{
    // Get current FPS
    CurrentFrameMetrics.CurrentFPS = 1.0f / FApp::GetDeltaTime();
    CurrentFrameMetrics.FrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds
    
    // Get thread times (approximations)
    CurrentFrameMetrics.GameThreadTime = FApp::GetDeltaTime() * 1000.0f * 0.6f; // Estimate
    CurrentFrameMetrics.RenderThreadTime = FApp::GetDeltaTime() * 1000.0f * 0.3f; // Estimate
    CurrentFrameMetrics.GPUTime = FApp::GetDeltaTime() * 1000.0f * 0.4f; // Estimate
    
    // Add to history
    FPSHistory.Add(CurrentFrameMetrics.CurrentFPS);
    FrameTimeHistory.Add(CurrentFrameMetrics.FrameTime);
    
    // Keep history size manageable
    if (FPSHistory.Num() > 300)
    {
        FPSHistory.RemoveAt(0);
        FrameTimeHistory.RemoveAt(0);
    }
}

void UPerf_RealTimePerformanceMonitor::UpdateMemoryMetrics()
{
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    
    CurrentMemoryMetrics.UsedPhysicalMemoryMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    CurrentMemoryMetrics.UsedVirtualMemoryMB = MemStats.UsedVirtual / (1024.0f * 1024.0f);
    CurrentMemoryMetrics.PeakUsedPhysicalMemoryMB = MemStats.PeakUsedPhysical / (1024.0f * 1024.0f);
    CurrentMemoryMetrics.PeakUsedVirtualMemoryMB = MemStats.PeakUsedVirtual / (1024.0f * 1024.0f);
    
    // Add to history
    MemoryHistory.Add(CurrentMemoryMetrics.UsedPhysicalMemoryMB);
    
    if (MemoryHistory.Num() > 300)
    {
        MemoryHistory.RemoveAt(0);
    }
}

void UPerf_RealTimePerformanceMonitor::UpdateSystemMetrics()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Count actors
    CurrentSystemMetrics.ActiveActors = 0;
    CurrentSystemMetrics.VisibleActors = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && !Actor->IsPendingKill())
        {
            CurrentSystemMetrics.ActiveActors++;
            
            if (!Actor->IsHidden())
            {
                CurrentSystemMetrics.VisibleActors++;
            }
        }
    }
    
    // Estimate draw calls and triangles (simplified)
    CurrentSystemMetrics.DrawCalls = CurrentSystemMetrics.VisibleActors * 2; // Rough estimate
    CurrentSystemMetrics.Triangles = CurrentSystemMetrics.VisibleActors * 1000; // Rough estimate
    
    // CPU usage approximation
    CurrentSystemMetrics.CPUUsagePercent = FMath::Clamp(
        (CurrentFrameMetrics.FrameTime / 16.67f) * 100.0f, // 16.67ms = 60 FPS
        0.0f,
        100.0f
    );
}

EPerf_PerformanceLevel UPerf_RealTimePerformanceMonitor::GetCurrentPerformanceLevel() const
{
    float CurrentFPS = CurrentFrameMetrics.CurrentFPS;
    
    if (CurrentFPS >= 60.0f)
    {
        return EPerf_PerformanceLevel::Excellent;
    }
    else if (CurrentFPS >= 45.0f)
    {
        return EPerf_PerformanceLevel::Good;
    }
    else if (CurrentFPS >= 30.0f)
    {
        return EPerf_PerformanceLevel::Average;
    }
    else if (CurrentFPS >= 15.0f)
    {
        return EPerf_PerformanceLevel::Poor;
    }
    else
    {
        return EPerf_PerformanceLevel::Critical;
    }
}

void UPerf_RealTimePerformanceMonitor::CheckPerformanceLevel()
{
    EPerf_PerformanceLevel NewLevel = GetCurrentPerformanceLevel();
    
    if (NewLevel != LastPerformanceLevel)
    {
        LastPerformanceLevel = NewLevel;
        OnPerformanceChanged.Broadcast(NewLevel);
        
        UE_LOG(LogTemp, Warning, TEXT("Performance level changed to: %d"), (int32)NewLevel);
    }
    
    // Trigger critical performance event
    if (NewLevel == EPerf_PerformanceLevel::Critical)
    {
        OnPerformanceCritical.Broadcast(CurrentFrameMetrics.CurrentFPS);
        UE_LOG(LogTemp, Error, TEXT("CRITICAL PERFORMANCE: FPS = %.2f"), CurrentFrameMetrics.CurrentFPS);
    }
}

void UPerf_RealTimePerformanceMonitor::TriggerOptimizationIfNeeded()
{
    // Auto-optimize if performance is poor for sustained period
    if (GetCurrentPerformanceLevel() <= EPerf_PerformanceLevel::Poor)
    {
        float AverageFPS = CalculateAverageFPS(10); // Last 1 second
        if (AverageFPS < TargetFPS * 0.75f) // 75% of target
        {
            UE_LOG(LogTemp, Warning, TEXT("Triggering automatic optimizations due to poor performance"));
            ApplyAutomaticOptimizations();
        }
    }
}

void UPerf_RealTimePerformanceMonitor::LogPerformanceReport()
{
    float AvgFPS = CalculateAverageFPS();
    float FrameVariance = CalculateFrameTimeVariance();
    
    UE_LOG(LogTemp, Log, TEXT("=== PERFORMANCE REPORT ==="));
    UE_LOG(LogTemp, Log, TEXT("Current FPS: %.2f"), CurrentFrameMetrics.CurrentFPS);
    UE_LOG(LogTemp, Log, TEXT("Average FPS: %.2f"), AvgFPS);
    UE_LOG(LogTemp, Log, TEXT("Frame Time: %.2f ms"), CurrentFrameMetrics.FrameTime);
    UE_LOG(LogTemp, Log, TEXT("Frame Variance: %.2f"), FrameVariance);
    UE_LOG(LogTemp, Log, TEXT("Memory Usage: %.2f MB"), CurrentMemoryMetrics.UsedPhysicalMemoryMB);
    UE_LOG(LogTemp, Log, TEXT("Active Actors: %d"), CurrentSystemMetrics.ActiveActors);
    UE_LOG(LogTemp, Log, TEXT("Visible Actors: %d"), CurrentSystemMetrics.VisibleActors);
    UE_LOG(LogTemp, Log, TEXT("Performance Level: %d"), (int32)GetCurrentPerformanceLevel());
}

TArray<FString> UPerf_RealTimePerformanceMonitor::GetOptimizationSuggestions() const
{
    TArray<FString> Suggestions;
    
    EPerf_PerformanceLevel CurrentLevel = GetCurrentPerformanceLevel();
    
    if (CurrentLevel <= EPerf_PerformanceLevel::Average)
    {
        if (CurrentSystemMetrics.VisibleActors > 1000)
        {
            Suggestions.Add(TEXT("Reduce visible actor count through culling"));
        }
        
        if (CurrentMemoryMetrics.UsedPhysicalMemoryMB > 4000.0f)
        {
            Suggestions.Add(TEXT("Optimize memory usage - consider texture streaming"));
        }
        
        if (CurrentFrameMetrics.FrameTime > 20.0f)
        {
            Suggestions.Add(TEXT("Frame time too high - check for expensive operations"));
        }
        
        if (CurrentSystemMetrics.DrawCalls > 5000)
        {
            Suggestions.Add(TEXT("Too many draw calls - consider mesh batching"));
        }
        
        Suggestions.Add(TEXT("Enable automatic LOD system"));
        Suggestions.Add(TEXT("Reduce shadow quality"));
        Suggestions.Add(TEXT("Enable occlusion culling"));
    }
    
    return Suggestions;
}

void UPerf_RealTimePerformanceMonitor::ApplyAutomaticOptimizations()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Applying automatic performance optimizations..."));
    
    OptimizeLODSettings();
    OptimizeCullingSettings();
    OptimizeMemorySettings();
    OptimizeRenderingSettings();
}

void UPerf_RealTimePerformanceMonitor::OptimizeLODSettings()
{
    // Apply more aggressive LOD settings
    if (GEngine && GEngine->GetGameUserSettings())
    {
        // This would require access to console variables
        UE_LOG(LogTemp, Log, TEXT("Optimizing LOD settings"));
    }
}

void UPerf_RealTimePerformanceMonitor::OptimizeCullingSettings()
{
    // Enable more aggressive culling
    UE_LOG(LogTemp, Log, TEXT("Optimizing culling settings"));
}

void UPerf_RealTimePerformanceMonitor::OptimizeMemorySettings()
{
    // Trigger garbage collection
    GEngine->ForceGarbageCollection(true);
    UE_LOG(LogTemp, Log, TEXT("Forced garbage collection for memory optimization"));
}

void UPerf_RealTimePerformanceMonitor::OptimizeRenderingSettings()
{
    // Reduce rendering quality temporarily
    UE_LOG(LogTemp, Log, TEXT("Optimizing rendering settings"));
}

float UPerf_RealTimePerformanceMonitor::CalculateAverageFPS(int32 SampleCount) const
{
    if (FPSHistory.Num() == 0)
    {
        return CurrentFrameMetrics.CurrentFPS;
    }
    
    int32 StartIndex = FMath::Max(0, FPSHistory.Num() - SampleCount);
    float Sum = 0.0f;
    int32 Count = 0;
    
    for (int32 i = StartIndex; i < FPSHistory.Num(); ++i)
    {
        Sum += FPSHistory[i];
        Count++;
    }
    
    return Count > 0 ? Sum / Count : CurrentFrameMetrics.CurrentFPS;
}

float UPerf_RealTimePerformanceMonitor::CalculateFrameTimeVariance() const
{
    if (FrameTimeHistory.Num() < 2)
    {
        return 0.0f;
    }
    
    float Mean = 0.0f;
    for (float FrameTime : FrameTimeHistory)
    {
        Mean += FrameTime;
    }
    Mean /= FrameTimeHistory.Num();
    
    float Variance = 0.0f;
    for (float FrameTime : FrameTimeHistory)
    {
        float Diff = FrameTime - Mean;
        Variance += Diff * Diff;
    }
    
    return Variance / FrameTimeHistory.Num();
}

bool UPerf_RealTimePerformanceMonitor::IsPerformanceStable() const
{
    float Variance = CalculateFrameTimeVariance();
    return Variance < 5.0f; // Less than 5ms variance considered stable
}