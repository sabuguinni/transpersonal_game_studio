// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "PerformanceOptimizer.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Stats/StatsHierarchical.h"
#include "RenderingThread.h"
#include "Engine/GameViewportClient.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Engine/RendererSettings.h"
#include "HAL/PlatformMemory.h"
#include "Misc/ConfigCacheIni.h"

DEFINE_LOG_CATEGORY(LogPerformanceOptimizer);

DECLARE_CYCLE_STAT(TEXT("Performance Optimization"), STAT_PerformanceOptimization, STATGROUP_Game);
DECLARE_CYCLE_STAT(TEXT("Metrics Collection"), STAT_MetricsCollection, STATGROUP_Game);
DECLARE_CYCLE_STAT(TEXT("Quality Adjustment"), STAT_QualityAdjustment, STATGROUP_Game);

UPerformanceOptimizer::UPerformanceOptimizer()
    : bOptimizationActive(false)
    , LastOptimizationTime(0.0f)
    , CPUBudgetMS(16.67f) // 60 FPS target
    , GPUBudgetMS(16.67f)
    , MemoryBudgetMB(4096.0f)
    , bDynamicQualityEnabled(true)
    , CurrentQualityScale(1.0f)
    , QualityScaleTarget(1.0f)
{
    // Initialize default optimization settings
    OptimizationSettings.OptimizationLevel = EPerformanceOptimizationLevel::Balanced;
    OptimizationSettings.TargetPlatform = EPerformanceTargetPlatform::HighEndPC;
    OptimizationSettings.TargetFrameRate = 60.0f;
    
    bEnableAutomaticOptimization = true;
    OptimizationInterval = 1.0f; // Check every second
    bEnableDetailedLogging = false;
}

void UPerformanceOptimizer::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("Performance Optimizer initialized"));
    
    // Set initial performance budgets based on platform
    SetPerformanceBudgets(CPUBudgetMS, GPUBudgetMS, MemoryBudgetMB);
}

void UPerformanceOptimizer::Deinitialize()
{
    StopOptimization();
    Super::Deinitialize();
    
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("Performance Optimizer deinitialized"));
}

bool UPerformanceOptimizer::ShouldCreateSubsystem(UObject* Outer) const
{
    return true;
}

void UPerformanceOptimizer::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    
    if (bEnableAutomaticOptimization)
    {
        StartOptimization();
    }
}

void UPerformanceOptimizer::StartOptimization()
{
    SCOPE_CYCLE_COUNTER(STAT_PerformanceOptimization);
    
    if (bOptimizationActive)
    {
        return;
    }
    
    bOptimizationActive = true;
    
    UWorld* World = GetWorld();
    if (World)
    {
        // Start optimization timer
        World->GetTimerManager().SetTimer(
            OptimizationTimerHandle,
            FTimerDelegate::CreateUObject(this, &UPerformanceOptimizer::UpdatePerformanceMetrics),
            OptimizationInterval,
            true
        );
        
        UE_LOG(LogPerformanceOptimizer, Log, TEXT("Performance optimization started - Target: %.1f FPS"), 
               OptimizationSettings.TargetFrameRate);
    }
}

void UPerformanceOptimizer::StopOptimization()
{
    if (!bOptimizationActive)
    {
        return;
    }
    
    bOptimizationActive = false;
    
    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().ClearTimer(OptimizationTimerHandle);
        World->GetTimerManager().ClearTimer(ProfilingTimerHandle);
    }
    
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("Performance optimization stopped"));
}

