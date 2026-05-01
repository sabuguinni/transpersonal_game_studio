#include "Perf_FPSMonitor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"
#include "RenderingThread.h"
#include "RHI.h"

APerf_FPSMonitor::APerf_FPSMonitor()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.0f; // Tick every frame for accurate FPS measurement

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create monitor mesh component
    MonitorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MonitorMesh"));
    MonitorMesh->SetupAttachment(RootComponent);
    MonitorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Initialize default values
    UpdateInterval = 0.1f;
    SampleCount = 60;
    bEnableDetailedProfiling = true;
    bLogPerformanceWarnings = true;
    WarningFPSThreshold = 30.0f;
    CriticalFPSThreshold = 15.0f;

    // Initialize metrics
    CurrentMetrics = FPerf_PerformanceMetrics();
    TimeSinceLastUpdate = 0.0f;
    TotalFPS = 0.0f;
    FPSSampleCount = 0;

    // Reserve space for history arrays
    FPSHistory.Reserve(SampleCount);
    GameThreadHistory.Reserve(SampleCount);
    RenderThreadHistory.Reserve(SampleCount);
}

void APerf_FPSMonitor::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Log, TEXT("Performance Monitor started - monitoring at %f second intervals"), UpdateInterval);
    
    // Initialize performance tracking
    ResetMetrics();
    
    // Log initial state
    LogCurrentMetrics();
}

void APerf_FPSMonitor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update current FPS
    if (DeltaTime > 0.0f)
    {
        CurrentMetrics.CurrentFPS = 1.0f / DeltaTime;
        TotalFPS += CurrentMetrics.CurrentFPS;
        FPSSampleCount++;
    }

    TimeSinceLastUpdate += DeltaTime;

    // Update metrics at specified interval
    if (TimeSinceLastUpdate >= UpdateInterval)
    {
        UpdatePerformanceMetrics();
        TimeSinceLastUpdate = 0.0f;
    }
}

void APerf_FPSMonitor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UE_LOG(LogTemp, Log, TEXT("Performance Monitor stopped - Final average FPS: %.2f"), CurrentMetrics.AverageFPS);
    Super::EndPlay(EndPlayReason);
}

void APerf_FPSMonitor::UpdatePerformanceMetrics()
{
    // Calculate average FPS
    if (FPSSampleCount > 0)
    {
        CurrentMetrics.AverageFPS = TotalFPS / FPSSampleCount;
    }

    // Update FPS history
    UpdateFPSHistory(CurrentMetrics.CurrentFPS);

    // Calculate min/max FPS from history
    if (FPSHistory.Num() > 0)
    {
        CurrentMetrics.MinFPS = FMath::Min(FPSHistory);
        CurrentMetrics.MaxFPS = FMath::Max(FPSHistory);
    }

    // Gather detailed metrics if enabled
    if (bEnableDetailedProfiling)
    {
        GatherDetailedMetrics();
    }

    // Calculate performance level
    CalculatePerformanceLevel();

    // Check for performance warnings
    if (bLogPerformanceWarnings)
    {
        CheckPerformanceWarnings();
    }
}

void APerf_FPSMonitor::UpdateFPSHistory(float NewFPS)
{
    FPSHistory.Add(NewFPS);
    
    // Keep only the last SampleCount samples
    if (FPSHistory.Num() > SampleCount)
    {
        FPSHistory.RemoveAt(0);
    }
}

void APerf_FPSMonitor::CalculatePerformanceLevel()
{
    float FPS = CurrentMetrics.CurrentFPS;
    
    if (FPS >= 60.0f)
    {
        CurrentMetrics.PerformanceLevel = EPerf_PerformanceLevel::Excellent;
    }
    else if (FPS >= 45.0f)
    {
        CurrentMetrics.PerformanceLevel = EPerf_PerformanceLevel::Good;
    }
    else if (FPS >= 30.0f)
    {
        CurrentMetrics.PerformanceLevel = EPerf_PerformanceLevel::Acceptable;
    }
    else if (FPS >= 15.0f)
    {
        CurrentMetrics.PerformanceLevel = EPerf_PerformanceLevel::Poor;
    }
    else
    {
        CurrentMetrics.PerformanceLevel = EPerf_PerformanceLevel::Critical;
    }
}

