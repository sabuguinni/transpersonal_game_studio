// Copyright Transpersonal Game Studio. All Rights Reserved.
// SystemIntegrationManager.cpp - Implementation of system integration management

#include "SystemIntegrationManager.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "TimerManager.h"
#include "Misc/DateTime.h"
#include "HAL/PlatformFilemanager.h"

DEFINE_LOG_CATEGORY(LogSystemIntegration);

// Agent chain definition (19 agents in order)
const TArray<FString> USystemIntegrationManager::AgentChain = {
    TEXT("01_StudioDirector"),
    TEXT("02_EngineArchitect"),
    TEXT("03_CoreSystemsProgrammer"),
    TEXT("04_PerformanceOptimizer"),
    TEXT("05_ProceduralWorldGenerator"),
    TEXT("06_EnvironmentArtist"),
    TEXT("07_ArchitectureInterior"),
    TEXT("08_LightingAtmosphere"),
    TEXT("09_CharacterArtist"),
    TEXT("10_AnimationAgent"),
    TEXT("11_NPCBehavior"),
    TEXT("12_CombatEnemyAI"),
    TEXT("13_CrowdTrafficSimulation"),
    TEXT("14_QuestMissionDesigner"),
    TEXT("15_NarrativeDialogue"),
    TEXT("16_AudioAgent"),
    TEXT("17_VFXAgent"),
    TEXT("18_QATesting"),
    TEXT("19_IntegrationBuild")
};

void USystemIntegrationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogSystemIntegration, Log, TEXT("SystemIntegrationManager: Initializing..."));
    
    SystemInitializationStartTime = FPlatformTime::Seconds();
    bIntegrationInitialized = false;
    bAllSystemsInitialized = false;
    
    // Initialize architecture validator
    ArchitectureValidator = NewObject<UEngineArchitectureValidator>(this);
    if (ArchitectureValidator)
    {
        ArchitectureValidator->Initialize();
        UE_LOG(LogSystemIntegration, Log, TEXT("SystemIntegrationManager: Architecture validator initialized"));
    }
    
    // Register core systems first
    RegisterCoreAgentSystems();
    
    // Validate agent chain
    if (ValidateAgentChain())
    {
        UE_LOG(LogSystemIntegration, Log, TEXT("SystemIntegrationManager: Agent chain validation successful"));
    }
    else
    {
        UE_LOG(LogSystemIntegration, Warning, TEXT("SystemIntegrationManager: Agent chain validation failed"));
    }
    
    bIntegrationInitialized = true;
    UE_LOG(LogSystemIntegration, Log, TEXT("SystemIntegrationManager: Initialization complete"));
}

void USystemIntegrationManager::Deinitialize()
{
    UE_LOG(LogSystemIntegration, Log, TEXT("SystemIntegrationManager: Deinitializing..."));
    
    // Clear all registered systems
    RegisteredSystems.Empty();
    SystemNameToIndex.Empty();
    InitializationOrder.Empty();
    SystemInitializationTimes.Empty();
    
    // Clean up architecture validator
    if (ArchitectureValidator)
    {
        ArchitectureValidator->Cleanup();
        ArchitectureValidator = nullptr;
    }
    
    bIntegrationInitialized = false;
    bAllSystemsInitialized = false;
    
    Super::Deinitialize();
    UE_LOG(LogSystemIntegration, Log, TEXT("SystemIntegrationManager: Deinitialization complete"));
}

bool USystemIntegrationManager::RegisterAgentSystem(const FAgentSystemInfo& SystemInfo)
{
    if (SystemInfo.SystemName.IsEmpty())
    {
        UE_LOG(LogSystemIntegration, Error, TEXT("RegisterAgentSystem: Cannot register system with empty name"));
        return false;
    }
    
    // Check if system is already registered
    if (SystemNameToIndex.Contains(SystemInfo.SystemName))
    {
        UE_LOG(LogSystemIntegration, Warning, TEXT("RegisterAgentSystem: System %s is already registered"), *SystemInfo.SystemName);
        return false;
    }
    
    // Validate system requirements
    if (!ValidateSystemRequirements(SystemInfo))
    {
        UE_LOG(LogSystemIntegration, Error, TEXT("RegisterAgentSystem: System %s failed requirements validation"), *SystemInfo.SystemName);
        return false;
    }
    
    // Add to registry
    int32 NewIndex = RegisteredSystems.Add(SystemInfo);
    SystemNameToIndex.Add(SystemInfo.SystemName, NewIndex);
    
    UE_LOG(LogSystemIntegration, Log, TEXT("RegisterAgentSystem: Successfully registered %s (Agent: %s)"), 
           *SystemInfo.SystemName, *SystemInfo.AgentName);
    
    // Broadcast registration event
    OnSystemRegistered.Broadcast(SystemInfo.SystemName);
    
    return true;
}

