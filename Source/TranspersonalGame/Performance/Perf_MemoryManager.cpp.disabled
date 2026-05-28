#include "Perf_MemoryManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "HAL/PlatformMemory.h"
#include "HAL/PlatformProcess.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Animation/SkeletalMeshActor.h"
#include "UObject/UObjectGlobals.h"

UPerf_MemoryManager::UPerf_MemoryManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Check every second
}

void UPerf_MemoryManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Perf_MemoryManager initialized - Auto optimization: %s"), 
           bAutoOptimizeMemory ? TEXT("Enabled") : TEXT("Disabled"));
    
    // Initial memory check
    UpdateMemoryLevel();
}

void UPerf_MemoryManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    LastMemoryCheck += DeltaTime;
    
    if (LastMemoryCheck >= MemoryCheckInterval)
    {
        LastMemoryCheck = 0.0f;
        UpdateMemoryLevel();
        
        if (bAutoOptimizeMemory && CurrentMemoryLevel >= EPerf_MemoryLevel::High)
        {
            HandleMemoryPressure();
        }
    }
}

FPerf_MemoryStats UPerf_MemoryManager::GetCurrentMemoryStats()
{
    FPerf_MemoryStats Stats;
    
    // Get platform memory stats
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    
    Stats.UsedMemoryMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    Stats.AvailableMemoryMB = MemStats.AvailablePhysical / (1024.0f * 1024.0f);
    Stats.MemoryUsagePercent = GetSystemMemoryUsagePercent();
    Stats.MemoryLevel = CurrentMemoryLevel;
    
    // Count active objects
    if (UWorld* World = GetWorld())
    {
        Stats.ActiveActorCount = World->GetActorCount();
        
        // Count components
        int32 ComponentCount = 0;
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor)
            {
                ComponentCount += Actor->GetRootComponent() ? Actor->GetRootComponent()->GetAttachChildren().Num() + 1 : 0;
            }
        }
        Stats.ActiveComponentCount = ComponentCount;
    }
    
    return Stats;
}

void UPerf_MemoryManager::ForceGarbageCollection()
{
    UE_LOG(LogTemp, Warning, TEXT("Perf_MemoryManager: Forcing garbage collection"));
    
    CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
    
    float MemoryAfter = GetSystemMemoryUsagePercent();
    UE_LOG(LogTemp, Log, TEXT("Garbage collection complete - Memory usage: %.1f%%"), MemoryAfter);
}

void UPerf_MemoryManager::OptimizeMemoryUsage()
{
    float MemoryBefore = GetSystemMemoryUsagePercent();
    
    UE_LOG(LogTemp, Warning, TEXT("Perf_MemoryManager: Starting memory optimization"));
    
    // Clear unused assets
    ClearUnusedAssets();
    
    // Reduce LOD if memory is critical
    if (CurrentMemoryLevel == EPerf_MemoryLevel::Critical)
    {
        ReduceActorLOD();
        DisableNonEssentialComponents();
    }
    
    // Force garbage collection
    if (bAutoGarbageCollection)
    {
        ForceGarbageCollection();
    }
    
    float MemoryAfter = GetSystemMemoryUsagePercent();
    float MemorySaved = MemoryBefore - MemoryAfter;
    
    if (MemorySaved > 0.0f)
    {
        OnMemoryOptimized.Broadcast(MemorySaved);
        UE_LOG(LogTemp, Log, TEXT("Memory optimization complete - Saved %.1f%% memory"), MemorySaved);
    }
}

void UPerf_MemoryManager::ClearUnusedAssets()
{
    UE_LOG(LogTemp, Log, TEXT("Clearing unused assets"));
    
    // This would normally clear asset caches and unused textures
    // In a real implementation, we'd interface with the asset manager
    CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
}

void UPerf_MemoryManager::ReduceActorLOD()
{
    UE_LOG(LogTemp, Log, TEXT("Reducing actor LOD levels"));
    
    UWorld* World = GetWorld();
    if (!World) return;
    
    int32 OptimizedActors = 0;
    
    for (TActorIterator<AStaticMeshActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AStaticMeshActor* MeshActor = *ActorItr;
        if (MeshActor && MeshActor->GetStaticMeshComponent())
        {
            UStaticMeshComponent* MeshComp = MeshActor->GetStaticMeshComponent();
            
            // Force lower LOD
            MeshComp->SetForcedLodModel(2); // Use LOD level 2
            OptimizedActors++;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Reduced LOD for %d static mesh actors"), OptimizedActors);
}

void UPerf_MemoryManager::DisableNonEssentialComponents()
{
    UE_LOG(LogTemp, Log, TEXT("Disabling non-essential components"));
    
    UWorld* World = GetWorld();
    if (!World) return;
    
    int32 DisabledComponents = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor) continue;
        
        // Disable particle systems and audio components on distant actors
        FVector PlayerLocation = FVector::ZeroVector;
        if (APawn* PlayerPawn = World->GetFirstPlayerController()->GetPawn())
        {
            PlayerLocation = PlayerPawn->GetActorLocation();
        }
        
        float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerLocation);
        if (Distance > 5000.0f) // 50 meters
        {
            // Disable tick on distant actors
            Actor->SetActorTickEnabled(false);
            DisabledComponents++;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Disabled tick on %d distant actors"), DisabledComponents);
}

void UPerf_MemoryManager::UpdateMemoryLevel()
{
    float MemoryUsage = GetSystemMemoryUsagePercent();
    EPerf_MemoryLevel NewLevel;
    
    if (MemoryUsage >= MemoryCriticalThreshold)
    {
        NewLevel = EPerf_MemoryLevel::Critical;
    }
    else if (MemoryUsage >= MemoryWarningThreshold)
    {
        NewLevel = EPerf_MemoryLevel::High;
    }
    else if (MemoryUsage >= 50.0f)
    {
        NewLevel = EPerf_MemoryLevel::Medium;
    }
    else
    {
        NewLevel = EPerf_MemoryLevel::Low;
    }
    
    if (NewLevel != CurrentMemoryLevel)
    {
        CurrentMemoryLevel = NewLevel;
        OnMemoryLevelChanged.Broadcast(CurrentMemoryLevel);
        
        UE_LOG(LogTemp, Log, TEXT("Memory level changed to: %d (%.1f%% usage)"), 
               (int32)CurrentMemoryLevel, MemoryUsage);
    }
}

void UPerf_MemoryManager::HandleMemoryPressure()
{
    UE_LOG(LogTemp, Warning, TEXT("Handling memory pressure - Level: %d"), (int32)CurrentMemoryLevel);
    
    switch (CurrentMemoryLevel)
    {
        case EPerf_MemoryLevel::High:
            ClearUnusedAssets();
            break;
            
        case EPerf_MemoryLevel::Critical:
            OptimizeMemoryUsage();
            break;
            
        default:
            break;
    }
}

float UPerf_MemoryManager::GetSystemMemoryUsagePercent()
{
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    
    if (MemStats.TotalPhysical > 0)
    {
        return (float(MemStats.UsedPhysical) / float(MemStats.TotalPhysical)) * 100.0f;
    }
    
    return 0.0f;
}