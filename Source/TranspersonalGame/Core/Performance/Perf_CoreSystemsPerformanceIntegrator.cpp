#include "Perf_CoreSystemsPerformanceIntegrator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"

APerf_CoreSystemsPerformanceIntegrator::APerf_CoreSystemsPerformanceIntegrator()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;

    // Initialize default optimization settings
    OptimizationSettings = FPerf_CoreSystemOptimizationSettings();
    CurrentMetrics = FPerf_CoreSystemMetrics();
    
    MonitoringUpdateInterval = 0.1f;
    bEnableRealTimeOptimization = true;
    bEnableDebugLogging = false;
    
    LastMonitoringUpdate = 0.0f;
    FrameTimeAccumulator = 0.0f;
    FrameCount = 0;
}

void APerf_CoreSystemsPerformanceIntegrator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Core Systems Performance Integrator - BeginPlay"));
    
    // Initialize performance monitoring
    ApplyOptimizationSettings();
    UpdateCoreSystemsMetrics();
}

void APerf_CoreSystemsPerformanceIntegrator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    LastMonitoringUpdate += DeltaTime;
    FrameTimeAccumulator += DeltaTime;
    FrameCount++;
    
    // Update metrics at specified interval
    if (LastMonitoringUpdate >= MonitoringUpdateInterval)
    {
        UpdateCoreSystemsMetrics();
        
        if (bEnableRealTimeOptimization)
        {
            AdaptiveOptimization();
        }
        
        if (bEnableDebugLogging)
        {
            LogPerformanceMetrics();
        }
        
        LastMonitoringUpdate = 0.0f;
    }
}

void APerf_CoreSystemsPerformanceIntegrator::UpdateCoreSystemsMetrics()
{
    TRACE_CPUPROFILER_EVENT_SCOPE(UpdateCoreSystemsMetrics);
    
    // Monitor individual core systems
    MonitorTerrainPhysicsPerformance();
    MonitorPhysicsSystemPerformance();
    MonitorRagdollSystemPerformance();
    MonitorDestructionSystemPerformance();
    
    // Calculate total core systems time
    CurrentMetrics.TotalCoreSystemsTime = 
        CurrentMetrics.TerrainPhysicsTime +
        CurrentMetrics.PhysicsSystemTime +
        CurrentMetrics.RagdollSystemTime +
        CurrentMetrics.DestructionSystemTime;
    
    // Calculate memory usage
    CalculateCoreSystemsMemoryUsage();
}

void APerf_CoreSystemsPerformanceIntegrator::MonitorTerrainPhysicsPerformance()
{
    // Simulate terrain physics performance monitoring
    float StartTime = FPlatformTime::Seconds();
    
    // Count active terrain physics actors
    UWorld* World = GetWorld();
    if (World)
    {
        int32 TerrainActorCount = 0;
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor->GetClass()->GetName().Contains(TEXT("Terrain")))
            {
                TerrainActorCount++;
            }
        }
        
        // Simulate terrain physics calculation time based on actor count
        CurrentMetrics.TerrainPhysicsTime = TerrainActorCount * 0.1f;
    }
    
    float EndTime = FPlatformTime::Seconds();
    float MonitoringTime = (EndTime - StartTime) * 1000.0f; // Convert to ms
    
    if (bEnableDebugLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("Terrain Physics Monitoring Time: %.2f ms"), MonitoringTime);
    }
}

void APerf_CoreSystemsPerformanceIntegrator::MonitorPhysicsSystemPerformance()
{
    // Simulate physics system performance monitoring
    float StartTime = FPlatformTime::Seconds();
    
    UWorld* World = GetWorld();
    if (World)
    {
        // Count physics actors
        CurrentMetrics.ActivePhysicsActors = 0;
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor->GetRootComponent() && Actor->GetRootComponent()->IsSimulatingPhysics())
            {
                CurrentMetrics.ActivePhysicsActors++;
            }
        }
        
        // Simulate physics system time based on active actors
        CurrentMetrics.PhysicsSystemTime = CurrentMetrics.ActivePhysicsActors * 0.05f;
    }
    
    float EndTime = FPlatformTime::Seconds();
    float MonitoringTime = (EndTime - StartTime) * 1000.0f;
    
    if (bEnableDebugLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("Physics System Monitoring Time: %.2f ms, Active Actors: %d"), 
               MonitoringTime, CurrentMetrics.ActivePhysicsActors);
    }
}

