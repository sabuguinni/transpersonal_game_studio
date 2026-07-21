#include "Perf_IntegratedPerformanceManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "HAL/PlatformMemory.h"
#include "Stats/Stats.h"

UPerf_IntegratedPerformanceManager::UPerf_IntegratedPerformanceManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Initialize performance data
    PerformanceData = FPerf_IntegratedPerformanceData();
    OptimizationSettings = FPerf_OptimizationSettings();
    
    // Initialize tracking variables
    bIsMonitoring = false;
    MonitoringStartTime = 0.0f;
    LastOptimizationTime = 0.0f;
    MaxFPSSamples = 100;
    
    // Initialize optimization flags
    bLODOptimizationEnabled = true;
    bPhysicsOptimizationEnabled = true;
    bMemoryOptimizationEnabled = true;
    
    FPSSamples.Reserve(MaxFPSSamples);
}

void UPerf_IntegratedPerformanceManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Perf_IntegratedPerformanceManager: BeginPlay - Starting performance monitoring"));
    
    // Start monitoring automatically
    StartPerformanceMonitoring();
    
    // Initialize performance baseline
    ResetPerformanceCounters();
    
    UE_LOG(LogTemp, Log, TEXT("Perf_IntegratedPerformanceManager: Initialized with target FPS: %.1f"), OptimizationSettings.MinFPSThreshold);
}

void UPerf_IntegratedPerformanceManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsMonitoring)
    {
        UpdatePerformanceMetrics(DeltaTime);
        
        if (OptimizationSettings.bEnableAutomaticOptimization)
        {
            CheckOptimizationThresholds();
        }
    }
}

void UPerf_IntegratedPerformanceManager::StartPerformanceMonitoring()
{
    bIsMonitoring = true;
    MonitoringStartTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Log, TEXT("Perf_IntegratedPerformanceManager: Performance monitoring started"));
}

void UPerf_IntegratedPerformanceManager::StopPerformanceMonitoring()
{
    bIsMonitoring = false;
    
    UE_LOG(LogTemp, Log, TEXT("Perf_IntegratedPerformanceManager: Performance monitoring stopped"));
}

FPerf_IntegratedPerformanceData UPerf_IntegratedPerformanceManager::GetCurrentPerformanceData() const
{
    return PerformanceData;
}

void UPerf_IntegratedPerformanceManager::SetOptimizationSettings(const FPerf_OptimizationSettings& NewSettings)
{
    OptimizationSettings = NewSettings;
    
    // Update individual optimization flags
    bLODOptimizationEnabled = NewSettings.bEnableLODOptimization;
    bPhysicsOptimizationEnabled = NewSettings.bEnablePhysicsOptimization;
    bMemoryOptimizationEnabled = NewSettings.bEnableMemoryOptimization;
    
    UE_LOG(LogTemp, Log, TEXT("Perf_IntegratedPerformanceManager: Optimization settings updated - Target FPS: %.1f"), NewSettings.MinFPSThreshold);
}

void UPerf_IntegratedPerformanceManager::ForceOptimizationPass()
{
    if (bIsMonitoring)
    {
        UE_LOG(LogTemp, Log, TEXT("Perf_IntegratedPerformanceManager: Forcing optimization pass"));
        ApplyOptimizations();
    }
}

void UPerf_IntegratedPerformanceManager::ResetPerformanceCounters()
{
    FPSSamples.Empty();
    PerformanceData = FPerf_IntegratedPerformanceData();
    PerformanceData.TargetFPS = OptimizationSettings.MinFPSThreshold;
    
    UE_LOG(LogTemp, Log, TEXT("Perf_IntegratedPerformanceManager: Performance counters reset"));
}

