#include "Perf_FrameRateManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Stats/Stats.h"
#include "RenderingThread.h"
#include "Engine/GameViewportClient.h"

UPerf_FrameRateManager::UPerf_FrameRateManager()
{
    bMonitoringActive = false;
    bAdaptiveQualityEnabled = false;
    TargetPerformance = EPerf_PerformanceTarget::PC_High;
    TargetFrameRate = 60.0f;
    FrameCounter = 0;
    LastFrameTime = 0.0;
    MonitoringStartTime = 0.0;
    FrameTimeHistory.Reserve(120); // 2 seconds at 60fps
}

void UPerf_FrameRateManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("FrameRateManager: Subsystem initialized"));
    
    // Set initial performance target based on platform
    #if PLATFORM_DESKTOP
        SetPerformanceTarget(EPerf_PerformanceTarget::PC_High);
    #else
        SetPerformanceTarget(EPerf_PerformanceTarget::Console_High);
    #endif
    
    // Start monitoring automatically
    StartPerformanceMonitoring();
}

void UPerf_FrameRateManager::Deinitialize()
{
    StopPerformanceMonitoring();
    Super::Deinitialize();
}

void UPerf_FrameRateManager::StartPerformanceMonitoring()
{
    if (bMonitoringActive)
    {
        return;
    }

    bMonitoringActive = true;
    MonitoringStartTime = FPlatformTime::Seconds();
    LastFrameTime = MonitoringStartTime;
    FrameCounter = 0;
    FrameTimeHistory.Empty();
    
    // Set up timer to update stats every frame
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            MonitoringTimerHandle,
            this,
            &UPerf_FrameRateManager::UpdateFrameStats,
            0.0f, // No delay
            true  // Loop
        );
    }
    
    UE_LOG(LogTemp, Log, TEXT("FrameRateManager: Performance monitoring started"));
}

void UPerf_FrameRateManager::StopPerformanceMonitoring()
{
    if (!bMonitoringActive)
    {
        return;
    }

    bMonitoringActive = false;
    
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(MonitoringTimerHandle);
    }
    
    UE_LOG(LogTemp, Log, TEXT("FrameRateManager: Performance monitoring stopped"));
}

FPerf_FrameStats UPerf_FrameRateManager::GetCurrentFrameStats() const
{
    return CurrentStats;
}

void UPerf_FrameRateManager::SetPerformanceTarget(EPerf_PerformanceTarget Target)
{
    TargetPerformance = Target;
    
    switch (Target)
    {
        case EPerf_PerformanceTarget::PC_High:
            TargetFrameRate = 60.0f;
            break;
        case EPerf_PerformanceTarget::PC_Medium:
            TargetFrameRate = 45.0f;
            break;
        case EPerf_PerformanceTarget::Console_High:
            TargetFrameRate = 30.0f;
            break;
        case EPerf_PerformanceTarget::Console_Low:
            TargetFrameRate = 24.0f;
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("FrameRateManager: Performance target set to %f FPS"), TargetFrameRate);
}

bool UPerf_FrameRateManager::IsPerformanceTargetMet() const
{
    return CurrentStats.AverageFPS >= (TargetFrameRate * 0.9f); // 90% threshold
}

void UPerf_FrameRateManager::ResetFrameStats()
{
    FrameCounter = 0;
    FrameTimeHistory.Empty();
    MonitoringStartTime = FPlatformTime::Seconds();
    LastFrameTime = MonitoringStartTime;
    
    CurrentStats = FPerf_FrameStats();
    
    UE_LOG(LogTemp, Log, TEXT("FrameRateManager: Frame stats reset"));
}

void UPerf_FrameRateManager::EnableAdaptiveQuality(bool bEnable)
{
    bAdaptiveQualityEnabled = bEnable;
    UE_LOG(LogTemp, Log, TEXT("FrameRateManager: Adaptive quality %s"), 
           bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UPerf_FrameRateManager::UpdateFrameStats()
{
    if (!bMonitoringActive)
    {
        return;
    }

    double CurrentTime = FPlatformTime::Seconds();
    double DeltaTime = CurrentTime - LastFrameTime;
    LastFrameTime = CurrentTime;
    
    if (DeltaTime > 0.0)
    {
        float CurrentFPS = 1.0f / DeltaTime;
        float FrameTimeMs = DeltaTime * 1000.0f;
        
        // Update current stats
        CurrentStats.CurrentFPS = CurrentFPS;
        CurrentStats.FrameTime = FrameTimeMs;
        
        // Add to history
        FrameTimeHistory.Add(FrameTimeMs);
        if (FrameTimeHistory.Num() > 120)
        {
            FrameTimeHistory.RemoveAt(0);
        }
        
        // Update min/max
        if (FrameCounter == 0)
        {
            CurrentStats.MinFPS = CurrentFPS;
            CurrentStats.MaxFPS = CurrentFPS;
        }
        else
        {
            CurrentStats.MinFPS = FMath::Min(CurrentStats.MinFPS, CurrentFPS);
            CurrentStats.MaxFPS = FMath::Max(CurrentStats.MaxFPS, CurrentFPS);
        }
        
        // Calculate average
        CurrentStats.AverageFPS = CalculateAverageFPS();
        
        // Get thread times (approximate)
        CurrentStats.GameThreadTime = FrameTimeMs * 0.6f; // Estimate
        CurrentStats.RenderThreadTime = FrameTimeMs * 0.4f; // Estimate
        CurrentStats.GPUTime = FrameTimeMs * 0.8f; // Estimate
        
        FrameCounter++;
        
        // Check performance thresholds
        if (FrameCounter % 60 == 0) // Every 60 frames
        {
            CheckPerformanceThresholds();
        }
    }
}

void UPerf_FrameRateManager::CheckPerformanceThresholds()
{
    if (!IsPerformanceTargetMet())
    {
        UE_LOG(LogTemp, Warning, TEXT("FrameRateManager: Performance below target (%.1f/%.1f FPS)"), 
               CurrentStats.AverageFPS, TargetFrameRate);
        
        if (bAdaptiveQualityEnabled)
        {
            AdjustQualitySettings();
        }
    }
}

void UPerf_FrameRateManager::AdjustQualitySettings()
{
    // Basic adaptive quality adjustments
    if (CurrentStats.AverageFPS < TargetFrameRate * 0.8f)
    {
        // Severe performance issues - reduce quality significantly
        if (GEngine && GEngine->GetGameViewport())
        {
            // Example quality adjustments (would need more sophisticated implementation)
            UE_LOG(LogTemp, Log, TEXT("FrameRateManager: Applying severe quality reduction"));
        }
    }
    else if (CurrentStats.AverageFPS < TargetFrameRate * 0.9f)
    {
        // Moderate performance issues - minor quality reduction
        UE_LOG(LogTemp, Log, TEXT("FrameRateManager: Applying moderate quality reduction"));
    }
}

float UPerf_FrameRateManager::CalculateAverageFPS() const
{
    if (FrameTimeHistory.Num() == 0)
    {
        return 60.0f;
    }
    
    float TotalFrameTime = 0.0f;
    for (float FrameTime : FrameTimeHistory)
    {
        TotalFrameTime += FrameTime;
    }
    
    float AverageFrameTime = TotalFrameTime / FrameTimeHistory.Num();
    return AverageFrameTime > 0.0f ? (1000.0f / AverageFrameTime) : 60.0f;
}