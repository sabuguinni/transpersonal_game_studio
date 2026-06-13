#include "Perf_FrameRateController.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformApplicationMisc.h"
#include "RHI.h"
#include "Stats/Stats.h"
#include "TimerManager.h"

UPerf_FrameRateController::UPerf_FrameRateController()
{
    LastFrameTime = 0.0f;
    MetricsAccumulator = 0.0f;
    MetricsFrameCount = 0;
    bInitialized = false;
}

void UPerf_FrameRateController::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    if (!bInitialized)
    {
        // Initialize frame rate settings
        FrameRateSettings = FPerf_FrameRateSettings();
        CurrentMetrics = FPerf_FrameRateMetrics();
        
        // Reserve history array
        FrameTimeHistory.Reserve(MetricsHistorySize);
        
        // Start metrics timer
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().SetTimer(
                MetricsTimerHandle,
                this,
                &UPerf_FrameRateController::UpdateMetrics,
                MetricsUpdateInterval,
                true
            );
        }
        
        // Apply initial frame rate settings
        OptimizeForPlatform();
        ApplyFrameRateLimit();
        
        bInitialized = true;
        
        UE_LOG(LogTemp, Log, TEXT("FrameRateController: Initialized successfully"));
    }
}

void UPerf_FrameRateController::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(MetricsTimerHandle);
    }
    
    bInitialized = false;
    Super::Deinitialize();
}

void UPerf_FrameRateController::SetTargetFrameRate(EPerf_FrameRateTarget TargetRate)
{
    FrameRateSettings.TargetFrameRate = TargetRate;
    ApplyFrameRateLimit();
    HandleFrameRateChange();
    
    UE_LOG(LogTemp, Log, TEXT("FrameRateController: Target frame rate set to %d"), (int32)TargetRate);
}

void UPerf_FrameRateController::SetCustomFrameRate(float CustomRate)
{
    FrameRateSettings.CustomFrameRate = FMath::Clamp(CustomRate, 10.0f, 240.0f);
    if (FrameRateSettings.TargetFrameRate == EPerf_FrameRateTarget::Auto)
    {
        ApplyFrameRateLimit();
    }
    
    UE_LOG(LogTemp, Log, TEXT("FrameRateController: Custom frame rate set to %.1f"), CustomRate);
}

