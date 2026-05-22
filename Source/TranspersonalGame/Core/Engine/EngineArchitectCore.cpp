#include "EngineArchitectCore.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"

UEngineArchitectCore::UEngineArchitectCore()
{
    PerformanceUpdateInterval = 1.0f;
    MaxAllowedFrameTime = 33.33f; // 30 FPS minimum
    MaxActorCount = 50000;
}

void UEngineArchitectCore::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("EngineArchitectCore: Initializing..."));
    
    // Initialize default systems
    InitializeDefaultSystems();
    
    // Start performance monitoring
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            PerformanceTimerHandle,
            this,
            &UEngineArchitectCore::UpdatePerformanceMetrics,
            PerformanceUpdateInterval,
            true
        );
    }
    
    UE_LOG(LogTemp, Log, TEXT("EngineArchitectCore: Initialization complete"));
}

void UEngineArchitectCore::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(PerformanceTimerHandle);
    }
    
    RegisteredSystems.Empty();
    ValidationErrors.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("EngineArchitectCore: Deinitialized"));
    
    Super::Deinitialize();
}

void UEngineArchitectCore::RegisterSystem(const FString& SystemName, EEng_SystemPriority Priority)
{
    if (SystemName.IsEmpty())
    {
        LogError(TEXT("Cannot register system with empty name"));
        return;
    }
    
    FEng_SystemInfo SystemInfo;
    SystemInfo.SystemName = SystemName;
    SystemInfo.Priority = Priority;
    SystemInfo.State = EEng_ModuleState::Initializing;
    SystemInfo.LastUpdateTime = FPlatformTime::Seconds();
    SystemInfo.ErrorCount = 0;
    
    RegisteredSystems.Add(SystemName, SystemInfo);
    
    UE_LOG(LogTemp, Log, TEXT("EngineArchitectCore: Registered system '%s' with priority %d"), 
           *SystemName, (int32)Priority);
}

void UEngineArchitectCore::UnregisterSystem(const FString& SystemName)
{
    if (RegisteredSystems.Remove(SystemName) > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("EngineArchitectCore: Unregistered system '%s'"), *SystemName);
    }
    else
    {
        LogError(FString::Printf(TEXT("Attempted to unregister unknown system '%s'"), *SystemName));
    }
}

bool UEngineArchitectCore::IsSystemRegistered(const FString& SystemName) const
{
    return RegisteredSystems.Contains(SystemName);
}

void UEngineArchitectCore::SetSystemState(const FString& SystemName, EEng_ModuleState NewState)
{
    if (FEng_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemName))
    {
        SystemInfo->State = NewState;
        SystemInfo->LastUpdateTime = FPlatformTime::Seconds();
        
        if (NewState == EEng_ModuleState::Error)
        {
            SystemInfo->ErrorCount++;
            LogError(FString::Printf(TEXT("System '%s' entered error state (error count: %d)"), 
                                   *SystemName, SystemInfo->ErrorCount));
        }
        
        UE_LOG(LogTemp, Log, TEXT("EngineArchitectCore: System '%s' state changed to %d"), 
               *SystemName, (int32)NewState);
    }
    else
    {
        LogError(FString::Printf(TEXT("Attempted to set state for unknown system '%s'"), *SystemName));
    }
}

EEng_ModuleState UEngineArchitectCore::GetSystemState(const FString& SystemName) const
{
    if (const FEng_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemName))
    {
        return SystemInfo->State;
    }
    
    return EEng_ModuleState::Uninitialized;
}

TArray<FEng_SystemInfo> UEngineArchitectCore::GetAllSystemInfo() const
{
    TArray<FEng_SystemInfo> SystemInfoArray;
    RegisteredSystems.GenerateValueArray(SystemInfoArray);
    return SystemInfoArray;
}

FEng_PerformanceMetrics UEngineArchitectCore::GetCurrentPerformanceMetrics() const
{
    return CurrentMetrics;
}

