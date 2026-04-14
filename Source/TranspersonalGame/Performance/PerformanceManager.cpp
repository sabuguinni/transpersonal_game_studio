#include "PerformanceManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformMemory.h"
#include "Stats/Stats.h"
#include "RenderingThread.h"
#include "Engine/GameViewportClient.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/Actor.h"

UPerformanceManager::UPerformanceManager()
{
    bIsMonitoring = false;
    bIsProfiling = false;
    CurrentState = EPerf_OptimizationState::Stable;
    CurrentLevel = EPerf_PerformanceLevel::High;
    
    UpdateInterval = 0.1f; // Update metrics 10 times per second
    MaxHistorySize = 300; // Keep 30 seconds of history at 10 updates/sec
    
    OptimizationCooldown = 2.0f; // Wait 2 seconds between optimization passes
    LastOptimizationTime = 0.0f;
    
    CriticalFPSThreshold = 20.0f;
    LowFPSThreshold = 45.0f;
    ConsecutiveLowFrames = 0;
    MaxConsecutiveLowFrames = 30; // 3 seconds at 10 updates/sec
    
    // Initialize default optimization settings
    OptimizationSettings = FPerf_OptimizationSettings();
}

void UPerformanceManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("PerformanceManager: Initialized"));
    
    // Start monitoring by default
    StartPerformanceMonitoring();
}

void UPerformanceManager::Deinitialize()
{
    StopPerformanceMonitoring();
    StopProfiling();
    
    Super::Deinitialize();
}

void UPerformanceManager::StartPerformanceMonitoring()
{
    if (bIsMonitoring)
    {
        return;
    }
    
    bIsMonitoring = true;
    LastUpdateTime = FDateTime::Now();
    
    // Clear history
    FPSHistory.Empty();
    FrameTimeHistory.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("PerformanceManager: Started performance monitoring"));
    
    // Start a timer for regular updates
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            FTimerHandle(),
            FTimerDelegate::CreateUObject(this, &UPerformanceManager::UpdatePerformanceMetrics),
            UpdateInterval,
            true
        );
    }
}

void UPerformanceManager::StopPerformanceMonitoring()
{
    if (!bIsMonitoring)
    {
        return;
    }
    
    bIsMonitoring = false;
    
    UE_LOG(LogTemp, Log, TEXT("PerformanceManager: Stopped performance monitoring"));
}

void UPerformanceManager::UpdatePerformanceMetrics()
{
    if (!bIsMonitoring)
    {
        return;
    }
    
    // Get current FPS
    float DeltaTime = FApp::GetDeltaTime();
    CurrentMetrics.CurrentFPS = DeltaTime > 0.0f ? (1.0f / DeltaTime) : 0.0f;
    CurrentMetrics.FrameTime = DeltaTime * 1000.0f; // Convert to milliseconds
    
    // Update FPS history
    FPSHistory.Add(CurrentMetrics.CurrentFPS);
    FrameTimeHistory.Add(CurrentMetrics.FrameTime);
    
    if (FPSHistory.Num() > MaxHistorySize)
    {
        FPSHistory.RemoveAt(0);
        FrameTimeHistory.RemoveAt(0);
    }
    
    // Calculate average, min, max FPS
    if (FPSHistory.Num() > 0)
    {
        float TotalFPS = 0.0f;
        CurrentMetrics.MinFPS = FPSHistory[0];
        CurrentMetrics.MaxFPS = FPSHistory[0];
        
        for (float FPS : FPSHistory)
        {
            TotalFPS += FPS;
            CurrentMetrics.MinFPS = FMath::Min(CurrentMetrics.MinFPS, FPS);
            CurrentMetrics.MaxFPS = FMath::Max(CurrentMetrics.MaxFPS, FPS);
        }
        
        CurrentMetrics.AverageFPS = TotalFPS / FPSHistory.Num();
    }
    
    // Get memory usage
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.UsedMemoryMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    CurrentMetrics.AvailableMemoryMB = MemStats.AvailablePhysical / (1024.0f * 1024.0f);
    
    // Get world statistics
    if (UWorld* World = GetWorld())
    {
        CurrentMetrics.ActorCount = World->GetActorCount();
        
        // Count components
        int32 ComponentCount = 0;
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor)
            {
                ComponentCount += Actor->GetRootComponent() ? Actor->GetComponents<UActorComponent>().Num() : 0;
            }
        }
        CurrentMetrics.ComponentCount = ComponentCount;
    }
    
    // Estimate rendering stats (simplified)
    CurrentMetrics.DrawCalls = FMath::Clamp(CurrentMetrics.ActorCount / 2, 100, 5000);
    CurrentMetrics.TriangleCount = FMath::Clamp(CurrentMetrics.ActorCount * 1000, 10000, 2000000);
    
    // Check performance thresholds
    CheckPerformanceThresholds();
    
    // Apply optimizations if needed
    if (OptimizationSettings.bAutoOptimization)
    {
        float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
        if (CurrentTime - LastOptimizationTime > OptimizationCooldown)
        {
            AnalyzePerformanceBottlenecks();
        }
    }
}

