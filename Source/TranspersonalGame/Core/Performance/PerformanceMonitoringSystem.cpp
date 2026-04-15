#include "PerformanceMonitoringSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Stats/StatsHierarchical.h"
#include "RenderingThread.h"
#include "RHI.h"
#include "Engine/GameViewportClient.h"
#include "EngineUtils.h"

DEFINE_STAT(STAT_PerformanceMonitorUpdate);
DEFINE_STAT(STAT_ActiveActors);
DEFINE_STAT(STAT_FrameTimeMS);

APerformanceMonitoringSystem::APerformanceMonitoringSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.0f; // Tick every frame for accurate monitoring

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default values
    MonitoringInterval = 0.1f; // Update metrics 10 times per second
    bEnableLogging = true;
    bEnableAutomaticOptimization = false;
    MaxHistorySize = 300; // 30 seconds at 10Hz
    
    TimeSinceLastUpdate = 0.0f;
    AccumulatedFrameTime = 0.0f;
    FrameCount = 0;

    // Initialize thresholds
    PerformanceThresholds = FPerf_PerformanceThresholds();
}

void APerformanceMonitoringSystem::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Warning, TEXT("Performance Monitoring System started"));
    
    // Initialize performance history
    FrameTimeHistory.Reserve(MaxHistorySize);
    
    // Enable stats collection
    if (GEngine && GEngine->GetGameViewport())
    {
        GEngine->Exec(GetWorld(), TEXT("stat fps"));
        GEngine->Exec(GetWorld(), TEXT("stat unit"));
    }
}

void APerformanceMonitoringSystem::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (bEnableLogging)
    {
        LogPerformanceReport();
    }

    Super::EndPlay(EndPlayReason);
}

void APerformanceMonitoringSystem::Tick(float DeltaTime)
{
    SCOPE_CYCLE_COUNTER(STAT_PerformanceMonitorUpdate);
    
    Super::Tick(DeltaTime);

    // Accumulate frame time data
    AccumulatedFrameTime += DeltaTime;
    FrameCount++;
    TimeSinceLastUpdate += DeltaTime;

    // Update metrics at specified interval
    if (TimeSinceLastUpdate >= MonitoringInterval)
    {
        UpdatePerformanceMetrics();
        TimeSinceLastUpdate = 0.0f;
        
        // Check if automatic optimization is needed
        if (bEnableAutomaticOptimization && !IsPerformanceAcceptable())
        {
            TriggerPerformanceOptimization();
        }
    }
}

void APerformanceMonitoringSystem::UpdatePerformanceMetrics()
{
    if (FrameCount == 0) return;

    // Calculate average frame time and FPS
    float AvgFrameTime = AccumulatedFrameTime / FrameCount;
    CurrentMetrics.FrameTimeMS = AvgFrameTime * 1000.0f;
    CurrentMetrics.FPS = 1.0f / AvgFrameTime;

    // Add to history
    FrameTimeHistory.Add(CurrentMetrics.FrameTimeMS);
    if (FrameTimeHistory.Num() > MaxHistorySize)
    {
        FrameTimeHistory.RemoveAt(0);
    }

    // Count active actors in the world
    if (UWorld* World = GetWorld())
    {
        CurrentMetrics.ActiveActorCount = 0;
        CurrentMetrics.VisibleActorCount = 0;
        
        for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
        {
            AActor* Actor = *ActorIterator;
            if (Actor && !Actor->IsPendingKill())
            {
                CurrentMetrics.ActiveActorCount++;
                
                // Check if actor is visible (has renderable components)
                if (Actor->GetRootComponent() && !Actor->IsHidden())
                {
                    CurrentMetrics.VisibleActorCount++;
                }
            }
        }
    }

    // Update stats
    SET_DWORD_STAT(STAT_ActiveActors, CurrentMetrics.ActiveActorCount);
    SET_FLOAT_STAT(STAT_FrameTimeMS, CurrentMetrics.FrameTimeMS);

    // Get memory usage (approximate)
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);

    // Estimate draw calls and triangles (these would need more complex implementation)
    // For now, use approximations based on visible actors
    CurrentMetrics.DrawCalls = CurrentMetrics.VisibleActorCount * 2; // Rough estimate
    CurrentMetrics.TriangleCount = CurrentMetrics.VisibleActorCount * 1000; // Very rough estimate

    // Reset accumulation
    AccumulatedFrameTime = 0.0f;
    FrameCount = 0;

    // Log performance if enabled
    if (bEnableLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("Performance: FPS=%.1f, FrameTime=%.2fms, Actors=%d, Memory=%.1fMB"), 
               CurrentMetrics.FPS, CurrentMetrics.FrameTimeMS, 
               CurrentMetrics.ActiveActorCount, CurrentMetrics.MemoryUsageMB);
    }
}

EPerf_PerformanceLevel APerformanceMonitoringSystem::GetCurrentPerformanceLevel() const
{
    float CurrentFPS = CurrentMetrics.FPS;
    
    // Determine target FPS based on platform
    float TargetFPS = PerformanceThresholds.TargetFPS_PC; // Default to PC target
    
    if (CurrentFPS >= TargetFPS * 0.95f)
    {
        return EPerf_PerformanceLevel::Excellent;
    }
    else if (CurrentFPS >= TargetFPS * 0.85f)
    {
        return EPerf_PerformanceLevel::Good;
    }
    else if (CurrentFPS >= TargetFPS * 0.70f)
    {
        return EPerf_PerformanceLevel::Acceptable;
    }
    else if (CurrentFPS >= TargetFPS * 0.50f)
    {
        return EPerf_PerformanceLevel::Poor;
    }
    else
    {
        return EPerf_PerformanceLevel::Critical;
    }
}

