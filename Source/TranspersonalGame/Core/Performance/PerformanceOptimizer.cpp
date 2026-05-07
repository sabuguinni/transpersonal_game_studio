#include "PerformanceOptimizer.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"
#include "RenderingThread.h"
#include "RHI.h"
#include "Engine/GameViewportClient.h"

DEFINE_LOG_CATEGORY_STATIC(LogPerformanceOptimizer, Log, All);

UPerformanceOptimizer::UPerformanceOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Initialize performance targets
    TargetFrameRate = 60.0f;
    MinimumFrameRate = 30.0f;
    MaxMemoryUsageMB = 8192.0f; // 8GB
    
    // Initialize monitoring variables
    CurrentFrameRate = 0.0f;
    CurrentMemoryUsageMB = 0.0f;
    FrameTimeHistory.Reserve(100);
    
    // Performance thresholds
    LowPerformanceThreshold = 0.7f; // 70% of target
    CriticalPerformanceThreshold = 0.5f; // 50% of target
    
    bIsOptimizationActive = false;
    bAutoOptimization = true;
    
    LastOptimizationTime = 0.0f;
    OptimizationCooldown = 5.0f; // 5 seconds between optimizations
}

void UPerformanceOptimizer::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("Performance Optimizer initialized"));
    
    // Start performance monitoring
    StartPerformanceMonitoring();
}

void UPerformanceOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdatePerformanceMetrics(DeltaTime);
    
    if (bAutoOptimization)
    {
        CheckPerformanceThresholds();
    }
}

void UPerformanceOptimizer::StartPerformanceMonitoring()
{
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("Starting performance monitoring"));
    
    // Reset metrics
    FrameTimeHistory.Empty();
    CurrentFrameRate = 0.0f;
    CurrentMemoryUsageMB = 0.0f;
    
    // Start monitoring timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            MonitoringTimerHandle,
            this,
            &UPerformanceOptimizer::CollectPerformanceData,
            0.1f, // Collect data every 100ms
            true
        );
    }
}

void UPerformanceOptimizer::StopPerformanceMonitoring()
{
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("Stopping performance monitoring"));
    
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(MonitoringTimerHandle);
    }
}

void UPerformanceOptimizer::UpdatePerformanceMetrics(float DeltaTime)
{
    // Update frame rate
    if (DeltaTime > 0.0f)
    {
        float InstantFPS = 1.0f / DeltaTime;
        
        // Add to history
        FrameTimeHistory.Add(DeltaTime);
        if (FrameTimeHistory.Num() > 100)
        {
            FrameTimeHistory.RemoveAt(0);
        }
        
        // Calculate average FPS over last 100 frames
        if (FrameTimeHistory.Num() > 0)
        {
            float AverageFrameTime = 0.0f;
            for (float FrameTime : FrameTimeHistory)
            {
                AverageFrameTime += FrameTime;
            }
            AverageFrameTime /= FrameTimeHistory.Num();
            CurrentFrameRate = 1.0f / AverageFrameTime;
        }
    }
    
    // Update memory usage
    UpdateMemoryUsage();
}

void UPerformanceOptimizer::UpdateMemoryUsage()
{
    FPlatformMemoryStats MemoryStats = FPlatformMemory::GetStats();
    CurrentMemoryUsageMB = MemoryStats.UsedPhysical / (1024.0f * 1024.0f);
}

void UPerformanceOptimizer::CollectPerformanceData()
{
    // Collect additional performance data
    if (GEngine && GEngine->GetGameViewport())
    {
        // Get render thread stats
        ENQUEUE_RENDER_COMMAND(GetRenderStats)(
            [this](FRHICommandListImmediate& RHICmdList)
            {
                // Collect GPU stats here if needed
            });
    }
}

void UPerformanceOptimizer::CheckPerformanceThresholds()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Don't optimize too frequently
    if (CurrentTime - LastOptimizationTime < OptimizationCooldown)
    {
        return;
    }
    
    // Check frame rate performance
    float PerformanceRatio = CurrentFrameRate / TargetFrameRate;
    
    if (PerformanceRatio < CriticalPerformanceThreshold)
    {
        UE_LOG(LogPerformanceOptimizer, Warning, TEXT("Critical performance detected: %.1f FPS (%.1f%% of target)"), 
               CurrentFrameRate, PerformanceRatio * 100.0f);
        ApplyCriticalOptimizations();
        LastOptimizationTime = CurrentTime;
    }
    else if (PerformanceRatio < LowPerformanceThreshold)
    {
        UE_LOG(LogPerformanceOptimizer, Warning, TEXT("Low performance detected: %.1f FPS (%.1f%% of target)"), 
               CurrentFrameRate, PerformanceRatio * 100.0f);
        ApplyStandardOptimizations();
        LastOptimizationTime = CurrentTime;
    }
    
    // Check memory usage
    if (CurrentMemoryUsageMB > MaxMemoryUsageMB)
    {
        UE_LOG(LogPerformanceOptimizer, Warning, TEXT("High memory usage detected: %.1f MB"), CurrentMemoryUsageMB);
        ApplyMemoryOptimizations();
        LastOptimizationTime = CurrentTime;
    }
}

