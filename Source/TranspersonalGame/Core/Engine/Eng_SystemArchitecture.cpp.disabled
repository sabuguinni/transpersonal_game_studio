#include "Eng_SystemArchitecture.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"
#include "Engine/GameViewportClient.h"

UEng_SystemArchitecture::UEng_SystemArchitecture()
{
    LastMetricsUpdateTime = 0.0f;
}

void UEng_SystemArchitecture::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architecture System Initializing..."));
    
    // Initialize core system dependencies
    InitializeSystemDependencies();
    
    // Register core systems
    RegisterSystem(EEng_SystemType::Core, TEXT("Core Systems"), TEXT("/Core/"));
    RegisterSystem(EEng_SystemType::WorldGeneration, TEXT("World Generation"), TEXT("/WorldGeneration/"));
    RegisterSystem(EEng_SystemType::Character, TEXT("Character Systems"), TEXT("/Characters/"));
    RegisterSystem(EEng_SystemType::AI, TEXT("AI & Behavior"), TEXT("/AI/"));
    RegisterSystem(EEng_SystemType::Combat, TEXT("Combat Systems"), TEXT("/Combat/"));
    RegisterSystem(EEng_SystemType::Audio, TEXT("Audio Systems"), TEXT("/Audio/"));
    RegisterSystem(EEng_SystemType::VFX, TEXT("Visual Effects"), TEXT("/VFX/"));
    RegisterSystem(EEng_SystemType::Performance, TEXT("Performance Monitor"), TEXT("/Performance/"));
    RegisterSystem(EEng_SystemType::Integration, TEXT("Integration Systems"), TEXT("/Integration/"));
    
    // Update initial metrics
    UpdatePerformanceMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architecture System Initialized Successfully"));
}

void UEng_SystemArchitecture::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architecture System Shutting Down..."));
    
    // Shutdown all systems in reverse dependency order
    ShutdownAllSystems();
    
    // Clear all registrations
    RegisteredSystems.Empty();
    SystemDependencies.Empty();
    
    Super::Deinitialize();
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architecture System Shutdown Complete"));
}

void UEng_SystemArchitecture::RegisterSystem(EEng_SystemType SystemType, const FString& SystemName, const FString& ModulePath)
{
    FEng_SystemInfo SystemInfo;
    SystemInfo.SystemType = SystemType;
    SystemInfo.Status = EEng_SystemStatus::Inactive;
    SystemInfo.SystemName = SystemName;
    SystemInfo.ModulePath = ModulePath;
    SystemInfo.InitializationTime = 0.0f;
    SystemInfo.LastUpdateTime = FPlatformTime::Seconds();
    SystemInfo.ErrorCount = 0;
    SystemInfo.LastError = TEXT("");
    
    RegisteredSystems.Add(SystemType, SystemInfo);
    
    LogSystemEvent(SystemType, FString::Printf(TEXT("System registered: %s at %s"), *SystemName, *ModulePath));
}

void UEng_SystemArchitecture::UnregisterSystem(EEng_SystemType SystemType)
{
    if (RegisteredSystems.Contains(SystemType))
    {
        // Shutdown system before unregistering
        ShutdownSystem(SystemType);
        
        FEng_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemType);
        if (SystemInfo)
        {
            LogSystemEvent(SystemType, FString::Printf(TEXT("System unregistered: %s"), *SystemInfo->SystemName));
        }
        
        RegisteredSystems.Remove(SystemType);
        SystemDependencies.Remove(SystemType);
    }
}

