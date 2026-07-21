#include "Eng_PerformanceOptimizer.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformMemory.h"

void UEng_PerformanceOptimizer::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // Initialize default settings
    OptimizationSettings = FEng_OptimizationSettings();
    bAutomaticOptimizationEnabled = true;
    PerformanceThresholdFPS = 30.0f;
    PerformanceThresholdFrameTime = 33.33f; // 30 FPS
    
    // Initialize metrics
    CurrentMetrics = FEng_PerformanceMetrics();
    FPSHistory.Reserve(300); // 5 minutes at 60fps
    
    UE_LOG(LogTemp, Warning, TEXT("Performance Optimizer System Initialized"));
}

void UEng_PerformanceOptimizer::Deinitialize()
{
    FPSHistory.Empty();
    Super::Deinitialize();
}

FEng_PerformanceMetrics UEng_PerformanceOptimizer::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

void UEng_PerformanceOptimizer::UpdatePerformanceMetrics()
{
    CollectPerformanceData();
    
    if (bAutomaticOptimizationEnabled && !IsPerformanceTargetMet())
    {
        ApplyAutomaticOptimizations();
    }
}

bool UEng_PerformanceOptimizer::IsPerformanceTargetMet() const
{
    return CurrentMetrics.CurrentFPS >= OptimizationSettings.TargetFPS * 0.9f;
}

void UEng_PerformanceOptimizer::SetOptimizationSettings(const FEng_OptimizationSettings& NewSettings)
{
    OptimizationSettings = NewSettings;
    UE_LOG(LogTemp, Warning, TEXT("Optimization settings updated: Target FPS %.1f"), OptimizationSettings.TargetFPS);
}

FEng_OptimizationSettings UEng_PerformanceOptimizer::GetOptimizationSettings() const
{
    return OptimizationSettings;
}

