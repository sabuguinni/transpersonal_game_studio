#include "Eng_ArchitecturalCore.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

// Constructor
UEng_ArchitecturalCore::UEng_ArchitecturalCore()
{
    bIsArchitectureInitialized = false;
    TotalInitializationTime = 0.0f;
    
    // Set default configuration
    ArchitecturalConfig = FEng_ArchitecturalConfig();
}

// USubsystem interface implementation
void UEng_ArchitecturalCore::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Eng_ArchitecturalCore: Initializing architectural core system"));
    
    // Initialize the architecture
    InitializeArchitecture();
    
    // Start performance monitoring if enabled
    if (ArchitecturalConfig.bEnablePerformanceMonitoring)
    {
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().SetTimer(
                PerformanceMonitoringTimer,
                this,
                &UEng_ArchitecturalCore::OnPerformanceMonitoringTick,
                1.0f,
                true
            );
        }
    }
    
    // Start validation timer if enabled
    if (ArchitecturalConfig.bEnableSystemValidation)
    {
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().SetTimer(
                ValidationTimer,
                this,
                &UEng_ArchitecturalCore::OnValidationTick,
                5.0f,
                true
            );
        }
    }
}

void UEng_ArchitecturalCore::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Eng_ArchitecturalCore: Shutting down architectural core system"));
    
    // Clear timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(PerformanceMonitoringTimer);
        World->GetTimerManager().ClearTimer(ValidationTimer);
    }
    
    // Shutdown all systems
    ShutdownAllSystems();
    
    Super::Deinitialize();
}

// Core architectural functions
bool UEng_ArchitecturalCore::InitializeArchitecture()
{
    if (bIsArchitectureInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("Eng_ArchitecturalCore: Architecture already initialized"));
        return true;
    }
    
    float StartTime = FPlatformTime::Seconds();
    
    UE_LOG(LogTemp, Warning, TEXT("Eng_ArchitecturalCore: Starting architecture initialization"));
    
    // Register core systems
    RegisterSystem(TEXT("PhysicsCore"), EEng_SystemPriority::Critical);
    RegisterSystem(TEXT("WorldGeneration"), EEng_SystemPriority::High);
    RegisterSystem(TEXT("CharacterSystems"), EEng_SystemPriority::High);
    RegisterSystem(TEXT("BiomeManager"), EEng_SystemPriority::High);
    RegisterSystem(TEXT("DinosaurAI"), EEng_SystemPriority::Medium);
    RegisterSystem(TEXT("CombatSystem"), EEng_SystemPriority::Medium);
    RegisterSystem(TEXT("AudioSystem"), EEng_SystemPriority::Medium);
    RegisterSystem(TEXT("VFXSystem"), EEng_SystemPriority::Low);
    RegisterSystem(TEXT("UISystem"), EEng_SystemPriority::Low);
    RegisterSystem(TEXT("AnalyticsSystem"), EEng_SystemPriority::Background);
    
    // Initialize systems by priority
    InitializeSystemsByPriority();
    
    // Validate configuration
    if (!ValidateSystemConfiguration())
    {
        UE_LOG(LogTemp, Error, TEXT("Eng_ArchitecturalCore: System configuration validation failed"));
        return false;
    }
    
    TotalInitializationTime = FPlatformTime::Seconds() - StartTime;
    bIsArchitectureInitialized = true;
    
    UE_LOG(LogTemp, Warning, TEXT("Eng_ArchitecturalCore: Architecture initialization complete in %.3f seconds"), TotalInitializationTime);
    
    return true;
}

