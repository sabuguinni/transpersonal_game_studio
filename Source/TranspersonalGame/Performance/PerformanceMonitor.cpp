#include "PerformanceMonitor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"
#include "RenderingThread.h"
#include "Engine/GameViewportClient.h"

APerformanceMonitor::APerformanceMonitor()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Initialize default values
    UpdateInterval = 1.0f;
    bEnableAutoQualityAdjustment = true;
    TargetFPS = 60.0f;
    MinAcceptableFPS = 30.0f;
    TimeSinceLastUpdate = 0.0f;
    MaxHistorySize = 60;
    
    // Reserve space for frame history
    FrameTimeHistory.Reserve(MaxHistorySize);
}

void APerformanceMonitor::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("PerformanceMonitor: Starting performance monitoring"));
    
    // Initialize metrics
    UpdatePerformanceMetrics();
    
    // Enable basic performance stats
    EnablePerformanceStats(true);
}

void APerformanceMonitor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Add current frame time to history
    FrameTimeHistory.Add(DeltaTime);
    if (FrameTimeHistory.Num() > MaxHistorySize)
    {
        FrameTimeHistory.RemoveAt(0);
    }
    
    TimeSinceLastUpdate += DeltaTime;
    
    // Update metrics at specified interval
    if (TimeSinceLastUpdate >= UpdateInterval)
    {
        UpdatePerformanceMetrics();
        
        if (bEnableAutoQualityAdjustment)
        {
            AdjustQualityBasedOnPerformance();
        }
        
        TimeSinceLastUpdate = 0.0f;
    }
}

void APerformanceMonitor::UpdatePerformanceMetrics()
{
    // Calculate current FPS
    float AverageFrameTime = CalculateAverageFrameTime();
    CurrentMetrics.CurrentFPS = (AverageFrameTime > 0.0f) ? (1.0f / AverageFrameTime) : 0.0f;
    CurrentMetrics.AverageFrameTime = AverageFrameTime * 1000.0f; // Convert to milliseconds
    
    // Estimate GPU frame time (simplified)
    CurrentMetrics.GPUFrameTime = CurrentMetrics.AverageFrameTime * 0.7f; // Rough estimate
    
    // Get memory usage (simplified)
    CurrentMetrics.MemoryUsageMB = FPlatformMemory::GetStats().UsedPhysical / (1024.0f * 1024.0f);
    
    // Estimate draw calls and triangles (placeholder values)
    CurrentMetrics.DrawCalls = FMath::RandRange(50, 200);
    CurrentMetrics.TriangleCount = FMath::RandRange(10000, 100000);
}

void APerformanceMonitor::AdjustQualityBasedOnPerformance()
{
    if (CurrentMetrics.CurrentFPS < MinAcceptableFPS)
    {
        // Performance is poor, reduce quality
        if (CurrentMetrics.QualityLevel == EPerf_PerformanceLevel::Ultra)
        {
            SetQualityLevel(EPerf_PerformanceLevel::High);
        }
        else if (CurrentMetrics.QualityLevel == EPerf_PerformanceLevel::High)
        {
            SetQualityLevel(EPerf_PerformanceLevel::Medium);
        }
        else if (CurrentMetrics.QualityLevel == EPerf_PerformanceLevel::Medium)
        {
            SetQualityLevel(EPerf_PerformanceLevel::Low);
        }
    }
    else if (CurrentMetrics.CurrentFPS > TargetFPS + 10.0f)
    {
        // Performance is good, can increase quality
        if (CurrentMetrics.QualityLevel == EPerf_PerformanceLevel::Low)
        {
            SetQualityLevel(EPerf_PerformanceLevel::Medium);
        }
        else if (CurrentMetrics.QualityLevel == EPerf_PerformanceLevel::Medium)
        {
            SetQualityLevel(EPerf_PerformanceLevel::High);
        }
        else if (CurrentMetrics.QualityLevel == EPerf_PerformanceLevel::High)
        {
            SetQualityLevel(EPerf_PerformanceLevel::Ultra);
        }
    }
}

