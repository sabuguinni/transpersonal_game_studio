#include "Eng_GameplayArchitect.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UEng_GameplayArchitect::UEng_GameplayArchitect()
{
    // Initialize default configuration
    ArchitectureConfig = FEng_GameplayArchitectureConfig();
    
    // Initialize performance tracking
    bPerformanceMonitoringEnabled = true;
    TotalUpdateTime = 0.0f;
    
    // Initialize state
    bIsArchitectureInitialized = false;
    LastSystemUpdateTime = 0.0;
    SystemUpdateCounter = 0;
}

void UEng_GameplayArchitect::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Engine Architect Gameplay Architecture initializing..."));
    
    // Initialize the architecture
    InitializeGameplayArchitecture();
}

void UEng_GameplayArchitect::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("Engine Architect Gameplay Architecture shutting down..."));
    
    // Shutdown the architecture
    ShutdownGameplayArchitecture();
    
    Super::Deinitialize();
}

void UEng_GameplayArchitect::InitializeGameplayArchitecture()
{
    if (bIsArchitectureInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("Gameplay Architecture already initialized"));
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Initializing Gameplay Architecture..."));
    
    // Clear existing systems
    RegisteredSystems.Empty();
    SystemPerformanceMetrics.Empty();
    
    // Initialize default systems if enabled
    if (ArchitectureConfig.bAutoInitializeSystems)
    {
        InitializeDefaultSystems();
    }
    
    // Set up system update timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            SystemUpdateTimerHandle,
            FTimerDelegate::CreateUObject(this, &UEng_GameplayArchitect::UpdateAllSystems, ArchitectureConfig.SystemUpdateInterval),
            ArchitectureConfig.SystemUpdateInterval,
            true
        );
    }
    
    bIsArchitectureInitialized = true;
    LastSystemUpdateTime = FPlatformTime::Seconds();
    
    UE_LOG(LogTemp, Log, TEXT("Gameplay Architecture initialized successfully"));
    LogSystemEvent(EEng_GameplaySystemType::None, TEXT("Architecture Initialized"));
}

void UEng_GameplayArchitecture::ShutdownGameplayArchitecture()
{
    if (!bIsArchitectureInitialized)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Shutting down Gameplay Architecture..."));
    
    // Clear the update timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(SystemUpdateTimerHandle);
    }
    
    // Cleanup all systems
    CleanupSystems();
    
    bIsArchitectureInitialized = false;
    
    UE_LOG(LogTemp, Log, TEXT("Gameplay Architecture shutdown complete"));
    LogSystemEvent(EEng_GameplaySystemType::None, TEXT("Architecture Shutdown"));
}

bool UEng_GameplayArchitect::RegisterGameplaySystem(EEng_GameplaySystemType SystemType, const FString& SystemName)
{
    if (SystemType == EEng_GameplaySystemType::None)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot register system with type 'None'"));
        return false;
    }
    
    if (RegisteredSystems.Contains(SystemType))
    {
        UE_LOG(LogTemp, Warning, TEXT("System %s already registered"), *SystemName);
        return false;
    }
    
    // Create system info
    FEng_GameplaySystemInfo SystemInfo;
    SystemInfo.SystemType = SystemType;
    SystemInfo.SystemName = SystemName;
    SystemInfo.Priority = EEng_GameplayPriority::Medium;
    SystemInfo.bIsActive = false;
    SystemInfo.bIsInitialized = true;
    SystemInfo.InitializationTime = FPlatformTime::Seconds();
    SystemInfo.LastUpdateTime = 0.0f;
    
    // Register the system
    RegisteredSystems.Add(SystemType, SystemInfo);
    SystemPerformanceMetrics.Add(SystemType, 0.0f);
    
    UE_LOG(LogTemp, Log, TEXT("Registered gameplay system: %s"), *SystemName);
    LogSystemEvent(SystemType, TEXT("System Registered"));
    
    return true;
}

bool UEng_GameplayArchitect::UnregisterGameplaySystem(EEng_GameplaySystemType SystemType)
{
    if (!RegisteredSystems.Contains(SystemType))
    {
        UE_LOG(LogTemp, Warning, TEXT("System not registered for unregistration"));
        return false;
    }
    
    FEng_GameplaySystemInfo* SystemInfo = RegisteredSystems.Find(SystemType);
    if (SystemInfo)
    {
        UE_LOG(LogTemp, Log, TEXT("Unregistering gameplay system: %s"), *SystemInfo->SystemName);
        LogSystemEvent(SystemType, TEXT("System Unregistered"));
    }
    
    // Remove from registries
    RegisteredSystems.Remove(SystemType);
    SystemPerformanceMetrics.Remove(SystemType);
    
    return true;
}

