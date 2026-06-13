#include "Perf_MemoryManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "HAL/PlatformMemory.h"
#include "HAL/PlatformFilemanager.h"
#include "Engine/Texture.h"
#include "Engine/StaticMesh.h"
#include "Sound/SoundBase.h"

UPerf_MemoryManager::UPerf_MemoryManager()
{
    bIsMonitoring = false;
    MemoryPressureThreshold = 0.85f; // 85% memory usage triggers pressure
    bAutoOptimizeEnabled = true;
    bWasHighPressure = false;
    LastGCTime = 0.0f;
}

void UPerf_MemoryManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // Start monitoring automatically
    StartMemoryMonitoring();
    
    UE_LOG(LogTemp, Log, TEXT("MemoryManager: Initialized"));
}

void UPerf_MemoryManager::Deinitialize()
{
    StopMemoryMonitoring();
    Super::Deinitialize();
}

bool UPerf_MemoryManager::ShouldCreateSubsystem(UObject* Outer) const
{
    return true;
}

FPerf_MemoryStats UPerf_MemoryManager::GetCurrentMemoryStats()
{
    UpdateMemoryStats();
    return CurrentStats;
}

void UPerf_MemoryManager::StartMemoryMonitoring()
{
    if (bIsMonitoring)
        return;
        
    bIsMonitoring = true;
    
    UGameInstance* GameInstance = GetGameInstance();
    if (GameInstance && GameInstance->GetWorld())
    {
        GameInstance->GetWorld()->GetTimerManager().SetTimer(
            MemoryUpdateTimer,
            this,
            &UPerf_MemoryManager::UpdateMemoryStats,
            2.0f, // Update every 2 seconds
            true
        );
    }
    
    UE_LOG(LogTemp, Log, TEXT("MemoryManager: Started memory monitoring"));
}

void UPerf_MemoryManager::StopMemoryMonitoring()
{
    if (!bIsMonitoring)
        return;
        
    bIsMonitoring = false;
    
    UGameInstance* GameInstance = GetGameInstance();
    if (GameInstance && GameInstance->GetWorld())
    {
        GameInstance->GetWorld()->GetTimerManager().ClearTimer(MemoryUpdateTimer);
    }
    
    UE_LOG(LogTemp, Log, TEXT("MemoryManager: Stopped memory monitoring"));
}

bool UPerf_MemoryManager::IsMemoryPressureHigh() const
{
    return CurrentStats.bIsMemoryPressureHigh;
}

void UPerf_MemoryManager::ForceGarbageCollection()
{
    float CurrentTime = FPlatformTime::Seconds();
    
    // Prevent too frequent GC calls
    if (CurrentTime - LastGCTime < 5.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("MemoryManager: GC called too recently, skipping"));
        return;
    }
    
    LastGCTime = CurrentTime;
    
    UE_LOG(LogTemp, Log, TEXT("MemoryManager: Forcing garbage collection"));
    
    // Force full garbage collection
    GEngine->ForceGarbageCollection(true);
    
    // Update stats after GC
    UpdateMemoryStats();
}

void UPerf_MemoryManager::SetMemoryPressureThreshold(float Threshold)
{
    MemoryPressureThreshold = FMath::Clamp(Threshold, 0.5f, 0.95f);
    UE_LOG(LogTemp, Log, TEXT("MemoryManager: Pressure threshold set to %.2f"), MemoryPressureThreshold);
}

void UPerf_MemoryManager::OptimizeMemoryUsage()
{
    UE_LOG(LogTemp, Log, TEXT("MemoryManager: Starting memory optimization"));
    
    // Clear unused assets
    ClearUnusedAssets();
    
    // Force garbage collection
    ForceGarbageCollection();
    
    // Flush rendering commands
    if (GEngine && GEngine->GetWorld())
    {
        FlushRenderingCommands();
    }
    
    UE_LOG(LogTemp, Log, TEXT("MemoryManager: Memory optimization complete"));
}

float UPerf_MemoryManager::GetMemoryUsageByCategory(EPerf_MemoryCategory Category)
{
    if (CategoryMemoryUsage.Contains(Category))
    {
        return CategoryMemoryUsage[Category];
    }
    return 0.0f;
}

void UPerf_MemoryManager::UpdateMemoryStats()
{
    // Get platform memory stats
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    
    CurrentStats.TotalPhysicalMemoryMB = MemStats.TotalPhysical / (1024.0f * 1024.0f);
    CurrentStats.UsedPhysicalMemoryMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    CurrentStats.AvailablePhysicalMemoryMB = MemStats.AvailablePhysical / (1024.0f * 1024.0f);
    
    // Calculate game memory usage (approximation)
    CurrentStats.GameMemoryUsageMB = (MemStats.UsedPhysical - MemStats.AvailablePhysical) / (1024.0f * 1024.0f);
    
    // Calculate memory pressure
    if (CurrentStats.TotalPhysicalMemoryMB > 0)
    {
        CurrentStats.MemoryPressureLevel = CurrentStats.UsedPhysicalMemoryMB / CurrentStats.TotalPhysicalMemoryMB;
    }
    
    // Update category usage (simplified estimates)
    CategoryMemoryUsage[EPerf_MemoryCategory::Textures] = CurrentStats.TextureMemoryMB;
    CategoryMemoryUsage[EPerf_MemoryCategory::Meshes] = CurrentStats.MeshMemoryMB;
    CategoryMemoryUsage[EPerf_MemoryCategory::Audio] = CurrentStats.AudioMemoryMB;
    
    CheckMemoryPressure();
}

void UPerf_MemoryManager::CheckMemoryPressure()
{
    bool bHighPressure = CurrentStats.MemoryPressureLevel > MemoryPressureThreshold;
    
    if (bHighPressure != bWasHighPressure)
    {
        CurrentStats.bIsMemoryPressureHigh = bHighPressure;
        bWasHighPressure = bHighPressure;
        
        // Broadcast pressure change
        OnMemoryPressureChanged.Broadcast(bHighPressure);
        
        if (bHighPressure)
        {
            UE_LOG(LogTemp, Warning, TEXT("MemoryManager: High memory pressure detected (%.1f%%)"), 
                   CurrentStats.MemoryPressureLevel * 100.0f);
                   
            if (bAutoOptimizeEnabled)
            {
                HandleMemoryPressure();
            }
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("MemoryManager: Memory pressure normalized (%.1f%%)"), 
                   CurrentStats.MemoryPressureLevel * 100.0f);
        }
    }
}

void UPerf_MemoryManager::HandleMemoryPressure()
{
    UE_LOG(LogTemp, Warning, TEXT("MemoryManager: Handling memory pressure"));
    
    // Step 1: Clear unused assets
    ClearUnusedAssets();
    
    // Step 2: Force garbage collection
    ForceGarbageCollection();
    
    // Step 3: Flush rendering commands
    FlushRenderingCommands();
    
    // Step 4: Reduce texture quality if still high pressure
    UpdateMemoryStats();
    if (CurrentStats.bIsMemoryPressureHigh)
    {
        UE_LOG(LogTemp, Warning, TEXT("MemoryManager: Still high pressure after cleanup, considering quality reduction"));
    }
}

void UPerf_MemoryManager::ClearUnusedAssets()
{
    UE_LOG(LogTemp, Log, TEXT("MemoryManager: Clearing unused assets"));
    
    // This is a simplified implementation
    // In a full implementation, you would:
    // 1. Identify unused textures, meshes, sounds
    // 2. Unload them from memory
    // 3. Clear asset caches
    
    // For now, just trigger asset cleanup
    if (GEngine)
    {
        GEngine->TrimMemory();
    }
}