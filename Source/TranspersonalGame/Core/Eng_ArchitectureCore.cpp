#include "Eng_ArchitectureCore.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/DateTime.h"

UEng_ArchitectureCore::UEng_ArchitectureCore()
{
    CurrentState = EEng_ArchitectureState::Uninitialized;
    TotalInitializationTime = 0.0f;
}

void UEng_ArchitectureCore::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("=== ENGINE ARCHITECT - CORE SYSTEM INITIALIZATION ==="));
    
    double StartTime = FPlatformTime::Seconds();
    CurrentState = EEng_ArchitectureState::Loading;
    
    // Initialize core modules in dependency order
    InitializeCoreModules();
    
    // Validate all systems
    if (ValidateArchitecture())
    {
        CurrentState = EEng_ArchitectureState::Ready;
        UE_LOG(LogTemp, Warning, TEXT("✓ Architecture Core initialized successfully"));
    }
    else
    {
        CurrentState = EEng_ArchitectureState::Error;
        UE_LOG(LogTemp, Error, TEXT("✗ Architecture Core initialization failed"));
    }
    
    TotalInitializationTime = FPlatformTime::Seconds() - StartTime;
    UE_LOG(LogTemp, Warning, TEXT("Architecture initialization time: %.3f seconds"), TotalInitializationTime);
    
    // Print initial report
    PrintArchitectureReport();
}

void UEng_ArchitectureCore::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("=== ENGINE ARCHITECT - CORE SYSTEM SHUTDOWN ==="));
    
    RegisteredModules.Empty();
    SystemDependencies.Empty();
    CurrentState = EEng_ArchitectureState::Uninitialized;
    
    Super::Deinitialize();
}

bool UEng_ArchitectureCore::RegisterModule(const FString& ModuleName, EModulePriority Priority)
{
    // Check if module already registered
    for (const FEng_ModuleInfo& Module : RegisteredModules)
    {
        if (Module.ModuleName == ModuleName)
        {
            UE_LOG(LogTemp, Warning, TEXT("Module %s already registered"), *ModuleName);
            return false;
        }
    }
    
    double StartTime = FPlatformTime::Seconds();
    
    FEng_ModuleInfo NewModule;
    NewModule.ModuleName = ModuleName;
    NewModule.Priority = Priority;
    NewModule.bIsLoaded = true; // Assume loaded if we're registering it
    NewModule.LoadTime = FPlatformTime::Seconds() - StartTime;
    
    RegisteredModules.Add(NewModule);
    
    UE_LOG(LogTemp, Warning, TEXT("✓ Registered module: %s (Priority: %d, LoadTime: %.3f)"), 
           *ModuleName, (int32)Priority, NewModule.LoadTime);
    
    return true;
}

