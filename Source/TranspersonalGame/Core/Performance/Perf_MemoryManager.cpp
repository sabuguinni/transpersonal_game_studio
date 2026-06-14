#include "Perf_MemoryManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameViewportClient.h"
#include "HAL/PlatformMemory.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/App.h"
#include "RenderingThread.h"
#include "AudioDevice.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Animation/AnimSequence.h"
#include "Materials/MaterialInterface.h"
#include "Engine/Texture.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Sound/SoundBase.h"

UPerf_MemoryManager::UPerf_MemoryManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Update every second by default
    
    MonitoringInterval = 1.0f;
    bAutoOptimizeMemory = true;
    bLogMemoryWarnings = true;
    GCTriggerThreshold = 0.85f;
    
    bIsMonitoring = false;
    bIsProfiling = false;
    LastMonitorTime = 0.0f;
    LastGCTime = 0.0f;
    
    // Set reasonable default budgets
    MemoryBudget.MaxTextureMemoryMB = 2048.0f;
    MemoryBudget.MaxMeshMemoryMB = 1024.0f;
    MemoryBudget.MaxAudioMemoryMB = 512.0f;
    MemoryBudget.MaxAnimationMemoryMB = 256.0f;
    MemoryBudget.MaxActiveActors = 8000;
    MemoryBudget.MaxActiveComponents = 50000;
    MemoryBudget.MemoryWarningThreshold = 0.75f;
    MemoryBudget.MemoryCriticalThreshold = 0.9f;
}

void UPerf_MemoryManager::BeginPlay()
{
    Super::BeginPlay();
    
    bIsMonitoring = true;
    UpdateMemoryStats();
    
    UE_LOG(LogTemp, Log, TEXT("PerformanceOptimizer: Memory Manager initialized"));
    UE_LOG(LogTemp, Log, TEXT("Memory Budget - Textures: %.0fMB, Meshes: %.0fMB, Audio: %.0fMB"), 
           MemoryBudget.MaxTextureMemoryMB, MemoryBudget.MaxMeshMemoryMB, MemoryBudget.MaxAudioMemoryMB);
}

void UPerf_MemoryManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIsMonitoring)
        return;
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastMonitorTime >= MonitoringInterval)
    {
        UpdateMemoryStats();
        CheckMemoryThresholds();
        
        if (bAutoOptimizeMemory)
        {
            AutoOptimizeIfNeeded();
        }
        
        LastMonitorTime = CurrentTime;
    }
}

FPerf_MemoryStats UPerf_MemoryManager::GetCurrentMemoryStats()
{
    UpdateMemoryStats();
    return CurrentStats;
}

void UPerf_MemoryManager::UpdateMemoryStats()
{
    FPlatformMemoryStats PlatformStats = FPlatformMemory::GetStats();
    
    CurrentStats.PhysicalMemoryUsedMB = (PlatformStats.TotalPhysical - PlatformStats.AvailablePhysical) / (1024.0f * 1024.0f);
    CurrentStats.VirtualMemoryUsedMB = (PlatformStats.TotalVirtual - PlatformStats.AvailableVirtual) / (1024.0f * 1024.0f);
    CurrentStats.AvailablePhysicalMB = PlatformStats.AvailablePhysical / (1024.0f * 1024.0f);
    
    // Estimate memory usage by asset type
    CurrentStats.TextureMemoryMB = 0.0f;
    CurrentStats.MeshMemoryMB = 0.0f;
    CurrentStats.AudioMemoryMB = 0.0f;
    CurrentStats.AnimationMemoryMB = 0.0f;
    
    // Count active actors and components
    if (UWorld* World = GetWorld())
    {
        CurrentStats.ActiveActorCount = 0;
        CurrentStats.ActiveComponentCount = 0;
        
        for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
        {
            AActor* Actor = *ActorIterator;
            if (IsValid(Actor))
            {
                CurrentStats.ActiveActorCount++;
                CurrentStats.ActiveComponentCount += Actor->GetRootComponent() ? Actor->GetRootComponent()->GetAttachChildren().Num() + 1 : 0;
                
                // Estimate memory usage from components
                TArray<UStaticMeshComponent*> StaticMeshComps;
                Actor->GetComponents<UStaticMeshComponent>(StaticMeshComps);
                for (UStaticMeshComponent* MeshComp : StaticMeshComps)
                {
                    if (MeshComp && MeshComp->GetStaticMesh())
                    {
                        CurrentStats.MeshMemoryMB += 2.0f; // Rough estimate per mesh
                    }
                }
                
                TArray<USkeletalMeshComponent*> SkelMeshComps;
                Actor->GetComponents<USkeletalMeshComponent>(SkelMeshComps);
                for (USkeletalMeshComponent* SkelComp : SkelMeshComps)
                {
                    if (SkelComp && SkelComp->GetSkeletalMeshAsset())
                    {
                        CurrentStats.MeshMemoryMB += 5.0f; // Skeletal meshes are larger
                        CurrentStats.AnimationMemoryMB += 1.0f; // Animation data
                    }
                }
                
                TArray<UAudioComponent*> AudioComps;
                Actor->GetComponents<UAudioComponent>(AudioComps);
                CurrentStats.AudioMemoryMB += AudioComps.Num() * 0.5f; // Rough estimate per audio component
            }
        }
    }
    
    CurrentStats.MemoryPressureLevel = CalculateMemoryPressure();
    
    // Store in history for trending
    MemoryHistory.Add(CurrentStats);
    if (MemoryHistory.Num() > 60) // Keep last 60 samples (1 minute at 1Hz)
    {
        MemoryHistory.RemoveAt(0);
    }
}

