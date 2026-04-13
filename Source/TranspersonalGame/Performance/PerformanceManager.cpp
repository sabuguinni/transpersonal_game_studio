#include "PerformanceManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Stats/Stats.h"
#include "RenderingThread.h"
#include "HAL/IConsoleManager.h"
#include "Engine/GameViewportClient.h"

UPerformanceManager::UPerformanceManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Set default performance targets
    TargetFPS = 60.0f;
    MinAcceptableFPS = 30.0f;
    bAutoQualityAdjustment = true;
    QualityAdjustmentInterval = 2.0f;
    CurrentQualityLevel = EPerf_QualityLevel::High;
    OptimizationStrategy = EPerf_OptimizationStrategy::Balanced;
    
    // Set performance budgets
    MaxDrawCalls = 2000;
    MaxVisibleTriangles = 2000000;
    MaxMemoryUsageMB = 8000.0f;
    MaxFrameTimeMS = 16.67f; // 60 FPS target
    
    // Initialize metrics
    CurrentMetrics = FPerf_PerformanceMetrics();
    LastQualityAdjustmentTime = 0.0f;
    bProfilingActive = false;
}

void UPerformanceManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: Starting performance monitoring"));
    
    // Apply initial quality settings
    ApplyQualitySettings();
    
    // Start performance monitoring
    StartPerformanceProfiling();
}

void UPerformanceManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (bProfilingActive)
    {
        StopPerformanceProfiling();
    }
    
    Super::EndPlay(EndPlayReason);
}

void UPerformanceManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update performance metrics
    UpdatePerformanceMetrics();
    
    // Check performance budgets
    CheckPerformanceBudgets();
    
    // Auto-adjust quality if enabled
    if (bAutoQualityAdjustment)
    {
        AutoAdjustQuality();
    }
    
    // Store profiling data if active
    if (bProfilingActive)
    {
        ProfilingData.Add(CurrentMetrics);
        
        // Limit profiling data size to prevent memory issues
        if (ProfilingData.Num() > 1000)
        {
            ProfilingData.RemoveAt(0, 100);
        }
    }
    
    // Broadcast performance changes
    OnPerformanceChanged.Broadcast(CurrentMetrics);
}

FPerf_PerformanceMetrics UPerformanceManager::GetCurrentPerformanceMetrics() const
{
    return CurrentMetrics;
}

bool UPerformanceManager::IsPerformanceWithinBudget() const
{
    return CurrentMetrics.bIsPerformanceGood;
}

void UPerformanceManager::SetTargetFPS(float NewTargetFPS)
{
    TargetFPS = FMath::Clamp(NewTargetFPS, 15.0f, 120.0f);
    MaxFrameTimeMS = 1000.0f / TargetFPS;
    
    UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: Target FPS set to %.1f (%.2fms frame time)"), TargetFPS, MaxFrameTimeMS);
}

void UPerformanceManager::SetQualityLevel(EPerf_QualityLevel NewQualityLevel)
{
    if (CurrentQualityLevel != NewQualityLevel)
    {
        CurrentQualityLevel = NewQualityLevel;
        ApplyQualitySettings();
        OnQualityLevelChanged.Broadcast(CurrentQualityLevel);
        
        UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: Quality level changed to %d"), (int32)CurrentQualityLevel);
    }
}