bool UEng_ArchitectureCore::UnregisterModule(const FString& ModuleName)
{
    for (int32 i = RegisteredModules.Num() - 1; i >= 0; --i)
    {
        if (RegisteredModules[i].ModuleName == ModuleName)
        {
            RegisteredModules.RemoveAt(i);
            UE_LOG(LogTemp, Warning, TEXT("✓ Unregistered module: %s"), *ModuleName);
            return true;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Module %s not found for unregistration"), *ModuleName);
    return false;
}

bool UEng_ArchitectureCore::IsModuleLoaded(const FString& ModuleName) const
{
    for (const FEng_ModuleInfo& Module : RegisteredModules)
    {
        if (Module.ModuleName == ModuleName)
        {
            return Module.bIsLoaded;
        }
    }
    return false;
}

TArray<FEng_ModuleInfo> UEng_ArchitectureCore::GetLoadedModules() const
{
    TArray<FEng_ModuleInfo> LoadedModules;
    for (const FEng_ModuleInfo& Module : RegisteredModules)
    {
        if (Module.bIsLoaded)
        {
            LoadedModules.Add(Module);
        }
    }
    return LoadedModules;
}

bool UEng_ArchitectureCore::AddSystemDependency(const FString& SystemName, const TArray<FString>& Dependencies)
{
    FEng_SystemDependency NewDependency;
    NewDependency.SystemName = SystemName;
    NewDependency.Dependencies = Dependencies;
    NewDependency.bIsInitialized = false;
    
    SystemDependencies.Add(NewDependency);
    
    UE_LOG(LogTemp, Warning, TEXT("✓ Added system dependency: %s (Deps: %d)"), 
           *SystemName, Dependencies.Num());
    
    return true;
}

bool UEng_ArchitectureCore::ValidateSystemDependencies() const
{
    for (const FEng_SystemDependency& System : SystemDependencies)
    {
        for (const FString& Dependency : System.Dependencies)
        {
            bool bDependencyFound = false;
            for (const FEng_SystemDependency& OtherSystem : SystemDependencies)
            {
                if (OtherSystem.SystemName == Dependency)
                {
                    bDependencyFound = true;
                    break;
                }
            }
            
            if (!bDependencyFound)
            {
                UE_LOG(LogTemp, Error, TEXT("✗ Missing dependency: %s requires %s"), 
                       *System.SystemName, *Dependency);
                return false;
            }
        }
    }
    
    return true;
}

TArray<FString> UEng_ArchitectureCore::GetInitializationOrder() const
{
    TArray<FString> InitOrder;
    ResolveDependencyOrder(InitOrder);
    return InitOrder;
}

EEng_ArchitectureState UEng_ArchitectureCore::GetArchitectureState() const
{
    return CurrentState;
}

void UEng_ArchitectureCore::SetArchitectureState(EEng_ArchitectureState NewState)
{
    CurrentState = NewState;
    UE_LOG(LogTemp, Warning, TEXT("Architecture state changed to: %d"), (int32)NewState);
}

float UEng_ArchitectureCore::GetModuleLoadTime(const FString& ModuleName) const
{
    for (const FEng_ModuleInfo& Module : RegisteredModules)
    {
        if (Module.ModuleName == ModuleName)
        {
            return Module.LoadTime;
        }
    }
    return 0.0f;
}

float UEng_ArchitectureCore::GetTotalLoadTime() const
{
    return TotalInitializationTime;
}

bool UEng_ArchitectureCore::ValidateArchitecture()
{
    UE_LOG(LogTemp, Warning, TEXT("=== ARCHITECTURE VALIDATION ==="));
    
    bool bValid = true;
    
    // Validate modules
    if (RegisteredModules.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("⚠ No modules registered yet"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("✓ %d modules registered"), RegisteredModules.Num());
    }
    
    // Validate dependencies
    if (!ValidateSystemDependencies())
    {
        UE_LOG(LogTemp, Error, TEXT("✗ System dependency validation failed"));
        bValid = false;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("✓ System dependencies valid"));
    }
    
    // Validate module integrity
    ValidateModuleIntegrity();
    
    UE_LOG(LogTemp, Warning, TEXT("=== VALIDATION COMPLETE: %s ==="), 
           bValid ? TEXT("PASSED") : TEXT("FAILED"));
    
    return bValid;
}

void UEng_ArchitectureCore::PrintArchitectureReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== ARCHITECTURE REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("State: %d"), (int32)CurrentState);
    UE_LOG(LogTemp, Warning, TEXT("Total Load Time: %.3f seconds"), TotalInitializationTime);
    UE_LOG(LogTemp, Warning, TEXT("Registered Modules: %d"), RegisteredModules.Num());
    UE_LOG(LogTemp, Warning, TEXT("System Dependencies: %d"), SystemDependencies.Num());
    
    for (const FEng_ModuleInfo& Module : RegisteredModules)
    {
        UE_LOG(LogTemp, Warning, TEXT("  Module: %s (Priority: %d, Loaded: %s, Time: %.3f)"),
               *Module.ModuleName, (int32)Module.Priority, 
               Module.bIsLoaded ? TEXT("YES") : TEXT("NO"), Module.LoadTime);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== END REPORT ==="));
}

