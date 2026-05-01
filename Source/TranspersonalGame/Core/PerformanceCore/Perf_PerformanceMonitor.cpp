#include "Perf_PerformanceMonitor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"
#include "RenderingThread.h"
#include "RHI.h"

APerf_PerformanceMonitor::APerf_PerformanceMonitor()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Initialize default values
    MonitoringInterval = 1.0f;
    bEnablePerformanceLogging = true;
    bEnableAutoOptimization = false;
    TargetFPS = 60.0f;
    MinAcceptableFPS = 30.0f;
    
    bIsMonitoring = false;
    MonitoringTimer = 0.0f;
    MaxHistorySize = 300; // 30 seconds of history at 10Hz
    LastLogTime = 0.0f;
}

void APerf_PerformanceMonitor::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Performance Monitor started - Target FPS: %.1f"), TargetFPS);
    StartMonitoring();
}

void APerf_PerformanceMonitor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bIsMonitoring)
    {
        MonitoringTimer += DeltaTime;
        
        // Update metrics every frame for accurate FPS calculation
        UpdatePerformanceMetrics();
        
        // Log performance data at specified intervals
        if (bEnablePerformanceLogging && MonitoringTimer - LastLogTime >= MonitoringInterval)
        {
            LogPerformanceReport();
            LastLogTime = MonitoringTimer;
        }
        
        // Check if performance has dropped below acceptable levels
        CheckPerformanceThresholds();
    }
}

void APerf_PerformanceMonitor::StartMonitoring()
{
    bIsMonitoring = true;
    MonitoringTimer = 0.0f;
    LastLogTime = 0.0f;
    FPSHistory.Empty();
    ResetMetrics();
    
    UE_LOG(LogTemp, Log, TEXT("Performance monitoring started"));
}

void APerf_PerformanceMonitor::StopMonitoring()
{
    bIsMonitoring = false;
    UE_LOG(LogTemp, Log, TEXT("Performance monitoring stopped"));
}

FPerf_PerformanceMetrics APerf_PerformanceMonitor::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

void APerf_PerformanceMonitor::ResetMetrics()
{
    CurrentMetrics = FPerf_PerformanceMetrics();
    FPSHistory.Empty();
    UE_LOG(LogTemp, Log, TEXT("Performance metrics reset"));
}

void APerf_PerformanceMonitor::LogPerformanceReport()
{
    UE_LOG(LogTemp, Log, TEXT("=== PERFORMANCE REPORT ==="));
    UE_LOG(LogTemp, Log, TEXT("Current FPS: %.2f"), CurrentMetrics.CurrentFPS);
    UE_LOG(LogTemp, Log, TEXT("Average FPS: %.2f"), CurrentMetrics.AverageFPS);
    UE_LOG(LogTemp, Log, TEXT("Min FPS: %.2f"), CurrentMetrics.MinFPS);
    UE_LOG(LogTemp, Log, TEXT("Max FPS: %.2f"), CurrentMetrics.MaxFPS);
    UE_LOG(LogTemp, Log, TEXT("Frame Time: %.2f ms"), CurrentMetrics.FrameTime);
    UE_LOG(LogTemp, Log, TEXT("Memory Usage: %.2f MB"), CurrentMetrics.MemoryUsageMB);
    UE_LOG(LogTemp, Log, TEXT("Performance Level: %s"), 
        CurrentMetrics.PerformanceLevel == EPerf_PerformanceLevel::Excellent ? TEXT("Excellent") :
        CurrentMetrics.PerformanceLevel == EPerf_PerformanceLevel::Good ? TEXT("Good") :
        CurrentMetrics.PerformanceLevel == EPerf_PerformanceLevel::Fair ? TEXT("Fair") :
        CurrentMetrics.PerformanceLevel == EPerf_PerformanceLevel::Poor ? TEXT("Poor") :
        TEXT("Critical"));
    UE_LOG(LogTemp, Log, TEXT("========================"));
}

