#include "PerformanceMonitor.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"
#include "Stats/StatsHierarchical.h"
#include "RenderingThread.h"
#include "HAL/PlatformMemory.h"
#include "Engine/Level.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "TimerManager.h"

DEFINE_STAT(STAT_PerformanceMonitorUpdate);
DEFINE_STAT(STAT_ActiveActors);
DEFINE_STAT(STAT_FrameTimeMS);

UPerformanceMonitor::UPerformanceMonitor()
    : bIsMonitoring(false)
    , MonitoringInterval(0.1f)  // Update every 100ms
    , LastUpdateTime(0.0f)
{
    // Initialize default thresholds
    Thresholds = FPerf_PerformanceThresholds();
}

void UPerformanceMonitor::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("PerformanceMonitor: Initialized"));
    
    // Start monitoring by default in development builds
#if UE_BUILD_DEVELOPMENT || UE_BUILD_DEBUG
    StartPerformanceMonitoring();
#endif
}

void UPerformanceMonitor::Deinitialize()
{
    StopPerformanceMonitoring();
    Super::Deinitialize();
}

bool UPerformanceMonitor::ShouldCreateSubsystem(UObject* Outer) const
{
    return true;
}

void UPerformanceMonitor::StartPerformanceMonitoring()
{
    if (bIsMonitoring)
    {
        return;
    }

    bIsMonitoring = true;
    
    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().SetTimer(
            MonitoringTimerHandle,
            this,
            &UPerformanceMonitor::UpdatePerformanceMetrics,
            MonitoringInterval,
            true
        );
    }

    UE_LOG(LogTemp, Log, TEXT("PerformanceMonitor: Started monitoring"));
}

void UPerformanceMonitor::StopPerformanceMonitoring()
{
    if (!bIsMonitoring)
    {
        return;
    }

    bIsMonitoring = false;
    
    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().ClearTimer(MonitoringTimerHandle);
    }

    UE_LOG(LogTemp, Log, TEXT("PerformanceMonitor: Stopped monitoring"));
}

void UPerformanceMonitor::UpdatePerformanceMetrics()
{
    SCOPE_CYCLE_COUNTER(STAT_PerformanceMonitorUpdate);

    if (!bIsMonitoring)
    {
        return;
    }

    // Collect all performance metrics
    CollectFrameMetrics();
    CollectActorMetrics();
    CollectGPUMetrics();
    CollectMemoryMetrics();

    // Check if we're exceeding thresholds
    CheckPerformanceThresholds();

    // Update stats
    SET_FLOAT_STAT(STAT_FrameTimeMS, CurrentMetrics.FrameTimeMS);
    SET_DWORD_STAT(STAT_ActiveActors, CurrentMetrics.ActiveActorCount);

    LastUpdateTime = GetWorld()->GetTimeSeconds();
}

void UPerformanceMonitor::CollectFrameMetrics()
{
    // Get frame time from engine stats
    if (GEngine)
    {
        CurrentMetrics.FrameTimeMS = GEngine->GetMaxTickRate() > 0 ? (1000.0f / GEngine->GetMaxTickRate()) : 0.0f;
        CurrentMetrics.FPS = CurrentMetrics.FrameTimeMS > 0 ? (1000.0f / CurrentMetrics.FrameTimeMS) : 0.0f;
    }

    // Get more accurate frame time from game thread
    if (FPlatformTime::Seconds() > 0)
    {
        static double LastFrameTime = FPlatformTime::Seconds();
        double CurrentTime = FPlatformTime::Seconds();
        double DeltaTime = CurrentTime - LastFrameTime;
        
        if (DeltaTime > 0)
        {
            CurrentMetrics.FrameTimeMS = DeltaTime * 1000.0f;
            CurrentMetrics.FPS = 1.0f / DeltaTime;
        }
        
        LastFrameTime = CurrentTime;
    }
}

