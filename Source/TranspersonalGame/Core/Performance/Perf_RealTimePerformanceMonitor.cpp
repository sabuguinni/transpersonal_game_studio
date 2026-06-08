#include "Perf_RealTimePerformanceMonitor.h"
#include "Engine/World.h"
#include "Engine/GameViewportClient.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"
#include "RenderCore.h"
#include "RHI.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

UPerf_RealTimePerformanceMonitor::UPerf_RealTimePerformanceMonitor()
    : bIsMonitoring(false)
    , CurrentPerformanceLevel(EPerf_PerformanceLevel::Good)
    , TargetFPS(60.0f)
    , bAutoQualityAdjustment(true)
    , MonitoringInterval(0.5f)
    , PerformanceHistorySize(120)
    , QualityAdjustmentCooldown(0)
    , ConsecutivePoorFrames(0)
    , ConsecutiveGoodFrames(0)
{
}

void UPerf_RealTimePerformanceMonitor::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Performance Monitor: Initialized"));
    
    // Reserve space for performance history
    PerformanceHistory.Reserve(PerformanceHistorySize);
    
    // Start monitoring by default
    StartMonitoring();
}

void UPerf_RealTimePerformanceMonitor::Deinitialize()
{
    StopMonitoring();
    Super::Deinitialize();
}

void UPerf_RealTimePerformanceMonitor::StartMonitoring()
{
    if (bIsMonitoring)
    {
        return;
    }
    
    bIsMonitoring = true;
    
    // Get world for timer manager
    UWorld* World = GetWorld();
    if (World)
    {
        // Start the monitoring timer
        World->GetTimerManager().SetTimer(
            MonitoringTimerHandle,
            this,
            &UPerf_RealTimePerformanceMonitor::UpdatePerformanceMetrics,
            MonitoringInterval,
            true
        );
        
        UE_LOG(LogTemp, Log, TEXT("Performance Monitor: Started monitoring"));
    }
}

void UPerf_RealTimePerformanceMonitor::StopMonitoring()
{
    if (!bIsMonitoring)
    {
        return;
    }
    
    bIsMonitoring = false;
    
    // Get world for timer manager
    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().ClearTimer(MonitoringTimerHandle);
        UE_LOG(LogTemp, Log, TEXT("Performance Monitor: Stopped monitoring"));
    }
}

FPerf_PerformanceSnapshot UPerf_RealTimePerformanceMonitor::GetCurrentSnapshot() const
{
    FPerf_PerformanceSnapshot Snapshot;
    
    Snapshot.CurrentFPS = CalculateCurrentFPS();
    Snapshot.FrameTimeMS = CalculateFrameTime();
    Snapshot.GPUTimeMS = CalculateGPUTime();
    Snapshot.MemoryUsageMB = CalculateMemoryUsage();
    Snapshot.DrawCalls = CalculateDrawCalls();
    Snapshot.TriangleCount = CalculateTriangleCount();
    Snapshot.TimeStamp = FPlatformTime::Seconds();
    
    return Snapshot;
}

float UPerf_RealTimePerformanceMonitor::GetAverageFPS() const
{
    if (PerformanceHistory.Num() == 0)
    {
        return 0.0f;
    }
    
    float TotalFPS = 0.0f;
    for (const FPerf_PerformanceSnapshot& Snapshot : PerformanceHistory)
    {
        TotalFPS += Snapshot.CurrentFPS;
    }
    
    return TotalFPS / PerformanceHistory.Num();
}

float UPerf_RealTimePerformanceMonitor::GetAverageFrameTime() const
{
    if (PerformanceHistory.Num() == 0)
    {
        return 0.0f;
    }
    
    float TotalFrameTime = 0.0f;
    for (const FPerf_PerformanceSnapshot& Snapshot : PerformanceHistory)
    {
        TotalFrameTime += Snapshot.FrameTimeMS;
    }
    
    return TotalFrameTime / PerformanceHistory.Num();
}

void UPerf_RealTimePerformanceMonitor::SetTargetFPS(float NewTargetFPS)
{
    TargetFPS = FMath::Clamp(NewTargetFPS, 15.0f, 120.0f);
    UE_LOG(LogTemp, Log, TEXT("Performance Monitor: Target FPS set to %.1f"), TargetFPS);
}