void APerf_FPSMonitor::CheckPerformanceWarnings()
{
    if (CurrentMetrics.CurrentFPS < CriticalFPSThreshold)
    {
        UE_LOG(LogTemp, Error, TEXT("CRITICAL PERFORMANCE: FPS dropped to %.2f (below %.2f threshold)"), 
               CurrentMetrics.CurrentFPS, CriticalFPSThreshold);
    }
    else if (CurrentMetrics.CurrentFPS < WarningFPSThreshold)
    {
        UE_LOG(LogTemp, Warning, TEXT("Performance Warning: FPS is %.2f (below %.2f threshold)"), 
               CurrentMetrics.CurrentFPS, WarningFPSThreshold);
    }
}

void APerf_FPSMonitor::GatherDetailedMetrics()
{
    // Get engine stats
    if (GEngine)
    {
        // Game thread time (approximate)
        CurrentMetrics.GameThreadTime = 1000.0f / FMath::Max(CurrentMetrics.CurrentFPS, 1.0f);
        
        // Render thread time (approximate - same as game thread for now)
        CurrentMetrics.RenderThreadTime = CurrentMetrics.GameThreadTime;
        
        // GPU time (approximate)
        CurrentMetrics.GPUTime = CurrentMetrics.GameThreadTime * 0.8f; // Rough estimate
        
        // Memory usage (rough estimate)
        CurrentMetrics.MemoryUsageMB = FPlatformMemory::GetStats().UsedPhysical / (1024.0f * 1024.0f);
    }

    // Update thread time histories
    GameThreadHistory.Add(CurrentMetrics.GameThreadTime);
    if (GameThreadHistory.Num() > SampleCount)
    {
        GameThreadHistory.RemoveAt(0);
    }

    RenderThreadHistory.Add(CurrentMetrics.RenderThreadTime);
    if (RenderThreadHistory.Num() > SampleCount)
    {
        RenderThreadHistory.RemoveAt(0);
    }
}

void APerf_FPSMonitor::ResetMetrics()
{
    CurrentMetrics = FPerf_PerformanceMetrics();
    FPSHistory.Empty();
    GameThreadHistory.Empty();
    RenderThreadHistory.Empty();
    TotalFPS = 0.0f;
    FPSSampleCount = 0;
    TimeSinceLastUpdate = 0.0f;

    UE_LOG(LogTemp, Log, TEXT("Performance metrics reset"));
}

void APerf_FPSMonitor::LogCurrentMetrics()
{
    UE_LOG(LogTemp, Log, TEXT("=== PERFORMANCE METRICS ==="));
    UE_LOG(LogTemp, Log, TEXT("Current FPS: %.2f"), CurrentMetrics.CurrentFPS);
    UE_LOG(LogTemp, Log, TEXT("Average FPS: %.2f"), CurrentMetrics.AverageFPS);
    UE_LOG(LogTemp, Log, TEXT("Min FPS: %.2f"), CurrentMetrics.MinFPS);
    UE_LOG(LogTemp, Log, TEXT("Max FPS: %.2f"), CurrentMetrics.MaxFPS);
    UE_LOG(LogTemp, Log, TEXT("Game Thread: %.2fms"), CurrentMetrics.GameThreadTime);
    UE_LOG(LogTemp, Log, TEXT("Render Thread: %.2fms"), CurrentMetrics.RenderThreadTime);
    UE_LOG(LogTemp, Log, TEXT("GPU Time: %.2fms"), CurrentMetrics.GPUTime);
    UE_LOG(LogTemp, Log, TEXT("Memory Usage: %.2fMB"), CurrentMetrics.MemoryUsageMB);
    
    FString PerformanceLevelStr;
    switch (CurrentMetrics.PerformanceLevel)
    {
        case EPerf_PerformanceLevel::Excellent: PerformanceLevelStr = TEXT("Excellent"); break;
        case EPerf_PerformanceLevel::Good: PerformanceLevelStr = TEXT("Good"); break;
        case EPerf_PerformanceLevel::Acceptable: PerformanceLevelStr = TEXT("Acceptable"); break;
        case EPerf_PerformanceLevel::Poor: PerformanceLevelStr = TEXT("Poor"); break;
        case EPerf_PerformanceLevel::Critical: PerformanceLevelStr = TEXT("Critical"); break;
        default: PerformanceLevelStr = TEXT("Unknown"); break;
    }
    UE_LOG(LogTemp, Log, TEXT("Performance Level: %s"), *PerformanceLevelStr);
    UE_LOG(LogTemp, Log, TEXT("=========================="));
}