void APerformanceMonitor::ApplyQualitySettings(EPerf_PerformanceLevel Level)
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    switch (Level)
    {
        case EPerf_PerformanceLevel::Low:
            if (GEngine && GEngine->GameViewport)
            {
                GEngine->Exec(World, TEXT("r.ScreenPercentage 75"));
                GEngine->Exec(World, TEXT("r.ShadowQuality 1"));
                GEngine->Exec(World, TEXT("r.ViewDistanceScale 0.6"));
                GEngine->Exec(World, TEXT("r.PostProcessAAQuality 2"));
            }
            break;
            
        case EPerf_PerformanceLevel::Medium:
            if (GEngine && GEngine->GameViewport)
            {
                GEngine->Exec(World, TEXT("r.ScreenPercentage 85"));
                GEngine->Exec(World, TEXT("r.ShadowQuality 2"));
                GEngine->Exec(World, TEXT("r.ViewDistanceScale 0.8"));
                GEngine->Exec(World, TEXT("r.PostProcessAAQuality 3"));
            }
            break;
            
        case EPerf_PerformanceLevel::High:
            if (GEngine && GEngine->GameViewport)
            {
                GEngine->Exec(World, TEXT("r.ScreenPercentage 100"));
                GEngine->Exec(World, TEXT("r.ShadowQuality 3"));
                GEngine->Exec(World, TEXT("r.ViewDistanceScale 1.0"));
                GEngine->Exec(World, TEXT("r.PostProcessAAQuality 4"));
            }
            break;
            
        case EPerf_PerformanceLevel::Ultra:
            if (GEngine && GEngine->GameViewport)
            {
                GEngine->Exec(World, TEXT("r.ScreenPercentage 120"));
                GEngine->Exec(World, TEXT("r.ShadowQuality 4"));
                GEngine->Exec(World, TEXT("r.ViewDistanceScale 1.2"));
                GEngine->Exec(World, TEXT("r.PostProcessAAQuality 6"));
            }
            break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("PerformanceMonitor: Applied quality level %d"), (int32)Level);
}

float APerformanceMonitor::CalculateAverageFrameTime() const
{
    if (FrameTimeHistory.Num() == 0) return 0.0f;
    
    float Total = 0.0f;
    for (float FrameTime : FrameTimeHistory)
    {
        Total += FrameTime;
    }
    
    return Total / FrameTimeHistory.Num();
}

FPerf_PerformanceMetrics APerformanceMonitor::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

void APerformanceMonitor::SetQualityLevel(EPerf_PerformanceLevel NewLevel)
{
    if (CurrentMetrics.QualityLevel != NewLevel)
    {
        CurrentMetrics.QualityLevel = NewLevel;
        ApplyQualitySettings(NewLevel);
        
        UE_LOG(LogTemp, Warning, TEXT("PerformanceMonitor: Quality level changed to %d"), (int32)NewLevel);
    }
}

void APerformanceMonitor::EnablePerformanceStats(bool bEnable)
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    if (bEnable)
    {
        if (GEngine && GEngine->GameViewport)
        {
            GEngine->Exec(World, TEXT("stat fps"));
            GEngine->Exec(World, TEXT("stat unit"));
            GEngine->Exec(World, TEXT("stat memory"));
        }
        UE_LOG(LogTemp, Warning, TEXT("PerformanceMonitor: Performance stats enabled"));
    }
    else
    {
        if (GEngine && GEngine->GameViewport)
        {
            GEngine->Exec(World, TEXT("stat none"));
        }
        UE_LOG(LogTemp, Warning, TEXT("PerformanceMonitor: Performance stats disabled"));
    }
}

void APerformanceMonitor::LogPerformanceReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PERFORMANCE REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Current FPS: %.2f"), CurrentMetrics.CurrentFPS);
    UE_LOG(LogTemp, Warning, TEXT("Frame Time: %.2f ms"), CurrentMetrics.AverageFrameTime);
    UE_LOG(LogTemp, Warning, TEXT("GPU Time: %.2f ms"), CurrentMetrics.GPUFrameTime);
    UE_LOG(LogTemp, Warning, TEXT("Memory: %.2f MB"), CurrentMetrics.MemoryUsageMB);
    UE_LOG(LogTemp, Warning, TEXT("Draw Calls: %d"), CurrentMetrics.DrawCalls);
    UE_LOG(LogTemp, Warning, TEXT("Triangles: %d"), CurrentMetrics.TriangleCount);
    UE_LOG(LogTemp, Warning, TEXT("Quality Level: %d"), (int32)CurrentMetrics.QualityLevel);
    UE_LOG(LogTemp, Warning, TEXT("========================"));
}

void APerformanceMonitor::TestPerformanceSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("PerformanceMonitor: Testing performance system..."));
    
    UpdatePerformanceMetrics();
    LogPerformanceReport();
    
    // Test quality level changes
    SetQualityLevel(EPerf_PerformanceLevel::Low);
    FPlatformProcess::Sleep(1.0f);
    SetQualityLevel(EPerf_PerformanceLevel::High);
    
    UE_LOG(LogTemp, Warning, TEXT("PerformanceMonitor: Test complete"));
}