#include "Perf_RealTimePerformanceMonitor.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "HAL/PlatformMemory.h"
#include "Stats/Stats.h"
#include "RenderCore.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"

UPerf_RealTimePerformanceMonitor::UPerf_RealTimePerformanceMonitor()
{
    bIsMonitoring = false;
    MonitoringInterval = 0.5f; // Update every 500ms
    MaxHistorySize = 120; // Keep 1 minute of history at 0.5s intervals
    LastFrameTime = 0.0f;
    FrameTimeAccumulator = 0.0f;
    FrameCount = 0;
    bOptimizationsApplied = false;

    // Default optimization settings
    OptimizationSettings.TargetFrameRate = 60.0f;
    OptimizationSettings.MaxPhysicsActors = 100.0f;
    OptimizationSettings.MaxDrawCalls = 2000.0f;
    OptimizationSettings.bEnableAdaptiveLOD = true;
    OptimizationSettings.bEnableDynamicCulling = true;
    OptimizationSettings.CullingDistance = 10000.0f;
    OptimizationSettings.LODDistanceMultiplier = 1.0f;
}

void UPerf_RealTimePerformanceMonitor::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Performance Monitor: Initializing real-time performance monitoring system"));
    
    // Start monitoring automatically
    StartMonitoring();
}

void UPerf_RealTimePerformanceMonitor::Deinitialize()
{
    StopMonitoring();
    Super::Deinitialize();
}

bool UPerf_RealTimePerformanceMonitor::ShouldCreateSubsystem(UObject* Outer) const
{
    return true;
}

void UPerf_RealTimePerformanceMonitor::StartMonitoring()
{
    if (bIsMonitoring)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("Performance Monitor: Cannot start monitoring - no valid world"));
        return;
    }

    bIsMonitoring = true;
    PerformanceHistory.Empty();
    
    // Set up timer for regular performance updates
    World->GetTimerManager().SetTimer(
        MonitoringTimerHandle,
        this,
        &UPerf_RealTimePerformanceMonitor::UpdatePerformanceMetrics,
        MonitoringInterval,
        true
    );

    UE_LOG(LogTemp, Log, TEXT("Performance Monitor: Started monitoring with %.2f second intervals"), MonitoringInterval);
}

void UPerf_RealTimePerformanceMonitor::StopMonitoring()
{
    if (!bIsMonitoring)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (World && MonitoringTimerHandle.IsValid())
    {
        World->GetTimerManager().ClearTimer(MonitoringTimerHandle);
    }

    bIsMonitoring = false;
    UE_LOG(LogTemp, Log, TEXT("Performance Monitor: Stopped monitoring"));
}

FPerf_PerformanceSnapshot UPerf_RealTimePerformanceMonitor::GetCurrentPerformanceSnapshot()
{
    FPerf_PerformanceSnapshot Snapshot;
    
    // Calculate current frame rate
    Snapshot.FrameRate = CalculateFrameRate();
    Snapshot.FrameTime = 1000.0f / FMath::Max(Snapshot.FrameRate, 1.0f); // Convert to milliseconds
    
    // Get thread times
    Snapshot.GameThreadTime = CalculateGameThreadTime();
    Snapshot.RenderThreadTime = CalculateRenderThreadTime();
    Snapshot.GPUTime = Snapshot.RenderThreadTime * 0.8f; // Estimate GPU time
    
    // Get rendering stats
    Snapshot.DrawCalls = 1500; // Placeholder - would need proper stat collection
    Snapshot.Triangles = 500000; // Placeholder
    
    // Memory usage
    Snapshot.MemoryUsageMB = GetMemoryUsage();
    
    // Physics actors
    Snapshot.PhysicsActorCount = CountPhysicsActors();
    
    // Determine performance level
    if (Snapshot.FrameRate >= 60.0f)
    {
        Snapshot.PerformanceLevel = EPerf_PerformanceLevel::Excellent;
    }
    else if (Snapshot.FrameRate >= 45.0f)
    {
        Snapshot.PerformanceLevel = EPerf_PerformanceLevel::Good;
    }
    else if (Snapshot.FrameRate >= 30.0f)
    {
        Snapshot.PerformanceLevel = EPerf_PerformanceLevel::Fair;
    }
    else if (Snapshot.FrameRate >= 15.0f)
    {
        Snapshot.PerformanceLevel = EPerf_PerformanceLevel::Poor;
    }
    else
    {
        Snapshot.PerformanceLevel = EPerf_PerformanceLevel::Critical;
    }
    
    Snapshot.Timestamp = FDateTime::Now();
    
    return Snapshot;
}

