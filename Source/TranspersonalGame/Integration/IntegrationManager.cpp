#include "IntegrationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Misc/Paths.h"
#include "Stats/Stats.h"
#include "HAL/MemoryBase.h"

DEFINE_LOG_CATEGORY(LogIntegration);

UIntegrationManager::UIntegrationManager()
{
    bIsInitializing = false;
    bIntegrationComplete = false;
    IntegrationStartTime = 0.0f;
    bPerformanceMonitoringActive = false;
}

void UIntegrationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogIntegration, Log, TEXT("IntegrationManager: Initializing..."));
    
    // Initialize default system registry
    RegisterSystem(TEXT("CoreSystems"), ESystemPriority::Critical, {});
    RegisterSystem(TEXT("PhysicsCore"), ESystemPriority::Critical, {TEXT("CoreSystems")});
    RegisterSystem(TEXT("Performance"), ESystemPriority::Critical, {TEXT("CoreSystems")});
    RegisterSystem(TEXT("WorldGeneration"), ESystemPriority::High, {TEXT("CoreSystems"), TEXT("PhysicsCore")});
    RegisterSystem(TEXT("Environment"), ESystemPriority::High, {TEXT("WorldGeneration")});
    RegisterSystem(TEXT("Architecture"), ESystemPriority::High, {TEXT("Environment")});
    RegisterSystem(TEXT("Lighting"), ESystemPriority::High, {TEXT("Architecture")});
    RegisterSystem(TEXT("Characters"), ESystemPriority::High, {TEXT("CoreSystems")});
    RegisterSystem(TEXT("Animation"), ESystemPriority::Medium, {TEXT("Characters")});
    RegisterSystem(TEXT("NPCBehavior"), ESystemPriority::Medium, {TEXT("Characters", TEXT("Animation")});
    RegisterSystem(TEXT("CombatAI"), ESystemPriority::Medium, {TEXT("NPCBehavior")});
    RegisterSystem(TEXT("CrowdSimulation"), ESystemPriority::Medium, {TEXT("NPCBehavior")});
    RegisterSystem(TEXT("Narrative"), ESystemPriority::Low, {TEXT("Characters")});
    RegisterSystem(TEXT("Quest"), ESystemPriority::Low, {TEXT("Narrative")});
    RegisterSystem(TEXT("Audio"), ESystemPriority::Medium, {TEXT("CoreSystems")});
    RegisterSystem(TEXT("VFX"), ESystemPriority::Low, {TEXT("CoreSystems")});
    RegisterSystem(TEXT("QA"), ESystemPriority::Testing, {});
    
    UE_LOG(LogIntegration, Log, TEXT("IntegrationManager: Initialized with %d systems"), RegisteredSystems.Num());
}

void UIntegrationManager::Deinitialize()
{
    UE_LOG(LogIntegration, Log, TEXT("IntegrationManager: Shutting down..."));
    
    ShutdownAllSystems();
    RegisteredSystems.Empty();
    
    Super::Deinitialize();
}

void UIntegrationManager::InitializeAllSystems()
{
    if (bIsInitializing)
    {
        UE_LOG(LogIntegration, Warning, TEXT("InitializeAllSystems: Already initializing"));
        return;
    }
    
    UE_LOG(LogIntegration, Log, TEXT("InitializeAllSystems: Starting initialization of %d systems"), RegisteredSystems.Num());
    
    bIsInitializing = true;
    bIntegrationComplete = false;
    IntegrationStartTime = FPlatformTime::Seconds();
    
    InitializeSystemsByPriority();
}

void UIntegrationManager::ShutdownAllSystems()
{
    UE_LOG(LogIntegration, Log, TEXT("ShutdownAllSystems: Shutting down all systems"));
    
    // Shutdown in reverse priority order
    for (int32 Priority = static_cast<int32>(ESystemPriority::Testing); Priority >= 0; Priority--)
    {
        for (auto& SystemPair : RegisteredSystems)
        {
            if (static_cast<int32>(SystemPair.Value.Priority) == Priority)
            {
                SetSystemStatus(SystemPair.Key, EIntegrationStatus::NotInitialized);
            }
        }
    }
    
    bIsInitializing = false;
    bIntegrationComplete = false;
}

