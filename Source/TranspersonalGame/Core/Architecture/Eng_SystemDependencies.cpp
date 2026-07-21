#include "Eng_SystemDependencies.h"
#include "Engine/Engine.h"

UEng_SystemDependencies::UEng_SystemDependencies()
{
    bDependenciesValidated = false;
    LastValidationTime = 0.0f;
}

void UEng_SystemDependencies::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Eng_SystemDependencies: Initializing system dependency manager"));
    
    InitializeCoreDependencies();
    ValidateSystemDependencies();
}

void UEng_SystemDependencies::Deinitialize()
{
    SystemDependencies.Empty();
    LoadedModules.Empty();
    bDependenciesValidated = false;
    
    Super::Deinitialize();
}

void UEng_SystemDependencies::InitializeCoreDependencies()
{
    // Core systems with their dependencies
    FEng_SystemDependency WorldArchitecture;
    WorldArchitecture.SystemName = TEXT("WorldArchitecture");
    WorldArchitecture.InitializationPriority = 10;
    WorldArchitecture.bIsCritical = true;
    RegisterSystemDependency(WorldArchitecture);

    FEng_SystemDependency BiomeSystem;
    BiomeSystem.SystemName = TEXT("BiomeSystem");
    BiomeSystem.RequiredSystems.Add(TEXT("WorldArchitecture"));
    BiomeSystem.InitializationPriority = 20;
    BiomeSystem.bIsCritical = true;
    RegisterSystemDependency(BiomeSystem);

    FEng_SystemDependency PCGWorldGenerator;
    PCGWorldGenerator.SystemName = TEXT("PCGWorldGenerator");
    PCGWorldGenerator.RequiredSystems.Add(TEXT("WorldArchitecture"));
    PCGWorldGenerator.RequiredSystems.Add(TEXT("BiomeSystem"));
    PCGWorldGenerator.InitializationPriority = 30;
    PCGWorldGenerator.bIsCritical = true;
    RegisterSystemDependency(PCGWorldGenerator);

    FEng_SystemDependency FoliageManager;
    FoliageManager.SystemName = TEXT("FoliageManager");
    FoliageManager.RequiredSystems.Add(TEXT("PCGWorldGenerator"));
    FoliageManager.InitializationPriority = 40;
    FoliageManager.bIsCritical = false;
    RegisterSystemDependency(FoliageManager);

    FEng_SystemDependency DinosaurAI;
    DinosaurAI.SystemName = TEXT("DinosaurAI");
    DinosaurAI.RequiredSystems.Add(TEXT("BiomeSystem"));
    DinosaurAI.InitializationPriority = 50;
    DinosaurAI.bIsCritical = true;
    RegisterSystemDependency(DinosaurAI);

    FEng_SystemDependency CrowdSimulation;
    CrowdSimulation.SystemName = TEXT("CrowdSimulation");
    CrowdSimulation.RequiredSystems.Add(TEXT("DinosaurAI"));
    CrowdSimulation.InitializationPriority = 60;
    CrowdSimulation.bIsCritical = false;
    RegisterSystemDependency(CrowdSimulation);

    UE_LOG(LogTemp, Warning, TEXT("Eng_SystemDependencies: Registered %d core system dependencies"), 
           SystemDependencies.Num());
}

void UEng_SystemDependencies::RegisterSystemDependency(const FEng_SystemDependency& Dependency)
{
    // Check if system already exists
    for (int32 i = 0; i < SystemDependencies.Num(); i++)
    {
        if (SystemDependencies[i].SystemName == Dependency.SystemName)
        {
            SystemDependencies[i] = Dependency;
            UE_LOG(LogTemp, Warning, TEXT("Eng_SystemDependencies: Updated dependency for %s"), 
                   *Dependency.SystemName);
            return;
        }
    }
    
    SystemDependencies.Add(Dependency);
    UE_LOG(LogTemp, Warning, TEXT("Eng_SystemDependencies: Registered new dependency %s (Priority: %d)"), 
           *Dependency.SystemName, Dependency.InitializationPriority);
}

bool UEng_SystemDependencies::ValidateSystemDependencies()
{
    if (CheckCircularDependencies())
    {
        UE_LOG(LogTemp, Error, TEXT("Eng_SystemDependencies: Circular dependencies detected!"));
        bDependenciesValidated = false;
        return false;
    }
    
    TArray<FString> InitOrder = GetInitializationOrder();
    if (InitOrder.Num() != SystemDependencies.Num())
    {
        UE_LOG(LogTemp, Error, TEXT("Eng_SystemDependencies: Failed to resolve all dependencies"));
        bDependenciesValidated = false;
        return false;
    }
    
    bDependenciesValidated = true;
    LastValidationTime = FPlatformTime::Seconds();
    
    UE_LOG(LogTemp, Warning, TEXT("Eng_SystemDependencies: All dependencies validated successfully"));
    return true;
}

TArray<FString> UEng_SystemDependencies::GetInitializationOrder()
{
    TArray<FString> InitOrder;
    
    if (!ResolveDependencyOrder(InitOrder))
    {
        UE_LOG(LogTemp, Error, TEXT("Eng_SystemDependencies: Failed to resolve dependency order"));
        return TArray<FString>();
    }
    
    return InitOrder;
}

