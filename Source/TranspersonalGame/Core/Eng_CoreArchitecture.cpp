#include "Eng_CoreArchitecture.h"
#include "Eng_MasterArchitect.h"
#include "Eng_SystemArchitect.h"
#include "Eng_GameModeArchitect.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"

UEng_CoreArchitecture::UEng_CoreArchitecture()
{
    bArchitectureInitialized = false;
    bDependenciesResolved = false;
    TotalInitializationTime = 0.0f;
    LastValidationTime = FDateTime::Now();
    
    MasterArchitect = nullptr;
    SystemArchitect = nullptr;
    GameModeArchitect = nullptr;
}

void UEng_CoreArchitecture::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("UEng_CoreArchitecture::Initialize - Starting core architecture initialization"));
    
    // Initialize the core architectural components
    InitializeArchitecture();
}

void UEng_CoreArchitecture::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("UEng_CoreArchitecture::Deinitialize - Shutting down architecture systems"));
    
    // Shutdown all registered systems
    for (auto& SystemPair : RegisteredSystems)
    {
        if (SystemPair.Value && SystemPair.Value->GetClass()->ImplementsInterface(UEng_ArchitecturalSystem::StaticClass()))
        {
            IEng_ArchitecturalSystem::Execute_ShutdownSystem(SystemPair.Value);
        }
    }
    
    RegisteredSystems.Empty();
    SystemDependencies.Empty();
    SystemInitTimes.Empty();
    ArchitecturalViolations.Empty();
    
    bArchitectureInitialized = false;
    bDependenciesResolved = false;
    
    Super::Deinitialize();
}

bool UEng_CoreArchitecture::InitializeArchitecture()
{
    UE_LOG(LogTemp, Warning, TEXT("UEng_CoreArchitecture::InitializeArchitecture - Beginning architecture setup"));
    
    if (bArchitectureInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("Architecture already initialized"));
        return true;
    }
    
    float StartTime = FPlatformTime::Seconds();
    
    // Initialize core architectural components in dependency order
    InitializeMasterArchitect();
    InitializeSystemArchitect();
    InitializeGameModeArchitect();
    
    // Resolve system dependencies
    if (!ResolveDependencies())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to resolve system dependencies"));
        return false;
    }
    
    // Validate architectural compliance
    if (!ValidateArchitecturalCompliance())
    {
        UE_LOG(LogTemp, Error, TEXT("Architecture validation failed"));
        return false;
    }
    
    TotalInitializationTime = FPlatformTime::Seconds() - StartTime;
    bArchitectureInitialized = true;
    
    UE_LOG(LogTemp, Warning, TEXT("Core Architecture initialized successfully in %.3f seconds"), TotalInitializationTime);
    return true;
}

void UEng_CoreArchitecture::InitializeMasterArchitect()
{
    StartSystemTimer(TEXT("MasterArchitect"));
    
    MasterArchitect = NewObject<UEng_MasterArchitect>(this);
    if (MasterArchitect)
    {
        RegisterArchitecturalSystem(TEXT("MasterArchitect"), MasterArchitect);
        UE_LOG(LogTemp, Log, TEXT("MasterArchitect initialized"));
    }
    
    EndSystemTimer(TEXT("MasterArchitect"));
}

void UEng_CoreArchitecture::InitializeSystemArchitect()
{
    StartSystemTimer(TEXT("SystemArchitect"));
    
    SystemArchitect = NewObject<UEng_SystemArchitect>(this);
    if (SystemArchitect)
    {
        RegisterArchitecturalSystem(TEXT("SystemArchitect"), SystemArchitect);
        AddSystemDependency(TEXT("SystemArchitect"), TEXT("MasterArchitect"));
        UE_LOG(LogTemp, Log, TEXT("SystemArchitect initialized"));
    }
    
    EndSystemTimer(TEXT("SystemArchitect"));
}

void UEng_CoreArchitecture::InitializeGameModeArchitect()
{
    StartSystemTimer(TEXT("GameModeArchitect"));
    
    GameModeArchitect = NewObject<UEng_GameModeArchitect>(this);
    if (GameModeArchitect)
    {
        RegisterArchitecturalSystem(TEXT("GameModeArchitect"), GameModeArchitect);
        AddSystemDependency(TEXT("GameModeArchitect"), TEXT("SystemArchitect"));
        UE_LOG(LogTemp, Log, TEXT("GameModeArchitect initialized"));
    }
    
    EndSystemTimer(TEXT("GameModeArchitect"));
}

