#include "SystemIntegrationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/DateTime.h"

// Define core systems that must be initialized first
const TArray<FString> USystemIntegrationManager::CoreSystemNames = {
    TEXT("EngineArchitectureCore"),
    TEXT("PhysicsSystemManager"),
    TEXT("WorldPartitionManager"),
    TEXT("PerformanceManager")
};

USystemIntegrationManager::USystemIntegrationManager()
{
    bAllSystemsInitialized = false;
    bIntegrationHealthy = true;
}

void USystemIntegrationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("SystemIntegrationManager: Initializing system integration"));
    
    // Clear any previous state
    RegisteredSystems.Empty();
    InitializationOrder.Empty();
    SystemEventListeners.Empty();
    IntegrationErrors.Empty();
    IntegrationWarnings.Empty();
    
    bAllSystemsInitialized = false;
    bIntegrationHealthy = true;
}

void USystemIntegrationManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("SystemIntegrationManager: Shutting down system integration"));
    
    ShutdownAllSystems();
    
    RegisteredSystems.Empty();
    InitializationOrder.Empty();
    SystemEventListeners.Empty();
    IntegrationErrors.Empty();
    IntegrationWarnings.Empty();
    
    Super::Deinitialize();
}

void USystemIntegrationManager::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    
    UE_LOG(LogTemp, Warning, TEXT("SystemIntegrationManager: World begin play - initializing all systems"));
    InitializeAllSystems();
}

void USystemIntegrationManager::RegisterSystem(const FString& SystemName, UObject* SystemInstance, int32 Priority)
{
    if (!SystemInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("SystemIntegrationManager: Cannot register null system: %s"), *SystemName);
        IntegrationErrors.Add(FString::Printf(TEXT("Null system registration: %s"), *SystemName));
        return;
    }

    if (RegisteredSystems.Contains(SystemName))
    {
        UE_LOG(LogTemp, Warning, TEXT("SystemIntegrationManager: System already registered, replacing: %s"), *SystemName);
        IntegrationWarnings.Add(FString::Printf(TEXT("Duplicate system registration: %s"), *SystemName));
    }

    FEng_SystemInfo SystemInfo;
    SystemInfo.SystemInstance = SystemInstance;
    SystemInfo.Priority = Priority;
    SystemInfo.bInitialized = false;
    SystemInfo.bHealthy = true;
    SystemInfo.LastUpdate = FDateTime::Now();

    RegisteredSystems.Add(SystemName, SystemInfo);
    UpdateInitializationOrder();

    UE_LOG(LogTemp, Log, TEXT("SystemIntegrationManager: Registered system: %s (Priority: %d)"), *SystemName, Priority);
}

void USystemIntegrationManager::UnregisterSystem(const FString& SystemName)
{
    if (RegisteredSystems.Remove(SystemName) > 0)
    {
        UpdateInitializationOrder();
        UE_LOG(LogTemp, Log, TEXT("SystemIntegrationManager: Unregistered system: %s"), *SystemName);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("SystemIntegrationManager: System not found for unregistration: %s"), *SystemName);
    }
}

UObject* USystemIntegrationManager::GetSystem(const FString& SystemName) const
{
    if (const FEng_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemName))
    {
        return SystemInfo->SystemInstance;
    }
    return nullptr;
}

bool USystemIntegrationManager::IsSystemRegistered(const FString& SystemName) const
{
    return RegisteredSystems.Contains(SystemName);
}

void USystemIntegrationManager::InitializeAllSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("SystemIntegrationManager: Starting system initialization sequence"));
    
    IntegrationErrors.Empty();
    IntegrationWarnings.Empty();
    
    // Validate dependencies first
    if (!ValidateDependencies())
    {
        UE_LOG(LogTemp, Error, TEXT("SystemIntegrationManager: Dependency validation failed"));
        bIntegrationHealthy = false;
        return;
    }
    
    // Initialize systems in priority order
    for (const FString& SystemName : InitializationOrder)
    {
        InitializeSystemInOrder(SystemName);
    }
    
    bAllSystemsInitialized = true;
    bIntegrationHealthy = IntegrationErrors.Num() == 0;
    
    UE_LOG(LogTemp, Warning, TEXT("SystemIntegrationManager: System initialization complete. Healthy: %s"), 
           bIntegrationHealthy ? TEXT("YES") : TEXT("NO"));
    
    // Broadcast initialization complete event
    BroadcastSystemEvent(TEXT("SystemIntegration"), TEXT("InitializationComplete"));
}

