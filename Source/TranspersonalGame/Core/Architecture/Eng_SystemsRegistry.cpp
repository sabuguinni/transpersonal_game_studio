#include "Eng_SystemsRegistry.h"
#include "Engine/World.h"
#include "TimerManager.h"

UEng_SystemsRegistry::UEng_SystemsRegistry()
{
    MaxPerformanceSamples = 100;
    SystemTimeoutSeconds = 30.0f;
    bEnablePerformanceTracking = true;
    bLogSystemEvents = true;
}

void UEng_SystemsRegistry::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    if (bLogSystemEvents)
    {
        UE_LOG(LogTemp, Log, TEXT("Engine Architect - Systems Registry initialized"));
    }
    
    // Register core engine systems that must be available
    RegisterSystem("PhysicsCore", EEng_SystemPriority::Critical, {});
    RegisterSystem("WorldGeneration", EEng_SystemPriority::Critical, {"PhysicsCore"});
    RegisterSystem("CharacterSystem", EEng_SystemPriority::High, {"PhysicsCore", "WorldGeneration"});
    RegisterSystem("DinosaurAI", EEng_SystemPriority::High, {"PhysicsCore", "WorldGeneration"});
    RegisterSystem("BiomeManager", EEng_SystemPriority::High, {"WorldGeneration"});
    RegisterSystem("SurvivalSystem", EEng_SystemPriority::Medium, {"CharacterSystem"});
    RegisterSystem("QuestSystem", EEng_SystemPriority::Medium, {"CharacterSystem", "DinosaurAI"});
    RegisterSystem("AudioSystem", EEng_SystemPriority::Medium, {"WorldGeneration"});
    RegisterSystem("VFXSystem", EEng_SystemPriority::Low, {"WorldGeneration"});
    RegisterSystem("UISystem", EEng_SystemPriority::Low, {"SurvivalSystem", "QuestSystem"});
    
    // Validate all dependencies are correct
    if (!ValidateSystemDependencies())
    {
        UE_LOG(LogTemp, Error, TEXT("Engine Architect - System dependency validation failed!"));
    }
}

void UEng_SystemsRegistry::Deinitialize()
{
    if (bLogSystemEvents)
    {
        UE_LOG(LogTemp, Log, TEXT("Engine Architect - Systems Registry shutting down"));
    }
    
    ShutdownAllSystems();
    RegisteredSystems.Empty();
    SystemUpdateTimes.Empty();
    
    Super::Deinitialize();
}

bool UEng_SystemsRegistry::RegisterSystem(const FString& SystemName, EEng_SystemPriority Priority, const TArray<FString>& Dependencies)
{
    if (SystemName.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot register system with empty name"));
        return false;
    }
    
    if (RegisteredSystems.Contains(SystemName))
    {
        UE_LOG(LogTemp, Warning, TEXT("System %s already registered, updating info"), *SystemName);
    }
    
    FEng_SystemInfo SystemInfo;
    SystemInfo.SystemName = SystemName;
    SystemInfo.Status = EEng_SystemStatus::Uninitialized;
    SystemInfo.Priority = Priority;
    SystemInfo.Dependencies = Dependencies;
    SystemInfo.InitializationTime = 0.0f;
    SystemInfo.LastUpdateTime = FPlatformTime::Seconds();
    
    RegisteredSystems.Add(SystemName, SystemInfo);
    
    if (bEnablePerformanceTracking)
    {
        SystemUpdateTimes.Add(SystemName, TArray<float>());
    }
    
    if (bLogSystemEvents)
    {
        UE_LOG(LogTemp, Log, TEXT("Registered system: %s (Priority: %d, Dependencies: %d)"), 
               *SystemName, (int32)Priority, Dependencies.Num());
    }
    
    return true;
}

bool UEng_SystemsRegistry::UnregisterSystem(const FString& SystemName)
{
    if (!RegisteredSystems.Contains(SystemName))
    {
        return false;
    }
    
    // Shutdown system before unregistering
    SetSystemStatus(SystemName, EEng_SystemStatus::Shutdown);
    
    RegisteredSystems.Remove(SystemName);
    SystemUpdateTimes.Remove(SystemName);
    
    if (bLogSystemEvents)
    {
        UE_LOG(LogTemp, Log, TEXT("Unregistered system: %s"), *SystemName);
    }
    
    return true;
}