bool UEng_CoreArchitecture::ValidateArchitecturalCompliance()
{
    UE_LOG(LogTemp, Log, TEXT("UEng_CoreArchitecture::ValidateArchitecturalCompliance - Starting validation"));
    
    ArchitecturalViolations.Empty();
    
    // Validate all registered systems
    for (auto& SystemPair : RegisteredSystems)
    {
        ValidateSystemCompliance(SystemPair.Key, SystemPair.Value);
    }
    
    // Check architectural standards
    CheckArchitecturalStandards();
    
    LastValidationTime = FDateTime::Now();
    
    bool bIsCompliant = ArchitecturalViolations.Num() == 0;
    UE_LOG(LogTemp, Warning, TEXT("Architecture validation complete. Violations found: %d"), ArchitecturalViolations.Num());
    
    return bIsCompliant;
}

void UEng_CoreArchitecture::RegisterArchitecturalSystem(const FString& SystemName, UObject* SystemInstance)
{
    if (!SystemInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot register null system: %s"), *SystemName);
        return;
    }
    
    RegisteredSystems.Add(SystemName, SystemInstance);
    UE_LOG(LogTemp, Log, TEXT("Registered architectural system: %s"), *SystemName);
}

bool UEng_CoreArchitecture::IsSystemRegistered(const FString& SystemName) const
{
    return RegisteredSystems.Contains(SystemName);
}

bool UEng_CoreArchitecture::ResolveDependencies()
{
    UE_LOG(LogTemp, Log, TEXT("UEng_CoreArchitecture::ResolveDependencies - Resolving system dependencies"));
    
    if (!ValidateDependencyGraph())
    {
        UE_LOG(LogTemp, Error, TEXT("Dependency graph validation failed"));
        return false;
    }
    
    TArray<FString> InitOrder = GetInitializationOrder();
    
    // Initialize systems in dependency order
    for (const FString& SystemName : InitOrder)
    {
        if (UObject** SystemPtr = RegisteredSystems.Find(SystemName))
        {
            UObject* System = *SystemPtr;
            if (System && System->GetClass()->ImplementsInterface(UEng_ArchitecturalSystem::StaticClass()))
            {
                StartSystemTimer(SystemName);
                bool bInitSuccess = IEng_ArchitecturalSystem::Execute_InitializeSystem(System);
                EndSystemTimer(SystemName);
                
                if (!bInitSuccess)
                {
                    UE_LOG(LogTemp, Error, TEXT("Failed to initialize system: %s"), *SystemName);
                    return false;
                }
            }
        }
    }
    
    bDependenciesResolved = true;
    UE_LOG(LogTemp, Warning, TEXT("System dependencies resolved successfully"));
    return true;
}

void UEng_CoreArchitecture::AddSystemDependency(const FString& SystemName, const FString& DependsOn)
{
    if (!SystemDependencies.Contains(SystemName))
    {
        SystemDependencies.Add(SystemName, TArray<FString>());
    }
    
    SystemDependencies[SystemName].AddUnique(DependsOn);
    UE_LOG(LogTemp, Log, TEXT("Added dependency: %s depends on %s"), *SystemName, *DependsOn);
}

TArray<FString> UEng_CoreArchitecture::GetArchitecturalViolations() const
{
    return ArchitecturalViolations;
}

bool UEng_CoreArchitecture::EnforceArchitecturalStandards()
{
    UE_LOG(LogTemp, Log, TEXT("UEng_CoreArchitecture::EnforceArchitecturalStandards - Enforcing standards"));
    
    bool bStandardsEnforced = true;
    
    // Check system naming conventions
    for (auto& SystemPair : RegisteredSystems)
    {
        if (!SystemPair.Key.StartsWith(TEXT("Eng_")) && !SystemPair.Key.StartsWith(TEXT("Dir_")))
        {
            ArchitecturalViolations.Add(FString::Printf(TEXT("System %s does not follow naming convention"), *SystemPair.Key));
            bStandardsEnforced = false;
        }
    }
    
    // Check dependency compliance
    if (!bDependenciesResolved)
    {
        ArchitecturalViolations.Add(TEXT("System dependencies not properly resolved"));
        bStandardsEnforced = false;
    }
    
    return bStandardsEnforced;
}