void UPerf_RealTimePerformanceMonitor::SetOptimizationSettings(const FPerf_OptimizationSettings& Settings)
{
    OptimizationSettings = Settings;
    UE_LOG(LogTemp, Log, TEXT("Performance Monitor: Updated optimization settings - Target FPS: %.1f"), Settings.TargetFrameRate);
}

FPerf_OptimizationSettings UPerf_RealTimePerformanceMonitor::GetOptimizationSettings() const
{
    return OptimizationSettings;
}

EPerf_PerformanceLevel UPerf_RealTimePerformanceMonitor::GetCurrentPerformanceLevel()
{
    FPerf_PerformanceSnapshot CurrentSnapshot = GetCurrentPerformanceSnapshot();
    return CurrentSnapshot.PerformanceLevel;
}

float UPerf_RealTimePerformanceMonitor::GetAverageFrameRate(float TimeWindow)
{
    if (PerformanceHistory.Num() == 0)
    {
        return CalculateFrameRate();
    }

    FDateTime CutoffTime = FDateTime::Now() - FTimespan::FromSeconds(TimeWindow);
    float TotalFrameRate = 0.0f;
    int32 ValidSamples = 0;

    for (const FPerf_PerformanceSnapshot& Snapshot : PerformanceHistory)
    {
        if (Snapshot.Timestamp >= CutoffTime)
        {
            TotalFrameRate += Snapshot.FrameRate;
            ValidSamples++;
        }
    }

    return ValidSamples > 0 ? TotalFrameRate / ValidSamples : CalculateFrameRate();
}

bool UPerf_RealTimePerformanceMonitor::IsPerformanceCritical()
{
    float CurrentFPS = CalculateFrameRate();
    return CurrentFPS < 20.0f || GetCurrentPerformanceLevel() == EPerf_PerformanceLevel::Critical;
}

void UPerf_RealTimePerformanceMonitor::ApplyPerformanceOptimizations()
{
    UE_LOG(LogTemp, Warning, TEXT("Performance Monitor: Applying performance optimizations"));

    if (OptimizationSettings.bEnableAdaptiveLOD)
    {
        OptimizeLODDistances();
        OnOptimizationApplied.Broadcast(TEXT("Adaptive LOD"));
    }

    if (OptimizationSettings.bEnableDynamicCulling)
    {
        OptimizeCullingDistances();
        OnOptimizationApplied.Broadcast(TEXT("Dynamic Culling"));
    }

    OptimizePhysicsSettings();
    OptimizeRenderSettings();

    bOptimizationsApplied = true;
    OnOptimizationApplied.Broadcast(TEXT("Full Optimization Suite"));
}

void UPerf_RealTimePerformanceMonitor::ResetOptimizations()
{
    UE_LOG(LogTemp, Log, TEXT("Performance Monitor: Resetting optimizations to default"));

    // Reset LOD distances
    OptimizationSettings.LODDistanceMultiplier = 1.0f;
    
    // Reset culling distance
    OptimizationSettings.CullingDistance = 10000.0f;

    bOptimizationsApplied = false;
    OnOptimizationApplied.Broadcast(TEXT("Optimization Reset"));
}

