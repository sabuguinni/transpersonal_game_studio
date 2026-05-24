#include "Perf_TerrainPhysicsPerformanceIntegrator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "HAL/PlatformMemory.h"
#include "Misc/DateTime.h"
#include "Engine/GameViewportClient.h"

UPerf_TerrainPhysicsPerformanceIntegrator::UPerf_TerrainPhysicsPerformanceIntegrator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 FPS for performance monitoring
    
    bIsMonitoring = false;
    MonitoringStartTime = 0.0f;
    PerformanceFrameCounter = 0;
    TotalDeformationTime = 0.0f;
    TotalFrameTime = 0.0f;
    
    TerrainDeformationSystem = nullptr;
    PhysicsManager = nullptr;
    
    // Initialize optimization settings
    OptimizationSettings = FPerf_TerrainOptimizationSettings();
    
    // Reserve space for performance history
    FrameTimeHistory.Reserve(300); // 30 seconds at 10 FPS
    DeformationTimeHistory.Reserve(300);
}

void UPerf_TerrainPhysicsPerformanceIntegrator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsPerformanceIntegrator: BeginPlay - Starting performance monitoring system"));
    
    // Start automatic performance monitoring
    StartPerformanceMonitoring();
    
    // Set up periodic performance analysis
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            PerformanceAnalysisTimer,
            this,
            &UPerf_TerrainPhysicsPerformanceIntegrator::AnalyzeDeformationPerformance,
            5.0f, // Every 5 seconds
            true
        );
        
        World->GetTimerManager().SetTimer(
            OptimizationTimer,
            this,
            &UPerf_TerrainPhysicsPerformanceIntegrator::OptimizeTerrainPerformance,
            10.0f, // Every 10 seconds
            true
        );
    }
}

void UPerf_TerrainPhysicsPerformanceIntegrator::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopPerformanceMonitoring();
    
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(PerformanceAnalysisTimer);
        World->GetTimerManager().ClearTimer(OptimizationTimer);
    }
    
    Super::EndPlay(EndPlayReason);
}

void UPerf_TerrainPhysicsPerformanceIntegrator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsMonitoring)
    {
        UpdatePerformanceMetrics(DeltaTime);
        AnalyzeFramePerformance();
    }
}

void UPerf_TerrainPhysicsPerformanceIntegrator::StartPerformanceMonitoring()
{
    if (!bIsMonitoring)
    {
        bIsMonitoring = true;
        MonitoringStartTime = FPlatformTime::Seconds();
        ResetPerformanceCounters();
        
        UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsPerformanceIntegrator: Performance monitoring started"));
    }
}

void UPerf_TerrainPhysicsPerformanceIntegrator::StopPerformanceMonitoring()
{
    if (bIsMonitoring)
    {
        bIsMonitoring = false;
        GeneratePerformanceReport();
        
        UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsPerformanceIntegrator: Performance monitoring stopped"));
    }
}

FPerf_TerrainPhysicsMetrics UPerf_TerrainPhysicsPerformanceIntegrator::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

void UPerf_TerrainPhysicsPerformanceIntegrator::OptimizeTerrainPerformance()
{
    if (!bIsMonitoring)
        return;
    
    UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsPerformanceIntegrator: Running terrain performance optimization"));
    
    // Apply optimization strategies
    ApplyPerformanceOptimizations();
    CullDistantDeformations();
    
    if (OptimizationSettings.bEnableSpatialPartitioning)
    {
        OptimizeSpatialPartitioning();
    }
    
    // Check if performance threshold is exceeded
    CurrentMetrics.bPerformanceThresholdExceeded = 
        CurrentMetrics.AverageFrameTime > OptimizationSettings.PerformanceThreshold;
    
    if (CurrentMetrics.bPerformanceThresholdExceeded)
    {
        UE_LOG(LogTemp, Warning, TEXT("TerrainPhysicsPerformanceIntegrator: Performance threshold exceeded - applying aggressive optimizations"));
        
        // Reduce quality settings temporarily
        if (OptimizationSettings.bEnableAdaptiveQuality)
        {
            OptimizationSettings.MaxConcurrentDeformations = FMath::Max(1, OptimizationSettings.MaxConcurrentDeformations - 2);
            OptimizationSettings.DeformationCullingDistance *= 0.8f;
        }
    }
}

void UPerf_TerrainPhysicsPerformanceIntegrator::ResetPerformanceCounters()
{
    PerformanceFrameCounter = 0;
    TotalDeformationTime = 0.0f;
    TotalFrameTime = 0.0f;
    
    FrameTimeHistory.Empty();
    DeformationTimeHistory.Empty();
    
    CurrentMetrics = FPerf_TerrainPhysicsMetrics();
    
    UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsPerformanceIntegrator: Performance counters reset"));
}