float UEng_CoreArchitecture::GetSystemInitializationTime(const FString& SystemName) const
{
    if (const float* InitTime = SystemInitTimes.Find(SystemName))
    {
        return *InitTime;
    }
    return 0.0f;
}

FString UEng_CoreArchitecture::GetArchitecturalHealthReport() const
{
    FString Report;
    Report += FString::Printf(TEXT("=== ARCHITECTURAL HEALTH REPORT ===\n"));
    Report += FString::Printf(TEXT("Architecture Initialized: %s\n"), bArchitectureInitialized ? TEXT("YES") : TEXT("NO"));
    Report += FString::Printf(TEXT("Dependencies Resolved: %s\n"), bDependenciesResolved ? TEXT("YES") : TEXT("NO"));
    Report += FString::Printf(TEXT("Registered Systems: %d\n"), RegisteredSystems.Num());
    Report += FString::Printf(TEXT("Total Init Time: %.3f seconds\n"), TotalInitializationTime);
    Report += FString::Printf(TEXT("Violations: %d\n"), ArchitecturalViolations.Num());
    
    if (ArchitecturalViolations.Num() > 0)
    {
        Report += TEXT("\nVIOLATIONS:\n");
        for (const FString& Violation : ArchitecturalViolations)
        {
            Report += FString::Printf(TEXT("- %s\n"), *Violation);
        }
    }
    
    return Report;
}

bool UEng_CoreArchitecture::ValidateDependencyGraph()
{
    // Simple cycle detection - more sophisticated algorithms can be added later
    TSet<FString> Visited;
    TSet<FString> InStack;
    
    for (auto& SystemPair : SystemDependencies)
    {
        if (!Visited.Contains(SystemPair.Key))
        {
            // For now, assume no cycles - implement proper DFS cycle detection if needed
        }
    }
    
    return true;
}

TArray<FString> UEng_CoreArchitecture::GetInitializationOrder()
{
    TArray<FString> InitOrder;
    
    // Simple topological sort - start with systems that have no dependencies
    TSet<FString> Processed;
    
    // Add systems with no dependencies first
    for (auto& SystemPair : RegisteredSystems)
    {
        if (!SystemDependencies.Contains(SystemPair.Key) || SystemDependencies[SystemPair.Key].Num() == 0)
        {
            InitOrder.Add(SystemPair.Key);
            Processed.Add(SystemPair.Key);
        }
    }
    
    // Add remaining systems (simplified - proper topological sort would be better)
    for (auto& SystemPair : RegisteredSystems)
    {
        if (!Processed.Contains(SystemPair.Key))
        {
            InitOrder.Add(SystemPair.Key);
        }
    }
    
    return InitOrder;
}

void UEng_CoreArchitecture::ValidateSystemCompliance(const FString& SystemName, UObject* System)
{
    if (!System)
    {
        ArchitecturalViolations.Add(FString::Printf(TEXT("System %s is null"), *SystemName));
        return;
    }
    
    // Check if system implements architectural interface
    if (!System->GetClass()->ImplementsInterface(UEng_ArchitecturalSystem::StaticClass()))
    {
        ArchitecturalViolations.Add(FString::Printf(TEXT("System %s does not implement IEng_ArchitecturalSystem"), *SystemName));
    }
}

void UEng_CoreArchitecture::CheckArchitecturalStandards()
{
    // Verify core systems are present
    if (!IsSystemRegistered(TEXT("MasterArchitect")))
    {
        ArchitecturalViolations.Add(TEXT("MasterArchitect system not registered"));
    }
    
    if (!IsSystemRegistered(TEXT("SystemArchitect")))
    {
        ArchitecturalViolations.Add(TEXT("SystemArchitect system not registered"));
    }
    
    if (!IsSystemRegistered(TEXT("GameModeArchitect")))
    {
        ArchitecturalViolations.Add(TEXT("GameModeArchitect system not registered"));
    }
}

void UEng_CoreArchitecture::StartSystemTimer(const FString& SystemName)
{
    SystemInitTimes.Add(SystemName, FPlatformTime::Seconds());
}

void UEng_CoreArchitecture::EndSystemTimer(const FString& SystemName)
{
    if (float* StartTime = SystemInitTimes.Find(SystemName))
    {
        *StartTime = FPlatformTime::Seconds() - *StartTime;
    }
}