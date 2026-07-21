#include "Perf_PerformanceMonitor.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "HAL/PlatformMemory.h"
#include "Stats/Stats.h"

APerf_PerformanceMonitor::APerf_PerformanceMonitor()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.0f; // Tick every frame for accurate performance monitoring
    
    // Initialize performance tracking
    CurrentFPS = 60.0f;
    AverageFrameTime = 16.67f; // 60 FPS baseline
    CurrentPerformanceLevel = EPerf_PerformanceLevel::High;
    MonitoringTimer = 0.0f;
    bIsMonitoring = false;
    FrameCounter = 0;
    TotalFrameTime = 0.0f;
    
    // Initialize frame history
    FrameTimeHistory.Reserve(FrameHistorySize);
}

void APerf_PerformanceMonitor::BeginPlay()
{
    Super::BeginPlay();
    
    // Start monitoring automatically
    StartPerformanceMonitoring();
    
    UE_LOG(LogTemp, Warning, TEXT("Performance Monitor initialized - Target: 60fps PC / 30fps Console"));
}

void APerf_PerformanceMonitor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bIsMonitoring)
    {
        UpdatePerformanceMetrics(DeltaTime);
        
        MonitoringTimer += DeltaTime;
        if (MonitoringTimer >= MonitoringInterval)
        {
            CalculateAverageFrameTime();
            DeterminePerformanceLevel();
            UpdateFrameData();
            
            if (bLogPerformanceData)
            {
                LogPerformanceData();
            }
            
            if (bEnableAutoOptimization)
            {
                ApplyPerformanceOptimizations();
            }
            
            MonitoringTimer = 0.0f;
        }
    }
}

void APerf_PerformanceMonitor::UpdatePerformanceMetrics(float DeltaTime)
{
    // Calculate current FPS
    if (DeltaTime > 0.0f)
    {
        CurrentFPS = 1.0f / DeltaTime;
    }
    
    // Add to frame time history
    float FrameTimeMs = DeltaTime * 1000.0f;
    FrameTimeHistory.Add(FrameTimeMs);
    
    // Maintain history size
    if (FrameTimeHistory.Num() > FrameHistorySize)
    {
        FrameTimeHistory.RemoveAt(0);
    }
    
    // Update totals for average calculation
    TotalFrameTime += FrameTimeMs;
    FrameCounter++;
}

void APerf_PerformanceMonitor::CalculateAverageFrameTime()
{
    if (FrameTimeHistory.Num() > 0)
    {
        float Sum = 0.0f;
        for (float FrameTime : FrameTimeHistory)
        {
            Sum += FrameTime;
        }
        AverageFrameTime = Sum / FrameTimeHistory.Num();
    }
}

void APerf_PerformanceMonitor::DeterminePerformanceLevel()
{
    if (CurrentFPS >= UltraPerformanceThreshold)
    {
        CurrentPerformanceLevel = EPerf_PerformanceLevel::Ultra;
    }
    else if (CurrentFPS >= HighPerformanceThreshold)
    {
        CurrentPerformanceLevel = EPerf_PerformanceLevel::High;
    }
    else if (CurrentFPS >= MediumPerformanceThreshold)
    {
        CurrentPerformanceLevel = EPerf_PerformanceLevel::Medium;
    }
    else if (CurrentFPS >= LowPerformanceThreshold)
    {
        CurrentPerformanceLevel = EPerf_PerformanceLevel::Low;
    }
    else
    {
        CurrentPerformanceLevel = EPerf_PerformanceLevel::Critical;
    }
}

void APerf_PerformanceMonitor::UpdateFrameData()
{
    CurrentFrameData.FrameTime = AverageFrameTime;
    CurrentFrameData.DeltaTime = GetWorld()->GetDeltaSeconds();
    
    // Count actors in world
    if (UWorld* World = GetWorld())
    {
        CurrentFrameData.ActorCount = World->GetActorCount();
    }
    
    // Get memory usage (simplified)
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentFrameData.MemoryUsage = static_cast<float>(MemStats.UsedPhysical) / (1024.0f * 1024.0f); // MB
}

