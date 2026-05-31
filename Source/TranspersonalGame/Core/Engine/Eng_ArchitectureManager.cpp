#include "Eng_ArchitectureManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "HAL/PlatformFilemanager.h"
#include "Stats/Stats.h"
#include "Engine/StaticMeshActor.h"

UEng_ArchitectureManager::UEng_ArchitectureManager()
{
    bPerformanceMonitoringActive = false;
    LastHealthCheckTime = 0.0f;
}

void UEng_ArchitectureManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("ArchitectureManager: Initializing system architecture monitoring"));
    
    // Start automatic performance monitoring
    StartPerformanceMonitoring();
    
    // Validate initial system state
    ValidateSystemIntegrity();
    
    // Log initial architecture state
    LogSystemArchitecture();
}

void UEng_ArchitectureManager::Deinitialize()
{
    StopPerformanceMonitoring();
    
    UE_LOG(LogTemp, Warning, TEXT("ArchitectureManager: Shutting down"));
    
    Super::Deinitialize();
}

FEng_SystemPerformanceMetrics UEng_ArchitectureManager::GetCurrentPerformanceMetrics()
{
    UpdatePerformanceMetrics();
    return LastPerformanceMetrics;
}

void UEng_ArchitectureManager::StartPerformanceMonitoring()
{
    bPerformanceMonitoringActive = true;
    UE_LOG(LogTemp, Warning, TEXT("ArchitectureManager: Performance monitoring started"));
}

void UEng_ArchitectureManager::StopPerformanceMonitoring()
{
    bPerformanceMonitoringActive = false;
    UE_LOG(LogTemp, Warning, TEXT("ArchitectureManager: Performance monitoring stopped"));
}

TArray<FEng_ModuleLoadInfo> UEng_ArchitectureManager::GetLoadedModuleInfo()
{
    TArray<FEng_ModuleLoadInfo> ModuleInfo;
    
    // Core modules to check
    TArray<FString> CoreModules = {
        TEXT("TranspersonalGame"),
        TEXT("Engine"),
        TEXT("Core"),
        TEXT("CoreUObject"),
        TEXT("RenderCore"),
        TEXT("RHI")
    };
    
    for (const FString& ModuleName : CoreModules)
    {
        FEng_ModuleLoadInfo Info;
        Info.ModuleName = ModuleName;
        Info.bIsLoaded = FModuleManager::Get().IsModuleLoaded(*ModuleName);
        Info.LoadTime = 0.0f; // Would need profiling data
        Info.ClassCount = 0; // Would need reflection data
        
        ModuleInfo.Add(Info);
    }
    
    LoadedModules = ModuleInfo;
    return ModuleInfo;
}

bool UEng_ArchitectureManager::ValidateSystemIntegrity()
{
    bool bSystemHealthy = true;
    
    // Check memory usage
    if (!CheckMemoryUsage())
    {
        UE_LOG(LogTemp, Error, TEXT("ArchitectureManager: Memory usage validation failed"));
        bSystemHealthy = false;
    }
    
    // Check frame rate
    if (!CheckFrameRate())
    {
        UE_LOG(LogTemp, Error, TEXT("ArchitectureManager: Frame rate validation failed"));
        bSystemHealthy = false;
    }
    
    // Check actor limits
    if (!EnforceActorLimits())
    {
        UE_LOG(LogTemp, Error, TEXT("ArchitectureManager: Actor limit validation failed"));
        bSystemHealthy = false;
    }
    
    if (bSystemHealthy)
    {
        UE_LOG(LogTemp, Warning, TEXT("ArchitectureManager: System integrity validation PASSED"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("ArchitectureManager: System integrity validation FAILED"));
    }
    
    return bSystemHealthy;
}

void UEng_ArchitectureManager::LogSystemArchitecture()
{
    UE_LOG(LogTemp, Warning, TEXT("=== TRANSPERSONAL GAME ARCHITECTURE STATUS ==="));
    
    // Log performance metrics
    FEng_SystemPerformanceMetrics Metrics = GetCurrentPerformanceMetrics();
    UE_LOG(LogTemp, Warning, TEXT("Performance: FrameTime=%.2fms, Actors=%d, Memory=%.1fMB"), 
           Metrics.FrameTime, Metrics.ActorCount, Metrics.MemoryUsageMB);
    
    // Log module status
    TArray<FEng_ModuleLoadInfo> Modules = GetLoadedModuleInfo();
    for (const FEng_ModuleLoadInfo& Module : Modules)
    {
        UE_LOG(LogTemp, Warning, TEXT("Module: %s - %s"), 
               *Module.ModuleName, Module.bIsLoaded ? TEXT("LOADED") : TEXT("NOT LOADED"));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== END ARCHITECTURE STATUS ==="));
}

bool UEng_ArchitectureManager::EnforceActorLimits()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    int32 TotalActors = GetTotalActorCount();
    
    if (TotalActors > MAX_TOTAL_ACTORS)
    {
        UE_LOG(LogTemp, Error, TEXT("ArchitectureManager: CRITICAL - Actor count %d exceeds limit %d"), 
               TotalActors, MAX_TOTAL_ACTORS);
        
        CleanupExcessActors();
        return false;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("ArchitectureManager: Actor count OK - %d/%d"), TotalActors, MAX_TOTAL_ACTORS);
    return true;
}

int32 UEng_ArchitectureManager::GetTotalActorCount() const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return 0;
    }
    
    int32 Count = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        Count++;
    }
    
    return Count;
}

