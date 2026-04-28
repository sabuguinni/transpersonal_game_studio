#include "EngineArchitectureCore.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"

UEngineArchitectureCore::UEngineArchitectureCore()
{
    bSystemHealthy = false;
    CurrentFrameTime = 0.0f;
    ActiveActorCount = 0;
    MemoryUsageMB = 0.0f;
    bWorldPartitionHealthy = false;
    bActorSystemsHealthy = false;
    bMemoryConstraintsHealthy = false;
    bPerformanceTargetsHealthy = false;
    LastPerformanceUpdate = FDateTime::Now();
}

void UEngineArchitectureCore::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureCore: Initializing core architecture systems"));
    
    InitializeArchitecture();
}

void UEngineArchitectureCore::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureCore: Shutting down architecture systems"));
    
    RegisteredSystems.Empty();
    SystemWarnings.Empty();
    SystemErrors.Empty();
    FrameTimeHistory.Empty();
    
    Super::Deinitialize();
}

void UEngineArchitectureCore::InitializeArchitecture()
{
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureCore: Starting architecture initialization"));
    
    // Clear previous state
    SystemWarnings.Empty();
    SystemErrors.Empty();
    
    // Initialize core systems
    ValidateSystemIntegrity();
    UpdatePerformanceMetrics();
    
    bSystemHealthy = bWorldPartitionHealthy && bActorSystemsHealthy && 
                     bMemoryConstraintsHealthy && bPerformanceTargetsHealthy;
    
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureCore: Architecture initialization complete. System healthy: %s"), 
           bSystemHealthy ? TEXT("YES") : TEXT("NO"));
}

void UEngineArchitectureCore::ValidateSystemIntegrity()
{
    UE_LOG(LogTemp, Log, TEXT("EngineArchitectureCore: Validating system integrity"));
    
    ValidateWorldPartition();
    ValidateActorSystems();
    ValidateMemoryConstraints();
    ValidatePerformanceTargets();
    
    // Log validation results
    if (SystemErrors.Num() > 0)
    {
        UE_LOG(LogTemp, Error, TEXT("EngineArchitectureCore: Found %d system errors"), SystemErrors.Num());
        for (const FString& Error : SystemErrors)
        {
            UE_LOG(LogTemp, Error, TEXT("  ERROR: %s"), *Error);
        }
    }
    
    if (SystemWarnings.Num() > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureCore: Found %d system warnings"), SystemWarnings.Num());
        for (const FString& Warning : SystemWarnings)
        {
            UE_LOG(LogTemp, Warning, TEXT("  WARNING: %s"), *Warning);
        }
    }
}

void UEngineArchitectureCore::UpdatePerformanceMetrics()
{
    FDateTime CurrentTime = FDateTime::Now();
    float DeltaTime = (CurrentTime - LastPerformanceUpdate).GetTotalSeconds();
    
    if (DeltaTime >= 0.016f) // Update at ~60Hz
    {
        // Update frame time
        if (GEngine && GEngine->GetWorldContexts().Num() > 0)
        {
            UWorld* World = GEngine->GetWorldContexts()[0].World();
            if (World)
            {
                CurrentFrameTime = World->GetDeltaSeconds();
                
                // Track frame time history
                FrameTimeHistory.Add(CurrentFrameTime);
                if (FrameTimeHistory.Num() > MaxFrameTimeHistory)
                {
                    FrameTimeHistory.RemoveAt(0);
                }
                
                // Count active actors
                ActiveActorCount = World->GetActorCount();
            }
        }
        
        // Update memory usage (simplified)
        FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
        MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
        
        LastPerformanceUpdate = CurrentTime;
    }
}

void UEngineArchitectureCore::OptimizeMemoryUsage()
{
    UE_LOG(LogTemp, Log, TEXT("EngineArchitectureCore: Running memory optimization"));
    
    // Force garbage collection
    if (GEngine)
    {
        GEngine->ForceGarbageCollection(true);
    }
    
    // Update memory metrics after optimization
    UpdatePerformanceMetrics();
    
    UE_LOG(LogTemp, Log, TEXT("EngineArchitectureCore: Memory optimization complete. Current usage: %.2f MB"), MemoryUsageMB);
}