void UEngineArchitectCore::UpdatePerformanceMetrics()
{
    // Update frame rate and frame time
    if (GEngine && GEngine->GetAverageFPS() > 0)
    {
        CurrentMetrics.FrameRate = GEngine->GetAverageFPS();
        CurrentMetrics.FrameTime = 1000.0f / CurrentMetrics.FrameRate; // Convert to milliseconds
    }
    
    // Update actor and component counts
    if (UWorld* World = GetWorld())
    {
        CurrentMetrics.ActorCount = World->GetActorCount();
        
        int32 ComponentCount = 0;
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            if (AActor* Actor = *ActorItr)
            {
                ComponentCount += Actor->GetRootComponent() ? Actor->GetRootComponent()->GetAttachChildren().Num() + 1 : 0;
            }
        }
        CurrentMetrics.ComponentCount = ComponentCount;
    }
    
    // Update memory usage (approximate)
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    
    // Check performance thresholds
    CheckPerformanceThresholds();
}

bool UEngineArchitectCore::ValidateSystemIntegrity()
{
    ValidationErrors.Empty();
    bool bAllSystemsHealthy = true;
    
    // Check each registered system
    for (const auto& SystemPair : RegisteredSystems)
    {
        const FEng_SystemInfo& SystemInfo = SystemPair.Value;
        
        // Check for error states
        if (SystemInfo.State == EEng_ModuleState::Error)
        {
            ValidationErrors.Add(FString::Printf(TEXT("System '%s' is in error state"), *SystemInfo.SystemName));
            bAllSystemsHealthy = false;
        }
        
        // Check for systems that haven't updated recently
        float TimeSinceUpdate = FPlatformTime::Seconds() - SystemInfo.LastUpdateTime;
        if (TimeSinceUpdate > 60.0f && SystemInfo.State == EEng_ModuleState::Active)
        {
            ValidationErrors.Add(FString::Printf(TEXT("System '%s' hasn't updated in %.1f seconds"), 
                                               *SystemInfo.SystemName, TimeSinceUpdate));
            bAllSystemsHealthy = false;
        }
        
        // Check for high error counts
        if (SystemInfo.ErrorCount > 10)
        {
            ValidationErrors.Add(FString::Printf(TEXT("System '%s' has high error count: %d"), 
                                               *SystemInfo.SystemName, SystemInfo.ErrorCount));
            bAllSystemsHealthy = false;
        }
    }
    
    // Validate core systems
    ValidateCoreSystems();
    
    if (ValidationErrors.Num() > 0)
    {
        bAllSystemsHealthy = false;
    }
    
    UE_LOG(LogTemp, Log, TEXT("EngineArchitectCore: System integrity validation %s. Found %d errors."), 
           bAllSystemsHealthy ? TEXT("PASSED") : TEXT("FAILED"), ValidationErrors.Num());
    
    return bAllSystemsHealthy;
}

TArray<FString> UEngineArchitectCore::GetSystemErrors() const
{
    return ValidationErrors;
}

void UEngineArchitectCore::LogSystemStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== ENGINE ARCHITECT SYSTEM STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Registered Systems: %d"), RegisteredSystems.Num());
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        const FEng_SystemInfo& SystemInfo = SystemPair.Value;
        UE_LOG(LogTemp, Warning, TEXT("  %s: State=%d, Priority=%d, Errors=%d"), 
               *SystemInfo.SystemName, (int32)SystemInfo.State, (int32)SystemInfo.Priority, SystemInfo.ErrorCount);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Performance: FPS=%.1f, Actors=%d, Memory=%.1fMB"), 
           CurrentMetrics.FrameRate, CurrentMetrics.ActorCount, CurrentMetrics.MemoryUsageMB);
    
    UE_LOG(LogTemp, Warning, TEXT("Validation Errors: %d"), ValidationErrors.Num());
    for (const FString& Error : ValidationErrors)
    {
        UE_LOG(LogTemp, Error, TEXT("  %s"), *Error);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== END SYSTEM STATUS ==="));
}

void UEngineArchitectCore::ForceSystemValidation()
{
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitectCore: Forcing system validation..."));
    ValidateSystemIntegrity();
    LogSystemStatus();
}

