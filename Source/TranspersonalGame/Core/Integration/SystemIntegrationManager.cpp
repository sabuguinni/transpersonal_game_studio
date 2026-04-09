// Copyright Transpersonal Game Studio. All Rights Reserved.
// SystemIntegrationManager.cpp - Implementation of system integration management

#include "SystemIntegrationManager.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Misc/DateTime.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

DEFINE_LOG_CATEGORY(LogSystemIntegration);

// Agent chain definition (19 agents in order)
const TArray<FString> USystemIntegrationManager::AgentChain = {
    TEXT("01_StudioDirector"),
    TEXT("02_EngineArchitect"),
    TEXT("03_CoreSystemsProgrammer"),
    TEXT("04_PerformanceOptimizer"),
    TEXT("05_ProceduralWorldGenerator"),
    TEXT("06_EnvironmentArtist"),
    TEXT("07_ArchitectureInteriorAgent"),
    TEXT("08_LightingAtmosphereAgent"),
    TEXT("09_CharacterArtistAgent"),
    TEXT("10_AnimationAgent"),
    TEXT("11_NPCBehaviorAgent"),
    TEXT("12_CombatEnemyAIAgent"),
    TEXT("13_CrowdTrafficSimulation"),
    TEXT("14_QuestMissionDesigner"),
    TEXT("15_NarrativeDialogueAgent"),
    TEXT("16_AudioAgent"),
    TEXT("17_VFXAgent"),
    TEXT("18_QATestingAgent"),
    TEXT("19_IntegrationBuildAgent")
};

void USystemIntegrationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogSystemIntegration, Log, TEXT("SystemIntegrationManager: Initializing..."));
    
    // Initialize system registry
    RegisteredSystems.Empty();
    SystemNameToIndex.Empty();
    InitializationOrder.Empty();
    
    // Get architecture validator reference
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        ArchitectureValidator = GameInstance->GetSubsystem<UEngineArchitectureValidator>();
        if (!ArchitectureValidator)
        {
            UE_LOG(LogSystemIntegration, Warning, TEXT("SystemIntegrationManager: EngineArchitectureValidator not found"));
        }
    }
    
    // Initialize timing
    SystemInitializationStartTime = FPlatformTime::Seconds();
    SystemInitializationTimes.Empty();
    
    bIntegrationInitialized = true;
    bAllSystemsInitialized = false;
    
    UE_LOG(LogSystemIntegration, Log, TEXT("SystemIntegrationManager: Initialization complete"));
}

void USystemIntegrationManager::Deinitialize()
{
    UE_LOG(LogSystemIntegration, Log, TEXT("SystemIntegrationManager: Deinitializing..."));
    
    // Generate final integration report
    GenerateSystemIntegrationReport();
    
    // Clear all systems
    RegisteredSystems.Empty();
    SystemNameToIndex.Empty();
    InitializationOrder.Empty();
    SystemInitializationTimes.Empty();
    
    bIntegrationInitialized = false;
    bAllSystemsInitialized = false;
    
    Super::Deinitialize();
}

bool USystemIntegrationManager::RegisterAgentSystem(const FAgentSystemInfo& SystemInfo)
{
    if (!bIntegrationInitialized)
    {
        UE_LOG(LogSystemIntegration, Error, TEXT("SystemIntegrationManager: Cannot register system - manager not initialized"));
        return false;
    }
    
    if (SystemInfo.SystemName.IsEmpty())
    {
        UE_LOG(LogSystemIntegration, Error, TEXT("SystemIntegrationManager: Cannot register system with empty name"));
        return false;
    }
    
    // Check if system already registered
    if (SystemNameToIndex.Contains(SystemInfo.SystemName))
    {
        UE_LOG(LogSystemIntegration, Warning, TEXT("SystemIntegrationManager: System '%s' already registered, updating..."), *SystemInfo.SystemName);
        
        int32 Index = SystemNameToIndex[SystemInfo.SystemName];
        RegisteredSystems[Index] = SystemInfo;
    }
    else
    {
        // Add new system
        int32 NewIndex = RegisteredSystems.Add(SystemInfo);
        SystemNameToIndex.Add(SystemInfo.SystemName, NewIndex);
        
        UE_LOG(LogSystemIntegration, Log, TEXT("SystemIntegrationManager: Registered system '%s' (Agent %s: %s)"), 
               *SystemInfo.SystemName, *SystemInfo.AgentNumber, *SystemInfo.AgentName);
    }
    
    // Broadcast registration event
    OnSystemRegistered.Broadcast(SystemInfo.SystemName);
    
    // Update system status
    UpdateSystemStatus();
    
    return true;
}