void UEngineArchitectureCore::RegisterCoreSystem(const FString& SystemName, UObject* SystemObject)
{
    if (!SystemObject)
    {
        UE_LOG(LogTemp, Error, TEXT("EngineArchitectureCore: Cannot register null system: %s"), *SystemName);
        return;
    }
    
    RegisteredSystems.Add(SystemName, SystemObject);
    UE_LOG(LogTemp, Log, TEXT("EngineArchitectureCore: Registered system: %s"), *SystemName);
}

void UEngineArchitectureCore::UnregisterCoreSystem(const FString& SystemName)
{
    if (RegisteredSystems.Remove(SystemName) > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("EngineArchitectureCore: Unregistered system: %s"), *SystemName);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureCore: System not found for unregistration: %s"), *SystemName);
    }
}

UObject* UEngineArchitectureCore::GetRegisteredSystem(const FString& SystemName) const
{
    if (TObjectPtr<UObject> const* SystemPtr = RegisteredSystems.Find(SystemName))
    {
        return *SystemPtr;
    }
    return nullptr;
}

void UEngineArchitectureCore::ValidateWorldPartition()
{
    bWorldPartitionHealthy = true;
    
    if (GEngine && GEngine->GetWorldContexts().Num() > 0)
    {
        UWorld* World = GEngine->GetWorldContexts()[0].World();
        if (World)
        {
            // Check if world partition is enabled for large worlds
            if (ActiveActorCount > 1000)
            {
                // In a real implementation, we'd check if World Partition is actually enabled
                SystemWarnings.Add(TEXT("Large world detected - consider enabling World Partition"));
            }
        }
        else
        {
            SystemErrors.Add(TEXT("No valid world context found"));
            bWorldPartitionHealthy = false;
        }
    }
    else
    {
        SystemErrors.Add(TEXT("No world contexts available"));
        bWorldPartitionHealthy = false;
    }
}

void UEngineArchitectureCore::ValidateActorSystems()
{
    bActorSystemsHealthy = true;
    
    if (ActiveActorCount > 10000)
    {
        SystemWarnings.Add(TEXT("High actor count detected - performance may be impacted"));
    }
    
    if (ActiveActorCount == 0)
    {
        SystemWarnings.Add(TEXT("No actors found in current level"));
    }
}

void UEngineArchitectureCore::ValidateMemoryConstraints()
{
    bMemoryConstraintsHealthy = true;
    
    // Check memory usage thresholds
    if (MemoryUsageMB > 8192.0f) // 8GB threshold
    {
        SystemErrors.Add(TEXT("Memory usage exceeds 8GB threshold"));
        bMemoryConstraintsHealthy = false;
    }
    else if (MemoryUsageMB > 4096.0f) // 4GB warning
    {
        SystemWarnings.Add(TEXT("Memory usage exceeds 4GB - monitor closely"));
    }
}

void UEngineArchitectureCore::ValidatePerformanceTargets()
{
    bPerformanceTargetsHealthy = true;
    
    if (FrameTimeHistory.Num() > 10)
    {
        // Calculate average frame time
        float TotalFrameTime = 0.0f;
        for (float FrameTime : FrameTimeHistory)
        {
            TotalFrameTime += FrameTime;
        }
        float AverageFrameTime = TotalFrameTime / FrameTimeHistory.Num();
        
        // Check performance targets (60 FPS = 16.67ms, 30 FPS = 33.33ms)
        if (AverageFrameTime > 0.0333f) // Worse than 30 FPS
        {
            SystemErrors.Add(TEXT("Performance below 30 FPS target"));
            bPerformanceTargetsHealthy = false;
        }
        else if (AverageFrameTime > 0.0167f) // Worse than 60 FPS
        {
            SystemWarnings.Add(TEXT("Performance below 60 FPS target"));
        }
    }
}