bool UEng_SystemArchitecture::InitializeSystem(EEng_SystemType SystemType)
{
    if (!RegisteredSystems.Contains(SystemType))
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot initialize unregistered system: %d"), (int32)SystemType);
        return false;
    }
    
    if (!CanInitializeSystem(SystemType))
    {
        UE_LOG(LogTemp, Warning, TEXT("System dependencies not met for system: %d"), (int32)SystemType);
        return false;
    }
    
    FEng_SystemInfo& SystemInfo = RegisteredSystems[SystemType];
    
    // Mark as initializing
    UpdateSystemStatus(SystemType, EEng_SystemStatus::Initializing);
    
    double StartTime = FPlatformTime::Seconds();
    
    // Perform system-specific initialization
    bool bInitSuccess = true;
    
    switch (SystemType)
    {
        case EEng_SystemType::Core:
            // Core systems are always available
            break;
            
        case EEng_SystemType::WorldGeneration:
            // Check if world generation components exist
            bInitSuccess = true; // For now, assume success
            break;
            
        case EEng_SystemType::Character:
            // Check if character systems are available
            bInitSuccess = true; // For now, assume success
            break;
            
        case EEng_SystemType::AI:
            // Check if AI systems are available
            bInitSuccess = true; // For now, assume success
            break;
            
        case EEng_SystemType::Combat:
            // Check if combat systems are available
            bInitSuccess = true; // For now, assume success
            break;
            
        case EEng_SystemType::Audio:
            // Check if audio systems are available
            bInitSuccess = true; // For now, assume success
            break;
            
        case EEng_SystemType::VFX:
            // Check if VFX systems are available
            bInitSuccess = true; // For now, assume success
            break;
            
        case EEng_SystemType::Performance:
            // Performance monitoring is always available
            break;
            
        case EEng_SystemType::Integration:
            // Integration systems depend on other systems being active
            break;
    }
    
    double EndTime = FPlatformTime::Seconds();
    SystemInfo.InitializationTime = EndTime - StartTime;
    
    if (bInitSuccess)
    {
        UpdateSystemStatus(SystemType, EEng_SystemStatus::Active);
        LogSystemEvent(SystemType, FString::Printf(TEXT("System initialized successfully in %.3f seconds"), SystemInfo.InitializationTime));
    }
    else
    {
        UpdateSystemStatus(SystemType, EEng_SystemStatus::Error);
        ReportSystemError(SystemType, TEXT("System initialization failed"));
    }
    
    return bInitSuccess;
}

void UEng_SystemArchitecture::ShutdownSystem(EEng_SystemType SystemType)
{
    if (!RegisteredSystems.Contains(SystemType))
    {
        return;
    }
    
    FEng_SystemInfo& SystemInfo = RegisteredSystems[SystemType];
    
    if (SystemInfo.Status == EEng_SystemStatus::Active || SystemInfo.Status == EEng_SystemStatus::Error)
    {
        UpdateSystemStatus(SystemType, EEng_SystemStatus::Shutdown);
        
        // Perform system-specific shutdown
        LogSystemEvent(SystemType, TEXT("System shutdown initiated"));
        
        // Mark as inactive
        UpdateSystemStatus(SystemType, EEng_SystemStatus::Inactive);
        
        LogSystemEvent(SystemType, TEXT("System shutdown complete"));
    }
}

void UEng_SystemArchitecture::ShutdownAllSystems()
{
    // Shutdown in reverse dependency order
    TArray<EEng_SystemType> ShutdownOrder = {
        EEng_SystemType::Integration,
        EEng_SystemType::VFX,
        EEng_SystemType::Audio,
        EEng_SystemType::Combat,
        EEng_SystemType::AI,
        EEng_SystemType::Character,
        EEng_SystemType::WorldGeneration,
        EEng_SystemType::Performance,
        EEng_SystemType::Core
    };
    
    for (EEng_SystemType SystemType : ShutdownOrder)
    {
        ShutdownSystem(SystemType);
    }
}

EEng_SystemStatus UEng_SystemArchitecture::GetSystemStatus(EEng_SystemType SystemType) const
{
    const FEng_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemType);
    return SystemInfo ? SystemInfo->Status : EEng_SystemStatus::Inactive;
}

FEng_SystemInfo UEng_SystemArchitecture::GetSystemInfo(EEng_SystemType SystemType) const
{
    const FEng_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemType);
    return SystemInfo ? *SystemInfo : FEng_SystemInfo();
}

TArray<FEng_SystemInfo> UEng_SystemArchitecture::GetAllSystemInfo() const
{
    TArray<FEng_SystemInfo> AllSystemInfo;
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        AllSystemInfo.Add(SystemPair.Value);
    }
    
    return AllSystemInfo;
}

