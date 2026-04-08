// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "PerformanceOptimizer.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformApplicationMisc.h"
#include "HAL/PlatformMemory.h"
#include "Stats/StatsData.h"
#include "RenderingThread.h"
#include "RHI.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogPerformanceOptimizer, Log, All);

UPerformanceOptimizer::UPerformanceOptimizer()
{
    // Initialize platform-specific performance targets
    FPlatformPerformanceTarget PCTarget;
    PCTarget.TargetFPS = 60.0f;
    PCTarget.MaxFrameTimeMS = 16.67f;
    PCTarget.TargetQuality = EPerformanceQuality::High;
    PCTarget.MemoryBudgetMB = 8192.0f;
    PCTarget.MaxDrawCalls = 5000;
    PCTarget.MaxPhysicsObjects = 1000;
    PlatformTargets.Add(TEXT("Windows"), PCTarget);

    FPlatformPerformanceTarget ConsoleTarget;
    ConsoleTarget.TargetFPS = 30.0f;
    ConsoleTarget.MaxFrameTimeMS = 33.33f;
    ConsoleTarget.TargetQuality = EPerformanceQuality::Medium;
    ConsoleTarget.MemoryBudgetMB = 6144.0f;
    ConsoleTarget.MaxDrawCalls = 3000;
    ConsoleTarget.MaxPhysicsObjects = 500;
    PlatformTargets.Add(TEXT("Console"), ConsoleTarget);

    // Initialize optimization categories (all enabled by default)
    EnabledOptimizations.Add(EOptimizationCategory::Rendering, true);
    EnabledOptimizations.Add(EOptimizationCategory::Physics, true);
    EnabledOptimizations.Add(EOptimizationCategory::Collision, true);
    EnabledOptimizations.Add(EOptimizationCategory::LOD, true);
    EnabledOptimizations.Add(EOptimizationCategory::Shadows, true);
    EnabledOptimizations.Add(EOptimizationCategory::PostProcessing, true);
    EnabledOptimizations.Add(EOptimizationCategory::Particles, true);
    EnabledOptimizations.Add(EOptimizationCategory::Audio, true);
    EnabledOptimizations.Add(EOptimizationCategory::Streaming, true);
    EnabledOptimizations.Add(EOptimizationCategory::Memory, true);

    // Get current platform
    CurrentPlatform = FPlatformProperties::PlatformName();
    
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("PerformanceOptimizer initialized for platform: %s"), *CurrentPlatform);
}

void UPerformanceOptimizer::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    ApplyPlatformOptimizations();
    
    if (bEnablePerformanceMonitoring)
    {
        StartPerformanceMonitoring();
    }
    
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("PerformanceOptimizer subsystem initialized"));
}

void UPerformanceOptimizer::Deinitialize()
{
    StopPerformanceMonitoring();
    Super::Deinitialize();
}

void UPerformanceOptimizer::StartPerformanceMonitoring()
{
    if (!GetWorld())
    {
        UE_LOG(LogPerformanceOptimizer, Warning, TEXT("Cannot start performance monitoring without valid world"));
        return;
    }

    GetWorld()->GetTimerManager().SetTimer(
        PerformanceMonitoringTimer,
        this,
        &UPerformanceOptimizer::UpdatePerformanceMetrics,
        MonitoringUpdateInterval,
        true
    );

    bEnablePerformanceMonitoring = true;
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("Performance monitoring started"));
}

void UPerformanceOptimizer::StopPerformanceMonitoring()
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(PerformanceMonitoringTimer);
    }

    bEnablePerformanceMonitoring = false;
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("Performance monitoring stopped"));
}

UPerformanceOptimizer::FPerformanceMetrics UPerformanceOptimizer::GetCurrentPerformanceMetrics() const
{
    return CurrentMetrics;
}

