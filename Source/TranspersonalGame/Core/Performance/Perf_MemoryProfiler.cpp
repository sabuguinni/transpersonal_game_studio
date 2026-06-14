#include "Perf_MemoryProfiler.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformMemory.h"
#include "Stats/Stats.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture.h"
#include "TimerManager.h"

UPerf_MemoryProfiler::UPerf_MemoryProfiler()
{
    bIsProfilingActive = false;
    MemoryWarningThresholdMB = 4096.0f; // 4GB default threshold
}

void UPerf_MemoryProfiler::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Memory Profiler initialized"));
    
    // Start automatic profiling every 5 seconds
    StartMemoryProfiling();
}

void UPerf_MemoryProfiler::Deinitialize()
{
    StopMemoryProfiling();
    Super::Deinitialize();
}

FPerf_MemoryStats UPerf_MemoryProfiler::GetCurrentMemoryStats()
{
    FPerf_MemoryStats Stats;
    
    // Get platform memory stats
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    
    Stats.UsedPhysicalMemoryMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    Stats.UsedVirtualMemoryMB = MemStats.UsedVirtual / (1024.0f * 1024.0f);
    Stats.PeakPhysicalMemoryMB = MemStats.PeakUsedPhysical / (1024.0f * 1024.0f);
    
    // Count loaded objects
    Stats.NumLoadedObjects = GUObjectArray.GetObjectArrayNum();
    Stats.NumStaticMeshes = CountObjectsByClass(UStaticMesh::StaticClass());
    Stats.NumTextures = CountObjectsByClass(UTexture::StaticClass());
    Stats.TextureMemoryMB = GetTextureMemoryUsage();
    
    return Stats;
}

void UPerf_MemoryProfiler::StartMemoryProfiling()
{
    if (!bIsProfilingActive)
    {
        bIsProfilingActive = true;
        
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().SetTimer(
                ProfilingTimerHandle,
                this,
                &UPerf_MemoryProfiler::UpdateMemoryStats,
                5.0f, // Every 5 seconds
                true
            );
        }
        
        UE_LOG(LogTemp, Log, TEXT("Memory profiling started"));
    }
}

void UPerf_MemoryProfiler::StopMemoryProfiling()
{
    if (bIsProfilingActive)
    {
        bIsProfilingActive = false;
        
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().ClearTimer(ProfilingTimerHandle);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Memory profiling stopped"));
    }
}

void UPerf_MemoryProfiler::LogMemoryReport()
{
    FPerf_MemoryStats CurrentStats = GetCurrentMemoryStats();
    
    UE_LOG(LogTemp, Warning, TEXT("=== MEMORY REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Physical Memory: %.2f MB"), CurrentStats.UsedPhysicalMemoryMB);
    UE_LOG(LogTemp, Warning, TEXT("Virtual Memory: %.2f MB"), CurrentStats.UsedVirtualMemoryMB);
    UE_LOG(LogTemp, Warning, TEXT("Peak Physical: %.2f MB"), CurrentStats.PeakPhysicalMemoryMB);
    UE_LOG(LogTemp, Warning, TEXT("Loaded Objects: %d"), CurrentStats.NumLoadedObjects);
    UE_LOG(LogTemp, Warning, TEXT("Static Meshes: %d"), CurrentStats.NumStaticMeshes);
    UE_LOG(LogTemp, Warning, TEXT("Textures: %d"), CurrentStats.NumTextures);
    UE_LOG(LogTemp, Warning, TEXT("Texture Memory: %.2f MB"), CurrentStats.TextureMemoryMB);
    UE_LOG(LogTemp, Warning, TEXT("====================="));
}

void UPerf_MemoryProfiler::ForceGarbageCollection()
{
    UE_LOG(LogTemp, Warning, TEXT("Forcing garbage collection..."));
    
    FPerf_MemoryStats BeforeGC = GetCurrentMemoryStats();
    
    CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
    
    FPerf_MemoryStats AfterGC = GetCurrentMemoryStats();
    
    float MemoryFreed = BeforeGC.UsedPhysicalMemoryMB - AfterGC.UsedPhysicalMemoryMB;
    int32 ObjectsFreed = BeforeGC.NumLoadedObjects - AfterGC.NumLoadedObjects;
    
    UE_LOG(LogTemp, Warning, TEXT("GC Complete: %.2f MB freed, %d objects collected"), 
           MemoryFreed, ObjectsFreed);
}

bool UPerf_MemoryProfiler::IsMemoryUsageHigh()
{
    FPerf_MemoryStats Stats = GetCurrentMemoryStats();
    return Stats.UsedPhysicalMemoryMB > MemoryWarningThresholdMB;
}

void UPerf_MemoryProfiler::SetMemoryWarningThreshold(float ThresholdMB)
{
    MemoryWarningThresholdMB = ThresholdMB;
    UE_LOG(LogTemp, Log, TEXT("Memory warning threshold set to %.2f MB"), ThresholdMB);
}

void UPerf_MemoryProfiler::UpdateMemoryStats()
{
    if (!bIsProfilingActive) return;
    
    FPerf_MemoryStats CurrentStats = GetCurrentMemoryStats();
    
    // Add to history (keep last 100 entries)
    MemoryHistory.Add(CurrentStats);
    if (MemoryHistory.Num() > 100)
    {
        MemoryHistory.RemoveAt(0);
    }
    
    CheckMemoryThresholds();
}

void UPerf_MemoryProfiler::CheckMemoryThresholds()
{
    FPerf_MemoryStats Stats = GetCurrentMemoryStats();
    
    if (Stats.UsedPhysicalMemoryMB > MemoryWarningThresholdMB)
    {
        UE_LOG(LogTemp, Warning, TEXT("HIGH MEMORY USAGE: %.2f MB (threshold: %.2f MB)"), 
               Stats.UsedPhysicalMemoryMB, MemoryWarningThresholdMB);
        
        // Auto-trigger GC if memory is critically high
        if (Stats.UsedPhysicalMemoryMB > MemoryWarningThresholdMB * 1.2f)
        {
            ForceGarbageCollection();
        }
    }
}

float UPerf_MemoryProfiler::GetTextureMemoryUsage()
{
    float TotalTextureMemory = 0.0f;
    
    for (TObjectIterator<UTexture> TextureIt; TextureIt; ++TextureIt)
    {
        UTexture* Texture = *TextureIt;
        if (IsValid(Texture))
        {
            TotalTextureMemory += Texture->CalcTextureMemorySizeEnum(TMC_ResidentMips);
        }
    }
    
    return TotalTextureMemory / (1024.0f * 1024.0f); // Convert to MB
}

int32 UPerf_MemoryProfiler::CountObjectsByClass(UClass* ObjectClass)
{
    int32 Count = 0;
    
    for (TObjectIterator<UObject> ObjectIt(ObjectClass); ObjectIt; ++ObjectIt)
    {
        if (IsValid(*ObjectIt))
        {
            Count++;
        }
    }
    
    return Count;
}