FPerformanceMetrics UPerformanceOptimizer::GetCurrentMetrics()
{
    SCOPE_CYCLE_COUNTER(STAT_MetricsCollection);
    
    FPerformanceMetrics Metrics;
    
    // Collect frame timing
    Metrics.FrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds
    Metrics.GameThreadTime = GetGameThreadTime();
    Metrics.RenderThreadTime = GetRenderThreadTime();
    Metrics.GPUTime = GetGPUTime();
    Metrics.PhysicsTime = GetPhysicsTime();
    
    // Collect rendering stats
    Metrics.DrawCalls = GetDrawCallCount();
    Metrics.Triangles = GetTriangleCount();
    
    // Collect memory usage
    Metrics.MemoryUsageMB = GetMemoryUsage();
    
    // Collect physics stats
    Metrics.ActivePhysicsObjects = GetActivePhysicsObjectCount();
    
    // Collect quality settings
    Metrics.LODBias = GetConsoleVariableFloat(TEXT("r.StaticMeshLODBias"));
    Metrics.ShadowQuality = GetConsoleVariableFloat(TEXT("sg.ShadowQuality"));
    Metrics.TextureQuality = GetConsoleVariableFloat(TEXT("sg.TextureQuality"));
    Metrics.EffectsQuality = GetConsoleVariableFloat(TEXT("sg.EffectsQuality"));
    
    return Metrics;
}

void UPerformanceOptimizer::ApplyOptimizationPreset(EPerformanceOptimizationLevel Level, EPerformanceTargetPlatform Platform)
{
    OptimizationSettings.OptimizationLevel = Level;
    OptimizationSettings.TargetPlatform = Platform;
    
    // Apply platform-specific settings
    switch (Platform)
    {
        case EPerformanceTargetPlatform::HighEndPC:
            OptimizationSettings.TargetFrameRate = 60.0f;
            OptimizationSettings.MaxMemoryUsageMB = 8192.0f;
            ApplyPCOptimizations();
            break;
            
        case EPerformanceTargetPlatform::MidRangePC:
            OptimizationSettings.TargetFrameRate = 60.0f;
            OptimizationSettings.MaxMemoryUsageMB = 4096.0f;
            ApplyPCOptimizations();
            break;
            
        case EPerformanceTargetPlatform::Console:
            OptimizationSettings.TargetFrameRate = 30.0f;
            OptimizationSettings.MaxMemoryUsageMB = 6144.0f;
            ApplyConsoleOptimizations();
            break;
            
        case EPerformanceTargetPlatform::Mobile:
            OptimizationSettings.TargetFrameRate = 30.0f;
            OptimizationSettings.MaxMemoryUsageMB = 2048.0f;
            ApplyMobileOptimizations();
            break;
            
        case EPerformanceTargetPlatform::VR:
            OptimizationSettings.TargetFrameRate = 90.0f;
            OptimizationSettings.MaxMemoryUsageMB = 4096.0f;
            ApplyVROptimizations();
            break;
    }
    
    // Apply optimization level settings
    switch (Level)
    {
        case EPerformanceOptimizationLevel::Conservative:
            SetConsoleVariable(TEXT("r.StaticMeshLODBias"), 0.0f);
            SetConsoleVariable(TEXT("sg.ShadowQuality"), 3);
            SetConsoleVariable(TEXT("sg.TextureQuality"), 3);
            SetConsoleVariable(TEXT("sg.EffectsQuality"), 3);
            break;
            
        case EPerformanceOptimizationLevel::Balanced:
            SetConsoleVariable(TEXT("r.StaticMeshLODBias"), 0.5f);
            SetConsoleVariable(TEXT("sg.ShadowQuality"), 2);
            SetConsoleVariable(TEXT("sg.TextureQuality"), 2);
            SetConsoleVariable(TEXT("sg.EffectsQuality"), 2);
            break;
            
        case EPerformanceOptimizationLevel::Aggressive:
            SetConsoleVariable(TEXT("r.StaticMeshLODBias"), 1.0f);
            SetConsoleVariable(TEXT("sg.ShadowQuality"), 1);
            SetConsoleVariable(TEXT("sg.TextureQuality"), 1);
            SetConsoleVariable(TEXT("sg.EffectsQuality"), 1);
            break;
            
        case EPerformanceOptimizationLevel::Maximum:
            SetConsoleVariable(TEXT("r.StaticMeshLODBias"), 2.0f);
            SetConsoleVariable(TEXT("sg.ShadowQuality"), 0);
            SetConsoleVariable(TEXT("sg.TextureQuality"), 0);
            SetConsoleVariable(TEXT("sg.EffectsQuality"), 0);
            break;
    }
    
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("Applied optimization preset: Level=%d, Platform=%d"), 
           (int32)Level, (int32)Platform);
}

