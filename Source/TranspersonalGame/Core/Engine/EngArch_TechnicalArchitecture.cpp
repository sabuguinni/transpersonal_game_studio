#include "EngArch_TechnicalArchitecture.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Stats/Stats.h"
#include "Engine/GameViewportClient.h"

UEngArch_TechnicalArchitecture::UEngArch_TechnicalArchitecture()
{
    // Initialize default performance metrics
    CachedMetrics = FEngArch_PerformanceMetrics();
}

void UEngArch_TechnicalArchitecture::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architecture subsystem initialized"));
    
    // Register core systems
    RegisterSystem(TEXT("Physics"), EEngArch_SystemPriority::Critical);
    RegisterSystem(TEXT("Rendering"), EEngArch_SystemPriority::Critical);
    RegisterSystem(TEXT("Audio"), EEngArch_SystemPriority::High);
    RegisterSystem(TEXT("AI"), EEngArch_SystemPriority::High);
    RegisterSystem(TEXT("WorldGeneration"), EEngArch_SystemPriority::Medium);
    RegisterSystem(TEXT("UI"), EEngArch_SystemPriority::Medium);
    
    // Start performance monitoring
    UpdatePerformanceMetrics();
}

void UEngArch_TechnicalArchitecture::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architecture subsystem shutting down"));
    
    ShutdownAllSystems();
    RegisteredSystems.Empty();
    SystemErrors.Empty();
    
    Super::Deinitialize();
}

void UEngArch_TechnicalArchitecture::RegisterSystem(const FString& SystemName, EEngArch_SystemPriority Priority)
{
    if (RegisteredSystems.Contains(SystemName))
    {
        UE_LOG(LogTemp, Warning, TEXT("System %s already registered"), *SystemName);
        return;
    }
    
    FEngArch_SystemInfo NewSystem;
    NewSystem.SystemName = SystemName;
    NewSystem.Priority = Priority;
    NewSystem.State = EEngArch_ModuleState::Uninitialized;
    NewSystem.InitializationTime = 0.0f;
    NewSystem.MemoryUsageMB = 0;
    
    RegisteredSystems.Add(SystemName, NewSystem);
    
    UE_LOG(LogTemp, Log, TEXT("Registered system: %s with priority %d"), *SystemName, (int32)Priority);
}

void UEngArch_TechnicalArchitecture::SetSystemState(const FString& SystemName, EEngArch_ModuleState State)
{
    if (!RegisteredSystems.Contains(SystemName))
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot set state for unregistered system: %s"), *SystemName);
        return;
    }
    
    FEngArch_SystemInfo& SystemInfo = RegisteredSystems[SystemName];
    EEngArch_ModuleState OldState = SystemInfo.State;
    SystemInfo.State = State;
    
    if (State == EEngArch_ModuleState::Error)
    {
        FString ErrorMessage = FString::Printf(TEXT("System %s entered error state"), *SystemName);
        SystemErrors.AddUnique(ErrorMessage);
        UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorMessage);
    }
    
    UE_LOG(LogTemp, Log, TEXT("System %s state changed from %d to %d"), *SystemName, (int32)OldState, (int32)State);
}

EEngArch_ModuleState UEngArch_TechnicalArchitecture::GetSystemState(const FString& SystemName) const
{
    if (const FEngArch_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemName))
    {
        return SystemInfo->State;
    }
    
    return EEngArch_ModuleState::Uninitialized;
}

TArray<FEngArch_SystemInfo> UEngArch_TechnicalArchitecture::GetAllSystems() const
{
    TArray<FEngArch_SystemInfo> Systems;
    for (const auto& SystemPair : RegisteredSystems)
    {
        Systems.Add(SystemPair.Value);
    }
    
    return Systems;
}

FEngArch_PerformanceMetrics UEngArch_TechnicalArchitecture::GetCurrentPerformanceMetrics() const
{
    // Update metrics before returning
    const_cast<UEngArch_TechnicalArchitecture*>(this)->UpdatePerformanceMetrics();
    return CachedMetrics;
}

bool UEngArch_TechnicalArchitecture::IsPerformanceWithinTargets() const
{
    const FEngArch_PerformanceMetrics& Metrics = GetCurrentPerformanceMetrics();
    
    bool bWithinTargets = true;
    bWithinTargets &= (Metrics.FrameRate >= (TargetFrameRate * 0.9f)); // 10% tolerance
    bWithinTargets &= (Metrics.GameThreadTime <= MaxGameThreadTime);
    bWithinTargets &= (Metrics.DrawCalls <= MaxDrawCalls);
    bWithinTargets &= (Metrics.MemoryUsageGB <= MaxMemoryUsageGB);
    
    return bWithinTargets;
}