float UPerf_MemoryManager::CalculateMemoryPressure()
{
    float TexturePressure = CurrentStats.TextureMemoryMB / MemoryBudget.MaxTextureMemoryMB;
    float MeshPressure = CurrentStats.MeshMemoryMB / MemoryBudget.MaxMeshMemoryMB;
    float AudioPressure = CurrentStats.AudioMemoryMB / MemoryBudget.MaxAudioMemoryMB;
    float ActorPressure = (float)CurrentStats.ActiveActorCount / (float)MemoryBudget.MaxActiveActors;
    
    return FMath::Max({TexturePressure, MeshPressure, AudioPressure, ActorPressure});
}

float UPerf_MemoryManager::GetMemoryPressureLevel()
{
    return CurrentStats.MemoryPressureLevel;
}

bool UPerf_MemoryManager::IsMemoryUnderPressure()
{
    return CurrentStats.MemoryPressureLevel >= MemoryBudget.MemoryWarningThreshold;
}

bool UPerf_MemoryManager::IsMemoryCritical()
{
    return CurrentStats.MemoryPressureLevel >= MemoryBudget.MemoryCriticalThreshold;
}

void UPerf_MemoryManager::CheckMemoryThresholds()
{
    if (IsMemoryCritical())
    {
        LogMemoryWarning(TEXT("CRITICAL: Memory usage is critical! Triggering aggressive optimization."));
        TriggerGarbageCollection();
        OptimizeTextureMemory();
        CullDistantActors(5000.0f);
    }
    else if (IsMemoryUnderPressure())
    {
        LogMemoryWarning(TEXT("WARNING: Memory usage is high. Consider optimization."));
        if (GetWorld()->GetTimeSeconds() - LastGCTime > 30.0f) // GC every 30 seconds max
        {
            TriggerGarbageCollection();
        }
    }
}

void UPerf_MemoryManager::AutoOptimizeIfNeeded()
{
    if (IsMemoryUnderPressure())
    {
        // Gentle optimization when under pressure
        ClearUnusedAssets();
        OptimizeActorComponents();
    }
    
    if (IsMemoryCritical())
    {
        // Aggressive optimization when critical
        OptimizeTextureMemory();
        OptimizeMeshMemory();
        CullDistantActors(3000.0f);
        CullActorsByPriority(EPerf_MemoryPriority::Low);
    }
}

void UPerf_MemoryManager::TriggerGarbageCollection()
{
    UE_LOG(LogTemp, Warning, TEXT("PerformanceOptimizer: Triggering garbage collection"));
    GEngine->ForceGarbageCollection(true);
    LastGCTime = GetWorld()->GetTimeSeconds();
}