void UEng_PerformanceOptimizer::EnableAutomaticOptimization(bool bEnable)
{
    bAutomaticOptimizationEnabled = bEnable;
    UE_LOG(LogTemp, Warning, TEXT("Automatic optimization %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UEng_PerformanceOptimizer::OptimizeForTarget(float TargetFPS)
{
    OptimizationSettings.TargetFPS = TargetFPS;
    
    UE_LOG(LogTemp, Warning, TEXT("Optimizing for target FPS: %.1f"), TargetFPS);
    
    // Apply optimizations based on current performance
    if (CurrentMetrics.CurrentFPS < TargetFPS)
    {
        ApplyLODOptimization();
        ApplyCullingOptimization();
        OptimizeMemoryUsage();
    }
}

void UEng_PerformanceOptimizer::ApplyLODOptimization()
{
    if (!OptimizationSettings.bEnableAutomaticLOD)
        return;
        
    UE_LOG(LogTemp, Log, TEXT("Applying LOD optimization"));
    
    // Adjust LOD bias based on performance
    float LODBias = 0.0f;
    if (CurrentMetrics.CurrentFPS < OptimizationSettings.TargetFPS * 0.8f)
    {
        LODBias = 1.0f; // Force higher LOD levels
    }
    else if (CurrentMetrics.CurrentFPS < OptimizationSettings.TargetFPS * 0.9f)
    {
        LODBias = 0.5f; // Moderate LOD adjustment
    }
    
    // Apply LOD bias (this would normally interface with rendering settings)
    UE_LOG(LogTemp, Log, TEXT("LOD bias set to: %.1f"), LODBias);
}

void UEng_PerformanceOptimizer::ApplyCullingOptimization()
{
    if (!OptimizationSettings.bEnableCulling)
        return;
        
    UE_LOG(LogTemp, Log, TEXT("Applying culling optimization"));
    
    // Adjust culling distance based on performance
    float CullingMultiplier = 1.0f;
    if (CurrentMetrics.CurrentFPS < OptimizationSettings.TargetFPS * 0.8f)
    {
        CullingMultiplier = 0.7f; // Reduce culling distance
    }
    else if (CurrentMetrics.CurrentFPS < OptimizationSettings.TargetFPS * 0.9f)
    {
        CullingMultiplier = 0.85f; // Slightly reduce culling distance
    }
    
    float AdjustedCullingDistance = OptimizationSettings.CullingDistance * CullingMultiplier;
    UE_LOG(LogTemp, Log, TEXT("Culling distance adjusted to: %.1f"), AdjustedCullingDistance);
}

void UEng_PerformanceOptimizer::SetPerformanceThreshold(float MinFPS, float MaxFrameTime)
{
    PerformanceThresholdFPS = MinFPS;
    PerformanceThresholdFrameTime = MaxFrameTime;
    
    UE_LOG(LogTemp, Warning, TEXT("Performance thresholds set: %.1f FPS, %.2fms frame time"), MinFPS, MaxFrameTime);
}

bool UEng_PerformanceOptimizer::IsPerformanceBelowThreshold() const
{
    return CurrentMetrics.CurrentFPS < PerformanceThresholdFPS || 
           CurrentMetrics.FrameTime > PerformanceThresholdFrameTime;
}

void UEng_PerformanceOptimizer::OptimizeMemoryUsage()
{
    UE_LOG(LogTemp, Log, TEXT("Optimizing memory usage"));
    
    // Force garbage collection if memory usage is high
    if (CurrentMetrics.MemoryUsageMB > OptimizationSettings.MemoryLimitMB * 0.8f)
    {
        GEngine->ForceGarbageCollection(true);
        UE_LOG(LogTemp, Warning, TEXT("Forced garbage collection due to high memory usage"));
    }
    
    // Additional memory optimizations could be applied here
}

float UEng_PerformanceOptimizer::GetMemoryUsageMB() const
{
    return CurrentMetrics.MemoryUsageMB;
}

void UEng_PerformanceOptimizer::CollectPerformanceData()
{
    // Get current FPS
    if (UWorld* World = GEngine->GetWorld())
    {
        float DeltaTime = World->GetDeltaSeconds();
        if (DeltaTime > 0.0f)
        {
            CurrentMetrics.CurrentFPS = 1.0f / DeltaTime;
            CurrentMetrics.FrameTime = DeltaTime * 1000.0f; // Convert to milliseconds
            
            UpdateFPSHistory(CurrentMetrics.CurrentFPS);
            CurrentMetrics.AverageFPS = CalculateAverageFPS();
            
            // Update min/max FPS
            if (CurrentMetrics.CurrentFPS < CurrentMetrics.MinFPS)
            {
                CurrentMetrics.MinFPS = CurrentMetrics.CurrentFPS;
            }
            if (CurrentMetrics.CurrentFPS > CurrentMetrics.MaxFPS)
            {
                CurrentMetrics.MaxFPS = CurrentMetrics.CurrentFPS;
            }
        }
    }
    
    // Get memory usage
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    
    // Simulate draw calls and triangle count (would normally come from renderer)
    CurrentMetrics.DrawCalls = FMath::RandRange(800, 1500);
    CurrentMetrics.TriangleCount = FMath::RandRange(500000, 900000);
}

void UEng_PerformanceOptimizer::ApplyAutomaticOptimizations()
{
    UE_LOG(LogTemp, Log, TEXT("Applying automatic optimizations - Current FPS: %.1f, Target: %.1f"), 
           CurrentMetrics.CurrentFPS, OptimizationSettings.TargetFPS);
    
    // Apply optimizations in order of impact
    if (CurrentMetrics.CurrentFPS < OptimizationSettings.TargetFPS * 0.7f)
    {
        // Severe performance issues - apply aggressive optimizations
        ApplyLODOptimization();
        ApplyCullingOptimization();
        OptimizeMemoryUsage();
    }
    else if (CurrentMetrics.CurrentFPS < OptimizationSettings.TargetFPS * 0.9f)
    {
        // Moderate performance issues - apply conservative optimizations
        ApplyLODOptimization();
    }
}

void UEng_PerformanceOptimizer::UpdateFPSHistory(float FPS)
{
    FPSHistory.Add(FPS);
    
    // Keep only last 300 samples (5 minutes at 60fps)
    if (FPSHistory.Num() > 300)
    {
        FPSHistory.RemoveAt(0);
    }
}

float UEng_PerformanceOptimizer::CalculateAverageFPS() const
{
    if (FPSHistory.Num() == 0)
        return 0.0f;
        
    float Total = 0.0f;
    for (float FPS : FPSHistory)
    {
        Total += FPS;
    }
    
    return Total / FPSHistory.Num();
}