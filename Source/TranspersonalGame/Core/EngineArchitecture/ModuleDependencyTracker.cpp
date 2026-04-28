#include "ModuleDependencyTracker.h"
#include "Engine/Engine.h"

UModuleDependencyTracker::UModuleDependencyTracker()
{
    // Initialize with empty registry
}

void UModuleDependencyTracker::RegisterModule(const FString& ModuleName, const TArray<FString>& RequiredModules, int32 Priority)
{
    FEng_ModuleDependency NewModule;
    NewModule.ModuleName = ModuleName;
    NewModule.RequiredModules = RequiredModules;
    NewModule.InitializationPriority = Priority;
    NewModule.bIsInitialized = false;

    ModuleRegistry.Add(ModuleName, NewModule);
    ModuleStatus.Add(ModuleName, EEng_DependencyStatus::Unresolved);

    UE_LOG(LogTemp, Log, TEXT("Module registered: %s with %d dependencies"), *ModuleName, RequiredModules.Num());
}

void UModuleDependencyTracker::RegisterOptionalDependency(const FString& ModuleName, const FString& OptionalModule)
{
    if (FEng_ModuleDependency* Module = ModuleRegistry.Find(ModuleName))
    {
        Module->OptionalModules.AddUnique(OptionalModule);
        UE_LOG(LogTemp, Log, TEXT("Optional dependency added: %s -> %s"), *ModuleName, *OptionalModule);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot add optional dependency - module not found: %s"), *ModuleName);
    }
}

bool UModuleDependencyTracker::IsModuleRegistered(const FString& ModuleName) const
{
    return ModuleRegistry.Contains(ModuleName);
}

TArray<FString> UModuleDependencyTracker::GetInitializationOrder()
{
    TArray<FString> InitOrder;
    TSet<FString> Visiting;

    // Reset all statuses
    for (auto& StatusPair : ModuleStatus)
    {
        StatusPair.Value = EEng_DependencyStatus::Unresolved;
    }

    // Sort modules by priority first
    TArray<TPair<FString, FEng_ModuleDependency>> SortedModules;
    for (const auto& ModulePair : ModuleRegistry)
    {
        SortedModules.Add(TPair<FString, FEng_ModuleDependency>(ModulePair.Key, ModulePair.Value));
    }

    SortedModules.Sort([](const TPair<FString, FEng_ModuleDependency>& A, const TPair<FString, FEng_ModuleDependency>& B)
    {
        return A.Value.InitializationPriority > B.Value.InitializationPriority;
    });

    // Resolve dependencies for each module
    for (const auto& ModulePair : SortedModules)
    {
        if (ModuleStatus[ModulePair.Key] == EEng_DependencyStatus::Unresolved)
        {
            if (!ResolveDependenciesRecursive(ModulePair.Key, InitOrder, Visiting))
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to resolve dependencies for module: %s"), *ModulePair.Key);
                FailedModules.AddUnique(ModulePair.Key);
                ModuleStatus[ModulePair.Key] = EEng_DependencyStatus::Failed;
            }
        }
    }

    return InitOrder;
}

bool UModuleDependencyTracker::ValidateAllDependencies()
{
    bool bAllValid = true;

    for (const auto& ModulePair : ModuleRegistry)
    {
        const FString& ModuleName = ModulePair.Key;
        const FEng_ModuleDependency& Module = ModulePair.Value;

        // Check if all required dependencies are registered
        for (const FString& RequiredModule : Module.RequiredModules)
        {
            if (!IsModuleRegistered(RequiredModule))
            {
                UE_LOG(LogTemp, Error, TEXT("Module %s requires unregistered module: %s"), *ModuleName, *RequiredModule);
                bAllValid = false;
            }
        }
    }

    // Check for circular dependencies
    if (HasCircularDependencies())
    {
        UE_LOG(LogTemp, Error, TEXT("Circular dependencies detected"));
        bAllValid = false;
    }

    return bAllValid;
}

EEng_DependencyStatus UModuleDependencyTracker::GetModuleStatus(const FString& ModuleName) const
{
    if (const EEng_DependencyStatus* Status = ModuleStatus.Find(ModuleName))
    {
        return *Status;
    }
    return EEng_DependencyStatus::Unresolved;
}

bool UModuleDependencyTracker::HasCircularDependencies()
{
    CircularDependencyChain.Empty();
    DetectCircularDependencies();
    return CircularDependencyChain.Num() > 0;
}

TArray<FString> UModuleDependencyTracker::GetCircularDependencyChain()
{
    return CircularDependencyChain;
}

void UModuleDependencyTracker::MarkModuleInitialized(const FString& ModuleName)
{
    if (FEng_ModuleDependency* Module = ModuleRegistry.Find(ModuleName))
    {
        Module->bIsInitialized = true;
        ModuleStatus[ModuleName] = EEng_DependencyStatus::Resolved;
        UE_LOG(LogTemp, Log, TEXT("Module marked as initialized: %s"), *ModuleName);
    }
}

void UModuleDependencyTracker::MarkModuleFailed(const FString& ModuleName)
{
    ModuleStatus[ModuleName] = EEng_DependencyStatus::Failed;
    FailedModules.AddUnique(ModuleName);
    UE_LOG(LogTemp, Error, TEXT("Module marked as failed: %s"), *ModuleName);
}