void UPerf_MemoryManager::OptimizeTextureMemory()
{
    UE_LOG(LogTemp, Log, TEXT("PerformanceOptimizer: Optimizing texture memory"));
    
    // Force texture streaming optimization
    if (GEngine && GEngine->GetGameViewport())
    {
        // Trigger texture streaming update
        IStreamingManager::Get().UpdateResourceStreaming(0.0f, true);
    }
}

void UPerf_MemoryManager::OptimizeMeshMemory()
{
    UE_LOG(LogTemp, Log, TEXT("PerformanceOptimizer: Optimizing mesh memory"));
    
    // This would typically involve LOD adjustments and mesh streaming
    // For now, we'll just log the intent
}

void UPerf_MemoryManager::OptimizeAudioMemory()
{
    UE_LOG(LogTemp, Log, TEXT("PerformanceOptimizer: Optimizing audio memory"));
    
    if (UWorld* World = GetWorld())
    {
        if (FAudioDevice* AudioDevice = World->GetAudioDeviceRaw())
        {
            // Stop distant audio sources
            AudioDevice->StopAllSounds(true);
        }
    }
}

void UPerf_MemoryManager::ClearUnusedAssets()
{
    UE_LOG(LogTemp, Log, TEXT("PerformanceOptimizer: Clearing unused assets"));
    
    // This would involve asset registry cleanup
    // For now, trigger GC which handles most unused assets
    TriggerGarbageCollection();
}

