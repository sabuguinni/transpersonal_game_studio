#include "Perf_FrameRateMonitor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"

UPerf_FrameRateMonitor::UPerf_FrameRateMonitor()
{
    bIsMonitoring = false;
    TargetFPS = 60.0f;
    TotalFrameTime = 0.0f;
    RecentFrameTimes.Reserve(MaxRecentFrames);
}

void UPerf_FrameRateMonitor::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("FrameRateMonitor: Initialized"));
    
    // Start monitoring automatically
    StartMonitoring();
}

void UPerf_FrameRateMonitor::Deinitialize()
{
    StopMonitoring();
    Super::Deinitialize();
}

void UPerf_FrameRateMonitor::StartMonitoring()
{
    if (bIsMonitoring)
    {
        return;
    }

    bIsMonitoring = true;
    ResetStatistics();

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            MonitoringTimerHandle,
            this,
            &UPerf_FrameRateMonitor::UpdateFrameRateData,
            0.1f, // Update every 100ms
            true
        );
    }

    UE_LOG(LogTemp, Log, TEXT("FrameRateMonitor: Started monitoring"));
}

void UPerf_FrameRateMonitor::StopMonitoring()
{
    if (!bIsMonitoring)
    {
        return;
    }

    bIsMonitoring = false;

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(MonitoringTimerHandle);
    }

    UE_LOG(LogTemp, Log, TEXT("FrameRateMonitor: Stopped monitoring"));
}

void UPerf_FrameRateMonitor::UpdateFrameRateData()
{
    if (!bIsMonitoring)
    {
        return;
    }

    // Get current frame time from engine
    float CurrentFrameTime = FApp::GetDeltaTime();
    
    if (CurrentFrameTime > 0.0f)
    {
        float CurrentFPS = 1.0f / CurrentFrameTime;
        
        // Update current values
        FrameRateData.CurrentFPS = CurrentFPS;
        FrameRateData.FrameTime = CurrentFrameTime * 1000.0f; // Convert to milliseconds
        FrameRateData.TotalFrameCount++;

        // Track recent frame times
        RecentFrameTimes.Add(CurrentFrameTime);
        if (RecentFrameTimes.Num() > MaxRecentFrames)
        {
            TotalFrameTime -= RecentFrameTimes[0];
            RecentFrameTimes.RemoveAt(0);
        }
        TotalFrameTime += CurrentFrameTime;

        // Update min/max
        if (CurrentFPS < FrameRateData.MinFPS)
        {
            FrameRateData.MinFPS = CurrentFPS;
        }
        if (CurrentFPS > FrameRateData.MaxFPS)
        {
            FrameRateData.MaxFPS = CurrentFPS;
        }

        // Calculate average FPS from recent frames
        if (RecentFrameTimes.Num() > 0)
        {
            float AverageFrameTime = TotalFrameTime / RecentFrameTimes.Num();
            FrameRateData.AverageFPS = 1.0f / AverageFrameTime;
        }
    }
}

FPerf_FrameRateData UPerf_FrameRateMonitor::GetCurrentFrameRateData() const
{
    return FrameRateData;
}

void UPerf_FrameRateMonitor::ResetStatistics()
{
    FrameRateData = FPerf_FrameRateData();
    RecentFrameTimes.Empty();
    TotalFrameTime = 0.0f;
}

void UPerf_FrameRateMonitor::SetTargetFPS(float InTargetFPS)
{
    TargetFPS = FMath::Clamp(InTargetFPS, 15.0f, 240.0f);
    UE_LOG(LogTemp, Log, TEXT("FrameRateMonitor: Target FPS set to %f"), TargetFPS);
}

bool UPerf_FrameRateMonitor::IsPerformingWell() const
{
    if (!bIsMonitoring || FrameRateData.TotalFrameCount < 10)
    {
        return true; // Not enough data yet
    }

    // Consider performance good if average FPS is within 10% of target
    float PerformanceThreshold = TargetFPS * 0.9f;
    return FrameRateData.AverageFPS >= PerformanceThreshold;
}