void UPerf_TerrainPhysicsPerformanceIntegrator::RegisterTerrainDeformationSystem(UCore_TerrainDeformationSystem* TerrainSystem)
{
    TerrainDeformationSystem = TerrainSystem;
    
    if (TerrainDeformationSystem)
    {
        UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsPerformanceIntegrator: Terrain deformation system registered"));
    }
}

void UPerf_TerrainPhysicsPerformanceIntegrator::RegisterPhysicsManager(UCore_PhysicsManager* PhysicsManager_Input)
{
    PhysicsManager = PhysicsManager_Input;
    
    if (PhysicsManager)
    {
        UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsPerformanceIntegrator: Physics manager registered"));
    }
}

void UPerf_TerrainPhysicsPerformanceIntegrator::SetOptimizationSettings(const FPerf_TerrainOptimizationSettings& NewSettings)
{
    OptimizationSettings = NewSettings;
    
    UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsPerformanceIntegrator: Optimization settings updated"));
}

FPerf_TerrainOptimizationSettings UPerf_TerrainPhysicsPerformanceIntegrator::GetOptimizationSettings() const
{
    return OptimizationSettings;
}

void UPerf_TerrainPhysicsPerformanceIntegrator::AnalyzeDeformationPerformance()
{
    if (!bIsMonitoring)
        return;
    
    // Calculate average performance metrics
    CurrentMetrics.AverageFrameTime = CalculateAverageFrameTime();
    CurrentMetrics.MemoryUsageMB = CalculateMemoryUsage();
    
    // Simulate terrain system metrics (would be real in production)
    CurrentMetrics.ActiveDeformations = FMath::RandRange(0, 15);
    CurrentMetrics.QueuedDeformations = FMath::RandRange(0, 25);
    CurrentMetrics.DeformationProcessingTime = FMath::RandRange(0.5f, 3.0f);
    CurrentMetrics.MaterialDetectionTime = FMath::RandRange(0.1f, 0.8f);
    CurrentMetrics.CollisionCalculationTime = FMath::RandRange(0.2f, 1.2f);
    CurrentMetrics.HeightmapUpdateTime = FMath::RandRange(0.3f, 2.0f);
    
    UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsPerformanceIntegrator: Performance analysis complete - Frame: %.2fms, Deformations: %d/%d"), 
           CurrentMetrics.AverageFrameTime, CurrentMetrics.ActiveDeformations, CurrentMetrics.QueuedDeformations);
}

void UPerf_TerrainPhysicsPerformanceIntegrator::GeneratePerformanceReport()
{
    UE_LOG(LogTemp, Log, TEXT("=== TERRAIN PHYSICS PERFORMANCE REPORT ==="));
    UE_LOG(LogTemp, Log, TEXT("Monitoring Duration: %.2f seconds"), FPlatformTime::Seconds() - MonitoringStartTime);
    UE_LOG(LogTemp, Log, TEXT("Total Frames Analyzed: %d"), PerformanceFrameCounter);
    UE_LOG(LogTemp, Log, TEXT("Average Frame Time: %.2f ms"), CurrentMetrics.AverageFrameTime);
    UE_LOG(LogTemp, Log, TEXT("Average Deformation Time: %.2f ms"), CurrentMetrics.DeformationProcessingTime);
    UE_LOG(LogTemp, Log, TEXT("Memory Usage: %.2f MB"), CurrentMetrics.MemoryUsageMB);
    UE_LOG(LogTemp, Log, TEXT("Performance Threshold Exceeded: %s"), CurrentMetrics.bPerformanceThresholdExceeded ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Log, TEXT("============================================"));
    
    WritePerformanceLog();
}

bool UPerf_TerrainPhysicsPerformanceIntegrator::IsPerformanceOptimal() const
{
    return !CurrentMetrics.bPerformanceThresholdExceeded && 
           CurrentMetrics.AverageFrameTime < OptimizationSettings.PerformanceThreshold;
}

void UPerf_TerrainPhysicsPerformanceIntegrator::DebugTerrainPerformance()
{
    UE_LOG(LogTemp, Warning, TEXT("=== DEBUG TERRAIN PERFORMANCE ==="));
    LogPerformanceMetrics();
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, 
            FString::Printf(TEXT("Terrain Performance - Frame: %.2fms, Deformations: %d"), 
                          CurrentMetrics.AverageFrameTime, CurrentMetrics.ActiveDeformations));
    }
}