bool USystemIntegrationManager::UnregisterAgentSystem(const FString& SystemName)
{
    if (!SystemNameToIndex.Contains(SystemName))
    {
        UE_LOG(LogSystemIntegration, Warning, TEXT("UnregisterAgentSystem: System %s not found"), *SystemName);
        return false;
    }
    
    int32 SystemIndex = SystemNameToIndex[SystemName];
    
    // Remove from arrays
    RegisteredSystems.RemoveAt(SystemIndex);
    SystemNameToIndex.Remove(SystemName);
    
    // Rebuild index map
    SystemNameToIndex.Empty();
    for (int32 i = 0; i < RegisteredSystems.Num(); i++)
    {
        SystemNameToIndex.Add(RegisteredSystems[i].SystemName, i);
    }
    
    // Remove from initialization tracking
    SystemInitializationTimes.Remove(SystemName);
    InitializationOrder.Remove(SystemName);
    
    UE_LOG(LogSystemIntegration, Log, TEXT("UnregisterAgentSystem: Successfully unregistered %s"), *SystemName);
    return true;
}

FAgentSystemInfo USystemIntegrationManager::GetSystemInfo(const FString& SystemName) const
{
    if (SystemNameToIndex.Contains(SystemName))
    {
        int32 Index = SystemNameToIndex[SystemName];
        return RegisteredSystems[Index];
    }
    
    return FAgentSystemInfo(); // Return empty info if not found
}

bool USystemIntegrationManager::InitializeAllSystems()
{
    UE_LOG(LogSystemIntegration, Log, TEXT("InitializeAllSystems: Starting system initialization..."));
    
    if (!bIntegrationInitialized)
    {
        UE_LOG(LogSystemIntegration, Error, TEXT("InitializeAllSystems: Integration manager not initialized"));
        return false;
    }
    
    // Validate dependencies first
    FSystemDependencyResult DependencyResult = ValidateSystemDependencies();
    if (!DependencyResult.bAllDependenciesMet)
    {
        UE_LOG(LogSystemIntegration, Error, TEXT("InitializeAllSystems: Dependency validation failed"));
        for (const FString& Missing : DependencyResult.MissingDependencies)
        {
            UE_LOG(LogSystemIntegration, Error, TEXT("  Missing dependency: %s"), *Missing);
        }
        return false;
    }
    
    // Get initialization order
    TArray<FString> InitOrder = GetInitializationOrder();
    if (InitOrder.Num() == 0)
    {
        UE_LOG(LogSystemIntegration, Warning, TEXT("InitializeAllSystems: No systems to initialize"));
        return true;
    }
    
    // Initialize systems in order
    int32 SuccessfulInitializations = 0;
    for (const FString& SystemName : InitOrder)
    {
        if (InitializeSystemByName(SystemName))
        {
            SuccessfulInitializations++;
        }
        else
        {
            UE_LOG(LogSystemIntegration, Error, TEXT("InitializeAllSystems: Failed to initialize %s"), *SystemName);
        }
    }
    
    // Update status
    UpdateSystemStatus();
    
    bool bSuccess = (SuccessfulInitializations == InitOrder.Num());
    if (bSuccess)
    {
        bAllSystemsInitialized = true;
        OnAllSystemsReady.Broadcast();
        UE_LOG(LogSystemIntegration, Log, TEXT("InitializeAllSystems: All %d systems initialized successfully"), SuccessfulInitializations);
    }
    else
    {
        UE_LOG(LogSystemIntegration, Warning, TEXT("InitializeAllSystems: %d/%d systems initialized"), 
               SuccessfulInitializations, InitOrder.Num());
    }
    
    return bSuccess;
}