void UPerformanceOptimizer::OptimizePhysicsPerformance()
{
    if (!OptimizationSettings.bOptimizePhysics)
    {
        return;
    }
    
    // Optimize physics substep settings
    SetConsoleVariable(TEXT("p.DefaultMaxSubstepDeltaTime"), OptimizationSettings.PhysicsSubstepDeltaTime);
    SetConsoleVariable(TEXT("p.MaxSubsteps"), 6);
    
    // Optimize Chaos physics settings
    SetConsoleVariable(TEXT("p.Chaos.Solver.IterationCount"), 4);
    SetConsoleVariable(TEXT("p.Chaos.Solver.PushOutIterationCount"), 2);
    SetConsoleVariable(TEXT("p.Chaos.Solver.CollisionCullDistance"), 5.0f);
    
    // Optimize physics object limits
    int32 CurrentPhysicsObjects = GetActivePhysicsObjectCount();
    if (CurrentPhysicsObjects > OptimizationSettings.MaxPhysicsObjects)
    {
        UE_LOG(LogPerformanceOptimizer, Warning, 
               TEXT("Physics object count (%d) exceeds limit (%d)"), 
               CurrentPhysicsObjects, OptimizationSettings.MaxPhysicsObjects);
    }
    
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("Physics performance optimized"));
}

void UPerformanceOptimizer::OptimizeRenderingPerformance()
{
    if (!OptimizationSettings.bOptimizeRendering)
    {
        return;
    }
    
    // Apply dynamic LOD bias
    SetConsoleVariable(TEXT("r.StaticMeshLODBias"), OptimizationSettings.DynamicLODBias);
    SetConsoleVariable(TEXT("r.SkeletalMeshLODBias"), OptimizationSettings.DynamicLODBias);
    
    // Optimize shadow rendering
    SetConsoleVariable(TEXT("r.Shadow.MaxResolution"), 2048);
    SetConsoleVariable(TEXT("r.Shadow.RadiusThreshold"), 0.05f);
    
    // Optimize texture streaming
    SetConsoleVariable(TEXT("r.Streaming.MipBias"), 0.5f);
    SetConsoleVariable(TEXT("r.Streaming.PoolSize"), 2048);
    
    // Enable dynamic resolution if configured
    if (OptimizationSettings.bUseDynamicResolution)
    {
        SetConsoleVariable(TEXT("r.DynamicRes.OperationMode"), 2);
        SetConsoleVariable(TEXT("r.DynamicRes.MinResolutionFraction"), OptimizationSettings.MinResolutionScale);
    }
    
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("Rendering performance optimized"));
}

void UPerformanceOptimizer::OptimizeCullingPerformance()
{
    if (!OptimizationSettings.bOptimizeCulling)
    {
        return;
    }
    
    // Optimize view distance culling
    SetConsoleVariable(TEXT("r.ViewDistanceScale"), OptimizationSettings.CullDistanceMultiplier);
    
    // Enable occlusion culling if configured
    if (OptimizationSettings.bUseOcclusionCulling)
    {
        SetConsoleVariable(TEXT("r.AllowOcclusionQueries"), 1);
        SetConsoleVariable(TEXT("r.HZBOcclusion"), 1);
    }
    
    // Optimize frustum culling
    SetConsoleVariable(TEXT("r.EarlyZPass"), 3);
    SetConsoleVariable(TEXT("r.EarlyZPassOnlyMaterialMasking"), 1);
    
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("Culling performance optimized"));
}

