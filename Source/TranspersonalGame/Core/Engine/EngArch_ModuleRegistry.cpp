#include "EngArch_ModuleRegistry.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Subsystems/GameInstanceSubsystem.h"

UEngArch_ModuleRegistry::UEngArch_ModuleRegistry()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize module status tracking
    ModuleLoadOrder = {
        EEng_ModuleType::Core,
        EEng_ModuleType::Physics,
        EEng_ModuleType::WorldGeneration,
        EEng_ModuleType::Environment,
        EEng_ModuleType::Character,
        EEng_ModuleType::AI,
        EEng_ModuleType::Combat,
        EEng_ModuleType::Quest,
        EEng_ModuleType::Audio,
        EEng_ModuleType::VFX,
        EEng_ModuleType::UI
    };
    
    // Initialize all modules as unloaded
    for (EEng_ModuleType ModuleType : ModuleLoadOrder)
    {
        FEng_ModuleInfo ModuleInfo;
        ModuleInfo.ModuleType = ModuleType;
        ModuleInfo.LoadStatus = EEng_ModuleStatus::Unloaded;
        ModuleInfo.LoadPriority = GetModulePriority(ModuleType);
        ModuleInfo.DependentModules = GetModuleDependencies(ModuleType);
        
        RegisteredModules.Add(ModuleType, ModuleInfo);
    }
}

void UEngArch_ModuleRegistry::BeginPlay()
{
    Super::BeginPlay();
    
    if (GetWorld())
    {
        UE_LOG(LogTemp, Warning, TEXT("EngArch_ModuleRegistry: Registry initialized"));
        InitializeModuleRegistry();
    }
}

void UEngArch_ModuleRegistry::InitializeModuleRegistry()
{
    // Validate all registered modules
    for (auto& ModulePair : RegisteredModules)
    {
        ValidateModuleDependencies(ModulePair.Key);
    }
    
    // Start loading core modules
    LoadModule(EEng_ModuleType::Core);
    
    UE_LOG(LogTemp, Warning, TEXT("EngArch_ModuleRegistry: %d modules registered"), RegisteredModules.Num());
}

bool UEngArch_ModuleRegistry::LoadModule(EEng_ModuleType ModuleType)
{
    FEng_ModuleInfo* ModuleInfo = RegisteredModules.Find(ModuleType);
    if (!ModuleInfo)
    {
        UE_LOG(LogTemp, Error, TEXT("EngArch_ModuleRegistry: Module %d not registered"), (int32)ModuleType);
        return false;
    }
    
    // Check if already loaded
    if (ModuleInfo->LoadStatus == EEng_ModuleStatus::Loaded)
    {
        return true;
    }
    
    // Check dependencies
    for (EEng_ModuleType Dependency : ModuleInfo->DependentModules)
    {
        if (!IsModuleLoaded(Dependency))
        {
            UE_LOG(LogTemp, Warning, TEXT("EngArch_ModuleRegistry: Loading dependency %d for module %d"), (int32)Dependency, (int32)ModuleType);
            if (!LoadModule(Dependency))
            {
                UE_LOG(LogTemp, Error, TEXT("EngArch_ModuleRegistry: Failed to load dependency %d"), (int32)Dependency);
                return false;
            }
        }
    }
    
    // Attempt to load the module
    ModuleInfo->LoadStatus = EEng_ModuleStatus::Loading;
    
    bool bLoadSuccess = LoadModuleImplementation(ModuleType);
    
    if (bLoadSuccess)
    {
        ModuleInfo->LoadStatus = EEng_ModuleStatus::Loaded;
        ModuleInfo->LoadTimestamp = FDateTime::Now();
        
        UE_LOG(LogTemp, Warning, TEXT("EngArch_ModuleRegistry: Module %d loaded successfully"), (int32)ModuleType);
        
        // Notify dependent modules
        OnModuleLoaded.Broadcast(ModuleType);
        
        return true;
    }
    else
    {
        ModuleInfo->LoadStatus = EEng_ModuleStatus::Failed;
        UE_LOG(LogTemp, Error, TEXT("EngArch_ModuleRegistry: Module %d failed to load"), (int32)ModuleType);
        return false;
    }
}