bool USystemIntegrationManager::InitializeSystemByName(const FString& SystemName)
{
    if (!SystemNameToIndex.Contains(SystemName))
    {
        UE_LOG(LogSystemIntegration, Error, TEXT("InitializeSystemByName: System %s not registered"), *SystemName);
        return false;
    }
    
    int32 SystemIndex = SystemNameToIndex[SystemName];
    FAgentSystemInfo& SystemInfo = RegisteredSystems[SystemIndex];
    
    if (SystemInfo.bIsInitialized)
    {
        UE_LOG(LogSystemIntegration, Warning, TEXT("InitializeSystemByName: System %s already initialized"), *SystemName);
        return true;
    }
    
    // Check if dependencies are met
    if (!CanInitializeSystem(SystemInfo))
    {
        UE_LOG(LogSystemIntegration, Error, TEXT("InitializeSystemByName: Dependencies not met for %s"), *SystemName);
        return false;
    }
    
    return InitializeSystemInternal(SystemInfo);
}

bool USystemIntegrationManager::InitializeSystemsByAgent(const FString& AgentNumber)
{
    TArray<FString> AgentSystems;
    
    // Find all systems for this agent
    for (const FAgentSystemInfo& SystemInfo : RegisteredSystems)
    {
        if (SystemInfo.AgentNumber == AgentNumber)
        {
            AgentSystems.Add(SystemInfo.SystemName);
        }
    }
    
    if (AgentSystems.Num() == 0)
    {
        UE_LOG(LogSystemIntegration, Warning, TEXT("InitializeSystemsByAgent: No systems found for agent %s"), *AgentNumber);
        return true;
    }
    
    // Initialize all systems for this agent
    int32 SuccessfulInitializations = 0;
    for (const FString& SystemName : AgentSystems)
    {
        if (InitializeSystemByName(SystemName))
        {
            SuccessfulInitializations++;
        }
    }
    
    bool bSuccess = (SuccessfulInitializations == AgentSystems.Num());
    UE_LOG(LogSystemIntegration, Log, TEXT("InitializeSystemsByAgent: Agent %s - %d/%d systems initialized"), 
           *AgentNumber, SuccessfulInitializations, AgentSystems.Num());
    
    return bSuccess;
}

FSystemDependencyResult USystemIntegrationManager::ValidateSystemDependencies()
{
    FSystemDependencyResult Result;
    
    // Check for missing dependencies
    for (const FAgentSystemInfo& SystemInfo : RegisteredSystems)
    {
        for (const FString& Dependency : SystemInfo.Dependencies)
        {
            if (!SystemNameToIndex.Contains(Dependency))
            {
                Result.MissingDependencies.AddUnique(Dependency);
            }
        }
    }
    
    // Check for circular dependencies
    if (!HasCircularDependencies(Result.CircularDependencies))
    {
        // Generate initialization order if no circular dependencies
        Result.InitializationOrder = ResolveDependencyOrder();
    }
    
    Result.bAllDependenciesMet = (Result.MissingDependencies.Num() == 0 && Result.CircularDependencies.Num() == 0);
    
    return Result;
}

TArray<FString> USystemIntegrationManager::GetInitializationOrder()
{
    if (InitializationOrder.Num() == 0)
    {
        InitializationOrder = ResolveDependencyOrder();
    }
    return InitializationOrder;
}

bool USystemIntegrationManager::CheckSystemDependency(const FString& SystemName, const FString& DependencyName)
{
    if (!SystemNameToIndex.Contains(SystemName))
    {
        return false;
    }
    
    int32 SystemIndex = SystemNameToIndex[SystemName];
    const FAgentSystemInfo& SystemInfo = RegisteredSystems[SystemIndex];
    
    return SystemInfo.Dependencies.Contains(DependencyName);
}

