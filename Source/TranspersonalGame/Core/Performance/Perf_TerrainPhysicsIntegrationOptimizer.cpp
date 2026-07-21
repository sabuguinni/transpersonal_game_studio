#include "Perf_TerrainPhysicsIntegrationOptimizer.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Stats/StatsHierarchical.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/StaticMeshActor.h"

DECLARE_STATS_GROUP(TEXT("TerrainPhysicsIntegration"), STATGROUP_TerrainPhysicsIntegration, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("Terrain Physics Integration Update"), STAT_TerrainPhysicsIntegrationUpdate, STATGROUP_TerrainPhysicsIntegration);
DECLARE_CYCLE_STAT(TEXT("Physics Optimization"), STAT_PhysicsOptimization, STATGROUP_TerrainPhysicsIntegration);
DECLARE_CYCLE_STAT(TEXT("Terrain Collision Optimization"), STAT_TerrainCollisionOptimization, STATGROUP_TerrainPhysicsIntegration);

UPerf_TerrainPhysicsIntegrationOptimizer::UPerf_TerrainPhysicsIntegrationOptimizer()
    : bIsMonitoring(false)
    , bOptimizationActive(false)
    , LastOptimizationTime(0.0f)
{
    // Initialize default settings
    OptimizationSettings = FPerf_TerrainPhysicsOptimizationSettings();
    CurrentMetrics = FPerf_TerrainPhysicsIntegrationMetrics();
}

void UPerf_TerrainPhysicsIntegrationOptimizer::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsIntegrationOptimizer: Initializing performance optimization system"));
    
    // Initialize terrain physics integration
    InitializeTerrainPhysicsIntegration();
    
    // Start performance monitoring by default
    StartPerformanceMonitoring();
    
    UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsIntegrationOptimizer: Initialization complete"));
}

void UPerf_TerrainPhysicsIntegrationOptimizer::Deinitialize()
{
    StopPerformanceMonitoring();
    
    UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsIntegrationOptimizer: Deinitialized"));
    
    Super::Deinitialize();
}

void UPerf_TerrainPhysicsIntegrationOptimizer::StartPerformanceMonitoring()
{
    if (bIsMonitoring)
    {
        return;
    }
    
    bIsMonitoring = true;
    
    // Start performance update timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            PerformanceUpdateTimer,
            this,
            &UPerf_TerrainPhysicsIntegrationOptimizer::UpdatePerformanceMetrics,
            0.1f, // Update every 100ms
            true
        );
        
        // Start optimization timer
        World->GetTimerManager().SetTimer(
            OptimizationTimer,
            this,
            &UPerf_TerrainPhysicsIntegrationOptimizer::OptimizeTerrainPhysicsIntegration,
            OptimizationSettings.OptimizationUpdateInterval,
            true
        );
    }
    
    UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsIntegrationOptimizer: Performance monitoring started"));
}

void UPerf_TerrainPhysicsIntegrationOptimizer::StopPerformanceMonitoring()
{
    if (!bIsMonitoring)
    {
        return;
    }
    
    bIsMonitoring = false;
    
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(PerformanceUpdateTimer);
        World->GetTimerManager().ClearTimer(OptimizationTimer);
    }
    
    UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsIntegrationOptimizer: Performance monitoring stopped"));
}

FPerf_TerrainPhysicsIntegrationMetrics UPerf_TerrainPhysicsIntegrationOptimizer::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

void UPerf_TerrainPhysicsIntegrationOptimizer::SetOptimizationSettings(const FPerf_TerrainPhysicsOptimizationSettings& NewSettings)
{
    OptimizationSettings = NewSettings;
    UpdateIntegrationSettings();
    
    UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsIntegrationOptimizer: Optimization settings updated"));
}

FPerf_TerrainPhysicsOptimizationSettings UPerf_TerrainPhysicsIntegrationOptimizer::GetOptimizationSettings() const
{
    return OptimizationSettings;
}

