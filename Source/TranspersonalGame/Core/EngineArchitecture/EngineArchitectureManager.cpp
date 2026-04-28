#include "EngineArchitectureManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Stats/Stats.h"
#include "TimerManager.h"
#include "Engine/GameInstance.h"

UEngineArchitectureManager::UEngineArchitectureManager()
{
    // Set default performance thresholds
    MaxAllowedFrameTime = 33.33f; // 30 FPS minimum
    MaxAllowedActorCount = 10000;
    MaxMemoryUsageMB = 4096.0f; // 4GB limit
    
    LastFrameTime = 0.0f;
    LastActorCount = 0;
}

void UEngineArchitectureManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architecture Manager Initialized"));
    
    // Register core modules
    RegisterSystemModule(TEXT("Core"), 100);
    RegisterSystemModule(TEXT("Physics"), 90);
    RegisterSystemModule(TEXT("Rendering"), 80);
    RegisterSystemModule(TEXT("Audio"), 70);
    RegisterSystemModule(TEXT("AI"), 60);
    RegisterSystemModule(TEXT("UI"), 50);
    
    // Start performance monitoring
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            PerformanceUpdateTimer,
            this,
            &UEngineArchitectureManager::UpdatePerformanceMetrics,
            1.0f,
            true
        );
        
        World->GetTimerManager().SetTimer(
            ValidationTimer,
            this,
            &UEngineArchitectureManager::ValidatePerformanceThresholds,
            5.0f,
            true
        );
    }
    
    LogArchitectureEvent(TEXT("Architecture Manager Started"));
}

void UEngineArchitectureManager::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(PerformanceUpdateTimer);
        World->GetTimerManager().ClearTimer(ValidationTimer);
    }
    
    LogArchitectureEvent(TEXT("Architecture Manager Shutdown"));
    Super::Deinitialize();
}

bool UEngineArchitectureManager::ValidateSystemDependencies()
{
    // Check that core modules are registered
    TArray<FString> RequiredModules = {
        TEXT("Core"),
        TEXT("Physics"),
        TEXT("Rendering")
    };
    
    for (const FString& Module : RequiredModules)
    {
        if (!IsSystemModuleRegistered(Module))
        {
            UE_LOG(LogTemp, Error, TEXT("Required module not registered: %s"), *Module);
            return false;
        }
    }
    
    LogArchitectureEvent(TEXT("System Dependencies Validated"));
    return true;
}

void UEngineArchitectureManager::RegisterSystemModule(const FString& ModuleName, int32 Priority)
{
    RegisteredModules.Add(ModuleName, Priority);
    LogArchitectureEvent(FString::Printf(TEXT("Module Registered: %s (Priority: %d)"), *ModuleName, Priority));
}

bool UEngineArchitectureManager::IsSystemModuleRegistered(const FString& ModuleName) const
{
    return RegisteredModules.Contains(ModuleName);
}

float UEngineArchitectureManager::GetCurrentFrameTime() const
{
    if (GEngine && GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull))
    {
        return GEngine->GetMaxFPS() > 0 ? 1000.0f / GEngine->GetMaxFPS() : LastFrameTime;
    }
    return LastFrameTime;
}

int32 UEngineArchitectureManager::GetActiveActorCount() const
{
    if (UWorld* World = GetWorld())
    {
        return World->GetActorCount();
    }
    return LastActorCount;
}

FString UEngineArchitectureManager::GetPerformanceReport() const
{
    FString Report = TEXT("=== PERFORMANCE REPORT ===\n");
    Report += FString::Printf(TEXT("Frame Time: %.2f ms\n"), GetCurrentFrameTime());
    Report += FString::Printf(TEXT("Actor Count: %d\n"), GetActiveActorCount());
    Report += FString::Printf(TEXT("Memory Usage: %.2f MB\n"), GetMemoryUsageMB());
    Report += FString::Printf(TEXT("Registered Modules: %d\n"), RegisteredModules.Num());
    
    Report += TEXT("\nModule Registry:\n");
    for (const auto& Module : RegisteredModules)
    {
        Report += FString::Printf(TEXT("  %s (Priority: %d)\n"), *Module.Key, Module.Value);
    }
    
    return Report;
}

void UEngineArchitectureManager::ForceGarbageCollection()
{
    GEngine->ForceGarbageCollection(true);
    LogArchitectureEvent(TEXT("Forced Garbage Collection"));
}

float UEngineArchitectureManager::GetMemoryUsageMB() const
{
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    return MemStats.UsedPhysical / (1024.0f * 1024.0f);
}

void UEngineArchitectureManager::NotifySystemEvent(const FString& SystemName, const FString& EventType)
{
    FString EventString = FString::Printf(TEXT("[%s] %s"), *SystemName, *EventType);
    SystemEventLog.Add(EventString);
    
    // Keep log size manageable
    if (SystemEventLog.Num() > 1000)
    {
        SystemEventLog.RemoveAt(0, 100);
    }
    
    LogArchitectureEvent(EventString);
}

TArray<FString> UEngineArchitectureManager::GetActiveSystemModules() const
{
    TArray<FString> ModuleNames;
    RegisteredModules.GetKeys(ModuleNames);
    return ModuleNames;
}

void UEngineArchitectureManager::ValidatePerformanceThresholds()
{
    float CurrentFrameTime = GetCurrentFrameTime();
    int32 CurrentActorCount = GetActiveActorCount();
    float CurrentMemoryMB = GetMemoryUsageMB();
    
    // Check frame time threshold
    if (CurrentFrameTime > MaxAllowedFrameTime)
    {
        UE_LOG(LogTemp, Warning, TEXT("Frame time exceeded threshold: %.2f ms (max: %.2f ms)"), 
               CurrentFrameTime, MaxAllowedFrameTime);
        NotifySystemEvent(TEXT("Performance"), TEXT("Frame Time Threshold Exceeded"));
    }
    
    // Check actor count threshold
    if (CurrentActorCount > MaxAllowedActorCount)
    {
        UE_LOG(LogTemp, Warning, TEXT("Actor count exceeded threshold: %d (max: %d)"), 
               CurrentActorCount, MaxAllowedActorCount);
        NotifySystemEvent(TEXT("Performance"), TEXT("Actor Count Threshold Exceeded"));
    }
    
    // Check memory threshold
    if (CurrentMemoryMB > MaxMemoryUsageMB)
    {
        UE_LOG(LogTemp, Warning, TEXT("Memory usage exceeded threshold: %.2f MB (max: %.2f MB)"), 
               CurrentMemoryMB, MaxMemoryUsageMB);
        NotifySystemEvent(TEXT("Performance"), TEXT("Memory Threshold Exceeded"));
    }
}

void UEngineArchitectureManager::UpdatePerformanceMetrics()
{
    LastFrameTime = GetCurrentFrameTime();
    LastActorCount = GetActiveActorCount();
}

void UEngineArchitectureManager::LogArchitectureEvent(const FString& Event)
{
    UE_LOG(LogTemp, Log, TEXT("Engine Architecture: %s"), *Event);
}