FSystemIntegrationStatus USystemIntegrationManager::GetIntegrationStatus()
{
    FSystemIntegrationStatus Status;
    
    Status.TotalSystems = RegisteredSystems.Num();
    Status.InitializedSystems = 0;
    Status.FailedSystems = 0;
    
    for (const FAgentSystemInfo& SystemInfo : RegisteredSystems)
    {
        if (SystemInfo.bIsInitialized)
        {
            Status.InitializedSystems++;
            Status.TotalInitializationTime += SystemInfo.InitializationTime;
        }
        else
        {
            Status.FailedSystems++;
        }
    }
    
    Status.bAllSystemsInitialized = (Status.InitializedSystems == Status.TotalSystems);
    Status.bArchitectureCompliant = ValidateSystemArchitecture();
    
    return Status;
}

bool USystemIntegrationManager::IsSystemInitialized(const FString& SystemName)
{
    if (!SystemNameToIndex.Contains(SystemName))
    {
        return false;
    }
    
    int32 SystemIndex = SystemNameToIndex[SystemName];
    return RegisteredSystems[SystemIndex].bIsInitialized;
}

bool USystemIntegrationManager::AreAllSystemsReady()
{
    return bAllSystemsInitialized && ValidateSystemArchitecture();
}

bool USystemIntegrationManager::ValidateSystemArchitecture()
{
    if (!ArchitectureValidator)
    {
        UE_LOG(LogSystemIntegration, Warning, TEXT("ValidateSystemArchitecture: No architecture validator available"));
        return false;
    }
    
    return ArchitectureValidator->ValidateArchitecture();
}

void USystemIntegrationManager::GenerateSystemIntegrationReport()
{
    UE_LOG(LogSystemIntegration, Log, TEXT("=== SYSTEM INTEGRATION REPORT ==="));
    
    FSystemIntegrationStatus Status = GetIntegrationStatus();
    
    UE_LOG(LogSystemIntegration, Log, TEXT("Total Systems: %d"), Status.TotalSystems);
    UE_LOG(LogSystemIntegration, Log, TEXT("Initialized: %d"), Status.InitializedSystems);
    UE_LOG(LogSystemIntegration, Log, TEXT("Failed: %d"), Status.FailedSystems);
    UE_LOG(LogSystemIntegration, Log, TEXT("Architecture Compliant: %s"), Status.bArchitectureCompliant ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogSystemIntegration, Log, TEXT("Total Init Time: %.3f seconds"), Status.TotalInitializationTime);
    
    UE_LOG(LogSystemIntegration, Log, TEXT("\n--- REGISTERED SYSTEMS ---"));
    for (const FAgentSystemInfo& SystemInfo : RegisteredSystems)
    {
        UE_LOG(LogSystemIntegration, Log, TEXT("%s [%s] - %s - Init: %s (%.3f s)"), 
               *SystemInfo.SystemName,
               *SystemInfo.AgentNumber,
               *SystemInfo.AgentName,
               SystemInfo.bIsInitialized ? TEXT("YES") : TEXT("NO"),
               SystemInfo.InitializationTime);
    }
    
    UE_LOG(LogSystemIntegration, Log, TEXT("=== END REPORT ==="));
}

bool USystemIntegrationManager::ValidateAgentChain()
{
    UE_LOG(LogSystemIntegration, Log, TEXT("ValidateAgentChain: Validating 19-agent chain..."));
    
    // Check if we have the correct number of agents
    if (AgentChain.Num() != 19)
    {
        UE_LOG(LogSystemIntegration, Error, TEXT("ValidateAgentChain: Expected 19 agents, found %d"), AgentChain.Num());
        return false;
    }
    
    // Validate agent sequence
    if (!ValidateAgentSequence())
    {
        UE_LOG(LogSystemIntegration, Error, TEXT("ValidateAgentChain: Agent sequence validation failed"));
        return false;
    }
    
    LogAgentChainStatus();
    return true;
}

TArray<FString> USystemIntegrationManager::GetAgentChainOrder()
{
    return AgentChain;
}

// Protected methods implementation