void UPerformanceOptimizer::ApplyDynamicOptimizations()
{
    if (!bOptimizationsActive)
    {
        return;
    }

    double StartTime = FPlatformTime::Seconds();

    // Analyze current performance bottlenecks
    AnalyzePerformanceBottlenecks();

    // Apply optimizations based on bottleneck type
    switch (CurrentMetrics.Bottleneck)
    {
        case EPerformanceBottleneck::CPU_GameThread:
            OptimizePhysicsSimulation();
            OptimizeCollisionDetection();
            break;

        case EPerformanceBottleneck::CPU_RenderThread:
            OptimizeLODSystem();
            OptimizeRenderingQuality();
            break;

        case EPerformanceBottleneck::GPU:
            OptimizeShadowQuality();
            OptimizePostProcessing();
            OptimizeRenderingQuality();
            break;

        case EPerformanceBottleneck::Memory:
            OptimizeMemoryUsage();
            OptimizeAssetStreaming();
            break;

        case EPerformanceBottleneck::Physics:
            OptimizePhysicsSimulation();
            OptimizeRagdollPhysics();
            break;

        case EPerformanceBottleneck::Collision:
            OptimizeCollisionDetection();
            break;

        default:
            // No specific bottleneck, apply general optimizations
            ApplyFrameRateOptimizations();
            break;
    }

    OptimizationApplicationCount++;
    LastOptimizationTime = FPlatformTime::Seconds() - StartTime;
    TotalOptimizationTime += LastOptimizationTime;

    UE_LOG(LogPerformanceOptimizer, Verbose, TEXT("Applied dynamic optimizations in %f ms"), LastOptimizationTime * 1000.0f);
}

void UPerformanceOptimizer::SetTargetFrameRate(float TargetFPS)
{
    CurrentTargetFPS = FMath::Clamp(TargetFPS, 15.0f, 120.0f);
    
    // Update platform target if it exists
    if (PlatformTargets.Contains(CurrentPlatform))
    {
        PlatformTargets[CurrentPlatform].TargetFPS = CurrentTargetFPS;
        PlatformTargets[CurrentPlatform].MaxFrameTimeMS = 1000.0f / CurrentTargetFPS;
    }

    UE_LOG(LogPerformanceOptimizer, Log, TEXT("Target frame rate set to %f FPS"), CurrentTargetFPS);
}

void UPerformanceOptimizer::SetOptimizationCategory(EOptimizationCategory Category, bool bEnabled)
{
    EnabledOptimizations[Category] = bEnabled;
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("Optimization category %d set to %s"), 
           static_cast<int32>(Category), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UPerformanceOptimizer::UpdatePerformanceMetrics()
{
    if (!GEngine || !GetWorld())
    {
        return;
    }

    // Get frame time metrics
    CurrentMetrics.FrameTimeMS = FPlatformTime::ToMilliseconds(GEngine->GetMaxTickRate());
    CurrentMetrics.CurrentFPS = CurrentMetrics.FrameTimeMS > 0.0f ? 1000.0f / CurrentMetrics.FrameTimeMS : 0.0f;

    // Get thread times
    CurrentMetrics.GameThreadTimeMS = FPlatformTime::ToMilliseconds(FStatsThreadState::GetLocalState().GetFastThreadFrameTime(EThreadType::Game));
    CurrentMetrics.RenderThreadTimeMS = FPlatformTime::ToMilliseconds(FStatsThreadState::GetLocalState().GetFastThreadFrameTime(EThreadType::Renderer));

    // Get GPU time (if available)
    if (GRHISupportsGPUTimestamps)
    {
        CurrentMetrics.GPUTimeMS = FPlatformTime::ToMilliseconds(RHIGetGPUFrameCycles()) / FPlatformTime::GetSecondsPerCycle();
    }

    // Get memory usage
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);

    // Get rendering stats (approximated)
    CurrentMetrics.DrawCalls = GEngine->GetNumDrawCallsLastFrame();
    CurrentMetrics.TrianglesRendered = GEngine->GetNumPrimitivesDrawnLastFrame();

    // Store metrics in history
    MetricsHistory.Add(CurrentMetrics);
    if (MetricsHistory.Num() > MetricsHistorySize)
    {
        MetricsHistory.RemoveAt(0);
    }

    // Check if optimizations should be applied
    if (CurrentMetrics.FrameTimeMS > FrameTimeWarningThreshold || 
        CurrentMetrics.MemoryUsageMB > MemoryWarningThreshold)
    {
        bOptimizationsActive = true;
        ApplyDynamicOptimizations();
    }
    else if (CurrentMetrics.FrameTimeMS < FrameTimeWarningThreshold * 0.8f &&
             CurrentMetrics.MemoryUsageMB < MemoryWarningThreshold * 0.8f)
    {
        // Performance is good, can potentially increase quality
        bOptimizationsActive = false;
    }
}