void APerf_CoreSystemsPerformanceIntegrator::MonitorRagdollSystemPerformance()
{
    // Simulate ragdoll system performance monitoring
    float StartTime = FPlatformTime::Seconds();
    
    UWorld* World = GetWorld();
    if (World)
    {
        // Count ragdoll actors (simplified simulation)
        CurrentMetrics.ActiveRagdolls = 0;
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor->GetClass()->GetName().Contains(TEXT("Ragdoll")))
            {
                CurrentMetrics.ActiveRagdolls++;
            }
        }
        
        // Simulate ragdoll system time
        CurrentMetrics.RagdollSystemTime = CurrentMetrics.ActiveRagdolls * 0.2f;
    }
    
    float EndTime = FPlatformTime::Seconds();
    float MonitoringTime = (EndTime - StartTime) * 1000.0f;
    
    if (bEnableDebugLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("Ragdoll System Monitoring Time: %.2f ms, Active Ragdolls: %d"), 
               MonitoringTime, CurrentMetrics.ActiveRagdolls);
    }
}

void APerf_CoreSystemsPerformanceIntegrator::MonitorDestructionSystemPerformance()
{
    // Simulate destruction system performance monitoring
    float StartTime = FPlatformTime::Seconds();
    
    UWorld* World = GetWorld();
    if (World)
    {
        // Count destructible actors
        CurrentMetrics.ActiveDestructibles = 0;
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor->GetClass()->GetName().Contains(TEXT("Destructible")))
            {
                CurrentMetrics.ActiveDestructibles++;
            }
        }
        
        // Simulate destruction system time
        CurrentMetrics.DestructionSystemTime = CurrentMetrics.ActiveDestructibles * 0.15f;
    }
    
    float EndTime = FPlatformTime::Seconds();
    float MonitoringTime = (EndTime - StartTime) * 1000.0f;
    
    if (bEnableDebugLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("Destruction System Monitoring Time: %.2f ms, Active Destructibles: %d"), 
               MonitoringTime, CurrentMetrics.ActiveDestructibles);
    }
}

void APerf_CoreSystemsPerformanceIntegrator::CalculateCoreSystemsMemoryUsage()
{
    // Simulate memory usage calculation for core systems
    float BaseMemory = 50.0f; // Base memory usage in MB
    float PhysicsMemory = CurrentMetrics.ActivePhysicsActors * 0.1f;
    float RagdollMemory = CurrentMetrics.ActiveRagdolls * 0.5f;
    float DestructionMemory = CurrentMetrics.ActiveDestructibles * 0.3f;
    
    CurrentMetrics.CoreSystemsMemoryUsage = BaseMemory + PhysicsMemory + RagdollMemory + DestructionMemory;
}

void APerf_CoreSystemsPerformanceIntegrator::ApplyOptimizationSettings()
{
    // Apply optimization settings based on current level
    switch (OptimizationSettings.OptimizationLevel)
    {
        case EPerf_CoreSystemOptimizationLevel::Ultra:
            OptimizationSettings.MaxActivePhysicsActors = 500;
            OptimizationSettings.MaxActiveRagdolls = 100;
            OptimizationSettings.MaxActiveDestructibles = 75;
            break;
            
        case EPerf_CoreSystemOptimizationLevel::High:
            OptimizationSettings.MaxActivePhysicsActors = 300;
            OptimizationSettings.MaxActiveRagdolls = 75;
            OptimizationSettings.MaxActiveDestructibles = 50;
            break;
            
        case EPerf_CoreSystemOptimizationLevel::Medium:
            OptimizationSettings.MaxActivePhysicsActors = 200;
            OptimizationSettings.MaxActiveRagdolls = 50;
            OptimizationSettings.MaxActiveDestructibles = 30;
            break;
            
        case EPerf_CoreSystemOptimizationLevel::Low:
            OptimizationSettings.MaxActivePhysicsActors = 100;
            OptimizationSettings.MaxActiveRagdolls = 25;
            OptimizationSettings.MaxActiveDestructibles = 15;
            break;
            
        case EPerf_CoreSystemOptimizationLevel::Minimal:
            OptimizationSettings.MaxActivePhysicsActors = 50;
            OptimizationSettings.MaxActiveRagdolls = 10;
            OptimizationSettings.MaxActiveDestructibles = 5;
            break;
    }
    
    if (bEnableDebugLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Applied optimization level: %d"), 
               static_cast<int32>(OptimizationSettings.OptimizationLevel));
    }
}

void APerf_CoreSystemsPerformanceIntegrator::OptimizeTerrainPhysics()
{
    if (!OptimizationSettings.bEnableTerrainPhysicsOptimization)
        return;
        
    // Optimize terrain physics based on performance metrics
    if (CurrentMetrics.TerrainPhysicsTime > OptimizationSettings.MaxTerrainPhysicsTime)
    {
        // Implement terrain physics optimization logic
        UE_LOG(LogTemp, Warning, TEXT("Optimizing terrain physics - Time: %.2f ms"), 
               CurrentMetrics.TerrainPhysicsTime);
    }
}

