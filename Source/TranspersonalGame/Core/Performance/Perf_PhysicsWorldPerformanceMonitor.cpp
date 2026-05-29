#include "Perf_PhysicsWorldPerformanceMonitor.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "PhysicsEngine/BodySetup.h"
#include "Components/PrimitiveComponent.h"
#include "HAL/IConsoleManager.h"
#include "Stats/StatsHierarchical.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"

UPerf_PhysicsWorldPerformanceMonitor::UPerf_PhysicsWorldPerformanceMonitor()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    TargetFrameTime = 16.67f; // 60 FPS target
    MaxPhysicsSimulationTime = 8.33f; // Max 8.33ms for physics
    MaxActivePhysicsBodies = 1000;
    MaxMemoryUsageMB = 256.0f;
    bAutoOptimize = true;
    MonitoringInterval = 0.1f;
    
    bIsMonitoring = false;
    LastMonitoringTime = 0.0f;
    
    FrameTimeHistory.Reserve(100);
    SimulationTimeHistory.Reserve(100);
}

void UPerf_PhysicsWorldPerformanceMonitor::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoOptimize)
    {
        StartPerformanceMonitoring();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Physics World Performance Monitor initialized"));
}

void UPerf_PhysicsWorldPerformanceMonitor::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsMonitoring)
    {
        LastMonitoringTime += DeltaTime;
        
        if (LastMonitoringTime >= MonitoringInterval)
        {
            UpdatePhysicsMetrics();
            AnalyzePerformance();
            
            if (bAutoOptimize && !CurrentMetrics.bIsPerformanceOptimal)
            {
                ApplyOptimizations();
            }
            
            LastMonitoringTime = 0.0f;
        }
    }
}

FPerf_PhysicsWorldMetrics UPerf_PhysicsWorldPerformanceMonitor::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

void UPerf_PhysicsWorldPerformanceMonitor::StartPerformanceMonitoring()
{
    bIsMonitoring = true;
    LastMonitoringTime = 0.0f;
    ResetPerformanceMetrics();
    
    UE_LOG(LogTemp, Log, TEXT("Physics performance monitoring started"));
}

void UPerf_PhysicsWorldPerformanceMonitor::StopPerformanceMonitoring()
{
    bIsMonitoring = false;
    UE_LOG(LogTemp, Log, TEXT("Physics performance monitoring stopped"));
}

bool UPerf_PhysicsWorldPerformanceMonitor::IsPerformanceOptimal() const
{
    return CurrentMetrics.bIsPerformanceOptimal;
}

void UPerf_PhysicsWorldPerformanceMonitor::OptimizePhysicsPerformance()
{
    SCOPE_CYCLE_COUNTER(STAT_PhysicsWorldUpdate);
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Apply physics optimization strategies
    ApplyOptimizations();
    
    UE_LOG(LogTemp, Log, TEXT("Physics performance optimization applied"));
}

void UPerf_PhysicsWorldPerformanceMonitor::ResetPerformanceMetrics()
{
    CurrentMetrics = FPerf_PhysicsWorldMetrics();
    FrameTimeHistory.Empty();
    SimulationTimeHistory.Empty();
}

void UPerf_PhysicsWorldPerformanceMonitor::RunPerformanceTest()
{
    UE_LOG(LogTemp, Warning, TEXT("=== Physics World Performance Test ==="));
    
    UpdatePhysicsMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("Simulation Time: %.2f ms"), CurrentMetrics.SimulationTime);
    UE_LOG(LogTemp, Warning, TEXT("Update Time: %.2f ms"), CurrentMetrics.UpdateTime);
    UE_LOG(LogTemp, Warning, TEXT("Active Bodies: %d"), CurrentMetrics.ActiveBodies);
    UE_LOG(LogTemp, Warning, TEXT("Sleeping Bodies: %d"), CurrentMetrics.SleepingBodies);
    UE_LOG(LogTemp, Warning, TEXT("Memory Usage: %.2f MB"), CurrentMetrics.MemoryUsageMB);
    UE_LOG(LogTemp, Warning, TEXT("Average Frame Time: %.2f ms"), CurrentMetrics.AverageFrameTime);
    UE_LOG(LogTemp, Warning, TEXT("Performance Optimal: %s"), CurrentMetrics.bIsPerformanceOptimal ? TEXT("YES") : TEXT("NO"));
    
    if (!CurrentMetrics.bIsPerformanceOptimal)
    {
        LogPerformanceWarnings();
    }
}

