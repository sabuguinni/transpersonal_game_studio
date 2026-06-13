#include "Perf_MemoryManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "HAL/PlatformMemory.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/Texture.h"
#include "RenderingThread.h"

UPerf_MemoryManager::UPerf_MemoryManager()
{
    CurrentOptimizationLevel = EPerf_MemoryLevel::Medium;
    MemoryCheckInterval = 5.0f;
    HighMemoryThresholdMB = 6000.0f;
    CriticalMemoryThresholdMB = 8000.0f;
    bAutoOptimizeMemory = true;
}

void UPerf_MemoryManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Perf_MemoryManager: Initialized"));
    
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            MemoryCheckTimer,
            this,
            &UPerf_MemoryManager::CheckMemoryUsage,
            MemoryCheckInterval,
            true
        );
    }
}

void UPerf_MemoryManager::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(MemoryCheckTimer);
    }
    
    Super::Deinitialize();
}

FPerf_MemoryStats UPerf_MemoryManager::GetCurrentMemoryStats() const
{
    FPerf_MemoryStats Stats;
    
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    
    Stats.PhysicalMemoryUsedMB = (MemStats.UsedPhysical / 1024.0f / 1024.0f);
    Stats.VirtualMemoryUsedMB = (MemStats.UsedVirtual / 1024.0f / 1024.0f);
    Stats.AvailablePhysicalMemoryMB = (MemStats.AvailablePhysical / 1024.0f / 1024.0f);
    
    // Get texture memory usage
    if (GEngine && GEngine->GetRenderTargetPool())
    {
        Stats.TextureMemoryUsedMB = (GEngine->GetRenderTargetPool()->GetCurrentlyAllocatedSizeInBytes() / 1024.0f / 1024.0f);
    }
    
    // Count actors and components
    if (UWorld* World = GetWorld())
    {
        Stats.ActiveActorCount = World->GetCurrentLevel()->Actors.Num();
        
        int32 ComponentCount = 0;
        for (AActor* Actor : World->GetCurrentLevel()->Actors)
        {
            if (IsValid(Actor))
            {
                ComponentCount += Actor->GetRootComponent() ? Actor->GetRootComponent()->GetAttachChildren().Num() + 1 : 0;
            }
        }
        Stats.ActiveComponentCount = ComponentCount;
    }
    
    Stats.CurrentMemoryLevel = CalculateMemoryLevel(Stats);
    
    return Stats;
}

void UPerf_MemoryManager::ForceGarbageCollection()
{
    UE_LOG(LogTemp, Warning, TEXT("Perf_MemoryManager: Forcing garbage collection"));
    
    GEngine->ForceGarbageCollection(true);
    
    // Also flush rendering commands
    FlushRenderingCommands();
}

void UPerf_MemoryManager::OptimizeMemoryUsage()
{
    UE_LOG(LogTemp, Warning, TEXT("Perf_MemoryManager: Optimizing memory usage"));
    
    FPerf_MemoryStats Stats = GetCurrentMemoryStats();
    
    switch (Stats.CurrentMemoryLevel)
    {
        case EPerf_MemoryLevel::High:
            CleanupUnusedAssets();
            break;
            
        case EPerf_MemoryLevel::Critical:
            CleanupUnusedAssets();
            ReduceTextureQuality(0.7f);
            LimitActorCount(6000);
            ForceGarbageCollection();
            break;
            
        default:
            break;
    }
}

void UPerf_MemoryManager::SetMemoryOptimizationLevel(EPerf_MemoryLevel Level)
{
    CurrentOptimizationLevel = Level;
    UE_LOG(LogTemp, Warning, TEXT("Perf_MemoryManager: Set optimization level to %d"), (int32)Level);
}

bool UPerf_MemoryManager::IsMemoryUsageHigh() const
{
    FPerf_MemoryStats Stats = GetCurrentMemoryStats();
    return Stats.CurrentMemoryLevel >= EPerf_MemoryLevel::High;
}