bool USystemIntegrationManager::UnregisterAgentSystem(const FString& SystemName)
{
    if (!SystemNameToIndex.Contains(SystemName))
    {
        UE_LOG(LogSystemIntegration, Warning, TEXT("SystemIntegrationManager: System '%s' not found for unregistration"), *SystemName);
        return false;
    }
    
    int32 Index = SystemNameToIndex[SystemName];
    RegisteredSystems.RemoveAt(Index);
    SystemNameToIndex.Remove(SystemName);
    
    // Rebuild index map
    SystemNameToIndex.Empty();
    for (int32 i = 0; i < RegisteredSystems.Num(); i++)
    {
        SystemNameToIndex.Add(RegisteredSystems[i].SystemName, i);
    }
    
    UE_LOG(LogSystemIntegration, Log, TEXT("SystemIntegrationManager: Unregistered system '%s'"), *SystemName);
    
    UpdateSystemStatus();
    return true;
}

FAgentSystemInfo USystemIntegrationManager::GetSystemInfo(const FString& SystemName) const
{
    if (SystemNameToIndex.Contains(SystemName))
    {
        int32 Index = SystemNameToIndex[SystemName];
        return RegisteredSystems[Index];
    }
    
    return FAgentSystemInfo();
}

bool USystemIntegrationManager::InitializeAllSystems()
{
    if (!bIntegrationInitialized)
    {
        UE_LOG(LogSystemIntegration, Error, TEXT("SystemIntegrationManager: Cannot initialize systems - manager not initialized"));
        return false;
    }
    
    UE_LOG(LogSystemIntegration, Log, TEXT("SystemIntegrationManager: Starting initialization of all systems..."));
    
    // Validate dependencies first
    FSystemDependencyResult DependencyResult = ValidateSystemDependencies();
    if (!DependencyResult.bAllDependenciesMet)
    {
        UE_LOG(LogSystemIntegration, Error, TEXT("SystemIntegrationManager: Cannot initialize - dependency validation failed"));
        
        for (const FString& MissingDep : DependencyResult.MissingDependencies)
        {
            UE_LOG(LogSystemIntegration, Error, TEXT("  Missing dependency: %s"), *MissingDep);
        }
        
        for (const FString& CircularDep : DependencyResult.CircularDependencies)
        {
            UE_LOG(LogSystemIntegration, Error, TEXT("  Circular dependency: %s"), *CircularDep);
        }
        
        return false;
    }
    
    // Get initialization order
    InitializationOrder = DependencyResult.InitializationOrder;
    
    // Initialize systems in dependency order
    bool bAllSucceeded = true;
    float StartTime = FPlatformTime::Seconds();
    
    for (const FString& SystemName : InitializationOrder)
    {
        if (SystemNameToIndex.Contains(SystemName))
        {
            int32 Index = SystemNameToIndex[SystemName];
            FAgentSystemInfo& SystemInfo = RegisteredSystems[Index];
            
            if (!SystemInfo.bIsInitialized)
            {
                float SystemStartTime = FPlatformTime::Seconds();
                
                if (InitializeSystemInternal(SystemInfo))
                {
                    float SystemEndTime = FPlatformTime::Seconds();
                    SystemInfo.InitializationTime = SystemEndTime - SystemStartTime;
                    SystemInitializationTimes.Add(SystemName, SystemInfo.InitializationTime);
                    
                    UE_LOG(LogSystemIntegration, Log, TEXT("SystemIntegrationManager: Initialized system '%s' in %.3f seconds"), 
                           *SystemName, SystemInfo.InitializationTime);
                    
                    OnSystemInitialized.Broadcast(SystemName);
                }
                else
                {
                    UE_LOG(LogSystemIntegration, Error, TEXT("SystemIntegrationManager: Failed to initialize system '%s'"), *SystemName);
                    OnSystemInitializationFailed.Broadcast(SystemName);
                    bAllSucceeded = false;
                }
            }
        }
    }
    
    float TotalTime = FPlatformTime::Seconds() - StartTime;
    
    if (bAllSucceeded)
    {
        bAllSystemsInitialized = true;
        UE_LOG(LogSystemIntegration, Log, TEXT("SystemIntegrationManager: All systems initialized successfully in %.3f seconds"), TotalTime);
        OnAllSystemsReady.Broadcast();
    }
    else
    {
        UE_LOG(LogSystemIntegration, Error, TEXT("SystemIntegrationManager: System initialization completed with errors in %.3f seconds"), TotalTime);
    }
    
    UpdateSystemStatus();
    return bAllSucceeded;
}

