#include "Perf_MemoryProfiler.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "HAL/PlatformMemory.h"
#include "Stats/Stats.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "UObject/UObjectGlobals.h"
#include "Engine/GarbageCollection.h"

UPerf_MemoryProfiler::UPerf_MemoryProfiler()
{
    bIsProfilingActive = false;
    MemoryWarningThresholdMB = 6144.0f; // 6GB warning
    MemoryCriticalThresholdMB = 7168.0f; // 7GB critical
    MemoryHistory.Reserve(1000);
}

FPerf_MemoryStats UPerf_MemoryProfiler::GetCurrentMemoryStats()
{
    FPerf_MemoryStats Stats;
    
    // Get platform memory stats
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    
    Stats.UsedPhysicalMemoryMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    Stats.UsedVirtualMemoryMB = MemStats.UsedVirtual / (1024.0f * 1024.0f);
    Stats.PeakUsedPhysicalMemoryMB = MemStats.PeakUsedPhysical / (1024.0f * 1024.0f);
    Stats.AvailablePhysicalMemoryMB = MemStats.AvailablePhysical / (1024.0f * 1024.0f);
    
    // Get game-specific stats
    Stats.ActiveActorCount = CountActiveActors();
    Stats.ActiveComponentCount = CountActiveComponents();
    Stats.TextureMemoryMB = GetTextureMemoryUsage();
    Stats.MeshMemoryMB = GetMeshMemoryUsage();
    
    LastRecordedStats = Stats;
    return Stats;
}

void UPerf_MemoryProfiler::StartMemoryProfiling()
{
    if (!bIsProfilingActive)
    {
        bIsProfilingActive = true;
        MemoryHistory.Empty();
        UE_LOG(LogTemp, Warning, TEXT("Memory profiling started"));
        
        // Start periodic memory updates
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().SetTimer(
                FTimerHandle(),
                this,
                &UPerf_MemoryProfiler::UpdateMemoryStats,
                1.0f, // Every second
                true
            );
        }
    }
}

void UPerf_MemoryProfiler::StopMemoryProfiling()
{
    if (bIsProfilingActive)
    {
        bIsProfilingActive = false;
        UE_LOG(LogTemp, Warning, TEXT("Memory profiling stopped. Recorded %d samples"), MemoryHistory.Num());
        
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().ClearAllTimersForObject(this);
        }
    }
}

EPerf_MemoryWarningLevel UPerf_MemoryProfiler::GetMemoryWarningLevel()
{
    FPerf_MemoryStats CurrentStats = GetCurrentMemoryStats();
    
    if (CurrentStats.UsedPhysicalMemoryMB >= MemoryCriticalThresholdMB)
    {
        return EPerf_MemoryWarningLevel::Critical;
    }
    else if (CurrentStats.UsedPhysicalMemoryMB >= MemoryWarningThresholdMB)
    {
        return EPerf_MemoryWarningLevel::High;
    }
    else if (CurrentStats.UsedPhysicalMemoryMB >= MemoryWarningThresholdMB * 0.8f)
    {
        return EPerf_MemoryWarningLevel::Medium;
    }
    else if (CurrentStats.UsedPhysicalMemoryMB >= MemoryWarningThresholdMB * 0.6f)
    {
        return EPerf_MemoryWarningLevel::Low;
    }
    
    return EPerf_MemoryWarningLevel::None;
}

void UPerf_MemoryProfiler::ForceGarbageCollection()
{
    UE_LOG(LogTemp, Warning, TEXT("Forcing garbage collection..."));
    
    // Force full GC
    GEngine->ForceGarbageCollection(true);
    
    // Log memory improvement
    FPerf_MemoryStats StatsAfterGC = GetCurrentMemoryStats();
    UE_LOG(LogTemp, Warning, TEXT("GC Complete. Memory usage: %.1f MB"), StatsAfterGC.UsedPhysicalMemoryMB);
}

void UPerf_MemoryProfiler::OptimizeMemoryUsage()
{
    UE_LOG(LogTemp, Warning, TEXT("Starting memory optimization..."));
    
    CleanupUnusedAssets();
    OptimizeTextureMemory();
    OptimizeMeshMemory();
    ForceGarbageCollection();
    
    UE_LOG(LogTemp, Warning, TEXT("Memory optimization complete"));
}

