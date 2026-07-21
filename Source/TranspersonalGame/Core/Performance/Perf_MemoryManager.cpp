#include "Perf_MemoryManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "HAL/PlatformMemory.h"
#include "RenderingThread.h"
#include "RHI.h"
#include "TimerManager.h"
#include "Engine/Level.h"
#include "Components/ActorComponent.h"

UPerf_MemoryManager::UPerf_MemoryManager()
{
    bIsProfilingActive = false;
    PhysicalMemoryBudgetMB = 8192.0f; // 8GB default
    TextureMemoryBudgetMB = 2048.0f;  // 2GB default
}

void UPerf_MemoryManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Perf_MemoryManager: Initialized"));
    
    // Start with initial memory reading
    UpdateMemoryStats();
}

void UPerf_MemoryManager::Deinitialize()
{
    StopMemoryProfiling();
    Super::Deinitialize();
}

FPerf_MemoryStats UPerf_MemoryManager::GetCurrentMemoryStats()
{
    UpdateMemoryStats();
    return LastMemoryStats;
}

void UPerf_MemoryManager::StartMemoryProfiling()
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
            &UPerf_MemoryManager::UpdateMemoryStats,
            1.0f, // Update every second
            true
        );
    }

    UE_LOG(LogTemp, Warning, TEXT("Perf_MemoryManager: Memory profiling started"));
}

void UPerf_MemoryManager::StopMemoryProfiling()
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

    UE_LOG(LogTemp, Warning, TEXT("Perf_MemoryManager: Memory profiling stopped"));
}

void UPerf_MemoryManager::ForceGarbageCollection()
{
    UE_LOG(LogTemp, Warning, TEXT("Perf_MemoryManager: Forcing garbage collection"));
    
    // Force garbage collection
    GEngine->ForceGarbageCollection(true);
    
    // Update stats after GC
    UpdateMemoryStats();
}

void UPerf_MemoryManager::OptimizeTextureMemory()
{
    UE_LOG(LogTemp, Warning, TEXT("Perf_MemoryManager: Optimizing texture memory"));
    
    // Force texture streaming optimization
    if (GEngine)
    {
        GEngine->Exec(GetWorld(), TEXT("r.Streaming.PoolSize 512"));
        GEngine->Exec(GetWorld(), TEXT("r.Streaming.MaxTempMemoryAllowed 50"));
    }
}

void UPerf_MemoryManager::OptimizeMeshMemory()
{
    UE_LOG(LogTemp, Warning, TEXT("Perf_MemoryManager: Optimizing mesh memory"));
    
    // Force mesh LOD optimization
    if (GEngine)
    {
        GEngine->Exec(GetWorld(), TEXT("r.ForceLOD 1"));
    }
}

void UPerf_MemoryManager::SetMemoryBudget(float PhysicalBudgetMB, float TextureBudgetMB)
{
    PhysicalMemoryBudgetMB = PhysicalBudgetMB;
    TextureMemoryBudgetMB = TextureBudgetMB;
    
    UE_LOG(LogTemp, Warning, TEXT("Perf_MemoryManager: Memory budget set - Physical: %.1f MB, Texture: %.1f MB"), 
           PhysicalBudgetMB, TextureBudgetMB);
    
    CheckMemoryBudget();
}

bool UPerf_MemoryManager::IsWithinMemoryBudget()
{
    UpdateMemoryStats();
    
    bool bWithinPhysicalBudget = LastMemoryStats.PhysicalMemoryMB <= PhysicalMemoryBudgetMB;
    bool bWithinTextureBudget = LastMemoryStats.TextureMemoryMB <= TextureMemoryBudgetMB;
    
    return bWithinPhysicalBudget && bWithinTextureBudget;
}

void UPerf_MemoryManager::UpdateMemoryStats()
{
    LastMemoryStats.PhysicalMemoryMB = GetPhysicalMemoryUsageMB();
    LastMemoryStats.VirtualMemoryMB = LastMemoryStats.PhysicalMemoryMB * 1.2f; // Estimate
    LastMemoryStats.TextureMemoryMB = GetTextureMemoryUsageMB();
    LastMemoryStats.MeshMemoryMB = GetMeshMemoryUsageMB();
    LastMemoryStats.AudioMemoryMB = GetAudioMemoryUsageMB();
    
    // Count actors and components
    if (UWorld* World = GetWorld())
    {
        LastMemoryStats.ActiveActorCount = 0;
        LastMemoryStats.ActiveComponentCount = 0;
        
        for (ULevel* Level : World->GetLevels())
        {
            if (Level)
            {
                LastMemoryStats.ActiveActorCount += Level->Actors.Num();
                
                for (AActor* Actor : Level->Actors)
                {
                    if (Actor)
                    {
                        LastMemoryStats.ActiveComponentCount += Actor->GetRootComponent() ? 
                            Actor->GetRootComponent()->GetAttachChildren().Num() + 1 : 0;
                    }
                }
            }
        }
    }
    
    if (bIsProfilingActive)
    {
        CheckMemoryBudget();
    }
}

void UPerf_MemoryManager::CheckMemoryBudget()
{
    if (!IsWithinMemoryBudget())
    {
        UE_LOG(LogTemp, Error, TEXT("Perf_MemoryManager: Memory budget exceeded! Physical: %.1f/%.1f MB, Texture: %.1f/%.1f MB"),
               LastMemoryStats.PhysicalMemoryMB, PhysicalMemoryBudgetMB,
               LastMemoryStats.TextureMemoryMB, TextureMemoryBudgetMB);
        
        // Auto-optimize when budget is exceeded
        if (LastMemoryStats.TextureMemoryMB > TextureMemoryBudgetMB)
        {
            OptimizeTextureMemory();
        }
        
        ForceGarbageCollection();
    }
}

float UPerf_MemoryManager::GetPhysicalMemoryUsageMB()
{
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    return static_cast<float>(MemStats.UsedPhysical) / (1024.0f * 1024.0f);
}

float UPerf_MemoryManager::GetTextureMemoryUsageMB()
{
    // Get texture memory from RHI if available
    if (GRHISupportsTextureStreaming)
    {
        return static_cast<float>(GCurrentTextureMemorySize) / (1024.0f * 1024.0f);
    }
    
    return 0.0f;
}

float UPerf_MemoryManager::GetMeshMemoryUsageMB()
{
    // Estimate based on actor count (rough approximation)
    return static_cast<float>(LastMemoryStats.ActiveActorCount) * 0.5f; // 0.5MB per actor average
}

float UPerf_MemoryManager::GetAudioMemoryUsageMB()
{
    // Basic estimate - would need audio subsystem integration for accurate data
    return 64.0f; // 64MB estimate for audio
}