bool USystemIntegrationManager::InitializeSystemByName(const FString& SystemName)
{
    if (!SystemNameToIndex.Contains(SystemName))
    {
        UE_LOG(LogSystemIntegration, Error, TEXT("SystemIntegrationManager: System '%s' not found"), *SystemName);
        return false;
    }
    
    int32 Index = SystemNameToIndex[SystemName];
    FAgentSystemInfo& SystemInfo = RegisteredSystems[Index];
    
    if (SystemInfo.bIsInitialized)
    {
        UE_LOG(LogSystemIntegration, Warning, TEXT("SystemIntegrationManager: System '%s' already initialized"), *SystemName);
        return true;
    }
    
    // Check dependencies
    if (!CanInitializeSystem(SystemInfo))
    {
        UE_LOG(LogSystemIntegration, Error, TEXT("SystemIntegrationManager: Cannot initialize system '%s' - dependencies not met"), *SystemName);
        return false;
    }
    
    float StartTime = FPlatformTime::Seconds();
    bool bSuccess = InitializeSystemInternal(SystemInfo);
    float EndTime = FPlatformTime::Seconds();
    
    if (bSuccess)
    {
        SystemInfo.InitializationTime = EndTime - StartTime;
        SystemInitializationTimes.Add(SystemName, SystemInfo.InitializationTime);
        
        UE_LOG(LogSystemIntegration, Log, TEXT("SystemIntegrationManager: Initialized system '%s' in %.3f seconds"), 
               *SystemName, SystemInfo.InitializationTime);
        
        OnSystemInitialized.Broadcast(SystemName);
    }
    else
    {
        UE_LOG(LogSystemIntegration, Error, TEXT("SystemIntegrationManager: Failed to initialize system '%s'"), *SystemName);
        OnSystemInitializationFailed.Broadcast(SystemName);
    }
    
    UpdateSystemStatus();
    return bSuccess;
}

bool USystemIntegrationManager::InitializeSystemsByAgent(const FString& AgentNumber)
{
    TArray<FString> SystemsToInitialize;
    
    // Find all systems for this agent
    for (const FAgentSystemInfo& SystemInfo : RegisteredSystems)
    {
        if (SystemInfo.AgentNumber == AgentNumber && !SystemInfo.bIsInitialized)
        {
            SystemsToInitialize.Add(SystemInfo.SystemName);
        }
    }
    
    if (SystemsToInitialize.Num() == 0)
    {
        UE_LOG(LogSystemIntegration, Warning, TEXT("SystemIntegrationManager: No systems found for agent '%s'"), *AgentNumber);
        return true;
    }
    
    UE_LOG(LogSystemIntegration, Log, TEXT("SystemIntegrationManager: Initializing %d systems for agent '%s'"), 
           SystemsToInitialize.Num(), *AgentNumber);
    
    bool bAllSucceeded = true;
    for (const FString& SystemName : SystemsToInitialize)
    {
        if (!InitializeSystemByName(SystemName))
        {
            bAllSucceeded = false;
        }
    }
    
    return bAllSucceeded;
}

