#include "Eng_CoreSystemsManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "HAL/PlatformMemory.h"
#include "Stats/Stats.h"

UEng_CoreSystemsManager::UEng_CoreSystemsManager()
{
    CurrentStatus = EEng_SystemStatus::Uninitialized;
    CurrentMetrics = FEng_SystemMetrics();
}

void UEng_CoreSystemsManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    CurrentStatus = EEng_SystemStatus::Initializing;
    
    // Initialize core systems
    InitializeCoreSystems();
    
    // Start metrics update timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            MetricsUpdateTimer,
            this,
            &UEng_CoreSystemsManager::InternalMetricsUpdate,
            1.0f,
            true
        );
    }
    
    CurrentStatus = EEng_SystemStatus::Active;
    
    UE_LOG(LogTemp, Warning, TEXT("CoreSystemsManager: Initialized successfully"));
}

void UEng_CoreSystemsManager::Deinitialize()
{
    CurrentStatus = EEng_SystemStatus::Shutdown;
    
    // Clear timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(MetricsUpdateTimer);
    }
    
    // Shutdown all registered systems
    ShutdownCoreSystems();
    
    Super::Deinitialize();
}

bool UEng_CoreSystemsManager::ShouldCreateSubsystem(UObject* Outer) const
{
    return true;
}

void UEng_CoreSystemsManager::InitializeCoreSystems()
{
    if (CurrentStatus != EEng_SystemStatus::Initializing && CurrentStatus != EEng_SystemStatus::Uninitialized)
    {
        return;
    }
    
    // Validate system dependencies
    if (!CheckSystemDependencies())
    {
        CurrentStatus = EEng_SystemStatus::Error;
        UE_LOG(LogTemp, Error, TEXT("CoreSystemsManager: System dependencies validation failed"));
        return;
    }
    
    // Initialize metrics
    UpdatePerformanceMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("CoreSystemsManager: Core systems initialized"));
}

void UEng_CoreSystemsManager::ShutdownCoreSystems()
{
    // Unregister all systems
    for (auto& SystemPair : RegisteredSystems)
    {
        if (SystemPair.Value.IsValid())
        {
            UE_LOG(LogTemp, Warning, TEXT("CoreSystemsManager: Shutting down system: %s"), *SystemPair.Key);
        }
    }
    
    RegisteredSystems.Empty();
    CurrentStatus = EEng_SystemStatus::Shutdown;
}

EEng_SystemStatus UEng_CoreSystemsManager::GetSystemStatus() const
{
    return CurrentStatus;
}

FEng_SystemMetrics UEng_CoreSystemsManager::GetSystemMetrics() const
{
    return CurrentMetrics;
}

void UEng_CoreSystemsManager::UpdatePerformanceMetrics()
{
    if (UWorld* World = GetWorld())
    {
        // Update frame time
        CurrentMetrics.FrameTime = FApp::GetDeltaTime();
        
        // Update actor count
        CurrentMetrics.ActorCount = World->GetCurrentLevel()->Actors.Num();
        
        // Update memory usage (approximate)
        FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
        CurrentMetrics.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
        
        // Update status
        CurrentMetrics.Status = CurrentStatus;
    }
}

bool UEng_CoreSystemsManager::ValidateSystemIntegrity()
{
    // Validate all registered systems are still valid
    ValidateRegisteredSystems();
    
    // Check performance thresholds
    if (CurrentMetrics.FrameTime > 0.033f) // 30 FPS threshold
    {
        UE_LOG(LogTemp, Warning, TEXT("CoreSystemsManager: Performance warning - Frame time: %f"), CurrentMetrics.FrameTime);
    }
    
    if (CurrentMetrics.ActorCount > 8000) // Actor count threshold
    {
        UE_LOG(LogTemp, Warning, TEXT("CoreSystemsManager: Performance warning - Actor count: %d"), CurrentMetrics.ActorCount);
        return false;
    }
    
    return true;
}

void UEng_CoreSystemsManager::OptimizeSystemPerformance()
{
    // Trigger garbage collection if memory usage is high
    if (CurrentMetrics.MemoryUsageMB > 2000.0f) // 2GB threshold
    {
        GEngine->ForceGarbageCollection(true);
        UE_LOG(LogTemp, Warning, TEXT("CoreSystemsManager: Forced garbage collection due to high memory usage"));
    }
    
    // Validate system integrity
    if (!ValidateSystemIntegrity())
    {
        UE_LOG(LogTemp, Warning, TEXT("CoreSystemsManager: System integrity validation failed - performance optimization needed"));
    }
}

void UEng_CoreSystemsManager::RegisterCoreSystem(const FString& SystemName, UObject* SystemInstance)
{
    if (!SystemInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("CoreSystemsManager: Cannot register null system: %s"), *SystemName);
        return;
    }
    
    RegisteredSystems.Add(SystemName, SystemInstance);
    UE_LOG(LogTemp, Warning, TEXT("CoreSystemsManager: Registered system: %s"), *SystemName);
}

void UEng_CoreSystemsManager::UnregisterCoreSystem(const FString& SystemName)
{
    if (RegisteredSystems.Remove(SystemName) > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("CoreSystemsManager: Unregistered system: %s"), *SystemName);
    }
}

UObject* UEng_CoreSystemsManager::GetRegisteredSystem(const FString& SystemName)
{
    if (TWeakObjectPtr<UObject>* SystemPtr = RegisteredSystems.Find(SystemName))
    {
        return SystemPtr->IsValid() ? SystemPtr->Get() : nullptr;
    }
    return nullptr;
}

void UEng_CoreSystemsManager::InternalMetricsUpdate()
{
    UpdatePerformanceMetrics();
    
    // Periodic system validation
    if (!ValidateSystemIntegrity())
    {
        OptimizeSystemPerformance();
    }
}

void UEng_CoreSystemsManager::ValidateRegisteredSystems()
{
    TArray<FString> InvalidSystems;
    
    for (auto& SystemPair : RegisteredSystems)
    {
        if (!SystemPair.Value.IsValid())
        {
            InvalidSystems.Add(SystemPair.Key);
        }
    }
    
    // Remove invalid systems
    for (const FString& InvalidSystem : InvalidSystems)
    {
        RegisteredSystems.Remove(InvalidSystem);
        UE_LOG(LogTemp, Warning, TEXT("CoreSystemsManager: Removed invalid system: %s"), *InvalidSystem);
    }
}

bool UEng_CoreSystemsManager::CheckSystemDependencies()
{
    // Validate essential UE5 systems are available
    if (!GEngine)
    {
        UE_LOG(LogTemp, Error, TEXT("CoreSystemsManager: GEngine not available"));
        return false;
    }
    
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("CoreSystemsManager: World not available"));
        return false;
    }
    
    return true;
}