bool UIntegrationManager::RegisterSystem(const FString& SystemName, ESystemPriority Priority, const TArray<FString>& Dependencies)
{
    if (SystemName.IsEmpty())
    {
        UE_LOG(LogIntegration, Error, TEXT("RegisterSystem: SystemName cannot be empty"));
        return false;
    }
    
    if (RegisteredSystems.Contains(SystemName))
    {
        UE_LOG(LogIntegration, Warning, TEXT("RegisterSystem: System '%s' already registered"), *SystemName);
        return false;
    }
    
    FSystemIntegrationInfo NewSystem;
    NewSystem.SystemName = SystemName;
    NewSystem.Priority = Priority;
    NewSystem.Status = EIntegrationStatus::NotInitialized;
    NewSystem.Dependencies = Dependencies;
    NewSystem.InitializationTime = 0.0f;
    
    RegisteredSystems.Add(SystemName, NewSystem);
    
    UE_LOG(LogIntegration, Log, TEXT("RegisterSystem: Registered '%s' with priority %d"), *SystemName, static_cast<int32>(Priority));
    
    return true;
}

bool UIntegrationManager::UnregisterSystem(const FString& SystemName)
{
    if (!RegisteredSystems.Contains(SystemName))
    {
        UE_LOG(LogIntegration, Warning, TEXT("UnregisterSystem: System '%s' not found"), *SystemName);
        return false;
    }
    
    RegisteredSystems.Remove(SystemName);
    UE_LOG(LogIntegration, Log, TEXT("UnregisterSystem: Unregistered '%s'"), *SystemName);
    
    return true;
}

void UIntegrationManager::SetSystemStatus(const FString& SystemName, EIntegrationStatus Status)
{
    if (!RegisteredSystems.Contains(SystemName))
    {
        UE_LOG(LogIntegration, Warning, TEXT("SetSystemStatus: System '%s' not found"), *SystemName);
        return;
    }
    
    FSystemIntegrationInfo& SystemInfo = RegisteredSystems[SystemName];
    EIntegrationStatus OldStatus = SystemInfo.Status;
    SystemInfo.Status = Status;
    
    LogSystemStatus(SystemName, Status);
    
    // Broadcast status change
    OnSystemStatusChanged.Broadcast(SystemName, Status);
    
    // Update overall integration progress
    UpdateIntegrationProgress();
}

EIntegrationStatus UIntegrationManager::GetSystemStatus(const FString& SystemName) const
{
    if (const FSystemIntegrationInfo* SystemInfo = RegisteredSystems.Find(SystemName))
    {
        return SystemInfo->Status;
    }
    
    return EIntegrationStatus::NotInitialized;
}

TArray<FSystemIntegrationInfo> UIntegrationManager::GetAllSystemsInfo() const
{
    TArray<FSystemIntegrationInfo> AllSystems;
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        AllSystems.Add(SystemPair.Value);
    }
    
    // Sort by priority
    AllSystems.Sort([](const FSystemIntegrationInfo& A, const FSystemIntegrationInfo& B)
    {
        return static_cast<int32>(A.Priority) < static_cast<int32>(B.Priority);
    });
    
    return AllSystems;
}

bool UIntegrationManager::AreAllCriticalSystemsReady() const
{
    for (const auto& SystemPair : RegisteredSystems)
    {
        if (SystemPair.Value.Priority == ESystemPriority::Critical)
        {
            if (SystemPair.Value.Status != EIntegrationStatus::Ready)
            {
                return false;
            }
        }
    }
    
    return true;
}

float UIntegrationManager::GetOverallIntegrationProgress() const
{
    if (RegisteredSystems.Num() == 0)
    {
        return 0.0f;
    }
    
    int32 ReadySystems = 0;
    for (const auto& SystemPair : RegisteredSystems)
    {
        if (SystemPair.Value.Status == EIntegrationStatus::Ready)
        {
            ReadySystems++;
        }
    }
    
    return static_cast<float>(ReadySystems) / static_cast<float>(RegisteredSystems.Num());
}

void UIntegrationManager::ValidateBuildIntegrity()
{
    UE_LOG(LogIntegration, Log, TEXT("ValidateBuildIntegrity: Starting build validation"));
    
    // Get Build Validator subsystem
    if (UBuildValidator* BuildValidator = GetWorld()->GetSubsystem<UBuildValidator>())
    {
        BuildValidator->RunFullValidation();
    }
    else
    {
        UE_LOG(LogIntegration, Error, TEXT("ValidateBuildIntegrity: BuildValidator subsystem not available"));
    }
}