FSystemDependencyResult USystemIntegrationManager::ValidateSystemDependencies()
{
    FSystemDependencyResult Result;
    
    // Check for missing dependencies
    for (const FAgentSystemInfo& SystemInfo : RegisteredSystems)
    {
        for (const FString& Dependency : SystemInfo.Dependencies)
        {
            bool bFound = false;
            for (const FAgentSystemInfo& OtherSystem : RegisteredSystems)
            {
                if (OtherSystem.Provides.Contains(Dependency))
                {
                    bFound = true;
                    break;
                }
            }
            
            if (!bFound)
            {
                Result.MissingDependencies.AddUnique(Dependency);
            }
        }
    }
    
    // Check for circular dependencies
    if (!HasCircularDependencies(Result.CircularDependencies))
    {
        // Generate initialization order
        Result.InitializationOrder = ResolveDependencyOrder();
        Result.bAllDependenciesMet = (Result.MissingDependencies.Num() == 0);
    }
    else
    {
        Result.bAllDependenciesMet = false;
    }
    
    return Result;
}

TArray<FString> USystemIntegrationManager::GetInitializationOrder()
{
    return ResolveDependencyOrder();
}

bool USystemIntegrationManager::CheckSystemDependency(const FString& SystemName, const FString& DependencyName)
{
    if (!SystemNameToIndex.Contains(SystemName))
    {
        return false;
    }
    
    int32 Index = SystemNameToIndex[SystemName];
    const FAgentSystemInfo& SystemInfo = RegisteredSystems[Index];
    
    return SystemInfo.Dependencies.Contains(DependencyName);
}

FSystemIntegrationStatus USystemIntegrationManager::GetIntegrationStatus()
{
    FSystemIntegrationStatus Status;
    
    Status.TotalSystems = RegisteredSystems.Num();
    Status.InitializedSystems = 0;
    Status.FailedSystems = 0;
    Status.bAllSystemsInitialized = true;
    Status.TotalInitializationTime = 0.0f;
    
    for (const FAgentSystemInfo& SystemInfo : RegisteredSystems)
    {
        if (SystemInfo.bIsInitialized)
        {
            Status.InitializedSystems++;
            Status.TotalInitializationTime += SystemInfo.InitializationTime;
        }
        else
        {
            Status.bAllSystemsInitialized = false;
        }
    }
    
    Status.FailedSystems = Status.TotalSystems - Status.InitializedSystems;
    
    // Check architecture compliance
    Status.bArchitectureCompliant = ValidateSystemArchitecture();
    
    return Status;
}

bool USystemIntegrationManager::IsSystemInitialized(const FString& SystemName)
{
    if (!SystemNameToIndex.Contains(SystemName))
    {
        return false;
    }
    
    int32 Index = SystemNameToIndex[SystemName];
    return RegisteredSystems[Index].bIsInitialized;
}

bool USystemIntegrationManager::AreAllSystemsReady()
{
    return bAllSystemsInitialized;
}

bool USystemIntegrationManager::ValidateSystemArchitecture()
{
    if (!ArchitectureValidator)
    {
        UE_LOG(LogSystemIntegration, Warning, TEXT("SystemIntegrationManager: ArchitectureValidator not available"));
        return false;
    }
    
    // Validate each registered system against architecture standards
    bool bAllValid = true;
    
    for (const FAgentSystemInfo& SystemInfo : RegisteredSystems)
    {
        // This would call into the architecture validator to check compliance
        // For now, we'll do basic validation
        if (SystemInfo.SystemName.IsEmpty() || SystemInfo.AgentNumber.IsEmpty())
        {
            bAllValid = false;
            UE_LOG(LogSystemIntegration, Error, TEXT("SystemIntegrationManager: System validation failed - missing required fields"));
        }
    }
    
    return bAllValid;
}

