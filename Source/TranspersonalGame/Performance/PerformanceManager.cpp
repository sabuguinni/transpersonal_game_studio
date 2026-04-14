#include "PerformanceManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameViewportClient.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Misc/FileHelper.h"
#include "Stats/Stats.h"
#include "RenderingThread.h"
#include "Engine/GameInstance.h"
#include "TimerManager.h"

UPerformanceManager::UPerformanceManager()
{
    bIsMonitoring = false;
    MonitoringInterval = 0.1f;
    MaxHistorySize = 300;
    
    // Initialize default optimization settings
    OptimizationSettings.TargetPerformanceLevel = EPerf_PerformanceLevel::High;
    OptimizationSettings.TargetFPS = 60.0f;
    OptimizationSettings.MaxDrawCalls = 2000;
    OptimizationSettings.MaxTriangles = 2000000;
    OptimizationSettings.MaxMemoryUsageMB = 4096.0f;
    OptimizationSettings.bEnableAutoOptimization = true;
    OptimizationSettings.bEnableLODOptimization = true;
    OptimizationSettings.bEnableCullingOptimization = true;
    OptimizationSettings.bEnableTextureStreaming = true;
    OptimizationSettings.bEnableGarbageCollectionOptimization = true;
}

void UPerformanceManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("PerformanceManager initialized"));
    
    // Start monitoring by default
    StartPerformanceMonitoring();
    
    // Apply default performance settings
    ApplyPerformanceLevel(OptimizationSettings.TargetPerformanceLevel);
}

void UPerformanceManager::Deinitialize()
{
    StopPerformanceMonitoring();
    Super::Deinitialize();
}