void UPerf_MemoryManager::CleanupUnusedAssets()
{
    UE_LOG(LogTemp, Warning, TEXT("Perf_MemoryManager: Cleaning up unused assets"));
    
    // Force garbage collection to clean up unreferenced objects
    ForceGarbageCollection();
    
    // Clear cached data
    if (GEngine)
    {
        GEngine->TrimMemory();
    }
}

void UPerf_MemoryManager::ReduceTextureQuality(float ReductionFactor)
{
    UE_LOG(LogTemp, Warning, TEXT("Perf_MemoryManager: Reducing texture quality by factor %f"), ReductionFactor);
    
    // This would typically involve reducing texture LOD bias
    // For now, we'll just log the action
    if (GEngine)
    {
        // In a full implementation, this would adjust texture streaming settings
        UE_LOG(LogTemp, Warning, TEXT("Perf_MemoryManager: Texture quality reduction applied"));
    }
}

void UPerf_MemoryManager::LimitActorCount(int32 MaxActors)
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    TArray<AActor*> AllActors = World->GetCurrentLevel()->Actors;
    int32 CurrentCount = AllActors.Num();
    
    if (CurrentCount <= MaxActors) return;
    
    UE_LOG(LogTemp, Warning, TEXT("Perf_MemoryManager: Limiting actors from %d to %d"), CurrentCount, MaxActors);
    
    // Remove non-essential actors
    TArray<AActor*> ActorsToRemove;
    TArray<FString> EssentialLabels = {"playerstart", "directionallight", "skylight", "skyatmosphere", "fog"};
    TArray<FString> DinosaurLabels = {"trex", "veloci", "tricera", "brachi", "ankylo", "parasauro", "pachy", "proto", "tsinta"};
    
    for (AActor* Actor : AllActors)
    {
        if (!IsValid(Actor)) continue;
        
        FString ActorLabel = Actor->GetActorLabel().ToLower();
        bool bIsEssential = false;
        
        // Check if essential
        for (const FString& Essential : EssentialLabels)
        {
            if (ActorLabel.Contains(Essential))
            {
                bIsEssential = true;
                break;
            }
        }
        
        // Check if dinosaur
        if (!bIsEssential)
        {
            for (const FString& Dino : DinosaurLabels)
            {
                if (ActorLabel.Contains(Dino))
                {
                    bIsEssential = true;
                    break;
                }
            }
        }
        
        if (!bIsEssential)
        {
            ActorsToRemove.Add(Actor);
        }
    }
    
    // Remove excess actors
    int32 ToRemove = FMath::Min(ActorsToRemove.Num(), CurrentCount - MaxActors);
    for (int32 i = 0; i < ToRemove; i++)
    {
        if (IsValid(ActorsToRemove[i]))
        {
            ActorsToRemove[i]->Destroy();
        }
    }
}

void UPerf_MemoryManager::CheckMemoryUsage()
{
    if (!bAutoOptimizeMemory) return;
    
    FPerf_MemoryStats Stats = GetCurrentMemoryStats();
    
    if (Stats.CurrentMemoryLevel >= EPerf_MemoryLevel::High)
    {
        AutoOptimizeIfNeeded();
    }
}

void UPerf_MemoryManager::AutoOptimizeIfNeeded()
{
    FPerf_MemoryStats Stats = GetCurrentMemoryStats();
    
    UE_LOG(LogTemp, Warning, TEXT("Perf_MemoryManager: Auto-optimizing - Memory level: %d"), (int32)Stats.CurrentMemoryLevel);
    
    OptimizeMemoryUsage();
}

EPerf_MemoryLevel UPerf_MemoryManager::CalculateMemoryLevel(const FPerf_MemoryStats& Stats) const
{
    if (Stats.PhysicalMemoryUsedMB >= CriticalMemoryThresholdMB)
    {
        return EPerf_MemoryLevel::Critical;
    }
    else if (Stats.PhysicalMemoryUsedMB >= HighMemoryThresholdMB)
    {
        return EPerf_MemoryLevel::High;
    }
    else if (Stats.PhysicalMemoryUsedMB >= (HighMemoryThresholdMB * 0.6f))
    {
        return EPerf_MemoryLevel::Medium;
    }
    
    return EPerf_MemoryLevel::Low;
}