void UPerformanceOptimizer::AnalyzePerformanceBottlenecks()
{
    // Determine primary bottleneck based on metrics
    if (CurrentMetrics.MemoryUsageMB > MemoryCriticalThreshold)
    {
        CurrentMetrics.Bottleneck = EPerformanceBottleneck::Memory;
    }
    else if (CurrentMetrics.GameThreadTimeMS > CurrentMetrics.RenderThreadTimeMS && 
             CurrentMetrics.GameThreadTimeMS > CurrentMetrics.GPUTimeMS)
    {
        if (CurrentMetrics.PhysicsObjects > PlatformTargets[CurrentPlatform].MaxPhysicsObjects * 0.8f)
        {
            CurrentMetrics.Bottleneck = EPerformanceBottleneck::Physics;
        }
        else if (CurrentMetrics.CollisionTests > 1000) // Arbitrary threshold
        {
            CurrentMetrics.Bottleneck = EPerformanceBottleneck::Collision;
        }
        else
        {
            CurrentMetrics.Bottleneck = EPerformanceBottleneck::CPU_GameThread;
        }
    }
    else if (CurrentMetrics.RenderThreadTimeMS > CurrentMetrics.GPUTimeMS)
    {
        CurrentMetrics.Bottleneck = EPerformanceBottleneck::CPU_RenderThread;
    }
    else if (CurrentMetrics.GPUTimeMS > 0.0f)
    {
        CurrentMetrics.Bottleneck = EPerformanceBottleneck::GPU;
    }
    else
    {
        CurrentMetrics.Bottleneck = EPerformanceBottleneck::None;
    }
}

void UPerformanceOptimizer::ApplyFrameRateOptimizations()
{
    if (!EnabledOptimizations[EOptimizationCategory::Rendering])
    {
        return;
    }

    // Apply general frame rate optimizations
    float TargetFrameTime = PlatformTargets[CurrentPlatform].MaxFrameTimeMS;
    
    if (CurrentMetrics.FrameTimeMS > TargetFrameTime * 1.2f)
    {
        // Reduce quality level
        if (CurrentQualityLevel > EPerformanceQuality::Low)
        {
            CurrentQualityLevel = static_cast<EPerformanceQuality>(static_cast<int32>(CurrentQualityLevel) - 1);
            OptimizeRenderingQuality();
        }
    }
    else if (CurrentMetrics.FrameTimeMS < TargetFrameTime * 0.8f)
    {
        // Increase quality level
        if (CurrentQualityLevel < EPerformanceQuality::Cinematic)
        {
            CurrentQualityLevel = static_cast<EPerformanceQuality>(static_cast<int32>(CurrentQualityLevel) + 1);
            OptimizeRenderingQuality();
        }
    }
}

void UPerformanceOptimizer::OptimizeRenderingQuality()
{
    if (!EnabledOptimizations[EOptimizationCategory::Rendering])
    {
        return;
    }

    // Adjust rendering quality based on current quality level
    switch (CurrentQualityLevel)
    {
        case EPerformanceQuality::Low:
            SetConsoleVariable(TEXT("sg.ViewDistanceScale"), 0.6f);
            SetConsoleVariable(TEXT("sg.AntiAliasingQuality"), 0);
            SetConsoleVariable(TEXT("sg.TextureQuality"), 0);
            break;

        case EPerformanceQuality::Medium:
            SetConsoleVariable(TEXT("sg.ViewDistanceScale"), 0.8f);
            SetConsoleVariable(TEXT("sg.AntiAliasingQuality"), 2);
            SetConsoleVariable(TEXT("sg.TextureQuality"), 2);
            break;

        case EPerformanceQuality::High:
            SetConsoleVariable(TEXT("sg.ViewDistanceScale"), 1.0f);
            SetConsoleVariable(TEXT("sg.AntiAliasingQuality"), 3);
            SetConsoleVariable(TEXT("sg.TextureQuality"), 3);
            break;

        case EPerformanceQuality::Epic:
            SetConsoleVariable(TEXT("sg.ViewDistanceScale"), 1.2f);
            SetConsoleVariable(TEXT("sg.AntiAliasingQuality"), 4);
            SetConsoleVariable(TEXT("sg.TextureQuality"), 4);
            break;

        case EPerformanceQuality::Cinematic:
            SetConsoleVariable(TEXT("sg.ViewDistanceScale"), 1.5f);
            SetConsoleVariable(TEXT("sg.AntiAliasingQuality"), 4);
            SetConsoleVariable(TEXT("sg.TextureQuality"), 4);
            break;
    }

    UE_LOG(LogPerformanceOptimizer, Log, TEXT("Rendering quality optimized to level %d"), static_cast<int32>(CurrentQualityLevel));
}

