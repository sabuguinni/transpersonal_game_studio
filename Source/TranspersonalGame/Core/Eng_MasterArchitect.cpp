#include "Eng_MasterArchitect.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Misc/DateTime.h"
#include "HAL/PlatformFilemanager.h"

UEng_MasterArchitect::UEng_MasterArchitect()
{
    bEnforceStrictDependencies = true;
    bShutdownOnCriticalFailure = true;
    MaxInitializationTime = 30.0f; // 30 seconds max per system
    CurrentMetrics = FEng_ArchitectureMetrics();
}

void UEng_MasterArchitect::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("=== ENGINE ARCHITECT MASTER SYSTEM INITIALIZING ==="));
    
    // Initialize core architecture metrics
    CurrentMetrics = FEng_ArchitectureMetrics();
    CurrentMetrics.LastUpdate = FDateTime::Now();
    
    // Register critical core systems in proper order
    RegisterCriticalSystems();
    
    // Build initialization order based on dependencies
    BuildInitializationOrder();
    
    UE_LOG(LogTemp, Warning, TEXT("Master Architect initialized with %d systems"), RegisteredSystems.Num());
}

void UEng_MasterArchitect::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("=== ENGINE ARCHITECT MASTER SYSTEM SHUTTING DOWN ==="));
    
    // Shutdown all systems in reverse order
    for (int32 i = InitializationOrder.Num() - 1; i >= 0; i--)
    {
        const FString& SystemName = InitializationOrder[i];
        ShutdownSystem(SystemName);
    }
    
    RegisteredSystems.Empty();
    SystemDependencies.Empty();
    InitializationOrder.Empty();
    
    Super::Deinitialize();
}

bool UEng_MasterArchitect::RegisterSystem(const FString& SystemName, EEng_SystemPriority Priority, const TArray<FString>& Dependencies)
{
    if (SystemName.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot register system with empty name"));
        return false;
    }
    
    if (RegisteredSystems.Contains(SystemName))
    {
        UE_LOG(LogTemp, Warning, TEXT("System %s already registered, updating..."), *SystemName);
    }
    
    // Create system info
    FEng_SystemInfo SystemInfo;
    SystemInfo.SystemName = SystemName;
    SystemInfo.CurrentState = EEng_SystemState::Uninitialized;
    SystemInfo.Priority = Priority;
    SystemInfo.InitializationTime = 0.0f;
    SystemInfo.LastError = TEXT("");
    SystemInfo.DependencyCount = Dependencies.Num();
    
    // Register system
    RegisteredSystems.Add(SystemName, SystemInfo);
    SystemDependencies.Add(SystemName, Dependencies);
    
    // Rebuild initialization order
    BuildInitializationOrder();
    
    LogSystemEvent(SystemName, TEXT("Registered"));
    UpdateMetrics();
    
    return true;
}

