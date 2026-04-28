#include "PerformanceMonitor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Engine/GameViewportClient.h"
#include "RenderCore.h"
#include "Stats/Stats.h"
#include "HAL/IConsoleManager.h"

APerf_PerformanceMonitor::APerf_PerformanceMonitor()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.0f; // Tick every frame for accurate monitoring

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default settings
    MonitoringInterval = 1.0f;
    bDisplayOnScreen = true;
    bLogToConsole = false;
    bAutoOptimize = true;
    TimeSinceLastUpdate = 0.0f;
    MaxHistorySize = 60;
    TotalFPSSum = 0.0f;
    FrameCount = 0;

    // Reserve space for FPS history
    FPSHistory.Reserve(MaxHistorySize);
}

void APerf_PerformanceMonitor::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Log, TEXT("Performance Monitor started - monitoring at %f second intervals"), MonitoringInterval);
    
    // Apply initial optimization settings
    ApplyOptimizationSettings();
    
    // Reset metrics
    ResetMetrics();
}

void APerf_PerformanceMonitor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update FPS every frame for accuracy
    UpdateFPSMetrics(DeltaTime);

    TimeSinceLastUpdate += DeltaTime;

    // Update other metrics at the specified interval
    if (TimeSinceLastUpdate >= MonitoringInterval)
    {
        UpdateMetrics();
        
        if (bDisplayOnScreen)
        {
            DisplayMetricsOnScreen();
        }

        if (bLogToConsole)
        {
            UE_LOG(LogTemp, Log, TEXT("Performance: FPS=%.1f, FrameTime=%.2fms, DrawCalls=%d"), 
                CurrentMetrics.CurrentFPS, CurrentMetrics.FrameTime, CurrentMetrics.DrawCalls);
        }

        if (bAutoOptimize)
        {
            ApplyAutoOptimizations();
        }

        TimeSinceLastUpdate = 0.0f;
    }
}

void APerf_PerformanceMonitor::UpdateMetrics()
{
    UpdateRenderMetrics();
    UpdateMemoryMetrics();
    DeterminePerformanceLevel();
}

void APerf_PerformanceMonitor::UpdateFPSMetrics(float DeltaTime)
{
    if (DeltaTime > 0.0f)
    {
        CurrentMetrics.CurrentFPS = 1.0f / DeltaTime;
        CurrentMetrics.FrameTime = DeltaTime * 1000.0f; // Convert to milliseconds

        // Update FPS history
        FPSHistory.Add(CurrentMetrics.CurrentFPS);
        TotalFPSSum += CurrentMetrics.CurrentFPS;
        FrameCount++;

        // Maintain history size
        if (FPSHistory.Num() > MaxHistorySize)
        {
            TotalFPSSum -= FPSHistory[0];
            FPSHistory.RemoveAt(0);
        }

        // Calculate average FPS
        if (FPSHistory.Num() > 0)
        {
            CurrentMetrics.AverageFPS = TotalFPSSum / FPSHistory.Num();
        }

        // Update min/max FPS
        if (FrameCount == 1)
        {
            CurrentMetrics.MinFPS = CurrentMetrics.CurrentFPS;
            CurrentMetrics.MaxFPS = CurrentMetrics.CurrentFPS;
        }
        else
        {
            CurrentMetrics.MinFPS = FMath::Min(CurrentMetrics.MinFPS, CurrentMetrics.CurrentFPS);
            CurrentMetrics.MaxFPS = FMath::Max(CurrentMetrics.MaxFPS, CurrentMetrics.CurrentFPS);
        }
    }
}

void APerf_PerformanceMonitor::UpdateRenderMetrics()
{
    // Get render thread timing (approximate)
    CurrentMetrics.RenderThreadTime = CurrentMetrics.FrameTime * 0.6f; // Estimate
    CurrentMetrics.GameThreadTime = CurrentMetrics.FrameTime * 0.4f;   // Estimate
    CurrentMetrics.GPUTime = CurrentMetrics.FrameTime * 0.8f;          // Estimate

    // Get draw calls and triangle count (these are estimates in shipping builds)
    CurrentMetrics.DrawCalls = 500 + FMath::RandRange(-100, 200); // Simulated for now
    CurrentMetrics.Triangles = 100000 + FMath::RandRange(-20000, 50000); // Simulated for now
}

