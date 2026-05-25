#include "Perf_FrameRateManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "HAL/PlatformApplicationMisc.h"
#include "RenderingThread.h"
#include "Stats/Stats.h"

UPerf_FrameRateManager::UPerf_FrameRateManager()
{
    CurrentTarget = EPerf_PerformanceTarget::PC_Ultra;
    bIsMonitoring = false;
    MonitoringDuration = 0.0f;
    FrameCount = 0;
    TotalFrameTime = 0.0f;
    LastFrameTime = 0.0f;
    
    // Reserve space for frame history
    FrameTimeHistory.Reserve(300); // 5 seconds at 60fps
}

void UPerf_FrameRateManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Performance Frame Rate Manager initialized"));
    
    // Set initial performance target based on platform
    #if PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX
        SetPerformanceTarget(EPerf_PerformanceTarget::PC_Ultra);
    #else
        SetPerformanceTarget(EPerf_PerformanceTarget::Console);
    #endif
    
    // Start monitoring by default
    StartFrameMonitoring();
}

void UPerf_FrameRateManager::Deinitialize()
{
    StopFrameMonitoring();
    Super::Deinitialize();
}

void UPerf_FrameRateManager::SetPerformanceTarget(EPerf_PerformanceTarget Target)
{
    CurrentTarget = Target;
    
    // Apply target-specific settings
    float TargetFPS = GetTargetFPS();
    
    if (GEngine)
    {
        // Set max FPS
        GEngine->SetMaxFPS(TargetFPS);
        
        // Configure VSync based on target
        bool bUseVSync = (Target == EPerf_PerformanceTarget::PC_Ultra);
        EnableVSync(bUseVSync);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Performance target set to %d (%.1f FPS)"), 
           static_cast<int32>(Target), TargetFPS);
}

float UPerf_FrameRateManager::GetTargetFPS() const
{
    switch (CurrentTarget)
    {
        case EPerf_PerformanceTarget::PC_Ultra:
            return 60.0f;
        case EPerf_PerformanceTarget::PC_High:
            return 45.0f;
        case EPerf_PerformanceTarget::Console:
            return 30.0f;
        case EPerf_PerformanceTarget::Mobile:
            return 24.0f;
        default:
            return 60.0f;
    }
}

void UPerf_FrameRateManager::StartFrameMonitoring()
{
    if (bIsMonitoring)
    {
        return;
    }
    
    bIsMonitoring = true;
    ResetMetrics();
    
    // Start timer for regular updates
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            MonitoringTimer,
            this,
            &UPerf_FrameRateManager::OnFrameTick,
            0.1f, // Update every 100ms
            true
        );
    }
    
    UE_LOG(LogTemp, Log, TEXT("Frame rate monitoring started"));
}

void UPerf_FrameRateManager::StopFrameMonitoring()
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
    
    UE_LOG(LogTemp, Log, TEXT("Frame rate monitoring stopped"));
}

void UPerf_FrameRateManager::ResetMetrics()
{
    CurrentMetrics = FPerf_FrameMetrics();
    FrameTimeHistory.Empty();
    FrameCount = 0;
    TotalFrameTime = 0.0f;
    MonitoringDuration = 0.0f;
}

void UPerf_FrameRateManager::OnFrameTick()
{
    if (!bIsMonitoring)
    {
        return;
    }
    
    UpdateMetrics();
}

void UPerf_FrameRateManager::UpdateMetrics()
{
    if (!GEngine)
    {
        return;
    }
    
    // Get current frame time
    float CurrentFrameTime = FApp::GetDeltaTime();
    
    if (CurrentFrameTime > 0.0f)
    {
        // Calculate current FPS
        CurrentMetrics.CurrentFPS = 1.0f / CurrentFrameTime;
        CurrentMetrics.FrameTime = CurrentFrameTime * 1000.0f; // Convert to milliseconds
        
        // Update history
        FrameTimeHistory.Add(CurrentFrameTime);
        if (FrameTimeHistory.Num() > 300) // Keep last 5 seconds at 60fps
        {
            FrameTimeHistory.RemoveAt(0);
        }
        
        // Update min/max
        if (CurrentMetrics.CurrentFPS < CurrentMetrics.MinFPS)
        {
            CurrentMetrics.MinFPS = CurrentMetrics.CurrentFPS;
        }
        if (CurrentMetrics.CurrentFPS > CurrentMetrics.MaxFPS)
        {
            CurrentMetrics.MaxFPS = CurrentMetrics.CurrentFPS;
        }
        
        // Calculate average
        CurrentMetrics.AverageFPS = CalculateAverageFPS();
        
        // Count dropped frames (frames below target)
        float TargetFPS = GetTargetFPS();
        if (CurrentMetrics.CurrentFPS < TargetFPS * 0.9f) // 10% tolerance
        {
            CurrentMetrics.DroppedFrames++;
        }
        
        FrameCount++;
        TotalFrameTime += CurrentFrameTime;
        MonitoringDuration += CurrentFrameTime;
    }
}

float UPerf_FrameRateManager::CalculateAverageFPS() const
{
    if (FrameTimeHistory.Num() == 0)
    {
        return 0.0f;
    }
    
    float TotalTime = 0.0f;
    for (float FrameTime : FrameTimeHistory)
    {
        TotalTime += FrameTime;
    }
    
    return FrameTimeHistory.Num() / TotalTime;
}

bool UPerf_FrameRateManager::IsPerformanceAcceptable() const
{
    float TargetFPS = GetTargetFPS();
    float Tolerance = TargetFPS * 0.1f; // 10% tolerance
    
    return CurrentMetrics.AverageFPS >= (TargetFPS - Tolerance);
}

void UPerf_FrameRateManager::EnableVSync(bool bEnable)
{
    if (GEngine)
    {
        // Set VSync via console command
        FString Command = FString::Printf(TEXT("r.VSync %d"), bEnable ? 1 : 0);
        GEngine->Exec(GetWorld(), *Command);
        
        UE_LOG(LogTemp, Log, TEXT("VSync %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
    }
}

void UPerf_FrameRateManager::SetMaxFPS(int32 MaxFPS)
{
    if (GEngine)
    {
        GEngine->SetMaxFPS(static_cast<float>(MaxFPS));
        UE_LOG(LogTemp, Log, TEXT("Max FPS set to %d"), MaxFPS);
    }
}