bool UEng_MasterArchitect::InitializeSystem(const FString& SystemName)
{
    if (!RegisteredSystems.Contains(SystemName))
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot initialize unregistered system: %s"), *SystemName);
        return false;
    }
    
    FEng_SystemInfo& SystemInfo = RegisteredSystems[SystemName];
    
    // Check if already initialized
    if (SystemInfo.CurrentState == EEng_SystemState::Operational)
    {
        UE_LOG(LogTemp, Warning, TEXT("System %s already operational"), *SystemName);
        return true;
    }
    
    // Check dependencies
    if (bEnforceStrictDependencies && !CheckDependenciesResolved(SystemName))
    {
        SystemInfo.CurrentState = EEng_SystemState::Error;
        SystemInfo.LastError = TEXT("Dependencies not resolved");
        LogSystemEvent(SystemName, TEXT("Failed - Dependencies not resolved"));
        return false;
    }
    
    // Begin initialization
    SystemInfo.CurrentState = EEng_SystemState::Initializing;
    float StartTime = FPlatformTime::Seconds();
    
    LogSystemEvent(SystemName, TEXT("Initializing"));
    OnSystemStateChanged.Broadcast(SystemName, EEng_SystemState::Initializing);
    
    // Simulate system initialization (in real implementation, this would call actual system init)
    bool bInitSuccess = true;
    
    // Special handling for critical systems
    if (IsSystemCritical(SystemName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Initializing critical system: %s"), *SystemName);
    }
    
    // Complete initialization
    float InitTime = FPlatformTime::Seconds() - StartTime;
    SystemInfo.InitializationTime = InitTime;
    
    if (bInitSuccess && InitTime < MaxInitializationTime)
    {
        SystemInfo.CurrentState = EEng_SystemState::Operational;
        SystemInfo.LastError = TEXT("");
        LogSystemEvent(SystemName, FString::Printf(TEXT("Initialized successfully in %.2fs"), InitTime));
        OnSystemStateChanged.Broadcast(SystemName, EEng_SystemState::Operational);
    }
    else
    {
        SystemInfo.CurrentState = EEng_SystemState::Error;
        SystemInfo.LastError = InitTime >= MaxInitializationTime ? TEXT("Initialization timeout") : TEXT("Initialization failed");
        LogSystemEvent(SystemName, FString::Printf(TEXT("Failed - %s"), *SystemInfo.LastError));
        OnSystemStateChanged.Broadcast(SystemName, EEng_SystemState::Error);
        
        // Handle critical system failure
        if (IsSystemCritical(SystemName) && bShutdownOnCriticalFailure)
        {
            OnCriticalSystemFailure.Broadcast(SystemName, SystemInfo.LastError);
            UE_LOG(LogTemp, Error, TEXT("Critical system failure: %s - %s"), *SystemName, *SystemInfo.LastError);
        }
        
        bInitSuccess = false;
    }
    
    UpdateMetrics();
    return bInitSuccess;
}

bool UEng_MasterArchitect::ShutdownSystem(const FString& SystemName)
{
    if (!RegisteredSystems.Contains(SystemName))
    {
        return false;
    }
    
    FEng_SystemInfo& SystemInfo = RegisteredSystems[SystemName];
    SystemInfo.CurrentState = EEng_SystemState::Shutdown;
    
    LogSystemEvent(SystemName, TEXT("Shutdown"));
    OnSystemStateChanged.Broadcast(SystemName, EEng_SystemState::Shutdown);
    
    UpdateMetrics();
    return true;
}

EEng_SystemState UEng_MasterArchitect::GetSystemState(const FString& SystemName) const
{
    if (const FEng_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemName))
    {
        return SystemInfo->CurrentState;
    }
    return EEng_SystemState::Uninitialized;
}

bool UEng_MasterArchitect::ValidateArchitecture()
{
    TArray<FString> ValidationErrors;
    
    // Check for circular dependencies
    for (const auto& SystemPair : SystemDependencies)
    {
        const FString& SystemName = SystemPair.Key;
        const TArray<FString>& Dependencies = SystemPair.Value;
        
        // Simple circular dependency check (could be more sophisticated)
        for (const FString& Dependency : Dependencies)
        {
            if (const TArray<FString>* DepDependencies = SystemDependencies.Find(Dependency))
            {
                if (DepDependencies->Contains(SystemName))
                {
                    ValidationErrors.Add(FString::Printf(TEXT("Circular dependency: %s <-> %s"), *SystemName, *Dependency));
                }
            }
        }
    }
    
    // Check for missing dependencies
    for (const auto& SystemPair : SystemDependencies)
    {
        const FString& SystemName = SystemPair.Key;
        const TArray<FString>& Dependencies = SystemPair.Value;
        
        for (const FString& Dependency : Dependencies)
        {
            if (!RegisteredSystems.Contains(Dependency))
            {
                ValidationErrors.Add(FString::Printf(TEXT("Missing dependency: %s requires %s"), *SystemName, *Dependency));
            }
        }
    }
    
    if (ValidationErrors.Num() > 0)
    {
        OnArchitectureValidationFailed.Broadcast(ValidationErrors);
        for (const FString& Error : ValidationErrors)
        {
            UE_LOG(LogTemp, Error, TEXT("Architecture Validation Error: %s"), *Error);
        }
        return false;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Architecture validation passed"));
    return true;
}

bool UEng_MasterArchitect::ValidateSystemDependencies()
{
    return ValidateArchitecture();
}

TArray<FString> UEng_MasterArchitect::GetSystemErrors() const
{
    TArray<FString> Errors;
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        const FEng_SystemInfo& SystemInfo = SystemPair.Value;
        if (SystemInfo.CurrentState == EEng_SystemState::Error && !SystemInfo.LastError.IsEmpty())
        {
            Errors.Add(FString::Printf(TEXT("%s: %s"), *SystemInfo.SystemName, *SystemInfo.LastError));
        }
    }
    
    return Errors;
}