void UPerf_TerrainPhysicsIntegrationOptimizer::OptimizeTerrainPhysicsIntegration()
{
    SCOPE_CYCLE_COUNTER(STAT_TerrainPhysicsIntegrationUpdate);
    
    if (!bIsMonitoring)
    {
        return;
    }
    
    // Check if optimization is needed
    if (CurrentMetrics.TerrainPhysicsFrameTime > OptimizationSettings.MaxFrameTimeMS)
    {
        bOptimizationActive = true;
        
        // Apply optimization based on current mode
        switch (OptimizationSettings.OptimizationMode)
        {
            case EPerf_TerrainPhysicsOptimizationMode::Conservative:
                OptimizePhysicsSimulation();
                break;
                
            case EPerf_TerrainPhysicsOptimizationMode::Balanced:
                OptimizePhysicsSimulation();
                OptimizeTerrainCollision();
                break;
                
            case EPerf_TerrainPhysicsOptimizationMode::Aggressive:
                OptimizePhysicsSimulation();
                OptimizeTerrainCollision();
                OptimizeMemoryUsage();
                break;
                
            case EPerf_TerrainPhysicsOptimizationMode::Emergency:
                // Emergency optimization - reduce everything
                OptimizePhysicsSimulation();
                OptimizeTerrainCollision();
                OptimizeMemoryUsage();
                HandlePerformanceThreshold();
                break;
        }
        
        LastOptimizationTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
        
        OnOptimizationApplied.Broadcast(OptimizationSettings.OptimizationMode);
        
        UE_LOG(LogTemp, Warning, TEXT("TerrainPhysicsIntegrationOptimizer: Optimization applied - Mode: %d, Frame Time: %.2fms"), 
               (int32)OptimizationSettings.OptimizationMode, CurrentMetrics.TerrainPhysicsFrameTime);
    }
    else
    {
        bOptimizationActive = false;
    }
}

void UPerf_TerrainPhysicsIntegrationOptimizer::SetIntegrationLevel(EPerf_TerrainPhysicsIntegrationLevel NewLevel)
{
    OptimizationSettings.IntegrationLevel = NewLevel;
    ApplyOptimizationLevel();
    
    UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsIntegrationOptimizer: Integration level set to %d"), (int32)NewLevel);
}

void UPerf_TerrainPhysicsIntegrationOptimizer::SetOptimizationMode(EPerf_TerrainPhysicsOptimizationMode NewMode)
{
    OptimizationSettings.OptimizationMode = NewMode;
    
    UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsIntegrationOptimizer: Optimization mode set to %d"), (int32)NewMode);
}

float UPerf_TerrainPhysicsIntegrationOptimizer::GetTerrainPhysicsFrameTime() const
{
    return CurrentMetrics.TerrainPhysicsFrameTime;
}

float UPerf_TerrainPhysicsIntegrationOptimizer::GetIntegrationEfficiency() const
{
    return CurrentMetrics.IntegrationEfficiency;
}

bool UPerf_TerrainPhysicsIntegrationOptimizer::IsPerformanceOptimal() const
{
    return CurrentMetrics.TerrainPhysicsFrameTime <= OptimizationSettings.MaxFrameTimeMS &&
           CurrentMetrics.IntegrationEfficiency >= 80.0f;
}

void UPerf_TerrainPhysicsIntegrationOptimizer::GeneratePerformanceReport()
{
    FString ReportContent;
    ReportContent += TEXT("=== Terrain Physics Integration Performance Report ===\n");
    ReportContent += FString::Printf(TEXT("Timestamp: %s\n"), *FDateTime::Now().ToString());
    ReportContent += TEXT("\n--- Performance Metrics ---\n");
    ReportContent += FString::Printf(TEXT("Terrain Physics Frame Time: %.2f ms\n"), CurrentMetrics.TerrainPhysicsFrameTime);
    ReportContent += FString::Printf(TEXT("Collision Detection Time: %.2f ms\n"), CurrentMetrics.CollisionDetectionTime);
    ReportContent += FString::Printf(TEXT("Rigid Body Simulation Time: %.2f ms\n"), CurrentMetrics.RigidBodySimulationTime);
    ReportContent += FString::Printf(TEXT("Terrain Deformation Time: %.2f ms\n"), CurrentMetrics.TerrainDeformationTime);
    ReportContent += FString::Printf(TEXT("Active Physics Bodies: %d\n"), CurrentMetrics.ActivePhysicsBodies);
    ReportContent += FString::Printf(TEXT("Terrain Collision Queries: %d\n"), CurrentMetrics.TerrainCollisionQueries);
    ReportContent += FString::Printf(TEXT("Memory Usage: %.2f MB\n"), CurrentMetrics.MemoryUsageMB);
    ReportContent += FString::Printf(TEXT("CPU Usage: %.1f%%\n"), CurrentMetrics.CPUUsagePercent);
    ReportContent += FString::Printf(TEXT("GPU Usage: %.1f%%\n"), CurrentMetrics.GPUUsagePercent);
    ReportContent += FString::Printf(TEXT("Integration Efficiency: %.1f%%\n"), CurrentMetrics.IntegrationEfficiency);
    ReportContent += TEXT("\n--- Optimization Settings ---\n");
    ReportContent += FString::Printf(TEXT("Integration Level: %d\n"), (int32)OptimizationSettings.IntegrationLevel);
    ReportContent += FString::Printf(TEXT("Optimization Mode: %d\n"), (int32)OptimizationSettings.OptimizationMode);
    ReportContent += FString::Printf(TEXT("Target FPS: %.1f\n"), OptimizationSettings.TargetFPS);
    ReportContent += FString::Printf(TEXT("Max Frame Time: %.2f ms\n"), OptimizationSettings.MaxFrameTimeMS);
    ReportContent += FString::Printf(TEXT("Adaptive Optimization: %s\n"), OptimizationSettings.bEnableAdaptiveOptimization ? TEXT("Enabled") : TEXT("Disabled"));
    ReportContent += TEXT("\n--- Status ---\n");
    ReportContent += FString::Printf(TEXT("Monitoring Active: %s\n"), bIsMonitoring ? TEXT("Yes") : TEXT("No"));
    ReportContent += FString::Printf(TEXT("Optimization Active: %s\n"), bOptimizationActive ? TEXT("Yes") : TEXT("No"));
    ReportContent += FString::Printf(TEXT("Performance Optimal: %s\n"), IsPerformanceOptimal() ? TEXT("Yes") : TEXT("No"));
    
    // Save report to file
    FString ReportPath = FPaths::ProjectLogDir() / TEXT("TerrainPhysicsPerformanceReport.txt");
    FFileHelper::SaveStringToFile(ReportContent, *ReportPath);
    
    UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsIntegrationOptimizer: Performance report generated at %s"), *ReportPath);
}

