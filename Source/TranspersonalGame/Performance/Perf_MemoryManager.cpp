#include "Perf_MemoryManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "HAL/PlatformMemory.h"
#include "UObject/UObjectGlobals.h"
#include "Engine/AssetManager.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"

UPerf_MemoryManager::UPerf_MemoryManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
    bAutoActivate = true;
}

void UPerf_MemoryManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize memory tracking
    UpdateMemoryStats();
    
    UE_LOG(LogTemp, Log, TEXT("Performance Memory Manager initialized"));
    UE_LOG(LogTemp, Log, TEXT("Initial memory usage: %.1f MB (%.1f%%)"), 
           CachedMemoryStats.UsedMemoryMB, CachedMemoryStats.MemoryUsagePercent);
}

void UPerf_MemoryManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bMemoryTrackingEnabled)
    {
        return;
    }
    
    LastMemoryCheck += DeltaTime;
    if (LastMemoryCheck >= MemoryCheckInterval)
    {
        UpdateMemoryStats();
        HandleMemoryPressure(CachedMemoryStats.MemoryPressure);
        LastMemoryCheck = 0.0f;
    }
}

FPerf_MemoryStats UPerf_MemoryManager::GetCurrentMemoryStats()
{
    UpdateMemoryStats();
    return CachedMemoryStats;
}

bool UPerf_MemoryManager::IsMemoryPressureHigh() const
{
    return CachedMemoryStats.MemoryPressure == EPerf_MemoryPressure::High || 
           CachedMemoryStats.MemoryPressure == EPerf_MemoryPressure::Critical;
}

void UPerf_MemoryManager::ForceGarbageCollection()
{
    UE_LOG(LogTemp, Warning, TEXT("Forcing garbage collection - Memory usage: %.1f%%"), 
           CachedMemoryStats.MemoryUsagePercent);
    
    CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
    
    // Update stats after GC
    UpdateMemoryStats();
    
    UE_LOG(LogTemp, Log, TEXT("Garbage collection complete - New memory usage: %.1f%%"), 
           CachedMemoryStats.MemoryUsagePercent);
}

void UPerf_MemoryManager::OptimizeMemoryUsage()
{
    UE_LOG(LogTemp, Log, TEXT("Starting memory optimization"));
    
    // Step 1: Unload unused assets
    if (bAutoUnloadAssets)
    {
        UnloadUnusedAssets();
    }
    
    // Step 2: Force garbage collection
    if (bAutoGarbageCollection)
    {
        ForceGarbageCollection();
    }
    
    // Step 3: Optimize actor count if needed
    UWorld* World = GetWorld();
    if (World && CachedMemoryStats.ActiveActorCount > MaxActiveActors)
    {
        UE_LOG(LogTemp, Warning, TEXT("High actor count detected: %d (max: %d)"), 
               CachedMemoryStats.ActiveActorCount, MaxActiveActors);
        
        // Could implement actor culling/pooling here
        PerformMemoryOptimization();
    }
    
    UpdateMemoryStats();
    UE_LOG(LogTemp, Log, TEXT("Memory optimization complete - Usage: %.1f%%"), 
           CachedMemoryStats.MemoryUsagePercent);
}

void UPerf_MemoryManager::SetMemoryThresholds(float MediumThreshold, float HighThreshold, float CriticalThreshold)
{
    MediumPressureThreshold = FMath::Clamp(MediumThreshold, 0.0f, 100.0f);
    HighPressureThreshold = FMath::Clamp(HighThreshold, 0.0f, 100.0f);
    CriticalPressureThreshold = FMath::Clamp(CriticalThreshold, 0.0f, 100.0f);
    
    UE_LOG(LogTemp, Log, TEXT("Memory thresholds updated: Medium=%.1f%%, High=%.1f%%, Critical=%.1f%%"),
           MediumPressureThreshold, HighPressureThreshold, CriticalPressureThreshold);
}

void UPerf_MemoryManager::UnloadUnusedAssets()
{
    UE_LOG(LogTemp, Log, TEXT("Unloading unused assets"));
    
    // Use engine's asset management to unload unused assets
    if (UAssetManager* AssetManager = UAssetManager::GetIfValid())
    {
        // This would typically involve more sophisticated asset tracking
        // For now, we'll rely on the garbage collector
        CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
    }
}

void UPerf_MemoryManager::PreloadCriticalAssets()
{
    UE_LOG(LogTemp, Log, TEXT("Preloading critical assets"));
    
    // This would typically preload essential game assets
    // Implementation depends on specific game requirements
}