bool UEng_ArchitecturalCore::RegisterSystem(const FString& SystemName, EEng_SystemPriority Priority)
{
    if (!IsValidSystemName(SystemName))
    {
        UE_LOG(LogTemp, Error, TEXT("Eng_ArchitecturalCore: Invalid system name: %s"), *SystemName);
        return false;
    }
    
    if (RegisteredSystems.Contains(SystemName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Eng_ArchitecturalCore: System already registered: %s"), *SystemName);
        return false;
    }
    
    FEng_SystemInfo SystemInfo;
    SystemInfo.SystemName = SystemName;
    SystemInfo.Priority = Priority;
    SystemInfo.bIsInitialized = false;
    SystemInfo.bIsActive = false;
    SystemInfo.InitializationTime = 0.0f;
    
    RegisteredSystems.Add(SystemName, SystemInfo);
    
    LogSystemEvent(SystemName, TEXT("Registered"));
    
    return true;
}

bool UEng_ArchitecturalCore::UnregisterSystem(const FString& SystemName)
{
    if (!RegisteredSystems.Contains(SystemName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Eng_ArchitecturalCore: System not found for unregistration: %s"), *SystemName);
        return false;
    }
    
    // Stop the system if it's running
    StopSystem(SystemName);
    
    // Remove from registry
    RegisteredSystems.Remove(SystemName);
    SystemPerformanceMetrics.Remove(SystemName);
    
    LogSystemEvent(SystemName, TEXT("Unregistered"));
    
    return true;
}

bool UEng_ArchitecturalCore::IsSystemRegistered(const FString& SystemName) const
{
    return RegisteredSystems.Contains(SystemName);
}

bool UEng_ArchitecturalCore::IsSystemInitialized(const FString& SystemName) const
{
    if (const FEng_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemName))
    {
        return SystemInfo->bIsInitialized;
    }
    return false;
}

TArray<FEng_SystemInfo> UEng_ArchitecturalCore::GetAllSystemInfo() const
{
    TArray<FEng_SystemInfo> SystemInfoArray;
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        SystemInfoArray.Add(SystemPair.Value);
    }
    
    // Sort by priority
    SystemInfoArray.Sort([](const FEng_SystemInfo& A, const FEng_SystemInfo& B) {
        return static_cast<int32>(A.Priority) < static_cast<int32>(B.Priority);
    });
    
    return SystemInfoArray;
}

FEng_SystemInfo UEng_ArchitecturalCore::GetSystemInfo(const FString& SystemName) const
{
    if (const FEng_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemName))
    {
        return *SystemInfo;
    }
    
    // Return default if not found
    return FEng_SystemInfo();
}

// System lifecycle management
bool UEng_ArchitecturalCore::StartSystem(const FString& SystemName)
{
    FEng_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemName);
    if (!SystemInfo)
    {
        UE_LOG(LogTemp, Error, TEXT("Eng_ArchitecturalCore: Cannot start unregistered system: %s"), *SystemName);
        return false;
    }
    
    if (SystemInfo->bIsActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Eng_ArchitecturalCore: System already active: %s"), *SystemName);
        return true;
    }
    
    float StartTime = FPlatformTime::Seconds();
    
    // Mark as initialized and active
    SystemInfo->bIsInitialized = true;
    SystemInfo->bIsActive = true;
    SystemInfo->InitializationTime = FPlatformTime::Seconds() - StartTime;
    
    LogSystemEvent(SystemName, TEXT("Started"));
    
    return true;
}

bool UEng_ArchitecturalCore::StopSystem(const FString& SystemName)
{
    FEng_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemName);
    if (!SystemInfo)
    {
        UE_LOG(LogTemp, Error, TEXT("Eng_ArchitecturalCore: Cannot stop unregistered system: %s"), *SystemName);
        return false;
    }
    
    if (!SystemInfo->bIsActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Eng_ArchitecturalCore: System already inactive: %s"), *SystemName);
        return true;
    }
    
    SystemInfo->bIsActive = false;
    
    LogSystemEvent(SystemName, TEXT("Stopped"));
    
    return true;
}

bool UEng_ArchitecturalCore::RestartSystem(const FString& SystemName)
{
    if (!StopSystem(SystemName))
    {
        return false;
    }
    
    return StartSystem(SystemName);
}

// Performance monitoring
float UEng_ArchitecturalCore::GetSystemPerformanceMetric(const FString& SystemName) const
{
    if (const float* Metric = SystemPerformanceMetrics.Find(SystemName))
    {
        return *Metric;
    }
    return 0.0f;
}