int32 UEng_ArchitectureManager::GetBiomeActorCount(EBiomeType BiomeType) const
{
    // This would need biome position data to implement properly
    // For now, return estimated count
    int32 TotalActors = GetTotalActorCount();
    return TotalActors / 5; // Assume equal distribution across 5 biomes
}

bool UEng_ArchitectureManager::CheckMemoryUsage()
{
    // Get basic memory stats
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    
    float UsedMemoryMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    float TotalMemoryMB = MemStats.TotalPhysical / (1024.0f * 1024.0f);
    
    float MemoryUsagePercent = (UsedMemoryMB / TotalMemoryMB) * 100.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("ArchitectureManager: Memory usage %.1f%% (%.1fMB / %.1fMB)"), 
           MemoryUsagePercent, UsedMemoryMB, TotalMemoryMB);
    
    // Warn if memory usage is high
    if (MemoryUsagePercent > 85.0f)
    {
        UE_LOG(LogTemp, Error, TEXT("ArchitectureManager: HIGH MEMORY USAGE WARNING"));
        return false;
    }
    
    return true;
}

bool UEng_ArchitectureManager::CheckFrameRate()
{
    // Get current frame time
    float CurrentFrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to ms
    
    UE_LOG(LogTemp, Warning, TEXT("ArchitectureManager: Current frame time %.2fms"), CurrentFrameTime);
    
    // Warn if frame time is too high (below 30 FPS)
    if (CurrentFrameTime > 33.33f)
    {
        UE_LOG(LogTemp, Error, TEXT("ArchitectureManager: LOW FRAME RATE WARNING"));
        return false;
    }
    
    return true;
}

void UEng_ArchitectureManager::GenerateArchitectureReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== TRANSPERSONAL GAME ARCHITECTURE REPORT ==="));
    
    // System validation
    bool bSystemHealthy = ValidateSystemIntegrity();
    UE_LOG(LogTemp, Warning, TEXT("System Health: %s"), bSystemHealthy ? TEXT("HEALTHY") : TEXT("ISSUES DETECTED"));
    
    // Performance metrics
    FEng_SystemPerformanceMetrics Metrics = GetCurrentPerformanceMetrics();
    UE_LOG(LogTemp, Warning, TEXT("Performance Summary:"));
    UE_LOG(LogTemp, Warning, TEXT("  Frame Time: %.2fms"), Metrics.FrameTime);
    UE_LOG(LogTemp, Warning, TEXT("  Actor Count: %d/%d"), Metrics.ActorCount, MAX_TOTAL_ACTORS);
    UE_LOG(LogTemp, Warning, TEXT("  Memory Usage: %.1fMB"), Metrics.MemoryUsageMB);
    
    // Module status
    UE_LOG(LogTemp, Warning, TEXT("Module Status:"));
    TArray<FEng_ModuleLoadInfo> Modules = GetLoadedModuleInfo();
    for (const FEng_ModuleLoadInfo& Module : Modules)
    {
        UE_LOG(LogTemp, Warning, TEXT("  %s: %s"), *Module.ModuleName, 
               Module.bIsLoaded ? TEXT("OK") : TEXT("FAILED"));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== END ARCHITECTURE REPORT ==="));
}

void UEng_ArchitectureManager::UpdatePerformanceMetrics()
{
    LastPerformanceMetrics.FrameTime = FApp::GetDeltaTime() * 1000.0f;
    LastPerformanceMetrics.ActorCount = GetTotalActorCount();
    
    // Get memory stats
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    LastPerformanceMetrics.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    LastPerformanceMetrics.GPUMemoryUsageMB = 0.0f; // Would need GPU stats
    
    LastPerformanceMetrics.ComponentCount = 0; // Would need component iteration
}

void UEng_ArchitectureManager::CheckSystemHealth()
{
    if (!bPerformanceMonitoringActive)
    {
        return;
    }
    
    float CurrentTime = FApp::GetCurrentTime();
    if (CurrentTime - LastHealthCheckTime > 5.0f) // Check every 5 seconds
    {
        ValidateSystemIntegrity();
        LastHealthCheckTime = CurrentTime;
    }
}

void UEng_ArchitectureManager::ValidateActorCounts()
{
    int32 TotalActors = GetTotalActorCount();
    
    if (TotalActors > MAX_TOTAL_ACTORS)
    {
        UE_LOG(LogTemp, Error, TEXT("ArchitectureManager: Actor count exceeded - %d/%d"), 
               TotalActors, MAX_TOTAL_ACTORS);
        CleanupExcessActors();
    }
}

void UEng_ArchitectureManager::CleanupExcessActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    TArray<AActor*> ActorsToDestroy;
    
    // Find non-essential actors to remove
    for (TActorIterator<AStaticMeshActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && !Actor->IsA<APawn>() && !Actor->GetName().Contains(TEXT("PlayerStart")))
        {
            ActorsToDestroy.Add(Actor);
            
            // Stop when we have enough to remove
            if (ActorsToDestroy.Num() >= (GetTotalActorCount() - MAX_TOTAL_ACTORS))
            {
                break;
            }
        }
    }
    
    // Destroy excess actors
    for (AActor* Actor : ActorsToDestroy)
    {
        if (Actor)
        {
            Actor->Destroy();
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("ArchitectureManager: Cleaned up %d excess actors"), ActorsToDestroy.Num());
}