bool USystemIntegrationManager::InitializeSystemInternal(FAgentSystemInfo& SystemInfo)
{
    float StartTime = FPlatformTime::Seconds();
    
    UE_LOG(LogSystemIntegration, Log, TEXT("InitializeSystemInternal: Initializing %s..."), *SystemInfo.SystemName);
    
    // Simulate system initialization
    // In a real implementation, this would call the actual system initialization
    bool bInitSuccess = true;
    
    // Record timing
    float EndTime = FPlatformTime::Seconds();
    SystemInfo.InitializationTime = EndTime - StartTime;
    SystemInfo.bIsInitialized = bInitSuccess;
    
    // Update tracking
    SystemInitializationTimes.Add(SystemInfo.SystemName, SystemInfo.InitializationTime);
    
    if (bInitSuccess)
    {
        OnSystemInitialized.Broadcast(SystemInfo.SystemName);
        UE_LOG(LogSystemIntegration, Log, TEXT("InitializeSystemInternal: %s initialized successfully (%.3f s)"), 
               *SystemInfo.SystemName, SystemInfo.InitializationTime);
    }
    else
    {
        OnSystemInitializationFailed.Broadcast(SystemInfo.SystemName);
        UE_LOG(LogSystemIntegration, Error, TEXT("InitializeSystemInternal: %s initialization failed"), *SystemInfo.SystemName);
    }
    
    return bInitSuccess;
}

bool USystemIntegrationManager::ValidateSystemRequirements(const FAgentSystemInfo& SystemInfo)
{
    // Basic validation
    if (SystemInfo.SystemName.IsEmpty() || SystemInfo.AgentNumber.IsEmpty())
    {
        return false;
    }
    
    // Validate agent number format
    if (!SystemInfo.AgentNumber.StartsWith(TEXT("0")) || SystemInfo.AgentNumber.Len() < 2)
    {
        UE_LOG(LogSystemIntegration, Warning, TEXT("ValidateSystemRequirements: Invalid agent number format: %s"), *SystemInfo.AgentNumber);
        return false;
    }
    
    return true;
}

void USystemIntegrationManager::UpdateSystemStatus()
{
    // Count initialized systems
    int32 InitializedCount = 0;
    for (const FAgentSystemInfo& SystemInfo : RegisteredSystems)
    {
        if (SystemInfo.bIsInitialized)
        {
            InitializedCount++;
        }
    }
    
    // Update global status
    bAllSystemsInitialized = (InitializedCount == RegisteredSystems.Num() && RegisteredSystems.Num() > 0);
}

TArray<FString> USystemIntegrationManager::ResolveDependencyOrder()
{
    TArray<FString> Order;
    TSet<FString> Visited;
    TSet<FString> InProgress;
    
    // Use topological sort with agent priority
    for (const FAgentSystemInfo& SystemInfo : RegisteredSystems)
    {
        if (!Visited.Contains(SystemInfo.SystemName))
        {
            ResolveDependencyOrderRecursive(SystemInfo.SystemName, Order, Visited, InProgress);
        }
    }
    
    return Order;
}

void USystemIntegrationManager::ResolveDependencyOrderRecursive(const FString& SystemName, TArray<FString>& Order, TSet<FString>& Visited, TSet<FString>& InProgress)
{
    if (InProgress.Contains(SystemName))
    {
        // Circular dependency detected
        return;
    }
    
    if (Visited.Contains(SystemName))
    {
        return;
    }
    
    InProgress.Add(SystemName);
    
    // Process dependencies first
    if (SystemNameToIndex.Contains(SystemName))
    {
        int32 SystemIndex = SystemNameToIndex[SystemName];
        const FAgentSystemInfo& SystemInfo = RegisteredSystems[SystemIndex];
        
        for (const FString& Dependency : SystemInfo.Dependencies)
        {
            ResolveDependencyOrderRecursive(Dependency, Order, Visited, InProgress);
        }
    }
    
    InProgress.Remove(SystemName);
    Visited.Add(SystemName);
    Order.Add(SystemName);
}

bool USystemIntegrationManager::HasCircularDependencies(TArray<FString>& CircularDeps)
{
    TSet<FString> Visited;
    TSet<FString> InProgress;
    
    for (const FAgentSystemInfo& SystemInfo : RegisteredSystems)
    {
        if (!Visited.Contains(SystemInfo.SystemName))
        {
            if (CheckCircularDependencyRecursive(SystemInfo.SystemName, Visited, InProgress, CircularDeps))
            {
                return true;
            }
        }
    }
    
    return false;
}