void UPerformanceOptimizer::OptimizeMemoryUsage()
{
    // Force garbage collection
    GEngine->ForceGarbageCollection(true);
    
    // Optimize texture memory
    SetConsoleVariable(TEXT("r.Streaming.PoolSize"), 1024);
    SetConsoleVariable(TEXT("r.Streaming.MaxTempMemoryAllowed"), 50);
    
    // Optimize audio memory
    SetConsoleVariable(TEXT("au.StreamCaching.MaxMemoryInMB"), 128);
    
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("Memory usage optimized"));
}

void UPerformanceOptimizer::ResetToDefaultSettings()
{
    // Reset all console variables to defaults
    SetConsoleVariable(TEXT("r.StaticMeshLODBias"), 0.0f);
    SetConsoleVariable(TEXT("r.SkeletalMeshLODBias"), 0.0f);
    SetConsoleVariable(TEXT("sg.ShadowQuality"), 3);
    SetConsoleVariable(TEXT("sg.TextureQuality"), 3);
    SetConsoleVariable(TEXT("sg.EffectsQuality"), 3);
    SetConsoleVariable(TEXT("r.ViewDistanceScale"), 1.0f);
    
    // Reset optimization settings
    OptimizationSettings = FPerformanceOptimizationSettings();
    
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("Settings reset to defaults"));
}

void UPerformanceOptimizer::StartPerformanceProfiling()
{
    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().SetTimer(
            ProfilingTimerHandle,
            FTimerDelegate::CreateLambda([this]()
            {
                FPerformanceMetrics Metrics = GetCurrentMetrics();
                MetricsHistory.Add(Metrics);
                
                if (bEnableDetailedLogging)
                {
                    LogPerformanceMetrics(Metrics);
                }
            }),
            0.1f, // Sample every 100ms for detailed profiling
            true
        );
        
        UE_LOG(LogPerformanceOptimizer, Log, TEXT("Performance profiling started"));
    }
}

void UPerformanceOptimizer::StopPerformanceProfiling()
{
    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().ClearTimer(ProfilingTimerHandle);
    }
    
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("Performance profiling stopped"));
}

void UPerformanceOptimizer::GeneratePerformanceReport()
{
    FString ReportPath = FPaths::ProjectSavedDir() / TEXT("PerformanceReports");
    FString FileName = FString::Printf(TEXT("PerformanceReport_%s.txt"), 
                                       *FDateTime::Now().ToString());
    FString FullPath = ReportPath / FileName;
    
    SavePerformanceReport(FullPath);
    
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("Performance report generated: %s"), *FullPath);
}

void UPerformanceOptimizer::OptimizeForTargetFramerate(float TargetFPS)
{
    OptimizationSettings.TargetFrameRate = TargetFPS;
    OptimizationSettings.MaxFrameTime = 1000.0f / TargetFPS; // Convert to milliseconds
    
    CPUBudgetMS = OptimizationSettings.MaxFrameTime;
    GPUBudgetMS = OptimizationSettings.MaxFrameTime;
    
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("Target framerate set to %.1f FPS"), TargetFPS);
}

