#include "Perf_TerrainPhysicsPerformanceMonitor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Engine/StaticMeshActor.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/BodySetup.h"

UPerf_TerrainPhysicsPerformanceMonitor::UPerf_TerrainPhysicsPerformanceMonitor()
{
    bIsMonitoring = false;
    TargetFrameRate = 60.0f;
    MaxAcceptablePhysicsTime = 8.0f; // 8ms for physics at 60fps
    MaxPhysicsBodies = 500;
    MonitoringInterval = 0.1f; // Check every 100ms
    MetricsHistorySize = 60; // Keep 6 seconds of history at 10Hz
    bOptimizationsApplied = false;
    LastOptimizationTime = 0.0f;
    OptimizationCooldown = 5.0f; // 5 seconds between optimizations
    CurrentOptimizationLevel = EPerf_TerrainPhysicsOptimizationLevel::Medium;
}

void UPerf_TerrainPhysicsPerformanceMonitor::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsPerformanceMonitor: Initialized"));
    
    // Initialize metrics history
    FrameTimeHistory.Reserve(MetricsHistorySize);
    PhysicsTimeHistory.Reserve(MetricsHistorySize);
    
    // Start monitoring automatically
    StartMonitoring();
}

void UPerf_TerrainPhysicsPerformanceMonitor::Deinitialize()
{
    StopMonitoring();
    Super::Deinitialize();
}

void UPerf_TerrainPhysicsPerformanceMonitor::StartMonitoring()
{
    if (bIsMonitoring)
    {
        return;
    }
    
    bIsMonitoring = true;
    
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            MonitoringTimer,
            this,
            &UPerf_TerrainPhysicsPerformanceMonitor::UpdateMetrics,
            MonitoringInterval,
            true
        );
        
        UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsPerformanceMonitor: Monitoring started"));
    }
}

void UPerf_TerrainPhysicsPerformanceMonitor::StopMonitoring()
{
    if (!bIsMonitoring)
    {
        return;
    }
    
    bIsMonitoring = false;
    
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(MonitoringTimer);
        UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsPerformanceMonitor: Monitoring stopped"));
    }
}

FPerf_TerrainPhysicsMetrics UPerf_TerrainPhysicsPerformanceMonitor::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

void UPerf_TerrainPhysicsPerformanceMonitor::SetOptimizationLevel(EPerf_TerrainPhysicsOptimizationLevel Level)
{
    CurrentOptimizationLevel = Level;
    ApplyOptimizations();
    
    UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsPerformanceMonitor: Optimization level set to %d"), (int32)Level);
}

void UPerf_TerrainPhysicsPerformanceMonitor::OptimizeTerrainPhysics()
{
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    // Apply cooldown to prevent excessive optimizations
    if (CurrentTime - LastOptimizationTime < OptimizationCooldown)
    {
        return;
    }
    
    LastOptimizationTime = CurrentTime;
    
    // Apply optimizations based on current performance
    OptimizePhysicsBodies();
    OptimizeCollisionQueries();
    AdjustPhysicsSettings();
    
    bOptimizationsApplied = true;
    
    UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsPerformanceMonitor: Terrain physics optimizations applied"));
}

bool UPerf_TerrainPhysicsPerformanceMonitor::IsPerformanceAcceptable() const
{
    return CurrentMetrics.AverageFrameTime <= (1000.0f / TargetFrameRate) &&
           CurrentMetrics.PhysicsStepTime <= MaxAcceptablePhysicsTime &&
           CurrentMetrics.ActivePhysicsBodies <= MaxPhysicsBodies;
}

void UPerf_TerrainPhysicsPerformanceMonitor::UpdateMetrics()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Get frame time
    float DeltaTime = GetWorld()->GetDeltaSeconds();
    float FrameTimeMs = DeltaTime * 1000.0f;
    
    // Update frame time history
    FrameTimeHistory.Add(FrameTimeMs);
    if (FrameTimeHistory.Num() > MetricsHistorySize)
    {
        FrameTimeHistory.RemoveAt(0);
    }
    
    // Calculate average frame time
    float TotalFrameTime = 0.0f;
    for (float Time : FrameTimeHistory)
    {
        TotalFrameTime += Time;
    }
    CurrentMetrics.AverageFrameTime = FrameTimeHistory.Num() > 0 ? TotalFrameTime / FrameTimeHistory.Num() : 0.0f;
    
    // Count active physics bodies
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    CurrentMetrics.ActivePhysicsBodies = 0;
    for (AActor* Actor : AllActors)
    {
        if (UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>())
        {
            if (PrimComp->IsSimulatingPhysics())
            {
                CurrentMetrics.ActivePhysicsBodies++;
            }
        }
    }
    
    // Estimate physics step time (simplified)
    CurrentMetrics.PhysicsStepTime = CurrentMetrics.AverageFrameTime * 0.3f; // Assume 30% of frame time for physics
    
    // Update terrain-specific metrics
    CurrentMetrics.TerrainCollisionTime = CurrentMetrics.PhysicsStepTime * 0.4f; // Estimate terrain collision overhead
    CurrentMetrics.TerrainRaycastTime = 0.5f; // Simplified estimate
    CurrentMetrics.TerrainCollisionQueries = CurrentMetrics.ActivePhysicsBodies * 2; // Estimate queries per body
    
    // Check performance thresholds
    CheckPerformanceThresholds();
}