FEng_ArchitectureMetrics UEng_MasterArchitect::GetArchitectureMetrics() const
{
    return CurrentMetrics;
}

TArray<FEng_SystemInfo> UEng_MasterArchitect::GetAllSystemInfo() const
{
    TArray<FEng_SystemInfo> SystemInfoArray;
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        SystemInfoArray.Add(SystemPair.Value);
    }
    
    return SystemInfoArray;
}

bool UEng_MasterArchitect::EnforceInitializationOrder()
{
    UE_LOG(LogTemp, Warning, TEXT("Enforcing initialization order for %d systems"), InitializationOrder.Num());
    
    for (const FString& SystemName : InitializationOrder)
    {
        if (!InitializeSystem(SystemName))
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to initialize system in order: %s"), *SystemName);
            return false;
        }
    }
    
    return true;
}

void UEng_MasterArchitect::SetCriticalSystemFailureResponse(bool bShutdownOnCriticalFailure)
{
    this->bShutdownOnCriticalFailure = bShutdownOnCriticalFailure;
    UE_LOG(LogTemp, Log, TEXT("Critical system failure response set to: %s"), 
           bShutdownOnCriticalFailure ? TEXT("Shutdown") : TEXT("Continue"));
}

void UEng_MasterArchitect::BuildInitializationOrder()
{
    InitializationOrder.Empty();
    
    // Simple dependency-based ordering (topological sort would be better)
    TArray<FString> ProcessedSystems;
    TArray<FString> RemainingSystemsNames;
    
    RegisteredSystems.GetKeys(RemainingSystemsNames);
    
    // Add systems with no dependencies first
    for (const FString& SystemName : RemainingSystemsNames)
    {
        const TArray<FString>& Dependencies = SystemDependencies.FindRef(SystemName);
        if (Dependencies.Num() == 0)
        {
            InitializationOrder.Add(SystemName);
            ProcessedSystems.Add(SystemName);
        }
    }
    
    // Add remaining systems based on dependency resolution
    while (ProcessedSystems.Num() < RemainingSystemsNames.Num())
    {
        bool bAddedAny = false;
        
        for (const FString& SystemName : RemainingSystemsNames)
        {
            if (ProcessedSystems.Contains(SystemName))
                continue;
                
            const TArray<FString>& Dependencies = SystemDependencies.FindRef(SystemName);
            bool bAllDependenciesProcessed = true;
            
            for (const FString& Dependency : Dependencies)
            {
                if (!ProcessedSystems.Contains(Dependency))
                {
                    bAllDependenciesProcessed = false;
                    break;
                }
            }
            
            if (bAllDependenciesProcessed)
            {
                InitializationOrder.Add(SystemName);
                ProcessedSystems.Add(SystemName);
                bAddedAny = true;
            }
        }
        
        if (!bAddedAny)
        {
            UE_LOG(LogTemp, Error, TEXT("Circular dependency detected in system initialization order"));
            break;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Built initialization order with %d systems"), InitializationOrder.Num());
}

bool UEng_MasterArchitect::CheckDependenciesResolved(const FString& SystemName) const
{
    const TArray<FString>& Dependencies = SystemDependencies.FindRef(SystemName);
    
    for (const FString& Dependency : Dependencies)
    {
        if (const FEng_SystemInfo* DepInfo = RegisteredSystems.Find(Dependency))
        {
            if (DepInfo->CurrentState != EEng_SystemState::Operational)
            {
                return false;
            }
        }
        else
        {
            return false; // Dependency not even registered
        }
    }
    
    return true;
}

void UEng_MasterArchitect::UpdateMetrics()
{
    CurrentMetrics.TotalSystems = RegisteredSystems.Num();
    CurrentMetrics.OperationalSystems = 0;
    CurrentMetrics.ErrorSystems = 0;
    
    float TotalInitTime = 0.0f;
    int32 InitializedCount = 0;
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        const FEng_SystemInfo& SystemInfo = SystemPair.Value;
        
        switch (SystemInfo.CurrentState)
        {
            case EEng_SystemState::Operational:
                CurrentMetrics.OperationalSystems++;
                TotalInitTime += SystemInfo.InitializationTime;
                InitializedCount++;
                break;
            case EEng_SystemState::Error:
                CurrentMetrics.ErrorSystems++;
                break;
        }
    }
    
    CurrentMetrics.OverallHealth = CurrentMetrics.TotalSystems > 0 ? 
        (float)CurrentMetrics.OperationalSystems / CurrentMetrics.TotalSystems * 100.0f : 0.0f;
    
    CurrentMetrics.AverageInitTime = InitializedCount > 0 ? TotalInitTime / InitializedCount : 0.0f;
    CurrentMetrics.LastUpdate = FDateTime::Now();
}

void UEng_MasterArchitect::LogSystemEvent(const FString& SystemName, const FString& Event) const
{
    UE_LOG(LogTemp, Log, TEXT("[ARCHITECT] %s: %s"), *SystemName, *Event);
}

bool UEng_MasterArchitect::IsSystemCritical(const FString& SystemName) const
{
    if (const FEng_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemName))
    {
        return SystemInfo->Priority == EEng_SystemPriority::Critical;
    }
    return false;
}

