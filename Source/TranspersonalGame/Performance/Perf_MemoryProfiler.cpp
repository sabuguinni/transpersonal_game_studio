#include "Perf_MemoryProfiler.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformMemory.h"
#include "Stats/Stats.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture.h"
#include "Sound/SoundWave.h"
#include "TimerManager.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"

void UPerf_MemoryProfiler::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    bIsProfilingActive = false;
    ProfilingInterval = 5.0f;
    MemoryWarningThresholdMB = 6000.0f;  // 6GB warning
    MemoryCriticalThresholdMB = 8000.0f; // 8GB critical
    MemoryHistory.Reserve(1000); // Reserve space for history
    
    UE_LOG(LogTemp, Log, TEXT("Performance Memory Profiler initialized"));
}

void UPerf_MemoryProfiler::Deinitialize()
{
    StopMemoryProfiling();
    MemoryHistory.Empty();
    Super::Deinitialize();
}

FPerf_MemorySnapshot UPerf_MemoryProfiler::CaptureMemorySnapshot()
{
    FPerf_MemorySnapshot Snapshot;
    
    // Get platform memory stats
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    Snapshot.PhysicalMemoryMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    Snapshot.VirtualMemoryMB = MemStats.UsedVirtual / (1024.0f * 1024.0f);
    
    // Calculate texture memory usage
    Snapshot.TextureMemoryMB = CalculateTextureMemoryUsage();
    
    // Calculate mesh memory usage
    Snapshot.MeshMemoryMB = CalculateMeshMemoryUsage();
    
    // Calculate audio memory usage
    Snapshot.AudioMemoryMB = CalculateAudioMemoryUsage();
    
    // Count actors and components
    if (UWorld* World = GetWorld())
    {
        Snapshot.ActorCount = 0;
        Snapshot.ComponentCount = 0;
        
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (IsValid(Actor))
            {
                Snapshot.ActorCount++;
                Snapshot.ComponentCount += Actor->GetRootComponent() ? 
                    Actor->GetRootComponent()->GetAttachChildren().Num() + 1 : 0;
            }
        }
    }
    
    Snapshot.Timestamp = FDateTime::Now();
    return Snapshot;
}

void UPerf_MemoryProfiler::StartMemoryProfiling(float IntervalSeconds)
{
    if (bIsProfilingActive)
    {
        StopMemoryProfiling();
    }
    
    ProfilingInterval = FMath::Max(IntervalSeconds, 1.0f);
    bIsProfilingActive = true;
    
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            ProfilingTimerHandle,
            this,
            &UPerf_MemoryProfiler::OnProfilingTick,
            ProfilingInterval,
            true
        );
    }
    
    UE_LOG(LogTemp, Log, TEXT("Memory profiling started with %f second intervals"), ProfilingInterval);
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

float UPerf_MemoryProfiler::GetAverageMemoryUsage() const
{
    if (MemoryHistory.Num() == 0)
    {
        return 0.0f;
    }
    
    float TotalMemory = 0.0f;
    for (const FPerf_MemorySnapshot& Snapshot : MemoryHistory)
    {
        TotalMemory += Snapshot.PhysicalMemoryMB;
    }
    
    return TotalMemory / MemoryHistory.Num();
}

float UPerf_MemoryProfiler::GetPeakMemoryUsage() const
{
    float PeakMemory = 0.0f;
    for (const FPerf_MemorySnapshot& Snapshot : MemoryHistory)
    {
        PeakMemory = FMath::Max(PeakMemory, Snapshot.PhysicalMemoryMB);
    }
    return PeakMemory;
}

bool UPerf_MemoryProfiler::IsMemoryUsageCritical() const
{
    FPerf_MemorySnapshot CurrentSnapshot = const_cast<UPerf_MemoryProfiler*>(this)->CaptureMemorySnapshot();
    return CurrentSnapshot.PhysicalMemoryMB > MemoryCriticalThresholdMB;
}

void UPerf_MemoryProfiler::ForceGarbageCollection()
{
    UE_LOG(LogTemp, Warning, TEXT("Forcing garbage collection due to high memory usage"));
    GEngine->ForceGarbageCollection(true);
}

void UPerf_MemoryProfiler::OptimizeMemoryUsage()
{
    UE_LOG(LogTemp, Log, TEXT("Starting memory optimization"));
    
    // Force garbage collection
    ForceGarbageCollection();
    
    // Trim memory pools
    FPlatformMemory::Trim();
    
    // Log optimization results
    FPerf_MemorySnapshot PostOptimization = CaptureMemorySnapshot();
    UE_LOG(LogTemp, Log, TEXT("Memory optimization complete. Current usage: %f MB"), 
           PostOptimization.PhysicalMemoryMB);
}

void UPerf_MemoryProfiler::OnProfilingTick()
{
    FPerf_MemorySnapshot Snapshot = CaptureMemorySnapshot();
    MemoryHistory.Add(Snapshot);
    
    // Check for memory warnings
    if (Snapshot.PhysicalMemoryMB > MemoryCriticalThresholdMB)
    {
        UE_LOG(LogTemp, Error, TEXT("CRITICAL: Memory usage at %f MB (threshold: %f MB)"), 
               Snapshot.PhysicalMemoryMB, MemoryCriticalThresholdMB);
        OptimizeMemoryUsage();
    }
    else if (Snapshot.PhysicalMemoryMB > MemoryWarningThresholdMB)
    {
        UE_LOG(LogTemp, Warning, TEXT("WARNING: High memory usage at %f MB (threshold: %f MB)"), 
               Snapshot.PhysicalMemoryMB, MemoryWarningThresholdMB);
    }
    
    // Clean up old snapshots to prevent memory growth
    CleanupOldSnapshots();
}

float UPerf_MemoryProfiler::CalculateTextureMemoryUsage() const
{
    // Simplified texture memory calculation
    // In a real implementation, you'd iterate through loaded textures
    return 0.0f; // Placeholder
}

float UPerf_MemoryProfiler::CalculateMeshMemoryUsage() const
{
    // Simplified mesh memory calculation
    // In a real implementation, you'd iterate through loaded meshes
    return 0.0f; // Placeholder
}

float UPerf_MemoryProfiler::CalculateAudioMemoryUsage() const
{
    // Simplified audio memory calculation
    // In a real implementation, you'd iterate through loaded audio assets
    return 0.0f; // Placeholder
}

void UPerf_MemoryProfiler::CleanupOldSnapshots()
{
    // Keep only the last 200 snapshots to prevent memory growth
    const int32 MaxSnapshots = 200;
    if (MemoryHistory.Num() > MaxSnapshots)
    {
        int32 ToRemove = MemoryHistory.Num() - MaxSnapshots;
        MemoryHistory.RemoveAt(0, ToRemove);
    }
}