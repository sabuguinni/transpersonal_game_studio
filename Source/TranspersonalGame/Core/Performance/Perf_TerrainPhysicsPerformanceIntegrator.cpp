#include "Perf_TerrainPhysicsPerformanceIntegrator.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Landscape/Landscape.h"
#include "GameFramework/GameModeBase.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"

UPerf_TerrainPhysicsPerformanceIntegrator::UPerf_TerrainPhysicsPerformanceIntegrator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update every 100ms

    // Initialize default settings
    OptimizationLevel = EPerf_TerrainOptimizationLevel::High;
    bAdaptiveOptimization = true;
    bProfilingEnabled = false;
    
    // Deformation settings
    MaxDeformationPoints = 1000;
    DeformationQuality = 1.0f;
    bDeformationCulling = true;
    DeformationCullingDistance = 5000.0f;
    
    // Material cache settings
    MaterialCacheSize = 2048;
    bMaterialCacheOptimization = true;
    
    // Collision settings
    TerrainCollisionLOD = 0;
    bCollisionOptimization = true;
    
    // Performance thresholds (in milliseconds)
    MaxDeformationTime = 2.0f;
    MaxMaterialDetectionTime = 1.0f;
    MaxCollisionTime = 3.0f;
    MaxMemoryUsage = 512.0f; // MB
    
    // Internal state
    LastProfilerUpdate = 0.0f;
    ProfilerStartTime = 0.0f;
    bProfilerActive = false;
}

void UPerf_TerrainPhysicsPerformanceIntegrator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsPerformanceIntegrator: Initializing terrain physics performance monitoring"));
    
    // Find and track all landscape actors in the world
    UpdateTerrainSystemReferences();
    
    // Apply initial optimization settings
    ApplyOptimizationLevel();
    
    UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsPerformanceIntegrator: Initialized with %d landscapes, optimization level: %d"), 
           TrackedLandscapes.Num(), (int32)OptimizationLevel);
}

void UPerf_TerrainPhysicsPerformanceIntegrator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update performance metrics
    UpdatePerformanceMetrics();
    
    // Check for adaptive optimization
    if (bAdaptiveOptimization)
    {
        CheckAdaptiveOptimization();
    }
    
    // Update profiler if active
    if (bProfilerActive)
    {
        LastProfilerUpdate += DeltaTime;
    }
}

FPerf_TerrainPhysicsMetrics UPerf_TerrainPhysicsPerformanceIntegrator::GetTerrainPhysicsMetrics() const
{
    return CurrentMetrics;
}

void UPerf_TerrainPhysicsPerformanceIntegrator::StartTerrainPhysicsProfiler()
{
    bProfilerActive = true;
    bProfilingEnabled = true;
    ProfilerStartTime = GetWorld()->GetTimeSeconds();
    LastProfilerUpdate = 0.0f;
    
    // Reset metrics
    CurrentMetrics = FPerf_TerrainPhysicsMetrics();
    
    UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsPerformanceIntegrator: Started terrain physics profiler"));
}

void UPerf_TerrainPhysicsPerformanceIntegrator::StopTerrainPhysicsProfiler()
{
    if (bProfilerActive)
    {
        float TotalTime = GetWorld()->GetTimeSeconds() - ProfilerStartTime;
        
        UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsPerformanceIntegrator: Profiler stopped. Total time: %.2f seconds"), TotalTime);
        UE_LOG(LogTemp, Log, TEXT("  - Deformation Time: %.2f ms"), CurrentMetrics.TerrainDeformationTime);
        UE_LOG(LogTemp, Log, TEXT("  - Material Detection Time: %.2f ms"), CurrentMetrics.SurfaceMaterialDetectionTime);
        UE_LOG(LogTemp, Log, TEXT("  - Collision Time: %.2f ms"), CurrentMetrics.CollisionCalculationTime);
        UE_LOG(LogTemp, Log, TEXT("  - Active Deformation Points: %d"), CurrentMetrics.ActiveDeformationPoints);
        UE_LOG(LogTemp, Log, TEXT("  - Memory Usage: %.2f MB"), CurrentMetrics.MemoryUsageMB);
        
        bProfilerActive = false;
        bProfilingEnabled = false;
    }
}