float APerformanceMonitoringSystem::GetAverageFrameTime() const
{
    if (FrameTimeHistory.Num() == 0) return 0.0f;
    
    float Sum = 0.0f;
    for (float FrameTime : FrameTimeHistory)
    {
        Sum += FrameTime;
    }
    
    return Sum / FrameTimeHistory.Num();
}

float APerformanceMonitoringSystem::GetAverageFPS() const
{
    float AvgFrameTime = GetAverageFrameTime();
    return AvgFrameTime > 0.0f ? 1000.0f / AvgFrameTime : 0.0f;
}

void APerformanceMonitoringSystem::ResetPerformanceHistory()
{
    FrameTimeHistory.Empty();
    AccumulatedFrameTime = 0.0f;
    FrameCount = 0;
    TimeSinceLastUpdate = 0.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("Performance history reset"));
}

bool APerformanceMonitoringSystem::IsPerformanceAcceptable() const
{
    EPerf_PerformanceLevel Level = GetCurrentPerformanceLevel();
    return Level == EPerf_PerformanceLevel::Excellent || 
           Level == EPerf_PerformanceLevel::Good || 
           Level == EPerf_PerformanceLevel::Acceptable;
}

void APerformanceMonitoringSystem::TriggerPerformanceOptimization()
{
    UE_LOG(LogTemp, Warning, TEXT("Triggering automatic performance optimization"));
    
    EPerf_PerformanceLevel Level = GetCurrentPerformanceLevel();
    
    switch (Level)
    {
        case EPerf_PerformanceLevel::Poor:
            OptimizeLODSettings();
            OptimizeCullingSettings();
            break;
            
        case EPerf_PerformanceLevel::Critical:
            OptimizeLODSettings();
            OptimizeCullingSettings();
            OptimizeShadowSettings();
            OptimizeTextureStreaming();
            break;
            
        default:
            break;
    }
}

void APerformanceMonitoringSystem::LogPerformanceReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PERFORMANCE REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Current FPS: %.1f"), CurrentMetrics.FPS);
    UE_LOG(LogTemp, Warning, TEXT("Frame Time: %.2f ms"), CurrentMetrics.FrameTimeMS);
    UE_LOG(LogTemp, Warning, TEXT("Active Actors: %d"), CurrentMetrics.ActiveActorCount);
    UE_LOG(LogTemp, Warning, TEXT("Visible Actors: %d"), CurrentMetrics.VisibleActorCount);
    UE_LOG(LogTemp, Warning, TEXT("Memory Usage: %.1f MB"), CurrentMetrics.MemoryUsageMB);
    UE_LOG(LogTemp, Warning, TEXT("Performance Level: %s"), 
           *UEnum::GetValueAsString(GetCurrentPerformanceLevel()));
    UE_LOG(LogTemp, Warning, TEXT("Average FPS (history): %.1f"), GetAverageFPS());
    UE_LOG(LogTemp, Warning, TEXT("=== END REPORT ==="));
}

void APerformanceMonitoringSystem::OptimizeLODSettings()
{
    if (UWorld* World = GetWorld())
    {
        UE_LOG(LogTemp, Warning, TEXT("Optimizing LOD settings for better performance"));
        
        // Increase LOD bias to use lower detail models sooner
        if (GEngine)
        {
            GEngine->Exec(World, TEXT("r.ForceLOD 1"));
            GEngine->Exec(World, TEXT("r.StaticMeshLODDistanceScale 0.8"));
            GEngine->Exec(World, TEXT("r.SkeletalMeshLODBias 1"));
        }
    }
}

void APerformanceMonitoringSystem::OptimizeCullingSettings()
{
    if (UWorld* World = GetWorld())
    {
        UE_LOG(LogTemp, Warning, TEXT("Optimizing culling settings for better performance"));
        
        if (GEngine)
        {
            // Reduce view distance
            GEngine->Exec(World, TEXT("r.ViewDistanceScale 0.7"));
            
            // Enable more aggressive occlusion culling
            GEngine->Exec(World, TEXT("r.HZBOcclusion 1"));
            GEngine->Exec(World, TEXT("r.AllowOcclusionQueries 1"));
            
            // Reduce foliage render distance
            GEngine->Exec(World, TEXT("foliage.MaxTrianglesToRender 500000"));
        }
    }
}

void APerformanceMonitoringSystem::OptimizeShadowSettings()
{
    if (UWorld* World = GetWorld())
    {
        UE_LOG(LogTemp, Warning, TEXT("Optimizing shadow settings for better performance"));
        
        if (GEngine)
        {
            // Reduce shadow resolution
            GEngine->Exec(World, TEXT("r.Shadow.MaxResolution 1024"));
            
            // Reduce shadow distance
            GEngine->Exec(World, TEXT("r.Shadow.DistanceScale 0.6"));
            
            // Disable some shadow features
            GEngine->Exec(World, TEXT("r.Shadow.Virtual.Enable 0"));
        }
    }
}

void APerformanceMonitoringSystem::OptimizeTextureStreaming()
{
    if (UWorld* World = GetWorld())
    {
        UE_LOG(LogTemp, Warning, TEXT("Optimizing texture streaming for better performance"));
        
        if (GEngine)
        {
            // Reduce texture pool size
            GEngine->Exec(World, TEXT("r.Streaming.PoolSize 512"));
            
            // Enable more aggressive texture streaming
            GEngine->Exec(World, TEXT("r.TextureStreaming 1"));
            GEngine->Exec(World, TEXT("r.Streaming.MipBias 1"));
        }
    }
}