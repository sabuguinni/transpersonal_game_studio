#include "Eng_CoreArchitecture.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Stats/Stats.h"
#include "HAL/PlatformFilemanager.h"

UEng_CoreArchitecture::UEng_CoreArchitecture()
{
    TargetFrameTime = 16.67f; // 60 FPS target
    MaxDrawCalls = 2000;
    MaxMemoryUsageMB = 4096.0f;
    bEnablePerformanceMonitoring = true;
    PerformanceUpdateInterval = 1.0f;
}

void UEng_CoreArchitecture::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architecture System Initializing..."));
    
    InitializeCoreArchitecture();
    RegisterCoreGameSystems();
    
    // Start performance monitoring
    if (bEnablePerformanceMonitoring)
    {
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().SetTimer(
                PerformanceTimerHandle,
                this,
                &UEng_CoreArchitecture::UpdatePerformanceMetrics,
                PerformanceUpdateInterval,
                true
            );
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architecture System Initialized Successfully"));
}

void UEng_CoreArchitecture::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architecture System Shutting Down..."));
    
    // Clear performance timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(PerformanceTimerHandle);
    }
    
    // Clear all registered systems
    RegisteredSystems.Empty();
    
    Super::Deinitialize();
}

void UEng_CoreArchitecture::InitializeCoreArchitecture()
{
    // Initialize performance metrics
    CurrentMetrics = FEng_PerformanceMetrics();
    CurrentMetrics.OverallLevel = EEng_PerformanceLevel::Acceptable;
    
    UE_LOG(LogTemp, Log, TEXT("Core Architecture initialized with default settings"));
}

void UEng_CoreArchitecture::RegisterCoreGameSystems()
{
    // Register essential game systems in priority order
    TArray<FString> NoDependencies;
    TArray<FString> CoreDependencies = { TEXT("CoreArchitecture") };
    TArray<FString> WorldDependencies = { TEXT("CoreArchitecture"), TEXT("Physics") };
    TArray<FString> GameplayDependencies = { TEXT("CoreArchitecture"), TEXT("WorldGeneration"), TEXT("Physics") };
    
    // Core systems (highest priority)
    RegisterSystem(TEXT("CoreArchitecture"), 1000, NoDependencies);
    RegisterSystem(TEXT("Physics"), 900, CoreDependencies);
    RegisterSystem(TEXT("Rendering"), 850, CoreDependencies);
    
    // World systems
    RegisterSystem(TEXT("WorldGeneration"), 800, CoreDependencies);
    RegisterSystem(TEXT("Environment"), 750, WorldDependencies);
    RegisterSystem(TEXT("Lighting"), 700, WorldDependencies);
    
    // Gameplay systems
    RegisterSystem(TEXT("CharacterSystems"), 650, GameplayDependencies);
    RegisterSystem(TEXT("AI"), 600, GameplayDependencies);
    RegisterSystem(TEXT("Combat"), 550, GameplayDependencies);
    RegisterSystem(TEXT("Audio"), 500, GameplayDependencies);
    RegisterSystem(TEXT("VFX"), 450, GameplayDependencies);
    
    // Set initial status for core systems
    SetSystemStatus(TEXT("CoreArchitecture"), EEng_SystemStatus::Active);
    SetSystemStatus(TEXT("Physics"), EEng_SystemStatus::Active);
    SetSystemStatus(TEXT("Rendering"), EEng_SystemStatus::Active);
    
    UE_LOG(LogTemp, Log, TEXT("Registered %d core game systems"), RegisteredSystems.Num());
}

void UEng_CoreArchitecture::RegisterSystem(const FString& SystemName, int32 Priority, const TArray<FString>& Dependencies)
{
    FEng_SystemInfo SystemInfo;
    SystemInfo.SystemName = SystemName;
    SystemInfo.Status = EEng_SystemStatus::Uninitialized;
    SystemInfo.Priority = Priority;
    SystemInfo.Dependencies = Dependencies;
    SystemInfo.InitializationTime = 0.0f;
    
    RegisteredSystems.Add(SystemName, SystemInfo);
    
    UE_LOG(LogTemp, Log, TEXT("Registered system: %s (Priority: %d)"), *SystemName, Priority);
}

void UEng_CoreArchitecture::UnregisterSystem(const FString& SystemName)
{
    if (RegisteredSystems.Contains(SystemName))
    {
        RegisteredSystems.Remove(SystemName);
        UE_LOG(LogTemp, Log, TEXT("Unregistered system: %s"), *SystemName);
    }
}

bool UEng_CoreArchitecture::IsSystemRegistered(const FString& SystemName) const
{
    return RegisteredSystems.Contains(SystemName);
}