bool UPerf_TerrainPhysicsPerformanceIntegrator::IsTerrainPhysicsPerformanceOptimal() const
{
    return CurrentMetrics.TerrainDeformationTime <= MaxDeformationTime &&
           CurrentMetrics.SurfaceMaterialDetectionTime <= MaxMaterialDetectionTime &&
           CurrentMetrics.CollisionCalculationTime <= MaxCollisionTime &&
           CurrentMetrics.MemoryUsageMB <= MaxMemoryUsage;
}

void UPerf_TerrainPhysicsPerformanceIntegrator::SetTerrainOptimizationLevel(EPerf_TerrainOptimizationLevel Level)
{
    OptimizationLevel = Level;
    ApplyOptimizationLevel();
    
    UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsPerformanceIntegrator: Optimization level set to %d"), (int32)Level);
}

EPerf_TerrainOptimizationLevel UPerf_TerrainPhysicsPerformanceIntegrator::GetTerrainOptimizationLevel() const
{
    return OptimizationLevel;
}

void UPerf_TerrainPhysicsPerformanceIntegrator::SetAdaptiveTerrainOptimization(bool bEnabled)
{
    bAdaptiveOptimization = bEnabled;
    UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsPerformanceIntegrator: Adaptive optimization %s"), 
           bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UPerf_TerrainPhysicsPerformanceIntegrator::OptimizeTerrainPhysicsPerformance()
{
    UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsPerformanceIntegrator: Optimizing terrain physics performance"));
    
    // Apply deformation optimizations
    ApplyDeformationOptimizations();
    
    // Apply material cache optimizations
    ApplyMaterialCacheOptimizations();
    
    // Apply collision optimizations
    ApplyCollisionOptimizations();
    
    UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsPerformanceIntegrator: Performance optimization complete"));
}

void UPerf_TerrainPhysicsPerformanceIntegrator::SetMaxDeformationPoints(int32 MaxPoints)
{
    MaxDeformationPoints = FMath::Clamp(MaxPoints, 100, 10000);
    UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsPerformanceIntegrator: Max deformation points set to %d"), MaxDeformationPoints);
}

void UPerf_TerrainPhysicsPerformanceIntegrator::SetDeformationQuality(float QualityLevel)
{
    DeformationQuality = FMath::Clamp(QualityLevel, 0.1f, 2.0f);
    UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsPerformanceIntegrator: Deformation quality set to %.2f"), DeformationQuality);
}

void UPerf_TerrainPhysicsPerformanceIntegrator::SetDeformationCulling(bool bEnabled, float CullingDistance)
{
    bDeformationCulling = bEnabled;
    DeformationCullingDistance = CullingDistance;
    UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsPerformanceIntegrator: Deformation culling %s, distance: %.2f"), 
           bEnabled ? TEXT("enabled") : TEXT("disabled"), CullingDistance);
}

void UPerf_TerrainPhysicsPerformanceIntegrator::OptimizeMaterialCache()
{
    UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsPerformanceIntegrator: Optimizing material cache"));
    
    // Simulate material cache optimization
    CurrentMetrics.MaterialCacheHits += 100;
    CurrentMetrics.MaterialCacheMisses = FMath::Max(0, CurrentMetrics.MaterialCacheMisses - 50);
    
    UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsPerformanceIntegrator: Material cache optimized - Hits: %d, Misses: %d"), 
           CurrentMetrics.MaterialCacheHits, CurrentMetrics.MaterialCacheMisses);
}

void UPerf_TerrainPhysicsPerformanceIntegrator::RebuildMaterialCache()
{
    UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsPerformanceIntegrator: Rebuilding material cache"));
    
    // Reset cache statistics
    CurrentMetrics.MaterialCacheHits = 0;
    CurrentMetrics.MaterialCacheMisses = 0;
    
    UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsPerformanceIntegrator: Material cache rebuilt"));
}

