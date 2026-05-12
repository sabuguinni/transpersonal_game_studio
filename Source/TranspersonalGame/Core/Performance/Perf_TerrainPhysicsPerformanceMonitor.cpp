#include "Perf_TerrainPhysicsPerformanceMonitor.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/StatsHierarchical.h"
#include "PhysicsEngine/PhysicsSettings.h"

UPerf_TerrainPhysicsPerformanceMonitor::UPerf_TerrainPhysicsPerformanceMonitor()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second

    // Initialize default values
    CurrentPerformanceLevel = EPerf_TerrainPerformanceLevel::High;
    TargetFrameTime = 16.67f; // 60 FPS
    MaxAcceptableFrameTime = 33.33f; // 30 FPS
    bAutoOptimizationEnabled = true;
    bPerformanceAcceptable = true;
    AverageFrameTime = 16.67f;

    // Performance thresholds (in milliseconds)
    DeformationTimeThreshold = 2.0f;
    CollisionUpdateThreshold = 1.5f;
    MeshRebuildThreshold = 5.0f;
    MaxDeformationPoints = 1000;
    MaxPhysicsBodies = 500;

    // Initialize timing
    DeformationStartTime = 0.0;
    CollisionStartTime = 0.0;
    MeshRebuildStartTime = 0.0;

    // Frame time tracking
    FrameTimeHistory.SetNum(FrameTimeHistorySize);
    for (int32 i = 0; i < FrameTimeHistorySize; i++)
    {
        FrameTimeHistory[i] = TargetFrameTime;
    }
    FrameTimeHistoryIndex = 0;

    // Optimization state
    LastOptimizationTime = 0.0f;
    OptimizationCooldown = 5.0f; // 5 seconds between optimizations
}

void UPerf_TerrainPhysicsPerformanceMonitor::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsPerformanceMonitor: Initialized with %s performance level"), 
           *UEnum::GetValueAsString(CurrentPerformanceLevel));

    // Apply initial performance settings
    ApplyPerformanceLevel(CurrentPerformanceLevel);
}

void UPerf_TerrainPhysicsPerformanceMonitor::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update frame time tracking
    UpdateFrameTimeTracking(DeltaTime);

    // Collect performance metrics
    CollectMemoryMetrics();
    UpdatePhysicsBodyCount();

    // Check if performance optimization is needed
    CheckPerformanceThresholds();

    // Auto-optimization if enabled
    if (bAutoOptimizationEnabled && !bPerformanceAcceptable)
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - LastOptimizationTime > OptimizationCooldown)
        {
            TriggerPerformanceOptimization();
            LastOptimizationTime = CurrentTime;
        }
    }
}

void UPerf_TerrainPhysicsPerformanceMonitor::StartTerrainDeformationProfiling()
{
    SCOPE_CYCLE_COUNTER(STAT_TerrainDeformation);
    DeformationStartTime = FPlatformTime::Seconds();
}

void UPerf_TerrainPhysicsPerformanceMonitor::EndTerrainDeformationProfiling()
{
    if (DeformationStartTime > 0.0)
    {
        double EndTime = FPlatformTime::Seconds();
        CurrentMetrics.DeformationTime = (EndTime - DeformationStartTime) * 1000.0; // Convert to milliseconds
        DeformationStartTime = 0.0;
    }
}

void UPerf_TerrainPhysicsPerformanceMonitor::StartPhysicsCollisionProfiling()
{
    SCOPE_CYCLE_COUNTER(STAT_PhysicsCollisionUpdate);
    CollisionStartTime = FPlatformTime::Seconds();
}

void UPerf_TerrainPhysicsPerformanceMonitor::EndPhysicsCollisionProfiling()
{
    if (CollisionStartTime > 0.0)
    {
        double EndTime = FPlatformTime::Seconds();
        CurrentMetrics.CollisionUpdateTime = (EndTime - CollisionStartTime) * 1000.0; // Convert to milliseconds
        CollisionStartTime = 0.0;
    }
}

void UPerf_TerrainPhysicsPerformanceMonitor::StartMeshRebuildProfiling()
{
    SCOPE_CYCLE_COUNTER(STAT_TerrainMeshRebuild);
    MeshRebuildStartTime = FPlatformTime::Seconds();
}

void UPerf_TerrainPhysicsPerformanceMonitor::EndMeshRebuildProfiling()
{
    if (MeshRebuildStartTime > 0.0)
    {
        double EndTime = FPlatformTime::Seconds();
        CurrentMetrics.MeshRebuildTime = (EndTime - MeshRebuildStartTime) * 1000.0; // Convert to milliseconds
        MeshRebuildStartTime = 0.0;
    }
}