void APerf_PerformanceMonitor::UpdateMemoryMetrics()
{
    // Get memory usage (approximate)
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.UsedMemoryMB = static_cast<float>(MemStats.UsedPhysical) / (1024.0f * 1024.0f);
}

void APerf_PerformanceMonitor::DeterminePerformanceLevel()
{
    EPerf_PerformanceLevel OldLevel = CurrentMetrics.PerformanceLevel;

    if (CurrentMetrics.CurrentFPS >= 60.0f)
    {
        CurrentMetrics.PerformanceLevel = EPerf_PerformanceLevel::Excellent;
    }
    else if (CurrentMetrics.CurrentFPS >= 45.0f)
    {
        CurrentMetrics.PerformanceLevel = EPerf_PerformanceLevel::Good;
    }
    else if (CurrentMetrics.CurrentFPS >= 30.0f)
    {
        CurrentMetrics.PerformanceLevel = EPerf_PerformanceLevel::Fair;
    }
    else if (CurrentMetrics.CurrentFPS >= 15.0f)
    {
        CurrentMetrics.PerformanceLevel = EPerf_PerformanceLevel::Poor;
    }
    else
    {
        CurrentMetrics.PerformanceLevel = EPerf_PerformanceLevel::Critical;
    }

    // Trigger events if performance level changed
    if (OldLevel != CurrentMetrics.PerformanceLevel)
    {
        OnPerformanceLevelChanged(CurrentMetrics.PerformanceLevel);
        
        if (CurrentMetrics.PerformanceLevel == EPerf_PerformanceLevel::Critical)
        {
            OnCriticalPerformance(CurrentMetrics.CurrentFPS);
            UE_LOG(LogTemp, Warning, TEXT("CRITICAL PERFORMANCE: FPS dropped to %.1f"), CurrentMetrics.CurrentFPS);
        }
    }
}

void APerf_PerformanceMonitor::ApplyAutoOptimizations()
{
    if (!bAutoOptimize) return;

    UWorld* World = GetWorld();
    if (!World) return;

    // Apply optimizations based on performance level
    switch (CurrentMetrics.PerformanceLevel)
    {
        case EPerf_PerformanceLevel::Critical:
        case EPerf_PerformanceLevel::Poor:
            // Aggressive optimizations
            if (GEngine && GEngine->GetGameViewport())
            {
                // Reduce view distance
                OptimizationSettings.ViewDistanceScale = 0.6f;
                
                // Increase LOD bias
                OptimizationSettings.LODBias = 2.0f;
                
                // Reduce shadow resolution
                OptimizationSettings.MaxShadowResolution = 1024;
                
                // Enable dynamic resolution
                OptimizationSettings.bDynamicResolution = true;
                
                ApplyOptimizationSettings();
                
                UE_LOG(LogTemp, Log, TEXT("Applied aggressive performance optimizations"));
            }
            break;

        case EPerf_PerformanceLevel::Fair:
            // Moderate optimizations
            OptimizationSettings.ViewDistanceScale = 0.8f;
            OptimizationSettings.LODBias = 1.0f;
            OptimizationSettings.MaxShadowResolution = 1536;
            ApplyOptimizationSettings();
            break;

        case EPerf_PerformanceLevel::Good:
        case EPerf_PerformanceLevel::Excellent:
            // Restore quality settings
            OptimizationSettings.ViewDistanceScale = 1.0f;
            OptimizationSettings.LODBias = 0.0f;
            OptimizationSettings.MaxShadowResolution = 2048;
            OptimizationSettings.bDynamicResolution = false;
            ApplyOptimizationSettings();
            break;
    }
}

