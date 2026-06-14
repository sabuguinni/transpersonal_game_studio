#include "Perf_PerformanceMonitor.h"
#include "Engine/Engine.h"
#include "HAL/PlatformMemory.h"
#include "Stats/Stats.h"
#include "TimerManager.h"
#include "Engine/World.h"

void UPerf_PerformanceMonitor::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Performance Monitor initialized"));
    
    // Start monitoring by default in development builds
    #if UE_BUILD_DEVELOPMENT || UE_BUILD_DEBUG
    StartMonitoring();
    #endif
}

void UPerf_PerformanceMonitor::Deinitialize()
{
    StopMonitoring();
    Super::Deinitialize();
}

void UPerf_PerformanceMonitor::StartMonitoring()
{
    if (bIsMonitoring)
    {
        return;
    }

    bIsMonitoring = true;
    FPSHistory.Empty();

    // Update metrics every frame
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            MonitoringTimer,
            this,
            &UPerf_PerformanceMonitor::UpdatePerformanceMetrics,
            0.0f, // No delay
            true  // Loop
        );
    }

    UE_LOG(LogTemp, Log, TEXT("Performance monitoring started"));
}

void UPerf_PerformanceMonitor::StopMonitoring()
{
    if (!bIsMonitoring)
    {
        return;
    }

    bIsMonitoring = false;

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(MonitoringTimer);
    }

    UE_LOG(LogTemp, Log, TEXT("Performance monitoring stopped"));
}

FPerf_FrameData UPerf_PerformanceMonitor::GetCurrentFrameData() const
{
    FPerf_FrameData FrameData;

    if (UWorld* World = GetWorld())
    {
        FrameData.DeltaTime = World->GetDeltaSeconds();
        FrameData.FPS = FrameData.DeltaTime > 0.0f ? 1.0f / FrameData.DeltaTime : 0.0f;
    }

    // Get render thread stats if available
    #if STATS
    FrameData.GameThreadTime = FPlatformTime::ToMilliseconds(GGameThreadTime);
    FrameData.RenderThreadTime = FPlatformTime::ToMilliseconds(GRenderThreadTime);
    FrameData.GPUTime = FPlatformTime::ToMilliseconds(GGPUFrameTime);
    #endif

    return FrameData;
}

FPerf_MemoryData UPerf_PerformanceMonitor::GetCurrentMemoryData() const
{
    FPerf_MemoryData MemoryData;

    FPlatformMemoryStats Stats = FPlatformMemory::GetStats();
    
    MemoryData.UsedPhysical = Stats.UsedPhysical / (1024.0f * 1024.0f); // Convert to MB
    MemoryData.UsedVirtual = Stats.UsedVirtual / (1024.0f * 1024.0f);
    MemoryData.PeakUsedPhysical = Stats.PeakUsedPhysical / (1024.0f * 1024.0f);
    MemoryData.PeakUsedVirtual = Stats.PeakUsedVirtual / (1024.0f * 1024.0f);

    return MemoryData;
}

float UPerf_PerformanceMonitor::GetAverageFPS() const
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

void UPerf_PerformanceMonitor::SetFPSTarget(float NewTargetFPS)
{
    TargetFPS = FMath::Max(1.0f, NewTargetFPS);
    UE_LOG(LogTemp, Log, TEXT("FPS target set to: %f"), TargetFPS);
}

void UPerf_PerformanceMonitor::EnablePerformanceWarnings(bool bEnable)
{
    bPerformanceWarningsEnabled = bEnable;
    UE_LOG(LogTemp, Log, TEXT("Performance warnings %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UPerf_PerformanceMonitor::UpdatePerformanceMetrics()
{
    if (!bIsMonitoring)
    {
        return;
    }

    FPerf_FrameData CurrentFrame = GetCurrentFrameData();
    
    // Add to history
    FPSHistory.Add(CurrentFrame.FPS);
    
    // Limit history size
    if (FPSHistory.Num() > MaxHistorySize)
    {
        FPSHistory.RemoveAt(0);
    }

    // Check performance thresholds
    CheckPerformanceThresholds();
}

void UPerf_PerformanceMonitor::CheckPerformanceThresholds()
{
    if (!bPerformanceWarningsEnabled)
    {
        return;
    }

    float CurrentFPS = FPSHistory.Num() > 0 ? FPSHistory.Last() : 0.0f;
    float AverageFPS = GetAverageFPS();

    // Low FPS warning
    if (CurrentFPS < TargetFPS * 0.5f)
    {
        FString AlertMsg = FString::Printf(TEXT("Low FPS detected: %.1f (Target: %.1f)"), CurrentFPS, TargetFPS);
        OnPerformanceAlert.Broadcast(AlertMsg);
        UE_LOG(LogTemp, Warning, TEXT("%s"), *AlertMsg);
    }

    // Memory warning
    FPerf_MemoryData MemData = GetCurrentMemoryData();
    if (MemData.UsedPhysical > 4000.0f) // 4GB warning threshold
    {
        FString AlertMsg = FString::Printf(TEXT("High memory usage: %.1f MB"), MemData.UsedPhysical);
        OnPerformanceAlert.Broadcast(AlertMsg);
        UE_LOG(LogTemp, Warning, TEXT("%s"), *AlertMsg);
    }
}