#include "Perf_FrameRateManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "HAL/PlatformApplicationMisc.h"

UPerf_FrameRateManager::UPerf_FrameRateManager()
{
    CurrentTarget = EPerf_PerformanceTarget::PC_60FPS;
    bIsTracking = false;
    bAdaptiveQualityEnabled = false;
    TargetFrameRate = 60.0f;
    FrameCounter = 0;
    TotalFrameTime = 0.0f;
    LastUpdateTime = 0.0;
    FrameTimeHistory.Reserve(120); // 2 seconds at 60fps
}

void UPerf_FrameRateManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogTemp, Log, TEXT("FrameRateManager: Initialized"));
}

void UPerf_FrameRateManager::Deinitialize()
{
    StopFrameTracking();
    Super::Deinitialize();
}

bool UPerf_FrameRateManager::ShouldCreateSubsystem(UObject* Outer) const
{
    return true;
}

void UPerf_FrameRateManager::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    
    // Auto-start tracking when world begins play
    StartFrameTracking();
    
    // Set default performance target based on platform
    if (FPlatformApplicationMisc::IsRunningOnBattery())
    {
        SetPerformanceTarget(EPerf_PerformanceTarget::Mobile_30FPS);
    }
    else
    {
        SetPerformanceTarget(EPerf_PerformanceTarget::PC_60FPS);
    }
}

void UPerf_FrameRateManager::SetPerformanceTarget(EPerf_PerformanceTarget Target)
{
    CurrentTarget = Target;
    
    switch (Target)
    {
        case EPerf_PerformanceTarget::PC_60FPS:
            TargetFrameRate = 60.0f;
            break;
        case EPerf_PerformanceTarget::Console_30FPS:
        case EPerf_PerformanceTarget::Mobile_30FPS:
            TargetFrameRate = 30.0f;
            break;
        case EPerf_PerformanceTarget::Adaptive:
            TargetFrameRate = 60.0f; // Start high, adapt down
            EnableAdaptiveQuality(true);
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("FrameRateManager: Target set to %f FPS"), TargetFrameRate);
}

FPerf_FrameMetrics UPerf_FrameRateManager::GetCurrentMetrics() const
{
    return FrameMetrics;
}

bool UPerf_FrameRateManager::IsPerformanceTargetMet() const
{
    return FrameMetrics.bIsTargetMet;
}

void UPerf_FrameRateManager::StartFrameTracking()
{
    if (bIsTracking)
        return;
        
    bIsTracking = true;
    ResetMetrics();
    
    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().SetTimer(
            MetricsUpdateTimer,
            this,
            &UPerf_FrameRateManager::UpdateFrameMetrics,
            0.1f, // Update every 100ms
            true
        );
    }
    
    UE_LOG(LogTemp, Log, TEXT("FrameRateManager: Started frame tracking"));
}

void UPerf_FrameRateManager::StopFrameTracking()
{
    if (!bIsTracking)
        return;
        
    bIsTracking = false;
    
    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().ClearTimer(MetricsUpdateTimer);
    }
    
    UE_LOG(LogTemp, Log, TEXT("FrameRateManager: Stopped frame tracking"));
}

void UPerf_FrameRateManager::ResetMetrics()
{
    FrameMetrics = FPerf_FrameMetrics();
    FrameTimeHistory.Empty();
    FrameCounter = 0;
    TotalFrameTime = 0.0f;
    LastUpdateTime = FPlatformTime::Seconds();
}

void UPerf_FrameRateManager::EnableAdaptiveQuality(bool bEnable)
{
    bAdaptiveQualityEnabled = bEnable;
    UE_LOG(LogTemp, Log, TEXT("FrameRateManager: Adaptive quality %s"), 
           bEnable ? TEXT("enabled") : TEXT("disabled"));
}

float UPerf_FrameRateManager::GetTargetFrameRate() const
{
    return TargetFrameRate;
}

void UPerf_FrameRateManager::UpdateFrameMetrics()
{
    if (!bIsTracking)
        return;
        
    double CurrentTime = FPlatformTime::Seconds();
    float DeltaTime = CurrentTime - LastUpdateTime;
    LastUpdateTime = CurrentTime;
    
    if (DeltaTime > 0.0f)
    {
        FrameMetrics.CurrentFPS = 1.0f / DeltaTime;
        FrameMetrics.FrameTime = DeltaTime * 1000.0f; // Convert to milliseconds
        
        // Update history
        FrameTimeHistory.Add(DeltaTime);
        if (FrameTimeHistory.Num() > 120) // Keep last 2 seconds
        {
            FrameTimeHistory.RemoveAt(0);
        }
        
        // Calculate statistics
        TotalFrameTime += DeltaTime;
        FrameCounter++;
        
        if (FrameCounter > 0)
        {
            FrameMetrics.AverageFPS = FrameCounter / TotalFrameTime;
        }
        
        // Update min/max
        if (FrameMetrics.CurrentFPS < FrameMetrics.MinFPS)
        {
            FrameMetrics.MinFPS = FrameMetrics.CurrentFPS;
        }
        if (FrameMetrics.CurrentFPS > FrameMetrics.MaxFPS)
        {
            FrameMetrics.MaxFPS = FrameMetrics.CurrentFPS;
        }
        
        CheckPerformanceTarget();
        
        if (bAdaptiveQualityEnabled)
        {
            AdjustQualitySettings();
        }
    }
}

void UPerf_FrameRateManager::CheckPerformanceTarget()
{
    float Tolerance = TargetFrameRate * 0.1f; // 10% tolerance
    FrameMetrics.bIsTargetMet = (FrameMetrics.AverageFPS >= (TargetFrameRate - Tolerance));
}

void UPerf_FrameRateManager::AdjustQualitySettings()
{
    if (FrameTimeHistory.Num() < 30) // Need enough samples
        return;
        
    // Calculate recent average
    float RecentTotal = 0.0f;
    int32 SampleCount = FMath::Min(30, FrameTimeHistory.Num());
    for (int32 i = FrameTimeHistory.Num() - SampleCount; i < FrameTimeHistory.Num(); i++)
    {
        RecentTotal += FrameTimeHistory[i];
    }
    float RecentAvgFPS = SampleCount / RecentTotal;
    
    // Adjust quality based on performance
    float TargetTolerance = TargetFrameRate * 0.15f; // 15% tolerance for adjustments
    
    if (RecentAvgFPS < (TargetFrameRate - TargetTolerance))
    {
        // Performance is poor, reduce quality
        UE_LOG(LogTemp, Warning, TEXT("FrameRateManager: Performance below target (%.1f < %.1f), considering quality reduction"), 
               RecentAvgFPS, TargetFrameRate);
    }
    else if (RecentAvgFPS > (TargetFrameRate + TargetTolerance))
    {
        // Performance is good, can increase quality
        UE_LOG(LogTemp, Log, TEXT("FrameRateManager: Performance above target (%.1f > %.1f), considering quality increase"), 
               RecentAvgFPS, TargetFrameRate);
    }
}