bool UEngArch_TechnicalArchitecture::ValidateSystemDependencies() const
{
    // Check critical systems are active
    TArray<FString> CriticalSystems = {TEXT("Physics"), TEXT("Rendering")};
    
    for (const FString& SystemName : CriticalSystems)
    {
        if (GetSystemState(SystemName) != EEngArch_ModuleState::Active)
        {
            UE_LOG(LogTemp, Error, TEXT("Critical system %s is not active"), *SystemName);
            return false;
        }
    }
    
    return true;
}

TArray<FString> UEngArch_TechnicalArchitecture::GetSystemErrors() const
{
    return SystemErrors;
}

void UEngArch_TechnicalArchitecture::InitializeAllSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("Initializing all registered systems"));
    
    // Initialize systems by priority
    TArray<EEngArch_SystemPriority> Priorities = {
        EEngArch_SystemPriority::Critical,
        EEngArch_SystemPriority::High,
        EEngArch_SystemPriority::Medium,
        EEngArch_SystemPriority::Low
    };
    
    for (EEngArch_SystemPriority Priority : Priorities)
    {
        for (auto& SystemPair : RegisteredSystems)
        {
            FEngArch_SystemInfo& SystemInfo = SystemPair.Value;
            if (SystemInfo.Priority == Priority && SystemInfo.State == EEngArch_ModuleState::Uninitialized)
            {
                SystemInfo.State = EEngArch_ModuleState::Initializing;
                
                // Simulate initialization time
                float InitTime = FMath::RandRange(0.1f, 1.0f);
                SystemInfo.InitializationTime = InitTime;
                
                SystemInfo.State = EEngArch_ModuleState::Active;
                UE_LOG(LogTemp, Log, TEXT("Initialized system: %s (%.2fs)"), *SystemInfo.SystemName, InitTime);
            }
        }
    }
}

void UEngArch_TechnicalArchitecture::ShutdownAllSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("Shutting down all systems"));
    
    for (auto& SystemPair : RegisteredSystems)
    {
        FEngArch_SystemInfo& SystemInfo = SystemPair.Value;
        if (SystemInfo.State == EEngArch_ModuleState::Active)
        {
            SystemInfo.State = EEngArch_ModuleState::Disabled;
            UE_LOG(LogTemp, Log, TEXT("Shutdown system: %s"), *SystemInfo.SystemName);
        }
    }
}

void UEngArch_TechnicalArchitecture::UpdatePerformanceMetrics()
{
    // Get current frame rate
    if (GEngine && GEngine->GetGameViewport())
    {
        CachedMetrics.FrameRate = 1.0f / FApp::GetDeltaTime();
    }
    
    // Get game thread time (approximation)
    CachedMetrics.GameThreadTime = FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds
    
    // Get render thread time (approximation)
    CachedMetrics.RenderThreadTime = CachedMetrics.GameThreadTime * 0.8f; // Estimate
    
    // Get active actors count
    if (UWorld* World = GetWorld())
    {
        CachedMetrics.ActiveActors = World->GetActorCount();
    }
    
    // Estimate memory usage (simplified)
    CachedMetrics.MemoryUsageGB = FPlatformMemory::GetStats().UsedPhysical / (1024.0f * 1024.0f * 1024.0f);
    
    // Estimate draw calls (simplified)
    CachedMetrics.DrawCalls = FMath::Max(100, CachedMetrics.ActiveActors * 2);
    
    ValidatePerformanceTargets();
}

void UEngArch_TechnicalArchitecture::ValidatePerformanceTargets()
{
    if (!IsPerformanceWithinTargets())
    {
        FString PerformanceWarning = FString::Printf(
            TEXT("Performance targets exceeded - FPS: %.1f, GameThread: %.1fms, DrawCalls: %d, Memory: %.2fGB"),
            CachedMetrics.FrameRate,
            CachedMetrics.GameThreadTime,
            CachedMetrics.DrawCalls,
            CachedMetrics.MemoryUsageGB
        );
        
        SystemErrors.AddUnique(PerformanceWarning);
        UE_LOG(LogTemp, Warning, TEXT("%s"), *PerformanceWarning);
    }
}

bool UEngArch_TechnicalArchitecture::CheckSystemDependency(const FString& SystemName) const
{
    // Check if system has all required dependencies
    // This is a simplified implementation - in practice would check actual dependencies
    
    if (SystemName == TEXT("AI"))
    {
        return GetSystemState(TEXT("Physics")) == EEngArch_ModuleState::Active;
    }
    
    if (SystemName == TEXT("WorldGeneration"))
    {
        return GetSystemState(TEXT("Rendering")) == EEngArch_ModuleState::Active;
    }
    
    return true; // No dependencies for other systems
}