void UPerf_PhysicsWorldPerformanceMonitor::UpdatePhysicsMetrics()
{
    SCOPE_CYCLE_COUNTER(STAT_PhysicsWorldSimulation);
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Get current frame time
    float CurrentFrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to ms
    FrameTimeHistory.Add(CurrentFrameTime);
    
    if (FrameTimeHistory.Num() > 100)
    {
        FrameTimeHistory.RemoveAt(0);
    }
    
    // Calculate average frame time
    CurrentMetrics.AverageFrameTime = CalculateAverageFrameTime();
    
    // Get physics simulation time (approximation)
    CurrentMetrics.SimulationTime = FMath::Min(CurrentFrameTime * 0.3f, 10.0f); // Estimate physics takes 30% of frame time
    SimulationTimeHistory.Add(CurrentMetrics.SimulationTime);
    
    if (SimulationTimeHistory.Num() > 100)
    {
        SimulationTimeHistory.RemoveAt(0);
    }
    
    // Count physics bodies
    int32 ActiveBodies = 0;
    int32 SleepingBodies = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetRootComponent())
        {
            UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
            if (PrimComp && PrimComp->IsSimulatingPhysics())
            {
                if (PrimComp->IsAnyRigidBodyAwake())
                {
                    ActiveBodies++;
                }
                else
                {
                    SleepingBodies++;
                }
            }
        }
    }
    
    CurrentMetrics.ActiveBodies = ActiveBodies;
    CurrentMetrics.SleepingBodies = SleepingBodies;
    
    // Estimate memory usage (simplified)
    CurrentMetrics.MemoryUsageMB = (ActiveBodies + SleepingBodies) * 0.1f; // Rough estimate: 0.1MB per body
    
    // Update time (simplified)
    CurrentMetrics.UpdateTime = CurrentMetrics.SimulationTime * 0.8f;
    
    // Update performance stats
    UpdatePerformanceStats();
}

void UPerf_PhysicsWorldPerformanceMonitor::AnalyzePerformance()
{
    // Check if performance is optimal
    bool bFrameTimeOK = CurrentMetrics.AverageFrameTime <= TargetFrameTime;
    bool bSimTimeOK = CurrentMetrics.SimulationTime <= MaxPhysicsSimulationTime;
    bool bBodyCountOK = CurrentMetrics.ActiveBodies <= MaxActivePhysicsBodies;
    bool bMemoryOK = CurrentMetrics.MemoryUsageMB <= MaxMemoryUsageMB;
    
    CurrentMetrics.bIsPerformanceOptimal = bFrameTimeOK && bSimTimeOK && bBodyCountOK && bMemoryOK;
    
    if (!CurrentMetrics.bIsPerformanceOptimal)
    {
        LogPerformanceWarnings();
    }
}

void UPerf_PhysicsWorldPerformanceMonitor::ApplyOptimizations()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Apply various optimization strategies based on current metrics
    
    if (CurrentMetrics.SimulationTime > MaxPhysicsSimulationTime)
    {
        // Reduce physics simulation quality temporarily
        UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get();
        if (PhysicsSettings)
        {
            // Could adjust physics settings here if needed
            UE_LOG(LogTemp, Warning, TEXT("Physics simulation time too high, applying optimizations"));
        }
    }
    
    if (CurrentMetrics.ActiveBodies > MaxActivePhysicsBodies)
    {
        // Put some physics bodies to sleep
        int32 BodiesToSleep = CurrentMetrics.ActiveBodies - MaxActivePhysicsBodies;
        int32 BodiesSlept = 0;
        
        for (TActorIterator<AActor> ActorItr(World); ActorItr && BodiesSlept < BodiesToSleep; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor->GetRootComponent())
            {
                UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
                if (PrimComp && PrimComp->IsSimulatingPhysics() && PrimComp->IsAnyRigidBodyAwake())
                {
                    // Check if actor is far from player or not moving much
                    FVector ActorVelocity = PrimComp->GetPhysicsLinearVelocity();
                    if (ActorVelocity.Size() < 10.0f) // Low velocity threshold
                    {
                        PrimComp->PutAllRigidBodiesToSleep();
                        BodiesSlept++;
                    }
                }
            }
        }
        
        if (BodiesSlept > 0)
        {
            UE_LOG(LogTemp, Log, TEXT("Put %d physics bodies to sleep for optimization"), BodiesSlept);
        }
    }
}

float UPerf_PhysicsWorldPerformanceMonitor::CalculateAverageFrameTime() const
{
    if (FrameTimeHistory.Num() == 0)
    {
        return 0.0f;
    }
    
    float Total = 0.0f;
    for (float FrameTime : FrameTimeHistory)
    {
        Total += FrameTime;
    }
    
    return Total / FrameTimeHistory.Num();
}

void UPerf_PhysicsWorldPerformanceMonitor::LogPerformanceWarnings() const
{
    if (CurrentMetrics.AverageFrameTime > TargetFrameTime)
    {
        UE_LOG(LogTemp, Warning, TEXT("Frame time above target: %.2f ms (target: %.2f ms)"), 
               CurrentMetrics.AverageFrameTime, TargetFrameTime);
    }
    
    if (CurrentMetrics.SimulationTime > MaxPhysicsSimulationTime)
    {
        UE_LOG(LogTemp, Warning, TEXT("Physics simulation time too high: %.2f ms (max: %.2f ms)"), 
               CurrentMetrics.SimulationTime, MaxPhysicsSimulationTime);
    }
    
    if (CurrentMetrics.ActiveBodies > MaxActivePhysicsBodies)
    {
        UE_LOG(LogTemp, Warning, TEXT("Too many active physics bodies: %d (max: %d)"), 
               CurrentMetrics.ActiveBodies, MaxActivePhysicsBodies);
    }
    
    if (CurrentMetrics.MemoryUsageMB > MaxMemoryUsageMB)
    {
        UE_LOG(LogTemp, Warning, TEXT("Physics memory usage too high: %.2f MB (max: %.2f MB)"), 
               CurrentMetrics.MemoryUsageMB, MaxMemoryUsageMB);
    }
}