bool UEng_SystemArchitecture::IsSystemActive(EEng_SystemType SystemType) const
{
    return GetSystemStatus(SystemType) == EEng_SystemStatus::Active;
}

FEng_PerformanceMetrics UEng_SystemArchitecture::GetCurrentPerformanceMetrics() const
{
    return CurrentMetrics;
}

void UEng_SystemArchitecture::UpdatePerformanceMetrics()
{
    double CurrentTime = FPlatformTime::Seconds();
    
    // Update frame rate
    if (GEngine && GEngine->GetGameViewport())
    {
        CurrentMetrics.FrameRate = 1.0f / FApp::GetDeltaTime();
    }
    
    // Update memory usage (approximation)
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    
    // CPU and GPU usage would require platform-specific code
    CurrentMetrics.CPUUsagePercent = 0.0f; // Placeholder
    CurrentMetrics.GPUUsagePercent = 0.0f; // Placeholder
    
    // Update actor counts if we have a world
    if (UWorld* World = GetWorld())
    {
        CurrentMetrics.ActiveActorCount = World->GetActorCount();
        CurrentMetrics.VisibleActorCount = CurrentMetrics.ActiveActorCount; // Simplified
    }
    
    LastMetricsUpdateTime = CurrentTime;
}

void UEng_SystemArchitecture::SetSystemDependency(EEng_SystemType System, EEng_SystemType Dependency)
{
    if (!SystemDependencies.Contains(System))
    {
        SystemDependencies.Add(System, TArray<EEng_SystemType>());
    }
    
    SystemDependencies[System].AddUnique(Dependency);
    
    LogSystemEvent(System, FString::Printf(TEXT("Dependency added: %d"), (int32)Dependency));
}

bool UEng_SystemArchitecture::ValidateSystemDependencies() const
{
    bool bAllDependenciesMet = true;
    
    for (const auto& SystemPair : SystemDependencies)
    {
        EEng_SystemType System = SystemPair.Key;
        const TArray<EEng_SystemType>& Dependencies = SystemPair.Value;
        
        for (EEng_SystemType Dependency : Dependencies)
        {
            if (!IsSystemActive(Dependency))
            {
                UE_LOG(LogTemp, Warning, TEXT("System %d dependency not met: %d"), (int32)System, (int32)Dependency);
                bAllDependenciesMet = false;
            }
        }
    }
    
    return bAllDependenciesMet;
}

void UEng_SystemArchitecture::ReportSystemError(EEng_SystemType SystemType, const FString& ErrorMessage)
{
    FEng_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemType);
    if (SystemInfo)
    {
        SystemInfo->ErrorCount++;
        SystemInfo->LastError = ErrorMessage;
        SystemInfo->LastUpdateTime = FPlatformTime::Seconds();
        
        UpdateSystemStatus(SystemType, EEng_SystemStatus::Error);
        
        UE_LOG(LogTemp, Error, TEXT("System Error [%s]: %s"), *SystemInfo->SystemName, *ErrorMessage);
    }
}

void UEng_SystemArchitecture::ClearSystemErrors(EEng_SystemType SystemType)
{
    FEng_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemType);
    if (SystemInfo)
    {
        SystemInfo->ErrorCount = 0;
        SystemInfo->LastError = TEXT("");
        SystemInfo->LastUpdateTime = FPlatformTime::Seconds();
        
        LogSystemEvent(SystemType, TEXT("Errors cleared"));
    }
}