void UPerf_TerrainPhysicsPerformanceMonitor::SetTerrainPerformanceLevel(EPerf_TerrainPerformanceLevel Level)
{
    if (CurrentPerformanceLevel != Level)
    {
        CurrentPerformanceLevel = Level;
        ApplyPerformanceLevel(Level);
        
        UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsPerformanceMonitor: Performance level changed to %s"), 
               *UEnum::GetValueAsString(Level));
    }
}

void UPerf_TerrainPhysicsPerformanceMonitor::TriggerPerformanceOptimization()
{
    UE_LOG(LogTemp, Warning, TEXT("TerrainPhysicsPerformanceMonitor: Triggering performance optimization"));
    
    // Automatically downgrade performance level if needed
    if (CurrentPerformanceLevel == EPerf_TerrainPerformanceLevel::Ultra)
    {
        SetTerrainPerformanceLevel(EPerf_TerrainPerformanceLevel::High);
    }
    else if (CurrentPerformanceLevel == EPerf_TerrainPerformanceLevel::High)
    {
        SetTerrainPerformanceLevel(EPerf_TerrainPerformanceLevel::Medium);
    }
    else if (CurrentPerformanceLevel == EPerf_TerrainPerformanceLevel::Medium)
    {
        SetTerrainPerformanceLevel(EPerf_TerrainPerformanceLevel::Low);
    }
    else if (CurrentPerformanceLevel == EPerf_TerrainPerformanceLevel::Low)
    {
        SetTerrainPerformanceLevel(EPerf_TerrainPerformanceLevel::Minimal);
    }

    OptimizeTerrainPerformance();
}

void UPerf_TerrainPhysicsPerformanceMonitor::RunPerformanceBenchmark()
{
    UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsPerformanceMonitor: Running performance benchmark"));

    // Simulate terrain deformation operations
    StartTerrainDeformationProfiling();
    
    // Simulate some work (in a real scenario, this would be actual terrain deformation)
    for (int32 i = 0; i < 1000; i++)
    {
        float DummyValue = FMath::Sin(i * 0.1f) * FMath::Cos(i * 0.05f);
        (void)DummyValue; // Suppress unused variable warning
    }
    
    EndTerrainDeformationProfiling();

    // Simulate physics collision update
    StartPhysicsCollisionProfiling();
    
    for (int32 i = 0; i < 500; i++)
    {
        FVector DummyVector = FVector(i, i * 2, i * 3).GetSafeNormal();
        (void)DummyVector; // Suppress unused variable warning
    }
    
    EndPhysicsCollisionProfiling();

    // Simulate mesh rebuild
    StartMeshRebuildProfiling();
    
    for (int32 i = 0; i < 2000; i++)
    {
        FMatrix DummyMatrix = FMatrix::Identity;
        DummyMatrix.SetOrigin(FVector(i, i, i));
        (void)DummyMatrix; // Suppress unused variable warning
    }
    
    EndMeshRebuildProfiling();

    LogCurrentPerformanceState();
}

void UPerf_TerrainPhysicsPerformanceMonitor::LogCurrentPerformanceState()
{
    UE_LOG(LogTemp, Log, TEXT("=== Terrain Physics Performance State ==="));
    UE_LOG(LogTemp, Log, TEXT("Performance Level: %s"), *UEnum::GetValueAsString(CurrentPerformanceLevel));
    UE_LOG(LogTemp, Log, TEXT("Average Frame Time: %.2f ms"), AverageFrameTime);
    UE_LOG(LogTemp, Log, TEXT("Performance Acceptable: %s"), bPerformanceAcceptable ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Log, TEXT("Deformation Time: %.2f ms"), CurrentMetrics.DeformationTime);
    UE_LOG(LogTemp, Log, TEXT("Collision Update Time: %.2f ms"), CurrentMetrics.CollisionUpdateTime);
    UE_LOG(LogTemp, Log, TEXT("Mesh Rebuild Time: %.2f ms"), CurrentMetrics.MeshRebuildTime);
    UE_LOG(LogTemp, Log, TEXT("Active Deformation Points: %d"), CurrentMetrics.ActiveDeformationPoints);
    UE_LOG(LogTemp, Log, TEXT("Physics Body Count: %d"), CurrentMetrics.PhysicsBodyCount);
    UE_LOG(LogTemp, Log, TEXT("Memory Usage: %.2f MB"), CurrentMetrics.MemoryUsageMB);
    UE_LOG(LogTemp, Log, TEXT("=========================================="));
}