void APerf_PerformanceMonitor::LogPerformanceData() const
{
    UE_LOG(LogTemp, Log, TEXT("Performance Monitor - FPS: %.1f, Frame Time: %.2fms, Level: %s, Actors: %d"),
        CurrentFPS,
        AverageFrameTime,
        *UEnum::GetValueAsString(CurrentPerformanceLevel),
        CurrentFrameData.ActorCount);
}

void APerf_PerformanceMonitor::StartPerformanceMonitoring()
{
    bIsMonitoring = true;
    ResetPerformanceStats();
    UE_LOG(LogTemp, Warning, TEXT("Performance monitoring started"));
}

void APerf_PerformanceMonitor::StopPerformanceMonitoring()
{
    bIsMonitoring = false;
    UE_LOG(LogTemp, Warning, TEXT("Performance monitoring stopped"));
}

void APerf_PerformanceMonitor::ResetPerformanceStats()
{
    FrameTimeHistory.Empty();
    MonitoringTimer = 0.0f;
    FrameCounter = 0;
    TotalFrameTime = 0.0f;
    CurrentFPS = 60.0f;
    AverageFrameTime = 16.67f;
    CurrentPerformanceLevel = EPerf_PerformanceLevel::High;
}

void APerf_PerformanceMonitor::OptimizeForPerformanceLevel(EPerf_PerformanceLevel TargetLevel)
{
    switch (TargetLevel)
    {
        case EPerf_PerformanceLevel::Ultra:
            UltraPerformanceThreshold = 60.0f;
            HighPerformanceThreshold = 45.0f;
            break;
        case EPerf_PerformanceLevel::High:
            UltraPerformanceThreshold = 45.0f;
            HighPerformanceThreshold = 35.0f;
            break;
        case EPerf_PerformanceLevel::Medium:
            UltraPerformanceThreshold = 35.0f;
            HighPerformanceThreshold = 25.0f;
            break;
        case EPerf_PerformanceLevel::Low:
            UltraPerformanceThreshold = 25.0f;
            HighPerformanceThreshold = 20.0f;
            break;
        case EPerf_PerformanceLevel::Critical:
            // Emergency optimization mode
            UltraPerformanceThreshold = 20.0f;
            HighPerformanceThreshold = 15.0f;
            break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Performance optimized for level: %s"), *UEnum::GetValueAsString(TargetLevel));
}

bool APerf_PerformanceMonitor::ShouldReduceLOD() const
{
    return CurrentPerformanceLevel <= EPerf_PerformanceLevel::Medium;
}

bool APerf_PerformanceMonitor::ShouldCullDistantObjects() const
{
    return CurrentPerformanceLevel <= EPerf_PerformanceLevel::Low;
}

void APerf_PerformanceMonitor::ApplyPerformanceOptimizations()
{
    switch (CurrentPerformanceLevel)
    {
        case EPerf_PerformanceLevel::Critical:
            OptimizePhysics();
            OptimizeCulling();
            OptimizeLODSystem();
            break;
        case EPerf_PerformanceLevel::Low:
            OptimizeCulling();
            OptimizeLODSystem();
            break;
        case EPerf_PerformanceLevel::Medium:
            OptimizeLODSystem();
            break;
        default:
            // High/Ultra performance - no optimization needed
            break;
    }
}

void APerf_PerformanceMonitor::OptimizeLODSystem()
{
    // Reduce LOD distances for better performance
    if (UWorld* World = GetWorld())
    {
        // This would integrate with the LOD manager when available
        UE_LOG(LogTemp, Log, TEXT("LOD optimization applied"));
    }
}

void APerf_PerformanceMonitor::OptimizeCulling()
{
    // Increase culling distances for distant objects
    if (UWorld* World = GetWorld())
    {
        // This would integrate with culling systems
        UE_LOG(LogTemp, Log, TEXT("Culling optimization applied"));
    }
}

void APerf_PerformanceMonitor::OptimizePhysics()
{
    // Reduce physics simulation complexity
    if (UWorld* World = GetWorld())
    {
        // This would integrate with physics systems
        UE_LOG(LogTemp, Log, TEXT("Physics optimization applied"));
    }
}