void UIntegrationManager::GenerateBuildReport()
{
    UE_LOG(LogIntegration, Log, TEXT("GenerateBuildReport: Generating integration report"));
    
    FString ReportPath = FPaths::ProjectLogDir() / TEXT("Integration") / FString::Printf(TEXT("IntegrationReport_%s.txt"), *FDateTime::Now().ToString());
    
    FString Report = TEXT("=== TRANSPERSONAL GAME INTEGRATION REPORT ===\n\n");
    Report += FString::Printf(TEXT("Generated: %s\n"), *FDateTime::Now().ToString());
    Report += FString::Printf(TEXT("Integration Progress: %.1f%%\n"), GetOverallIntegrationProgress() * 100.0f);
    Report += FString::Printf(TEXT("Critical Systems Ready: %s\n\n"), AreAllCriticalSystemsReady() ? TEXT("YES") : TEXT("NO"));
    
    Report += TEXT("SYSTEM STATUS:\n");
    for (const auto& SystemPair : RegisteredSystems)
    {
        const FSystemIntegrationInfo& SystemInfo = SystemPair.Value;
        FString StatusString;
        
        switch (SystemInfo.Status)
        {
            case EIntegrationStatus::NotInitialized: StatusString = TEXT("NOT_INITIALIZED"); break;
            case EIntegrationStatus::Initializing: StatusString = TEXT("INITIALIZING"); break;
            case EIntegrationStatus::Ready: StatusString = TEXT("READY"); break;
            case EIntegrationStatus::Error: StatusString = TEXT("ERROR"); break;
            case EIntegrationStatus::Disabled: StatusString = TEXT("DISABLED"); break;
        }
        
        Report += FString::Printf(TEXT("  %s: %s (Priority: %d, Init Time: %.2fs)\n"), 
            *SystemInfo.SystemName, *StatusString, static_cast<int32>(SystemInfo.Priority), SystemInfo.InitializationTime);
    }
    
    // Save report to file
    FFileHelper::SaveStringToFile(Report, *ReportPath);
    UE_LOG(LogIntegration, Log, TEXT("GenerateBuildReport: Report saved to %s"), *ReportPath);
}

bool UIntegrationManager::CheckSystemCompatibility(const FString& SystemA, const FString& SystemB)
{
    // Basic compatibility check - systems are compatible if they don't conflict
    // This is a simplified implementation - real compatibility would check for conflicts
    
    if (SystemA == SystemB)
    {
        return true; // System is compatible with itself
    }
    
    // Check if systems exist
    if (!RegisteredSystems.Contains(SystemA) || !RegisteredSystems.Contains(SystemB))
    {
        return false;
    }
    
    // For now, assume all systems are compatible unless explicitly conflicting
    // In a real implementation, you'd have a compatibility matrix
    
    return true;
}

void UIntegrationManager::StartPerformanceMonitoring()
{
    if (bPerformanceMonitoringActive)
    {
        UE_LOG(LogIntegration, Warning, TEXT("StartPerformanceMonitoring: Already active"));
        return;
    }
    
    bPerformanceMonitoringActive = true;
    FrameRateHistory.Empty();
    MemoryUsageHistory.Empty();
    
    UE_LOG(LogIntegration, Log, TEXT("StartPerformanceMonitoring: Performance monitoring started"));
    
    // Start monitoring timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(FTimerHandle(), this, &UIntegrationManager::UpdatePerformanceMetrics, 1.0f, true);
    }
}

void UIntegrationManager::StopPerformanceMonitoring()
{
    if (!bPerformanceMonitoringActive)
    {
        return;
    }
    
    bPerformanceMonitoringActive = false;
    
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearAllTimersForObject(this);
    }
    
    UE_LOG(LogIntegration, Log, TEXT("StopPerformanceMonitoring: Performance monitoring stopped"));
}

float UIntegrationManager::GetCurrentFrameRate() const
{
    if (FrameRateHistory.Num() > 0)
    {
        return FrameRateHistory.Last();
    }
    
    return 0.0f;
}

int32 UIntegrationManager::GetCurrentMemoryUsage() const
{
    if (MemoryUsageHistory.Num() > 0)
    {
        return MemoryUsageHistory.Last();
    }
    
    return 0;
}