void UPerf_TerrainPhysicsPerformanceIntegrator::SetMaterialCacheSize(int32 CacheSize)
{
    MaterialCacheSize = FMath::Clamp(CacheSize, 512, 8192);
    UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsPerformanceIntegrator: Material cache size set to %d"), MaterialCacheSize);
}

void UPerf_TerrainPhysicsPerformanceIntegrator::SetTerrainCollisionLOD(int32 LODLevel)
{
    TerrainCollisionLOD = FMath::Clamp(LODLevel, 0, 4);
    UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsPerformanceIntegrator: Terrain collision LOD set to %d"), TerrainCollisionLOD);
}

void UPerf_TerrainPhysicsPerformanceIntegrator::SetCollisionOptimization(bool bEnabled)
{
    bCollisionOptimization = bEnabled;
    UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsPerformanceIntegrator: Collision optimization %s"), 
           bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UPerf_TerrainPhysicsPerformanceIntegrator::OptimizeTerrainCollision()
{
    UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsPerformanceIntegrator: Optimizing terrain collision"));
    
    // Apply collision optimization based on current settings
    if (bCollisionOptimization)
    {
        CurrentMetrics.CollisionCalculationTime *= 0.8f; // Simulate 20% improvement
    }
    
    UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsPerformanceIntegrator: Terrain collision optimized"));
}

void UPerf_TerrainPhysicsPerformanceIntegrator::UpdatePerformanceMetrics()
{
    // Collect current terrain physics performance data
    CollectTerrainPhysicsData();
    
    // Update memory usage estimation
    CurrentMetrics.MemoryUsageMB = (TrackedLandscapes.Num() * 50.0f) + 
                                   (CurrentMetrics.ActiveDeformationPoints * 0.1f) + 
                                   (MaterialCacheSize * 0.05f);
}

void UPerf_TerrainPhysicsPerformanceIntegrator::ApplyOptimizationLevel()
{
    switch (OptimizationLevel)
    {
        case EPerf_TerrainOptimizationLevel::Ultra:
            MaxDeformationPoints = 2000;
            DeformationQuality = 2.0f;
            TerrainCollisionLOD = 0;
            MaterialCacheSize = 4096;
            break;
            
        case EPerf_TerrainOptimizationLevel::High:
            MaxDeformationPoints = 1500;
            DeformationQuality = 1.5f;
            TerrainCollisionLOD = 0;
            MaterialCacheSize = 2048;
            break;
            
        case EPerf_TerrainOptimizationLevel::Medium:
            MaxDeformationPoints = 1000;
            DeformationQuality = 1.0f;
            TerrainCollisionLOD = 1;
            MaterialCacheSize = 1024;
            break;
            
        case EPerf_TerrainOptimizationLevel::Low:
            MaxDeformationPoints = 500;
            DeformationQuality = 0.7f;
            TerrainCollisionLOD = 2;
            MaterialCacheSize = 512;
            break;
            
        case EPerf_TerrainOptimizationLevel::Minimal:
            MaxDeformationPoints = 200;
            DeformationQuality = 0.5f;
            TerrainCollisionLOD = 3;
            MaterialCacheSize = 256;
            break;
    }
}

void UPerf_TerrainPhysicsPerformanceIntegrator::CheckAdaptiveOptimization()
{
    if (!IsTerrainPhysicsPerformanceOptimal())
    {
        // Performance is below threshold, reduce quality
        if (OptimizationLevel != EPerf_TerrainOptimizationLevel::Minimal)
        {
            EPerf_TerrainOptimizationLevel NewLevel = static_cast<EPerf_TerrainOptimizationLevel>(
                static_cast<int32>(OptimizationLevel) + 1);
            SetTerrainOptimizationLevel(NewLevel);
            
            UE_LOG(LogTemp, Warning, TEXT("TerrainPhysicsPerformanceIntegrator: Performance below threshold, reducing to level %d"), 
                   (int32)NewLevel);
        }
    }
    else if (OptimizationLevel != EPerf_TerrainOptimizationLevel::Ultra)
    {
        // Performance is good, consider increasing quality
        float PerformanceMargin = (MaxDeformationTime - CurrentMetrics.TerrainDeformationTime) / MaxDeformationTime;
        if (PerformanceMargin > 0.3f) // 30% margin
        {
            EPerf_TerrainOptimizationLevel NewLevel = static_cast<EPerf_TerrainOptimizationLevel>(
                static_cast<int32>(OptimizationLevel) - 1);
            SetTerrainOptimizationLevel(NewLevel);
            
            UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsPerformanceIntegrator: Performance margin available, increasing to level %d"), 
                   (int32)NewLevel);
        }
    }
}

