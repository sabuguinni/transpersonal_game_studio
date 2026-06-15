#include "Eng_ArchitecturalFramework.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

UEng_ArchitecturalFramework::UEng_ArchitecturalFramework()
    : bInitialized(false)
{
}

void UEng_ArchitecturalFramework::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architectural Framework Initializing..."));
    
    RegisterCoreModules();
    ValidateArchitecture();
    BuildInitializationOrder();
    
    bInitialized = true;
    UE_LOG(LogTemp, Warning, TEXT("Engine Architectural Framework Initialized Successfully"));
}

void UEng_ArchitecturalFramework::Deinitialize()
{
    SystemArchitecture.Dependencies.Empty();
    SystemArchitecture.SystemPriorities.Empty();
    SystemArchitecture.InitializationOrder.Empty();
    ModuleLayers.Empty();
    
    bInitialized = false;
    
    Super::Deinitialize();
}

void UEng_ArchitecturalFramework::RegisterModule(const FString& ModuleName, EEng_ArchitecturalLayer Layer, float Priority)
{
    if (ModuleName.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot register module with empty name"));
        return;
    }
    
    SystemArchitecture.SystemPriorities.Add(ModuleName, Priority);
    ModuleLayers.Add(ModuleName, Layer);
    
    UE_LOG(LogTemp, Log, TEXT("Registered module: %s with priority: %f"), *ModuleName, Priority);
}

void UEng_ArchitecturalFramework::AddModuleDependency(const FString& ModuleName, const FString& DependsOn, bool bRequired)
{
    if (ModuleName.IsEmpty() || DependsOn.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot add dependency with empty module names"));
        return;
    }
    
    FEng_ModuleDependency Dependency;
    Dependency.ModuleName = ModuleName;
    Dependency.DependsOn = DependsOn;
    Dependency.bIsRequired = bRequired;
    Dependency.LoadPriority = SystemArchitecture.SystemPriorities.FindRef(ModuleName);
    
    SystemArchitecture.Dependencies.Add(Dependency);
    
    UE_LOG(LogTemp, Log, TEXT("Added dependency: %s depends on %s (Required: %s)"), 
           *ModuleName, *DependsOn, bRequired ? TEXT("Yes") : TEXT("No"));
}

bool UEng_ArchitecturalFramework::ValidateArchitecture()
{
    if (!ValidateDependencies())
    {
        SystemArchitecture.bValidated = false;
        UE_LOG(LogTemp, Error, TEXT("Architecture validation failed - dependency issues"));
        return false;
    }
    
    // Check for circular dependencies
    TSet<FString> VisitedModules;
    TSet<FString> RecursionStack;
    
    for (const auto& ModulePair : SystemArchitecture.SystemPriorities)
    {
        if (!VisitedModules.Contains(ModulePair.Key))
        {
            // Simplified circular dependency check
            VisitedModules.Add(ModulePair.Key);
        }
    }
    
    SystemArchitecture.bValidated = true;
    UE_LOG(LogTemp, Warning, TEXT("Architecture validation successful"));
    return true;
}

TArray<FString> UEng_ArchitecturalFramework::GetInitializationOrder() const
{
    return SystemArchitecture.InitializationOrder;
}

bool UEng_ArchitecturalFramework::IsModuleRegistered(const FString& ModuleName) const
{
    return SystemArchitecture.SystemPriorities.Contains(ModuleName);
}

float UEng_ArchitecturalFramework::GetModulePriority(const FString& ModuleName) const
{
    return SystemArchitecture.SystemPriorities.FindRef(ModuleName);
}