void UPerf_MemoryManager::CullDistantActors(float CullDistance)
{
    if (!GetWorld())
        return;
    
    APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
    if (!PlayerPawn)
        return;
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    int32 CulledCount = 0;
    
    for (TActorIterator<AActor> ActorIterator(GetWorld()); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (IsValid(Actor) && Actor != PlayerPawn)
        {
            float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerLocation);
            if (Distance > CullDistance)
            {
                // Don't destroy essential actors
                if (!Actor->IsA<APlayerController>() && !Actor->IsA<AGameModeBase>())
                {
                    Actor->SetActorHiddenInGame(true);
                    Actor->SetActorEnableCollision(false);
                    CulledCount++;
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("PerformanceOptimizer: Culled %d distant actors beyond %.0f units"), CulledCount, CullDistance);
}

void UPerf_MemoryManager::CullActorsByPriority(EPerf_MemoryPriority MinPriority)
{
    // This would require actors to have priority metadata
    // For now, just log the intent
    UE_LOG(LogTemp, Log, TEXT("PerformanceOptimizer: Culling actors by priority (min: %d)"), (int32)MinPriority);
}

void UPerf_MemoryManager::OptimizeActorComponents()
{
    if (!GetWorld())
        return;
    
    int32 OptimizedCount = 0;
    
    for (TActorIterator<AActor> ActorIterator(GetWorld()); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (IsValid(Actor))
        {
            // Disable tick on distant or inactive components
            TArray<UActorComponent*> Components;
            Actor->GetComponents<UActorComponent>(Components);
            
            for (UActorComponent* Component : Components)
            {
                if (Component && Component->PrimaryComponentTick.bCanEverTick)
                {
                    // Simple distance-based optimization
                    if (APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn())
                    {
                        float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerPawn->GetActorLocation());
                        if (Distance > 2000.0f)
                        {
                            Component->SetComponentTickEnabled(false);
                            OptimizedCount++;
                        }
                    }
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("PerformanceOptimizer: Optimized %d actor components"), OptimizedCount);
}

void UPerf_MemoryManager::SetMemoryBudget(const FPerf_MemoryBudget& NewBudget)
{
    MemoryBudget = NewBudget;
    UE_LOG(LogTemp, Log, TEXT("PerformanceOptimizer: Memory budget updated"));
}

FPerf_MemoryBudget UPerf_MemoryManager::GetMemoryBudget() const
{
    return MemoryBudget;
}

bool UPerf_MemoryManager::IsWithinMemoryBudget()
{
    return CurrentStats.TextureMemoryMB <= MemoryBudget.MaxTextureMemoryMB &&
           CurrentStats.MeshMemoryMB <= MemoryBudget.MaxMeshMemoryMB &&
           CurrentStats.AudioMemoryMB <= MemoryBudget.MaxAudioMemoryMB &&
           CurrentStats.ActiveActorCount <= MemoryBudget.MaxActiveActors;
}

void UPerf_MemoryManager::OptimizeWorldPartitionStreaming()
{
    UE_LOG(LogTemp, Log, TEXT("PerformanceOptimizer: Optimizing World Partition streaming"));
    
    // This would involve World Partition specific optimizations
    // For now, just trigger general streaming optimization
    if (IStreamingManager::Get_Concurrent())
    {
        IStreamingManager::Get().UpdateResourceStreaming(0.0f, true);
    }
}

void UPerf_MemoryManager::PreloadCriticalAssets()
{
    UE_LOG(LogTemp, Log, TEXT("PerformanceOptimizer: Preloading critical assets"));
    
    // This would involve loading essential game assets
    // Implementation would depend on specific asset management strategy
}

void UPerf_MemoryManager::UnloadDistantAssets(float UnloadDistance)
{
    UE_LOG(LogTemp, Log, TEXT("PerformanceOptimizer: Unloading assets beyond %.0f units"), UnloadDistance);
    
    // This would involve asset streaming management
    // For now, just log the intent
}

void UPerf_MemoryManager::LogMemoryReport()
{
    UpdateMemoryStats();
    
    UE_LOG(LogTemp, Warning, TEXT("=== MEMORY REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Physical Memory Used: %.2f MB"), CurrentStats.PhysicalMemoryUsedMB);
    UE_LOG(LogTemp, Warning, TEXT("Available Physical: %.2f MB"), CurrentStats.AvailablePhysicalMB);
    UE_LOG(LogTemp, Warning, TEXT("Texture Memory: %.2f MB / %.2f MB"), CurrentStats.TextureMemoryMB, MemoryBudget.MaxTextureMemoryMB);
    UE_LOG(LogTemp, Warning, TEXT("Mesh Memory: %.2f MB / %.2f MB"), CurrentStats.MeshMemoryMB, MemoryBudget.MaxMeshMemoryMB);
    UE_LOG(LogTemp, Warning, TEXT("Audio Memory: %.2f MB / %.2f MB"), CurrentStats.AudioMemoryMB, MemoryBudget.MaxAudioMemoryMB);
    UE_LOG(LogTemp, Warning, TEXT("Active Actors: %d / %d"), CurrentStats.ActiveActorCount, MemoryBudget.MaxActiveActors);
    UE_LOG(LogTemp, Warning, TEXT("Active Components: %d"), CurrentStats.ActiveComponentCount);
    UE_LOG(LogTemp, Warning, TEXT("Memory Pressure: %.2f%%"), CurrentStats.MemoryPressureLevel * 100.0f);
    UE_LOG(LogTemp, Warning, TEXT("=================="));
}

TArray<FString> UPerf_MemoryManager::GetMemoryHotspots()
{
    TArray<FString> Hotspots;
    
    if (CurrentStats.TextureMemoryMB / MemoryBudget.MaxTextureMemoryMB > 0.8f)
    {
        Hotspots.Add(TEXT("High texture memory usage"));
    }
    
    if (CurrentStats.MeshMemoryMB / MemoryBudget.MaxMeshMemoryMB > 0.8f)
    {
        Hotspots.Add(TEXT("High mesh memory usage"));
    }
    
    if (CurrentStats.ActiveActorCount > MemoryBudget.MaxActiveActors * 0.8f)
    {
        Hotspots.Add(TEXT("High actor count"));
    }
    
    if (CurrentStats.ActiveComponentCount > MemoryBudget.MaxActiveComponents * 0.8f)
    {
        Hotspots.Add(TEXT("High component count"));
    }
    
    return Hotspots;
}

void UPerf_MemoryManager::StartMemoryProfiling()
{
    bIsProfiling = true;
    MemoryHistory.Empty();
    UE_LOG(LogTemp, Log, TEXT("PerformanceOptimizer: Memory profiling started"));
}

void UPerf_MemoryManager::StopMemoryProfiling()
{
    bIsProfiling = false;
    UE_LOG(LogTemp, Log, TEXT("PerformanceOptimizer: Memory profiling stopped. Collected %d samples"), MemoryHistory.Num());
}

void UPerf_MemoryManager::LogMemoryWarning(const FString& Warning)
{
    if (bLogMemoryWarnings)
    {
        UE_LOG(LogTemp, Warning, TEXT("PerformanceOptimizer: %s"), *Warning);
    }
}