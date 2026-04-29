#include "PerformanceMonitor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Stats/Stats.h"
#include "RenderingThread.h"
#include "Engine/GameViewportClient.h"
#include "Misc/App.h"
#include "HAL/PlatformMemory.h"
#include "Engine/Level.h"

APerf_PerformanceMonitor::APerf_PerformanceMonitor()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Initialize default values
    MonitoringInterval = 1.0f;
    bEnableAutoOptimization = true;
    TargetPerformanceLevel = EPerf_PerformanceLevel::High;
    MinAcceptableFPS = 30.0f;
    MaxAcceptableFrameTime = 33.33f; // 30 FPS = 33.33ms
    MaxMemoryUsageMB = 2048.0f; // 2GB
    
    TimeSinceLastUpdate = 0.0f;
    LastUpdateTime = FDateTime::Now();
    
    // Initialize current metrics
    CurrentMetrics = FPerf_PerformanceMetrics();
}

void APerf_PerformanceMonitor::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Performance Monitor started - Target: %s"), 
           *UEnum::GetValueAsString(TargetPerformanceLevel));
    
    StartPerformanceMonitoring();
}

void APerf_PerformanceMonitor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    TimeSinceLastUpdate += DeltaTime;
    
    if (TimeSinceLastUpdate >= MonitoringInterval)
    {
        UpdatePerformanceMetrics();
        
        if (bEnableAutoOptimization && !IsPerformanceAcceptable())
        {
            TriggerPerformanceOptimization();
        }
        
        TimeSinceLastUpdate = 0.0f;
    }
}

void APerf_PerformanceMonitor::StartPerformanceMonitoring()
{
    UE_LOG(LogTemp, Log, TEXT("Performance monitoring started"));
    
    // Enable performance stats
    if (GEngine && GEngine->GetGameViewport())
    {
        GEngine->Exec(GetWorld(), TEXT("stat fps"));
        GEngine->Exec(GetWorld(), TEXT("stat unit"));
    }
    
    UpdatePerformanceMetrics();
}

void APerf_PerformanceMonitor::StopPerformanceMonitoring()
{
    UE_LOG(LogTemp, Log, TEXT("Performance monitoring stopped"));
    
    // Disable performance stats
    if (GEngine && GEngine->GetGameViewport())
    {
        GEngine->Exec(GetWorld(), TEXT("stat fps 0"));
        GEngine->Exec(GetWorld(), TEXT("stat unit 0"));
    }
}

FPerf_PerformanceMetrics APerf_PerformanceMonitor::GetCurrentPerformanceMetrics()
{
    UpdatePerformanceMetrics();
    return CurrentMetrics;
}

bool APerf_PerformanceMonitor::IsPerformanceAcceptable()
{
    return (CurrentMetrics.CurrentFPS >= MinAcceptableFPS &&
            CurrentMetrics.AverageFrameTime <= MaxAcceptableFrameTime &&
            CurrentMetrics.MemoryUsageMB <= MaxMemoryUsageMB);
}

void APerf_PerformanceMonitor::TriggerPerformanceOptimization()
{
    UE_LOG(LogTemp, Warning, TEXT("Performance below target - triggering optimization"));
    
    AnalyzePerformanceBottlenecks();
    ApplyAutoOptimizations();
    
    LogPerformanceWarning(TEXT("Auto-optimization triggered"));
}

void APerf_PerformanceMonitor::LogPerformanceReport()
{
    UE_LOG(LogTemp, Log, TEXT("=== PERFORMANCE REPORT ==="));
    UE_LOG(LogTemp, Log, TEXT("FPS: %.2f (Target: %.2f)"), CurrentMetrics.CurrentFPS, MinAcceptableFPS);
    UE_LOG(LogTemp, Log, TEXT("Frame Time: %.2f ms"), CurrentMetrics.AverageFrameTime);
    UE_LOG(LogTemp, Log, TEXT("GPU Time: %.2f ms"), CurrentMetrics.GPUFrameTime);
    UE_LOG(LogTemp, Log, TEXT("Draw Calls: %d"), CurrentMetrics.DrawCalls);
    UE_LOG(LogTemp, Log, TEXT("Triangles: %d"), CurrentMetrics.TriangleCount);
    UE_LOG(LogTemp, Log, TEXT("Memory: %.2f MB"), CurrentMetrics.MemoryUsageMB);
    UE_LOG(LogTemp, Log, TEXT("Active Actors: %d"), CurrentMetrics.ActiveActorCount);
    UE_LOG(LogTemp, Log, TEXT("Performance Acceptable: %s"), IsPerformanceAcceptable() ? TEXT("YES") : TEXT("NO"));
}

