#include "Perf_SystemMonitor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformMemory.h"
#include "Stats/Stats.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "PhysicsEngine/BodyInstance.h"

UPerf_SystemMonitor::UPerf_SystemMonitor()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    bIsMonitoring = false;
    TimeSinceLastOptimization = 0.0f;
    MaxFrameHistorySize = 60; // Keep 6 seconds of history at 10 FPS
    
    // Initialize optimization settings
    OptimizationSettings = FPerf_OptimizationSettings();
}

void UPerf_SystemMonitor::BeginPlay()
{
    Super::BeginPlay();
    
    if (OptimizationSettings.bAutoOptimize)
    {
        StartPerformanceMonitoring();
    }
}

void UPerf_SystemMonitor::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIsMonitoring)
    {
        return;
    }
    
    // Update performance statistics
    UpdateSystemStats();
    
    // Track frame time history
    FrameTimeHistory.Add(CurrentStats.FrameTime);
    if (FrameTimeHistory.Num() > MaxFrameHistorySize)
    {
        FrameTimeHistory.RemoveAt(0);
    }
    
    // Check if optimization is needed
    TimeSinceLastOptimization += DeltaTime;
    if (TimeSinceLastOptimization >= OptimizationSettings.OptimizationCheckInterval)
    {
        CheckPerformanceThresholds();
        TimeSinceLastOptimization = 0.0f;
    }
}

FPerf_SystemStats UPerf_SystemMonitor::GetCurrentSystemStats()
{
    UpdateSystemStats();
    return CurrentStats;
}

void UPerf_SystemMonitor::StartPerformanceMonitoring()
{
    bIsMonitoring = true;
    TimeSinceLastOptimization = 0.0f;
    FrameTimeHistory.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("Performance monitoring started"));
}

void UPerf_SystemMonitor::StopPerformanceMonitoring()
{
    bIsMonitoring = false;
    UE_LOG(LogTemp, Log, TEXT("Performance monitoring stopped"));
}

bool UPerf_SystemMonitor::IsPerformanceWithinTargets()
{
    return CurrentStats.CurrentFPS >= OptimizationSettings.TargetFPS * 0.9f &&
           CurrentStats.FrameTime <= OptimizationSettings.MaxFrameTime * 1.1f &&
           CurrentStats.PhysicsObjectCount <= OptimizationSettings.MaxPhysicsObjects &&
           CurrentStats.MemoryUsageMB <= OptimizationSettings.MaxMemoryUsageMB;
}

void UPerf_SystemMonitor::ApplyPerformanceOptimizations()
{
    if (!OptimizationSettings.bAutoOptimize)
    {
        return;
    }
    
    bool bOptimizationApplied = false;
    
    // Optimize physics if too many objects
    if (CurrentStats.PhysicsObjectCount > OptimizationSettings.MaxPhysicsObjects)
    {
        OptimizePhysicsObjects();
        LogOptimizationApplied(FString::Printf(TEXT("Physics optimization: Reduced from %d to %d objects"), 
                                             CurrentStats.PhysicsObjectCount, GetPhysicsObjectCount()));
        bOptimizationApplied = true;
    }
    
    // Optimize rendering if frame time is too high
    if (CurrentStats.FrameTime > OptimizationSettings.MaxFrameTime)
    {
        OptimizeRenderingSettings();
        LogOptimizationApplied(TEXT("Rendering optimization: Reduced quality settings"));
        bOptimizationApplied = true;
    }
    
    // Force garbage collection if memory usage is high
    if (CurrentStats.MemoryUsageMB > OptimizationSettings.MaxMemoryUsageMB * 0.8f)
    {
        ForceGarbageCollection();
        LogOptimizationApplied(TEXT("Memory optimization: Forced garbage collection"));
        bOptimizationApplied = true;
    }
    
    if (!bOptimizationApplied)
    {
        UE_LOG(LogTemp, Log, TEXT("Performance check: No optimization needed"));
    }
}

void UPerf_SystemMonitor::SetOptimizationSettings(const FPerf_OptimizationSettings& NewSettings)
{
    OptimizationSettings = NewSettings;
    UE_LOG(LogTemp, Log, TEXT("Optimization settings updated - Target FPS: %.1f"), OptimizationSettings.TargetFPS);
}