EEng_SystemStatus UEng_CoreArchitecture::GetSystemStatus(const FString& SystemName) const
{
    if (const FEng_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemName))
    {
        return SystemInfo->Status;
    }
    return EEng_SystemStatus::Uninitialized;
}

void UEng_CoreArchitecture::SetSystemStatus(const FString& SystemName, EEng_SystemStatus NewStatus)
{
    if (FEng_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemName))
    {
        SystemInfo->Status = NewStatus;
        UE_LOG(LogTemp, Log, TEXT("System %s status changed to: %d"), *SystemName, (int32)NewStatus);
    }
}

void UEng_CoreArchitecture::UpdatePerformanceMetrics()
{
    // Get current frame time
    CurrentMetrics.FrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds
    
    // Estimate thread times (simplified)
    CurrentMetrics.GameThreadTime = CurrentMetrics.FrameTime * 0.6f;
    CurrentMetrics.RenderThreadTime = CurrentMetrics.FrameTime * 0.4f;
    
    // Estimate draw calls (simplified - would need actual render stats in production)
    CurrentMetrics.DrawCalls = FMath::RandRange(800, 1500);
    
    // Estimate memory usage (simplified)
    CurrentMetrics.MemoryUsageMB = FPlatformMemory::GetStats().UsedPhysical / (1024.0f * 1024.0f);
    
    // Calculate overall performance level
    CalculatePerformanceLevel();
}

void UEng_CoreArchitecture::CalculatePerformanceLevel()
{
    int32 Score = 0;
    
    // Frame time scoring (60 FPS = 16.67ms, 30 FPS = 33.33ms)
    if (CurrentMetrics.FrameTime <= 16.67f) Score += 2;
    else if (CurrentMetrics.FrameTime <= 33.33f) Score += 1;
    else Score -= 1;
    
    // Draw calls scoring
    if (CurrentMetrics.DrawCalls <= 1000) Score += 2;
    else if (CurrentMetrics.DrawCalls <= 2000) Score += 1;
    else Score -= 1;
    
    // Memory usage scoring
    if (CurrentMetrics.MemoryUsageMB <= 2048.0f) Score += 2;
    else if (CurrentMetrics.MemoryUsageMB <= 4096.0f) Score += 1;
    else Score -= 1;
    
    // Determine performance level
    if (Score >= 5) CurrentMetrics.OverallLevel = EEng_PerformanceLevel::Excellent;
    else if (Score >= 3) CurrentMetrics.OverallLevel = EEng_PerformanceLevel::Good;
    else if (Score >= 1) CurrentMetrics.OverallLevel = EEng_PerformanceLevel::Acceptable;
    else if (Score >= -1) CurrentMetrics.OverallLevel = EEng_PerformanceLevel::Poor;
    else CurrentMetrics.OverallLevel = EEng_PerformanceLevel::Critical;
}

FEng_PerformanceMetrics UEng_CoreArchitecture::GetCurrentPerformanceMetrics() const
{
    return CurrentMetrics;
}

bool UEng_CoreArchitecture::IsPerformanceAcceptable() const
{
    return CurrentMetrics.OverallLevel >= EEng_PerformanceLevel::Acceptable;
}

bool UEng_CoreArchitecture::ValidateSystemDependencies()
{
    bool bAllValid = true;
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        const FString& SystemName = SystemPair.Key;
        const FEng_SystemInfo& SystemInfo = SystemPair.Value;
        
        TArray<FString> VisitedSystems;
        if (!CheckSystemDependencies(SystemName, VisitedSystems))
        {
            UE_LOG(LogTemp, Error, TEXT("System %s has invalid dependencies"), *SystemName);
            bAllValid = false;
        }
    }
    
    return bAllValid;
}

bool UEng_CoreArchitecture::CheckSystemDependencies(const FString& SystemName, TArray<FString>& VisitedSystems) const
{
    // Check for circular dependencies
    if (VisitedSystems.Contains(SystemName))
    {
        UE_LOG(LogTemp, Error, TEXT("Circular dependency detected involving system: %s"), *SystemName);
        return false;
    }
    
    VisitedSystems.Add(SystemName);
    
    const FEng_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemName);
    if (!SystemInfo)
    {
        return false;
    }
    
    // Check all dependencies exist and are valid
    for (const FString& Dependency : SystemInfo->Dependencies)
    {
        if (!RegisteredSystems.Contains(Dependency))
        {
            UE_LOG(LogTemp, Error, TEXT("System %s depends on non-existent system: %s"), *SystemName, *Dependency);
            return false;
        }
        
        if (!CheckSystemDependencies(Dependency, VisitedSystems))
        {
            return false;
        }
    }
    
    return true;
}