void UIntegrationManager::InitializeSystemsByPriority()
{
    // Initialize systems in priority order
    for (int32 Priority = 0; Priority <= static_cast<int32>(ESystemPriority::Testing); Priority++)
    {
        for (auto& SystemPair : RegisteredSystems)
        {
            if (static_cast<int32>(SystemPair.Value.Priority) == Priority)
            {
                InitializeSystem(SystemPair.Key);
            }
        }
    }
}

void UIntegrationManager::InitializeSystem(const FString& SystemName)
{
    if (!RegisteredSystems.Contains(SystemName))
    {
        return;
    }
    
    FSystemIntegrationInfo& SystemInfo = RegisteredSystems[SystemName];
    
    // Check dependencies first
    if (!CheckSystemDependencies(SystemName))
    {
        UE_LOG(LogIntegration, Error, TEXT("InitializeSystem: Dependencies not met for '%s'"), *SystemName);
        SetSystemStatus(SystemName, EIntegrationStatus::Error);
        return;
    }
    
    // Start initialization
    SetSystemStatus(SystemName, EIntegrationStatus::Initializing);
    
    float StartTime = FPlatformTime::Seconds();
    
    // Simulate system initialization
    // In a real implementation, this would call the actual system initialization
    bool bInitSuccess = true; // Assume success for now
    
    float EndTime = FPlatformTime::Seconds();
    SystemInfo.InitializationTime = EndTime - StartTime;
    
    if (bInitSuccess)
    {
        SetSystemStatus(SystemName, EIntegrationStatus::Ready);
    }
    else
    {
        SetSystemStatus(SystemName, EIntegrationStatus::Error);
    }
}

bool UIntegrationManager::CheckSystemDependencies(const FString& SystemName)
{
    const FSystemIntegrationInfo* SystemInfo = RegisteredSystems.Find(SystemName);
    if (!SystemInfo)
    {
        return false;
    }
    
    // Check if all dependencies are ready
    for (const FString& Dependency : SystemInfo->Dependencies)
    {
        const FSystemIntegrationInfo* DepInfo = RegisteredSystems.Find(Dependency);
        if (!DepInfo || DepInfo->Status != EIntegrationStatus::Ready)
        {
            UE_LOG(LogIntegration, Warning, TEXT("CheckSystemDependencies: Dependency '%s' not ready for '%s'"), *Dependency, *SystemName);
            return false;
        }
    }
    
    return true;
}

void UIntegrationManager::UpdateIntegrationProgress()
{
    float Progress = GetOverallIntegrationProgress();
    
    if (Progress >= 1.0f && !bIntegrationComplete)
    {
        bIntegrationComplete = true;
        bIsInitializing = false;
        
        UE_LOG(LogIntegration, Log, TEXT("UpdateIntegrationProgress: Integration complete!"));
        OnIntegrationComplete.Broadcast(true);
    }
}

void UIntegrationManager::LogSystemStatus(const FString& SystemName, EIntegrationStatus Status)
{
    FString StatusString;
    switch (Status)
    {
        case EIntegrationStatus::NotInitialized: StatusString = TEXT("Not Initialized"); break;
        case EIntegrationStatus::Initializing: StatusString = TEXT("Initializing"); break;
        case EIntegrationStatus::Ready: StatusString = TEXT("Ready"); break;
        case EIntegrationStatus::Error: StatusString = TEXT("Error"); break;
        case EIntegrationStatus::Disabled: StatusString = TEXT("Disabled"); break;
    }
    
    UE_LOG(LogIntegration, Log, TEXT("System '%s': %s"), *SystemName, *StatusString);
}

void UIntegrationManager::UpdatePerformanceMetrics()
{
    if (!bPerformanceMonitoringActive)
    {
        return;
    }
    
    // Get current frame rate
    float CurrentFPS = 1.0f / FApp::GetDeltaTime();
    FrameRateHistory.Add(CurrentFPS);
    
    // Get current memory usage (simplified)
    int32 CurrentMemory = FPlatformMemory::GetStats().UsedPhysical / (1024 * 1024); // Convert to MB
    MemoryUsageHistory.Add(CurrentMemory);
    
    // Keep history limited
    const int32 MaxHistorySize = 300; // 5 minutes at 1 second intervals
    if (FrameRateHistory.Num() > MaxHistorySize)
    {
        FrameRateHistory.RemoveAt(0);
    }
    if (MemoryUsageHistory.Num() > MaxHistorySize)
    {
        MemoryUsageHistory.RemoveAt(0);
    }
    
    // Validate performance thresholds
    ValidatePerformanceThresholds();
}