void APerf_FPSMonitor::OptimizeForTarget60FPS()
{
    UE_LOG(LogTemp, Log, TEXT("Applying optimizations for 60 FPS target..."));
    
    // Apply console commands for 60 FPS optimization
    if (UWorld* World = GetWorld())
    {
        // Reduce shadow quality slightly
        GEngine->Exec(World, TEXT("r.ShadowQuality 3"));
        
        // Optimize post-processing
        GEngine->Exec(World, TEXT("r.PostProcessAAQuality 2"));
        
        // Optimize effects quality
        GEngine->Exec(World, TEXT("r.EffectsQuality 3"));
        
        // Enable temporal upsampling for better performance
        GEngine->Exec(World, TEXT("r.TemporalAA.Upsampling 1"));
        
        UE_LOG(LogTemp, Log, TEXT("60 FPS optimizations applied"));
    }
}

void APerf_FPSMonitor::OptimizeForTarget30FPS()
{
    UE_LOG(LogTemp, Log, TEXT("Applying optimizations for 30 FPS target..."));
    
    if (UWorld* World = GetWorld())
    {
        // Higher quality settings for 30 FPS target
        GEngine->Exec(World, TEXT("r.ShadowQuality 4"));
        GEngine->Exec(World, TEXT("r.PostProcessAAQuality 3"));
        GEngine->Exec(World, TEXT("r.EffectsQuality 4"));
        GEngine->Exec(World, TEXT("r.ViewDistanceScale 1.0"));
        
        UE_LOG(LogTemp, Log, TEXT("30 FPS optimizations applied"));
    }
}

void APerf_FPSMonitor::ApplyLODOptimizations()
{
    UE_LOG(LogTemp, Log, TEXT("Applying LOD optimizations..."));
    
    if (UWorld* World = GetWorld())
    {
        // Aggressive LOD settings for performance
        GEngine->Exec(World, TEXT("r.ForceLOD 1"));
        GEngine->Exec(World, TEXT("r.SkeletalMeshLODBias 1"));
        GEngine->Exec(World, TEXT("r.StaticMeshLODDistanceScale 0.8"));
        
        UE_LOG(LogTemp, Log, TEXT("LOD optimizations applied"));
    }
}

void APerf_FPSMonitor::ReduceDrawCalls()
{
    UE_LOG(LogTemp, Log, TEXT("Applying draw call reduction optimizations..."));
    
    if (UWorld* World = GetWorld())
    {
        // Enable instancing and batching
        GEngine->Exec(World, TEXT("r.AllowStaticLighting 1"));
        GEngine->Exec(World, TEXT("r.GenerateMeshDistanceFields 0"));
        
        // Reduce particle complexity
        GEngine->Exec(World, TEXT("fx.MaxGPUParticlesSpawnedPerFrame 512"));
        
        UE_LOG(LogTemp, Log, TEXT("Draw call reduction optimizations applied"));
    }
}