bool UEngArch_ModuleRegistry::LoadModuleImplementation(EEng_ModuleType ModuleType)
{
    // Module-specific loading logic
    switch (ModuleType)
    {
        case EEng_ModuleType::Core:
            return LoadCoreModule();
            
        case EEng_ModuleType::Physics:
            return LoadPhysicsModule();
            
        case EEng_ModuleType::WorldGeneration:
            return LoadWorldGenerationModule();
            
        case EEng_ModuleType::Environment:
            return LoadEnvironmentModule();
            
        case EEng_ModuleType::Character:
            return LoadCharacterModule();
            
        case EEng_ModuleType::AI:
            return LoadAIModule();
            
        case EEng_ModuleType::Combat:
            return LoadCombatModule();
            
        case EEng_ModuleType::Quest:
            return LoadQuestModule();
            
        case EEng_ModuleType::Audio:
            return LoadAudioModule();
            
        case EEng_ModuleType::VFX:
            return LoadVFXModule();
            
        case EEng_ModuleType::UI:
            return LoadUIModule();
            
        default:
            return false;
    }
}

bool UEngArch_ModuleRegistry::LoadCoreModule()
{
    // Core module is always available
    return true;
}

bool UEngArch_ModuleRegistry::LoadPhysicsModule()
{
    // Check if physics subsystems are available
    if (GetWorld() && GetWorld()->GetPhysicsScene())
    {
        return true;
    }
    return false;
}

bool UEngArch_ModuleRegistry::LoadWorldGenerationModule()
{
    // Check for world generation components
    return true; // Simplified for now
}

bool UEngArch_ModuleRegistry::LoadEnvironmentModule()
{
    // Check for environment systems
    return true; // Simplified for now
}

bool UEngArch_ModuleRegistry::LoadCharacterModule()
{
    // Check for character systems
    return true; // Simplified for now
}

bool UEngArch_ModuleRegistry::LoadAIModule()
{
    // Check for AI systems
    return true; // Simplified for now
}

bool UEngArch_ModuleRegistry::LoadCombatModule()
{
    // Check for combat systems
    return true; // Simplified for now
}

bool UEngArch_ModuleRegistry::LoadQuestModule()
{
    // Check for quest systems
    return true; // Simplified for now
}

bool UEngArch_ModuleRegistry::LoadAudioModule()
{
    // Check for audio systems
    return true; // Simplified for now
}

bool UEngArch_ModuleRegistry::LoadVFXModule()
{
    // Check for VFX systems
    return true; // Simplified for now
}

bool UEngArch_ModuleRegistry::LoadUIModule()
{
    // Check for UI systems
    return true; // Simplified for now
}

bool UEngArch_ModuleRegistry::UnloadModule(EEng_ModuleType ModuleType)
{
    FEng_ModuleInfo* ModuleInfo = RegisteredModules.Find(ModuleType);
    if (!ModuleInfo || ModuleInfo->LoadStatus != EEng_ModuleStatus::Loaded)
    {
        return false;
    }
    
    // Check if other modules depend on this one
    for (const auto& OtherModulePair : RegisteredModules)
    {
        if (OtherModulePair.Value.DependentModules.Contains(ModuleType) && 
            OtherModulePair.Value.LoadStatus == EEng_ModuleStatus::Loaded)
        {
            UE_LOG(LogTemp, Error, TEXT("EngArch_ModuleRegistry: Cannot unload module %d - other modules depend on it"), (int32)ModuleType);
            return false;
        }
    }
    
    ModuleInfo->LoadStatus = EEng_ModuleStatus::Unloaded;
    OnModuleUnloaded.Broadcast(ModuleType);
    
    return true;
}

bool UEngArch_ModuleRegistry::IsModuleLoaded(EEng_ModuleType ModuleType) const
{
    const FEng_ModuleInfo* ModuleInfo = RegisteredModules.Find(ModuleType);
    return ModuleInfo && ModuleInfo->LoadStatus == EEng_ModuleStatus::Loaded;
}

EEng_ModuleStatus UEngArch_ModuleRegistry::GetModuleStatus(EEng_ModuleType ModuleType) const
{
    const FEng_ModuleInfo* ModuleInfo = RegisteredModules.Find(ModuleType);
    return ModuleInfo ? ModuleInfo->LoadStatus : EEng_ModuleStatus::Unloaded;
}

