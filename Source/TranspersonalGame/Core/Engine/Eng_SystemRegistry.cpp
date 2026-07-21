#include "Eng_SystemRegistry.h"
#include "Engine/World.h"
#include "TimerManager.h"

UEng_SystemRegistry::UEng_SystemRegistry()
{
    bIsInitialized = false;
    TotalInitTime = 0.0f;
}

void UEng_SystemRegistry::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    bIsInitialized = true;
    TotalInitTime = 0.0f;
    RegisteredSystems.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("SystemRegistry: Initialized"));
    
    // Register core engine systems
    RegisterSystem(TEXT("CoreSystems"), EEng_SystemType::Core, 100);
    RegisterSystem(TEXT("PhysicsCore"), EEng_SystemType::Physics, 90);
    RegisterSystem(TEXT("RenderingCore"), EEng_SystemType::Rendering, 80);
    RegisterSystem(TEXT("AudioCore"), EEng_SystemType::Audio, 70);
    
    SetSystemStatus(TEXT("CoreSystems"), EEng_SystemStatus::Running);
}

void UEng_SystemRegistry::Deinitialize()
{
    ShutdownAllSystems();
    RegisteredSystems.Empty();
    bIsInitialized = false;
    
    UE_LOG(LogTemp, Log, TEXT("SystemRegistry: Deinitialized"));
    Super::Deinitialize();
}

void UEng_SystemRegistry::RegisterSystem(const FString& SystemName, EEng_SystemType SystemType, int32 Priority)
{
    if (!bIsInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("SystemRegistry: Cannot register system %s - registry not initialized"), *SystemName);
        return;
    }
    
    if (RegisteredSystems.Contains(SystemName))
    {
        UE_LOG(LogTemp, Warning, TEXT("SystemRegistry: System %s already registered"), *SystemName);
        return;
    }
    
    FEng_SystemInfo NewSystem;
    NewSystem.SystemName = SystemName;
    NewSystem.SystemType = SystemType;
    NewSystem.Status = EEng_SystemStatus::Uninitialized;
    NewSystem.InitializationTime = 0.0f;
    NewSystem.LastUpdateTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    NewSystem.Priority = Priority;
    
    RegisteredSystems.Add(SystemName, NewSystem);
    
    UE_LOG(LogTemp, Log, TEXT("SystemRegistry: Registered system %s (Type: %d, Priority: %d)"), 
           *SystemName, (int32)SystemType, Priority);
}

void UEng_SystemRegistry::UnregisterSystem(const FString& SystemName)
{
    if (RegisteredSystems.Contains(SystemName))
    {
        SetSystemStatus(SystemName, EEng_SystemStatus::Shutdown);
        RegisteredSystems.Remove(SystemName);
        UE_LOG(LogTemp, Log, TEXT("SystemRegistry: Unregistered system %s"), *SystemName);
    }
}

void UEng_SystemRegistry::SetSystemStatus(const FString& SystemName, EEng_SystemStatus NewStatus)
{
    if (FEng_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemName))
    {
        EEng_SystemStatus OldStatus = SystemInfo->Status;
        SystemInfo->Status = NewStatus;
        UpdateSystemTime(SystemName);
        
        // Track initialization time
        if (OldStatus == EEng_SystemStatus::Initializing && NewStatus == EEng_SystemStatus::Running)
        {
            float InitTime = GetWorld() ? GetWorld()->GetTimeSeconds() - SystemInfo->LastUpdateTime : 0.0f;
            SystemInfo->InitializationTime = InitTime;
            TotalInitTime += InitTime;
        }
        
        UE_LOG(LogTemp, Log, TEXT("SystemRegistry: System %s status changed from %d to %d"), 
               *SystemName, (int32)OldStatus, (int32)NewStatus);
    }
}

EEng_SystemStatus UEng_SystemRegistry::GetSystemStatus(const FString& SystemName) const
{
    if (const FEng_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemName))
    {
        return SystemInfo->Status;
    }
    return EEng_SystemStatus::Uninitialized;
}

TArray<FEng_SystemInfo> UEng_SystemRegistry::GetAllSystems() const
{
    TArray<FEng_SystemInfo> Systems;
    for (const auto& SystemPair : RegisteredSystems)
    {
        Systems.Add(SystemPair.Value);
    }
    
    // Sort by priority
    Systems.Sort([](const FEng_SystemInfo& A, const FEng_SystemInfo& B)
    {
        return A.Priority > B.Priority;
    });
    
    return Systems;
}

TArray<FEng_SystemInfo> UEng_SystemRegistry::GetSystemsByType(EEng_SystemType SystemType) const
{
    TArray<FEng_SystemInfo> FilteredSystems;
    for (const auto& SystemPair : RegisteredSystems)
    {
        if (SystemPair.Value.SystemType == SystemType)
        {
            FilteredSystems.Add(SystemPair.Value);
        }
    }
    return FilteredSystems;
}

bool UEng_SystemRegistry::IsSystemRunning(const FString& SystemName) const
{
    return GetSystemStatus(SystemName) == EEng_SystemStatus::Running;
}

void UEng_SystemRegistry::InitializeAllSystems()
{
    TArray<FEng_SystemInfo> Systems = GetAllSystems();
    
    for (const FEng_SystemInfo& System : Systems)
    {
        if (System.Status == EEng_SystemStatus::Uninitialized)
        {
            SetSystemStatus(System.SystemName, EEng_SystemStatus::Initializing);
            // In a real implementation, this would trigger actual system initialization
            SetSystemStatus(System.SystemName, EEng_SystemStatus::Running);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("SystemRegistry: Initialized %d systems"), Systems.Num());
}

void UEng_SystemRegistry::ShutdownAllSystems()
{
    for (auto& SystemPair : RegisteredSystems)
    {
        if (SystemPair.Value.Status == EEng_SystemStatus::Running)
        {
            SetSystemStatus(SystemPair.Key, EEng_SystemStatus::Shutdown);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("SystemRegistry: Shutdown all systems"));
}

int32 UEng_SystemRegistry::GetSystemCount() const
{
    return RegisteredSystems.Num();
}

float UEng_SystemRegistry::GetTotalInitializationTime() const
{
    return TotalInitTime;
}

void UEng_SystemRegistry::UpdateSystemTime(const FString& SystemName)
{
    if (FEng_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemName))
    {
        SystemInfo->LastUpdateTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    }
}

void UEng_SystemRegistry::SortSystemsByPriority()
{
    // This would be used for ordered initialization/shutdown
    // Implementation depends on specific requirements
}