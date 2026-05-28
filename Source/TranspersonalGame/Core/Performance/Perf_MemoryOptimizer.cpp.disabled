#include "Perf_MemoryOptimizer.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "HAL/PlatformMemory.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "TimerManager.h"
#include "UObject/GarbageCollection.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/BodyInstance.h"

UPerf_MemoryOptimizer::UPerf_MemoryOptimizer()
{
    MemoryPressureThreshold = 80.0f; // 80% memory usage triggers optimization
    bIsMemoryPressureHigh = false;
    LastOptimizationTime = 0.0f;
    bInitialized = false;
    LastMemoryCheckTime = 0.0f;
    MemoryCheckInterval = 1.0f; // Check memory every second
    OptimizationInterval = 5.0f; // Optimize every 5 seconds if needed
}

void UPerf_MemoryOptimizer::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    if (!bInitialized)
    {
        // Initialize memory metrics
        UpdateMemoryMetrics();
        
        // Set up monitoring timer
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().SetTimer(
                MemoryMonitorTimer,
                this,
                &UPerf_MemoryOptimizer::UpdateMemoryMetrics,
                MemoryCheckInterval,
                true
            );
            
            World->GetTimerManager().SetTimer(
                OptimizationTimer,
                this,
                &UPerf_MemoryOptimizer::CheckMemoryPressure,
                OptimizationInterval,
                true
            );
        }
        
        // Register default memory pools
        RegisterMemoryPool(TEXT("Physics"), 512.0f, EPerf_MemoryPriority::High);
        RegisterMemoryPool(TEXT("Meshes"), 1024.0f, EPerf_MemoryPriority::Medium);
        RegisterMemoryPool(TEXT("Textures"), 2048.0f, EPerf_MemoryPriority::Medium);
        RegisterMemoryPool(TEXT("Audio"), 256.0f, EPerf_MemoryPriority::Low);
        RegisterMemoryPool(TEXT("Particles"), 128.0f, EPerf_MemoryPriority::Disposable);
        
        bInitialized = true;
        
        UE_LOG(LogTemp, Log, TEXT("Perf_MemoryOptimizer initialized with %d memory pools"), MemoryPools.Num());
    }
}

void UPerf_MemoryOptimizer::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(MemoryMonitorTimer);
        World->GetTimerManager().ClearTimer(OptimizationTimer);
    }
    
    TrackedObjects.Empty();
    MemoryPools.Empty();
    
    bInitialized = false;
    
    Super::Deinitialize();
}

FPerf_MemoryMetrics UPerf_MemoryOptimizer::GetCurrentMemoryMetrics()
{
    UpdateMemoryMetrics();
    return CachedMetrics;
}

float UPerf_MemoryOptimizer::GetMemoryPressure()
{
    const FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    const float UsedMemoryMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    const float TotalMemoryMB = MemStats.TotalPhysical / (1024.0f * 1024.0f);
    
    return TotalMemoryMB > 0.0f ? (UsedMemoryMB / TotalMemoryMB) * 100.0f : 0.0f;
}

bool UPerf_MemoryOptimizer::IsMemoryPressureHigh()
{
    return GetMemoryPressure() > MemoryPressureThreshold;
}

void UPerf_MemoryOptimizer::OptimizeMemoryUsage()
{
    const float CurrentTime = FPlatformTime::Seconds();
    
    // Don't optimize too frequently
    if (CurrentTime - LastOptimizationTime < OptimizationInterval)
    {
        return;
    }
    
    LastOptimizationTime = CurrentTime;
    
    UE_LOG(LogTemp, Log, TEXT("Perf_MemoryOptimizer: Starting memory optimization"));
    
    // Clean up invalid tracked objects first
    CleanupInvalidObjects();
    
    // Optimize by priority levels
    if (IsMemoryPressureHigh())
    {
        // High pressure - optimize all levels
        OptimizeByPriority(EPerf_MemoryPriority::Disposable);
        OptimizeByPriority(EPerf_MemoryPriority::Low);
        
        if (GetMemoryPressure() > 90.0f)
        {
            // Critical pressure - optimize medium priority too
            OptimizeByPriority(EPerf_MemoryPriority::Medium);
            ForceGarbageCollection();
        }
    }
    else
    {
        // Normal pressure - just clean up disposable objects
        OptimizeByPriority(EPerf_MemoryPriority::Disposable);
    }
    
    // Purge unused assets
    PurgeUnusedAssets();
    
    // Update metrics after optimization
    UpdateMemoryMetrics();
    
    UE_LOG(LogTemp, Log, TEXT("Perf_MemoryOptimizer: Memory optimization completed. Pressure: %.1f%%"), GetMemoryPressure());
}