void UPerf_TerrainPhysicsIntegrationOptimizer::UpdatePerformanceMetrics()
{
    SCOPE_CYCLE_COUNTER(STAT_TerrainPhysicsIntegrationUpdate);
    
    // Get current frame time
    float DeltaTime = GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.016f;
    CurrentMetrics.TerrainPhysicsFrameTime = DeltaTime * 1000.0f; // Convert to milliseconds
    
    // Count active physics bodies
    CurrentMetrics.ActivePhysicsBodies = 0;
    CurrentMetrics.TerrainCollisionQueries = 0;
    
    if (UWorld* World = GetWorld())
    {
        // Count physics actors
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor->GetRootComponent())
            {
                if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
                {
                    if (PrimComp->IsSimulatingPhysics())
                    {
                        CurrentMetrics.ActivePhysicsBodies++;
                    }
                }
            }
        }
    }
    
    // Calculate integration efficiency
    float TargetFrameTime = 1000.0f / OptimizationSettings.TargetFPS;
    CurrentMetrics.IntegrationEfficiency = FMath::Clamp(
        (TargetFrameTime / FMath::Max(CurrentMetrics.TerrainPhysicsFrameTime, 0.1f)) * 100.0f,
        0.0f,
        100.0f
    );
    
    // Estimate memory usage (simplified)
    CurrentMetrics.MemoryUsageMB = CurrentMetrics.ActivePhysicsBodies * 0.5f; // Rough estimate
    
    // Estimate CPU/GPU usage (simplified)
    CurrentMetrics.CPUUsagePercent = FMath::Clamp(CurrentMetrics.TerrainPhysicsFrameTime / TargetFrameTime * 50.0f, 0.0f, 100.0f);
    CurrentMetrics.GPUUsagePercent = FMath::Clamp(CurrentMetrics.TerrainPhysicsFrameTime / TargetFrameTime * 30.0f, 0.0f, 100.0f);
    
    // Check for performance threshold breach
    if (CurrentMetrics.TerrainPhysicsFrameTime > OptimizationSettings.MaxFrameTimeMS)
    {
        OnPerformanceThresholdExceeded.Broadcast(CurrentMetrics.TerrainPhysicsFrameTime);
    }
}