void UPerformanceOptimizer::OptimizeLODSystem()
{
    if (!EnabledOptimizations[EOptimizationCategory::LOD])
    {
        return;
    }

    // Adjust LOD bias based on performance
    float LODBias = 0.0f;
    
    if (CurrentMetrics.FrameTimeMS > FrameTimeWarningThreshold)
    {
        LODBias = 1.0f; // Force higher LODs (lower quality)
    }
    else if (CurrentMetrics.FrameTimeMS > FrameTimeCriticalThreshold)
    {
        LODBias = 2.0f; // Force even higher LODs
    }

    SetConsoleVariable(TEXT("r.StaticMeshLODBias"), LODBias);
    SetConsoleVariable(TEXT("r.SkeletalMeshLODBias"), LODBias);

    UE_LOG(LogPerformanceOptimizer, Verbose, TEXT("LOD system optimized with bias %f"), LODBias);
}

void UPerformanceOptimizer::OptimizeShadowQuality()
{
    if (!EnabledOptimizations[EOptimizationCategory::Shadows])
    {
        return;
    }

    // Adjust shadow quality based on performance
    if (CurrentMetrics.GPUTimeMS > 20.0f) // GPU bound
    {
        SetConsoleVariable(TEXT("r.ShadowQuality"), 2); // Medium shadows
        SetConsoleVariable(TEXT("r.Shadow.MaxResolution"), 1024);
    }
    else if (CurrentMetrics.GPUTimeMS > 30.0f)
    {
        SetConsoleVariable(TEXT("r.ShadowQuality"), 1); // Low shadows
        SetConsoleVariable(TEXT("r.Shadow.MaxResolution"), 512);
    }
    else
    {
        SetConsoleVariable(TEXT("r.ShadowQuality"), 3); // High shadows
        SetConsoleVariable(TEXT("r.Shadow.MaxResolution"), 2048);
    }

    UE_LOG(LogPerformanceOptimizer, Verbose, TEXT("Shadow quality optimized"));
}

void UPerformanceOptimizer::OptimizePostProcessing()
{
    if (!EnabledOptimizations[EOptimizationCategory::PostProcessing])
    {
        return;
    }

    // Adjust post-processing effects based on GPU performance
    if (CurrentMetrics.GPUTimeMS > 25.0f)
    {
        SetConsoleVariable(TEXT("sg.PostProcessQuality"), 1); // Low post-processing
        SetConsoleVariable(TEXT("r.BloomQuality"), 2);
        SetConsoleVariable(TEXT("r.MotionBlurQuality"), 1);
    }
    else
    {
        SetConsoleVariable(TEXT("sg.PostProcessQuality"), 3); // High post-processing
        SetConsoleVariable(TEXT("r.BloomQuality"), 4);
        SetConsoleVariable(TEXT("r.MotionBlurQuality"), 3);
    }

    UE_LOG(LogPerformanceOptimizer, Verbose, TEXT("Post-processing optimized"));
}

void UPerformanceOptimizer::OptimizePhysicsSimulation()
{
    if (!EnabledOptimizations[EOptimizationCategory::Physics])
    {
        return;
    }

    // Adjust physics simulation quality
    if (CurrentMetrics.GameThreadTimeMS > 20.0f)
    {
        SetConsoleVariable(TEXT("p.DefaultSolverIterations"), 4); // Reduce solver iterations
        SetConsoleVariable(TEXT("p.MaxSubsteps"), 3); // Reduce max substeps
    }
    else
    {
        SetConsoleVariable(TEXT("p.DefaultSolverIterations"), 8); // Default iterations
        SetConsoleVariable(TEXT("p.MaxSubsteps"), 6); // Default substeps
    }

    UE_LOG(LogPerformanceOptimizer, Verbose, TEXT("Physics simulation optimized"));
}

void UPerformanceOptimizer::OptimizeCollisionDetection()
{
    if (!EnabledOptimizations[EOptimizationCategory::Collision])
    {
        return;
    }

    // Adjust collision detection complexity
    if (CurrentMetrics.CollisionTests > 2000)
    {
        SetConsoleVariable(TEXT("p.EnableCCD"), 0); // Disable continuous collision detection
    }
    else
    {
        SetConsoleVariable(TEXT("p.EnableCCD"), 1); // Enable CCD
    }

    UE_LOG(LogPerformanceOptimizer, Verbose, TEXT("Collision detection optimized"));
}