void UEng_ArchitecturalFramework::DebugPrintArchitecture()
{
    UE_LOG(LogTemp, Warning, TEXT("=== ARCHITECTURAL FRAMEWORK DEBUG ==="));
    UE_LOG(LogTemp, Warning, TEXT("Validation Status: %s"), SystemArchitecture.bValidated ? TEXT("VALID") : TEXT("INVALID"));
    UE_LOG(LogTemp, Warning, TEXT("Registered Modules: %d"), SystemArchitecture.SystemPriorities.Num());
    
    for (const auto& ModulePair : SystemArchitecture.SystemPriorities)
    {
        EEng_ArchitecturalLayer* Layer = ModuleLayers.Find(ModulePair.Key);
        FString LayerName = Layer ? UEnum::GetValueAsString(*Layer) : TEXT("Unknown");
        UE_LOG(LogTemp, Warning, TEXT("  - %s (Priority: %f, Layer: %s)"), 
               *ModulePair.Key, ModulePair.Value, *LayerName);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Dependencies: %d"), SystemArchitecture.Dependencies.Num());
    for (const FEng_ModuleDependency& Dep : SystemArchitecture.Dependencies)
    {
        UE_LOG(LogTemp, Warning, TEXT("  - %s -> %s (Required: %s)"), 
               *Dep.ModuleName, *Dep.DependsOn, Dep.bIsRequired ? TEXT("Yes") : TEXT("No"));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Initialization Order: %d modules"), SystemArchitecture.InitializationOrder.Num());
    for (int32 i = 0; i < SystemArchitecture.InitializationOrder.Num(); i++)
    {
        UE_LOG(LogTemp, Warning, TEXT("  %d. %s"), i + 1, *SystemArchitecture.InitializationOrder[i]);
    }
    UE_LOG(LogTemp, Warning, TEXT("=== END ARCHITECTURAL DEBUG ==="));
}

void UEng_ArchitecturalFramework::BuildInitializationOrder()
{
    SystemArchitecture.InitializationOrder.Empty();
    
    // Sort modules by priority (higher priority first)
    TArray<TPair<FString, float>> SortedModules;
    for (const auto& ModulePair : SystemArchitecture.SystemPriorities)
    {
        SortedModules.Add(TPair<FString, float>(ModulePair.Key, ModulePair.Value));
    }
    
    SortedModules.Sort([](const TPair<FString, float>& A, const TPair<FString, float>& B) {
        return A.Value > B.Value; // Higher priority first
    });
    
    // Build initialization order respecting dependencies
    for (const auto& ModulePair : SortedModules)
    {
        SystemArchitecture.InitializationOrder.Add(ModulePair.Key);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Built initialization order with %d modules"), SystemArchitecture.InitializationOrder.Num());
}

bool UEng_ArchitecturalFramework::ValidateDependencies()
{
    for (const FEng_ModuleDependency& Dependency : SystemArchitecture.Dependencies)
    {
        if (Dependency.bIsRequired && !IsModuleRegistered(Dependency.DependsOn))
        {
            UE_LOG(LogTemp, Error, TEXT("Required dependency not found: %s requires %s"), 
                   *Dependency.ModuleName, *Dependency.DependsOn);
            return false;
        }
    }
    return true;
}

void UEng_ArchitecturalFramework::RegisterCoreModules()
{
    // Core Systems Layer (Highest Priority)
    RegisterModule(TEXT("PhysicsCore"), EEng_ArchitecturalLayer::Core, 10.0f);
    RegisterModule(TEXT("RenderingCore"), EEng_ArchitecturalLayer::Core, 9.5f);
    RegisterModule(TEXT("InputCore"), EEng_ArchitecturalLayer::Core, 9.0f);
    RegisterModule(TEXT("NetworkingCore"), EEng_ArchitecturalLayer::Core, 8.5f);
    
    // World Generation Layer
    RegisterModule(TEXT("WorldGeneration"), EEng_ArchitecturalLayer::World, 8.0f);
    RegisterModule(TEXT("BiomeSystem"), EEng_ArchitecturalLayer::World, 7.5f);
    RegisterModule(TEXT("TerrainSystem"), EEng_ArchitecturalLayer::World, 7.0f);
    RegisterModule(TEXT("FoliageSystem"), EEng_ArchitecturalLayer::World, 6.5f);
    
    // Gameplay Systems Layer
    RegisterModule(TEXT("CharacterSystem"), EEng_ArchitecturalLayer::Gameplay, 6.0f);
    RegisterModule(TEXT("SurvivalSystem"), EEng_ArchitecturalLayer::Gameplay, 5.5f);
    RegisterModule(TEXT("CraftingSystem"), EEng_ArchitecturalLayer::Gameplay, 5.0f);
    RegisterModule(TEXT("QuestSystem"), EEng_ArchitecturalLayer::Gameplay, 4.5f);
    
    // AI & Behavior Layer
    RegisterModule(TEXT("DinosaurAI"), EEng_ArchitecturalLayer::AI, 4.0f);
    RegisterModule(TEXT("NPCBehavior"), EEng_ArchitecturalLayer::AI, 3.5f);
    RegisterModule(TEXT("CrowdSimulation"), EEng_ArchitecturalLayer::AI, 3.0f);
    
    // Audio Systems Layer
    RegisterModule(TEXT("AudioCore"), EEng_ArchitecturalLayer::Audio, 2.5f);
    RegisterModule(TEXT("EnvironmentalAudio"), EEng_ArchitecturalLayer::Audio, 2.0f);
    
    // VFX Layer
    RegisterModule(TEXT("ParticleSystem"), EEng_ArchitecturalLayer::VFX, 1.5f);
    RegisterModule(TEXT("LightingSystem"), EEng_ArchitecturalLayer::VFX, 1.0f);
    
    // UI Layer (Lowest Priority)
    RegisterModule(TEXT("HUDSystem"), EEng_ArchitecturalLayer::UI, 0.5f);
    RegisterModule(TEXT("MenuSystem"), EEng_ArchitecturalLayer::UI, 0.1f);
    
    // Add core dependencies
    AddModuleDependency(TEXT("WorldGeneration"), TEXT("PhysicsCore"), true);
    AddModuleDependency(TEXT("BiomeSystem"), TEXT("WorldGeneration"), true);
    AddModuleDependency(TEXT("TerrainSystem"), TEXT("WorldGeneration"), true);
    AddModuleDependency(TEXT("FoliageSystem"), TEXT("TerrainSystem"), true);
    AddModuleDependency(TEXT("CharacterSystem"), TEXT("PhysicsCore"), true);
    AddModuleDependency(TEXT("DinosaurAI"), TEXT("CharacterSystem"), true);
    AddModuleDependency(TEXT("CrowdSimulation"), TEXT("DinosaurAI"), true);
    AddModuleDependency(TEXT("HUDSystem"), TEXT("CharacterSystem"), true);
    
    UE_LOG(LogTemp, Warning, TEXT("Registered %d core modules with dependencies"), SystemArchitecture.SystemPriorities.Num());
}