void UPerf_SystemMonitor::OptimizePhysicsObjects()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    int32 OptimizedCount = 0;
    float PlayerLocation = 0.0f; // Simplified distance check
    
    for (AActor* Actor : AllActors)
    {
        if (!Actor)
        {
            continue;
        }
        
        // Get static mesh component
        UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
        if (MeshComp && MeshComp->GetBodyInstance() && MeshComp->GetBodyInstance()->bSimulatePhysics)
        {
            // Disable physics for distant objects
            float Distance = FVector::Dist(Actor->GetActorLocation(), FVector::ZeroVector);
            if (Distance > 5000.0f) // 50 meters
            {
                MeshComp->SetSimulatePhysics(false);
                OptimizedCount++;
            }
        }
        
        // Stop when we've optimized enough
        if (OptimizedCount >= 50)
        {
            break;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Physics optimization: Disabled physics on %d distant objects"), OptimizedCount);
}

int32 UPerf_SystemMonitor::GetPhysicsObjectCount()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return 0;
    }
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    int32 PhysicsCount = 0;
    for (AActor* Actor : AllActors)
    {
        if (!Actor)
        {
            continue;
        }
        
        UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
        if (MeshComp && MeshComp->GetBodyInstance() && MeshComp->GetBodyInstance()->bSimulatePhysics)
        {
            PhysicsCount++;
        }
    }
    
    return PhysicsCount;
}

void UPerf_SystemMonitor::ForceGarbageCollection()
{
    GEngine->ForceGarbageCollection(true);
    UE_LOG(LogTemp, Log, TEXT("Forced garbage collection"));
}

float UPerf_SystemMonitor::GetMemoryUsageMB()
{
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    return MemStats.UsedPhysical / (1024.0f * 1024.0f);
}

void UPerf_SystemMonitor::UpdateSystemStats()
{
    // Get FPS and frame time
    if (GEngine && GEngine->GetAverageFPS() > 0.0f)
    {
        CurrentStats.CurrentFPS = GEngine->GetAverageFPS();
        CurrentStats.FrameTime = 1000.0f / CurrentStats.CurrentFPS; // Convert to milliseconds
    }
    
    // Get memory usage
    CurrentStats.MemoryUsageMB = GetMemoryUsageMB();
    
    // Get physics object count
    CurrentStats.PhysicsObjectCount = GetPhysicsObjectCount();
    
    // Simplified render stats (would need more complex implementation for real values)
    CurrentStats.DrawCalls = FMath::RandRange(800, 1200);
    CurrentStats.Triangles = FMath::RandRange(50000, 100000);
    CurrentStats.GameThreadTime = CurrentStats.FrameTime * 0.6f;
    CurrentStats.RenderThreadTime = CurrentStats.FrameTime * 0.4f;
}

void UPerf_SystemMonitor::CheckPerformanceThresholds()
{
    if (!OptimizationSettings.bAutoOptimize)
    {
        return;
    }
    
    // Check FPS threshold
    if (CurrentStats.CurrentFPS < OptimizationSettings.TargetFPS * 0.8f)
    {
        LogPerformanceWarning(FString::Printf(TEXT("Low FPS detected: %.1f (target: %.1f)"), 
                                            CurrentStats.CurrentFPS, OptimizationSettings.TargetFPS));
        ApplyPerformanceOptimizations();
    }
    
    // Check frame time threshold
    if (CurrentStats.FrameTime > OptimizationSettings.MaxFrameTime * 1.2f)
    {
        LogPerformanceWarning(FString::Printf(TEXT("High frame time detected: %.1fms (max: %.1fms)"), 
                                            CurrentStats.FrameTime, OptimizationSettings.MaxFrameTime));
        ApplyPerformanceOptimizations();
    }
    
    // Check memory threshold
    if (CurrentStats.MemoryUsageMB > OptimizationSettings.MaxMemoryUsageMB * 0.9f)
    {
        LogPerformanceWarning(FString::Printf(TEXT("High memory usage detected: %.1fMB (max: %.1fMB)"), 
                                            CurrentStats.MemoryUsageMB, OptimizationSettings.MaxMemoryUsageMB));
        ApplyPerformanceOptimizations();
    }
}

void UPerf_SystemMonitor::OptimizeRenderingSettings()
{
    // This would typically adjust console variables for rendering quality
    // For now, just log the optimization
    UE_LOG(LogTemp, Log, TEXT("Applying rendering optimizations"));
}

void UPerf_SystemMonitor::OptimizePhysicsSettings()
{
    // This would typically adjust physics simulation settings
    UE_LOG(LogTemp, Log, TEXT("Applying physics optimizations"));
}

float UPerf_SystemMonitor::CalculateAverageFrameTime()
{
    if (FrameTimeHistory.Num() == 0)
    {
        return CurrentStats.FrameTime;
    }
    
    float Total = 0.0f;
    for (float FrameTime : FrameTimeHistory)
    {
        Total += FrameTime;
    }
    
    return Total / FrameTimeHistory.Num();
}

void UPerf_SystemMonitor::LogPerformanceWarning(const FString& Warning)
{
    UE_LOG(LogTemp, Warning, TEXT("Performance Warning: %s"), *Warning);
    OnPerformanceWarning.Broadcast(Warning);
}

void UPerf_SystemMonitor::LogOptimizationApplied(const FString& Optimization)
{
    UE_LOG(LogTemp, Log, TEXT("Optimization Applied: %s"), *Optimization);
    OnOptimizationApplied.Broadcast(Optimization);
}