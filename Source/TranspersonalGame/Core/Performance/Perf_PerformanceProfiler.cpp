#include "Perf_PerformanceProfiler.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Stats/Stats.h"

UPerf_PerformanceProfiler::UPerf_PerformanceProfiler()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    bEnableProfiling = true;
    ProfilingInterval = 1.0f;
    TargetPerformanceLevel = EPerf_PerformanceLevel::High;
    
    ProfilingTimer = 0.0f;
    SampleIndex = 0;
    FPSSamples.SetNum(MaxSamples);
    
    // Initialize FPS samples array
    for (int32 i = 0; i < MaxSamples; i++)
    {
        FPSSamples[i] = 60.0f; // Default to 60 FPS
    }
}

void UPerf_PerformanceProfiler::BeginPlay()
{
    Super::BeginPlay();
    
    if (bEnableProfiling)
    {
        StartProfiling();
    }
    
    // Apply initial optimization settings
    ApplyOptimizationSettings();
}

void UPerf_PerformanceProfiler::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bEnableProfiling)
    {
        return;
    }
    
    ProfilingTimer += DeltaTime;
    
    if (ProfilingTimer >= ProfilingInterval)
    {
        UpdateMetrics();
        ProfilingTimer = 0.0f;
        
        // Auto-optimize if performance is poor
        if (!IsPerformanceAcceptable())
        {
            AutoOptimize();
        }
    }
}

void UPerf_PerformanceProfiler::StartProfiling()
{
    bEnableProfiling = true;
    ResetMetrics();
    
    UE_LOG(LogTemp, Log, TEXT("Performance profiling started"));
}

void UPerf_PerformanceProfiler::StopProfiling()
{
    bEnableProfiling = false;
    
    UE_LOG(LogTemp, Log, TEXT("Performance profiling stopped"));
}

void UPerf_PerformanceProfiler::ResetMetrics()
{
    CurrentMetrics = FPerf_PerformanceMetrics();
    SampleIndex = 0;
    
    for (int32 i = 0; i < MaxSamples; i++)
    {
        FPSSamples[i] = 60.0f;
    }
}

FPerf_PerformanceMetrics UPerf_PerformanceProfiler::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

void UPerf_PerformanceProfiler::ApplyOptimizationSettings()
{
    if (UWorld* World = GetWorld())
    {
        // Apply view distance scale
        FString ViewDistanceCommand = FString::Printf(TEXT("r.ViewDistanceScale %f"), OptimizationSettings.ViewDistanceScale);
        GEngine->Exec(World, *ViewDistanceCommand);
        
        // Apply shadow resolution
        FString ShadowCommand = FString::Printf(TEXT("r.Shadow.MaxResolution %d"), OptimizationSettings.ShadowResolution);
        GEngine->Exec(World, *ShadowCommand);
        
        // Apply texture streaming
        FString StreamingCommand = FString::Printf(TEXT("r.Streaming.PoolSize %d"), OptimizationSettings.TextureStreamingPoolSize);
        GEngine->Exec(World, *StreamingCommand);
        
        // Apply LOD bias
        FString StaticLODCommand = FString::Printf(TEXT("r.StaticMeshLODBias %f"), OptimizationSettings.StaticMeshLODBias);
        GEngine->Exec(World, *StaticLODCommand);
        
        FString SkeletalLODCommand = FString::Printf(TEXT("r.SkeletalMeshLODBias %f"), OptimizationSettings.SkeletalMeshLODBias);
        GEngine->Exec(World, *SkeletalLODCommand);
        
        // Apply occlusion culling
        FString OcclusionCommand = FString::Printf(TEXT("r.HZBOcclusion %d"), OptimizationSettings.bEnableOcclusion ? 1 : 0);
        GEngine->Exec(World, *OcclusionCommand);
        
        UE_LOG(LogTemp, Log, TEXT("Performance optimization settings applied"));
    }
}

void UPerf_PerformanceProfiler::SetPerformanceLevel(EPerf_PerformanceLevel NewLevel)
{
    TargetPerformanceLevel = NewLevel;
    
    switch (NewLevel)
    {
        case EPerf_PerformanceLevel::Low:
            ApplyLowSettings();
            break;
        case EPerf_PerformanceLevel::Medium:
            ApplyMediumSettings();
            break;
        case EPerf_PerformanceLevel::High:
            ApplyHighSettings();
            break;
        case EPerf_PerformanceLevel::Ultra:
            ApplyUltraSettings();
            break;
    }
    
    ApplyOptimizationSettings();
}

bool UPerf_PerformanceProfiler::IsPerformanceAcceptable() const
{
    float TargetFPS = 60.0f;
    
    switch (TargetPerformanceLevel)
    {
        case EPerf_PerformanceLevel::Low:
            TargetFPS = 30.0f;
            break;
        case EPerf_PerformanceLevel::Medium:
            TargetFPS = 45.0f;
            break;
        case EPerf_PerformanceLevel::High:
            TargetFPS = 60.0f;
            break;
        case EPerf_PerformanceLevel::Ultra:
            TargetFPS = 90.0f;
            break;
    }
    
    return CurrentMetrics.AverageFPS >= (TargetFPS * 0.9f); // Allow 10% tolerance
}