void UPerformanceManager::EnableAutoQualityAdjustment(bool bEnable)
{
    bAutoQualityAdjustment = bEnable;
    UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: Auto quality adjustment %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UPerformanceManager::ApplyOptimizationStrategy(EPerf_OptimizationStrategy Strategy)
{
    OptimizationStrategy = Strategy;
    
    switch (Strategy)
    {
        case EPerf_OptimizationStrategy::Aggressive:
            MaxDrawCalls = 1000;
            MaxVisibleTriangles = 1000000;
            MaxMemoryUsageMB = 6000.0f;
            break;
            
        case EPerf_OptimizationStrategy::Balanced:
            MaxDrawCalls = 2000;
            MaxVisibleTriangles = 2000000;
            MaxMemoryUsageMB = 8000.0f;
            break;
            
        case EPerf_OptimizationStrategy::Conservative:
            MaxDrawCalls = 3000;
            MaxVisibleTriangles = 3000000;
            MaxMemoryUsageMB = 10000.0f;
            break;
            
        case EPerf_OptimizationStrategy::Custom:
            // Keep current settings
            break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: Applied %d optimization strategy"), (int32)Strategy);
}

void UPerformanceManager::OptimizeLODSettings()
{
    if (UWorld* World = GetWorld())
    {
        // Apply LOD optimization settings
        IConsoleManager::Get().FindConsoleVariable(TEXT("r.ForceLOD"))->Set(-1); // Auto LOD
        IConsoleManager::Get().FindConsoleVariable(TEXT("r.LODScale"))->Set(1.0f);
        
        UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: LOD settings optimized"));
    }
}

void UPerformanceManager::OptimizeLightingSettings()
{
    // Optimize lighting for performance
    switch (CurrentQualityLevel)
    {
        case EPerf_QualityLevel::Low:
            IConsoleManager::Get().FindConsoleVariable(TEXT("r.Shadow.MaxResolution"))->Set(512);
            IConsoleManager::Get().FindConsoleVariable(TEXT("r.LightMaxDrawDistanceScale"))->Set(0.5f);
            break;
            
        case EPerf_QualityLevel::Medium:
            IConsoleManager::Get().FindConsoleVariable(TEXT("r.Shadow.MaxResolution"))->Set(1024);
            IConsoleManager::Get().FindConsoleVariable(TEXT("r.LightMaxDrawDistanceScale"))->Set(0.75f);
            break;
            
        case EPerf_QualityLevel::High:
            IConsoleManager::Get().FindConsoleVariable(TEXT("r.Shadow.MaxResolution"))->Set(2048);
            IConsoleManager::Get().FindConsoleVariable(TEXT("r.LightMaxDrawDistanceScale"))->Set(1.0f);
            break;
            
        case EPerf_QualityLevel::Ultra:
            IConsoleManager::Get().FindConsoleVariable(TEXT("r.Shadow.MaxResolution"))->Set(4096);
            IConsoleManager::Get().FindConsoleVariable(TEXT("r.LightMaxDrawDistanceScale"))->Set(1.0f);
            break;
            
        default:
            break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: Lighting settings optimized for quality level %d"), (int32)CurrentQualityLevel);
}

void UPerformanceManager::OptimizeRenderingSettings()
{
    // Apply rendering optimizations based on quality level
    switch (CurrentQualityLevel)
    {
        case EPerf_QualityLevel::Low:
            IConsoleManager::Get().FindConsoleVariable(TEXT("r.ScreenPercentage"))->Set(75.0f);
            IConsoleManager::Get().FindConsoleVariable(TEXT("r.PostProcessAAQuality"))->Set(2);
            break;
            
        case EPerf_QualityLevel::Medium:
            IConsoleManager::Get().FindConsoleVariable(TEXT("r.ScreenPercentage"))->Set(85.0f);
            IConsoleManager::Get().FindConsoleVariable(TEXT("r.PostProcessAAQuality"))->Set(3);
            break;
            
        case EPerf_QualityLevel::High:
            IConsoleManager::Get().FindConsoleVariable(TEXT("r.ScreenPercentage"))->Set(100.0f);
            IConsoleManager::Get().FindConsoleVariable(TEXT("r.PostProcessAAQuality"))->Set(4);
            break;
            
        case EPerf_QualityLevel::Ultra:
            IConsoleManager::Get().FindConsoleVariable(TEXT("r.ScreenPercentage"))->Set(100.0f);
            IConsoleManager::Get().FindConsoleVariable(TEXT("r.PostProcessAAQuality"))->Set(6);
            break;
            
        default:
            break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: Rendering settings optimized"));
}

void UPerformanceManager::OptimizeMemorySettings()
{
    // Apply memory optimizations
    switch (CurrentQualityLevel)
    {
        case EPerf_QualityLevel::Low:
            IConsoleManager::Get().FindConsoleVariable(TEXT("r.Streaming.PoolSize"))->Set(1000);
            break;
            
        case EPerf_QualityLevel::Medium:
            IConsoleManager::Get().FindConsoleVariable(TEXT("r.Streaming.PoolSize"))->Set(2000);
            break;
            
        case EPerf_QualityLevel::High:
            IConsoleManager::Get().FindConsoleVariable(TEXT("r.Streaming.PoolSize"))->Set(3000);
            break;
            
        case EPerf_QualityLevel::Ultra:
            IConsoleManager::Get().FindConsoleVariable(TEXT("r.Streaming.PoolSize"))->Set(4000);
            break;
            
        default:
            break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: Memory settings optimized"));
}

void UPerformanceManager::StartPerformanceProfiling()
{
    bProfilingActive = true;
    ProfilingData.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: Performance profiling started"));
}

void UPerformanceManager::StopPerformanceProfiling()
{
    bProfilingActive = false;
    
    UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: Performance profiling stopped. Collected %d samples"), ProfilingData.Num());
}

void UPerformanceManager::GeneratePerformanceReport()
{
    if (ProfilingData.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: No profiling data available"));
        return;
    }
    
    // Calculate statistics
    float AvgFPS = 0.0f;
    float MinFPS = FLT_MAX;
    float MaxFPS = 0.0f;
    float AvgFrameTime = 0.0f;
    
    for (const FPerf_PerformanceMetrics& Metrics : ProfilingData)
    {
        AvgFPS += Metrics.CurrentFPS;
        MinFPS = FMath::Min(MinFPS, Metrics.CurrentFPS);
        MaxFPS = FMath::Max(MaxFPS, Metrics.CurrentFPS);
        AvgFrameTime += Metrics.FrameTime;
    }
    
    AvgFPS /= ProfilingData.Num();
    AvgFrameTime /= ProfilingData.Num();
    
    // Log performance report
    UE_LOG(LogTemp, Warning, TEXT("=== PERFORMANCE REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Samples: %d"), ProfilingData.Num());
    UE_LOG(LogTemp, Warning, TEXT("Average FPS: %.2f"), AvgFPS);
    UE_LOG(LogTemp, Warning, TEXT("Min FPS: %.2f"), MinFPS);
    UE_LOG(LogTemp, Warning, TEXT("Max FPS: %.2f"), MaxFPS);
    UE_LOG(LogTemp, Warning, TEXT("Average Frame Time: %.2fms"), AvgFrameTime);
    UE_LOG(LogTemp, Warning, TEXT("Target FPS: %.2f"), TargetFPS);
    UE_LOG(LogTemp, Warning, TEXT("Performance Target Met: %s"), (AvgFPS >= MinAcceptableFPS) ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("========================"));
}

void UPerformanceManager::UpdatePerformanceMetrics()
{
    // Get current FPS
    CurrentMetrics.CurrentFPS = 1.0f / GetWorld()->GetDeltaSeconds();
    CurrentMetrics.FrameTime = GetWorld()->GetDeltaSeconds() * 1000.0f; // Convert to milliseconds
    
    // Get rendering stats (simplified - would need more complex implementation for real stats)
    CurrentMetrics.GameThreadTime = CurrentMetrics.FrameTime * 0.6f; // Estimate
    CurrentMetrics.RenderThreadTime = CurrentMetrics.FrameTime * 0.3f; // Estimate
    CurrentMetrics.GPUTime = CurrentMetrics.FrameTime * 0.4f; // Estimate
    
    // Estimate draw calls and triangles (would need RHI stats for real values)
    CurrentMetrics.DrawCalls = FMath::RandRange(500, 1500);
    CurrentMetrics.VisibleTriangles = FMath::RandRange(500000, 1500000);
    
    // Get memory usage (simplified)
    CurrentMetrics.MemoryUsageMB = FPlatformMemory::GetStats().UsedPhysical / (1024.0f * 1024.0f);
    
    // Determine if performance is good
    CurrentMetrics.bIsPerformanceGood = (CurrentMetrics.CurrentFPS >= MinAcceptableFPS) &&
                                       (CurrentMetrics.DrawCalls <= MaxDrawCalls) &&
                                       (CurrentMetrics.VisibleTriangles <= MaxVisibleTriangles) &&
                                       (CurrentMetrics.MemoryUsageMB <= MaxMemoryUsageMB);
}

void UPerformanceManager::CheckPerformanceBudgets()
{
    if (CurrentMetrics.CurrentFPS < MinAcceptableFPS)
    {
        LogPerformanceWarning(FString::Printf(TEXT("FPS below target: %.2f < %.2f"), CurrentMetrics.CurrentFPS, MinAcceptableFPS));
    }
    
    if (CurrentMetrics.DrawCalls > MaxDrawCalls)
    {
        LogPerformanceWarning(FString::Printf(TEXT("Draw calls exceed budget: %d > %d"), CurrentMetrics.DrawCalls, MaxDrawCalls));
    }
    
    if (CurrentMetrics.VisibleTriangles > MaxVisibleTriangles)
    {
        LogPerformanceWarning(FString::Printf(TEXT("Triangles exceed budget: %d > %d"), CurrentMetrics.VisibleTriangles, MaxVisibleTriangles));
    }
    
    if (CurrentMetrics.MemoryUsageMB > MaxMemoryUsageMB)
    {
        LogPerformanceWarning(FString::Printf(TEXT("Memory usage exceeds budget: %.2fMB > %.2fMB"), CurrentMetrics.MemoryUsageMB, MaxMemoryUsageMB));
    }
}

void UPerformanceManager::AutoAdjustQuality()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    if (CurrentTime - LastQualityAdjustmentTime < QualityAdjustmentInterval)
    {
        return;
    }
    
    LastQualityAdjustmentTime = CurrentTime;
    
    // Adjust quality based on performance
    if (CurrentMetrics.CurrentFPS < MinAcceptableFPS * 0.8f)
    {
        // Performance is poor, reduce quality
        switch (CurrentQualityLevel)
        {
            case EPerf_QualityLevel::Ultra:
                SetQualityLevel(EPerf_QualityLevel::High);
                break;
            case EPerf_QualityLevel::High:
                SetQualityLevel(EPerf_QualityLevel::Medium);
                break;
            case EPerf_QualityLevel::Medium:
                SetQualityLevel(EPerf_QualityLevel::Low);
                break;
            default:
                break;
        }
    }
    else if (CurrentMetrics.CurrentFPS > TargetFPS * 1.2f)
    {
        // Performance is excellent, can increase quality
        switch (CurrentQualityLevel)
        {
            case EPerf_QualityLevel::Low:
                SetQualityLevel(EPerf_QualityLevel::Medium);
                break;
            case EPerf_QualityLevel::Medium:
                SetQualityLevel(EPerf_QualityLevel::High);
                break;
            case EPerf_QualityLevel::High:
                SetQualityLevel(EPerf_QualityLevel::Ultra);
                break;
            default:
                break;
        }
    }
}

void UPerformanceManager::ApplyQualitySettings()
{
    OptimizeLODSettings();
    OptimizeLightingSettings();
    OptimizeRenderingSettings();
    OptimizeMemorySettings();
}

void UPerformanceManager::LogPerformanceWarning(const FString& Warning)
{
    UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: %s"), *Warning);
}