void USystemIntegrationManager::ShutdownAllSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("SystemIntegrationManager: Shutting down all systems"));
    
    // Shutdown in reverse order
    for (int32 i = InitializationOrder.Num() - 1; i >= 0; i--)
    {
        const FString& SystemName = InitializationOrder[i];
        if (FEng_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemName))
        {
            if (SystemInfo->bInitialized)
            {
                UE_LOG(LogTemp, Log, TEXT("SystemIntegrationManager: Shutting down system: %s"), *SystemName);
                SystemInfo->bInitialized = false;
                
                // Notify system of shutdown
                NotifySystemEvent(SystemName, TEXT("Shutdown"), TEXT(""));
            }
        }
    }
    
    bAllSystemsInitialized = false;
}

void USystemIntegrationManager::UpdateAllSystems(float DeltaTime)
{
    if (!bAllSystemsInitialized)
    {
        return;
    }
    
    // Update systems in order
    for (const FString& SystemName : InitializationOrder)
    {
        if (FEng_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemName))
        {
            if (SystemInfo->bInitialized && SystemInfo->bHealthy)
            {
                SystemInfo->LastUpdate = FDateTime::Now();
                
                // Notify system of update
                NotifySystemEvent(SystemName, TEXT("Update"), FString::Printf(TEXT("%.4f"), DeltaTime));
            }
        }
    }
}

void USystemIntegrationManager::AddSystemDependency(const FString& SystemName, const FString& DependencyName)
{
    if (FEng_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemName))
    {
        if (!SystemInfo->Dependencies.Contains(DependencyName))
        {
            SystemInfo->Dependencies.Add(DependencyName);
            UpdateInitializationOrder();
            
            UE_LOG(LogTemp, Log, TEXT("SystemIntegrationManager: Added dependency %s -> %s"), *SystemName, *DependencyName);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("SystemIntegrationManager: Cannot add dependency for unregistered system: %s"), *SystemName);
    }
}

void USystemIntegrationManager::RemoveSystemDependency(const FString& SystemName, const FString& DependencyName)
{
    if (FEng_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemName))
    {
        if (SystemInfo->Dependencies.Remove(DependencyName) > 0)
        {
            UpdateInitializationOrder();
            UE_LOG(LogTemp, Log, TEXT("SystemIntegrationManager: Removed dependency %s -> %s"), *SystemName, *DependencyName);
        }
    }
}

bool USystemIntegrationManager::ValidateDependencies() const
{
    for (const auto& SystemPair : RegisteredSystems)
    {
        const FString& SystemName = SystemPair.Key;
        TArray<FString> VisitedSystems;
        
        if (!ValidateSystemDependencies(SystemName, VisitedSystems))
        {
            UE_LOG(LogTemp, Error, TEXT("SystemIntegrationManager: Circular dependency detected for system: %s"), *SystemName);
            return false;
        }
    }
    
    return true;
}

void USystemIntegrationManager::BroadcastSystemEvent(const FString& EventName, const FString& EventData)
{
    UE_LOG(LogTemp, Log, TEXT("SystemIntegrationManager: Broadcasting event: %s"), *EventName);
    
    // Notify all registered systems
    for (const auto& SystemPair : RegisteredSystems)
    {
        const FString& SystemName = SystemPair.Key;
        NotifySystemEvent(SystemName, EventName, EventData);
    }
}