void UPerformanceOptimizer::OptimizeRagdollPhysics()
{
    if (!EnabledOptimizations[EOptimizationCategory::Physics])
    {
        return;
    }

    // Optimize ragdoll physics for performance
    SetConsoleVariable(TEXT("p.RagdollSolverIterations"), 2); // Reduce ragdoll solver iterations
    
    UE_LOG(LogPerformanceOptimizer, Verbose, TEXT("Ragdoll physics optimized"));
}

void UPerformanceOptimizer::OptimizeMemoryUsage()
{
    if (!EnabledOptimizations[EOptimizationCategory::Memory])
    {
        return;
    }

    // Force garbage collection if memory usage is high
    if (CurrentMetrics.MemoryUsageMB > MemoryWarningThreshold)
    {
        GEngine->ForceGarbageCollection(true);
        UE_LOG(LogPerformanceOptimizer, Log, TEXT("Forced garbage collection due to high memory usage"));
    }
}

void UPerformanceOptimizer::OptimizeAssetStreaming()
{
    if (!EnabledOptimizations[EOptimizationCategory::Streaming])
    {
        return;
    }

    // Adjust streaming pool sizes based on memory usage
    if (CurrentMetrics.MemoryUsageMB > MemoryWarningThreshold)
    {
        SetConsoleVariable(TEXT("r.Streaming.PoolSize"), 512); // Reduce streaming pool
    }
    else
    {
        SetConsoleVariable(TEXT("r.Streaming.PoolSize"), 1024); // Default streaming pool
    }

    UE_LOG(LogPerformanceOptimizer, Verbose, TEXT("Asset streaming optimized"));
}

void UPerformanceOptimizer::OptimizeTextureStreaming()
{
    if (!EnabledOptimizations[EOptimizationCategory::Streaming])
    {
        return;
    }

    // Adjust texture streaming based on memory pressure
    if (CurrentMetrics.MemoryUsageMB > MemoryWarningThreshold)
    {
        SetConsoleVariable(TEXT("r.Streaming.MipBias"), 1.0f); // Use lower mip levels
    }
    else
    {
        SetConsoleVariable(TEXT("r.Streaming.MipBias"), 0.0f); // Use full resolution
    }

    UE_LOG(LogPerformanceOptimizer, Verbose, TEXT("Texture streaming optimized"));
}

void UPerformanceOptimizer::ApplyPlatformOptimizations()
{
    if (CurrentPlatform.Contains(TEXT("Windows")) || CurrentPlatform.Contains(TEXT("Win64")))
    {
        ConfigurePCOptimizations();
    }
    else
    {
        ConfigureConsoleOptimizations();
    }
}

void UPerformanceOptimizer::ConfigureConsoleOptimizations()
{
    // Console-specific optimizations
    SetTargetFrameRate(30.0f);
    CurrentQualityLevel = EPerformanceQuality::Medium;
    
    // Reduce various quality settings for console
    SetConsoleVariable(TEXT("sg.ViewDistanceScale"), 0.8f);
    SetConsoleVariable(TEXT("r.Shadow.MaxResolution"), 1024);
    SetConsoleVariable(TEXT("sg.PostProcessQuality"), 2);
    
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("Applied console-specific optimizations"));
}

void UPerformanceOptimizer::ConfigurePCOptimizations()
{
    // PC-specific optimizations
    SetTargetFrameRate(60.0f);
    CurrentQualityLevel = EPerformanceQuality::High;
    
    // Higher quality settings for PC
    SetConsoleVariable(TEXT("sg.ViewDistanceScale"), 1.0f);
    SetConsoleVariable(TEXT("r.Shadow.MaxResolution"), 2048);
    SetConsoleVariable(TEXT("sg.PostProcessQuality"), 3);
    
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("Applied PC-specific optimizations"));
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

float UPerformanceOptimizer::GetConsoleVariableFloat(const FString& VariableName)
{
    IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(*VariableName);
    return CVar ? CVar->GetFloat() : 0.0f;
}

int32 UPerformanceOptimizer::GetConsoleVariableInt(const FString& VariableName)
{
    IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(*VariableName);
    return CVar ? CVar->GetInt() : 0;
}