void UEng_ArchitecturalCore::EnablePerformanceMonitoring(bool bEnable)
{
    ArchitecturalConfig.bEnablePerformanceMonitoring = bEnable;
    
    if (UWorld* World = GetWorld())
    {
        if (bEnable && !World->GetTimerManager().IsTimerActive(PerformanceMonitoringTimer))
        {
            World->GetTimerManager().SetTimer(
                PerformanceMonitoringTimer,
                this,
                &UEng_ArchitecturalCore::OnPerformanceMonitoringTick,
                1.0f,
                true
            );
        }
        else if (!bEnable && World->GetTimerManager().IsTimerActive(PerformanceMonitoringTimer))
        {
            World->GetTimerManager().ClearTimer(PerformanceMonitoringTimer);
        }
    }
}

// Configuration management
void UEng_ArchitecturalCore::SetArchitecturalConfig(const FEng_ArchitecturalConfig& NewConfig)
{
    ArchitecturalConfig = NewConfig;
    
    // Apply configuration changes
    EnablePerformanceMonitoring(ArchitecturalConfig.bEnablePerformanceMonitoring);
}

FEng_ArchitecturalConfig UEng_ArchitecturalCore::GetArchitecturalConfig() const
{
    return ArchitecturalConfig;
}

// System validation
bool UEng_ArchitecturalCore::ValidateSystemIntegrity()
{
    bool bAllSystemsValid = true;
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        const FString& SystemName = SystemPair.Key;
        const FEng_SystemInfo& SystemInfo = SystemPair.Value;
        
        // Check if system is in a valid state
        if (SystemInfo.bIsActive && !SystemInfo.bIsInitialized)
        {
            UE_LOG(LogTemp, Error, TEXT("Eng_ArchitecturalCore: System %s is active but not initialized"), *SystemName);
            bAllSystemsValid = false;
        }
        
        // Check initialization time
        if (SystemInfo.InitializationTime > ArchitecturalConfig.SystemTimeoutThreshold)
        {
            UE_LOG(LogTemp, Warning, TEXT("Eng_ArchitecturalCore: System %s took %.3f seconds to initialize (threshold: %.3f)"), 
                *SystemName, SystemInfo.InitializationTime, ArchitecturalConfig.SystemTimeoutThreshold);
        }
    }
    
    return bAllSystemsValid;
}

bool UEng_ArchitecturalCore::ValidateSystemDependencies()
{
    // For now, just check that critical systems are initialized first
    TArray<FString> CriticalSystems;
    TArray<FString> NonCriticalSystems;
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        if (SystemPair.Value.Priority == EEng_SystemPriority::Critical)
        {
            CriticalSystems.Add(SystemPair.Key);
        }
        else
        {
            NonCriticalSystems.Add(SystemPair.Key);
        }
    }
    
    // Ensure all critical systems are initialized
    for (const FString& CriticalSystem : CriticalSystems)
    {
        if (!IsSystemInitialized(CriticalSystem))
        {
            UE_LOG(LogTemp, Error, TEXT("Eng_ArchitecturalCore: Critical system %s is not initialized"), *CriticalSystem);
            return false;
        }
    }
    
    return true;
}

// Debug and diagnostics
void UEng_ArchitecturalCore::PrintSystemStatus() const
{
    UE_LOG(LogTemp, Warning, TEXT("=== ARCHITECTURAL CORE SYSTEM STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Architecture Initialized: %s"), bIsArchitectureInitialized ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Warning, TEXT("Total Initialization Time: %.3f seconds"), TotalInitializationTime);
    UE_LOG(LogTemp, Warning, TEXT("Registered Systems: %d"), RegisteredSystems.Num());
    
    TArray<FEng_SystemInfo> SystemInfoArray = GetAllSystemInfo();
    for (const FEng_SystemInfo& SystemInfo : SystemInfoArray)
    {
        UE_LOG(LogTemp, Warning, TEXT("  %s: Priority=%d, Init=%s, Active=%s, Time=%.3f"), 
            *SystemInfo.SystemName,
            static_cast<int32>(SystemInfo.Priority),
            SystemInfo.bIsInitialized ? TEXT("Yes") : TEXT("No"),
            SystemInfo.bIsActive ? TEXT("Yes") : TEXT("No"),
            SystemInfo.InitializationTime
        );
    }
}