void UPerf_IntegratedPerformanceManager::EnableLODOptimization(bool bEnable)
{
    bLODOptimizationEnabled = bEnable;
    OptimizationSettings.bEnableLODOptimization = bEnable;
    
    UE_LOG(LogTemp, Log, TEXT("Perf_IntegratedPerformanceManager: LOD optimization %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UPerf_IntegratedPerformanceManager::EnablePhysicsOptimization(bool bEnable)
{
    bPhysicsOptimizationEnabled = bEnable;
    OptimizationSettings.bEnablePhysicsOptimization = bEnable;
    
    UE_LOG(LogTemp, Log, TEXT("Perf_IntegratedPerformanceManager: Physics optimization %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UPerf_IntegratedPerformanceManager::EnableMemoryOptimization(bool bEnable)
{
    bMemoryOptimizationEnabled = bEnable;
    OptimizationSettings.bEnableMemoryOptimization = bEnable;
    
    UE_LOG(LogTemp, Log, TEXT("Perf_IntegratedPerformanceManager: Memory optimization %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

float UPerf_IntegratedPerformanceManager::GetAverageFPS() const
{
    if (FPSSamples.Num() == 0)
    {
        return PerformanceData.CurrentFPS;
    }
    
    float Sum = 0.0f;
    for (float FPS : FPSSamples)
    {
        Sum += FPS;
    }
    
    return Sum / FPSSamples.Num();
}

float UPerf_IntegratedPerformanceManager::GetMemoryUsage() const
{
    return PerformanceData.MemoryUsageMB;
}

int32 UPerf_IntegratedPerformanceManager::GetActivePhysicsObjectCount() const
{
    return PerformanceData.PhysicsObjectCount;
}

bool UPerf_IntegratedPerformanceManager::IsPerformanceTargetMet() const
{
    return PerformanceData.CurrentFPS >= OptimizationSettings.MinFPSThreshold &&
           PerformanceData.MemoryUsageMB <= OptimizationSettings.MaxMemoryThresholdMB &&
           PerformanceData.PhysicsObjectCount <= OptimizationSettings.MaxPhysicsObjects;
}

void UPerf_IntegratedPerformanceManager::LogPerformanceReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PERFORMANCE REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Current FPS: %.1f (Target: %.1f)"), PerformanceData.CurrentFPS, PerformanceData.TargetFPS);
    UE_LOG(LogTemp, Warning, TEXT("Average FPS: %.1f"), GetAverageFPS());
    UE_LOG(LogTemp, Warning, TEXT("Memory Usage: %.1f MB (Max: %.1f MB)"), PerformanceData.MemoryUsageMB, OptimizationSettings.MaxMemoryThresholdMB);
    UE_LOG(LogTemp, Warning, TEXT("Active Actors: %d"), PerformanceData.ActiveActorCount);
    UE_LOG(LogTemp, Warning, TEXT("Physics Objects: %d (Max: %d)"), PerformanceData.PhysicsObjectCount, OptimizationSettings.MaxPhysicsObjects);
    UE_LOG(LogTemp, Warning, TEXT("Physics Tick Time: %.2f ms"), PerformanceData.PhysicsTickTime);
    UE_LOG(LogTemp, Warning, TEXT("Render Thread Time: %.2f ms"), PerformanceData.RenderThreadTime);
    UE_LOG(LogTemp, Warning, TEXT("Optimization Active: %s"), PerformanceData.bIsOptimizationActive ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Warning, TEXT("Performance Target Met: %s"), IsPerformanceTargetMet() ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Warning, TEXT("========================"));
}

void UPerf_IntegratedPerformanceManager::RunPerformanceDiagnostic()
{
    UE_LOG(LogTemp, Warning, TEXT("Perf_IntegratedPerformanceManager: Running performance diagnostic..."));
    
    // Force update metrics
    UpdatePerformanceMetrics(0.016f); // Assume 60 FPS delta
    
    // Log detailed report
    LogPerformanceReport();
    
    // Check for performance issues
    if (PerformanceData.CurrentFPS < OptimizationSettings.MinFPSThreshold)
    {
        UE_LOG(LogTemp, Error, TEXT("PERFORMANCE ISSUE: FPS below target (%.1f < %.1f)"), PerformanceData.CurrentFPS, OptimizationSettings.MinFPSThreshold);
    }
    
    if (PerformanceData.MemoryUsageMB > OptimizationSettings.MaxMemoryThresholdMB)
    {
        UE_LOG(LogTemp, Error, TEXT("PERFORMANCE ISSUE: Memory usage above threshold (%.1f > %.1f MB)"), PerformanceData.MemoryUsageMB, OptimizationSettings.MaxMemoryThresholdMB);
    }
    
    if (PerformanceData.PhysicsObjectCount > OptimizationSettings.MaxPhysicsObjects)
    {
        UE_LOG(LogTemp, Error, TEXT("PERFORMANCE ISSUE: Too many physics objects (%d > %d)"), PerformanceData.PhysicsObjectCount, OptimizationSettings.MaxPhysicsObjects);
    }
}

void UPerf_IntegratedPerformanceManager::UpdatePerformanceMetrics(float DeltaTime)
{
    // Update FPS
    PerformanceData.CurrentFPS = CalculateCurrentFPS(DeltaTime);
    
    // Add FPS sample
    FPSSamples.Add(PerformanceData.CurrentFPS);
    if (FPSSamples.Num() > MaxFPSSamples)
    {
        FPSSamples.RemoveAt(0);
    }
    
    // Update memory usage
    PerformanceData.MemoryUsageMB = CalculateMemoryUsage();
    
    // Update actor count
    if (GetWorld())
    {
        PerformanceData.ActiveActorCount = GetWorld()->GetCurrentLevel()->Actors.Num();
    }
    
    // Update physics metrics
    PerformanceData.PhysicsObjectCount = CountActivePhysicsObjects();
    PerformanceData.PhysicsTickTime = GetPhysicsTickTime();
    PerformanceData.RenderThreadTime = GetRenderThreadTime();
}

void UPerf_IntegratedPerformanceManager::CheckOptimizationThresholds()
{
    bool bNeedsOptimization = false;
    
    // Check FPS threshold
    if (PerformanceData.CurrentFPS < OptimizationSettings.MinFPSThreshold)
    {
        bNeedsOptimization = true;
    }
    
    // Check memory threshold
    if (PerformanceData.MemoryUsageMB > OptimizationSettings.MaxMemoryThresholdMB)
    {
        bNeedsOptimization = true;
    }
    
    // Check physics object threshold
    if (PerformanceData.PhysicsObjectCount > OptimizationSettings.MaxPhysicsObjects)
    {
        bNeedsOptimization = true;
    }
    
    // Apply optimizations if needed
    if (bNeedsOptimization)
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - LastOptimizationTime > 1.0f) // Throttle optimizations to once per second
        {
            ApplyOptimizations();
            LastOptimizationTime = CurrentTime;
        }
    }
}

void UPerf_IntegratedPerformanceManager::ApplyOptimizations()
{
    PerformanceData.bIsOptimizationActive = true;
    
    UE_LOG(LogTemp, Warning, TEXT("Perf_IntegratedPerformanceManager: Applying optimizations - FPS: %.1f, Memory: %.1f MB"), 
           PerformanceData.CurrentFPS, PerformanceData.MemoryUsageMB);
    
    if (bLODOptimizationEnabled)
    {
        OptimizeLOD();
    }
    
    if (bPhysicsOptimizationEnabled)
    {
        OptimizePhysics();
    }
    
    if (bMemoryOptimizationEnabled)
    {
        OptimizeMemory();
    }
}

void UPerf_IntegratedPerformanceManager::OptimizeLOD()
{
    // LOD optimization implementation
    UE_LOG(LogTemp, Log, TEXT("Perf_IntegratedPerformanceManager: Applying LOD optimizations"));
    
    // This would typically adjust LOD bias, cull distance, etc.
    // For now, just log the optimization
}

void UPerf_IntegratedPerformanceManager::OptimizePhysics()
{
    // Physics optimization implementation
    UE_LOG(LogTemp, Log, TEXT("Perf_IntegratedPerformanceManager: Applying physics optimizations"));
    
    // This would typically disable physics on distant objects, reduce tick rates, etc.
    // For now, just log the optimization
}

void UPerf_IntegratedPerformanceManager::OptimizeMemory()
{
    // Memory optimization implementation
    UE_LOG(LogTemp, Log, TEXT("Perf_IntegratedPerformanceManager: Applying memory optimizations"));
    
    // This would typically trigger garbage collection, unload unused assets, etc.
    if (GEngine)
    {
        GEngine->ForceGarbageCollection(true);
    }
}

float UPerf_IntegratedPerformanceManager::CalculateCurrentFPS(float DeltaTime)
{
    if (DeltaTime > 0.0f)
    {
        return 1.0f / DeltaTime;
    }
    return 60.0f; // Default fallback
}

float UPerf_IntegratedPerformanceManager::CalculateMemoryUsage()
{
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    return static_cast<float>(MemStats.UsedPhysical) / (1024.0f * 1024.0f); // Convert to MB
}

int32 UPerf_IntegratedPerformanceManager::CountActivePhysicsObjects()
{
    int32 Count = 0;
    
    if (GetWorld())
    {
        for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor->GetRootComponent())
            {
                UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
                if (PrimComp && PrimComp->GetBodyInstance() && PrimComp->GetBodyInstance()->IsValidBodyInstance())
                {
                    Count++;
                }
            }
        }
    }
    
    return Count;
}

float UPerf_IntegratedPerformanceManager::GetPhysicsTickTime()
{
    // This would typically query the physics subsystem for tick time
    // For now, return a placeholder value
    return 8.0f; // ms
}

float UPerf_IntegratedPerformanceManager::GetRenderThreadTime()
{
    // This would typically query the render thread for timing information
    // For now, return a placeholder value
    return 12.0f; // ms
}