void UPerformanceManager::StartPerformanceMonitoring()
{
    if (bIsMonitoring)
    {
        return;
    }
    
    bIsMonitoring = true;
    FPSHistory.Empty();
    
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        if (UWorld* World = GameInstance->GetWorld())
        {
            World->GetTimerManager().SetTimer(
                MonitoringTimerHandle,
                this,
                &UPerformanceManager::UpdatePerformanceMetrics,
                MonitoringInterval,
                true
            );
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Performance monitoring started"));
}

void UPerformanceManager::StopPerformanceMonitoring()
{
    if (!bIsMonitoring)
    {
        return;
    }
    
    bIsMonitoring = false;
    
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        if (UWorld* World = GameInstance->GetWorld())
        {
            World->GetTimerManager().ClearTimer(MonitoringTimerHandle);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Performance monitoring stopped"));
}

FPerf_PerformanceMetrics UPerformanceManager::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

void UPerformanceManager::UpdatePerformanceMetrics()
{
    if (!bIsMonitoring)
    {
        return;
    }
    
    // Update FPS metrics
    if (GEngine && GEngine->GetGameViewport())
    {
        CurrentMetrics.CurrentFPS = 1.0f / FApp::GetDeltaTime();
        CurrentMetrics.FrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds
        
        UpdateFPSHistory(CurrentMetrics.CurrentFPS);
        CalculateAverageFPS();
    }
    
    // Update thread times (simplified - would need more complex stats integration)
    CurrentMetrics.GameThreadTime = FApp::GetDeltaTime() * 1000.0f * 0.6f; // Estimate
    CurrentMetrics.RenderThreadTime = FApp::GetDeltaTime() * 1000.0f * 0.3f; // Estimate
    CurrentMetrics.GPUTime = FApp::GetDeltaTime() * 1000.0f * 0.4f; // Estimate
    
    // Update memory usage
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.UsedMemoryMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    CurrentMetrics.AvailableMemoryMB = MemStats.AvailablePhysical / (1024.0f * 1024.0f);
    
    // Update actor counts
    if (UWorld* World = GetWorld())
    {
        CurrentMetrics.ActiveActors = World->GetActorCount();
        // VisibleActors would need renderer integration
        CurrentMetrics.VisibleActors = CurrentMetrics.ActiveActors; // Simplified
    }
    
    // Detect bottleneck
    CurrentMetrics.PrimaryBottleneck = DetectBottleneck();
    
    // Auto-optimize if enabled
    if (OptimizationSettings.bEnableAutoOptimization)
    {
        if (CurrentMetrics.CurrentFPS < OptimizationSettings.TargetFPS * 0.8f)
        {
            AutoOptimizeBottleneck();
        }
    }
}

void UPerformanceManager::SetOptimizationSettings(const FPerf_OptimizationSettings& NewSettings)
{
    OptimizationSettings = NewSettings;
    UE_LOG(LogTemp, Log, TEXT("Optimization settings updated"));
}

FPerf_OptimizationSettings UPerformanceManager::GetOptimizationSettings() const
{
    return OptimizationSettings;
}

void UPerformanceManager::ApplyPerformanceLevel(EPerf_PerformanceLevel Level)
{
    switch (Level)
    {
        case EPerf_PerformanceLevel::Ultra:
            ApplyUltraSettings();
            break;
        case EPerf_PerformanceLevel::High:
            ApplyHighSettings();
            break;
        case EPerf_PerformanceLevel::Medium:
            ApplyMediumSettings();
            break;
        case EPerf_PerformanceLevel::Low:
            ApplyLowSettings();
            break;
        case EPerf_PerformanceLevel::Potato:
            ApplyPotatoSettings();
            break;
    }
    
    OptimizationSettings.TargetPerformanceLevel = Level;
    UE_LOG(LogTemp, Log, TEXT("Applied performance level: %d"), (int32)Level);
}

void UPerformanceManager::OptimizeForTarget(float TargetFPS)
{
    OptimizationSettings.TargetFPS = TargetFPS;
    
    if (TargetFPS >= 120.0f)
    {
        ApplyPerformanceLevel(EPerf_PerformanceLevel::Ultra);
    }
    else if (TargetFPS >= 60.0f)
    {
        ApplyPerformanceLevel(EPerf_PerformanceLevel::High);
    }
    else if (TargetFPS >= 45.0f)
    {
        ApplyPerformanceLevel(EPerf_PerformanceLevel::Medium);
    }
    else if (TargetFPS >= 30.0f)
    {
        ApplyPerformanceLevel(EPerf_PerformanceLevel::Low);
    }
    else
    {
        ApplyPerformanceLevel(EPerf_PerformanceLevel::Potato);
    }
}

EPerf_BottleneckType UPerformanceManager::DetectBottleneck()
{
    // Simple bottleneck detection based on thread times
    if (CurrentMetrics.GPUTime > CurrentMetrics.GameThreadTime * 1.5f)
    {
        return EPerf_BottleneckType::GPU;
    }
    else if (CurrentMetrics.GameThreadTime > 16.0f) // 60 FPS threshold
    {
        return EPerf_BottleneckType::CPU;
    }
    else if (CurrentMetrics.UsedMemoryMB > OptimizationSettings.MaxMemoryUsageMB * 0.9f)
    {
        return EPerf_BottleneckType::Memory;
    }
    
    return EPerf_BottleneckType::None;
}

void UPerformanceManager::AutoOptimizeBottleneck()
{
    EPerf_BottleneckType Bottleneck = CurrentMetrics.PrimaryBottleneck;
    
    switch (Bottleneck)
    {
        case EPerf_BottleneckType::GPU:
            OptimizeRenderingSettings();
            OptimizeLODSettings();
            break;
        case EPerf_BottleneckType::CPU:
            OptimizeCullingSettings();
            break;
        case EPerf_BottleneckType::Memory:
            OptimizeMemoryUsage();
            ForceGarbageCollection();
            break;
        default:
            break;
    }
}

void UPerformanceManager::OptimizeLODSettings()
{
    if (!OptimizationSettings.bEnableLODOptimization)
    {
        return;
    }
    
    // Apply LOD optimizations via console commands
    if (UWorld* World = GetWorld())
    {
        GEngine->Exec(World, TEXT("r.ForceLOD 1"));
        GEngine->Exec(World, TEXT("r.SkeletalMeshLODBias 1"));
        GEngine->Exec(World, TEXT("r.StaticMeshLODDistanceScale 2.0"));
    }
    
    UE_LOG(LogTemp, Log, TEXT("LOD settings optimized"));
}

void UPerformanceManager::OptimizeCullingSettings()
{
    if (!OptimizationSettings.bEnableCullingOptimization)
    {
        return;
    }
    
    if (UWorld* World = GetWorld())
    {
        GEngine->Exec(World, TEXT("r.CullDistanceScale 0.8"));
        GEngine->Exec(World, TEXT("r.ViewDistanceScale 0.8"));
        GEngine->Exec(World, TEXT("foliage.CullDistanceScale 0.6"));
    }
    
    UE_LOG(LogTemp, Log, TEXT("Culling settings optimized"));
}

void UPerformanceManager::OptimizeMemoryUsage()
{
    if (!OptimizationSettings.bEnableGarbageCollectionOptimization)
    {
        return;
    }
    
    // Optimize texture streaming
    if (UWorld* World = GetWorld())
    {
        GEngine->Exec(World, TEXT("r.Streaming.PoolSize 1024"));
        GEngine->Exec(World, TEXT("r.Streaming.MaxTempMemoryAllowed 100"));
    }
    
    UE_LOG(LogTemp, Log, TEXT("Memory usage optimized"));
}

void UPerformanceManager::ForceGarbageCollection()
{
    if (UWorld* World = GetWorld())
    {
        GEngine->Exec(World, TEXT("gc.ForceGC"));
    }
    
    UE_LOG(LogTemp, Log, TEXT("Forced garbage collection"));
}

void UPerformanceManager::OptimizeRenderingSettings()
{
    if (UWorld* World = GetWorld())
    {
        // Reduce shadow quality
        GEngine->Exec(World, TEXT("r.Shadow.MaxResolution 1024"));
        GEngine->Exec(World, TEXT("r.Shadow.RadiusThreshold 0.03"));
        
        // Optimize post-processing
        GEngine->Exec(World, TEXT("r.PostProcessAAQuality 2"));
        GEngine->Exec(World, TEXT("r.MotionBlurQuality 1"));
    }
    
    UE_LOG(LogTemp, Log, TEXT("Rendering settings optimized"));
}

void UPerformanceManager::SetScreenPercentage(float Percentage)
{
    if (UWorld* World = GetWorld())
    {
        FString Command = FString::Printf(TEXT("r.ScreenPercentage %f"), Percentage);
        GEngine->Exec(World, *Command);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Screen percentage set to: %f"), Percentage);
}

void UPerformanceManager::GeneratePerformanceReport()
{
    FString Report = TEXT("=== PERFORMANCE REPORT ===\n");
    Report += FString::Printf(TEXT("Current FPS: %.2f\n"), CurrentMetrics.CurrentFPS);
    Report += FString::Printf(TEXT("Average FPS: %.2f\n"), CurrentMetrics.AverageFPS);
    Report += FString::Printf(TEXT("Min FPS: %.2f\n"), CurrentMetrics.MinFPS);
    Report += FString::Printf(TEXT("Max FPS: %.2f\n"), CurrentMetrics.MaxFPS);
    Report += FString::Printf(TEXT("Frame Time: %.2f ms\n"), CurrentMetrics.FrameTime);
    Report += FString::Printf(TEXT("Game Thread: %.2f ms\n"), CurrentMetrics.GameThreadTime);
    Report += FString::Printf(TEXT("Render Thread: %.2f ms\n"), CurrentMetrics.RenderThreadTime);
    Report += FString::Printf(TEXT("GPU Time: %.2f ms\n"), CurrentMetrics.GPUTime);
    Report += FString::Printf(TEXT("Memory Used: %.2f MB\n"), CurrentMetrics.UsedMemoryMB);
    Report += FString::Printf(TEXT("Active Actors: %d\n"), CurrentMetrics.ActiveActors);
    Report += FString::Printf(TEXT("Primary Bottleneck: %d\n"), (int32)CurrentMetrics.PrimaryBottleneck);
    
    // Save report to file
    FString FileName = FString::Printf(TEXT("PerformanceReport_%s.txt"), 
        *FDateTime::Now().ToString());
    FString FilePath = FPaths::ProjectLogDir() / FileName;
    FFileHelper::SaveStringToFile(Report, *FilePath);
    
    UE_LOG(LogTemp, Log, TEXT("Performance report generated: %s"), *FilePath);
}

void UPerformanceManager::RunPerformanceBenchmark()
{
    UE_LOG(LogTemp, Log, TEXT("Running performance benchmark..."));
    
    // Reset metrics
    FPSHistory.Empty();
    CurrentMetrics = FPerf_PerformanceMetrics();
    
    // Run benchmark for 30 seconds
    StartPerformanceMonitoring();
    
    if (UWorld* World = GetWorld())
    {
        // Enable all stats for comprehensive testing
        GEngine->Exec(World, TEXT("stat fps"));
        GEngine->Exec(World, TEXT("stat unit"));
        GEngine->Exec(World, TEXT("stat memory"));
        GEngine->Exec(World, TEXT("stat gpu"));
        GEngine->Exec(World, TEXT("stat rhi"));
        GEngine->Exec(World, TEXT("stat scenerendering"));
        
        // Schedule report generation
        FTimerHandle ReportTimer;
        World->GetTimerManager().SetTimer(ReportTimer, this, 
            &UPerformanceManager::GeneratePerformanceReport, 30.0f, false);
    }
}

void UPerformanceManager::UpdateFPSHistory(float NewFPS)
{
    FPSHistory.Add(NewFPS);
    
    if (FPSHistory.Num() > MaxHistorySize)
    {
        FPSHistory.RemoveAt(0);
    }
}

void UPerformanceManager::CalculateAverageFPS()
{
    if (FPSHistory.Num() == 0)
    {
        return;
    }
    
    float Total = 0.0f;
    float Min = FPSHistory[0];
    float Max = FPSHistory[0];
    
    for (float FPS : FPSHistory)
    {
        Total += FPS;
        Min = FMath::Min(Min, FPS);
        Max = FMath::Max(Max, FPS);
    }
    
    CurrentMetrics.AverageFPS = Total / FPSHistory.Num();
    CurrentMetrics.MinFPS = Min;
    CurrentMetrics.MaxFPS = Max;
}

void UPerformanceManager::ApplyUltraSettings()
{
    if (UWorld* World = GetWorld())
    {
        GEngine->Exec(World, TEXT("sg.ViewDistanceQuality 4"));
        GEngine->Exec(World, TEXT("sg.AntiAliasingQuality 4"));
        GEngine->Exec(World, TEXT("sg.ShadowQuality 4"));
        GEngine->Exec(World, TEXT("sg.PostProcessQuality 4"));
        GEngine->Exec(World, TEXT("sg.TextureQuality 4"));
        GEngine->Exec(World, TEXT("sg.EffectsQuality 4"));
        GEngine->Exec(World, TEXT("sg.FoliageQuality 4"));
        GEngine->Exec(World, TEXT("r.ScreenPercentage 100"));
    }
}

void UPerformanceManager::ApplyHighSettings()
{
    if (UWorld* World = GetWorld())
    {
        GEngine->Exec(World, TEXT("sg.ViewDistanceQuality 3"));
        GEngine->Exec(World, TEXT("sg.AntiAliasingQuality 3"));
        GEngine->Exec(World, TEXT("sg.ShadowQuality 3"));
        GEngine->Exec(World, TEXT("sg.PostProcessQuality 3"));
        GEngine->Exec(World, TEXT("sg.TextureQuality 3"));
        GEngine->Exec(World, TEXT("sg.EffectsQuality 3"));
        GEngine->Exec(World, TEXT("sg.FoliageQuality 3"));
        GEngine->Exec(World, TEXT("r.ScreenPercentage 100"));
    }
}

void UPerformanceManager::ApplyMediumSettings()
{
    if (UWorld* World = GetWorld())
    {
        GEngine->Exec(World, TEXT("sg.ViewDistanceQuality 2"));
        GEngine->Exec(World, TEXT("sg.AntiAliasingQuality 2"));
        GEngine->Exec(World, TEXT("sg.ShadowQuality 2"));
        GEngine->Exec(World, TEXT("sg.PostProcessQuality 2"));
        GEngine->Exec(World, TEXT("sg.TextureQuality 2"));
        GEngine->Exec(World, TEXT("sg.EffectsQuality 2"));
        GEngine->Exec(World, TEXT("sg.FoliageQuality 2"));
        GEngine->Exec(World, TEXT("r.ScreenPercentage 90"));
    }
}

void UPerformanceManager::ApplyLowSettings()
{
    if (UWorld* World = GetWorld())
    {
        GEngine->Exec(World, TEXT("sg.ViewDistanceQuality 1"));
        GEngine->Exec(World, TEXT("sg.AntiAliasingQuality 1"));
        GEngine->Exec(World, TEXT("sg.ShadowQuality 1"));
        GEngine->Exec(World, TEXT("sg.PostProcessQuality 1"));
        GEngine->Exec(World, TEXT("sg.TextureQuality 1"));
        GEngine->Exec(World, TEXT("sg.EffectsQuality 1"));
        GEngine->Exec(World, TEXT("sg.FoliageQuality 1"));
        GEngine->Exec(World, TEXT("r.ScreenPercentage 80"));
    }
}

void UPerformanceManager::ApplyPotatoSettings()
{
    if (UWorld* World = GetWorld())
    {
        GEngine->Exec(World, TEXT("sg.ViewDistanceQuality 0"));
        GEngine->Exec(World, TEXT("sg.AntiAliasingQuality 0"));
        GEngine->Exec(World, TEXT("sg.ShadowQuality 0"));
        GEngine->Exec(World, TEXT("sg.PostProcessQuality 0"));
        GEngine->Exec(World, TEXT("sg.TextureQuality 0"));
        GEngine->Exec(World, TEXT("sg.EffectsQuality 0"));
        GEngine->Exec(World, TEXT("sg.FoliageQuality 0"));
        GEngine->Exec(World, TEXT("r.ScreenPercentage 70"));
    }
}

void UPerformanceManager::LogPerformanceWarning(const FString& Warning)
{
    UE_LOG(LogTemp, Warning, TEXT("Performance Warning: %s"), *Warning);
}