void APerf_CoreSystemsPerformanceIntegrator::OptimizePhysicsSystem()
{
    // Optimize physics system if exceeding limits
    if (CurrentMetrics.ActivePhysicsActors > OptimizationSettings.MaxActivePhysicsActors ||
        CurrentMetrics.PhysicsSystemTime > OptimizationSettings.MaxPhysicsSystemTime)
    {
        UE_LOG(LogTemp, Warning, TEXT("Optimizing physics system - Actors: %d, Time: %.2f ms"), 
               CurrentMetrics.ActivePhysicsActors, CurrentMetrics.PhysicsSystemTime);
    }
}

void APerf_CoreSystemsPerformanceIntegrator::OptimizeRagdollSystem()
{
    if (!OptimizationSettings.bEnableRagdollOptimization)
        return;
        
    // Optimize ragdoll system if needed
    if (CurrentMetrics.ActiveRagdolls > OptimizationSettings.MaxActiveRagdolls ||
        CurrentMetrics.RagdollSystemTime > OptimizationSettings.MaxRagdollSystemTime)
    {
        UE_LOG(LogTemp, Warning, TEXT("Optimizing ragdoll system - Ragdolls: %d, Time: %.2f ms"), 
               CurrentMetrics.ActiveRagdolls, CurrentMetrics.RagdollSystemTime);
    }
}

void APerf_CoreSystemsPerformanceIntegrator::OptimizeDestructionSystem()
{
    if (!OptimizationSettings.bEnableDestructionOptimization)
        return;
        
    // Optimize destruction system if needed
    if (CurrentMetrics.ActiveDestructibles > OptimizationSettings.MaxActiveDestructibles ||
        CurrentMetrics.DestructionSystemTime > OptimizationSettings.MaxDestructionSystemTime)
    {
        UE_LOG(LogTemp, Warning, TEXT("Optimizing destruction system - Destructibles: %d, Time: %.2f ms"), 
               CurrentMetrics.ActiveDestructibles, CurrentMetrics.DestructionSystemTime);
    }
}

void APerf_CoreSystemsPerformanceIntegrator::AdaptiveOptimization()
{
    if (!OptimizationSettings.bEnableAdaptiveOptimization)
        return;
        
    // Determine optimal optimization level based on current performance
    EPerf_CoreSystemOptimizationLevel OptimalLevel = DetermineOptimalLevel();
    
    if (OptimalLevel != OptimizationSettings.OptimizationLevel)
    {
        SetOptimizationLevel(OptimalLevel);
    }
    
    // Apply specific optimizations
    OptimizeTerrainPhysics();
    OptimizePhysicsSystem();
    OptimizeRagdollSystem();
    OptimizeDestructionSystem();
}

void APerf_CoreSystemsPerformanceIntegrator::LogPerformanceMetrics()
{
    UE_LOG(LogTemp, Log, TEXT("=== Core Systems Performance Metrics ==="));
    UE_LOG(LogTemp, Log, TEXT("Terrain Physics Time: %.2f ms"), CurrentMetrics.TerrainPhysicsTime);
    UE_LOG(LogTemp, Log, TEXT("Physics System Time: %.2f ms"), CurrentMetrics.PhysicsSystemTime);
    UE_LOG(LogTemp, Log, TEXT("Ragdoll System Time: %.2f ms"), CurrentMetrics.RagdollSystemTime);
    UE_LOG(LogTemp, Log, TEXT("Destruction System Time: %.2f ms"), CurrentMetrics.DestructionSystemTime);
    UE_LOG(LogTemp, Log, TEXT("Total Core Systems Time: %.2f ms"), CurrentMetrics.TotalCoreSystemsTime);
    UE_LOG(LogTemp, Log, TEXT("Active Physics Actors: %d"), CurrentMetrics.ActivePhysicsActors);
    UE_LOG(LogTemp, Log, TEXT("Active Ragdolls: %d"), CurrentMetrics.ActiveRagdolls);
    UE_LOG(LogTemp, Log, TEXT("Active Destructibles: %d"), CurrentMetrics.ActiveDestructibles);
    UE_LOG(LogTemp, Log, TEXT("Core Systems Memory Usage: %.2f MB"), CurrentMetrics.CoreSystemsMemoryUsage);
    UE_LOG(LogTemp, Log, TEXT("=========================================="));
}