TArray<FString> UPerf_MemoryProfiler::GetMemoryHogs()
{
    TArray<FString> MemoryHogs;
    
    // Analyze textures
    float LargestTextureSize = 0.0f;
    FString LargestTextureName;
    
    for (TObjectIterator<UTexture2D> TextureItr; TextureItr; ++TextureItr)
    {
        UTexture2D* Texture = *TextureItr;
        if (IsValid(Texture))
        {
            float TextureSize = Texture->CalcTextureMemorySizeEnum(TMC_ResidentMips) / (1024.0f * 1024.0f);
            if (TextureSize > LargestTextureSize)
            {
                LargestTextureSize = TextureSize;
                LargestTextureName = Texture->GetName();
            }
        }
    }
    
    if (LargestTextureSize > 50.0f) // More than 50MB
    {
        MemoryHogs.Add(FString::Printf(TEXT("Large Texture: %s (%.1f MB)"), *LargestTextureName, LargestTextureSize));
    }
    
    // Analyze meshes
    float LargestMeshSize = 0.0f;
    FString LargestMeshName;
    
    for (TObjectIterator<UStaticMesh> MeshItr; MeshItr; ++MeshItr)
    {
        UStaticMesh* Mesh = *MeshItr;
        if (IsValid(Mesh))
        {
            float MeshSize = Mesh->GetResourceSizeBytes(EResourceSizeMode::EstimatedTotal) / (1024.0f * 1024.0f);
            if (MeshSize > LargestMeshSize)
            {
                LargestMeshSize = MeshSize;
                LargestMeshName = Mesh->GetName();
            }
        }
    }
    
    if (LargestMeshSize > 20.0f) // More than 20MB
    {
        MemoryHogs.Add(FString::Printf(TEXT("Large Mesh: %s (%.1f MB)"), *LargestMeshName, LargestMeshSize));
    }
    
    return MemoryHogs;
}

void UPerf_MemoryProfiler::ProfileCurrentLevel()
{
    UE_LOG(LogTemp, Warning, TEXT("=== MEMORY PROFILE REPORT ==="));
    
    FPerf_MemoryStats Stats = GetCurrentMemoryStats();
    
    UE_LOG(LogTemp, Warning, TEXT("Physical Memory: %.1f MB used, %.1f MB available"), 
        Stats.UsedPhysicalMemoryMB, Stats.AvailablePhysicalMemoryMB);
    UE_LOG(LogTemp, Warning, TEXT("Virtual Memory: %.1f MB"), Stats.UsedVirtualMemoryMB);
    UE_LOG(LogTemp, Warning, TEXT("Peak Physical: %.1f MB"), Stats.PeakUsedPhysicalMemoryMB);
    UE_LOG(LogTemp, Warning, TEXT("Active Actors: %d"), Stats.ActiveActorCount);
    UE_LOG(LogTemp, Warning, TEXT("Active Components: %d"), Stats.ActiveComponentCount);
    UE_LOG(LogTemp, Warning, TEXT("Texture Memory: %.1f MB"), Stats.TextureMemoryMB);
    UE_LOG(LogTemp, Warning, TEXT("Mesh Memory: %.1f MB"), Stats.MeshMemoryMB);
    
    TArray<FString> MemoryHogs = GetMemoryHogs();
    if (MemoryHogs.Num() > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Memory Hogs:"));
        for (const FString& Hog : MemoryHogs)
        {
            UE_LOG(LogTemp, Warning, TEXT("  - %s"), *Hog);
        }
    }
    
    EPerf_MemoryWarningLevel WarningLevel = GetMemoryWarningLevel();
    if (WarningLevel != EPerf_MemoryWarningLevel::None)
    {
        LogMemoryWarning(WarningLevel);
    }
}

void UPerf_MemoryProfiler::UpdateMemoryStats()
{
    if (bIsProfilingActive)
    {
        FPerf_MemoryStats CurrentStats = GetCurrentMemoryStats();
        MemoryHistory.Add(CurrentStats);
        
        // Keep only last 1000 samples
        if (MemoryHistory.Num() > 1000)
        {
            MemoryHistory.RemoveAt(0);
        }
        
        CheckMemoryThresholds();
    }
}