void UPerformanceMonitor::CollectActorMetrics()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    CurrentMetrics.ActiveActorCount = 0;
    CurrentMetrics.VisibleActorCount = 0;
    CurrentMetrics.Triangles = 0;

    // Count all actors in the world
    for (ULevel* Level : World->GetLevels())
    {
        if (Level)
        {
            CurrentMetrics.ActiveActorCount += Level->Actors.Num();
            
            // Count visible actors and triangles
            for (AActor* Actor : Level->Actors)
            {
                if (Actor && !Actor->IsHidden())
                {
                    CurrentMetrics.VisibleActorCount++;
                    
                    // Count triangles from mesh components
                    TArray<UStaticMeshComponent*> StaticMeshComps;
                    Actor->GetComponents<UStaticMeshComponent>(StaticMeshComps);
                    
                    for (UStaticMeshComponent* MeshComp : StaticMeshComps)
                    {
                        if (MeshComp && MeshComp->GetStaticMesh())
                        {
                            // Approximate triangle count
                            CurrentMetrics.Triangles += 1000; // Placeholder - would need proper LOD analysis
                        }
                    }
                }
            }
        }
    }
}

void UPerformanceMonitor::CollectGPUMetrics()
{
    // GPU metrics collection - simplified for now
    // In a full implementation, this would use RenderDoc integration or GPU profiling APIs
    
    // Estimate GPU time based on frame time and complexity
    float EstimatedGPUTime = CurrentMetrics.FrameTimeMS * 0.6f; // Assume 60% of frame time is GPU
    CurrentMetrics.GPUTimeMS = EstimatedGPUTime;
    
    // Estimate draw calls based on visible actors
    CurrentMetrics.DrawCalls = CurrentMetrics.VisibleActorCount * 2; // Rough estimate
}

void UPerformanceMonitor::CollectMemoryMetrics()
{
    // Get memory usage from platform
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
}

EPerf_PerformanceLevel UPerformanceMonitor::GetPerformanceLevel() const
{
    // Determine performance level based on multiple factors
    int32 IssueCount = 0;
    
    if (CurrentMetrics.FPS < Thresholds.TargetFPS * 0.9f) IssueCount++;
    if (CurrentMetrics.FrameTimeMS > Thresholds.MaxFrameTimeMS) IssueCount++;
    if (CurrentMetrics.GPUTimeMS > Thresholds.MaxGPUTimeMS) IssueCount++;
    if (CurrentMetrics.MemoryUsageMB > Thresholds.MaxMemoryUsageMB) IssueCount++;
    if (CurrentMetrics.DrawCalls > Thresholds.MaxDrawCalls) IssueCount++;
    
    if (IssueCount == 0)
    {
        return EPerf_PerformanceLevel::Excellent;
    }
    else if (IssueCount <= 1)
    {
        return EPerf_PerformanceLevel::Good;
    }
    else if (IssueCount <= 2)
    {
        return EPerf_PerformanceLevel::Acceptable;
    }
    else if (IssueCount <= 3)
    {
        return EPerf_PerformanceLevel::Poor;
    }
    else
    {
        return EPerf_PerformanceLevel::Critical;
    }
}

void UPerformanceMonitor::SetPerformanceThresholds(const FPerf_PerformanceThresholds& NewThresholds)
{
    Thresholds = NewThresholds;
    UE_LOG(LogTemp, Log, TEXT("PerformanceMonitor: Updated performance thresholds"));
}

void UPerformanceMonitor::CheckPerformanceThresholds()
{
    EPerf_PerformanceLevel Level = GetPerformanceLevel();
    
    if (Level == EPerf_PerformanceLevel::Poor || Level == EPerf_PerformanceLevel::Critical)
    {
        UE_LOG(LogTemp, Warning, TEXT("PerformanceMonitor: Performance level is %s - FPS: %.1f, Frame Time: %.2fms"), 
            Level == EPerf_PerformanceLevel::Poor ? TEXT("POOR") : TEXT("CRITICAL"),
            CurrentMetrics.FPS, CurrentMetrics.FrameTimeMS);
        
        // Auto-apply optimizations if performance is critical
        if (Level == EPerf_PerformanceLevel::Critical)
        {
            ApplyPerformanceOptimizations();
        }
    }
}

void UPerformanceMonitor::ApplyPerformanceOptimizations()
{
    UE_LOG(LogTemp, Log, TEXT("PerformanceMonitor: Applying automatic performance optimizations"));
    
    OptimizeLODSettings();
    OptimizeCullingSettings();
    OptimizeShadowSettings();
}