void UPerformanceManager::CheckPerformanceThresholds()
{
    EPerf_OptimizationState NewState = CurrentState;
    
    if (CurrentMetrics.CurrentFPS < CriticalFPSThreshold)
    {
        NewState = EPerf_OptimizationState::Critical;
        ConsecutiveLowFrames++;
        
        // Broadcast FPS drop event
        OnFPSDropDetected.Broadcast(CurrentMetrics.CurrentFPS);
    }
    else if (CurrentMetrics.CurrentFPS < LowFPSThreshold)
    {
        NewState = EPerf_OptimizationState::Optimizing;
        ConsecutiveLowFrames++;
    }
    else if (CurrentMetrics.CurrentFPS >= OptimizationSettings.TargetFPS * 0.9f)
    {
        NewState = EPerf_OptimizationState::Stable;
        ConsecutiveLowFrames = 0;
    }
    else
    {
        NewState = EPerf_OptimizationState::Analyzing;
        ConsecutiveLowFrames = FMath::Max(0, ConsecutiveLowFrames - 1);
    }
    
    if (NewState != CurrentState)
    {
        CurrentState = NewState;
        OnPerformanceStateChanged.Broadcast(CurrentState);
        
        UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: State changed to %d"), (int32)CurrentState);
    }
}

void UPerformanceManager::AnalyzePerformanceBottlenecks()
{
    if (CurrentState == EPerf_OptimizationState::Stable)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("PerformanceManager: Analyzing performance bottlenecks"));
    
    // Determine what needs optimization
    bool bNeedsOptimization = false;
    
    if (CurrentMetrics.CurrentFPS < OptimizationSettings.TargetFPS)
    {
        bNeedsOptimization = true;
    }
    
    if (CurrentMetrics.UsedMemoryMB > OptimizationSettings.MaxMemoryUsageMB * 0.8f)
    {
        bNeedsOptimization = true;
    }
    
    if (CurrentMetrics.DrawCalls > OptimizationSettings.MaxDrawCalls)
    {
        bNeedsOptimization = true;
    }
    
    if (bNeedsOptimization)
    {
        ApplyOptimizations();
    }
}

void UPerformanceManager::ApplyOptimizations()
{
    UE_LOG(LogTemp, Log, TEXT("PerformanceManager: Applying optimizations"));
    
    LastOptimizationTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    // Apply different optimization strategies based on current state
    switch (CurrentState)
    {
        case EPerf_OptimizationState::Critical:
            // Aggressive optimizations
            OptimizeLODLevels();
            OptimizeRenderingSettings();
            OptimizeCullingDistance();
            OptimizeMemorySettings();
            break;
            
        case EPerf_OptimizationState::Optimizing:
            // Moderate optimizations
            OptimizeLODLevels();
            OptimizeRenderingSettings();
            break;
            
        case EPerf_OptimizationState::Analyzing:
            // Light optimizations
            if (OptimizationSettings.bDynamicLOD)
            {
                UpdateDynamicLOD();
            }
            break;
            
        default:
            break;
    }
}