void USystemIntegrationManager::GenerateSystemIntegrationReport()
{
    UE_LOG(LogSystemIntegration, Log, TEXT("SystemIntegrationManager: Generating integration report..."));
    
    FString ReportContent;
    ReportContent += TEXT("=== SYSTEM INTEGRATION REPORT ===\n");
    ReportContent += FString::Printf(TEXT("Generated: %s\n"), *FDateTime::Now().ToString());
    ReportContent += TEXT("\n");
    
    // System overview
    FSystemIntegrationStatus Status = GetIntegrationStatus();
    ReportContent += TEXT("=== SYSTEM OVERVIEW ===\n");
    ReportContent += FString::Printf(TEXT("Total Systems: %d\n"), Status.TotalSystems);
    ReportContent += FString::Printf(TEXT("Initialized Systems: %d\n"), Status.InitializedSystems);
    ReportContent += FString::Printf(TEXT("Failed Systems: %d\n"), Status.FailedSystems);
    ReportContent += FString::Printf(TEXT("All Systems Ready: %s\n"), Status.bAllSystemsInitialized ? TEXT("YES") : TEXT("NO"));
    ReportContent += FString::Printf(TEXT("Architecture Compliant: %s\n"), Status.bArchitectureCompliant ? TEXT("YES") : TEXT("NO"));
    ReportContent += FString::Printf(TEXT("Total Initialization Time: %.3f seconds\n"), Status.TotalInitializationTime);
    ReportContent += TEXT("\n");
    
    // Agent chain validation
    ReportContent += TEXT("=== AGENT CHAIN VALIDATION ===\n");
    bool bChainValid = ValidateAgentChain();
    ReportContent += FString::Printf(TEXT("Agent Chain Valid: %s\n"), bChainValid ? TEXT("YES") : TEXT("NO"));
    
    TArray<FString> ChainOrder = GetAgentChainOrder();
    ReportContent += TEXT("Expected Agent Order:\n");
    for (int32 i = 0; i < ChainOrder.Num(); i++)
    {
        ReportContent += FString::Printf(TEXT("  %02d. %s\n"), i + 1, *ChainOrder[i]);
    }
    ReportContent += TEXT("\n");
    
    // System details
    ReportContent += TEXT("=== REGISTERED SYSTEMS ===\n");
    for (const FAgentSystemInfo& SystemInfo : RegisteredSystems)
    {
        ReportContent += FString::Printf(TEXT("System: %s\n"), *SystemInfo.SystemName);
        ReportContent += FString::Printf(TEXT("  Agent: %s - %s\n"), *SystemInfo.AgentNumber, *SystemInfo.AgentName);
        ReportContent += FString::Printf(TEXT("  Initialized: %s\n"), SystemInfo.bIsInitialized ? TEXT("YES") : TEXT("NO"));
        ReportContent += FString::Printf(TEXT("  Core System: %s\n"), SystemInfo.bIsCoreSystem ? TEXT("YES") : TEXT("NO"));
        ReportContent += FString::Printf(TEXT("  Priority: %d\n"), SystemInfo.InitializationPriority);
        ReportContent += FString::Printf(TEXT("  Init Time: %.3f seconds\n"), SystemInfo.InitializationTime);
        
        if (SystemInfo.Dependencies.Num() > 0)
        {
            ReportContent += TEXT("  Dependencies: ");
            for (int32 i = 0; i < SystemInfo.Dependencies.Num(); i++)
            {
                ReportContent += SystemInfo.Dependencies[i];
                if (i < SystemInfo.Dependencies.Num() - 1)
                {
                    ReportContent += TEXT(", ");
                }
            }
            ReportContent += TEXT("\n");
        }
        
        if (SystemInfo.Provides.Num() > 0)
        {
            ReportContent += TEXT("  Provides: ");
            for (int32 i = 0; i < SystemInfo.Provides.Num(); i++)
            {
                ReportContent += SystemInfo.Provides[i];
                if (i < SystemInfo.Provides.Num() - 1)
                {
                    ReportContent += TEXT(", ");
                }
            }
            ReportContent += TEXT("\n");
        }
        
        ReportContent += TEXT("\n");
    }
    
    // Save report to file
    FString ReportPath = FPaths::ProjectLogDir() / TEXT("SystemIntegrationReport.txt");
    if (FFileHelper::SaveStringToFile(ReportContent, *ReportPath))
    {
        UE_LOG(LogSystemIntegration, Log, TEXT("SystemIntegrationManager: Report saved to %s"), *ReportPath);
    }
    else
    {
        UE_LOG(LogSystemIntegration, Error, TEXT("SystemIntegrationManager: Failed to save report to %s"), *ReportPath);
    }
    
    // Also log the report
    UE_LOG(LogSystemIntegration, Log, TEXT("\n%s"), *ReportContent);
}