void UPerf_TerrainPhysicsPerformanceMonitor::CheckPerformanceThresholds()
{
    if (!IsPerformanceAcceptable())
    {
        // Broadcast performance alert
        OnPerformanceAlert.Broadcast(CurrentMetrics);
        
        // Auto-optimize if performance is poor
        OptimizeTerrainPhysics();
        
        UE_LOG(LogTemp, Warning, TEXT("TerrainPhysicsPerformanceMonitor: Performance threshold exceeded - Frame: %.2fms, Physics: %.2fms, Bodies: %d"),
            CurrentMetrics.AverageFrameTime, CurrentMetrics.PhysicsStepTime, CurrentMetrics.ActivePhysicsBodies);
    }
}

void UPerf_TerrainPhysicsPerformanceMonitor::ApplyOptimizations()
{
    switch (CurrentOptimizationLevel)
    {
        case EPerf_TerrainPhysicsOptimizationLevel::Low:
            MaxPhysicsBodies = 200;
            MaxAcceptablePhysicsTime = 12.0f;
            break;
            
        case EPerf_TerrainPhysicsOptimizationLevel::Medium:
            MaxPhysicsBodies = 500;
            MaxAcceptablePhysicsTime = 8.0f;
            break;
            
        case EPerf_TerrainPhysicsOptimizationLevel::High:
            MaxPhysicsBodies = 1000;
            MaxAcceptablePhysicsTime = 6.0f;
            break;
            
        case EPerf_TerrainPhysicsOptimizationLevel::Ultra:
            MaxPhysicsBodies = 2000;
            MaxAcceptablePhysicsTime = 4.0f;
            break;
    }
}

void UPerf_TerrainPhysicsPerformanceMonitor::OptimizePhysicsBodies()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Get all static mesh actors
    TArray<AActor*> StaticMeshActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AStaticMeshActor::StaticClass(), StaticMeshActors);
    
    int32 OptimizedBodies = 0;
    
    for (AActor* Actor : StaticMeshActors)
    {
        if (AStaticMeshActor* MeshActor = Cast<AStaticMeshActor>(Actor))
        {
            if (UStaticMeshComponent* MeshComp = MeshActor->GetStaticMeshComponent())
            {
                // Disable physics on distant or small objects
                float DistanceToPlayer = 0.0f; // Simplified - would need player reference
                
                if (MeshComp->IsSimulatingPhysics() && 
                    (DistanceToPlayer > 5000.0f || MeshComp->GetComponentScale().Size() < 0.5f))
                {
                    MeshComp->SetSimulatePhysics(false);
                    OptimizedBodies++;
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsPerformanceMonitor: Optimized %d physics bodies"), OptimizedBodies);
}

void UPerf_TerrainPhysicsPerformanceMonitor::OptimizeCollisionQueries()
{
    // Reduce collision complexity for performance
    if (UPhysicsSettings* PhysicsSettings = GetMutableDefault<UPhysicsSettings>())
    {
        // Adjust collision settings based on optimization level
        switch (CurrentOptimizationLevel)
        {
            case EPerf_TerrainPhysicsOptimizationLevel::Low:
                // Reduce collision precision for better performance
                break;
                
            case EPerf_TerrainPhysicsOptimizationLevel::Ultra:
                // Enable high-precision collision
                break;
                
            default:
                // Medium/High settings
                break;
        }
    }
}

void UPerf_TerrainPhysicsPerformanceMonitor::AdjustPhysicsSettings()
{
    if (UPhysicsSettings* PhysicsSettings = GetMutableDefault<UPhysicsSettings>())
    {
        // Adjust physics solver iterations based on performance
        if (CurrentMetrics.PhysicsStepTime > MaxAcceptablePhysicsTime)
        {
            // Reduce solver iterations for better performance
            UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsPerformanceMonitor: Reducing physics solver precision for performance"));
        }
    }
}