void UEng_ArchitecturalCore::GenerateArchitecturalReport() const
{
    UE_LOG(LogTemp, Warning, TEXT("=== ARCHITECTURAL REPORT ==="));
    PrintSystemStatus();
    
    // Performance metrics
    UE_LOG(LogTemp, Warning, TEXT("Performance Metrics:"));
    for (const auto& MetricPair : SystemPerformanceMetrics)
    {
        UE_LOG(LogTemp, Warning, TEXT("  %s: %.3f"), *MetricPair.Key, MetricPair.Value);
    }
    
    // Configuration
    UE_LOG(LogTemp, Warning, TEXT("Configuration:"));
    UE_LOG(LogTemp, Warning, TEXT("  Performance Monitoring: %s"), ArchitecturalConfig.bEnablePerformanceMonitoring ? TEXT("Enabled") : TEXT("Disabled"));
    UE_LOG(LogTemp, Warning, TEXT("  System Validation: %s"), ArchitecturalConfig.bEnableSystemValidation ? TEXT("Enabled") : TEXT("Disabled"));
    UE_LOG(LogTemp, Warning, TEXT("  Debug Logging: %s"), ArchitecturalConfig.bEnableDebugLogging ? TEXT("Enabled") : TEXT("Disabled"));
    UE_LOG(LogTemp, Warning, TEXT("  Timeout Threshold: %.3f seconds"), ArchitecturalConfig.SystemTimeoutThreshold);
    UE_LOG(LogTemp, Warning, TEXT("  Max Concurrent Systems: %d"), ArchitecturalConfig.MaxConcurrentSystems);
}

// Static access
UEng_ArchitecturalCore* UEng_ArchitecturalCore::GetArchitecturalCore(const UObject* WorldContext)
{
    if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull))
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            return GameInstance->GetSubsystem<UEng_ArchitecturalCore>();
        }
    }
    return nullptr;
}

// Protected methods
void UEng_ArchitecturalCore::InitializeSystemsByPriority()
{
    // Get all systems sorted by priority
    TArray<FEng_SystemInfo> SystemInfoArray = GetAllSystemInfo();
    
    for (const FEng_SystemInfo& SystemInfo : SystemInfoArray)
    {
        StartSystem(SystemInfo.SystemName);
    }
}

void UEng_ArchitecturalCore::ShutdownAllSystems()
{
    // Shutdown in reverse priority order
    TArray<FEng_SystemInfo> SystemInfoArray = GetAllSystemInfo();
    
    // Reverse the array to shutdown in reverse priority order
    for (int32 i = SystemInfoArray.Num() - 1; i >= 0; --i)
    {
        StopSystem(SystemInfoArray[i].SystemName);
    }
}

bool UEng_ArchitecturalCore::ValidateSystemConfiguration() const
{
    // Check if we have too many systems
    if (RegisteredSystems.Num() > ArchitecturalConfig.MaxConcurrentSystems)
    {
        UE_LOG(LogTemp, Error, TEXT("Eng_ArchitecturalCore: Too many systems registered (%d > %d)"), 
            RegisteredSystems.Num(), ArchitecturalConfig.MaxConcurrentSystems);
        return false;
    }
    
    // Ensure we have at least one critical system
    bool bHasCriticalSystem = false;
    for (const auto& SystemPair : RegisteredSystems)
    {
        if (SystemPair.Value.Priority == EEng_SystemPriority::Critical)
        {
            bHasCriticalSystem = true;
            break;
        }
    }
    
    if (!bHasCriticalSystem)
    {
        UE_LOG(LogTemp, Error, TEXT("Eng_ArchitecturalCore: No critical systems registered"));
        return false;
    }
    
    return true;
}