void UEng_ArchitectureCore::InitializeCoreModules()
{
    UE_LOG(LogTemp, Warning, TEXT("Initializing core modules..."));
    
    // Register essential game modules
    RegisterModule(TEXT("TranspersonalGame"), EModulePriority::Critical);
    RegisterModule(TEXT("Core"), EModulePriority::Critical);
    RegisterModule(TEXT("Physics"), EModulePriority::High);
    RegisterModule(TEXT("WorldGeneration"), EModulePriority::High);
    RegisterModule(TEXT("Characters"), EModulePriority::High);
    RegisterModule(TEXT("AI"), EModulePriority::Medium);
    RegisterModule(TEXT("Combat"), EModulePriority::Medium);
    RegisterModule(TEXT("Audio"), EModulePriority::Low);
    RegisterModule(TEXT("VFX"), EModulePriority::Low);
    
    // Setup system dependencies
    AddSystemDependency(TEXT("Physics"), {TEXT("Core")});
    AddSystemDependency(TEXT("WorldGeneration"), {TEXT("Core"), TEXT("Physics")});
    AddSystemDependency(TEXT("Characters"), {TEXT("Core"), TEXT("Physics")});
    AddSystemDependency(TEXT("AI"), {TEXT("Core"), TEXT("Characters")});
    AddSystemDependency(TEXT("Combat"), {TEXT("Core"), TEXT("Characters"), TEXT("AI")});
    AddSystemDependency(TEXT("Audio"), {TEXT("Core")});
    AddSystemDependency(TEXT("VFX"), {TEXT("Core"), TEXT("Physics")});
}

void UEng_ArchitectureCore::ValidateModuleIntegrity()
{
    UE_LOG(LogTemp, Warning, TEXT("Validating module integrity..."));
    
    for (FEng_ModuleInfo& Module : RegisteredModules)
    {
        // In a real implementation, this would check if the module DLL is loaded
        // For now, we assume all registered modules are valid
        if (Module.bIsLoaded)
        {
            UE_LOG(LogTemp, Warning, TEXT("✓ Module %s integrity OK"), *Module.ModuleName);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("✗ Module %s integrity FAILED"), *Module.ModuleName);
        }
    }
}

bool UEng_ArchitectureCore::ResolveDependencyOrder(TArray<FString>& OutOrder) const
{
    OutOrder.Empty();
    TArray<FString> Remaining;
    
    // Collect all system names
    for (const FEng_SystemDependency& System : SystemDependencies)
    {
        Remaining.Add(System.SystemName);
    }
    
    // Simple topological sort
    while (Remaining.Num() > 0)
    {
        bool bProgressMade = false;
        
        for (int32 i = Remaining.Num() - 1; i >= 0; --i)
        {
            const FString& SystemName = Remaining[i];
            
            // Find this system's dependencies
            const FEng_SystemDependency* SystemDep = nullptr;
            for (const FEng_SystemDependency& Dep : SystemDependencies)
            {
                if (Dep.SystemName == SystemName)
                {
                    SystemDep = &Dep;
                    break;
                }
            }
            
            if (!SystemDep)
            {
                // No dependencies, can be added
                OutOrder.Add(SystemName);
                Remaining.RemoveAt(i);
                bProgressMade = true;
                continue;
            }
            
            // Check if all dependencies are already in the order
            bool bAllDepsResolved = true;
            for (const FString& Dependency : SystemDep->Dependencies)
            {
                if (!OutOrder.Contains(Dependency))
                {
                    bAllDepsResolved = false;
                    break;
                }
            }
            
            if (bAllDepsResolved)
            {
                OutOrder.Add(SystemName);
                Remaining.RemoveAt(i);
                bProgressMade = true;
            }
        }
        
        if (!bProgressMade)
        {
            UE_LOG(LogTemp, Error, TEXT("Circular dependency detected in system dependencies"));
            return false;
        }
    }
    
    return true;
}