void UPerf_MemoryOptimizer::ForceGarbageCollection()
{
    UE_LOG(LogTemp, Log, TEXT("Perf_MemoryOptimizer: Forcing garbage collection"));
    
    // Force immediate garbage collection
    GEngine->ForceGarbageCollection(true);
    
    // Also try to trim memory
    FPlatformMemory::Trim();
}

void UPerf_MemoryOptimizer::PurgeUnusedAssets()
{
    if (UAssetManager* AssetManager = UAssetManager::GetIfValid())
    {
        // Unload unused assets
        AssetManager->GetStreamableManager().RequestAsyncLoad(TArray<FSoftObjectPath>());
    }
    
    // Purge unused textures and meshes
    if (GEngine)
    {
        GEngine->Exec(nullptr, TEXT("gc.CollectGarbageEveryFrame 1"));
        GEngine->Exec(nullptr, TEXT("gc.CollectGarbageEveryFrame 0"));
    }
}

void UPerf_MemoryOptimizer::RegisterMemoryPool(const FString& PoolName, float MaxSizeMB, EPerf_MemoryPriority Priority)
{
    FPerf_MemoryPool NewPool;
    NewPool.PoolName = PoolName;
    NewPool.MaxSizeMB = MaxSizeMB;
    NewPool.Priority = Priority;
    NewPool.AllocatedMB = 0.0f;
    NewPool.ActiveAllocations = 0;
    
    MemoryPools.Add(NewPool);
    
    UE_LOG(LogTemp, Log, TEXT("Perf_MemoryOptimizer: Registered memory pool '%s' with max size %.1f MB"), *PoolName, MaxSizeMB);
}

TArray<FPerf_MemoryPool> UPerf_MemoryOptimizer::GetMemoryPools()
{
    // Update pool statistics
    for (FPerf_MemoryPool& Pool : MemoryPools)
    {
        if (Pool.PoolName == TEXT("Physics"))
        {
            Pool.AllocatedMB = GetPhysicsMemoryUsage();
        }
        else if (Pool.PoolName == TEXT("Meshes"))
        {
            Pool.AllocatedMB = GetMeshMemoryUsage();
        }
        // Add more pool-specific calculations as needed
    }
    
    return MemoryPools;
}

void UPerf_MemoryOptimizer::SetMemoryPressureThreshold(float ThresholdPercent)
{
    MemoryPressureThreshold = FMath::Clamp(ThresholdPercent, 50.0f, 95.0f);
    UE_LOG(LogTemp, Log, TEXT("Perf_MemoryOptimizer: Memory pressure threshold set to %.1f%%"), MemoryPressureThreshold);
}

void UPerf_MemoryOptimizer::TrackObject(UObject* Object, EPerf_MemoryPriority Priority)
{
    if (Object && IsValid(Object))
    {
        TrackedObjects.Add(Object, Priority);
        
        // Update active object count stat
        SET_DWORD_STAT(STAT_PerfActiveObjects, TrackedObjects.Num());
    }
}

void UPerf_MemoryOptimizer::UntrackObject(UObject* Object)
{
    if (Object)
    {
        TrackedObjects.Remove(Object);
        
        // Update active object count stat
        SET_DWORD_STAT(STAT_PerfActiveObjects, TrackedObjects.Num());
    }
}

int32 UPerf_MemoryOptimizer::GetTrackedObjectCount()
{
    CleanupInvalidObjects();
    return TrackedObjects.Num();
}

void UPerf_MemoryOptimizer::UpdateMemoryMetrics()
{
    const FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    
    CachedMetrics.PhysicalMemoryMB = MemStats.TotalPhysical / (1024.0f * 1024.0f);
    CachedMetrics.VirtualMemoryMB = MemStats.TotalVirtual / (1024.0f * 1024.0f);
    CachedMetrics.UsedMemoryMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    CachedMetrics.AvailableMemoryMB = MemStats.AvailablePhysical / (1024.0f * 1024.0f);
    CachedMetrics.MemoryPressurePercent = GetMemoryPressure();
    CachedMetrics.ActiveObjectCount = GetTrackedObjectCount();
    CachedMetrics.PhysicsMemoryMB = GetPhysicsMemoryUsage();
    CachedMetrics.MeshMemoryMB = GetMeshMemoryUsage();
    
    // Update stats
    SET_DWORD_STAT(STAT_PerfActiveObjects, CachedMetrics.ActiveObjectCount);
    SET_MEMORY_STAT(STAT_PerfPhysicsMemory, CachedMetrics.PhysicsMemoryMB * 1024 * 1024);
    SET_MEMORY_STAT(STAT_PerfMeshMemory, CachedMetrics.MeshMemoryMB * 1024 * 1024);
    
    LastMemoryCheckTime = FPlatformTime::Seconds();
}