bool USystemIntegrationManager::ValidateAgentChain()
{
    // Check if we have systems registered for each agent in the chain
    TSet<FString> RegisteredAgents;
    
    for (const FAgentSystemInfo& SystemInfo : RegisteredSystems)
    {
        RegisteredAgents.Add(SystemInfo.AgentNumber);
    }
    
    bool bChainValid = true;
    for (const FString& ExpectedAgent : AgentChain)
    {
        if (!RegisteredAgents.Contains(ExpectedAgent))
        {
            UE_LOG(LogSystemIntegration, Warning, TEXT("SystemIntegrationManager: Missing systems for agent %s"), *ExpectedAgent);
            bChainValid = false;
        }
    }
    
    return bChainValid;
}

TArray<FString> USystemIntegrationManager::GetAgentChainOrder()
{
    return AgentChain;
}

// Protected methods implementation

bool USystemIntegrationManager::InitializeSystemInternal(FAgentSystemInfo& SystemInfo)
{
    if (!ValidateSystemRequirements(SystemInfo))
    {
        return false;
    }
    
    // Mark as initialized (in a real implementation, this would call the actual system initialization)
    SystemInfo.bIsInitialized = true;
    
    return true;
}

bool USystemIntegrationManager::ValidateSystemRequirements(const FAgentSystemInfo& SystemInfo)
{
    // Basic validation
    if (SystemInfo.SystemName.IsEmpty())
    {
        UE_LOG(LogSystemIntegration, Error, TEXT("SystemIntegrationManager: System name cannot be empty"));
        return false;
    }
    
    if (SystemInfo.AgentNumber.IsEmpty())
    {
        UE_LOG(LogSystemIntegration, Error, TEXT("SystemIntegrationManager: Agent number cannot be empty"));
        return false;
    }
    
    return true;
}

void USystemIntegrationManager::UpdateSystemStatus()
{
    // Check if all systems are initialized
    bool bAllInitialized = true;
    for (const FAgentSystemInfo& SystemInfo : RegisteredSystems)
    {
        if (!SystemInfo.bIsInitialized)
        {
            bAllInitialized = false;
            break;
        }
    }
    
    if (bAllInitialized && !bAllSystemsInitialized && RegisteredSystems.Num() > 0)
    {
        bAllSystemsInitialized = true;
        OnAllSystemsReady.Broadcast();
    }
}