void UEng_ArchitecturalCore::UpdateSystemMetrics()
{
    // Update performance metrics for all active systems
    for (const auto& SystemPair : RegisteredSystems)
    {
        const FString& SystemName = SystemPair.Key;
        const FEng_SystemInfo& SystemInfo = SystemPair.Value;
        
        if (SystemInfo.bIsActive)
        {
            // For now, use initialization time as a basic metric
            // In a real implementation, this would measure actual performance
            SystemPerformanceMetrics.Add(SystemName, SystemInfo.InitializationTime);
        }
    }
}

// Private methods
void UEng_ArchitecturalCore::LogSystemEvent(const FString& SystemName, const FString& Event) const
{
    if (ArchitecturalConfig.bEnableDebugLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("Eng_ArchitecturalCore: System %s - %s"), *SystemName, *Event);
    }
}

bool UEng_ArchitecturalCore::IsValidSystemName(const FString& SystemName) const
{
    return !SystemName.IsEmpty() && SystemName.Len() > 2 && SystemName.Len() < 64;
}

void UEng_ArchitecturalCore::CleanupInvalidSystems()
{
    TArray<FString> SystemsToRemove;
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        if (!IsValidSystemName(SystemPair.Key))
        {
            SystemsToRemove.Add(SystemPair.Key);
        }
    }
    
    for (const FString& SystemName : SystemsToRemove)
    {
        UnregisterSystem(SystemName);
    }
}

// Timer callbacks
void UEng_ArchitecturalCore::OnPerformanceMonitoringTick()
{
    UpdateSystemMetrics();
    
    if (ArchitecturalConfig.bEnableDebugLogging)
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("Eng_ArchitecturalCore: Performance monitoring tick"));
    }
}

void UEng_ArchitecturalCore::OnValidationTick()
{
    ValidateSystemIntegrity();
    ValidateSystemDependencies();
    CleanupInvalidSystems();
    
    if (ArchitecturalConfig.bEnableDebugLogging)
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("Eng_ArchitecturalCore: Validation tick"));
    }
}

// World Subsystem Implementation
UEng_ArchitecturalWorldSubsystem::UEng_ArchitecturalWorldSubsystem()
{
    bIsWorldArchitectureInitialized = false;
}

void UEng_ArchitecturalWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Eng_ArchitecturalWorldSubsystem: Initializing world architecture"));
    
    // Get reference to global architectural core
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            ArchitecturalCore = GameInstance->GetSubsystem<UEng_ArchitecturalCore>();
        }
    }
    
    InitializeWorldArchitecture();
}

void UEng_ArchitecturalWorldSubsystem::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Eng_ArchitecturalWorldSubsystem: Shutting down world architecture"));
    
    CleanupWorldSystems();
    
    Super::Deinitialize();
}

bool UEng_ArchitecturalWorldSubsystem::InitializeWorldArchitecture()
{
    if (bIsWorldArchitectureInitialized)
    {
        return true;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Eng_ArchitecturalWorldSubsystem: Starting world architecture initialization"));
    
    // Validate that global architecture is initialized
    if (ArchitecturalCore.IsValid())
    {
        // World architecture is ready
        bIsWorldArchitectureInitialized = true;
        UE_LOG(LogTemp, Warning, TEXT("Eng_ArchitecturalWorldSubsystem: World architecture initialization complete"));
        return true;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Eng_ArchitecturalWorldSubsystem: Global architectural core not available"));
        return false;
    }
}

bool UEng_ArchitecturalWorldSubsystem::ValidateWorldSystems()
{
    if (!bIsWorldArchitectureInitialized)
    {
        UE_LOG(LogTemp, Error, TEXT("Eng_ArchitecturalWorldSubsystem: Cannot validate - world architecture not initialized"));
        return false;
    }
    
    if (ArchitecturalCore.IsValid())
    {
        return ArchitecturalCore->ValidateSystemIntegrity();
    }
    
    return false;
}

void UEng_ArchitecturalWorldSubsystem::CleanupWorldSystems()
{
    bIsWorldArchitectureInitialized = false;
    ArchitecturalCore.Reset();
}