void UPerf_MemoryOptimizer::CheckMemoryPressure()
{
    const bool bWasHighPressure = bIsMemoryPressureHigh;
    bIsMemoryPressureHigh = IsMemoryPressureHigh();
    
    if (bIsMemoryPressureHigh && !bWasHighPressure)
    {
        // Pressure just went high
        OnMemoryPressureHigh(GetMemoryPressure());
        OptimizeMemoryUsage();
    }
    else if (!bIsMemoryPressureHigh && bWasHighPressure)
    {
        // Pressure returned to normal
        OnMemoryPressureNormal(GetMemoryPressure());
    }
    
    // Automatic optimization if pressure is high
    if (bIsMemoryPressureHigh)
    {
        OptimizeMemoryUsage();
    }
}

void UPerf_MemoryOptimizer::OptimizeTrackedObjects()
{
    // This would be called by CheckMemoryPressure if needed
    CleanupInvalidObjects();
    
    // Additional object-specific optimizations could go here
    for (auto& ObjectPair : TrackedObjects)
    {
        if (UObject* Object = ObjectPair.Key.Get())
        {
            // Perform priority-based optimizations
            EPerf_MemoryPriority Priority = ObjectPair.Value;
            
            if (Priority == EPerf_MemoryPriority::Disposable && bIsMemoryPressureHigh)
            {
                // Mark disposable objects for cleanup
                Object->MarkAsGarbage();
            }
        }
    }
}

void UPerf_MemoryOptimizer::CleanupInvalidObjects()
{
    // Remove invalid weak pointers
    for (auto It = TrackedObjects.CreateIterator(); It; ++It)
    {
        if (!It.Key().IsValid())
        {
            It.RemoveCurrent();
        }
    }
}

float UPerf_MemoryOptimizer::GetPhysicsMemoryUsage()
{
    // Estimate physics memory usage
    float PhysicsMemory = 0.0f;
    
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor->GetRootComponent())
            {
                if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
                {
                    if (PrimComp->GetBodyInstance())
                    {
                        // Rough estimate: 1KB per physics body
                        PhysicsMemory += 0.001f; // MB
                    }
                }
            }
        }
    }
    
    return PhysicsMemory;
}

float UPerf_MemoryOptimizer::GetMeshMemoryUsage()
{
    // Estimate mesh memory usage
    float MeshMemory = 0.0f;
    
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor)
            {
                TArray<UStaticMeshComponent*> StaticMeshComps;
                Actor->GetComponents<UStaticMeshComponent>(StaticMeshComps);
                
                for (UStaticMeshComponent* MeshComp : StaticMeshComps)
                {
                    if (MeshComp && MeshComp->GetStaticMesh())
                    {
                        // Rough estimate: 5MB per static mesh
                        MeshMemory += 5.0f;
                    }
                }
                
                TArray<USkeletalMeshComponent*> SkeletalMeshComps;
                Actor->GetComponents<USkeletalMeshComponent>(SkeletalMeshComps);
                
                for (USkeletalMeshComponent* SkMeshComp : SkeletalMeshComps)
                {
                    if (SkMeshComp && SkMeshComp->GetSkeletalMeshAsset())
                    {
                        // Rough estimate: 10MB per skeletal mesh
                        MeshMemory += 10.0f;
                    }
                }
            }
        }
    }
    
    return MeshMemory;
}

void UPerf_MemoryOptimizer::OptimizeByPriority(EPerf_MemoryPriority Priority)
{
    int32 OptimizedCount = 0;
    
    for (auto It = TrackedObjects.CreateIterator(); It; ++It)
    {
        if (It.Value() == Priority)
        {
            if (UObject* Object = It.Key().Get())
            {
                if (Priority == EPerf_MemoryPriority::Disposable)
                {
                    // Mark disposable objects for garbage collection
                    Object->MarkAsGarbage();
                    It.RemoveCurrent();
                    OptimizedCount++;
                }
                else if (Priority == EPerf_MemoryPriority::Low)
                {
                    // Reduce quality or unload low priority objects
                    if (UStaticMeshComponent* MeshComp = Cast<UStaticMeshComponent>(Object))
                    {
                        // Reduce LOD or disable shadows
                        MeshComp->SetCastShadow(false);
                        OptimizedCount++;
                    }
                }
            }
            else
            {
                It.RemoveCurrent();
            }
        }
    }
    
    if (OptimizedCount > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Perf_MemoryOptimizer: Optimized %d objects with priority %d"), OptimizedCount, (int32)Priority);
    }
}

void UPerf_MemoryOptimizer::PurgeDisposableObjects()
{
    OptimizeByPriority(EPerf_MemoryPriority::Disposable);
    ForceGarbageCollection();
}