bool UEng_SystemsRegistry::SetSystemStatus(const FString& SystemName, EEng_SystemStatus NewStatus, const FString& ErrorMessage)
{
    FEng_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemName);
    if (!SystemInfo)
    {
        return false;
    }
    
    EEng_SystemStatus OldStatus = SystemInfo->Status;
    SystemInfo->Status = NewStatus;
    SystemInfo->LastUpdateTime = FPlatformTime::Seconds();
    
    if (!ErrorMessage.IsEmpty())
    {
        SystemInfo->ErrorMessage = ErrorMessage;
        if (bLogSystemEvents)
        {
            UE_LOG(LogTemp, Error, TEXT("System %s error: %s"), *SystemName, *ErrorMessage);
        }
        OnSystemError.Broadcast(SystemName);
    }
    
    if (OldStatus != NewStatus)
    {
        if (bLogSystemEvents)
        {
            UE_LOG(LogTemp, Log, TEXT("System %s status changed: %d -> %d"), 
                   *SystemName, (int32)OldStatus, (int32)NewStatus);
        }
        OnSystemStatusChanged.Broadcast(SystemName, NewStatus);
    }
    
    // Check if all systems are ready
    bool bAllReady = true;
    for (const auto& System : RegisteredSystems)
    {
        if (System.Value.Status != EEng_SystemStatus::Ready && System.Value.Status != EEng_SystemStatus::Running)
        {
            bAllReady = false;
            break;
        }
    }
    
    if (bAllReady)
    {
        OnAllSystemsReady.Broadcast();
    }
    
    return true;
}

EEng_SystemStatus UEng_SystemsRegistry::GetSystemStatus(const FString& SystemName) const
{
    const FEng_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemName);
    return SystemInfo ? SystemInfo->Status : EEng_SystemStatus::Uninitialized;
}

bool UEng_SystemsRegistry::IsSystemReady(const FString& SystemName) const
{
    EEng_SystemStatus Status = GetSystemStatus(SystemName);
    return Status == EEng_SystemStatus::Ready || Status == EEng_SystemStatus::Running;
}

bool UEng_SystemsRegistry::AreSystemDependenciesReady(const FString& SystemName) const
{
    const FEng_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemName);
    if (!SystemInfo)
    {
        return false;
    }
    
    for (const FString& Dependency : SystemInfo->Dependencies)
    {
        if (!IsSystemReady(Dependency))
        {
            return false;
        }
    }
    
    return true;
}

TArray<FString> UEng_SystemsRegistry::GetSystemsByPriority(EEng_SystemPriority Priority) const
{
    TArray<FString> Systems;
    for (const auto& System : RegisteredSystems)
    {
        if (System.Value.Priority == Priority)
        {
            Systems.Add(System.Key);
        }
    }
    return Systems;
}

TArray<FString> UEng_SystemsRegistry::GetSystemsByStatus(EEng_SystemStatus Status) const
{
    TArray<FString> Systems;
    for (const auto& System : RegisteredSystems)
    {
        if (System.Value.Status == Status)
        {
            Systems.Add(System.Key);
        }
    }
    return Systems;
}

TArray<FEng_SystemInfo> UEng_SystemsRegistry::GetAllSystemInfo() const
{
    TArray<FEng_SystemInfo> SystemInfos;
    for (const auto& System : RegisteredSystems)
    {
        SystemInfos.Add(System.Value);
    }
    return SystemInfos;
}

void UEng_SystemsRegistry::InitializeSystemsInOrder()
{
    if (bLogSystemEvents)
    {
        UE_LOG(LogTemp, Log, TEXT("Engine Architect - Initializing systems in dependency order"));
    }
    
    // Get all systems sorted by priority
    TArray<FString> SystemNames;
    RegisteredSystems.GetKeys(SystemNames);
    SortSystemsByPriority(SystemNames);
    
    // Initialize systems respecting dependencies
    for (const FString& SystemName : SystemNames)
    {
        if (AreSystemDependenciesReady(SystemName))
        {
            InitializeSystem(SystemName);
        }
    }
}

void UEng_SystemsRegistry::ShutdownAllSystems()
{
    if (bLogSystemEvents)
    {
        UE_LOG(LogTemp, Log, TEXT("Engine Architect - Shutting down all systems"));
    }
    
    // Shutdown in reverse priority order
    TArray<FString> SystemNames;
    RegisteredSystems.GetKeys(SystemNames);
    SortSystemsByPriority(SystemNames);
    
    // Reverse the order for shutdown
    for (int32 i = SystemNames.Num() - 1; i >= 0; i--)
    {
        ShutdownSystem(SystemNames[i]);
    }
}

void UEng_SystemsRegistry::RestartSystem(const FString& SystemName)
{
    if (bLogSystemEvents)
    {
        UE_LOG(LogTemp, Log, TEXT("Restarting system: %s"), *SystemName);
    }
    
    ShutdownSystem(SystemName);
    InitializeSystem(SystemName);
}

void UEng_SystemsRegistry::UpdateSystemPerformance(const FString& SystemName, float DeltaTime)
{
    if (!bEnablePerformanceTracking)
    {
        return;
    }
    
    TArray<float>* UpdateTimes = SystemUpdateTimes.Find(SystemName);
    if (!UpdateTimes)
    {
        return;
    }
    
    UpdateTimes->Add(DeltaTime);
    
    // Keep only the last N samples
    if (UpdateTimes->Num() > MaxPerformanceSamples)
    {
        UpdateTimes->RemoveAt(0);
    }
}