void UPerf_TerrainPhysicsPerformanceMonitor::UpdateFrameTimeTracking(float DeltaTime)
{
    float FrameTimeMs = DeltaTime * 1000.0f;
    
    // Add to circular buffer
    FrameTimeHistory[FrameTimeHistoryIndex] = FrameTimeMs;
    FrameTimeHistoryIndex = (FrameTimeHistoryIndex + 1) % FrameTimeHistorySize;

    // Calculate average
    float TotalTime = 0.0f;
    for (int32 i = 0; i < FrameTimeHistorySize; i++)
    {
        TotalTime += FrameTimeHistory[i];
    }
    AverageFrameTime = TotalTime / FrameTimeHistorySize;
}

void UPerf_TerrainPhysicsPerformanceMonitor::CheckPerformanceThresholds()
{
    // Check frame time performance
    bPerformanceAcceptable = AverageFrameTime <= MaxAcceptableFrameTime;

    // Check individual operation thresholds
    if (CurrentMetrics.DeformationTime > DeformationTimeThreshold ||
        CurrentMetrics.CollisionUpdateTime > CollisionUpdateThreshold ||
        CurrentMetrics.MeshRebuildTime > MeshRebuildThreshold ||
        CurrentMetrics.ActiveDeformationPoints > MaxDeformationPoints ||
        CurrentMetrics.PhysicsBodyCount > MaxPhysicsBodies)
    {
        bPerformanceAcceptable = false;
    }
}

void UPerf_TerrainPhysicsPerformanceMonitor::ApplyPerformanceLevel(EPerf_TerrainPerformanceLevel Level)
{
    switch (Level)
    {
        case EPerf_TerrainPerformanceLevel::Ultra:
            MaxDeformationPoints = 2000;
            MaxPhysicsBodies = 1000;
            DeformationTimeThreshold = 5.0f;
            CollisionUpdateThreshold = 3.0f;
            MeshRebuildThreshold = 10.0f;
            break;

        case EPerf_TerrainPerformanceLevel::High:
            MaxDeformationPoints = 1500;
            MaxPhysicsBodies = 750;
            DeformationTimeThreshold = 3.0f;
            CollisionUpdateThreshold = 2.0f;
            MeshRebuildThreshold = 7.0f;
            break;

        case EPerf_TerrainPerformanceLevel::Medium:
            MaxDeformationPoints = 1000;
            MaxPhysicsBodies = 500;
            DeformationTimeThreshold = 2.0f;
            CollisionUpdateThreshold = 1.5f;
            MeshRebuildThreshold = 5.0f;
            break;

        case EPerf_TerrainPerformanceLevel::Low:
            MaxDeformationPoints = 500;
            MaxPhysicsBodies = 250;
            DeformationTimeThreshold = 1.0f;
            CollisionUpdateThreshold = 0.8f;
            MeshRebuildThreshold = 3.0f;
            break;

        case EPerf_TerrainPerformanceLevel::Minimal:
            MaxDeformationPoints = 200;
            MaxPhysicsBodies = 100;
            DeformationTimeThreshold = 0.5f;
            CollisionUpdateThreshold = 0.4f;
            MeshRebuildThreshold = 1.5f;
            break;
    }
}

void UPerf_TerrainPhysicsPerformanceMonitor::OptimizeTerrainPerformance()
{
    // Reduce active deformation points if over threshold
    if (CurrentMetrics.ActiveDeformationPoints > MaxDeformationPoints)
    {
        CurrentMetrics.ActiveDeformationPoints = MaxDeformationPoints;
        UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsPerformanceMonitor: Reduced deformation points to %d"), MaxDeformationPoints);
    }

    // Optimize physics bodies if over threshold
    if (CurrentMetrics.PhysicsBodyCount > MaxPhysicsBodies)
    {
        // In a real implementation, this would disable or simplify physics bodies
        UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsPerformanceMonitor: Physics body optimization needed (current: %d, max: %d)"), 
               CurrentMetrics.PhysicsBodyCount, MaxPhysicsBodies);
    }
}

void UPerf_TerrainPhysicsPerformanceMonitor::CollectMemoryMetrics()
{
    // Get basic memory stats
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
}

void UPerf_TerrainPhysicsPerformanceMonitor::UpdatePhysicsBodyCount()
{
    // In a real implementation, this would count actual physics bodies in the world
    // For now, simulate based on world complexity
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        
        // Estimate physics bodies (not all actors have physics)
        CurrentMetrics.PhysicsBodyCount = AllActors.Num() / 4; // Rough estimate
        CurrentMetrics.ActiveDeformationPoints = FMath::Min(CurrentMetrics.PhysicsBodyCount * 2, MaxDeformationPoints);
    }
}