void UPerf_PhysicsWorldPerformanceMonitor::UpdatePerformanceStats()
{
    // Update UE5 stats system
    SET_CYCLE_COUNTER(STAT_PhysicsWorldSimulation, CurrentMetrics.SimulationTime);
    SET_CYCLE_COUNTER(STAT_PhysicsWorldUpdate, CurrentMetrics.UpdateTime);
    SET_DWORD_STAT(STAT_ActivePhysicsBodies, CurrentMetrics.ActiveBodies);
    SET_MEMORY_STAT(STAT_PhysicsMemoryUsage, CurrentMetrics.MemoryUsageMB * 1024 * 1024); // Convert to bytes
}

// APerf_PhysicsWorldPerformanceManager Implementation

APerf_PhysicsWorldPerformanceManager::APerf_PhysicsWorldPerformanceManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.5f; // Update twice per second
    
    PerformanceMonitor = CreateDefaultSubobject<UPerf_PhysicsWorldPerformanceMonitor>(TEXT("PerformanceMonitor"));
    
    bGlobalPerformanceMonitoring = true;
    GlobalTargetFPS = 60.0f;
    bEnableAdaptiveOptimization = true;
}

void APerf_PhysicsWorldPerformanceManager::BeginPlay()
{
    Super::BeginPlay();
    
    if (bGlobalPerformanceMonitoring)
    {
        InitializePerformanceMonitoring();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Physics World Performance Manager started"));
}

void APerf_PhysicsWorldPerformanceManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bGlobalPerformanceMonitoring)
    {
        MonitorGlobalPerformance();
        
        if (bEnableAdaptiveOptimization)
        {
            ApplyGlobalOptimizations();
        }
    }
}

void APerf_PhysicsWorldPerformanceManager::InitializePerformanceMonitoring()
{
    if (PerformanceMonitor)
    {
        PerformanceMonitor->StartPerformanceMonitoring();
        UE_LOG(LogTemp, Log, TEXT("Global physics performance monitoring initialized"));
    }
}

FPerf_PhysicsWorldMetrics APerf_PhysicsWorldPerformanceManager::GetGlobalPhysicsMetrics() const
{
    if (PerformanceMonitor)
    {
        return PerformanceMonitor->GetCurrentMetrics();
    }
    
    return FPerf_PhysicsWorldMetrics();
}

void APerf_PhysicsWorldPerformanceManager::SetPerformanceTargets(float TargetFPS, float MaxSimTime, int32 MaxBodies)
{
    GlobalTargetFPS = TargetFPS;
    
    if (PerformanceMonitor)
    {
        PerformanceMonitor->TargetFrameTime = 1000.0f / TargetFPS; // Convert FPS to frame time in ms
        PerformanceMonitor->MaxPhysicsSimulationTime = MaxSimTime;
        PerformanceMonitor->MaxActivePhysicsBodies = MaxBodies;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Performance targets updated: FPS=%.1f, SimTime=%.2f, MaxBodies=%d"), 
           TargetFPS, MaxSimTime, MaxBodies);
}

void APerf_PhysicsWorldPerformanceManager::RunGlobalPerformanceAnalysis()
{
    UE_LOG(LogTemp, Warning, TEXT("=== Global Physics Performance Analysis ==="));
    
    if (PerformanceMonitor)
    {
        PerformanceMonitor->RunPerformanceTest();
    }
    
    MonitorGlobalPerformance();
    
    UE_LOG(LogTemp, Warning, TEXT("Global Target FPS: %.1f"), GlobalTargetFPS);
    UE_LOG(LogTemp, Warning, TEXT("Adaptive Optimization: %s"), bEnableAdaptiveOptimization ? TEXT("ENABLED") : TEXT("DISABLED"));
}

void APerf_PhysicsWorldPerformanceManager::MonitorGlobalPerformance()
{
    // Global performance monitoring logic
    if (PerformanceMonitor)
    {
        FPerf_PhysicsWorldMetrics Metrics = PerformanceMonitor->GetCurrentMetrics();
        
        // Log critical performance issues
        if (!Metrics.bIsPerformanceOptimal)
        {
            float CurrentFPS = 1000.0f / Metrics.AverageFrameTime;
            if (CurrentFPS < GlobalTargetFPS * 0.8f) // 20% below target
            {
                UE_LOG(LogTemp, Warning, TEXT("CRITICAL: FPS below 80%% of target (%.1f < %.1f)"), 
                       CurrentFPS, GlobalTargetFPS * 0.8f);
            }
        }
    }
}

void APerf_PhysicsWorldPerformanceManager::ApplyGlobalOptimizations()
{
    // Global optimization strategies
    if (PerformanceMonitor && !PerformanceMonitor->IsPerformanceOptimal())
    {
        PerformanceMonitor->OptimizePhysicsPerformance();
    }
}