float UEng_SystemsRegistry::GetSystemAverageUpdateTime(const FString& SystemName) const
{
    const TArray<float>* UpdateTimes = SystemUpdateTimes.Find(SystemName);
    if (!UpdateTimes || UpdateTimes->Num() == 0)
    {
        return 0.0f;
    }
    
    float Total = 0.0f;
    for (float Time : *UpdateTimes)
    {
        Total += Time;
    }
    
    return Total / UpdateTimes->Num();
}

void UEng_SystemsRegistry::LogSystemStatus() const
{
    UE_LOG(LogTemp, Log, TEXT("=== SYSTEMS REGISTRY STATUS ==="));
    for (const auto& System : RegisteredSystems)
    {
        const FEng_SystemInfo& Info = System.Value;
        UE_LOG(LogTemp, Log, TEXT("System: %s | Status: %d | Priority: %d | Dependencies: %d | Avg Update: %.3fms"),
               *Info.SystemName, (int32)Info.Status, (int32)Info.Priority, 
               Info.Dependencies.Num(), GetSystemAverageUpdateTime(Info.SystemName) * 1000.0f);
    }
    UE_LOG(LogTemp, Log, TEXT("=== END SYSTEMS STATUS ==="));
}

bool UEng_SystemsRegistry::ValidateSystemDependencies() const
{
    for (const auto& System : RegisteredSystems)
    {
        TSet<FString> VisitedSystems;
        if (CheckCircularDependencies(System.Key, VisitedSystems))
        {
            UE_LOG(LogTemp, Error, TEXT("Circular dependency detected for system: %s"), *System.Key);
            return false;
        }
        
        // Check if all dependencies exist
        for (const FString& Dependency : System.Value.Dependencies)
        {
            if (!RegisteredSystems.Contains(Dependency))
            {
                UE_LOG(LogTemp, Error, TEXT("System %s has missing dependency: %s"), *System.Key, *Dependency);
                return false;
            }
        }
    }
    
    return true;
}

TArray<FString> UEng_SystemsRegistry::GetSystemsWithErrors() const
{
    TArray<FString> ErrorSystems;
    for (const auto& System : RegisteredSystems)
    {
        if (System.Value.Status == EEng_SystemStatus::Error)
        {
            ErrorSystems.Add(System.Key);
        }
    }
    return ErrorSystems;
}

void UEng_SystemsRegistry::SortSystemsByPriority(TArray<FString>& SystemNames) const
{
    SystemNames.Sort([this](const FString& A, const FString& B) {
        const FEng_SystemInfo* InfoA = RegisteredSystems.Find(A);
        const FEng_SystemInfo* InfoB = RegisteredSystems.Find(B);
        
        if (!InfoA || !InfoB)
        {
            return false;
        }
        
        return (int32)InfoA->Priority < (int32)InfoB->Priority;
    });
}

bool UEng_SystemsRegistry::CheckCircularDependencies(const FString& SystemName, TSet<FString>& VisitedSystems) const
{
    if (VisitedSystems.Contains(SystemName))
    {
        return true; // Circular dependency found
    }
    
    VisitedSystems.Add(SystemName);
    
    const FEng_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemName);
    if (!SystemInfo)
    {
        return false;
    }
    
    for (const FString& Dependency : SystemInfo->Dependencies)
    {
        if (CheckCircularDependencies(Dependency, VisitedSystems))
        {
            return true;
        }
    }
    
    VisitedSystems.Remove(SystemName);
    return false;
}

void UEng_SystemsRegistry::InitializeSystem(const FString& SystemName)
{
    FEng_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemName);
    if (!SystemInfo)
    {
        return;
    }
    
    float StartTime = FPlatformTime::Seconds();
    SetSystemStatus(SystemName, EEng_SystemStatus::Initializing);
    
    // System-specific initialization would happen here
    // For now, we just mark it as ready
    SetSystemStatus(SystemName, EEng_SystemStatus::Ready);
    
    SystemInfo->InitializationTime = FPlatformTime::Seconds() - StartTime;
    
    if (bLogSystemEvents)
    {
        UE_LOG(LogTemp, Log, TEXT("Initialized system %s in %.3f seconds"), 
               *SystemName, SystemInfo->InitializationTime);
    }
}

void UEng_SystemsRegistry::ShutdownSystem(const FString& SystemName)
{
    FEng_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemName);
    if (!SystemInfo)
    {
        return;
    }
    
    SetSystemStatus(SystemName, EEng_SystemStatus::Shutdown);
    
    if (bLogSystemEvents)
    {
        UE_LOG(LogTemp, Log, TEXT("Shutdown system: %s"), *SystemName);
    }
}