void UPerformanceMonitor::OptimizeLODSettings()
{
    // Apply LOD optimizations
    if (auto* CVarForceLOD = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ForceLOD")))
    {
        CVarForceLOD->Set(1); // Force LOD 1 for better performance
    }
    
    if (auto* CVarLODBias = IConsoleManager::Get().FindConsoleVariable(TEXT("r.SkeletalMeshLODBias")))
    {
        CVarLODBias->Set(1); // Increase LOD bias
    }
    
    UE_LOG(LogTemp, Log, TEXT("PerformanceMonitor: Applied LOD optimizations"));
}

void UPerformanceMonitor::OptimizeCullingSettings()
{
    // Apply culling optimizations
    if (auto* CVarOcclusionCulling = IConsoleManager::Get().FindConsoleVariable(TEXT("r.AllowOcclusionQueries")))
    {
        CVarOcclusionCulling->Set(1); // Enable occlusion culling
    }
    
    if (auto* CVarViewDistanceScale = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ViewDistanceScale")))
    {
        CVarViewDistanceScale->Set(0.8f); // Reduce view distance
    }
    
    UE_LOG(LogTemp, Log, TEXT("PerformanceMonitor: Applied culling optimizations"));
}

void UPerformanceMonitor::OptimizeShadowSettings()
{
    // Apply shadow optimizations
    if (auto* CVarShadowResolution = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Shadow.MaxResolution")))
    {
        CVarShadowResolution->Set(1024); // Reduce shadow resolution
    }
    
    if (auto* CVarShadowDistance = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Shadow.DistanceScale")))
    {
        CVarShadowDistance->Set(0.6f); // Reduce shadow distance
    }
    
    UE_LOG(LogTemp, Log, TEXT("PerformanceMonitor: Applied shadow optimizations"));
}

void UPerformanceMonitor::OptimizeTextureSettings()
{
    // Apply texture optimizations
    if (auto* CVarTexturePool = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Streaming.PoolSize")))
    {
        CVarTexturePool->Set(1000); // Reduce texture pool size
    }
    
    UE_LOG(LogTemp, Log, TEXT("PerformanceMonitor: Applied texture optimizations"));
}

void UPerformanceMonitor::EnablePerformanceStats()
{
    UWorld* World = GetWorld();
    if (World)
    {
        GEngine->Exec(World, TEXT("stat fps"));
        GEngine->Exec(World, TEXT("stat unit"));
        GEngine->Exec(World, TEXT("stat memory"));
        GEngine->Exec(World, TEXT("stat gpu"));
    }
    
    UE_LOG(LogTemp, Log, TEXT("PerformanceMonitor: Enabled performance stats display"));
}

void UPerformanceMonitor::DisablePerformanceStats()
{
    UWorld* World = GetWorld();
    if (World)
    {
        GEngine->Exec(World, TEXT("stat none"));
    }
    
    UE_LOG(LogTemp, Log, TEXT("PerformanceMonitor: Disabled performance stats display"));
}

void UPerformanceMonitor::DumpPerformanceReport()
{
    UE_LOG(LogTemp, Log, TEXT("=== PERFORMANCE REPORT ==="));
    UE_LOG(LogTemp, Log, TEXT("FPS: %.1f"), CurrentMetrics.FPS);
    UE_LOG(LogTemp, Log, TEXT("Frame Time: %.2f ms"), CurrentMetrics.FrameTimeMS);
    UE_LOG(LogTemp, Log, TEXT("Active Actors: %d"), CurrentMetrics.ActiveActorCount);
    UE_LOG(LogTemp, Log, TEXT("Visible Actors: %d"), CurrentMetrics.VisibleActorCount);
    UE_LOG(LogTemp, Log, TEXT("GPU Time: %.2f ms"), CurrentMetrics.GPUTimeMS);
    UE_LOG(LogTemp, Log, TEXT("Memory Usage: %.1f MB"), CurrentMetrics.MemoryUsageMB);
    UE_LOG(LogTemp, Log, TEXT("Draw Calls: %d"), CurrentMetrics.DrawCalls);
    UE_LOG(LogTemp, Log, TEXT("Triangles: %d"), CurrentMetrics.Triangles);
    UE_LOG(LogTemp, Log, TEXT("Performance Level: %d"), (int32)GetPerformanceLevel());
    UE_LOG(LogTemp, Log, TEXT("=== END REPORT ==="));
}