void APerf_PerformanceMonitor::ApplyOptimizationSettings()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Apply console commands for optimization
    if (GEngine)
    {
        // View distance scaling
        FString ViewDistanceCmd = FString::Printf(TEXT("r.ViewDistanceScale %f"), OptimizationSettings.ViewDistanceScale);
        GEngine->Exec(World, *ViewDistanceCmd);

        // LOD bias
        FString LODBiasCmd = FString::Printf(TEXT("r.StaticMeshLODBias %f"), OptimizationSettings.LODBias);
        GEngine->Exec(World, *LODBiasCmd);

        // Shadow resolution
        FString ShadowResCmd = FString::Printf(TEXT("r.Shadow.MaxResolution %d"), OptimizationSettings.MaxShadowResolution);
        GEngine->Exec(World, *ShadowResCmd);

        // Dynamic resolution
        if (OptimizationSettings.bDynamicResolution)
        {
            GEngine->Exec(World, TEXT("r.ScreenPercentage.Mode 1"));
            GEngine->Exec(World, TEXT("r.ScreenPercentage.MinResolution 50"));
        }
        else
        {
            GEngine->Exec(World, TEXT("r.ScreenPercentage.Mode 0"));
            GEngine->Exec(World, TEXT("r.ScreenPercentage 100"));
        }

        // Culling settings
        if (OptimizationSettings.bEnableDistanceCulling)
        {
            FString MaxDrawDistanceCmd = FString::Printf(TEXT("r.MaxDrawDistance %f"), OptimizationSettings.MaxDrawDistance);
            GEngine->Exec(World, *MaxDrawDistanceCmd);
        }
    }
}

void APerf_PerformanceMonitor::ResetMetrics()
{
    CurrentMetrics = FPerf_PerformanceMetrics();
    FPSHistory.Empty();
    TotalFPSSum = 0.0f;
    FrameCount = 0;
    TimeSinceLastUpdate = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Performance metrics reset"));
}

void APerf_PerformanceMonitor::SetTargetFPS(float NewTargetFPS)
{
    OptimizationSettings.TargetFPS = FMath::Clamp(NewTargetFPS, 15.0f, 120.0f);
    UE_LOG(LogTemp, Log, TEXT("Target FPS set to %.1f"), OptimizationSettings.TargetFPS);
}

void APerf_PerformanceMonitor::EnableAutoOptimization(bool bEnable)
{
    bAutoOptimize = bEnable;
    UE_LOG(LogTemp, Log, TEXT("Auto-optimization %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void APerf_PerformanceMonitor::DisplayMetricsOnScreen()
{
    if (!GEngine) return;

    FString PerformanceLevelStr;
    FColor DisplayColor = FColor::Green;

    switch (CurrentMetrics.PerformanceLevel)
    {
        case EPerf_PerformanceLevel::Excellent:
            PerformanceLevelStr = TEXT("EXCELLENT");
            DisplayColor = FColor::Green;
            break;
        case EPerf_PerformanceLevel::Good:
            PerformanceLevelStr = TEXT("GOOD");
            DisplayColor = FColor::Yellow;
            break;
        case EPerf_PerformanceLevel::Fair:
            PerformanceLevelStr = TEXT("FAIR");
            DisplayColor = FColor::Orange;
            break;
        case EPerf_PerformanceLevel::Poor:
            PerformanceLevelStr = TEXT("POOR");
            DisplayColor = FColor::Red;
            break;
        case EPerf_PerformanceLevel::Critical:
            PerformanceLevelStr = TEXT("CRITICAL");
            DisplayColor = FColor::Red;
            break;
    }

    // Display performance info on screen
    FString DisplayText = FString::Printf(
        TEXT("PERFORMANCE: %s | FPS: %.1f (Avg: %.1f) | Frame: %.1fms | Memory: %.0fMB"),
        *PerformanceLevelStr,
        CurrentMetrics.CurrentFPS,
        CurrentMetrics.AverageFPS,
        CurrentMetrics.FrameTime,
        CurrentMetrics.UsedMemoryMB
    );

    GEngine->AddOnScreenDebugMessage(-1, MonitoringInterval + 0.1f, DisplayColor, DisplayText);
}