void APerf_PerformanceMonitor::RunPerformanceTest()
{
    UE_LOG(LogTemp, Log, TEXT("Running performance test..."));
    
    UpdatePerformanceMetrics();
    LogPerformanceReport();
    
    // Store in history
    PerformanceHistory.Add(CurrentMetrics);
    
    // Keep only last 100 entries
    if (PerformanceHistory.Num() > 100)
    {
        PerformanceHistory.RemoveAt(0);
    }
}

void APerf_PerformanceMonitor::UpdatePerformanceMetrics()
{
    // Get FPS
    CurrentMetrics.CurrentFPS = 1.0f / FApp::GetDeltaTime();
    CurrentMetrics.AverageFrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds
    
    // Estimate GPU frame time (simplified)
    CurrentMetrics.GPUFrameTime = CurrentMetrics.AverageFrameTime * 0.6f;
    
    // Get memory usage
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    
    // Count active actors
    if (UWorld* World = GetWorld())
    {
        CurrentMetrics.ActiveActorCount = World->GetCurrentLevel()->Actors.Num();
        
        // Estimate draw calls and triangles (simplified)
        CurrentMetrics.DrawCalls = CurrentMetrics.ActiveActorCount * 2; // Rough estimate
        CurrentMetrics.TriangleCount = CurrentMetrics.ActiveActorCount * 1000; // Rough estimate
    }
    
    // Check if meeting performance target
    CurrentMetrics.bIsPerformanceTarget = IsPerformanceAcceptable();
}

void APerf_PerformanceMonitor::AnalyzePerformanceBottlenecks()
{
    if (CurrentMetrics.CurrentFPS < MinAcceptableFPS)
    {
        if (CurrentMetrics.GPUFrameTime > 20.0f)
        {
            LogPerformanceWarning(TEXT("GPU bottleneck detected - consider reducing visual quality"));
        }
        
        if (CurrentMetrics.DrawCalls > 1000)
        {
            LogPerformanceWarning(TEXT("High draw call count - consider batching or instancing"));
        }
        
        if (CurrentMetrics.TriangleCount > 100000)
        {
            LogPerformanceWarning(TEXT("High triangle count - consider LOD or culling"));
        }
    }
    
    if (CurrentMetrics.MemoryUsageMB > MaxMemoryUsageMB * 0.8f)
    {
        LogPerformanceWarning(TEXT("High memory usage - consider texture streaming or garbage collection"));
    }
}

void APerf_PerformanceMonitor::ApplyAutoOptimizations()
{
    UE_LOG(LogTemp, Log, TEXT("Applying auto-optimizations..."));
    
    // Apply basic optimizations based on performance level
    switch (TargetPerformanceLevel)
    {
        case EPerf_PerformanceLevel::Low:
            if (GEngine) GEngine->Exec(GetWorld(), TEXT("sg.ViewDistanceScale 0.5"));
            if (GEngine) GEngine->Exec(GetWorld(), TEXT("sg.ShadowQuality 0"));
            break;
            
        case EPerf_PerformanceLevel::Medium:
            if (GEngine) GEngine->Exec(GetWorld(), TEXT("sg.ViewDistanceScale 0.7"));
            if (GEngine) GEngine->Exec(GetWorld(), TEXT("sg.ShadowQuality 1"));
            break;
            
        case EPerf_PerformanceLevel::High:
            if (GEngine) GEngine->Exec(GetWorld(), TEXT("sg.ViewDistanceScale 0.9"));
            if (GEngine) GEngine->Exec(GetWorld(), TEXT("sg.ShadowQuality 2"));
            break;
            
        case EPerf_PerformanceLevel::Ultra:
            if (GEngine) GEngine->Exec(GetWorld(), TEXT("sg.ViewDistanceScale 1.0"));
            if (GEngine) GEngine->Exec(GetWorld(), TEXT("sg.ShadowQuality 3"));
            break;
            
        case EPerf_PerformanceLevel::Critical:
            if (GEngine) GEngine->Exec(GetWorld(), TEXT("sg.ViewDistanceScale 0.3"));
            if (GEngine) GEngine->Exec(GetWorld(), TEXT("sg.ShadowQuality 0"));
            if (GEngine) GEngine->Exec(GetWorld(), TEXT("sg.EffectsQuality 0"));
            break;
    }
}

void APerf_PerformanceMonitor::LogPerformanceWarning(const FString& Warning)
{
    UE_LOG(LogTemp, Warning, TEXT("PERFORMANCE WARNING: %s"), *Warning);
    
    // Could trigger UI notifications or other systems here
}