void USystemIntegrationManager::SendSystemMessage(const FString& TargetSystem, const FString& MessageType, const FString& MessageData)
{
    if (IsSystemRegistered(TargetSystem))
    {
        UE_LOG(LogTemp, Log, TEXT("SystemIntegrationManager: Sending message to %s: %s"), *TargetSystem, *MessageType);
        NotifySystemEvent(TargetSystem, MessageType, MessageData);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("SystemIntegrationManager: Cannot send message to unregistered system: %s"), *TargetSystem);
    }
}

TArray<FString> USystemIntegrationManager::GetRegisteredSystemNames() const
{
    TArray<FString> SystemNames;
    RegisteredSystems.GetKeys(SystemNames);
    return SystemNames;
}

bool USystemIntegrationManager::AreAllSystemsHealthy() const
{
    for (const auto& SystemPair : RegisteredSystems)
    {
        if (!SystemPair.Value.bHealthy)
        {
            return false;
        }
    }
    return bIntegrationHealthy;
}

void USystemIntegrationManager::UpdateInitializationOrder()
{
    InitializationOrder.Empty();
    
    // Add core systems first (in order)
    for (const FString& CoreSystemName : CoreSystemNames)
    {
        if (RegisteredSystems.Contains(CoreSystemName))
        {
            InitializationOrder.Add(CoreSystemName);
        }
    }
    
    // Add remaining systems sorted by priority
    TArray<TPair<FString, int32>> SystemPriorities;
    for (const auto& SystemPair : RegisteredSystems)
    {
        const FString& SystemName = SystemPair.Key;
        if (!CoreSystemNames.Contains(SystemName))
        {
            SystemPriorities.Add(TPair<FString, int32>(SystemName, SystemPair.Value.Priority));
        }
    }
    
    // Sort by priority (higher priority first)
    SystemPriorities.Sort([](const TPair<FString, int32>& A, const TPair<FString, int32>& B) {
        return A.Value > B.Value;
    });
    
    // Add to initialization order
    for (const auto& SystemPriority : SystemPriorities)
    {
        InitializationOrder.Add(SystemPriority.Key);
    }
    
    UE_LOG(LogTemp, Log, TEXT("SystemIntegrationManager: Updated initialization order (%d systems)"), InitializationOrder.Num());
}

bool USystemIntegrationManager::ValidateSystemDependencies(const FString& SystemName, TArray<FString>& VisitedSystems) const
{
    if (VisitedSystems.Contains(SystemName))
    {
        return false; // Circular dependency detected
    }
    
    VisitedSystems.Add(SystemName);
    
    if (const FEng_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemName))
    {
        for (const FString& Dependency : SystemInfo->Dependencies)
        {
            if (!ValidateSystemDependencies(Dependency, VisitedSystems))
            {
                return false;
            }
        }
    }
    
    VisitedSystems.Remove(SystemName);
    return true;
}

void USystemIntegrationManager::InitializeSystemInOrder(const FString& SystemName)
{
    FEng_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemName);
    if (!SystemInfo || SystemInfo->bInitialized)
    {
        return;
    }
    
    // Initialize dependencies first
    for (const FString& Dependency : SystemInfo->Dependencies)
    {
        if (RegisteredSystems.Contains(Dependency))
        {
            InitializeSystemInOrder(Dependency);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("SystemIntegrationManager: Missing dependency %s for system %s"), *Dependency, *SystemName);
            IntegrationErrors.Add(FString::Printf(TEXT("Missing dependency: %s -> %s"), *SystemName, *Dependency));
            SystemInfo->bHealthy = false;
            return;
        }
    }
    
    // Initialize the system
    UE_LOG(LogTemp, Log, TEXT("SystemIntegrationManager: Initializing system: %s"), *SystemName);
    SystemInfo->bInitialized = true;
    
    // Notify system of initialization
    NotifySystemEvent(SystemName, TEXT("Initialize"), TEXT(""));
}

void USystemIntegrationManager::NotifySystemEvent(const FString& SystemName, const FString& EventName, const FString& EventData)
{
    // In a full implementation, this would use reflection or interfaces to call system methods
    // For now, we just log the event
    UE_LOG(LogTemp, VeryVerbose, TEXT("SystemIntegrationManager: Event %s -> %s: %s"), *SystemName, *EventName, *EventData);
}