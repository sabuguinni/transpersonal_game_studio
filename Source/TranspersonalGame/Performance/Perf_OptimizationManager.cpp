#include "Perf_OptimizationManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "HAL/PlatformFilemanager.h"
#include "TimerManager.h"

void UPerf_OptimizationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    bIsMonitoring = false;
    CurrentOptimizationLevel = EPerf_OptimizationLevel::Medium;
    TargetFrameRate = 60;
    MonitoringInterval = 1.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Performance Optimization Manager Initialized"));
    
    // Apply initial optimization settings
    ApplyOptimizationSettings();
}

void UPerf_OptimizationManager::Deinitialize()
{
    StopPerformanceMonitoring();
    Super::Deinitialize();
}

void UPerf_OptimizationManager::StartPerformanceMonitoring()
{
    if (bIsMonitoring)
    {
        return;
    }
    
    bIsMonitoring = true;
    
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            MonitoringTimerHandle,
            this,
            &UPerf_OptimizationManager::UpdatePerformanceMetrics,
            MonitoringInterval,
            true
        );
    }
    
    UE_LOG(LogTemp, Log, TEXT("Performance monitoring started"));
}

void UPerf_OptimizationManager::StopPerformanceMonitoring()
{
    if (!bIsMonitoring)
    {
        return;
    }
    
    bIsMonitoring = false;
    
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(MonitoringTimerHandle);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Performance monitoring stopped"));
}

FPerf_PerformanceMetrics UPerf_OptimizationManager::GetCurrentMetrics()
{
    if (bIsMonitoring)
    {
        UpdatePerformanceMetrics();
    }
    
    return CachedMetrics;
}

void UPerf_OptimizationManager::SetOptimizationLevel(EPerf_OptimizationLevel Level)
{
    CurrentOptimizationLevel = Level;
    ApplyOptimizationSettings();
    
    UE_LOG(LogTemp, Log, TEXT("Optimization level set to: %d"), (int32)Level);
}

void UPerf_OptimizationManager::OptimizePhysicsSettings()
{
    // Optimize physics settings based on current optimization level
    switch (CurrentOptimizationLevel)
    {
        case EPerf_OptimizationLevel::Low:
            // Minimal physics optimization
            if (auto* ConsoleVar = IConsoleManager::Get().FindConsoleVariable(TEXT("p.MaxSubsteps")))
            {
                ConsoleVar->Set(2);
            }
            break;
            
        case EPerf_OptimizationLevel::Medium:
            // Balanced physics settings
            if (auto* ConsoleVar = IConsoleManager::Get().FindConsoleVariable(TEXT("p.MaxSubsteps")))
            {
                ConsoleVar->Set(4);
            }
            break;
            
        case EPerf_OptimizationLevel::High:
            // Aggressive physics optimization
            if (auto* ConsoleVar = IConsoleManager::Get().FindConsoleVariable(TEXT("p.MaxSubsteps")))
            {
                ConsoleVar->Set(6);
            }
            break;
            
        case EPerf_OptimizationLevel::Ultra:
            // Maximum physics optimization
            if (auto* ConsoleVar = IConsoleManager::Get().FindConsoleVariable(TEXT("p.MaxSubsteps")))
            {
                ConsoleVar->Set(8);
            }
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Physics settings optimized"));
}

void UPerf_OptimizationManager::OptimizeRenderingSettings()
{
    // Optimize rendering settings based on optimization level
    switch (CurrentOptimizationLevel)
    {
        case EPerf_OptimizationLevel::Low:
            // Basic rendering optimizations
            if (auto* ConsoleVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ViewDistanceScale")))
            {
                ConsoleVar->Set(1.0f);
            }
            break;
            
        case EPerf_OptimizationLevel::Medium:
            // Moderate rendering optimizations
            if (auto* ConsoleVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ViewDistanceScale")))
            {
                ConsoleVar->Set(0.8f);
            }
            break;
            
        case EPerf_OptimizationLevel::High:
            // Aggressive rendering optimizations
            if (auto* ConsoleVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ViewDistanceScale")))
            {
                ConsoleVar->Set(0.6f);
            }
            break;
            
        case EPerf_OptimizationLevel::Ultra:
            // Maximum rendering optimizations
            if (auto* ConsoleVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ViewDistanceScale")))
            {
                ConsoleVar->Set(0.4f);
            }
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Rendering settings optimized"));
}

void UPerf_OptimizationManager::EnableLODOptimization(bool bEnable)
{
    if (bEnable)
    {
        // Enable LOD optimizations
        if (auto* ConsoleVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ForceLOD")))
        {
            ConsoleVar->Set(-1); // Auto LOD
        }
    }
    else
    {
        // Disable LOD optimizations
        if (auto* ConsoleVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ForceLOD")))
        {
            ConsoleVar->Set(0); // Force highest LOD
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("LOD optimization %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UPerf_OptimizationManager::SetTargetFPS(int32 TargetFPS)
{
    TargetFrameRate = FMath::Clamp(TargetFPS, 30, 120);
    OptimizeForTargetFPS();
    
    UE_LOG(LogTemp, Log, TEXT("Target FPS set to: %d"), TargetFrameRate);
}

void UPerf_OptimizationManager::UpdatePerformanceMetrics()
{
    if (UWorld* World = GetWorld())
    {
        // Update FPS
        if (GEngine && GEngine->GetGameViewport())
        {
            CachedMetrics.CurrentFPS = 1.0f / World->GetDeltaSeconds();
            CachedMetrics.AverageFrameTime = World->GetDeltaSeconds() * 1000.0f; // Convert to ms
        }
        
        // Count physics actors
        CachedMetrics.PhysicsActorCount = 0;
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor->GetRootComponent())
            {
                if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
                {
                    if (PrimComp->IsSimulatingPhysics())
                    {
                        CachedMetrics.PhysicsActorCount++;
                    }
                }
            }
        }
        
        // Update memory usage (simplified)
        CachedMetrics.MemoryUsageMB = FPlatformMemory::GetStats().UsedPhysical / (1024.0f * 1024.0f);
    }
}

void UPerf_OptimizationManager::ApplyOptimizationSettings()
{
    OptimizePhysicsSettings();
    OptimizeRenderingSettings();
    EnableLODOptimization(true);
    OptimizeForTargetFPS();
}

void UPerf_OptimizationManager::OptimizeForTargetFPS()
{
    // Adjust optimization level based on target FPS
    if (TargetFrameRate >= 60)
    {
        // High performance target
        if (auto* ConsoleVar = IConsoleManager::Get().FindConsoleVariable(TEXT("t.MaxFPS")))
        {
            ConsoleVar->Set(TargetFrameRate);
        }
    }
    else
    {
        // Lower performance target
        if (auto* ConsoleVar = IConsoleManager::Get().FindConsoleVariable(TEXT("t.MaxFPS")))
        {
            ConsoleVar->Set(TargetFrameRate);
        }
    }
}