void UEngineArchitectCore::InitializeDefaultSystems()
{
    // Register core engine systems
    RegisterSystem(TEXT("WorldGeneration"), EEng_SystemPriority::Critical);
    RegisterSystem(TEXT("Physics"), EEng_SystemPriority::Critical);
    RegisterSystem(TEXT("Rendering"), EEng_SystemPriority::Critical);
    RegisterSystem(TEXT("Audio"), EEng_SystemPriority::High);
    RegisterSystem(TEXT("AI"), EEng_SystemPriority::High);
    RegisterSystem(TEXT("Networking"), EEng_SystemPriority::Medium);
    RegisterSystem(TEXT("UI"), EEng_SystemPriority::Medium);
    RegisterSystem(TEXT("VFX"), EEng_SystemPriority::Low);
    
    UE_LOG(LogTemp, Log, TEXT("EngineArchitectCore: Default systems registered"));
}

void UEngineArchitectCore::ValidateCoreSystems()
{
    // Check if critical UE5 systems are available
    if (!GEngine)
    {
        ValidationErrors.Add(TEXT("GEngine is null - critical engine failure"));
    }
    
    if (!GetWorld())
    {
        ValidationErrors.Add(TEXT("World is null - world context missing"));
    }
    
    // Check game mode
    if (UWorld* World = GetWorld())
    {
        if (!World->GetAuthGameMode())
        {
            ValidationErrors.Add(TEXT("GameMode is null - game mode not set"));
        }
    }
}

void UEngineArchitectCore::CheckPerformanceThresholds()
{
    // Check frame time threshold
    if (CurrentMetrics.FrameTime > MaxAllowedFrameTime)
    {
        LogError(FString::Printf(TEXT("Frame time exceeded threshold: %.2fms > %.2fms"), 
                               CurrentMetrics.FrameTime, MaxAllowedFrameTime));
    }
    
    // Check actor count threshold
    if (CurrentMetrics.ActorCount > MaxActorCount)
    {
        LogError(FString::Printf(TEXT("Actor count exceeded threshold: %d > %d"), 
                               CurrentMetrics.ActorCount, MaxActorCount));
    }
    
    // Check memory usage (warning at 4GB)
    if (CurrentMetrics.MemoryUsageMB > 4096.0f)
    {
        LogError(FString::Printf(TEXT("High memory usage: %.1fMB"), CurrentMetrics.MemoryUsageMB));
    }
}

void UEngineArchitectCore::LogError(const FString& ErrorMessage)
{
    ValidationErrors.AddUnique(ErrorMessage);
    UE_LOG(LogTemp, Error, TEXT("EngineArchitectCore: %s"), *ErrorMessage);
}

// EngineArchitectComponent Implementation

UEngineArchitectComponent::UEngineArchitectComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
    
    bMonitorPerformance = true;
    MonitoringInterval = 5.0f;
    LastMonitoringTime = 0.0f;
    CachedEngineCore = nullptr;
}

void UEngineArchitectComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache reference to engine core
    if (UGameInstance* GameInstance = GetWorld()->GetGameInstance())
    {
        CachedEngineCore = GameInstance->GetSubsystem<UEngineArchitectCore>();
    }
    
    if (!CachedEngineCore)
    {
        UE_LOG(LogTemp, Error, TEXT("EngineArchitectComponent: Failed to get EngineArchitectCore subsystem"));
    }
}

void UEngineArchitectComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bMonitorPerformance && CachedEngineCore)
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - LastMonitoringTime >= MonitoringInterval)
        {
            // Trigger performance update
            CachedEngineCore->UpdatePerformanceMetrics();
            LastMonitoringTime = CurrentTime;
        }
    }
}

UEngineArchitectCore* UEngineArchitectComponent::GetEngineCore() const
{
    return CachedEngineCore;
}

bool UEngineArchitectComponent::IsSystemHealthy(const FString& SystemName) const
{
    if (CachedEngineCore)
    {
        EEng_ModuleState State = CachedEngineCore->GetSystemState(SystemName);
        return State == EEng_ModuleState::Active;
    }
    
    return false;
}

float UEngineArchitectComponent::GetCurrentFrameRate() const
{
    if (CachedEngineCore)
    {
        return CachedEngineCore->GetCurrentPerformanceMetrics().FrameRate;
    }
    
    return 0.0f;
}