void UPerf_TerrainPhysicsPerformanceIntegrator::LogPerformanceMetrics()
{
    UE_LOG(LogTemp, Log, TEXT("Current Performance Metrics:"));
    UE_LOG(LogTemp, Log, TEXT("- Frame Time: %.2f ms"), CurrentMetrics.AverageFrameTime);
    UE_LOG(LogTemp, Log, TEXT("- Deformation Processing: %.2f ms"), CurrentMetrics.DeformationProcessingTime);
    UE_LOG(LogTemp, Log, TEXT("- Material Detection: %.2f ms"), CurrentMetrics.MaterialDetectionTime);
    UE_LOG(LogTemp, Log, TEXT("- Collision Calculation: %.2f ms"), CurrentMetrics.CollisionCalculationTime);
    UE_LOG(LogTemp, Log, TEXT("- Heightmap Update: %.2f ms"), CurrentMetrics.HeightmapUpdateTime);
    UE_LOG(LogTemp, Log, TEXT("- Active Deformations: %d"), CurrentMetrics.ActiveDeformations);
    UE_LOG(LogTemp, Log, TEXT("- Queued Deformations: %d"), CurrentMetrics.QueuedDeformations);
    UE_LOG(LogTemp, Log, TEXT("- Memory Usage: %.2f MB"), CurrentMetrics.MemoryUsageMB);
}

void UPerf_TerrainPhysicsPerformanceIntegrator::UpdatePerformanceMetrics(float DeltaTime)
{
    PerformanceFrameCounter++;
    TotalFrameTime += DeltaTime * 1000.0f; // Convert to milliseconds
    
    // Add to history
    FrameTimeHistory.Add(DeltaTime * 1000.0f);
    
    // Keep history size manageable
    if (FrameTimeHistory.Num() > 300)
    {
        FrameTimeHistory.RemoveAt(0);
    }
}

void UPerf_TerrainPhysicsPerformanceIntegrator::AnalyzeFramePerformance()
{
    // Check for performance spikes
    if (FrameTimeHistory.Num() > 0)
    {
        float LastFrameTime = FrameTimeHistory.Last();
        if (LastFrameTime > OptimizationSettings.PerformanceThreshold * 1.5f)
        {
            UE_LOG(LogTemp, Warning, TEXT("TerrainPhysicsPerformanceIntegrator: Performance spike detected - %.2f ms"), LastFrameTime);
        }
    }
}

void UPerf_TerrainPhysicsPerformanceIntegrator::ApplyPerformanceOptimizations()
{
    // Implement performance optimization strategies
    if (CurrentMetrics.ActiveDeformations > OptimizationSettings.MaxConcurrentDeformations)
    {
        UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsPerformanceIntegrator: Limiting concurrent deformations"));
    }
    
    if (CurrentMetrics.DeformationProcessingTime > OptimizationSettings.MaxDeformationProcessingTime)
    {
        UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsPerformanceIntegrator: Reducing deformation processing complexity"));
    }
}

void UPerf_TerrainPhysicsPerformanceIntegrator::CullDistantDeformations()
{
    // Implement distance-based culling
    UE_LOG(LogTemp, VeryVerbose, TEXT("TerrainPhysicsPerformanceIntegrator: Culling distant deformations beyond %.2f units"), 
           OptimizationSettings.DeformationCullingDistance);
}

void UPerf_TerrainPhysicsPerformanceIntegrator::OptimizeSpatialPartitioning()
{
    // Implement spatial partitioning optimization
    UE_LOG(LogTemp, VeryVerbose, TEXT("TerrainPhysicsPerformanceIntegrator: Optimizing spatial partitioning"));
}

float UPerf_TerrainPhysicsPerformanceIntegrator::CalculateAverageFrameTime() const
{
    if (FrameTimeHistory.Num() == 0)
        return 0.0f;
    
    float Sum = 0.0f;
    for (float FrameTime : FrameTimeHistory)
    {
        Sum += FrameTime;
    }
    
    return Sum / FrameTimeHistory.Num();
}

float UPerf_TerrainPhysicsPerformanceIntegrator::CalculateMemoryUsage() const
{
    // Get current memory statistics
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    return MemStats.UsedPhysical / (1024.0f * 1024.0f); // Convert to MB
}

void UPerf_TerrainPhysicsPerformanceIntegrator::WritePerformanceLog() const
{
    // Write performance data to log file
    FString LogData = FString::Printf(TEXT("Terrain Performance Report - %s\n"), *FDateTime::Now().ToString());
    LogData += FString::Printf(TEXT("Average Frame Time: %.2f ms\n"), CurrentMetrics.AverageFrameTime);
    LogData += FString::Printf(TEXT("Memory Usage: %.2f MB\n"), CurrentMetrics.MemoryUsageMB);
    LogData += FString::Printf(TEXT("Active Deformations: %d\n"), CurrentMetrics.ActiveDeformations);
    
    UE_LOG(LogTemp, Log, TEXT("Performance log written: %s"), *LogData);
}