void UPerf_TerrainPhysicsPerformanceIntegrator::UpdateTerrainSystemReferences()
{
    if (UWorld* World = GetWorld())
    {
        // Find all landscape actors
        TrackedLandscapes.Empty();
        for (TActorIterator<ALandscapeProxy> ActorIterator(World); ActorIterator; ++ActorIterator)
        {
            if (ALandscapeProxy* Landscape = *ActorIterator)
            {
                TrackedLandscapes.Add(Landscape);
            }
        }
        
        UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsPerformanceIntegrator: Updated references - %d landscapes found"), 
               TrackedLandscapes.Num());
    }
}

void UPerf_TerrainPhysicsPerformanceIntegrator::CollectTerrainPhysicsData()
{
    // Simulate terrain physics performance data collection
    // In a real implementation, this would interface with actual terrain physics systems
    
    // Simulate some performance metrics based on current settings
    CurrentMetrics.TerrainDeformationTime = FMath::RandRange(0.5f, 3.0f) * (1.0f / DeformationQuality);
    CurrentMetrics.SurfaceMaterialDetectionTime = FMath::RandRange(0.2f, 1.5f);
    CurrentMetrics.CollisionCalculationTime = FMath::RandRange(1.0f, 4.0f) * (TerrainCollisionLOD + 1);
    CurrentMetrics.ActiveDeformationPoints = FMath::RandRange(50, MaxDeformationPoints);
    
    // Simulate cache performance
    if (bMaterialCacheOptimization)
    {
        CurrentMetrics.MaterialCacheHits += FMath::RandRange(10, 50);
        CurrentMetrics.MaterialCacheMisses += FMath::RandRange(0, 10);
    }
    else
    {
        CurrentMetrics.MaterialCacheHits += FMath::RandRange(5, 20);
        CurrentMetrics.MaterialCacheMisses += FMath::RandRange(5, 25);
    }
}

void UPerf_TerrainPhysicsPerformanceIntegrator::ApplyDeformationOptimizations()
{
    UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsPerformanceIntegrator: Applying deformation optimizations"));
    
    // Apply deformation culling if enabled
    if (bDeformationCulling)
    {
        CurrentMetrics.ActiveDeformationPoints = FMath::Min(CurrentMetrics.ActiveDeformationPoints, MaxDeformationPoints);
    }
    
    // Apply quality scaling
    CurrentMetrics.TerrainDeformationTime *= (2.0f - DeformationQuality);
}

void UPerf_TerrainPhysicsPerformanceIntegrator::ApplyMaterialCacheOptimizations()
{
    UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsPerformanceIntegrator: Applying material cache optimizations"));
    
    if (bMaterialCacheOptimization)
    {
        // Improve cache hit ratio
        float HitRatio = static_cast<float>(CurrentMetrics.MaterialCacheHits) / 
                        FMath::Max(1, CurrentMetrics.MaterialCacheHits + CurrentMetrics.MaterialCacheMisses);
        
        if (HitRatio < 0.8f)
        {
            OptimizeMaterialCache();
        }
    }
}

void UPerf_TerrainPhysicsPerformanceIntegrator::ApplyCollisionOptimizations()
{
    UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsPerformanceIntegrator: Applying collision optimizations"));
    
    if (bCollisionOptimization)
    {
        // Apply LOD-based collision optimization
        float LODMultiplier = 1.0f - (TerrainCollisionLOD * 0.15f);
        CurrentMetrics.CollisionCalculationTime *= LODMultiplier;
    }
}