void UEng_MasterArchitect::RegisterCriticalSystems()
{
    // Register core critical systems that all other systems depend on
    RegisterSystem(TEXT("PhysicsCore"), EEng_SystemPriority::Critical, {});
    RegisterSystem(TEXT("WorldManager"), EEng_SystemPriority::Critical, {TEXT("PhysicsCore")});
    RegisterSystem(TEXT("CharacterSystem"), EEng_SystemPriority::High, {TEXT("PhysicsCore"), TEXT("WorldManager")});
    RegisterSystem(TEXT("BiomeManager"), EEng_SystemPriority::High, {TEXT("WorldManager")});
    RegisterSystem(TEXT("DinosaurAI"), EEng_SystemPriority::Medium, {TEXT("CharacterSystem"), TEXT("BiomeManager")});
    RegisterSystem(TEXT("AudioSystem"), EEng_SystemPriority::Medium, {TEXT("WorldManager")});
    RegisterSystem(TEXT("VFXSystem"), EEng_SystemPriority::Low, {TEXT("WorldManager")});
    
    UE_LOG(LogTemp, Warning, TEXT("Registered %d critical systems"), RegisteredSystems.Num());
}

// World Architecture Coordinator Implementation

UEng_WorldArchitectureCoordinator::UEng_WorldArchitectureCoordinator()
{
    bWorldSystemsInitialized = false;
    WorldInitializationTime = 0.0f;
}

void UEng_WorldArchitectureCoordinator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("=== WORLD ARCHITECTURE COORDINATOR INITIALIZING ==="));
    
    // Get reference to master architect
    UEng_MasterArchitect* MasterArchitect = GetMasterArchitect();
    if (MasterArchitect)
    {
        UE_LOG(LogTemp, Log, TEXT("Connected to Master Architect"));
        InitializeWorldSystems();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to connect to Master Architect"));
    }
}