bool USystemIntegrationManager::CheckCircularDependencyRecursive(const FString& SystemName, TSet<FString>& Visited, TSet<FString>& InProgress, TArray<FString>& CircularDeps)
{
    if (InProgress.Contains(SystemName))
    {
        CircularDeps.Add(SystemName);
        return true;
    }
    
    if (Visited.Contains(SystemName))
    {
        return false;
    }
    
    InProgress.Add(SystemName);
    
    if (SystemNameToIndex.Contains(SystemName))
    {
        int32 SystemIndex = SystemNameToIndex[SystemName];
        const FAgentSystemInfo& SystemInfo = RegisteredSystems[SystemIndex];
        
        for (const FString& Dependency : SystemInfo.Dependencies)
        {
            if (CheckCircularDependencyRecursive(Dependency, Visited, InProgress, CircularDeps))
            {
                CircularDeps.Add(SystemName);
                return true;
            }
        }
    }
    
    InProgress.Remove(SystemName);
    Visited.Add(SystemName);
    return false;
}

bool USystemIntegrationManager::CanInitializeSystem(const FAgentSystemInfo& SystemInfo)
{
    // Check if all dependencies are initialized
    for (const FString& Dependency : SystemInfo.Dependencies)
    {
        if (!IsSystemInitialized(Dependency))
        {
            return false;
        }
    }
    
    return true;
}

bool USystemIntegrationManager::ValidateAgentSequence()
{
    // Validate that agents are numbered 01-19 in sequence
    for (int32 i = 0; i < AgentChain.Num(); i++)
    {
        FString ExpectedNumber = FString::Printf(TEXT("%02d"), i + 1);
        if (!AgentChain[i].StartsWith(ExpectedNumber))
        {
            UE_LOG(LogSystemIntegration, Error, TEXT("ValidateAgentSequence: Agent %d has incorrect number: %s"), 
                   i + 1, *AgentChain[i]);
            return false;
        }
    }
    
    return true;
}

void USystemIntegrationManager::LogAgentChainStatus()
{
    UE_LOG(LogSystemIntegration, Log, TEXT("Agent Chain Status:"));
    for (int32 i = 0; i < AgentChain.Num(); i++)
    {
        UE_LOG(LogSystemIntegration, Log, TEXT("  %02d: %s"), i + 1, *AgentChain[i]);
    }
}

void USystemIntegrationManager::RegisterCoreAgentSystems()
{
    // Register core systems for each agent
    
    // Agent 01 - Studio Director
    FAgentSystemInfo StudioDirectorSystem;
    StudioDirectorSystem.SystemName = TEXT("StudioDirectorSystem");
    StudioDirectorSystem.AgentNumber = TEXT("01");
    StudioDirectorSystem.AgentName = TEXT("Studio Director");
    StudioDirectorSystem.bIsCoreSystem = true;
    StudioDirectorSystem.InitializationPriority = 100;
    StudioDirectorSystem.Provides.Add(TEXT("ProjectManagement"));
    StudioDirectorSystem.Provides.Add(TEXT("AgentCoordination"));
    RegisterAgentSystem(StudioDirectorSystem);
    
    // Agent 02 - Engine Architect
    FAgentSystemInfo EngineArchitectSystem;
    EngineArchitectSystem.SystemName = TEXT("EngineArchitectureSystem");
    EngineArchitectSystem.AgentNumber = TEXT("02");
    EngineArchitectSystem.AgentName = TEXT("Engine Architect");
    EngineArchitectSystem.bIsCoreSystem = true;
    EngineArchitectSystem.InitializationPriority = 90;
    EngineArchitectSystem.Dependencies.Add(TEXT("StudioDirectorSystem"));
    EngineArchitectSystem.Provides.Add(TEXT("TechnicalArchitecture"));
    EngineArchitectSystem.Provides.Add(TEXT("SystemValidation"));
    RegisterAgentSystem(EngineArchitectSystem);
    
    UE_LOG(LogSystemIntegration, Log, TEXT("RegisterCoreAgentSystems: Registered %d core systems"), 2);
}