bool UEng_GameplayArchitect::IsSystemRegistered(EEng_GameplaySystemType SystemType) const
{
    return RegisteredSystems.Contains(SystemType);
}

bool UEng_GameplayArchitect::IsSystemActive(EEng_GameplaySystemType SystemType) const
{
    if (const FEng_GameplaySystemInfo* SystemInfo = RegisteredSystems.Find(SystemType))
    {
        return SystemInfo->bIsActive;
    }
    return false;
}

void UEng_GameplayArchitect::ActivateSystem(EEng_GameplaySystemType SystemType)
{
    if (FEng_GameplaySystemInfo* SystemInfo = RegisteredSystems.Find(SystemType))
    {
        if (!SystemInfo->bIsActive)
        {
            SystemInfo->bIsActive = true;
            SystemInfo->LastUpdateTime = FPlatformTime::Seconds();
            
            UE_LOG(LogTemp, Log, TEXT("Activated system: %s"), *SystemInfo->SystemName);
            LogSystemEvent(SystemType, TEXT("System Activated"));
        }
    }
}

void UEng_GameplayArchitect::DeactivateSystem(EEng_GameplaySystemType SystemType)
{
    if (FEng_GameplaySystemInfo* SystemInfo = RegisteredSystems.Find(SystemType))
    {
        if (SystemInfo->bIsActive)
        {
            SystemInfo->bIsActive = false;
            
            UE_LOG(LogTemp, Log, TEXT("Deactivated system: %s"), *SystemInfo->SystemName);
            LogSystemEvent(SystemType, TEXT("System Deactivated"));
        }
    }
}

void UEng_GameplayArchitect::UpdateAllSystems(float DeltaTime)
{
    if (!bIsArchitectureInitialized)
    {
        return;
    }
    
    double UpdateStartTime = FPlatformTime::Seconds();
    int32 ActiveSystemCount = 0;
    
    // Update all active systems
    for (auto& SystemPair : RegisteredSystems)
    {
        EEng_GameplaySystemType SystemType = SystemPair.Key;
        FEng_GameplaySystemInfo& SystemInfo = SystemPair.Value;
        
        if (SystemInfo.bIsActive)
        {
            UpdateSystem(SystemType, DeltaTime);
            ActiveSystemCount++;
        }
    }
    
    // Update performance metrics
    double UpdateEndTime = FPlatformTime::Seconds();
    TotalUpdateTime = UpdateEndTime - UpdateStartTime;
    LastSystemUpdateTime = UpdateEndTime;
    SystemUpdateCounter++;
    
    // Log periodic status
    if (SystemUpdateCounter % 600 == 0) // Every 60 seconds at 0.1s intervals
    {
        UE_LOG(LogTemp, Log, TEXT("Architecture Update: %d active systems, %.4f ms total update time"), 
               ActiveSystemCount, TotalUpdateTime * 1000.0f);
    }
}

void UEng_GameplayArchitect::UpdateSystem(EEng_GameplaySystemType SystemType, float DeltaTime)
{
    if (FEng_GameplaySystemInfo* SystemInfo = RegisteredSystems.Find(SystemType))
    {
        if (SystemInfo->bIsActive)
        {
            double SystemUpdateStartTime = FPlatformTime::Seconds();
            
            // Update the system's last update time
            SystemInfo->LastUpdateTime = SystemUpdateStartTime;
            
            // Calculate system update time for performance tracking
            double SystemUpdateEndTime = FPlatformTime::Seconds();
            float SystemUpdateTime = SystemUpdateEndTime - SystemUpdateStartTime;
            
            // Update performance metrics
            if (bPerformanceMonitoringEnabled)
            {
                UpdatePerformanceMetrics(SystemType, SystemUpdateTime);
            }
        }
    }
}

void UEng_GameplayArchitect::SetArchitectureConfig(const FEng_GameplayArchitectureConfig& NewConfig)
{
    ArchitectureConfig = NewConfig;
    
    UE_LOG(LogTemp, Log, TEXT("Architecture configuration updated"));
    LogSystemEvent(EEng_GameplaySystemType::None, TEXT("Configuration Updated"));
}

