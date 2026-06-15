#include "Perf_FrameRateMonitor.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Engine/GameViewportClient.h"

void UPerf_FrameRateMonitor::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    bIsMonitoring = false;
    CurrentPerformanceLevel = EPerf_PerformanceLevel::Optimal;
    FrameTimeHistory.Reserve(60); // Store last 60 frame times
    TotalFrameTime = 0.0f;
    FrameCount = 0;
    MonitoringStartTime = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("FrameRateMonitor initialized"));
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
    MonitoringStartTime = FPlatformTime::Seconds();
    ResetStatistics();

    // Start timer to update frame data every 0.1 seconds
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            MonitoringTimerHandle,
            this,
            &UPerf_FrameRateMonitor::UpdateFrameData,
            0.1f,
            true
        );
    }

    UE_LOG(LogTemp, Log, TEXT("FrameRateMonitor started"));
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

    UE_LOG(LogTemp, Log, TEXT("FrameRateMonitor stopped"));
}

void UPerf_FrameRateMonitor::UpdateFrameData()
{
    if (!bIsMonitoring)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Get current delta time
    float DeltaTime = World->GetDeltaSeconds();
    CurrentFrameData.DeltaTime = DeltaTime;

    // Calculate frame rate
    if (DeltaTime > 0.0f)
    {
        CurrentFrameData.FrameRate = 1.0f / DeltaTime;
    }
    else
    {
        CurrentFrameData.FrameRate = 60.0f; // Default fallback
    }

    // Update frame time history
    FrameTimeHistory.Add(DeltaTime);
    TotalFrameTime += DeltaTime;
    FrameCount++;

    // Keep only last 60 frames
    if (FrameTimeHistory.Num() > 60)
    {
        float OldestFrameTime = FrameTimeHistory[0];
        FrameTimeHistory.RemoveAt(0);
        TotalFrameTime -= OldestFrameTime;
    }

    // Calculate average frame time
    if (FrameTimeHistory.Num() > 0)
    {
        CurrentFrameData.AverageFrameTime = TotalFrameTime / FrameTimeHistory.Num();
    }

    // Count actors in world
    CurrentFrameData.TotalActors = World->GetCurrentLevel()->Actors.Num();

    // Count visible actors (simplified - counts all non-hidden actors)
    int32 VisibleCount = 0;
    for (AActor* Actor : World->GetCurrentLevel()->Actors)
    {
        if (Actor && !Actor->IsHidden())
        {
            VisibleCount++;
        }
    }
    CurrentFrameData.VisibleActors = VisibleCount;

    // Update performance level
    CalculatePerformanceLevel();
}

void UPerf_FrameRateMonitor::CalculatePerformanceLevel()
{
    float AvgFPS = GetAverageFrameRate();

    if (AvgFPS >= 60.0f)
    {
        CurrentPerformanceLevel = EPerf_PerformanceLevel::Optimal;
    }
    else if (AvgFPS >= 45.0f)
    {
        CurrentPerformanceLevel = EPerf_PerformanceLevel::Good;
    }
    else if (AvgFPS >= 30.0f)
    {
        CurrentPerformanceLevel = EPerf_PerformanceLevel::Acceptable;
    }
    else if (AvgFPS >= 15.0f)
    {
        CurrentPerformanceLevel = EPerf_PerformanceLevel::Poor;
    }
    else
    {
        CurrentPerformanceLevel = EPerf_PerformanceLevel::Critical;
    }
}

FPerf_FrameData UPerf_FrameRateMonitor::GetCurrentFrameData() const
{
    return CurrentFrameData;
}

EPerf_PerformanceLevel UPerf_FrameRateMonitor::GetCurrentPerformanceLevel() const
{
    return CurrentPerformanceLevel;
}

float UPerf_FrameRateMonitor::GetAverageFrameRate() const
{
    if (CurrentFrameData.AverageFrameTime > 0.0f)
    {
        return 1.0f / CurrentFrameData.AverageFrameTime;
    }
    return 60.0f; // Default fallback
}

void UPerf_FrameRateMonitor::ResetStatistics()
{
    FrameTimeHistory.Empty();
    TotalFrameTime = 0.0f;
    FrameCount = 0;
    CurrentFrameData = FPerf_FrameData();
    CurrentPerformanceLevel = EPerf_PerformanceLevel::Optimal;
}