void UPerf_MemoryProfiler::CheckMemoryThresholds()
{
    EPerf_MemoryWarningLevel WarningLevel = GetMemoryWarningLevel();
    
    if (WarningLevel == EPerf_MemoryWarningLevel::Critical)
    {
        LogMemoryWarning(WarningLevel);
        OptimizeMemoryUsage();
    }
    else if (WarningLevel == EPerf_MemoryWarningLevel::High)
    {
        LogMemoryWarning(WarningLevel);
        ForceGarbageCollection();
    }
}

void UPerf_MemoryProfiler::LogMemoryWarning(EPerf_MemoryWarningLevel WarningLevel)
{
    FString WarningText;
    switch (WarningLevel)
    {
        case EPerf_MemoryWarningLevel::Low:
            WarningText = TEXT("LOW");
            break;
        case EPerf_MemoryWarningLevel::Medium:
            WarningText = TEXT("MEDIUM");
            break;
        case EPerf_MemoryWarningLevel::High:
            WarningText = TEXT("HIGH");
            break;
        case EPerf_MemoryWarningLevel::Critical:
            WarningText = TEXT("CRITICAL");
            break;
        default:
            return;
    }
    
    UE_LOG(LogTemp, Error, TEXT("MEMORY WARNING [%s]: %.1f MB used"), 
        *WarningText, LastRecordedStats.UsedPhysicalMemoryMB);
}

void UPerf_MemoryProfiler::CleanupUnusedAssets()
{
    // Mark unused assets for garbage collection
    CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
}

void UPerf_MemoryProfiler::OptimizeTextureMemory()
{
    // This would implement texture streaming optimization
    // For now, just log the action
    UE_LOG(LogTemp, Warning, TEXT("Optimizing texture memory usage..."));
}

void UPerf_MemoryProfiler::OptimizeMeshMemory()
{
    // This would implement mesh LOD optimization
    // For now, just log the action
    UE_LOG(LogTemp, Warning, TEXT("Optimizing mesh memory usage..."));
}

void UPerf_MemoryProfiler::AnalyzeActorMemoryUsage()
{
    if (UWorld* World = GetWorld())
    {
        int32 TotalActors = 0;
        int32 TotalComponents = 0;
        
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (IsValid(Actor))
            {
                TotalActors++;
                TotalComponents += Actor->GetRootComponent() ? Actor->GetRootComponent()->GetAttachChildren().Num() + 1 : 0;
            }
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Actor Analysis: %d actors, %d components"), TotalActors, TotalComponents);
    }
}

float UPerf_MemoryProfiler::GetTextureMemoryUsage()
{
    float TotalTextureMemory = 0.0f;
    
    for (TObjectIterator<UTexture2D> TextureItr; TextureItr; ++TextureItr)
    {
        UTexture2D* Texture = *TextureItr;
        if (IsValid(Texture))
        {
            TotalTextureMemory += Texture->CalcTextureMemorySizeEnum(TMC_ResidentMips);
        }
    }
    
    return TotalTextureMemory / (1024.0f * 1024.0f);
}

float UPerf_MemoryProfiler::GetMeshMemoryUsage()
{
    float TotalMeshMemory = 0.0f;
    
    for (TObjectIterator<UStaticMesh> MeshItr; MeshItr; ++MeshItr)
    {
        UStaticMesh* Mesh = *MeshItr;
        if (IsValid(Mesh))
        {
            TotalMeshMemory += Mesh->GetResourceSizeBytes(EResourceSizeMode::EstimatedTotal);
        }
    }
    
    return TotalMeshMemory / (1024.0f * 1024.0f);
}

int32 UPerf_MemoryProfiler::CountActiveActors()
{
    int32 ActorCount = 0;
    
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            if (IsValid(*ActorItr))
            {
                ActorCount++;
            }
        }
    }
    
    return ActorCount;
}

int32 UPerf_MemoryProfiler::CountActiveComponents()
{
    int32 ComponentCount = 0;
    
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (IsValid(Actor))
            {
                TArray<UActorComponent*> Components;
                Actor->GetComponents<UActorComponent>(Components);
                ComponentCount += Components.Num();
            }
        }
    }
    
    return ComponentCount;
}