FEng_GameplayArchitectureConfig UEng_GameplayArchitect::GetArchitectureConfig() const
{
    return ArchitectureConfig;
}

TArray<FEng_GameplaySystemInfo> UEng_GameplayArchitect::GetAllSystemInfo() const
{
    TArray<FEng_GameplaySystemInfo> SystemInfoArray;
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        SystemInfoArray.Add(SystemPair.Value);
    }
    
    return SystemInfoArray;
}

FEng_GameplaySystemInfo UEng_GameplayArchitect::GetSystemInfo(EEng_GameplaySystemType SystemType) const
{
    if (const FEng_GameplaySystemInfo* SystemInfo = RegisteredSystems.Find(SystemType))
    {
        return *SystemInfo;
    }
    
    return FEng_GameplaySystemInfo(); // Return default info if not found
}

int32 UEng_GameplayArchitect::GetActiveSystemCount() const
{
    int32 ActiveCount = 0;
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        if (SystemPair.Value.bIsActive)
        {
            ActiveCount++;
        }
    }
    
    return ActiveCount;
}

float UEng_GameplayArchitect::GetTotalSystemUpdateTime() const
{
    return TotalUpdateTime;
}

void UEng_GameplayArchitect::EnablePerformanceMonitoring(bool bEnable)
{
    bPerformanceMonitoringEnabled = bEnable;
    
    UE_LOG(LogTemp, Log, TEXT("Performance monitoring %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
    LogSystemEvent(EEng_GameplaySystemType::Performance, bEnable ? TEXT("Monitoring Enabled") : TEXT("Monitoring Disabled"));
}

bool UEng_GameplayArchitect::IsPerformanceMonitoringEnabled() const
{
    return bPerformanceMonitoringEnabled;
}

float UEng_GameplayArchitect::GetSystemPerformanceMetric(EEng_GameplaySystemType SystemType) const
{
    if (const float* Metric = SystemPerformanceMetrics.Find(SystemType))
    {
        return *Metric;
    }
    return 0.0f;
}

void UEng_GameplayArchitect::ValidateArchitecture()
{
    UE_LOG(LogTemp, Log, TEXT("=== GAMEPLAY ARCHITECTURE VALIDATION ==="));
    
    // Validate initialization state
    UE_LOG(LogTemp, Log, TEXT("Architecture Initialized: %s"), bIsArchitectureInitialized ? TEXT("YES") : TEXT("NO"));
    
    // Validate registered systems
    UE_LOG(LogTemp, Log, TEXT("Registered Systems: %d"), RegisteredSystems.Num());
    UE_LOG(LogTemp, Log, TEXT("Active Systems: %d"), GetActiveSystemCount());
    
    // Validate configuration
    UE_LOG(LogTemp, Log, TEXT("Auto Initialize: %s"), ArchitectureConfig.bAutoInitializeSystems ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Log, TEXT("Performance Monitoring: %s"), bPerformanceMonitoringEnabled ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Log, TEXT("Update Interval: %.3f seconds"), ArchitectureConfig.SystemUpdateInterval);
    
    // Validate world state
    UWorld* World = GetWorld();
    UE_LOG(LogTemp, Log, TEXT("World Valid: %s"), World ? TEXT("YES") : TEXT("NO"));
    
    UE_LOG(LogTemp, Log, TEXT("=== VALIDATION COMPLETE ==="));
}

void UEng_GameplayArchitect::PrintSystemStatus()
{
    UE_LOG(LogTemp, Log, TEXT("=== SYSTEM STATUS REPORT ==="));
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        const FEng_GameplaySystemInfo& SystemInfo = SystemPair.Value;
        float PerformanceMetric = GetSystemPerformanceMetric(SystemPair.Key);
        
        UE_LOG(LogTemp, Log, TEXT("System: %s | Active: %s | Performance: %.4f ms"), 
               *SystemInfo.SystemName,
               SystemInfo.bIsActive ? TEXT("YES") : TEXT("NO"),
               PerformanceMetric * 1000.0f);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Total Update Time: %.4f ms"), TotalUpdateTime * 1000.0f);
    UE_LOG(LogTemp, Log, TEXT("=== STATUS REPORT COMPLETE ==="));
}

bool UEng_GameplayArchitect::RunArchitectureDiagnostics()
{
    UE_LOG(LogTemp, Log, TEXT("Running architecture diagnostics..."));
    
    bool bDiagnosticsPass = true;
    
    // Check initialization
    if (!bIsArchitectureInitialized)
    {
        UE_LOG(LogTemp, Error, TEXT("DIAGNOSTIC FAIL: Architecture not initialized"));
        bDiagnosticsPass = false;
    }
    
    // Check world validity
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("DIAGNOSTIC FAIL: No valid world"));
        bDiagnosticsPass = false;
    }
    
    // Check system registration
    if (RegisteredSystems.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("DIAGNOSTIC WARNING: No systems registered"));
    }
    
    // Check active systems
    if (GetActiveSystemCount() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("DIAGNOSTIC WARNING: No active systems"));
    }
    
    UE_LOG(LogTemp, Log, TEXT("Architecture diagnostics %s"), bDiagnosticsPass ? TEXT("PASSED") : TEXT("FAILED"));
    
    return bDiagnosticsPass;
}

