#include "Perf_MemoryProfiler.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "HAL/PlatformMemory.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"

UPerf_MemoryProfiler::UPerf_MemoryProfiler()
{
    bIsProfilingActive = false;
    MemoryThresholdMB = 8192.0f; // 8GB threshold
    ProfilingInterval = 1.0f; // Update every second
}

void UPerf_MemoryProfiler::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Performance Memory Profiler initialized"));
    
    // Start profiling automatically
    StartMemoryProfiling();
}

void UPerf_MemoryProfiler::Deinitialize()
{
    StopMemoryProfiling();
    Super::Deinitialize();
}

FPerf_MemoryStats UPerf_MemoryProfiler::GetCurrentMemoryStats()
{
    UpdateMemoryStats();
    return CurrentStats;
}

void UPerf_MemoryProfiler::StartMemoryProfiling()
{
    if (bIsProfilingActive)
    {
        return;
    }

    bIsProfilingActive = true;
    
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            ProfilingTimerHandle,
            this,
            &UPerf_MemoryProfiler::UpdateMemoryStats,
            ProfilingInterval,
            true
        );
    }
    
    UE_LOG(LogTemp, Log, TEXT("Memory profiling started"));
}

void UPerf_MemoryProfiler::StopMemoryProfiling()
{
    if (!bIsProfilingActive)
    {
        return;
    }

    bIsProfilingActive = false;
    
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(ProfilingTimerHandle);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Memory profiling stopped"));
}

bool UPerf_MemoryProfiler::IsMemoryUsageAboveThreshold() const
{
    return CurrentStats.UsedPhysicalMB > MemoryThresholdMB;
}

void UPerf_MemoryProfiler::LogMemoryReport()
{
    UpdateMemoryStats();
    
    UE_LOG(LogTemp, Warning, TEXT("=== MEMORY REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Used Physical: %.2f MB"), CurrentStats.UsedPhysicalMB);
    UE_LOG(LogTemp, Warning, TEXT("Used Virtual: %.2f MB"), CurrentStats.UsedVirtualMB);
    UE_LOG(LogTemp, Warning, TEXT("Peak Physical: %.2f MB"), CurrentStats.PeakUsedPhysicalMB);
    UE_LOG(LogTemp, Warning, TEXT("Total Physical: %.2f MB"), CurrentStats.TotalPhysicalMB);
    UE_LOG(LogTemp, Warning, TEXT("Available Physical: %.2f MB"), CurrentStats.AvailablePhysicalMB);
    UE_LOG(LogTemp, Warning, TEXT("Actor Count: %d"), CurrentStats.ActorCount);
    UE_LOG(LogTemp, Warning, TEXT("Component Count: %d"), CurrentStats.ComponentCount);
    UE_LOG(LogTemp, Warning, TEXT("Memory Threshold: %.2f MB"), MemoryThresholdMB);
    UE_LOG(LogTemp, Warning, TEXT("Above Threshold: %s"), IsMemoryUsageAboveThreshold() ? TEXT("YES") : TEXT("NO"));
}

void UPerf_MemoryProfiler::ForceGarbageCollection()
{
    UE_LOG(LogTemp, Warning, TEXT("Forcing garbage collection..."));
    GEngine->ForceGarbageCollection(true);
    UpdateMemoryStats();
    UE_LOG(LogTemp, Warning, TEXT("Garbage collection completed. Memory usage: %.2f MB"), CurrentStats.UsedPhysicalMB);
}

void UPerf_MemoryProfiler::UpdateMemoryStats()
{
    // Get platform memory stats
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    
    CurrentStats.UsedPhysicalMB = MemStats.UsedPhysical / 1024.0f / 1024.0f;
    CurrentStats.UsedVirtualMB = MemStats.UsedVirtual / 1024.0f / 1024.0f;
    CurrentStats.PeakUsedPhysicalMB = MemStats.PeakUsedPhysical / 1024.0f / 1024.0f;
    CurrentStats.TotalPhysicalMB = MemStats.TotalPhysical / 1024.0f / 1024.0f;
    CurrentStats.AvailablePhysicalMB = MemStats.AvailablePhysical / 1024.0f / 1024.0f;
    
    // Count actors and components in the world
    CurrentStats.ActorCount = 0;
    CurrentStats.ComponentCount = 0;
    
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
        {
            AActor* Actor = *ActorIterator;
            if (IsValid(Actor))
            {
                CurrentStats.ActorCount++;
                CurrentStats.ComponentCount += Actor->GetRootComponent() ? Actor->GetComponents<UActorComponent>().Num() : 0;
            }
        }
    }
    
    CheckMemoryThreshold();
}

void UPerf_MemoryProfiler::CheckMemoryThreshold()
{
    if (IsMemoryUsageAboveThreshold())
    {
        UE_LOG(LogTemp, Error, TEXT("MEMORY WARNING: Usage (%.2f MB) exceeds threshold (%.2f MB)!"), 
               CurrentStats.UsedPhysicalMB, MemoryThresholdMB);
        
        // Log detailed report when threshold is exceeded
        LogMemoryReport();
    }
}