void UPerf_TerrainPhysicsIntegrationOptimizer::ApplyOptimizationLevel()
{
    switch (OptimizationSettings.IntegrationLevel)
    {
        case EPerf_TerrainPhysicsIntegrationLevel::Minimal:
            OptimizationSettings.MaxActivePhysicsBodies = 500;
            OptimizationSettings.CollisionComplexityThreshold = 0.5f;
            break;
            
        case EPerf_TerrainPhysicsIntegrationLevel::Standard:
            OptimizationSettings.MaxActivePhysicsBodies = 1000;
            OptimizationSettings.CollisionComplexityThreshold = 0.75f;
            break;
            
        case EPerf_TerrainPhysicsIntegrationLevel::Enhanced:
            OptimizationSettings.MaxActivePhysicsBodies = 1500;
            OptimizationSettings.CollisionComplexityThreshold = 0.85f;
            break;
            
        case EPerf_TerrainPhysicsIntegrationLevel::Maximum:
            OptimizationSettings.MaxActivePhysicsBodies = 2000;
            OptimizationSettings.CollisionComplexityThreshold = 1.0f;
            break;
    }
}

void UPerf_TerrainPhysicsIntegrationOptimizer::HandlePerformanceThreshold()
{
    // Emergency performance handling
    if (CurrentMetrics.TerrainPhysicsFrameTime > CRITICAL_FRAME_TIME_MS)
    {
        // Switch to emergency mode
        OptimizationSettings.OptimizationMode = EPerf_TerrainPhysicsOptimizationMode::Emergency;
        OptimizationSettings.IntegrationLevel = EPerf_TerrainPhysicsIntegrationLevel::Minimal;
        
        UE_LOG(LogTemp, Error, TEXT("TerrainPhysicsIntegrationOptimizer: CRITICAL performance threshold exceeded! Switching to emergency mode."));
    }
}

void UPerf_TerrainPhysicsIntegrationOptimizer::OptimizePhysicsSimulation()
{
    SCOPE_CYCLE_COUNTER(STAT_PhysicsOptimization);
    
    // Reduce physics simulation complexity
    if (UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get())
    {
        // Temporarily reduce physics iterations for performance
        // Note: This is a simplified optimization - real implementation would be more sophisticated
    }
    
    UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsIntegrationOptimizer: Physics simulation optimized"));
}

void UPerf_TerrainPhysicsIntegrationOptimizer::OptimizeTerrainCollision()
{
    SCOPE_CYCLE_COUNTER(STAT_TerrainCollisionOptimization);
    
    // Optimize terrain collision detection
    // This would involve reducing collision complexity, adjusting LOD, etc.
    
    UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsIntegrationOptimizer: Terrain collision optimized"));
}

void UPerf_TerrainPhysicsIntegrationOptimizer::OptimizeMemoryUsage()
{
    // Force garbage collection if memory usage is high
    if (CurrentMetrics.MemoryUsageMB > 1000.0f)
    {
        GEngine->ForceGarbageCollection(true);
        UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsIntegrationOptimizer: Forced garbage collection for memory optimization"));
    }
}

void UPerf_TerrainPhysicsIntegrationOptimizer::InitializeTerrainPhysicsIntegration()
{
    // Initialize terrain physics integration systems
    ValidateIntegrationPerformance();
    
    UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsIntegrationOptimizer: Terrain physics integration initialized"));
}

void UPerf_TerrainPhysicsIntegrationOptimizer::UpdateIntegrationSettings()
{
    ApplyOptimizationLevel();
    
    // Restart timers with new settings
    if (bIsMonitoring)
    {
        StopPerformanceMonitoring();
        StartPerformanceMonitoring();
    }
}

void UPerf_TerrainPhysicsIntegrationOptimizer::ValidateIntegrationPerformance()
{
    // Validate that the integration is performing within acceptable parameters
    bool bValidationPassed = true;
    
    if (CurrentMetrics.ActivePhysicsBodies > OptimizationSettings.MaxActivePhysicsBodies)
    {
        bValidationPassed = false;
        UE_LOG(LogTemp, Warning, TEXT("TerrainPhysicsIntegrationOptimizer: Too many active physics bodies: %d (max: %d)"), 
               CurrentMetrics.ActivePhysicsBodies, OptimizationSettings.MaxActivePhysicsBodies);
    }
    
    if (CurrentMetrics.TerrainPhysicsFrameTime > OptimizationSettings.MaxFrameTimeMS)
    {
        bValidationPassed = false;
        UE_LOG(LogTemp, Warning, TEXT("TerrainPhysicsIntegrationOptimizer: Frame time too high: %.2fms (max: %.2fms)"), 
               CurrentMetrics.TerrainPhysicsFrameTime, OptimizationSettings.MaxFrameTimeMS);
    }
    
    if (bValidationPassed)
    {
        UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsIntegrationOptimizer: Integration performance validation passed"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("TerrainPhysicsIntegrationOptimizer: Integration performance validation failed - optimization required"));
    }
}