void UPerf_FrameRateController::EnableVSync(bool bEnable)
{
    FrameRateSettings.bVSyncEnabled = bEnable;
    
    // Apply VSync setting
    if (GEngine && GEngine->GameUserSettings)
    {
        GEngine->GameUserSettings->SetVSyncEnabled(bEnable);
        GEngine->GameUserSettings->ApplySettings(false);
    }
    
    UE_LOG(LogTemp, Log, TEXT("FrameRateController: VSync %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UPerf_FrameRateController::EnableAdaptiveFrameRate(bool bEnable)
{
    FrameRateSettings.bAdaptiveFrameRate = bEnable;
    
    if (bEnable)
    {
        DetectOptimalFrameRate();
    }
    
    UE_LOG(LogTemp, Log, TEXT("FrameRateController: Adaptive frame rate %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

FPerf_FrameRateMetrics UPerf_FrameRateController::GetFrameRateMetrics() const
{
    return CurrentMetrics;
}

float UPerf_FrameRateController::GetCurrentFPS() const
{
    return CurrentMetrics.CurrentFPS;
}

void UPerf_FrameRateController::ResetMetrics()
{
    CurrentMetrics = FPerf_FrameRateMetrics();
    FrameTimeHistory.Empty();
    MetricsAccumulator = 0.0f;
    MetricsFrameCount = 0;
    
    UE_LOG(LogTemp, Log, TEXT("FrameRateController: Metrics reset"));
}

void UPerf_FrameRateController::ApplyFrameRateSettings(const FPerf_FrameRateSettings& Settings)
{
    FrameRateSettings = Settings;
    ApplyFrameRateLimit();
    EnableVSync(Settings.bVSyncEnabled);
    
    UE_LOG(LogTemp, Log, TEXT("FrameRateController: Settings applied"));
}

FPerf_FrameRateSettings UPerf_FrameRateController::GetFrameRateSettings() const
{
    return FrameRateSettings;
}

void UPerf_FrameRateController::OptimizeForPlatform()
{
    // Detect platform capabilities
    bool bIsDesktop = FPlatformApplicationMisc::IsThisApplicationForeground();
    bool bIsHighEnd = GRHISupportsHDROutput;
    
    if (FrameRateSettings.TargetFrameRate == EPerf_FrameRateTarget::Auto)
    {
        if (bIsDesktop && bIsHighEnd)
        {
            FrameRateSettings.TargetFrameRate = EPerf_FrameRateTarget::FPS_60;
        }
        else
        {
            FrameRateSettings.TargetFrameRate = EPerf_FrameRateTarget::FPS_30;
        }
    }
    
    // Enable adaptive frame rate for better performance
    FrameRateSettings.bAdaptiveFrameRate = true;
    
    UE_LOG(LogTemp, Log, TEXT("FrameRateController: Platform optimization complete"));
}

bool UPerf_FrameRateController::IsFrameRateStable() const
{
    if (FrameTimeHistory.Num() < 10)
    {
        return false;
    }
    
    float TargetFPS = 60.0f;
    switch (FrameRateSettings.TargetFrameRate)
    {
        case EPerf_FrameRateTarget::FPS_30: TargetFPS = 30.0f; break;
        case EPerf_FrameRateTarget::FPS_60: TargetFPS = 60.0f; break;
        case EPerf_FrameRateTarget::FPS_120: TargetFPS = 120.0f; break;
        default: TargetFPS = FrameRateSettings.CustomFrameRate; break;
    }
    
    float Variance = FMath::Abs(CurrentMetrics.CurrentFPS - TargetFPS);
    return Variance <= FrameRateSettings.FrameRateTolerance;
}

void UPerf_FrameRateController::ForceGarbageCollection()
{
    if (GEngine)
    {
        GEngine->ForceGarbageCollection(true);
        UE_LOG(LogTemp, Log, TEXT("FrameRateController: Forced garbage collection"));
    }
}

void UPerf_FrameRateController::UpdateMetrics()
{
    if (!bInitialized)
    {
        return;
    }
    
    // Get current frame time
    float CurrentFrameTime = FApp::GetDeltaTime();
    if (CurrentFrameTime > 0.0f)
    {
        CurrentMetrics.CurrentFPS = 1.0f / CurrentFrameTime;
        CurrentMetrics.FrameTime = CurrentFrameTime * 1000.0f; // Convert to milliseconds
        
        // Update history
        FrameTimeHistory.Add(CurrentFrameTime);
        if (FrameTimeHistory.Num() > MetricsHistorySize)
        {
            FrameTimeHistory.RemoveAt(0);
        }
        
        // Calculate statistics
        if (FrameTimeHistory.Num() > 0)
        {
            float TotalTime = 0.0f;
            float MinTime = FrameTimeHistory[0];
            float MaxTime = FrameTimeHistory[0];
            
            for (float Time : FrameTimeHistory)
            {
                TotalTime += Time;
                MinTime = FMath::Min(MinTime, Time);
                MaxTime = FMath::Max(MaxTime, Time);
            }
            
            CurrentMetrics.AverageFPS = FrameTimeHistory.Num() / TotalTime;
            CurrentMetrics.MinFPS = 1.0f / MaxTime;
            CurrentMetrics.MaxFPS = 1.0f / MinTime;
        }
        
        // Check for frame drops
        float TargetFrameTime = 1.0f / 60.0f; // Default to 60 FPS
        if (CurrentFrameTime > TargetFrameTime * 1.5f)
        {
            CurrentMetrics.FrameDropCount++;
        }
        
        // Adaptive frame rate adjustment
        if (FrameRateSettings.bAdaptiveFrameRate)
        {
            if (!IsFrameRateStable() && CurrentMetrics.CurrentFPS < 30.0f)
            {
                // Consider reducing quality settings or frame rate target
                UE_LOG(LogTemp, Warning, TEXT("FrameRateController: Performance below threshold, consider optimization"));
            }
        }
    }
}

void UPerf_FrameRateController::ApplyFrameRateLimit()
{
    float TargetFPS = 60.0f;
    
    switch (FrameRateSettings.TargetFrameRate)
    {
        case EPerf_FrameRateTarget::FPS_30:
            TargetFPS = 30.0f;
            break;
        case EPerf_FrameRateTarget::FPS_60:
            TargetFPS = 60.0f;
            break;
        case EPerf_FrameRateTarget::FPS_120:
            TargetFPS = 120.0f;
            break;
        case EPerf_FrameRateTarget::Unlimited:
            TargetFPS = 0.0f; // No limit
            break;
        case EPerf_FrameRateTarget::Auto:
        default:
            TargetFPS = FrameRateSettings.CustomFrameRate;
            break;
    }
    
    // Apply frame rate limit
    if (GEngine)
    {
        if (TargetFPS > 0.0f)
        {
            GEngine->SetMaxFPS(TargetFPS);
        }
        else
        {
            GEngine->SetMaxFPS(0.0f); // Remove limit
        }
    }
}

void UPerf_FrameRateController::DetectOptimalFrameRate()
{
    // Simple heuristic for optimal frame rate detection
    if (CurrentMetrics.AverageFPS > 55.0f)
    {
        // System can handle 60 FPS
        FrameRateSettings.TargetFrameRate = EPerf_FrameRateTarget::FPS_60;
    }
    else if (CurrentMetrics.AverageFPS > 25.0f)
    {
        // System should target 30 FPS
        FrameRateSettings.TargetFrameRate = EPerf_FrameRateTarget::FPS_30;
    }
    
    ApplyFrameRateLimit();
}

void UPerf_FrameRateController::HandleFrameRateChange()
{
    // Reset metrics when frame rate target changes
    ResetMetrics();
    
    // Force a garbage collection to clear any performance issues
    ForceGarbageCollection();
}