void UPerformanceOptimizer::EnableDynamicQualityScaling(bool bEnable)
{
    bDynamicQualityEnabled = bEnable;
    
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("Dynamic quality scaling %s"), 
           bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UPerformanceOptimizer::SetPerformanceBudgets(float CPUBudgetMS_In, float GPUBudgetMS_In, float MemoryBudgetMB_In)
{
    CPUBudgetMS = CPUBudgetMS_In;
    GPUBudgetMS = GPUBudgetMS_In;
    MemoryBudgetMB = MemoryBudgetMB_In;
    
    UE_LOG(LogPerformanceOptimizer, Log, 
           TEXT("Performance budgets set - CPU: %.2fms, GPU: %.2fms, Memory: %.1fMB"),
           CPUBudgetMS, GPUBudgetMS, MemoryBudgetMB);
}

void UPerformanceOptimizer::UpdatePerformanceMetrics()
{
    CurrentMetrics = GetCurrentMetrics();
    AnalyzePerformance();
    
    if (bDynamicQualityEnabled)
    {
        AdjustQualitySettings();
    }
    
    OnPerformanceOptimized.Broadcast(CurrentMetrics);
}

void UPerformanceOptimizer::AnalyzePerformance()
{
    // Check if we're exceeding performance budgets
    if (CurrentMetrics.FrameTime > OptimizationSettings.MaxFrameTime)
    {
        OnPerformanceThresholdExceeded.Broadcast(TEXT("FrameTime"), CurrentMetrics.FrameTime);
        
        if (bEnableAutomaticOptimization)
        {
            ApplyOptimizations();
        }
    }
    
    if (CurrentMetrics.MemoryUsageMB > OptimizationSettings.MaxMemoryUsageMB)
    {
        OnPerformanceThresholdExceeded.Broadcast(TEXT("MemoryUsage"), CurrentMetrics.MemoryUsageMB);
        OptimizeMemoryUsage();
    }
}

void UPerformanceOptimizer::ApplyOptimizations()
{
    SCOPE_CYCLE_COUNTER(STAT_PerformanceOptimization);
    
    // Apply physics optimizations
    OptimizePhysicsSettings();
    
    // Apply rendering optimizations
    OptimizeRenderingSettings();
    
    // Apply culling optimizations
    OptimizeCullingSettings();
    
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("Automatic optimizations applied"));
}

void UPerformanceOptimizer::AdjustQualitySettings()
{
    SCOPE_CYCLE_COUNTER(STAT_QualityAdjustment);
    
    float TargetFrameTime = 1000.0f / OptimizationSettings.TargetFrameRate;
    float FrameTimeRatio = CurrentMetrics.FrameTime / TargetFrameTime;
    
    // Adjust quality scale based on performance
    if (FrameTimeRatio > 1.1f) // 10% over budget
    {
        QualityScaleTarget = FMath::Max(0.5f, CurrentQualityScale - 0.1f);
    }
    else if (FrameTimeRatio < 0.9f) // 10% under budget
    {
        QualityScaleTarget = FMath::Min(1.0f, CurrentQualityScale + 0.05f);
    }
    
    // Smoothly interpolate to target quality scale
    CurrentQualityScale = FMath::FInterpTo(CurrentQualityScale, QualityScaleTarget, 
                                           FApp::GetDeltaTime(), 2.0f);
    
    // Apply quality scale to various settings
    SetConsoleVariable(TEXT("r.ScreenPercentage"), CurrentQualityScale * 100.0f);
    SetConsoleVariable(TEXT("r.StaticMeshLODBias"), (1.0f - CurrentQualityScale) * 2.0f);
}

// Implementation of private helper methods continues...
// [Additional implementation methods for platform-specific optimizations, 
//  metrics collection, console variable management, etc.]

float UPerformanceOptimizer::GetGameThreadTime() const
{
    return FPlatformTime::ToMilliseconds(GGameThreadTime);
}

float UPerformanceOptimizer::GetRenderThreadTime() const
{
    return FPlatformTime::ToMilliseconds(GRenderThreadTime);
}

float UPerformanceOptimizer::GetGPUTime() const
{
    return FPlatformTime::ToMilliseconds(RHIGetGPUFrameCycles());
}

float UPerformanceOptimizer::GetPhysicsTime() const
{
    // Implementation depends on physics system integration
    return 0.0f; // Placeholder
}

int32 UPerformanceOptimizer::GetDrawCallCount() const
{
    // Implementation depends on rendering stats
    return 0; // Placeholder
}

int32 UPerformanceOptimizer::GetTriangleCount() const
{
    // Implementation depends on rendering stats
    return 0; // Placeholder
}