void UPerformanceManager::OptimizeLODLevels()
{
    if (!OptimizationSettings.bDynamicLOD)
    {
        return;
    }
    
    // Adjust LOD distance scale based on performance
    float NewLODScale = OptimizationSettings.LODDistanceScale;
    
    if (CurrentMetrics.CurrentFPS < OptimizationSettings.MinAcceptableFPS)
    {
        NewLODScale *= 0.8f; // More aggressive LOD
    }
    else if (CurrentMetrics.CurrentFPS < OptimizationSettings.TargetFPS)
    {
        NewLODScale *= 0.9f; // Slightly more aggressive LOD
    }
    
    NewLODScale = FMath::Clamp(NewLODScale, 0.5f, 2.0f);
    
    if (NewLODScale != OptimizationSettings.LODDistanceScale)
    {
        SetLODDistanceScale(NewLODScale);
        UE_LOG(LogTemp, Log, TEXT("PerformanceManager: Adjusted LOD scale to %f"), NewLODScale);
    }
}

void UPerformanceManager::OptimizeRenderingSettings()
{
    // Adjust view distance based on performance
    float NewViewDistance = OptimizationSettings.ViewDistanceScale;
    
    if (CurrentState == EPerf_OptimizationState::Critical)
    {
        NewViewDistance *= 0.7f;
    }
    else if (CurrentState == EPerf_OptimizationState::Optimizing)
    {
        NewViewDistance *= 0.85f;
    }
    
    NewViewDistance = FMath::Clamp(NewViewDistance, 0.5f, 1.5f);
    OptimizationSettings.ViewDistanceScale = NewViewDistance;
    
    UE_LOG(LogTemp, Log, TEXT("PerformanceManager: Adjusted view distance to %f"), NewViewDistance);
}

void UPerformanceManager::OptimizeCullingDistance()
{
    float NewCullingDistance = OptimizationSettings.CullingDistance;
    
    if (CurrentState == EPerf_OptimizationState::Critical)
    {
        NewCullingDistance *= 0.6f;
    }
    else if (CurrentState == EPerf_OptimizationState::Optimizing)
    {
        NewCullingDistance *= 0.8f;
    }
    
    OptimizationSettings.CullingDistance = FMath::Max(NewCullingDistance, 1000.0f);
    
    UE_LOG(LogTemp, Log, TEXT("PerformanceManager: Adjusted culling distance to %f"), OptimizationSettings.CullingDistance);
}

void UPerformanceManager::OptimizeMemorySettings()
{
    if (CurrentMetrics.UsedMemoryMB > OptimizationSettings.MaxMemoryUsageMB * 0.8f)
    {
        ForceGarbageCollection();
        UE_LOG(LogTemp, Log, TEXT("PerformanceManager: Forced garbage collection"));
    }
}

FPerf_PerformanceMetrics UPerformanceManager::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

EPerf_PerformanceLevel UPerformanceManager::GetCurrentPerformanceLevel() const
{
    return CurrentLevel;
}

EPerf_OptimizationState UPerformanceManager::GetOptimizationState() const
{
    return CurrentState;
}

void UPerformanceManager::SetOptimizationSettings(const FPerf_OptimizationSettings& NewSettings)
{
    OptimizationSettings = NewSettings;
    UE_LOG(LogTemp, Log, TEXT("PerformanceManager: Updated optimization settings"));
}

FPerf_OptimizationSettings UPerformanceManager::GetOptimizationSettings() const
{
    return OptimizationSettings;
}

void UPerformanceManager::ForceOptimizationPass()
{
    LastOptimizationTime = 0.0f; // Reset cooldown
    AnalyzePerformanceBottlenecks();
}

void UPerformanceManager::SetPerformanceLevel(EPerf_PerformanceLevel Level)
{
    CurrentLevel = Level;
    
    // Adjust settings based on performance level
    switch (Level)
    {
        case EPerf_PerformanceLevel::Low:
            OptimizationSettings.TargetFPS = 30.0f;
            OptimizationSettings.MinAcceptableFPS = 20.0f;
            OptimizationSettings.LODDistanceScale = 0.6f;
            OptimizationSettings.ViewDistanceScale = 0.7f;
            break;
            
        case EPerf_PerformanceLevel::Medium:
            OptimizationSettings.TargetFPS = 45.0f;
            OptimizationSettings.MinAcceptableFPS = 30.0f;
            OptimizationSettings.LODDistanceScale = 0.8f;
            OptimizationSettings.ViewDistanceScale = 0.85f;
            break;
            
        case EPerf_PerformanceLevel::High:
            OptimizationSettings.TargetFPS = 60.0f;
            OptimizationSettings.MinAcceptableFPS = 45.0f;
            OptimizationSettings.LODDistanceScale = 1.0f;
            OptimizationSettings.ViewDistanceScale = 1.0f;
            break;
            
        case EPerf_PerformanceLevel::Ultra:
            OptimizationSettings.TargetFPS = 120.0f;
            OptimizationSettings.MinAcceptableFPS = 60.0f;
            OptimizationSettings.LODDistanceScale = 1.2f;
            OptimizationSettings.ViewDistanceScale = 1.2f;
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("PerformanceManager: Set performance level to %d"), (int32)Level);
}