void UIntegrationManager::ValidatePerformanceThresholds()
{
    const float MinAcceptableFPS = 30.0f;
    const int32 MaxAcceptableMemoryMB = 4096; // 4GB
    
    float CurrentFPS = GetCurrentFrameRate();
    int32 CurrentMemory = GetCurrentMemoryUsage();
    
    if (CurrentFPS < MinAcceptableFPS)
    {
        UE_LOG(LogIntegration, Warning, TEXT("Performance: FPS below threshold (%.1f < %.1f)"), CurrentFPS, MinAcceptableFPS);
    }
    
    if (CurrentMemory > MaxAcceptableMemoryMB)
    {
        UE_LOG(LogIntegration, Warning, TEXT("Performance: Memory usage above threshold (%d MB > %d MB)"), CurrentMemory, MaxAcceptableMemoryMB);
    }
}

// Integration Utilities Implementation
bool UIntegrationUtilities::ValidateSystemModule(const FString& ModuleName)
{
    // Check if module exists and is loaded
    return FModuleManager::Get().IsModuleLoaded(*ModuleName);
}

TArray<FString> UIntegrationUtilities::GetMissingDependencies(const FString& SystemName)
{
    TArray<FString> MissingDeps;
    
    // This would check actual module dependencies
    // For now, return empty array
    
    return MissingDeps;
}

bool UIntegrationUtilities::CheckModuleCompilation(const FString& ModuleName)
{
    // Check if module compiled successfully
    return FModuleManager::Get().IsModuleLoaded(*ModuleName);
}

FString UIntegrationUtilities::GenerateSystemReport(const FString& SystemName)
{
    FString Report = FString::Printf(TEXT("=== SYSTEM REPORT: %s ===\n"), *SystemName);
    Report += FString::Printf(TEXT("Generated: %s\n"), *FDateTime::Now().ToString());
    
    // Add system-specific information
    if (UIntegrationManager* IntegrationManager = GEngine->GetEngineSubsystem<UIntegrationManager>())
    {
        EIntegrationStatus Status = IntegrationManager->GetSystemStatus(SystemName);
        Report += FString::Printf(TEXT("Status: %d\n"), static_cast<int32>(Status));
    }
    
    return Report;
}

bool UIntegrationUtilities::ExportIntegrationLog(const FString& FilePath)
{
    // Export integration log to file
    FString LogContent = TEXT("Integration Log Export\n");
    LogContent += FString::Printf(TEXT("Exported: %s\n"), *FDateTime::Now().ToString());
    
    return FFileHelper::SaveStringToFile(LogContent, *FilePath);
}

void UIntegrationUtilities::ClearIntegrationCache()
{
    // Clear any cached integration data
    UE_LOG(LogIntegration, Log, TEXT("Integration cache cleared"));
}

float UIntegrationUtilities::MeasureSystemInitTime(const FString& SystemName)
{
    // Measure system initialization time
    if (UIntegrationManager* IntegrationManager = GEngine->GetEngineSubsystem<UIntegrationManager>())
    {
        TArray<FSystemIntegrationInfo> AllSystems = IntegrationManager->GetAllSystemsInfo();
        for (const FSystemIntegrationInfo& SystemInfo : AllSystems)
        {
            if (SystemInfo.SystemName == SystemName)
            {
                return SystemInfo.InitializationTime;
            }
        }
    }
    
    return 0.0f;
}

int32 UIntegrationUtilities::MeasureSystemMemoryFootprint(const FString& SystemName)
{
    // Measure system memory footprint
    // This would require more detailed memory tracking
    return 0;
}

bool UIntegrationUtilities::ValidatePerformanceTarget(float TargetFPS, int32 MaxMemoryMB)
{
    if (UIntegrationManager* IntegrationManager = GEngine->GetEngineSubsystem<UIntegrationManager>())
    {
        float CurrentFPS = IntegrationManager->GetCurrentFrameRate();
        int32 CurrentMemory = IntegrationManager->GetCurrentMemoryUsage();
        
        return (CurrentFPS >= TargetFPS) && (CurrentMemory <= MaxMemoryMB);
    }
    
    return false;
}