void UEng_GameplayArchitect::InitializeDefaultSystems()
{
    UE_LOG(LogTemp, Log, TEXT("Initializing default gameplay systems..."));
    
    // Register core systems based on configuration
    if (ArchitectureConfig.bEnableBiomeSystem)
    {
        RegisterGameplaySystem(EEng_GameplaySystemType::Biome, TEXT("Biome Management System"));
        ActivateSystem(EEng_GameplaySystemType::Biome);
    }
    
    if (ArchitectureConfig.bEnableSurvivalSystem)
    {
        RegisterGameplaySystem(EEng_GameplaySystemType::Survival, TEXT("Survival Mechanics System"));
        ActivateSystem(EEng_GameplaySystemType::Survival);
    }
    
    if (ArchitectureConfig.bEnableDinosaurSystem)
    {
        RegisterGameplaySystem(EEng_GameplaySystemType::Dinosaur, TEXT("Dinosaur AI System"));
        ActivateSystem(EEng_GameplaySystemType::Dinosaur);
    }
    
    // Always register performance monitoring
    if (ArchitectureConfig.bEnablePerformanceMonitoring)
    {
        RegisterGameplaySystem(EEng_GameplaySystemType::Performance, TEXT("Performance Monitoring System"));
        ActivateSystem(EEng_GameplaySystemType::Performance);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Default systems initialized: %d systems registered"), RegisteredSystems.Num());
}

void UEng_GameplayArchitect::CleanupSystems()
{
    UE_LOG(LogTemp, Log, TEXT("Cleaning up gameplay systems..."));
    
    // Deactivate all systems
    for (auto& SystemPair : RegisteredSystems)
    {
        SystemPair.Value.bIsActive = false;
    }
    
    // Clear all registries
    RegisteredSystems.Empty();
    SystemPerformanceMetrics.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("System cleanup complete"));
}

void UEng_GameplayArchitect::UpdatePerformanceMetrics(EEng_GameplaySystemType SystemType, float UpdateTime)
{
    if (float* CurrentMetric = SystemPerformanceMetrics.Find(SystemType))
    {
        // Use exponential moving average for performance metrics
        *CurrentMetric = (*CurrentMetric * 0.9f) + (UpdateTime * 0.1f);
    }
    else
    {
        SystemPerformanceMetrics.Add(SystemType, UpdateTime);
    }
}

bool UEng_GameplayArchitect::ValidateSystemDependencies(EEng_GameplaySystemType SystemType) const
{
    // Basic dependency validation logic
    // This can be expanded based on specific system requirements
    
    switch (SystemType)
    {
        case EEng_GameplaySystemType::Biome:
            return IsSystemRegistered(EEng_GameplaySystemType::World);
            
        case EEng_GameplaySystemType::Dinosaur:
            return IsSystemRegistered(EEng_GameplaySystemType::Biome);
            
        case EEng_GameplaySystemType::Combat:
            return IsSystemRegistered(EEng_GameplaySystemType::Dinosaur);
            
        default:
            return true; // No dependencies for other systems
    }
}

void UEng_GameplayArchitect::LogSystemEvent(EEng_GameplaySystemType SystemType, const FString& Event) const
{
    FString SystemName = TEXT("Unknown");
    
    if (const FEng_GameplaySystemInfo* SystemInfo = RegisteredSystems.Find(SystemType))
    {
        SystemName = SystemInfo->SystemName;
    }
    else if (SystemType == EEng_GameplaySystemType::None)
    {
        SystemName = TEXT("Architecture");
    }
    
    UE_LOG(LogTemp, Log, TEXT("[%s] %s"), *SystemName, *Event);
}