TArray<EEng_ModuleType> UEngArch_ModuleRegistry::GetLoadedModules() const
{
    TArray<EEng_ModuleType> LoadedModules;
    
    for (const auto& ModulePair : RegisteredModules)
    {
        if (ModulePair.Value.LoadStatus == EEng_ModuleStatus::Loaded)
        {
            LoadedModules.Add(ModulePair.Key);
        }
    }
    
    return LoadedModules;
}

bool UEngArch_ModuleRegistry::ValidateModuleDependencies(EEng_ModuleType ModuleType)
{
    const FEng_ModuleInfo* ModuleInfo = RegisteredModules.Find(ModuleType);
    if (!ModuleInfo)
    {
        return false;
    }
    
    // Check for circular dependencies
    TArray<EEng_ModuleType> VisitedModules;
    return ValidateModuleDependenciesRecursive(ModuleType, VisitedModules);
}

bool UEngArch_ModuleRegistry::ValidateModuleDependenciesRecursive(EEng_ModuleType ModuleType, TArray<EEng_ModuleType>& VisitedModules)
{
    if (VisitedModules.Contains(ModuleType))
    {
        UE_LOG(LogTemp, Error, TEXT("EngArch_ModuleRegistry: Circular dependency detected for module %d"), (int32)ModuleType);
        return false;
    }
    
    VisitedModules.Add(ModuleType);
    
    const FEng_ModuleInfo* ModuleInfo = RegisteredModules.Find(ModuleType);
    if (ModuleInfo)
    {
        for (EEng_ModuleType Dependency : ModuleInfo->DependentModules)
        {
            if (!ValidateModuleDependenciesRecursive(Dependency, VisitedModules))
            {
                return false;
            }
        }
    }
    
    VisitedModules.Remove(ModuleType);
    return true;
}

int32 UEngArch_ModuleRegistry::GetModulePriority(EEng_ModuleType ModuleType)
{
    switch (ModuleType)
    {
        case EEng_ModuleType::Core: return 100;
        case EEng_ModuleType::Physics: return 90;
        case EEng_ModuleType::WorldGeneration: return 80;
        case EEng_ModuleType::Environment: return 70;
        case EEng_ModuleType::Character: return 60;
        case EEng_ModuleType::AI: return 50;
        case EEng_ModuleType::Combat: return 40;
        case EEng_ModuleType::Quest: return 30;
        case EEng_ModuleType::Audio: return 20;
        case EEng_ModuleType::VFX: return 15;
        case EEng_ModuleType::UI: return 10;
        default: return 0;
    }
}

TArray<EEng_ModuleType> UEngArch_ModuleRegistry::GetModuleDependencies(EEng_ModuleType ModuleType)
{
    TArray<EEng_ModuleType> Dependencies;
    
    switch (ModuleType)
    {
        case EEng_ModuleType::Physics:
            Dependencies.Add(EEng_ModuleType::Core);
            break;
            
        case EEng_ModuleType::WorldGeneration:
            Dependencies.Add(EEng_ModuleType::Core);
            Dependencies.Add(EEng_ModuleType::Physics);
            break;
            
        case EEng_ModuleType::Environment:
            Dependencies.Add(EEng_ModuleType::Core);
            Dependencies.Add(EEng_ModuleType::WorldGeneration);
            break;
            
        case EEng_ModuleType::Character:
            Dependencies.Add(EEng_ModuleType::Core);
            Dependencies.Add(EEng_ModuleType::Physics);
            break;
            
        case EEng_ModuleType::AI:
            Dependencies.Add(EEng_ModuleType::Core);
            Dependencies.Add(EEng_ModuleType::Character);
            break;
            
        case EEng_ModuleType::Combat:
            Dependencies.Add(EEng_ModuleType::Core);
            Dependencies.Add(EEng_ModuleType::Character);
            Dependencies.Add(EEng_ModuleType::AI);
            break;
            
        case EEng_ModuleType::Quest:
            Dependencies.Add(EEng_ModuleType::Core);
            Dependencies.Add(EEng_ModuleType::Character);
            Dependencies.Add(EEng_ModuleType::AI);
            break;
            
        case EEng_ModuleType::Audio:
            Dependencies.Add(EEng_ModuleType::Core);
            break;
            
        case EEng_ModuleType::VFX:
            Dependencies.Add(EEng_ModuleType::Core);
            break;
            
        case EEng_ModuleType::UI:
            Dependencies.Add(EEng_ModuleType::Core);
            Dependencies.Add(EEng_ModuleType::Character);
            break;
    }
    
    return Dependencies;
}