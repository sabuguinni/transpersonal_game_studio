#include "EngArch_ModuleManager.h"
#include "Engine/Engine.h"
#include "Misc/DateTime.h"

UEngArch_ModuleManager::UEngArch_ModuleManager()
{
    // Initialize agent states - all 19 agents start as ready
    for (int32 i = 1; i <= 19; ++i)
    {
        AgentStates.Add(i, true);
        BlockedReasons.Add(i, TEXT(""));
    }
}

void UEngArch_ModuleManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect Module Manager Initialized"));
    
    // Register core modules in dependency order
    RegisterModule(TEXT("Core"), {});
    RegisterModule(TEXT("Physics"), {TEXT("Core")});
    RegisterModule(TEXT("WorldGeneration"), {TEXT("Core"), TEXT("Physics")});
    RegisterModule(TEXT("Environment"), {TEXT("WorldGeneration")});
    RegisterModule(TEXT("Character"), {TEXT("Core"), TEXT("Physics")});
    RegisterModule(TEXT("Animation"), {TEXT("Character")});
    RegisterModule(TEXT("AI"), {TEXT("Character"), TEXT("Animation")});
    RegisterModule(TEXT("Combat"), {TEXT("AI"), TEXT("Physics")});
    RegisterModule(TEXT("Crowd"), {TEXT("AI")});
    RegisterModule(TEXT("Quest"), {TEXT("AI"), TEXT("Character")});
    RegisterModule(TEXT("Narrative"), {TEXT("Quest")});
    RegisterModule(TEXT("Audio"), {TEXT("Core")});
    RegisterModule(TEXT("VFX"), {TEXT("Core")});
    RegisterModule(TEXT("UI"), {TEXT("Character"), TEXT("Quest")});
}

void UEngArch_ModuleManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect Module Manager Deinitialized"));
    RegisteredModules.Empty();
    AgentStates.Empty();
    BlockedReasons.Empty();
    Super::Deinitialize();
}