TArray<FString> UModuleDependencyTracker::GetInitializedModules() const
{
    TArray<FString> InitializedModules;
    for (const auto& ModulePair : ModuleRegistry)
    {
        if (ModulePair.Value.bIsInitialized)
        {
            InitializedModules.Add(ModulePair.Key);
        }
    }
    return InitializedModules;
}

TArray<FString> UModuleDependencyTracker::GetFailedModules() const
{
    return FailedModules;
}

TArray<FString> UModuleDependencyTracker::GetModuleDependents(const FString& ModuleName) const
{
    TArray<FString> Dependents;
    for (const auto& ModulePair : ModuleRegistry)
    {
        if (ModulePair.Value.RequiredModules.Contains(ModuleName) || 
            ModulePair.Value.OptionalModules.Contains(ModuleName))
        {
            Dependents.Add(ModulePair.Key);
        }
    }
    return Dependents;
}

TArray<FString> UModuleDependencyTracker::GetModuleDependencies(const FString& ModuleName) const
{
    if (const FEng_ModuleDependency* Module = ModuleRegistry.Find(ModuleName))
    {
        TArray<FString> AllDependencies = Module->RequiredModules;
        AllDependencies.Append(Module->OptionalModules);
        return AllDependencies;
    }
    return TArray<FString>();
}

FString UModuleDependencyTracker::GenerateDependencyReport() const
{
    FString Report = TEXT("=== MODULE DEPENDENCY REPORT ===\n");
    Report += FString::Printf(TEXT("Total Modules: %d\n"), ModuleRegistry.Num());
    Report += FString::Printf(TEXT("Initialized: %d\n"), GetInitializedModules().Num());
    Report += FString::Printf(TEXT("Failed: %d\n"), FailedModules.Num());
    Report += TEXT("\n");

    for (const auto& ModulePair : ModuleRegistry)
    {
        const FString& ModuleName = ModulePair.Key;
        const FEng_ModuleDependency& Module = ModulePair.Value;
        EEng_DependencyStatus Status = GetModuleStatus(ModuleName);

        Report += FString::Printf(TEXT("Module: %s\n"), *ModuleName);
        Report += FString::Printf(TEXT("  Priority: %d\n"), Module.InitializationPriority);
        Report += FString::Printf(TEXT("  Status: %s\n"), *UEnum::GetValueAsString(Status));
        Report += FString::Printf(TEXT("  Initialized: %s\n"), Module.bIsInitialized ? TEXT("Yes") : TEXT("No"));
        
        if (Module.RequiredModules.Num() > 0)
        {
            Report += TEXT("  Required Dependencies:\n");
            for (const FString& Dep : Module.RequiredModules)
            {
                Report += FString::Printf(TEXT("    - %s\n"), *Dep);
            }
        }
        
        if (Module.OptionalModules.Num() > 0)
        {
            Report += TEXT("  Optional Dependencies:\n");
            for (const FString& Dep : Module.OptionalModules)
            {
                Report += FString::Printf(TEXT("    - %s\n"), *Dep);
            }
        }
        
        Report += TEXT("\n");
    }

    return Report;
}

bool UModuleDependencyTracker::ResolveDependenciesRecursive(const FString& ModuleName, TArray<FString>& ResolvedOrder, TSet<FString>& Visiting)
{
    if (Visiting.Contains(ModuleName))
    {
        // Circular dependency detected
        CircularDependencyChain.Add(ModuleName);
        UpdateModuleStatus(ModuleName, EEng_DependencyStatus::CircularRef);
        return false;
    }

    if (ModuleStatus[ModuleName] == EEng_DependencyStatus::Resolved)
    {
        return true; // Already resolved
    }

    Visiting.Add(ModuleName);
    UpdateModuleStatus(ModuleName, EEng_DependencyStatus::Resolving);

    const FEng_ModuleDependency* Module = ModuleRegistry.Find(ModuleName);
    if (!Module)
    {
        Visiting.Remove(ModuleName);
        return false;
    }

    // Resolve all required dependencies first
    for (const FString& RequiredModule : Module->RequiredModules)
    {
        if (!ResolveDependenciesRecursive(RequiredModule, ResolvedOrder, Visiting))
        {
            Visiting.Remove(ModuleName);
            return false;
        }
    }

    // Add this module to the resolved order
    ResolvedOrder.AddUnique(ModuleName);
    UpdateModuleStatus(ModuleName, EEng_DependencyStatus::Resolved);
    Visiting.Remove(ModuleName);

    return true;
}

void UModuleDependencyTracker::DetectCircularDependencies()
{
    TSet<FString> Visited;
    TSet<FString> RecStack;

    for (const auto& ModulePair : ModuleRegistry)
    {
        if (!Visited.Contains(ModulePair.Key))
        {
            // Implement circular dependency detection algorithm
            // This is a simplified version - full implementation would use DFS
        }
    }
}

bool UModuleDependencyTracker::CanModuleInitialize(const FString& ModuleName) const
{
    const FEng_ModuleDependency* Module = ModuleRegistry.Find(ModuleName);
    if (!Module)
    {
        return false;
    }

    // Check if all required dependencies are initialized
    for (const FString& RequiredModule : Module->RequiredModules)
    {
        const FEng_ModuleDependency* RequiredModuleData = ModuleRegistry.Find(RequiredModule);
        if (!RequiredModuleData || !RequiredModuleData->bIsInitialized)
        {
            return false;
        }
    }

    return true;
}

void UModuleDependencyTracker::UpdateModuleStatus(const FString& ModuleName, EEng_DependencyStatus Status)
{
    ModuleStatus[ModuleName] = Status;
}