EPerf_CoreSystemOptimizationLevel APerf_CoreSystemsPerformanceIntegrator::DetermineOptimalLevel()
{
    float TotalTime = CurrentMetrics.TotalCoreSystemsTime;
    float MemoryUsage = CurrentMetrics.CoreSystemsMemoryUsage;
    
    // Determine optimal level based on performance metrics
    if (TotalTime < 5.0f && MemoryUsage < 100.0f)
    {
        return EPerf_CoreSystemOptimizationLevel::Ultra;
    }
    else if (TotalTime < 10.0f && MemoryUsage < 200.0f)
    {
        return EPerf_CoreSystemOptimizationLevel::High;
    }
    else if (TotalTime < 15.0f && MemoryUsage < 300.0f)
    {
        return EPerf_CoreSystemOptimizationLevel::Medium;
    }
    else if (TotalTime < 20.0f && MemoryUsage < 400.0f)
    {
        return EPerf_CoreSystemOptimizationLevel::Low;
    }
    else
    {
        return EPerf_CoreSystemOptimizationLevel::Minimal;
    }
}

// Blueprint callable functions implementation
void APerf_CoreSystemsPerformanceIntegrator::SetOptimizationLevel(EPerf_CoreSystemOptimizationLevel NewLevel)
{
    OptimizationSettings.OptimizationLevel = NewLevel;
    ApplyOptimizationSettings();
    
    UE_LOG(LogTemp, Warning, TEXT("Optimization level changed to: %d"), static_cast<int32>(NewLevel));
}

FPerf_CoreSystemMetrics APerf_CoreSystemsPerformanceIntegrator::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

void APerf_CoreSystemsPerformanceIntegrator::ForceOptimizationUpdate()
{
    UpdateCoreSystemsMetrics();
    AdaptiveOptimization();
    
    UE_LOG(LogTemp, Warning, TEXT("Forced optimization update completed"));
}

void APerf_CoreSystemsPerformanceIntegrator::ResetPerformanceMetrics()
{
    CurrentMetrics = FPerf_CoreSystemMetrics();
    FrameTimeAccumulator = 0.0f;
    FrameCount = 0;
    
    UE_LOG(LogTemp, Warning, TEXT("Performance metrics reset"));
}

bool APerf_CoreSystemsPerformanceIntegrator::IsPerformanceWithinTargets() const
{
    bool bWithinTargets = true;
    
    bWithinTargets &= (CurrentMetrics.TerrainPhysicsTime <= OptimizationSettings.MaxTerrainPhysicsTime);
    bWithinTargets &= (CurrentMetrics.PhysicsSystemTime <= OptimizationSettings.MaxPhysicsSystemTime);
    bWithinTargets &= (CurrentMetrics.RagdollSystemTime <= OptimizationSettings.MaxRagdollSystemTime);
    bWithinTargets &= (CurrentMetrics.DestructionSystemTime <= OptimizationSettings.MaxDestructionSystemTime);
    bWithinTargets &= (CurrentMetrics.ActivePhysicsActors <= OptimizationSettings.MaxActivePhysicsActors);
    bWithinTargets &= (CurrentMetrics.ActiveRagdolls <= OptimizationSettings.MaxActiveRagdolls);
    bWithinTargets &= (CurrentMetrics.ActiveDestructibles <= OptimizationSettings.MaxActiveDestructibles);
    
    return bWithinTargets;
}

void APerf_CoreSystemsPerformanceIntegrator::EnableAdaptiveOptimization(bool bEnable)
{
    OptimizationSettings.bEnableAdaptiveOptimization = bEnable;
    bEnableRealTimeOptimization = bEnable;
    
    UE_LOG(LogTemp, Warning, TEXT("Adaptive optimization %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void APerf_CoreSystemsPerformanceIntegrator::RunCoreSystemsPerformanceTest()
{
    UE_LOG(LogTemp, Warning, TEXT("Running Core Systems Performance Test..."));
    
    // Force immediate metrics update
    UpdateCoreSystemsMetrics();
    
    // Log detailed performance report
    LogPerformanceMetrics();
    
    // Check if performance is within targets
    bool bWithinTargets = IsPerformanceWithinTargets();
    UE_LOG(LogTemp, Warning, TEXT("Performance within targets: %s"), bWithinTargets ? TEXT("YES") : TEXT("NO"));
    
    // Apply optimizations if needed
    if (!bWithinTargets)
    {
        AdaptiveOptimization();
        UE_LOG(LogTemp, Warning, TEXT("Applied adaptive optimizations"));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Core Systems Performance Test completed"));
}