TArray<FString> USystemIntegrationManager::ResolveDependencyOrder()
{
    TArray<FString> Order;
    TSet<FString> Visited;
    TSet<FString> InProgress;
    
    // Topological sort with dependency resolution
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
    
    // Find system info
    if (SystemNameToIndex.Contains(SystemName))
    {
        int32 Index = SystemNameToIndex[SystemName];
        const FAgentSystemInfo& SystemInfo = RegisteredSystems[Index];
        
        // Process dependencies first
        for (const FString& Dependency : SystemInfo.Dependencies)
        {
            // Find system that provides this dependency
            for (const FAgentSystemInfo& OtherSystem : RegisteredSystems)
            {
                if (OtherSystem.Provides.Contains(Dependency))
                {
                    ResolveDependencyOrderRecursive(OtherSystem.SystemName, Order, Visited, InProgress);
                    break;
                }
            }
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
        int32 Index = SystemNameToIndex[SystemName];
        const FAgentSystemInfo& SystemInfo = RegisteredSystems[Index];
        
        for (const FString& Dependency : SystemInfo.Dependencies)
        {
            for (const FAgentSystemInfo& OtherSystem : RegisteredSystems)
            {
                if (OtherSystem.Provides.Contains(Dependency))
                {
                    if (CheckCircularDependencyRecursive(OtherSystem.SystemName, Visited, InProgress, CircularDeps))
                    {
                        CircularDeps.Add(SystemName);
                        return true;
                    }
                    break;
                }
            }
        }
    }
    
    InProgress.Remove(SystemName);
    Visited.Add(SystemName);
    
    return false;
}

bool USystemIntegrationManager::CanInitializeSystem(const FAgentSystemInfo& SystemInfo)
{
    // Check if all dependencies are satisfied
    for (const FString& Dependency : SystemInfo.Dependencies)
    {
        bool bDependencyMet = false;
        
        for (const FAgentSystemInfo& OtherSystem : RegisteredSystems)
        {
            if (OtherSystem.Provides.Contains(Dependency) && OtherSystem.bIsInitialized)
            {
                bDependencyMet = true;
                break;
            }
        }
        
        if (!bDependencyMet)
        {
            return false;
        }
    }
    
    return true;
}

bool USystemIntegrationManager::ValidateAgentSequence()
{
    // Validate that systems are registered in the correct agent sequence
    TMap<FString, int32> AgentOrder;
    
    for (int32 i = 0; i < AgentChain.Num(); i++)
    {
        AgentOrder.Add(AgentChain[i], i);
    }
    
    // Check if registered systems follow the agent chain order
    int32 LastAgentOrder = -1;
    
    for (const FAgentSystemInfo& SystemInfo : RegisteredSystems)
    {
        if (AgentOrder.Contains(SystemInfo.AgentNumber))
        {
            int32 CurrentAgentOrder = AgentOrder[SystemInfo.AgentNumber];
            if (CurrentAgentOrder < LastAgentOrder)
            {
                UE_LOG(LogSystemIntegration, Warning, TEXT("SystemIntegrationManager: Agent sequence violation - %s should come before previous agent"), *SystemInfo.AgentNumber);
                return false;
            }
            LastAgentOrder = CurrentAgentOrder;
        }
    }
    
    return true;
}

void USystemIntegrationManager::LogAgentChainStatus()
{
    UE_LOG(LogSystemIntegration, Log, TEXT("SystemIntegrationManager: Agent Chain Status:"));
    
    for (int32 i = 0; i < AgentChain.Num(); i++)
    {
        const FString& AgentNumber = AgentChain[i];
        
        // Count systems for this agent
        int32 SystemCount = 0;
        int32 InitializedCount = 0;
        
        for (const FAgentSystemInfo& SystemInfo : RegisteredSystems)
        {
            if (SystemInfo.AgentNumber == AgentNumber)
            {
                SystemCount++;
                if (SystemInfo.bIsInitialized)
                {
                    InitializedCount++;
                }
            }
        }
        
        FString Status = TEXT("NOT REGISTERED");
        if (SystemCount > 0)
        {
            Status = FString::Printf(TEXT("%d/%d SYSTEMS INITIALIZED"), InitializedCount, SystemCount);
        }
        
        UE_LOG(LogSystemIntegration, Log, TEXT("  %02d. %s: %s"), i + 1, *AgentNumber, *Status);
    }
}