void UPerformanceOptimizer::ApplyStandardOptimizations()
{
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("Applying standard optimizations"));
    
    bIsOptimizationActive = true;
    
    // Reduce view distance for non-essential objects
    if (UWorld* World = GetWorld())
    {
        // Reduce foliage density
        if (GEngine)
        {
            GEngine->Exec(World, TEXT("foliage.DensityScale 0.8"));
        }
        
        // Reduce particle density
        if (GEngine)
        {
            GEngine->Exec(World, TEXT("fx.MaxGPUParticlesSpawnedPerFrame 512"));
        }
    }
    
    // Broadcast optimization event
    OnOptimizationApplied.Broadcast(EPerformanceOptimizationLevel::Standard);
}

void UPerformanceOptimizer::ApplyCriticalOptimizations()
{
    UE_LOG(LogPerformanceOptimizer, Warning, TEXT("Applying critical optimizations"));
    
    bIsOptimizationActive = true;
    
    // Apply more aggressive optimizations
    if (UWorld* World = GetWorld())
    {
        // Significantly reduce foliage
        if (GEngine)
        {
            GEngine->Exec(World, TEXT("foliage.DensityScale 0.5"));
        }
        
        // Reduce shadow quality
        if (GEngine)
        {
            GEngine->Exec(World, TEXT("r.ShadowQuality 2"));
        }
        
        // Reduce particle effects
        if (GEngine)
        {
            GEngine->Exec(World, TEXT("fx.MaxGPUParticlesSpawnedPerFrame 256"));
        }
        
        // Force garbage collection
        GEngine->ForceGarbageCollection(true);
    }
    
    // Broadcast optimization event
    OnOptimizationApplied.Broadcast(EPerformanceOptimizationLevel::Critical);
}

void UPerformanceOptimizer::ApplyMemoryOptimizations()
{
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("Applying memory optimizations"));
    
    if (UWorld* World = GetWorld())
    {
        // Force garbage collection
        if (GEngine)
        {
            GEngine->ForceGarbageCollection(true);
        }
        
        // Reduce texture streaming pool
        if (GEngine)
        {
            GEngine->Exec(World, TEXT("r.Streaming.PoolSize 512"));
        }
    }
    
    // Broadcast optimization event
    OnOptimizationApplied.Broadcast(EPerformanceOptimizationLevel::Memory);
}

void UPerformanceOptimizer::ResetOptimizations()
{
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("Resetting optimizations to default"));
    
    bIsOptimizationActive = false;
    
    if (UWorld* World = GetWorld())
    {
        // Reset to default values
        if (GEngine)
        {
            GEngine->Exec(World, TEXT("foliage.DensityScale 1.0"));
            GEngine->Exec(World, TEXT("r.ShadowQuality 3"));
            GEngine->Exec(World, TEXT("fx.MaxGPUParticlesSpawnedPerFrame 1024"));
            GEngine->Exec(World, TEXT("r.Streaming.PoolSize 1024"));
        }
    }
    
    // Broadcast reset event
    OnOptimizationReset.Broadcast();
}

float UPerformanceOptimizer::GetCurrentFrameRate() const
{
    return CurrentFrameRate;
}

float UPerformanceOptimizer::GetCurrentMemoryUsage() const
{
    return CurrentMemoryUsageMB;
}

float UPerformanceOptimizer::GetPerformanceRatio() const
{
    return CurrentFrameRate / TargetFrameRate;
}

bool UPerformanceOptimizer::IsPerformanceGood() const
{
    return GetPerformanceRatio() >= LowPerformanceThreshold;
}

void UPerformanceOptimizer::SetTargetFrameRate(float NewTargetFPS)
{
    TargetFrameRate = FMath::Clamp(NewTargetFPS, 15.0f, 120.0f);
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("Target frame rate set to %.1f FPS"), TargetFrameRate);
}

void UPerformanceOptimizer::SetAutoOptimization(bool bEnabled)
{
    bAutoOptimization = bEnabled;
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("Auto optimization %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
    
    if (!bEnabled && bIsOptimizationActive)
    {
        ResetOptimizations();
    }
}

FPerformanceStats UPerformanceOptimizer::GetPerformanceStats() const
{
    FPerformanceStats Stats;
    Stats.CurrentFPS = CurrentFrameRate;
    Stats.TargetFPS = TargetFrameRate;
    Stats.MemoryUsageMB = CurrentMemoryUsageMB;
    Stats.MaxMemoryMB = MaxMemoryUsageMB;
    Stats.PerformanceRatio = GetPerformanceRatio();
    Stats.bIsOptimizationActive = bIsOptimizationActive;
    return Stats;
}