bool UEngArch_ModuleManager::RegisterModule(const FString& ModuleName, const TArray<FString>& Dependencies)
{
    if (RegisteredModules.Contains(ModuleName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Module %s already registered"), *ModuleName);
        return false;
    }

    // Check for circular dependencies
    TSet<FString> Visited;
    if (CheckCircularDependencies(ModuleName, Visited))
    {
        UE_LOG(LogTemp, Error, TEXT("Circular dependency detected for module %s"), *ModuleName);
        return false;
    }

    FEng_ModuleInfo ModuleInfo;
    ModuleInfo.ModuleName = ModuleName;
    ModuleInfo.Dependencies = Dependencies;
    ModuleInfo.State = EEng_ModuleState::Unloaded;
    ModuleInfo.LoadTime = 0.0f;

    RegisteredModules.Add(ModuleName, ModuleInfo);
    UE_LOG(LogTemp, Log, TEXT("Registered module: %s with %d dependencies"), *ModuleName, Dependencies.Num());
    
    return true;
}

bool UEngArch_ModuleManager::LoadModule(const FString& ModuleName)
{
    if (!RegisteredModules.Contains(ModuleName))
    {
        UE_LOG(LogTemp, Error, TEXT("Module %s not registered"), *ModuleName);
        return false;
    }

    FEng_ModuleInfo& ModuleInfo = RegisteredModules[ModuleName];
    
    if (ModuleInfo.State == EEng_ModuleState::Loaded)
    {
        return true; // Already loaded
    }

    // Validate dependencies are loaded
    if (!ValidateDependencies(ModuleName))
    {
        UpdateModuleState(ModuleName, EEng_ModuleState::Failed, TEXT("Dependencies not met"));
        return false;
    }

    // Simulate loading time
    double StartTime = FPlatformTime::Seconds();
    UpdateModuleState(ModuleName, EEng_ModuleState::Loading);

    // Actual module loading would happen here
    // For now, just mark as loaded
    UpdateModuleState(ModuleName, EEng_ModuleState::Loaded);
    
    double EndTime = FPlatformTime::Seconds();
    ModuleInfo.LoadTime = EndTime - StartTime;

    UE_LOG(LogTemp, Log, TEXT("Module %s loaded in %.3f seconds"), *ModuleName, ModuleInfo.LoadTime);
    return true;
}

bool UEngArch_ModuleManager::UnloadModule(const FString& ModuleName)
{
    if (!RegisteredModules.Contains(ModuleName))
    {
        return false;
    }

    UpdateModuleState(ModuleName, EEng_ModuleState::Unloaded);
    UE_LOG(LogTemp, Log, TEXT("Module %s unloaded"), *ModuleName);
    return true;
}

EEng_ModuleState UEngArch_ModuleManager::GetModuleState(const FString& ModuleName) const
{
    if (const FEng_ModuleInfo* ModuleInfo = RegisteredModules.Find(ModuleName))
    {
        return ModuleInfo->State;
    }
    return EEng_ModuleState::Unloaded;
}

TArray<FEng_ModuleInfo> UEngArch_ModuleManager::GetAllModules() const
{
    TArray<FEng_ModuleInfo> Modules;
    for (const auto& Pair : RegisteredModules)
    {
        Modules.Add(Pair.Value);
    }
    return Modules;
}

bool UEngArch_ModuleManager::ValidateDependencies(const FString& ModuleName) const
{
    const FEng_ModuleInfo* ModuleInfo = RegisteredModules.Find(ModuleName);
    if (!ModuleInfo)
    {
        return false;
    }

    for (const FString& Dependency : ModuleInfo->Dependencies)
    {
        const FEng_ModuleInfo* DepInfo = RegisteredModules.Find(Dependency);
        if (!DepInfo || DepInfo->State != EEng_ModuleState::Loaded)
        {
            UE_LOG(LogTemp, Warning, TEXT("Dependency %s not loaded for module %s"), *Dependency, *ModuleName);
            return false;
        }
    }

    return true;
}

TArray<FString> UEngArch_ModuleManager::GetLoadOrder() const
{
    return TopologicalSort();
}

bool UEngArch_ModuleManager::ValidateCompilation()
{
    // Check if all registered modules can be found
    bool bAllValid = true;
    
    for (const auto& Pair : RegisteredModules)
    {
        const FString& ModuleName = Pair.Key;
        // Try to load a test class from each module
        FString TestClassName = FString::Printf(TEXT("/Script/TranspersonalGame.%sManager"), *ModuleName);
        
        try
        {
            UClass* TestClass = LoadClass<UObject>(nullptr, *TestClassName);
            if (!TestClass)
            {
                UE_LOG(LogTemp, Warning, TEXT("Module %s validation failed - no test class found"), *ModuleName);
                bAllValid = false;
            }
        }
        catch (...)
        {
            UE_LOG(LogTemp, Error, TEXT("Module %s compilation error"), *ModuleName);
            bAllValid = false;
        }
    }

    return bAllValid;
}

void UEngArch_ModuleManager::ForceRecompile()
{
    UE_LOG(LogTemp, Warning, TEXT("Force recompile requested - this would trigger UBT rebuild"));
    // In a real implementation, this would trigger Unreal Build Tool
}

bool UEngArch_ModuleManager::CanAgentProceed(int32 AgentID) const
{
    if (const bool* State = AgentStates.Find(AgentID))
    {
        return *State;
    }
    return false;
}

void UEngArch_ModuleManager::SetAgentBlocked(int32 AgentID, const FString& Reason)
{
    AgentStates.Add(AgentID, false);
    BlockedReasons.Add(AgentID, Reason);
    UE_LOG(LogTemp, Warning, TEXT("Agent #%d blocked: %s"), AgentID, *Reason);
}

bool UEngArch_ModuleManager::CheckCircularDependencies(const FString& ModuleName, TSet<FString>& Visited) const
{
    if (Visited.Contains(ModuleName))
    {
        return true; // Circular dependency found
    }

    const FEng_ModuleInfo* ModuleInfo = RegisteredModules.Find(ModuleName);
    if (!ModuleInfo)
    {
        return false;
    }

    Visited.Add(ModuleName);

    for (const FString& Dependency : ModuleInfo->Dependencies)
    {
        if (CheckCircularDependencies(Dependency, Visited))
        {
            return true;
        }
    }

    Visited.Remove(ModuleName);
    return false;
}

TArray<FString> UEngArch_ModuleManager::TopologicalSort() const
{
    TArray<FString> Result;
    TSet<FString> Visited;
    TSet<FString> Visiting;

    for (const auto& Pair : RegisteredModules)
    {
        if (!Visited.Contains(Pair.Key))
        {
            TopologicalSortHelper(Pair.Key, Visited, Visiting, Result);
        }
    }

    return Result;
}

void UEngArch_ModuleManager::TopologicalSortHelper(const FString& ModuleName, TSet<FString>& Visited, TSet<FString>& Visiting, TArray<FString>& Result) const
{
    if (Visiting.Contains(ModuleName))
    {
        return; // Circular dependency - skip
    }

    if (Visited.Contains(ModuleName))
    {
        return; // Already processed
    }

    Visiting.Add(ModuleName);

    const FEng_ModuleInfo* ModuleInfo = RegisteredModules.Find(ModuleName);
    if (ModuleInfo)
    {
        for (const FString& Dependency : ModuleInfo->Dependencies)
        {
            TopologicalSortHelper(Dependency, Visited, Visiting, Result);
        }
    }

    Visiting.Remove(ModuleName);
    Visited.Add(ModuleName);
    Result.Add(ModuleName);
}

void UEngArch_ModuleManager::UpdateModuleState(const FString& ModuleName, EEng_ModuleState NewState, const FString& Error)
{
    if (FEng_ModuleInfo* ModuleInfo = RegisteredModules.Find(ModuleName))
    {
        ModuleInfo->State = NewState;
        ModuleInfo->ErrorMessage = Error;
    }
}