void UPerf_MemoryManager::LogMemoryReport()
{
    UpdateMemoryStats();
    
    UE_LOG(LogTemp, Warning, TEXT("=== MEMORY REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Used Memory: %.1f MB"), CachedMemoryStats.UsedMemoryMB);
    UE_LOG(LogTemp, Warning, TEXT("Available Memory: %.1f MB"), CachedMemoryStats.AvailableMemoryMB);
    UE_LOG(LogTemp, Warning, TEXT("Total Memory: %.1f MB"), CachedMemoryStats.TotalMemoryMB);
    UE_LOG(LogTemp, Warning, TEXT("Usage Percentage: %.1f%%"), CachedMemoryStats.MemoryUsagePercent);
    UE_LOG(LogTemp, Warning, TEXT("Memory Pressure: %s"), 
           CachedMemoryStats.MemoryPressure == EPerf_MemoryPressure::Low ? TEXT("Low") :
           CachedMemoryStats.MemoryPressure == EPerf_MemoryPressure::Medium ? TEXT("Medium") :
           CachedMemoryStats.MemoryPressure == EPerf_MemoryPressure::High ? TEXT("High") : TEXT("Critical"));
    UE_LOG(LogTemp, Warning, TEXT("Active Actors: %d"), CachedMemoryStats.ActiveActorCount);
    UE_LOG(LogTemp, Warning, TEXT("Loaded Assets: %d"), CachedMemoryStats.LoadedAssetCount);
    UE_LOG(LogTemp, Warning, TEXT("=================="));
}

void UPerf_MemoryManager::EnableMemoryTracking(bool bEnable)
{
    bMemoryTrackingEnabled = bEnable;
    UE_LOG(LogTemp, Log, TEXT("Memory tracking %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UPerf_MemoryManager::UpdateMemoryStats()
{
    // Get platform memory stats
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    
    CachedMemoryStats.UsedMemoryMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    CachedMemoryStats.AvailableMemoryMB = MemStats.AvailablePhysical / (1024.0f * 1024.0f);
    CachedMemoryStats.TotalMemoryMB = MemStats.TotalPhysical / (1024.0f * 1024.0f);
    
    if (CachedMemoryStats.TotalMemoryMB > 0.0f)
    {
        CachedMemoryStats.MemoryUsagePercent = (CachedMemoryStats.UsedMemoryMB / CachedMemoryStats.TotalMemoryMB) * 100.0f;
    }
    
    CachedMemoryStats.MemoryPressure = CalculateMemoryPressure(CachedMemoryStats.MemoryUsagePercent);
    
    // Count active actors
    UWorld* World = GetWorld();
    if (World)
    {
        CachedMemoryStats.ActiveActorCount = World->GetActorCount();
    }
    
    // Estimate loaded assets (simplified)
    CachedMemoryStats.LoadedAssetCount = GUObjectArray.GetObjectArrayNum();
}

EPerf_MemoryPressure UPerf_MemoryManager::CalculateMemoryPressure(float UsagePercent)
{
    if (UsagePercent >= CriticalPressureThreshold)
    {
        return EPerf_MemoryPressure::Critical;
    }
    else if (UsagePercent >= HighPressureThreshold)
    {
        return EPerf_MemoryPressure::High;
    }
    else if (UsagePercent >= MediumPressureThreshold)
    {
        return EPerf_MemoryPressure::Medium;
    }
    
    return EPerf_MemoryPressure::Low;
}

void UPerf_MemoryManager::HandleMemoryPressure(EPerf_MemoryPressure Pressure)
{
    static EPerf_MemoryPressure LastPressure = EPerf_MemoryPressure::Low;
    
    // Only act if pressure level changed
    if (Pressure != LastPressure)
    {
        switch (Pressure)
        {
            case EPerf_MemoryPressure::Medium:
                UE_LOG(LogTemp, Warning, TEXT("Medium memory pressure detected (%.1f%%)"), 
                       CachedMemoryStats.MemoryUsagePercent);
                break;
                
            case EPerf_MemoryPressure::High:
                UE_LOG(LogTemp, Warning, TEXT("High memory pressure detected (%.1f%%) - Starting optimization"), 
                       CachedMemoryStats.MemoryUsagePercent);
                if (bAutoUnloadAssets)
                {
                    UnloadUnusedAssets();
                }
                break;
                
            case EPerf_MemoryPressure::Critical:
                UE_LOG(LogTemp, Error, TEXT("CRITICAL memory pressure detected (%.1f%%) - Emergency optimization"), 
                       CachedMemoryStats.MemoryUsagePercent);
                OptimizeMemoryUsage();
                break;
                
            default:
                break;
        }
        
        LastPressure = Pressure;
    }
}

void UPerf_MemoryManager::PerformMemoryOptimization()
{
    UE_LOG(LogTemp, Log, TEXT("Performing advanced memory optimization"));
    
    // This could include more aggressive optimization strategies:
    // - Actor pooling
    // - Texture streaming optimization  
    // - Mesh LOD adjustments
    // - Component deactivation
    
    // For now, just force garbage collection
    ForceGarbageCollection();
}