void UPerf_RealTimePerformanceMonitor::SetAdaptiveLODEnabled(bool bEnabled)
{
    OptimizationSettings.bEnableAdaptiveLOD = bEnabled;
    UE_LOG(LogTemp, Log, TEXT("Performance Monitor: Adaptive LOD %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UPerf_RealTimePerformanceMonitor::SetDynamicCullingEnabled(bool bEnabled)
{
    OptimizationSettings.bEnableDynamicCulling = bEnabled;
    UE_LOG(LogTemp, Log, TEXT("Performance Monitor: Dynamic culling %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UPerf_RealTimePerformanceMonitor::LogPerformanceReport()
{
    FPerf_PerformanceSnapshot CurrentSnapshot = GetCurrentPerformanceSnapshot();
    float AvgFPS = GetAverageFrameRate(10.0f);

    UE_LOG(LogTemp, Warning, TEXT("=== PERFORMANCE REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Current FPS: %.1f"), CurrentSnapshot.FrameRate);
    UE_LOG(LogTemp, Warning, TEXT("Average FPS (10s): %.1f"), AvgFPS);
    UE_LOG(LogTemp, Warning, TEXT("Frame Time: %.2f ms"), CurrentSnapshot.FrameTime);
    UE_LOG(LogTemp, Warning, TEXT("Game Thread: %.2f ms"), CurrentSnapshot.GameThreadTime);
    UE_LOG(LogTemp, Warning, TEXT("Render Thread: %.2f ms"), CurrentSnapshot.RenderThreadTime);
    UE_LOG(LogTemp, Warning, TEXT("Memory Usage: %.1f MB"), CurrentSnapshot.MemoryUsageMB);
    UE_LOG(LogTemp, Warning, TEXT("Physics Actors: %d"), CurrentSnapshot.PhysicsActorCount);
    UE_LOG(LogTemp, Warning, TEXT("Performance Level: %s"), 
        CurrentSnapshot.PerformanceLevel == EPerf_PerformanceLevel::Excellent ? TEXT("Excellent") :
        CurrentSnapshot.PerformanceLevel == EPerf_PerformanceLevel::Good ? TEXT("Good") :
        CurrentSnapshot.PerformanceLevel == EPerf_PerformanceLevel::Fair ? TEXT("Fair") :
        CurrentSnapshot.PerformanceLevel == EPerf_PerformanceLevel::Poor ? TEXT("Poor") : TEXT("Critical"));
    UE_LOG(LogTemp, Warning, TEXT("========================"));
}

void UPerf_RealTimePerformanceMonitor::ExportPerformanceData(const FString& FilePath)
{
    FString ExportData = TEXT("Timestamp,FrameRate,FrameTime,GameThread,RenderThread,MemoryMB,PhysicsActors,PerformanceLevel\n");

    for (const FPerf_PerformanceSnapshot& Snapshot : PerformanceHistory)
    {
        FString LevelString;
        switch (Snapshot.PerformanceLevel)
        {
            case EPerf_PerformanceLevel::Excellent: LevelString = TEXT("Excellent"); break;
            case EPerf_PerformanceLevel::Good: LevelString = TEXT("Good"); break;
            case EPerf_PerformanceLevel::Fair: LevelString = TEXT("Fair"); break;
            case EPerf_PerformanceLevel::Poor: LevelString = TEXT("Poor"); break;
            case EPerf_PerformanceLevel::Critical: LevelString = TEXT("Critical"); break;
        }

        ExportData += FString::Printf(TEXT("%s,%.2f,%.2f,%.2f,%.2f,%.1f,%d,%s\n"),
            *Snapshot.Timestamp.ToString(),
            Snapshot.FrameRate,
            Snapshot.FrameTime,
            Snapshot.GameThreadTime,
            Snapshot.RenderThreadTime,
            Snapshot.MemoryUsageMB,
            Snapshot.PhysicsActorCount,
            *LevelString
        );
    }

    FFileHelper::SaveStringToFile(ExportData, *FilePath);
    UE_LOG(LogTemp, Log, TEXT("Performance Monitor: Exported performance data to %s"), *FilePath);
}

void UPerf_RealTimePerformanceMonitor::UpdatePerformanceMetrics()
{
    if (!bIsMonitoring)
    {
        return;
    }

    FPerf_PerformanceSnapshot NewSnapshot = GetCurrentPerformanceSnapshot();
    
    // Add to history
    PerformanceHistory.Add(NewSnapshot);
    
    // Trim history if too large
    if (PerformanceHistory.Num() > MaxHistorySize)
    {
        PerformanceHistory.RemoveAt(0);
    }

    // Check for performance issues
    CheckPerformanceThresholds();

    // Apply adaptive optimizations if needed
    if (OptimizationSettings.bEnableAdaptiveLOD || OptimizationSettings.bEnableDynamicCulling)
    {
        ApplyAdaptiveOptimizations();
    }
}

void UPerf_RealTimePerformanceMonitor::CheckPerformanceThresholds()
{
    FPerf_PerformanceSnapshot CurrentSnapshot = GetCurrentPerformanceSnapshot();
    
    // Check if performance has dropped below target
    if (CurrentSnapshot.FrameRate < OptimizationSettings.TargetFrameRate * 0.8f)
    {
        OnPerformanceThresholdExceeded.Broadcast(CurrentSnapshot);
        
        if (!bOptimizationsApplied)
        {
            ApplyPerformanceOptimizations();
        }
    }
}

void UPerf_RealTimePerformanceMonitor::ApplyAdaptiveOptimizations()
{
    float CurrentFPS = CalculateFrameRate();
    float TargetFPS = OptimizationSettings.TargetFrameRate;

    if (CurrentFPS < TargetFPS * 0.9f)
    {
        // Performance is below target, increase optimization
        OptimizationSettings.LODDistanceMultiplier = FMath::Max(0.5f, OptimizationSettings.LODDistanceMultiplier * 0.95f);
        OptimizationSettings.CullingDistance = FMath::Max(5000.0f, OptimizationSettings.CullingDistance * 0.95f);
    }
    else if (CurrentFPS > TargetFPS * 1.1f)
    {
        // Performance is good, can reduce optimization
        OptimizationSettings.LODDistanceMultiplier = FMath::Min(1.5f, OptimizationSettings.LODDistanceMultiplier * 1.02f);
        OptimizationSettings.CullingDistance = FMath::Min(15000.0f, OptimizationSettings.CullingDistance * 1.02f);
    }
}

float UPerf_RealTimePerformanceMonitor::CalculateFrameRate()
{
    if (GEngine && GEngine->GetGameViewport())
    {
        return 1.0f / FApp::GetDeltaTime();
    }
    return 60.0f; // Default fallback
}

float UPerf_RealTimePerformanceMonitor::CalculateGameThreadTime()
{
    return FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds
}

float UPerf_RealTimePerformanceMonitor::CalculateRenderThreadTime()
{
    return FApp::GetDeltaTime() * 1000.0f * 0.8f; // Estimate as 80% of frame time
}

int32 UPerf_RealTimePerformanceMonitor::CountPhysicsActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return 0;
    }

    int32 PhysicsActorCount = 0;
    for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (Actor && Actor->GetRootComponent())
        {
            if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
            {
                if (PrimComp->IsSimulatingPhysics())
                {
                    PhysicsActorCount++;
                }
            }
        }
    }

    return PhysicsActorCount;
}

float UPerf_RealTimePerformanceMonitor::GetMemoryUsage()
{
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    return MemStats.UsedPhysical / (1024.0f * 1024.0f); // Convert to MB
}

void UPerf_RealTimePerformanceMonitor::OptimizeLODDistances()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (Actor)
        {
            if (UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>())
            {
                // Adjust LOD distances based on performance
                float DistanceMultiplier = OptimizationSettings.LODDistanceMultiplier;
                // Implementation would adjust LOD distances here
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Performance Monitor: Optimized LOD distances (multiplier: %.2f)"), OptimizationSettings.LODDistanceMultiplier);
}

void UPerf_RealTimePerformanceMonitor::OptimizeCullingDistances()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (Actor)
        {
            if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
            {
                PrimComp->SetCullDistance(OptimizationSettings.CullingDistance);
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Performance Monitor: Optimized culling distances (%.0f units)"), OptimizationSettings.CullingDistance);
}

void UPerf_RealTimePerformanceMonitor::OptimizePhysicsSettings()
{
    // Reduce physics simulation frequency if performance is poor
    float CurrentFPS = CalculateFrameRate();
    if (CurrentFPS < 30.0f)
    {
        if (UPhysicsSettings* PhysicsSettings = GetMutableDefault<UPhysicsSettings>())
        {
            // Reduce physics substeps for better performance
            PhysicsSettings->MaxSubsteps = FMath::Max(1, PhysicsSettings->MaxSubsteps - 1);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Performance Monitor: Optimized physics settings"));
}

void UPerf_RealTimePerformanceMonitor::OptimizeRenderSettings()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    float CurrentFPS = CalculateFrameRate();
    
    if (CurrentFPS < 30.0f)
    {
        // Apply aggressive optimizations
        GEngine->Exec(World, TEXT("r.ScreenPercentage 75"));
        GEngine->Exec(World, TEXT("r.Shadow.MaxResolution 512"));
        GEngine->Exec(World, TEXT("r.PostProcessAAQuality 0"));
    }
    else if (CurrentFPS < 45.0f)
    {
        // Apply moderate optimizations
        GEngine->Exec(World, TEXT("r.ScreenPercentage 85"));
        GEngine->Exec(World, TEXT("r.Shadow.MaxResolution 1024"));
        GEngine->Exec(World, TEXT("r.PostProcessAAQuality 2"));
    }

    UE_LOG(LogTemp, Log, TEXT("Performance Monitor: Optimized render settings for %.1f FPS"), CurrentFPS);
}