void UPerformanceManager::UpdateDynamicLOD()
{
    // This would update LOD levels for all relevant actors in the world
    // Implementation would iterate through actors and adjust their LOD settings
    UE_LOG(LogTemp, Log, TEXT("PerformanceManager: Updated dynamic LOD"));
}

void UPerformanceManager::SetLODDistanceScale(float Scale)
{
    OptimizationSettings.LODDistanceScale = FMath::Clamp(Scale, 0.1f, 3.0f);
    
    // Apply to console variables
    if (UWorld* World = GetWorld())
    {
        FString Command = FString::Printf(TEXT("r.StaticMeshLODDistanceScale %f"), Scale);
        GEngine->Exec(World, *Command);
        
        Command = FString::Printf(TEXT("r.SkeletalMeshLODDistanceScale %f"), Scale);
        GEngine->Exec(World, *Command);
    }
}

void UPerformanceManager::ForceGarbageCollection()
{
    GEngine->ForceGarbageCollection(true);
}

void UPerformanceManager::OptimizeMemoryUsage()
{
    ForceGarbageCollection();
    
    // Additional memory optimizations could be added here
    UE_LOG(LogTemp, Log, TEXT("PerformanceManager: Optimized memory usage"));
}

void UPerformanceManager::StartProfiling(const FString& ProfileName)
{
    if (bIsProfiling)
    {
        StopProfiling();
    }
    
    bIsProfiling = true;
    CurrentProfileName = ProfileName;
    
    UE_LOG(LogTemp, Log, TEXT("PerformanceManager: Started profiling '%s'"), *ProfileName);
}

void UPerformanceManager::StopProfiling()
{
    if (!bIsProfiling)
    {
        return;
    }
    
    bIsProfiling = false;
    
    UE_LOG(LogTemp, Log, TEXT("PerformanceManager: Stopped profiling '%s'"), *CurrentProfileName);
    
    CurrentProfileName.Empty();
}

void UPerformanceManager::SavePerformanceReport(const FString& ReportName)
{
    // Create a performance report with current metrics and history
    FString ReportContent = FString::Printf(TEXT(
        "Performance Report: %s\n"
        "Generated: %s\n"
        "Current FPS: %.2f\n"
        "Average FPS: %.2f\n"
        "Min FPS: %.2f\n"
        "Max FPS: %.2f\n"
        "Frame Time: %.2f ms\n"
        "Memory Used: %.2f MB\n"
        "Actor Count: %d\n"
        "Component Count: %d\n"
        "Draw Calls: %d\n"
        "Triangle Count: %d\n"
        "Optimization State: %d\n"
        "Performance Level: %d\n"
    ),
    *ReportName,
    *FDateTime::Now().ToString(),
    CurrentMetrics.CurrentFPS,
    CurrentMetrics.AverageFPS,
    CurrentMetrics.MinFPS,
    CurrentMetrics.MaxFPS,
    CurrentMetrics.FrameTime,
    CurrentMetrics.UsedMemoryMB,
    CurrentMetrics.ActorCount,
    CurrentMetrics.ComponentCount,
    CurrentMetrics.DrawCalls,
    CurrentMetrics.TriangleCount,
    (int32)CurrentState,
    (int32)CurrentLevel
    );
    
    // Save to file (simplified - would need proper file handling)
    UE_LOG(LogTemp, Log, TEXT("PerformanceManager: Performance report saved:\n%s"), *ReportContent);
}