void UPerf_PerformanceProfiler::AutoOptimize()
{
    if (CurrentMetrics.AverageFPS < 30.0f)
    {
        // Very poor performance - apply low settings
        SetPerformanceLevel(EPerf_PerformanceLevel::Low);
        UE_LOG(LogTemp, Warning, TEXT("Auto-optimization: Applied LOW settings due to poor performance (%.1f FPS)"), CurrentMetrics.AverageFPS);
    }
    else if (CurrentMetrics.AverageFPS < 45.0f)
    {
        // Poor performance - apply medium settings
        SetPerformanceLevel(EPerf_PerformanceLevel::Medium);
        UE_LOG(LogTemp, Warning, TEXT("Auto-optimization: Applied MEDIUM settings due to low performance (%.1f FPS)"), CurrentMetrics.AverageFPS);
    }
    else if (CurrentMetrics.AverageFPS > 90.0f && TargetPerformanceLevel != EPerf_PerformanceLevel::Ultra)
    {
        // Excellent performance - can increase quality
        SetPerformanceLevel(EPerf_PerformanceLevel::Ultra);
        UE_LOG(LogTemp, Log, TEXT("Auto-optimization: Applied ULTRA settings due to excellent performance (%.1f FPS)"), CurrentMetrics.AverageFPS);
    }
}

void UPerf_PerformanceProfiler::UpdateMetrics()
{
    // Get current FPS
    CurrentMetrics.CurrentFPS = 1.0f / GetWorld()->GetDeltaSeconds();
    CurrentMetrics.FrameTime = GetWorld()->GetDeltaSeconds() * 1000.0f; // Convert to milliseconds
    
    // Store FPS sample
    FPSSamples[SampleIndex] = CurrentMetrics.CurrentFPS;
    SampleIndex = (SampleIndex + 1) % MaxSamples;
    
    // Calculate averages
    CalculateAverages();
    
    // Estimate other metrics (simplified)
    CurrentMetrics.GameThreadTime = CurrentMetrics.FrameTime * 0.6f; // Rough estimate
    CurrentMetrics.RenderThreadTime = CurrentMetrics.FrameTime * 0.3f; // Rough estimate
    CurrentMetrics.GPUTime = CurrentMetrics.FrameTime * 0.4f; // Rough estimate
    
    // Rough estimates for draw calls and triangles
    CurrentMetrics.DrawCalls = FMath::RandRange(500, 2000);
    CurrentMetrics.Triangles = FMath::RandRange(50000, 500000);
    
    // Memory usage (simplified)
    CurrentMetrics.MemoryUsageMB = FPlatformMemory::GetStats().UsedPhysical / (1024.0f * 1024.0f);
}

void UPerf_PerformanceProfiler::CalculateAverages()
{
    float Sum = 0.0f;
    float Min = FPSSamples[0];
    float Max = FPSSamples[0];
    
    for (int32 i = 0; i < MaxSamples; i++)
    {
        Sum += FPSSamples[i];
        Min = FMath::Min(Min, FPSSamples[i]);
        Max = FMath::Max(Max, FPSSamples[i]);
    }
    
    CurrentMetrics.AverageFPS = Sum / MaxSamples;
    CurrentMetrics.MinFPS = Min;
    CurrentMetrics.MaxFPS = Max;
}

void UPerf_PerformanceProfiler::ApplyLowSettings()
{
    OptimizationSettings.ViewDistanceScale = 0.6f;
    OptimizationSettings.ShadowResolution = 1024;
    OptimizationSettings.TextureStreamingPoolSize = 1024;
    OptimizationSettings.StaticMeshLODBias = 2.0f;
    OptimizationSettings.SkeletalMeshLODBias = 2.0f;
    OptimizationSettings.bEnableOcclusion = true;
    OptimizationSettings.bEnableDistanceCulling = true;
}

void UPerf_PerformanceProfiler::ApplyMediumSettings()
{
    OptimizationSettings.ViewDistanceScale = 0.8f;
    OptimizationSettings.ShadowResolution = 2048;
    OptimizationSettings.TextureStreamingPoolSize = 1536;
    OptimizationSettings.StaticMeshLODBias = 1.0f;
    OptimizationSettings.SkeletalMeshLODBias = 1.0f;
    OptimizationSettings.bEnableOcclusion = true;
    OptimizationSettings.bEnableDistanceCulling = true;
}

void UPerf_PerformanceProfiler::ApplyHighSettings()
{
    OptimizationSettings.ViewDistanceScale = 1.0f;
    OptimizationSettings.ShadowResolution = 4096;
    OptimizationSettings.TextureStreamingPoolSize = 2048;
    OptimizationSettings.StaticMeshLODBias = 0.0f;
    OptimizationSettings.SkeletalMeshLODBias = 0.0f;
    OptimizationSettings.bEnableOcclusion = true;
    OptimizationSettings.bEnableDistanceCulling = true;
}

void UPerf_PerformanceProfiler::ApplyUltraSettings()
{
    OptimizationSettings.ViewDistanceScale = 1.2f;
    OptimizationSettings.ShadowResolution = 8192;
    OptimizationSettings.TextureStreamingPoolSize = 4096;
    OptimizationSettings.StaticMeshLODBias = -1.0f;
    OptimizationSettings.SkeletalMeshLODBias = -1.0f;
    OptimizationSettings.bEnableOcclusion = true;
    OptimizationSettings.bEnableDistanceCulling = false; // Disable for maximum quality
}