float UPerformanceOptimizer::GetMemoryUsage() const
{
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    return MemStats.UsedPhysical / (1024.0f * 1024.0f); // Convert to MB
}

int32 UPerformanceOptimizer::GetActivePhysicsObjectCount() const
{
    // Implementation depends on physics system integration
    return 0; // Placeholder
}

void UPerformanceOptimizer::SetConsoleVariable(const FString& VariableName, float Value)
{
    IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(*VariableName);
    if (CVar)
    {
        CVar->Set(Value);
    }
}

void UPerformanceOptimizer::SetConsoleVariable(const FString& VariableName, int32 Value)
{
    IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(*VariableName);
    if (CVar)
    {
        CVar->Set(Value);
    }
}

float UPerformanceOptimizer::GetConsoleVariableFloat(const FString& VariableName) const
{
    IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(*VariableName);
    return CVar ? CVar->GetFloat() : 0.0f;
}

int32 UPerformanceOptimizer::GetConsoleVariableInt(const FString& VariableName) const
{
    IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(*VariableName);
    return CVar ? CVar->GetInt() : 0;
}

void UPerformanceOptimizer::LogPerformanceMetrics(const FPerformanceMetrics& Metrics)
{
    UE_LOG(LogPerformanceOptimizer, Log, 
           TEXT("Performance Metrics - Frame: %.2fms, Game: %.2fms, Render: %.2fms, GPU: %.2fms, Memory: %.1fMB"),
           Metrics.FrameTime, Metrics.GameThreadTime, Metrics.RenderThreadTime, 
           Metrics.GPUTime, Metrics.MemoryUsageMB);
}

void UPerformanceOptimizer::LogOptimizationAction(const FString& Action, const FString& Details)
{
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("Optimization Action: %s - %s"), *Action, *Details);
}

void UPerformanceOptimizer::SavePerformanceReport(const FString& FilePath)
{
    // Implementation for saving detailed performance report to file
    // This would include historical metrics, optimization actions taken, etc.
}

void UPerformanceOptimizer::ApplyPCOptimizations()
{
    // PC-specific optimizations
    SetConsoleVariable(TEXT("r.Shadow.MaxResolution"), 4096);
    SetConsoleVariable(TEXT("r.TextureStreaming"), 1);
}

void UPerformanceOptimizer::ApplyConsoleOptimizations()
{
    // Console-specific optimizations
    SetConsoleVariable(TEXT("r.Shadow.MaxResolution"), 2048);
    SetConsoleVariable(TEXT("r.MobileHDR"), 0);
}

void UPerformanceOptimizer::ApplyMobileOptimizations()
{
    // Mobile-specific optimizations
    SetConsoleVariable(TEXT("r.Shadow.MaxResolution"), 1024);
    SetConsoleVariable(TEXT("r.MobileHDR"), 0);
    SetConsoleVariable(TEXT("r.Mobile.EnableStaticAndCSMShadowReceivers"), 0);
}

void UPerformanceOptimizer::ApplyVROptimizations()
{
    // VR-specific optimizations
    SetConsoleVariable(TEXT("r.VR.PixelDensity"), 1.0f);
    SetConsoleVariable(TEXT("r.VR.RoundRobinOcclusion"), 1);
}

void UPerformanceOptimizer::OptimizePhysicsSettings()
{
    OptimizePhysicsPerformance();
}

void UPerformanceOptimizer::OptimizeRenderingSettings()
{
    OptimizeRenderingPerformance();
}

void UPerformanceOptimizer::OptimizeCullingSettings()
{
    OptimizeCullingPerformance();
}

void UPerformanceOptimizer::MonitorMemoryUsage()
{
    float CurrentMemory = GetMemoryUsage();
    if (CurrentMemory > MemoryBudgetMB)
    {
        OptimizeMemoryUsage();
    }
}