void APerf_PerformanceMonitor::ApplyPerformanceOptimizations()
{
    if (!bEnableAutoOptimization)
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Applying performance optimizations..."));
    
    // Apply optimizations based on current performance level
    switch (CurrentMetrics.PerformanceLevel)
    {
        case EPerf_PerformanceLevel::Critical:
        case EPerf_PerformanceLevel::Poor:
            // Aggressive optimizations
            if (GEngine)
            {
                // Reduce view distance
                GEngine->Exec(GetWorld(), TEXT("r.ViewDistanceScale 0.5"));
                // Reduce shadow quality
                GEngine->Exec(GetWorld(), TEXT("r.ShadowQuality 1"));
                // Disable some post-processing
                GEngine->Exec(GetWorld(), TEXT("r.PostProcessAAQuality 0"));
                UE_LOG(LogTemp, Warning, TEXT("Applied aggressive performance optimizations"));
            }
            break;
            
        case EPerf_PerformanceLevel::Fair:
            // Moderate optimizations
            if (GEngine)
            {
                GEngine->Exec(GetWorld(), TEXT("r.ViewDistanceScale 0.8"));
                GEngine->Exec(GetWorld(), TEXT("r.ShadowQuality 2"));
                UE_LOG(LogTemp, Log, TEXT("Applied moderate performance optimizations"));
            }
            break;
            
        case EPerf_PerformanceLevel::Good:
        case EPerf_PerformanceLevel::Excellent:
            // Restore quality settings
            if (GEngine)
            {
                GEngine->Exec(GetWorld(), TEXT("r.ViewDistanceScale 1.0"));
                GEngine->Exec(GetWorld(), TEXT("r.ShadowQuality 3"));
                GEngine->Exec(GetWorld(), TEXT("r.PostProcessAAQuality 4"));
                UE_LOG(LogTemp, Log, TEXT("Restored high quality settings"));
            }
            break;
    }
}

void APerf_PerformanceMonitor::UpdatePerformanceMetrics()
{
    // Calculate current FPS
    float DeltaTime = GetWorld()->GetDeltaSeconds();
    if (DeltaTime > 0.0f)
    {
        CurrentMetrics.CurrentFPS = 1.0f / DeltaTime;
        CurrentMetrics.FrameTime = DeltaTime * 1000.0f; // Convert to milliseconds
    }
    
    // Add to history and maintain size limit
    FPSHistory.Add(CurrentMetrics.CurrentFPS);
    if (FPSHistory.Num() > MaxHistorySize)
    {
        FPSHistory.RemoveAt(0);
    }
    
    // Calculate averages
    CalculateAverages();
    
    // Determine performance level
    CurrentMetrics.PerformanceLevel = DeterminePerformanceLevel(CurrentMetrics.CurrentFPS);
    
    // Get memory usage (approximate)
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    
    // Get rendering stats (simplified)
    CurrentMetrics.DrawCalls = 0; // Would need access to render thread stats
    CurrentMetrics.Triangles = 0; // Would need access to render thread stats
}

void APerf_PerformanceMonitor::CalculateAverages()
{
    if (FPSHistory.Num() == 0)
    {
        return;
    }
    
    float Sum = 0.0f;
    float Min = FPSHistory[0];
    float Max = FPSHistory[0];
    
    for (float FPS : FPSHistory)
    {
        Sum += FPS;
        Min = FMath::Min(Min, FPS);
        Max = FMath::Max(Max, FPS);
    }
    
    CurrentMetrics.AverageFPS = Sum / FPSHistory.Num();
    CurrentMetrics.MinFPS = Min;
    CurrentMetrics.MaxFPS = Max;
}

EPerf_PerformanceLevel APerf_PerformanceMonitor::DeterminePerformanceLevel(float FPS) const
{
    if (FPS >= 60.0f)
    {
        return EPerf_PerformanceLevel::Excellent;
    }
    else if (FPS >= 45.0f)
    {
        return EPerf_PerformanceLevel::Good;
    }
    else if (FPS >= 30.0f)
    {
        return EPerf_PerformanceLevel::Fair;
    }
    else if (FPS >= 15.0f)
    {
        return EPerf_PerformanceLevel::Poor;
    }
    else
    {
        return EPerf_PerformanceLevel::Critical;
    }
}

void APerf_PerformanceMonitor::CheckPerformanceThresholds()
{
    // Check if FPS has dropped below minimum acceptable level
    if (CurrentMetrics.CurrentFPS < MinAcceptableFPS)
    {
        static float LastWarningTime = 0.0f;
        float CurrentTime = MonitoringTimer;
        
        // Only warn once per second to avoid spam
        if (CurrentTime - LastWarningTime >= 1.0f)
        {
            UE_LOG(LogTemp, Warning, TEXT("Performance below acceptable level: %.2f FPS (Target: %.2f)"), 
                CurrentMetrics.CurrentFPS, MinAcceptableFPS);
            
            if (bEnableAutoOptimization)
            {
                ApplyPerformanceOptimizations();
            }
            
            LastWarningTime = CurrentTime;
        }
    }
}