TArray<FString> UEng_CoreArchitecture::GetSystemInitializationOrder() const
{
    TArray<FString> InitOrder;
    TArray<TPair<FString, int32>> SystemPriorities;
    
    // Collect all systems with their priorities
    for (const auto& SystemPair : RegisteredSystems)
    {
        SystemPriorities.Add(TPair<FString, int32>(SystemPair.Key, SystemPair.Value.Priority));
    }
    
    // Sort by priority (highest first)
    SystemPriorities.Sort([](const TPair<FString, int32>& A, const TPair<FString, int32>& B)
    {
        return A.Value > B.Value;
    });
    
    // Extract system names in order
    for (const auto& SystemPriority : SystemPriorities)
    {
        InitOrder.Add(SystemPriority.Key);
    }
    
    return InitOrder;
}

void UEng_CoreArchitecture::InitializeAllSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("Initializing all registered systems..."));
    
    TArray<FString> InitOrder = GetSystemInitializationOrder();
    
    for (const FString& SystemName : InitOrder)
    {
        if (FEng_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemName))
        {
            if (SystemInfo->Status == EEng_SystemStatus::Uninitialized)
            {
                UE_LOG(LogTemp, Log, TEXT("Initializing system: %s"), *SystemName);
                
                // Mark as initializing
                SystemInfo->Status = EEng_SystemStatus::Initializing;
                
                // Simulate initialization time
                float StartTime = FPlatformTime::Seconds();
                
                // In a real implementation, this would call the actual system initialization
                // For now, we'll just mark it as active
                SystemInfo->Status = EEng_SystemStatus::Active;
                
                SystemInfo->InitializationTime = (FPlatformTime::Seconds() - StartTime) * 1000.0f;
                
                UE_LOG(LogTemp, Log, TEXT("System %s initialized in %.2fms"), *SystemName, SystemInfo->InitializationTime);
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("All systems initialization completed"));
}

void UEng_CoreArchitecture::LogSystemStatus() const
{
    UE_LOG(LogTemp, Warning, TEXT("=== ENGINE ARCHITECTURE SYSTEM STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Total Registered Systems: %d"), RegisteredSystems.Num());
    
    TArray<FString> InitOrder = GetSystemInitializationOrder();
    
    for (const FString& SystemName : InitOrder)
    {
        if (const FEng_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemName))
        {
            FString StatusString;
            switch (SystemInfo->Status)
            {
                case EEng_SystemStatus::Uninitialized: StatusString = TEXT("UNINITIALIZED"); break;
                case EEng_SystemStatus::Initializing: StatusString = TEXT("INITIALIZING"); break;
                case EEng_SystemStatus::Active: StatusString = TEXT("ACTIVE"); break;
                case EEng_SystemStatus::Error: StatusString = TEXT("ERROR"); break;
                case EEng_SystemStatus::Disabled: StatusString = TEXT("DISABLED"); break;
            }
            
            UE_LOG(LogTemp, Warning, TEXT("  %s: %s (Priority: %d, Init: %.2fms)"), 
                   *SystemName, *StatusString, SystemInfo->Priority, SystemInfo->InitializationTime);
        }
    }
    
    // Log performance metrics
    UE_LOG(LogTemp, Warning, TEXT("=== PERFORMANCE METRICS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Frame Time: %.2fms"), CurrentMetrics.FrameTime);
    UE_LOG(LogTemp, Warning, TEXT("Draw Calls: %d"), CurrentMetrics.DrawCalls);
    UE_LOG(LogTemp, Warning, TEXT("Memory Usage: %.2fMB"), CurrentMetrics.MemoryUsageMB);
    
    FString PerfLevelString;
    switch (CurrentMetrics.OverallLevel)
    {
        case EEng_PerformanceLevel::Critical: PerfLevelString = TEXT("CRITICAL"); break;
        case EEng_PerformanceLevel::Poor: PerfLevelString = TEXT("POOR"); break;
        case EEng_PerformanceLevel::Acceptable: PerfLevelString = TEXT("ACCEPTABLE"); break;
        case EEng_PerformanceLevel::Good: PerfLevelString = TEXT("GOOD"); break;
        case EEng_PerformanceLevel::Excellent: PerfLevelString = TEXT("EXCELLENT"); break;
    }
    UE_LOG(LogTemp, Warning, TEXT("Overall Performance: %s"), *PerfLevelString);
}

TArray<FEng_SystemInfo> UEng_CoreArchitecture::GetAllSystemInfo() const
{
    TArray<FEng_SystemInfo> AllSystems;
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        AllSystems.Add(SystemPair.Value);
    }
    
    // Sort by priority
    AllSystems.Sort([](const FEng_SystemInfo& A, const FEng_SystemInfo& B)
    {
        return A.Priority > B.Priority;
    });
    
    return AllSystems;
}