void UEng_WorldArchitectureCoordinator::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("=== WORLD ARCHITECTURE COORDINATOR SHUTTING DOWN ==="));
    
    ShutdownWorldSystems();
    WorldSystems.Empty();
    
    Super::Deinitialize();
}

bool UEng_WorldArchitectureCoordinator::InitializeWorldSystems()
{
    if (bWorldSystemsInitialized)
    {
        return true;
    }
    
    float StartTime = FPlatformTime::Seconds();
    
    UE_LOG(LogTemp, Warning, TEXT("Initializing world-specific systems..."));
    
    // Initialize world systems in proper order
    InitializePhysicsSystems();
    InitializeWorldGenerationSystems();
    InitializeCharacterSystems();
    InitializeAISystems();
    InitializeAudioSystems();
    
    WorldInitializationTime = FPlatformTime::Seconds() - StartTime;
    bWorldSystemsInitialized = true;
    
    UE_LOG(LogTemp, Warning, TEXT("World systems initialized in %.2fs"), WorldInitializationTime);
    return true;
}

bool UEng_WorldArchitectureCoordinator::ShutdownWorldSystems()
{
    if (!bWorldSystemsInitialized)
    {
        return true;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Shutting down world systems..."));
    
    // Shutdown in reverse order
    for (auto& SystemPair : WorldSystems)
    {
        UE_LOG(LogTemp, Log, TEXT("Shutting down world system: %s"), *SystemPair.Key);
    }
    
    WorldSystems.Empty();
    bWorldSystemsInitialized = false;
    
    return true;
}

bool UEng_WorldArchitectureCoordinator::ValidateWorldArchitecture()
{
    UEng_MasterArchitect* MasterArchitect = GetMasterArchitect();
    if (!MasterArchitect)
    {
        return false;
    }
    
    return MasterArchitect->ValidateArchitecture();
}

UEng_MasterArchitect* UEng_WorldArchitectureCoordinator::GetMasterArchitect() const
{
    if (UGameInstance* GameInstance = GetWorld()->GetGameInstance())
    {
        return GameInstance->GetSubsystem<UEng_MasterArchitect>();
    }
    return nullptr;
}

bool UEng_WorldArchitectureCoordinator::RegisterWorldSystem(const FString& SystemName, UObject* SystemInstance)
{
    if (!SystemInstance)
    {
        return false;
    }
    
    WorldSystems.Add(SystemName, SystemInstance);
    UE_LOG(LogTemp, Log, TEXT("Registered world system: %s"), *SystemName);
    
    return true;
}

UObject* UEng_WorldArchitectureCoordinator::GetWorldSystem(const FString& SystemName) const
{
    if (TObjectPtr<UObject> const* SystemPtr = WorldSystems.Find(SystemName))
    {
        return SystemPtr->Get();
    }
    return nullptr;
}

void UEng_WorldArchitectureCoordinator::InitializePhysicsSystems()
{
    UE_LOG(LogTemp, Log, TEXT("Initializing physics systems for world"));
    // Physics system initialization would go here
}

void UEng_WorldArchitectureCoordinator::InitializeWorldGenerationSystems()
{
    UE_LOG(LogTemp, Log, TEXT("Initializing world generation systems"));
    // World generation system initialization would go here
}

void UEng_WorldArchitectureCoordinator::InitializeCharacterSystems()
{
    UE_LOG(LogTemp, Log, TEXT("Initializing character systems"));
    // Character system initialization would go here
}

void UEng_WorldArchitectureCoordinator::InitializeAISystems()
{
    UE_LOG(LogTemp, Log, TEXT("Initializing AI systems"));
    // AI system initialization would go here
}

void UEng_WorldArchitectureCoordinator::InitializeAudioSystems()
{
    UE_LOG(LogTemp, Log, TEXT("Initializing audio systems"));
    // Audio system initialization would go here
}