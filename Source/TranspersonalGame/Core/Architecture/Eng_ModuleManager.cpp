#include "Eng_ModuleManager.h"
#include "Engine/Engine.h"

void UEng_ModuleManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("ModuleManager: Initializing module management system"));
    
    // Register core game modules
    RegisterModule(TEXT("TranspersonalGame"));
    RegisterModule(TEXT("WorldGeneration"), {TEXT("TranspersonalGame")});
    RegisterModule(TEXT("DinosaurAI"), {TEXT("TranspersonalGame"), TEXT("WorldGeneration")});
    RegisterModule(TEXT("CombatSystem"), {TEXT("TranspersonalGame"), TEXT("DinosaurAI")});
    RegisterModule(TEXT("QuestSystem"), {TEXT("TranspersonalGame")});
    RegisterModule(TEXT("AudioSystem"), {TEXT("TranspersonalGame")});
    
    LogModuleStatus();
}

void UEng_ModuleManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("ModuleManager: Shutting down - %d modules registered"), Modules.Num());
    
    // Unload all modules in reverse dependency order
    TArray<FString> ModuleNames;
    Modules.GetKeys(ModuleNames);
    
    for (const FString& ModuleName : ModuleNames)
    {
        UnloadModule(ModuleName);
    }
    
    Modules.Empty();
    Super::Deinitialize();
}

void UEng_ModuleManager::RegisterModule(const FString& ModuleName, const TArray<FString>& Dependencies)
{
    if (Modules.Contains(ModuleName))
    {
        UE_LOG(LogTemp, Warning, TEXT("ModuleManager: Module %s already registered"), *ModuleName);
        return;
    }

    FEng_ModuleInfo NewModule;
    NewModule.ModuleName = ModuleName;
    NewModule.Dependencies = Dependencies;
    NewModule.State = EEng_ModuleState::Unloaded;
    NewModule.LoadTime = 0.0f;

    Modules.Add(ModuleName, NewModule);
    UE_LOG(LogTemp, Log, TEXT("ModuleManager: Registered module %s with %d dependencies"), 
           *ModuleName, Dependencies.Num());
}

void UEng_ModuleManager::LoadModule(const FString& ModuleName)
{
    FEng_ModuleInfo* Module = Modules.Find(ModuleName);
    if (!Module)
    {
        UE_LOG(LogTemp, Error, TEXT("ModuleManager: Module %s not registered"), *ModuleName);
        return;
    }

    if (Module->State == EEng_ModuleState::Ready)
    {
        UE_LOG(LogTemp, Log, TEXT("ModuleManager: Module %s already loaded"), *ModuleName);
        return;
    }

    // Check dependencies
    if (!CheckDependencies(ModuleName))
    {
        SetModuleState(ModuleName, EEng_ModuleState::Error, TEXT("Dependency check failed"));
        return;
    }

    SetModuleState(ModuleName, EEng_ModuleState::Loading);
    
    // Simulate module loading time
    float StartTime = FPlatformTime::Seconds();
    
    // Mark as loaded (in real implementation, this would load actual module)
    SetModuleState(ModuleName, EEng_ModuleState::Loaded);
    SetModuleState(ModuleName, EEng_ModuleState::Initializing);
    SetModuleState(ModuleName, EEng_ModuleState::Ready);
    
    Module->LoadTime = FPlatformTime::Seconds() - StartTime;
    
    UE_LOG(LogTemp, Log, TEXT("ModuleManager: Module %s loaded successfully in %.3fs"), 
           *ModuleName, Module->LoadTime);
}

void UEng_ModuleManager::UnloadModule(const FString& ModuleName)
{
    if (FEng_ModuleInfo* Module = Modules.Find(ModuleName))
    {
        SetModuleState(ModuleName, EEng_ModuleState::Unloaded);
        UE_LOG(LogTemp, Log, TEXT("ModuleManager: Module %s unloaded"), *ModuleName);
    }
}

bool UEng_ModuleManager::IsModuleLoaded(const FString& ModuleName) const
{
    if (const FEng_ModuleInfo* Module = Modules.Find(ModuleName))
    {
        return Module->State == EEng_ModuleState::Ready;
    }
    return false;
}

EEng_ModuleState UEng_ModuleManager::GetModuleState(const FString& ModuleName) const
{
    if (const FEng_ModuleInfo* Module = Modules.Find(ModuleName))
    {
        return Module->State;
    }
    return EEng_ModuleState::Unloaded;
}

TArray<FEng_ModuleInfo> UEng_ModuleManager::GetAllModules() const
{
    TArray<FEng_ModuleInfo> ModuleList;
    for (const auto& Pair : Modules)
    {
        ModuleList.Add(Pair.Value);
    }
    return ModuleList;
}

void UEng_ModuleManager::ValidateModuleDependencies()
{
    int32 ReadyCount = 0;
    int32 ErrorCount = 0;
    
    for (const auto& Pair : Modules)
    {
        switch (Pair.Value.State)
        {
        case EEng_ModuleState::Ready:
            ReadyCount++;
            break;
        case EEng_ModuleState::Error:
            ErrorCount++;
            break;
        default:
            break;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("ModuleManager: %d ready, %d errors, %d total modules"), 
           ReadyCount, ErrorCount, Modules.Num());
}

void UEng_ModuleManager::SetModuleState(const FString& ModuleName, EEng_ModuleState NewState, const FString& ErrorMsg)
{
    if (FEng_ModuleInfo* Module = Modules.Find(ModuleName))
    {
        Module->State = NewState;
        if (!ErrorMsg.IsEmpty())
        {
            Module->ErrorMessage = ErrorMsg;
        }
    }
}

bool UEng_ModuleManager::CheckDependencies(const FString& ModuleName) const
{
    const FEng_ModuleInfo* Module = Modules.Find(ModuleName);
    if (!Module)
    {
        return false;
    }

    for (const FString& Dependency : Module->Dependencies)
    {
        if (!IsModuleLoaded(Dependency))
        {
            UE_LOG(LogTemp, Error, TEXT("ModuleManager: Dependency %s not loaded for module %s"), 
                   *Dependency, *ModuleName);
            return false;
        }
    }
    
    return true;
}

void UEng_ModuleManager::LogModuleStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("ModuleManager: Current module status:"));
    
    for (const auto& Pair : Modules)
    {
        const FEng_ModuleInfo& Module = Pair.Value;
        FString StateStr;
        
        switch (Module.State)
        {
        case EEng_ModuleState::Unloaded: StateStr = TEXT("UNLOADED"); break;
        case EEng_ModuleState::Loading: StateStr = TEXT("LOADING"); break;
        case EEng_ModuleState::Loaded: StateStr = TEXT("LOADED"); break;
        case EEng_ModuleState::Initializing: StateStr = TEXT("INITIALIZING"); break;
        case EEng_ModuleState::Ready: StateStr = TEXT("READY"); break;
        case EEng_ModuleState::Error: StateStr = FString::Printf(TEXT("ERROR: %s"), *Module.ErrorMessage); break;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("  %s: %s"), *Module.ModuleName, *StateStr);
    }
}