void UEng_SystemArchitecture::PrintSystemStatus() const
{
    UE_LOG(LogTemp, Warning, TEXT("=== SYSTEM ARCHITECTURE STATUS ==="));
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        const FEng_SystemInfo& SystemInfo = SystemPair.Value;
        FString StatusString;
        
        switch (SystemInfo.Status)
        {
            case EEng_SystemStatus::Inactive: StatusString = TEXT("INACTIVE"); break;
            case EEng_SystemStatus::Initializing: StatusString = TEXT("INITIALIZING"); break;
            case EEng_SystemStatus::Active: StatusString = TEXT("ACTIVE"); break;
            case EEng_SystemStatus::Error: StatusString = TEXT("ERROR"); break;
            case EEng_SystemStatus::Shutdown: StatusString = TEXT("SHUTDOWN"); break;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("%s: %s (Errors: %d)"), *SystemInfo.SystemName, *StatusString, SystemInfo.ErrorCount);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Performance: %.1f FPS, %.1f MB Memory"), CurrentMetrics.FrameRate, CurrentMetrics.MemoryUsageMB);
    UE_LOG(LogTemp, Warning, TEXT("=== END STATUS ==="));
}

void UEng_SystemArchitecture::ValidateArchitecture() const
{
    UE_LOG(LogTemp, Warning, TEXT("=== ARCHITECTURE VALIDATION ==="));
    
    bool bValidationPassed = true;
    
    // Check if all core systems are registered
    TArray<EEng_SystemType> RequiredSystems = {
        EEng_SystemType::Core,
        EEng_SystemType::WorldGeneration,
        EEng_SystemType::Character,
        EEng_SystemType::Performance
    };
    
    for (EEng_SystemType RequiredSystem : RequiredSystems)
    {
        if (!RegisteredSystems.Contains(RequiredSystem))
        {
            UE_LOG(LogTemp, Error, TEXT("Required system not registered: %d"), (int32)RequiredSystem);
            bValidationPassed = false;
        }
    }
    
    // Validate dependencies
    if (!ValidateSystemDependencies())
    {
        UE_LOG(LogTemp, Error, TEXT("System dependencies not met"));
        bValidationPassed = false;
    }
    
    // Check for systems in error state
    for (const auto& SystemPair : RegisteredSystems)
    {
        if (SystemPair.Value.Status == EEng_SystemStatus::Error)
        {
            UE_LOG(LogTemp, Error, TEXT("System in error state: %s"), *SystemPair.Value.SystemName);
            bValidationPassed = false;
        }
    }
    
    if (bValidationPassed)
    {
        UE_LOG(LogTemp, Warning, TEXT("Architecture validation PASSED"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Architecture validation FAILED"));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== END VALIDATION ==="));
}

void UEng_SystemArchitecture::InitializeSystemDependencies()
{
    // Core has no dependencies
    
    // World Generation depends on Core
    SetSystemDependency(EEng_SystemType::WorldGeneration, EEng_SystemType::Core);
    
    // Character depends on Core and World Generation
    SetSystemDependency(EEng_SystemType::Character, EEng_SystemType::Core);
    SetSystemDependency(EEng_SystemType::Character, EEng_SystemType::WorldGeneration);
    
    // AI depends on Character
    SetSystemDependency(EEng_SystemType::AI, EEng_SystemType::Character);
    
    // Combat depends on Character and AI
    SetSystemDependency(EEng_SystemType::Combat, EEng_SystemType::Character);
    SetSystemDependency(EEng_SystemType::Combat, EEng_SystemType::AI);
    
    // Audio depends on Core
    SetSystemDependency(EEng_SystemType::Audio, EEng_SystemType::Core);
    
    // VFX depends on Core
    SetSystemDependency(EEng_SystemType::VFX, EEng_SystemType::Core);
    
    // Performance depends on Core
    SetSystemDependency(EEng_SystemType::Performance, EEng_SystemType::Core);
    
    // Integration depends on all other systems
    SetSystemDependency(EEng_SystemType::Integration, EEng_SystemType::Core);
    SetSystemDependency(EEng_SystemType::Integration, EEng_SystemType::WorldGeneration);
    SetSystemDependency(EEng_SystemType::Integration, EEng_SystemType::Character);
    SetSystemDependency(EEng_SystemType::Integration, EEng_SystemType::AI);
}

void UEng_SystemArchitecture::UpdateSystemStatus(EEng_SystemType SystemType, EEng_SystemStatus NewStatus)
{
    FEng_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemType);
    if (SystemInfo)
    {
        SystemInfo->Status = NewStatus;
        SystemInfo->LastUpdateTime = FPlatformTime::Seconds();
    }
}

