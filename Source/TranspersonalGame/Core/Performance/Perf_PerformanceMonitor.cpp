#include "Perf_PerformanceMonitor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Stats/Stats.h"
#include "RenderingThread.h"
#include "RHI.h"
#include "Engine/GameViewportClient.h"
#include "Kismet/GameplayStatics.h"

APerf_PerformanceMonitor::APerf_PerformanceMonitor()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
    
    MonitoringInterval = 1.0f;
    bEnableLogging = true;
    bEnableAutomaticOptimization = true;
    bShowDebugInfo = false;
    
    TotalFPSSum = 0.0f;
    FPSMeasurements = 0;
    LastMonitorTime = 0.0f;
    
    OptimizationSettings = FPerf_OptimizationSettings();
    CurrentMetrics = FPerf_PerformanceMetrics();
    
    FPSHistory.Reserve(60); // Store last 60 measurements
}

void APerf_PerformanceMonitor::BeginPlay()
{
    Super::BeginPlay();
    
    if (bEnableLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("Performance Monitor started - Target FPS: %.1f"), OptimizationSettings.TargetFPS);
    }
    
    StartMonitoring();
}

void APerf_PerformanceMonitor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopMonitoring();
    Super::EndPlay(EndPlayReason);
}

void APerf_PerformanceMonitor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bShowDebugInfo && GEngine)
    {
        FString DebugString = FString::Printf(
            TEXT("FPS: %.1f (Avg: %.1f) | Game: %.2fms | Render: %.2fms | GPU: %.2fms | Actors: %d"),
            CurrentMetrics.CurrentFPS,
            CurrentMetrics.AverageFPS,
            CurrentMetrics.GameThreadTime,
            CurrentMetrics.RenderThreadTime,
            CurrentMetrics.GPUTime,
            CurrentMetrics.ActiveActors
        );
        
        GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Green, DebugString);
    }
}

void APerf_PerformanceMonitor::StartMonitoring()
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            MonitoringTimerHandle,
            this,
            &APerf_PerformanceMonitor::UpdatePerformanceMetrics,
            MonitoringInterval,
            true
        );
        
        if (bEnableLogging)
        {
            UE_LOG(LogTemp, Log, TEXT("Performance monitoring started with interval: %.2f seconds"), MonitoringInterval);
        }
    }
}

void APerf_PerformanceMonitor::StopMonitoring()
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(MonitoringTimerHandle);
        
        if (bEnableLogging)
        {
            UE_LOG(LogTemp, Log, TEXT("Performance monitoring stopped"));
        }
    }
}

void APerf_PerformanceMonitor::UpdatePerformanceMetrics()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Calculate FPS
    float DeltaTime = GetWorld()->GetDeltaSeconds();
    if (DeltaTime > 0.0f)
    {
        CurrentMetrics.CurrentFPS = 1.0f / DeltaTime;
        UpdateFPSHistory(CurrentMetrics.CurrentFPS);
        
        // Update min/max FPS
        CurrentMetrics.MinFPS = FMath::Min(CurrentMetrics.MinFPS, CurrentMetrics.CurrentFPS);
        CurrentMetrics.MaxFPS = FMath::Max(CurrentMetrics.MaxFPS, CurrentMetrics.CurrentFPS);
        
        // Calculate average FPS
        CurrentMetrics.AverageFPS = CalculateAverageFPS();
    }
    
    // Get timing stats
    CurrentMetrics.GameThreadTime = FPlatformTime::ToMilliseconds(GGameThreadTime);
    CurrentMetrics.RenderThreadTime = FPlatformTime::ToMilliseconds(GRenderThreadTime);
    CurrentMetrics.GPUTime = FPlatformTime::ToMilliseconds(GGPUFrameTime);
    
    // Count actors
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    CurrentMetrics.ActiveActors = AllActors.Num();
    
    // Estimate visible actors (simplified)
    CurrentMetrics.VisibleActors = CurrentMetrics.ActiveActors; // TODO: Implement proper visibility culling check
    
    // Get memory usage (simplified)
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    
    // Estimate draw calls and triangles (simplified)
    CurrentMetrics.DrawCalls = CurrentMetrics.VisibleActors * 2; // Rough estimate
    CurrentMetrics.Triangles = CurrentMetrics.VisibleActors * 1000; // Rough estimate
    
    // Check performance thresholds
    CheckPerformanceThresholds();
    
    // Log performance data if enabled
    if (bEnableLogging)
    {
        LogPerformanceData();
    }
    
    // Apply automatic optimizations if enabled
    if (bEnableAutomaticOptimization)
    {
        ApplyAutomaticOptimizations();
    }
}