bool UEng_SystemDependencies::IsSystemInitialized(const FString& SystemName)
{
    for (const FEng_SystemDependency& Dependency : SystemDependencies)
    {
        if (Dependency.SystemName == SystemName)
        {
            return Dependency.bIsInitialized;
        }
    }
    return false;
}

void UEng_SystemDependencies::MarkSystemInitialized(const FString& SystemName)
{
    for (FEng_SystemDependency& Dependency : SystemDependencies)
    {
        if (Dependency.SystemName == SystemName)
        {
            Dependency.bIsInitialized = true;
            UE_LOG(LogTemp, Warning, TEXT("Eng_SystemDependencies: Marked %s as initialized"), 
                   *SystemName);
            return;
        }
    }
}

void UEng_SystemDependencies::RegisterModule(const FEng_ModuleInfo& ModuleInfo)
{
    // Check if module already exists
    for (int32 i = 0; i < LoadedModules.Num(); i++)
    {
        if (LoadedModules[i].ModuleName == ModuleInfo.ModuleName)
        {
            LoadedModules[i] = ModuleInfo;
            return;
        }
    }
    
    LoadedModules.Add(ModuleInfo);
    UE_LOG(LogTemp, Warning, TEXT("Eng_SystemDependencies: Registered module %s v%s"), 
           *ModuleInfo.ModuleName, *ModuleInfo.ModuleVersion);
}

TArray<FEng_ModuleInfo> UEng_SystemDependencies::GetLoadedModules() const
{
    return LoadedModules;
}

bool UEng_SystemDependencies::CheckCircularDependencies()
{
    for (const FEng_SystemDependency& Dependency : SystemDependencies)
    {
        TArray<FString> VisitedSystems;
        if (HasCircularDependency(Dependency.SystemName, VisitedSystems))
        {
            return true;
        }
    }
    return false;
}

void UEng_SystemDependencies::GenerateDependencyReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== SYSTEM DEPENDENCY REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Total Systems: %d"), SystemDependencies.Num());
    UE_LOG(LogTemp, Warning, TEXT("Dependencies Validated: %s"), 
           bDependenciesValidated ? TEXT("YES") : TEXT("NO"));
    
    TArray<FString> InitOrder = GetInitializationOrder();
    UE_LOG(LogTemp, Warning, TEXT("Initialization Order:"));
    for (int32 i = 0; i < InitOrder.Num(); i++)
    {
        UE_LOG(LogTemp, Warning, TEXT("  %d. %s"), i + 1, *InitOrder[i]);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Loaded Modules: %d"), LoadedModules.Num());
    for (const FEng_ModuleInfo& Module : LoadedModules)
    {
        UE_LOG(LogTemp, Warning, TEXT("  - %s v%s (Loaded: %s)"), 
               *Module.ModuleName, *Module.ModuleVersion, 
               Module.bIsLoaded ? TEXT("YES") : TEXT("NO"));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== END REPORT ==="));
}

bool UEng_SystemDependencies::ResolveDependencyOrder(TArray<FString>& OutOrder)
{
    OutOrder.Empty();
    TArray<FEng_SystemDependency> RemainingDeps = SystemDependencies;
    
    while (RemainingDeps.Num() > 0)
    {
        bool bFoundNext = false;
        
        for (int32 i = 0; i < RemainingDeps.Num(); i++)
        {
            bool bCanInitialize = true;
            
            // Check if all dependencies are already in the order
            for (const FString& RequiredSystem : RemainingDeps[i].RequiredSystems)
            {
                if (!OutOrder.Contains(RequiredSystem))
                {
                    bCanInitialize = false;
                    break;
                }
            }
            
            if (bCanInitialize)
            {
                OutOrder.Add(RemainingDeps[i].SystemName);
                RemainingDeps.RemoveAt(i);
                bFoundNext = true;
                break;
            }
        }
        
        if (!bFoundNext)
        {
            // Could not resolve dependencies
            return false;
        }
    }
    
    return true;
}

bool UEng_SystemDependencies::HasCircularDependency(const FString& SystemName, TArray<FString>& VisitedSystems)
{
    if (VisitedSystems.Contains(SystemName))
    {
        return true; // Circular dependency found
    }
    
    VisitedSystems.Add(SystemName);
    
    // Find the system dependency
    for (const FEng_SystemDependency& Dependency : SystemDependencies)
    {
        if (Dependency.SystemName == SystemName)
        {
            for (const FString& RequiredSystem : Dependency.RequiredSystems)
            {
                if (HasCircularDependency(RequiredSystem, VisitedSystems))
                {
                    return true;
                }
            }
            break;
        }
    }
    
    VisitedSystems.Remove(SystemName);
    return false;
}

void UEng_SystemDependencies::LogDependencyChain(const FString& SystemName, int32 Depth)
{
    FString Indent = FString::ChrN(Depth * 2, TEXT(' '));
    UE_LOG(LogTemp, Warning, TEXT("%s- %s"), *Indent, *SystemName);
    
    for (const FEng_SystemDependency& Dependency : SystemDependencies)
    {
        if (Dependency.SystemName == SystemName)
        {
            for (const FString& RequiredSystem : Dependency.RequiredSystems)
            {
                LogDependencyChain(RequiredSystem, Depth + 1);
            }
            break;
        }
    }
}