void UPerf_RealTimePerformanceMonitor::EnableAutoQualityAdjustment(bool bEnable)
{
    bAutoQualityAdjustment = bEnable;
    UE_LOG(LogTemp, Log, TEXT("Performance Monitor: Auto quality adjustment %s"), 
           bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UPerf_RealTimePerformanceMonitor::LogCurrentPerformance()
{
    FPerf_PerformanceSnapshot Snapshot = GetCurrentSnapshot();
    
    UE_LOG(LogTemp, Warning, TEXT("=== PERFORMANCE SNAPSHOT ==="));
    UE_LOG(LogTemp, Warning, TEXT("FPS: %.1f"), Snapshot.CurrentFPS);
    UE_LOG(LogTemp, Warning, TEXT("Frame Time: %.2f ms"), Snapshot.FrameTimeMS);
    UE_LOG(LogTemp, Warning, TEXT("GPU Time: %.2f ms"), Snapshot.GPUTimeMS);
    UE_LOG(LogTemp, Warning, TEXT("Memory: %.1f MB"), Snapshot.MemoryUsageMB);
    UE_LOG(LogTemp, Warning, TEXT("Draw Calls: %d"), Snapshot.DrawCalls);
    UE_LOG(LogTemp, Warning, TEXT("Triangles: %d"), Snapshot.TriangleCount);
    UE_LOG(LogTemp, Warning, TEXT("Performance Level: %s"), 
           *UEnum::GetValueAsString(CurrentPerformanceLevel));
}

void UPerf_RealTimePerformanceMonitor::ResetPerformanceHistory()
{
    PerformanceHistory.Empty();
    ConsecutivePoorFrames = 0;
    ConsecutiveGoodFrames = 0;
    QualityAdjustmentCooldown = 0;
    UE_LOG(LogTemp, Log, TEXT("Performance Monitor: History reset"));
}

void UPerf_RealTimePerformanceMonitor::UpdatePerformanceMetrics()
{
    // Get current performance snapshot
    FPerf_PerformanceSnapshot CurrentSnapshot = GetCurrentSnapshot();
    
    // Add to history
    PerformanceHistory.Add(CurrentSnapshot);
    
    // Maintain history size limit
    if (PerformanceHistory.Num() > PerformanceHistorySize)
    {
        PerformanceHistory.RemoveAt(0);
    }
    
    // Analyze performance level
    AnalyzePerformanceLevel();
    
    // Adjust quality if auto-adjustment is enabled
    if (bAutoQualityAdjustment)
    {
        AdjustQualitySettings();
    }
    
    // Broadcast snapshot event
    OnPerformanceSnapshot.Broadcast(CurrentSnapshot);
}

void UPerf_RealTimePerformanceMonitor::AnalyzePerformanceLevel()
{
    float CurrentFPS = CalculateCurrentFPS();
    EPerf_PerformanceLevel NewLevel = CurrentPerformanceLevel;
    
    // Determine performance level based on FPS
    if (CurrentFPS >= EXCELLENT_FPS_THRESHOLD)
    {
        NewLevel = EPerf_PerformanceLevel::Excellent;
        ConsecutiveGoodFrames++;
        ConsecutivePoorFrames = 0;
    }
    else if (CurrentFPS >= GOOD_FPS_THRESHOLD)
    {
        NewLevel = EPerf_PerformanceLevel::Good;
        ConsecutiveGoodFrames++;
        ConsecutivePoorFrames = 0;
    }
    else if (CurrentFPS >= ACCEPTABLE_FPS_THRESHOLD)
    {
        NewLevel = EPerf_PerformanceLevel::Acceptable;
        ConsecutiveGoodFrames = 0;
        ConsecutivePoorFrames++;
    }
    else if (CurrentFPS >= POOR_FPS_THRESHOLD)
    {
        NewLevel = EPerf_PerformanceLevel::Poor;
        ConsecutiveGoodFrames = 0;
        ConsecutivePoorFrames++;
    }
    else
    {
        NewLevel = EPerf_PerformanceLevel::Critical;
        ConsecutiveGoodFrames = 0;
        ConsecutivePoorFrames++;
    }
    
    // Update performance level if changed
    if (NewLevel != CurrentPerformanceLevel)
    {
        CurrentPerformanceLevel = NewLevel;
        OnPerformanceLevelChanged.Broadcast(CurrentPerformanceLevel);
        
        UE_LOG(LogTemp, Warning, TEXT("Performance Monitor: Level changed to %s (%.1f FPS)"),
               *UEnum::GetValueAsString(CurrentPerformanceLevel), CurrentFPS);
    }
}

void UPerf_RealTimePerformanceMonitor::AdjustQualitySettings()
{
    // Update cooldown counter
    if (QualityAdjustmentCooldown > 0)
    {
        QualityAdjustmentCooldown--;
        return;
    }
    
    // Check if we need to reduce quality
    if (ConsecutivePoorFrames >= POOR_FRAME_THRESHOLD && CanReduceQuality())
    {
        ReduceQualitySettings();
        QualityAdjustmentCooldown = QUALITY_ADJUSTMENT_COOLDOWN_FRAMES;
        ConsecutivePoorFrames = 0;
        return;
    }
    
    // Check if we can increase quality
    if (ConsecutiveGoodFrames >= GOOD_FRAME_THRESHOLD && CanIncreaseQuality())
    {
        IncreaseQualitySettings();
        QualityAdjustmentCooldown = QUALITY_ADJUSTMENT_COOLDOWN_FRAMES;
        ConsecutiveGoodFrames = 0;
        return;
    }
}

float UPerf_RealTimePerformanceMonitor::CalculateCurrentFPS() const
{
    if (GEngine && GEngine->GetGameViewport())
    {
        return 1.0f / FApp::GetDeltaTime();
    }
    return 0.0f;
}

float UPerf_RealTimePerformanceMonitor::CalculateFrameTime() const
{
    return FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds
}

float UPerf_RealTimePerformanceMonitor::CalculateGPUTime() const
{
    // This would require access to GPU timing stats
    // For now, return a placeholder value
    return CalculateFrameTime() * 0.7f; // Estimate GPU time as 70% of frame time
}

float UPerf_RealTimePerformanceMonitor::CalculateMemoryUsage() const
{
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    return MemStats.UsedPhysical / (1024.0f * 1024.0f); // Convert to MB
}

int32 UPerf_RealTimePerformanceMonitor::CalculateDrawCalls() const
{
    // This would require access to render stats
    // For now, return a placeholder value
    return 1000; // Placeholder
}

int32 UPerf_RealTimePerformanceMonitor::CalculateTriangleCount() const
{
    // This would require access to render stats
    // For now, return a placeholder value
    return 100000; // Placeholder
}

void UPerf_RealTimePerformanceMonitor::ReduceQualitySettings()
{
    UE_LOG(LogTemp, Warning, TEXT("Performance Monitor: Reducing quality settings"));
    
    // Execute console commands to reduce quality
    if (GEngine)
    {
        // Reduce shadow quality
        GEngine->Exec(GetWorld(), TEXT("r.ShadowQuality 2"));
        
        // Reduce post-processing quality
        GEngine->Exec(GetWorld(), TEXT("r.PostProcessAAQuality 1"));
        
        // Reduce texture quality
        GEngine->Exec(GetWorld(), TEXT("r.Streaming.PoolSize 1000"));
        
        // Reduce view distance
        GEngine->Exec(GetWorld(), TEXT("r.ViewDistanceScale 0.8"));
    }
}

void UPerf_RealTimePerformanceMonitor::IncreaseQualitySettings()
{
    UE_LOG(LogTemp, Log, TEXT("Performance Monitor: Increasing quality settings"));
    
    // Execute console commands to increase quality
    if (GEngine)
    {
        // Increase shadow quality
        GEngine->Exec(GetWorld(), TEXT("r.ShadowQuality 3"));
        
        // Increase post-processing quality
        GEngine->Exec(GetWorld(), TEXT("r.PostProcessAAQuality 2"));
        
        // Increase texture quality
        GEngine->Exec(GetWorld(), TEXT("r.Streaming.PoolSize 2000"));
        
        // Increase view distance
        GEngine->Exec(GetWorld(), TEXT("r.ViewDistanceScale 1.0"));
    }
}

bool UPerf_RealTimePerformanceMonitor::CanReduceQuality() const
{
    // Check if we can reduce quality further
    // This would check current quality settings
    return true; // Placeholder - always allow reduction for now
}

bool UPerf_RealTimePerformanceMonitor::CanIncreaseQuality() const
{
    // Check if we can increase quality
    // This would check current quality settings and performance headroom
    return CurrentPerformanceLevel == EPerf_PerformanceLevel::Excellent;
}