void APerf_PerformanceMonitor::CheckPerformanceThresholds()
{
    if (CurrentMetrics.CurrentFPS < OptimizationSettings.MinAcceptableFPS)
    {
        if (bEnableLogging)
        {
            UE_LOG(LogTemp, Warning, TEXT("Performance threshold exceeded! FPS: %.1f (Min: %.1f)"), 
                CurrentMetrics.CurrentFPS, OptimizationSettings.MinAcceptableFPS);
        }
        
        OnPerformanceThresholdExceeded(CurrentMetrics);
    }
    
    if (CurrentMetrics.MemoryUsageMB > OptimizationSettings.MaxMemoryUsageMB)
    {
        if (bEnableLogging)
        {
            UE_LOG(LogTemp, Warning, TEXT("Memory threshold exceeded! Usage: %.1f MB (Max: %.1f MB)"), 
                CurrentMetrics.MemoryUsageMB, OptimizationSettings.MaxMemoryUsageMB);
        }
    }
    
    if (CurrentMetrics.DrawCalls > OptimizationSettings.MaxDrawCalls)
    {
        if (bEnableLogging)
        {
            UE_LOG(LogTemp, Warning, TEXT("Draw call threshold exceeded! Calls: %d (Max: %d)"), 
                CurrentMetrics.DrawCalls, OptimizationSettings.MaxDrawCalls);
        }
    }
}

void APerf_PerformanceMonitor::LogPerformanceData()
{
    static float LastLogTime = 0.0f;
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Log every 10 seconds to avoid spam
    if (CurrentTime - LastLogTime >= 10.0f)
    {
        UE_LOG(LogTemp, Log, TEXT("Performance Report - FPS: %.1f (%.1f avg) | Memory: %.1f MB | Actors: %d | Draw Calls: %d"),
            CurrentMetrics.CurrentFPS,
            CurrentMetrics.AverageFPS,
            CurrentMetrics.MemoryUsageMB,
            CurrentMetrics.ActiveActors,
            CurrentMetrics.DrawCalls
        );
        
        LastLogTime = CurrentTime;
    }
}

void APerf_PerformanceMonitor::ApplyAutomaticOptimizations()
{
    if (CurrentMetrics.CurrentFPS < OptimizationSettings.MinAcceptableFPS)
    {
        // Determine optimization level based on how far below target we are
        float PerformanceRatio = CurrentMetrics.CurrentFPS / OptimizationSettings.TargetFPS;
        
        if (PerformanceRatio < 0.5f)
        {
            SetOptimizationLevel(3); // Aggressive optimization
        }
        else if (PerformanceRatio < 0.7f)
        {
            SetOptimizationLevel(2); // Moderate optimization
        }
        else if (PerformanceRatio < 0.9f)
        {
            SetOptimizationLevel(1); // Light optimization
        }
    }
    else if (CurrentMetrics.CurrentFPS > OptimizationSettings.TargetFPS * 1.2f)
    {
        SetOptimizationLevel(0); // Restore quality
    }
}

float APerf_PerformanceMonitor::CalculateAverageFPS() const
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

void APerf_PerformanceMonitor::UpdateFPSHistory(float NewFPS)
{
    FPSHistory.Add(NewFPS);
    
    // Keep only last 60 measurements (1 minute at 1Hz monitoring)
    if (FPSHistory.Num() > 60)
    {
        FPSHistory.RemoveAt(0);
    }
}

FPerf_PerformanceMetrics APerf_PerformanceMonitor::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

void APerf_PerformanceMonitor::ApplyOptimizations()
{
    if (bEnableLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("Applying performance optimizations..."));
    }
    
    // Apply various optimization techniques
    if (OptimizationSettings.bEnableAutomaticLOD)
    {
        // Adjust LOD distance scale
        if (GEngine && GEngine->GetGameUserSettings())
        {
            // Implementation would go here for LOD adjustments
        }
    }
    
    // Force garbage collection if memory usage is high
    if (CurrentMetrics.MemoryUsageMB > OptimizationSettings.MaxMemoryUsageMB * 0.8f)
    {
        GEngine->ForceGarbageCollection(true);
        
        if (bEnableLogging)
        {
            UE_LOG(LogTemp, Log, TEXT("Forced garbage collection due to high memory usage"));
        }
    }
}

void APerf_PerformanceMonitor::ResetMetrics()
{
    CurrentMetrics = FPerf_PerformanceMetrics();
    FPSHistory.Empty();
    TotalFPSSum = 0.0f;
    FPSMeasurements = 0;
    
    if (bEnableLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("Performance metrics reset"));
    }
}

bool APerf_PerformanceMonitor::IsPerformanceAcceptable() const
{
    return CurrentMetrics.CurrentFPS >= OptimizationSettings.MinAcceptableFPS &&
           CurrentMetrics.MemoryUsageMB <= OptimizationSettings.MaxMemoryUsageMB &&
           CurrentMetrics.DrawCalls <= OptimizationSettings.MaxDrawCalls;
}

void APerf_PerformanceMonitor::SetOptimizationLevel(int32 Level)
{
    if (bEnableLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("Setting optimization level to: %d"), Level);
    }
    
    // Apply optimization settings based on level
    switch (Level)
    {
        case 0: // High Quality
            OptimizationSettings.LODDistanceScale = 1.0f;
            break;
            
        case 1: // Medium Quality
            OptimizationSettings.LODDistanceScale = 0.8f;
            break;
            
        case 2: // Low Quality
            OptimizationSettings.LODDistanceScale = 0.6f;
            break;
            
        case 3: // Minimum Quality
            OptimizationSettings.LODDistanceScale = 0.4f;
            break;
            
        default:
            Level = 0;
            break;
    }
    
    OnOptimizationApplied(Level);
}