bool UEng_SystemArchitecture::CanInitializeSystem(EEng_SystemType SystemType) const
{
    const TArray<EEng_SystemType>* Dependencies = SystemDependencies.Find(SystemType);
    if (!Dependencies)
    {
        return true; // No dependencies
    }
    
    for (EEng_SystemType Dependency : *Dependencies)
    {
        if (!IsSystemActive(Dependency))
        {
            return false;
        }
    }
    
    return true;
}

void UEng_SystemArchitecture::LogSystemEvent(EEng_SystemType SystemType, const FString& Event) const
{
    const FEng_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemType);
    FString SystemName = SystemInfo ? SystemInfo->SystemName : TEXT("Unknown");
    
    UE_LOG(LogTemp, Log, TEXT("[%s] %s"), *SystemName, *Event);
}

// World Architecture Monitor Implementation

UEng_WorldArchitectureMonitor::UEng_WorldArchitectureMonitor()
{
    MonitoringInterval = 1.0f; // Monitor every second
    LastMonitoringTime = 0.0f;
    WorldPerformanceScore = 100.0f;
}

void UEng_WorldArchitectureMonitor::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("World Architecture Monitor Initialized"));
}

void UEng_WorldArchitectureMonitor::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("World Architecture Monitor Shutdown"));
    
    Super::Deinitialize();
}

void UEng_WorldArchitectureMonitor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    if (CurrentTime - LastMonitoringTime >= MonitoringInterval)
    {
        MonitorWorldSystems();
        LastMonitoringTime = CurrentTime;
    }
}

bool UEng_WorldArchitectureMonitor::ShouldCreateSubsystem(UObject* Outer) const
{
    return Super::ShouldCreateSubsystem(Outer);
}

void UEng_WorldArchitectureMonitor::MonitorWorldSystems()
{
    UpdateWorldMetrics();
    CheckSystemHealth();
}

int32 UEng_WorldArchitectureMonitor::GetActiveActorCount() const
{
    if (UWorld* World = GetWorld())
    {
        return World->GetActorCount();
    }
    return 0;
}

int32 UEng_WorldArchitectureMonitor::GetVisibleActorCount() const
{
    // For now, return the same as active count
    // In a real implementation, this would check visibility culling
    return GetActiveActorCount();
}

float UEng_WorldArchitectureMonitor::GetWorldPerformanceScore() const
{
    return WorldPerformanceScore;
}

void UEng_WorldArchitectureMonitor::UpdateWorldMetrics()
{
    // Update performance score based on various factors
    float FrameRate = 1.0f / FApp::GetDeltaTime();
    int32 ActorCount = GetActiveActorCount();
    
    // Simple performance scoring
    WorldPerformanceScore = 100.0f;
    
    // Penalize low frame rate
    if (FrameRate < 30.0f)
    {
        WorldPerformanceScore -= (30.0f - FrameRate) * 2.0f;
    }
    
    // Penalize high actor count
    if (ActorCount > 1000)
    {
        WorldPerformanceScore -= (ActorCount - 1000) * 0.01f;
    }
    
    // Clamp score
    WorldPerformanceScore = FMath::Clamp(WorldPerformanceScore, 0.0f, 100.0f);
}

void UEng_WorldArchitectureMonitor::CheckSystemHealth()
{
    // Check if the main architecture system is available
    if (UEng_SystemArchitecture* ArchSystem = GetGameInstance()->GetSubsystem<UEng_SystemArchitecture>())
    {
        // Update performance metrics in the main system
        ArchSystem->UpdatePerformanceMetrics();
        
        // Check for any systems in error state
        TArray<FEng_SystemInfo> AllSystems = ArchSystem->GetAllSystemInfo();
        for (const FEng_SystemInfo& SystemInfo : AllSystems)
        {
            if (SystemInfo.Status == EEng_SystemStatus::Error)
            {
                UE_LOG(LogTemp, Warning, TEXT("World Monitor detected system error: %s"), *SystemInfo.SystemName);
            }
        }
    }
}