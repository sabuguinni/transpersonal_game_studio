#include "Perf_FPSMonitor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UPerf_FPSMonitor::UPerf_FPSMonitor()
{
    bIsMonitoring = false;
    MonitoringDuration = 1.0f;
    MaxSamples = 60;
    LastFrameTime = 0.0f;
    AccumulatedTime = 0.0f;
    FrameCount = 0;
}

void UPerf_FPSMonitor::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Perf_FPSMonitor: Initialized"));
    
    // Start monitoring by default
    StartMonitoring();
}

void UPerf_FPSMonitor::Deinitialize()
{
    StopMonitoring();
    Super::Deinitialize();
}

FPerf_FPSData UPerf_FPSMonitor::GetCurrentFPSData() const
{
    return CurrentData;
}

void UPerf_FPSMonitor::StartMonitoring()
{
    if (bIsMonitoring)
    {
        return;
    }

    bIsMonitoring = true;
    ResetStatistics();

    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().SetTimer(
            MonitoringTimer,
            this,
            &UPerf_FPSMonitor::UpdateFPSData,
            0.1f,  // Update every 0.1 seconds
            true
        );
        
        UE_LOG(LogTemp, Warning, TEXT("Perf_FPSMonitor: Started monitoring"));
    }
}

void UPerf_FPSMonitor::StopMonitoring()
{
    if (!bIsMonitoring)
    {
        return;
    }

    bIsMonitoring = false;

    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().ClearTimer(MonitoringTimer);
        UE_LOG(LogTemp, Warning, TEXT("Perf_FPSMonitor: Stopped monitoring"));
    }
}

void UPerf_FPSMonitor::ResetStatistics()
{
    CurrentData = FPerf_FPSData();
    FPSSamples.Empty();
    AccumulatedTime = 0.0f;
    FrameCount = 0;
    LastFrameTime = FPlatformTime::Seconds();
}

void UPerf_FPSMonitor::UpdateFPSData()
{
    if (!bIsMonitoring)
    {
        return;
    }

    float CurrentTime = FPlatformTime::Seconds();
    float DeltaTime = CurrentTime - LastFrameTime;
    LastFrameTime = CurrentTime;

    if (DeltaTime > 0.0f)
    {
        CurrentData.CurrentFPS = 1.0f / DeltaTime;
        CurrentData.FrameTime = DeltaTime * 1000.0f; // Convert to milliseconds

        // Add to samples
        FPSSamples.Add(CurrentData.CurrentFPS);
        if (FPSSamples.Num() > MaxSamples)
        {
            FPSSamples.RemoveAt(0);
        }

        // Calculate statistics
        if (FPSSamples.Num() > 0)
        {
            float Sum = 0.0f;
            CurrentData.MinFPS = FPSSamples[0];
            CurrentData.MaxFPS = FPSSamples[0];

            for (float Sample : FPSSamples)
            {
                Sum += Sample;
                CurrentData.MinFPS = FMath::Min(CurrentData.MinFPS, Sample);
                CurrentData.MaxFPS = FMath::Max(CurrentData.MaxFPS, Sample);
            }

            CurrentData.AverageFPS = Sum / FPSSamples.Num();
        }

        // Log performance warnings
        if (CurrentData.CurrentFPS < 30.0f)
        {
            UE_LOG(LogTemp, Warning, TEXT("Perf_FPSMonitor: Low FPS detected: %.2f"), CurrentData.CurrentFPS);
        }
    }
}