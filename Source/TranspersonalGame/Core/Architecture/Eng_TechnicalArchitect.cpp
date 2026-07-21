#include "Eng_TechnicalArchitect.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"

UEng_TechnicalArchitect::UEng_TechnicalArchitect()
{
    bArchitectureValidated = false;
    LastValidationTime = 0.0f;
    PerformanceTargets = FEng_PerformanceTargets();
}

void UEng_TechnicalArchitect::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Technical Architect: Initializing architectural framework"));
    
    // Register core modules with proper dependencies
    RegisterModule(TEXT("CoreEngine"), EEng_ArchitecturalLayer::Core, 100, TArray<FString>());
    RegisterModule(TEXT("PhysicsSystem"), EEng_ArchitecturalLayer::Core, 90, {TEXT("CoreEngine")});
    RegisterModule(TEXT("WorldGeneration"), EEng_ArchitecturalLayer::World, 80, {TEXT("PhysicsSystem")});
    RegisterModule(TEXT("CharacterSystem"), EEng_ArchitecturalLayer::Character, 70, {TEXT("WorldGeneration")});
    RegisterModule(TEXT("AIBehavior"), EEng_ArchitecturalLayer::AI, 60, {TEXT("CharacterSystem")});
    RegisterModule(TEXT("AudioSystem"), EEng_ArchitecturalLayer::Audio, 50, {TEXT("WorldGeneration")});
    RegisterModule(TEXT("VFXSystem"), EEng_ArchitecturalLayer::VFX, 40, {TEXT("WorldGeneration")});
    RegisterModule(TEXT("UISystem"), EEng_ArchitecturalLayer::UI, 30, {TEXT("CharacterSystem")});
    
    // Validate architecture integrity
    ValidateModuleDependencies();
    
    // Set performance targets for prehistoric survival game
    SetPerformanceTarget(60.0f, 8000, 4096.0f);
    
    bArchitectureValidated = true;
    LastValidationTime = FPlatformTime::Seconds();
    
    UE_LOG(LogTemp, Warning, TEXT("Technical Architect: Architecture framework initialized with %d modules"), RegisteredModules.Num());
}

void UEng_TechnicalArchitect::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Technical Architect: Shutting down architectural framework"));
    RegisteredModules.Empty();
    Super::Deinitialize();
}

void UEng_TechnicalArchitect::RegisterModule(const FString& ModuleName, EEng_ArchitecturalLayer Layer, int32 Priority, const TArray<FString>& Dependencies)
{
    FEng_ModuleInfo NewModule;
    NewModule.ModuleName = ModuleName;
    NewModule.Layer = Layer;
    NewModule.Priority = Priority;
    NewModule.Dependencies = Dependencies;
    NewModule.bIsInitialized = false;
    NewModule.InitializationTime = 0.0f;
    
    RegisteredModules.Add(NewModule);
    
    UE_LOG(LogTemp, Log, TEXT("Technical Architect: Registered module '%s' at layer %d with priority %d"), 
           *ModuleName, (int32)Layer, Priority);
}

bool UEng_TechnicalArchitect::ValidateModuleDependencies()
{
    TArray<FString> CircularDeps = GetCircularDependencies();
    
    if (CircularDeps.Num() > 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Technical Architect: Circular dependencies detected: %s"), 
               *FString::Join(CircularDeps, TEXT(" -> ")));
        return false;
    }
    
    // Check for missing dependencies
    for (const FEng_ModuleInfo& Module : RegisteredModules)
    {
        for (const FString& Dependency : Module.Dependencies)
        {
            if (!FindModule(Dependency))
            {
                UE_LOG(LogTemp, Error, TEXT("Technical Architect: Module '%s' depends on missing module '%s'"), 
                       *Module.ModuleName, *Dependency);
                return false;
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Technical Architect: Module dependency validation passed"));
    return true;
}

void UEng_TechnicalArchitect::InitializeModulesInOrder()
{
    SortModulesByPriority();
    
    for (FEng_ModuleInfo& Module : RegisteredModules)
    {
        if (!Module.bIsInitialized && CheckDependenciesInitialized(Module))
        {
            InitializeModule(Module);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Technical Architect: Module initialization sequence completed"));
}

bool UEng_TechnicalArchitect::IsModuleInitialized(const FString& ModuleName)
{
    FEng_ModuleInfo* Module = FindModule(ModuleName);
    return Module ? Module->bIsInitialized : false;
}

FEng_PerformanceTargets UEng_TechnicalArchitect::GetPerformanceTargets() const
{
    return PerformanceTargets;
}

bool UEng_TechnicalArchitect::ValidatePerformance()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Check actor count
    int32 ActorCount = World->GetCurrentLevel()->Actors.Num();
    if (ActorCount > PerformanceTargets.MaxActors)
    {
        UE_LOG(LogTemp, Warning, TEXT("Technical Architect: Actor count %d exceeds target %d"), 
               ActorCount, (int32)PerformanceTargets.MaxActors);
        return false;
    }
    
    // Check FPS (simplified check)
    float CurrentFPS = 1.0f / World->GetDeltaSeconds();
    if (CurrentFPS < PerformanceTargets.TargetFPS * 0.9f) // 10% tolerance
    {
        UE_LOG(LogTemp, Warning, TEXT("Technical Architect: FPS %.1f below target %.1f"), 
               CurrentFPS, PerformanceTargets.TargetFPS);
        return false;
    }
    
    return true;
}

TArray<FString> UEng_TechnicalArchitect::GetCircularDependencies()
{
    TArray<FString> CircularDeps;
    
    for (const FEng_ModuleInfo& Module : RegisteredModules)
    {
        TArray<FString> VisitedModules;
        if (HasCircularDependency(Module.ModuleName, VisitedModules))
        {
            CircularDeps.Append(VisitedModules);
            break;
        }
    }
    
    return CircularDeps;
}

void UEng_TechnicalArchitect::SetPerformanceTarget(float FPS, int32 MaxActors, float MaxMemoryMB)
{
    PerformanceTargets.TargetFPS = FPS;
    PerformanceTargets.MaxActors = MaxActors;
    PerformanceTargets.MaxMemoryMB = MaxMemoryMB;
    
    UE_LOG(LogTemp, Warning, TEXT("Technical Architect: Performance targets set - FPS: %.1f, Actors: %d, Memory: %.1fMB"), 
           FPS, MaxActors, MaxMemoryMB);
}

TArray<FEng_ModuleInfo> UEng_TechnicalArchitect::GetModulesByLayer(EEng_ArchitecturalLayer Layer)
{
    TArray<FEng_ModuleInfo> LayerModules;
    
    for (const FEng_ModuleInfo& Module : RegisteredModules)
    {
        if (Module.Layer == Layer)
        {
            LayerModules.Add(Module);
        }
    }
    
    return LayerModules;
}

void UEng_TechnicalArchitect::SortModulesByPriority()
{
    RegisteredModules.Sort([](const FEng_ModuleInfo& A, const FEng_ModuleInfo& B)
    {
        return A.Priority > B.Priority; // Higher priority first
    });
}

bool UEng_TechnicalArchitect::HasCircularDependency(const FString& ModuleName, TArray<FString>& VisitedModules)
{
    if (VisitedModules.Contains(ModuleName))
    {
        VisitedModules.Add(ModuleName); // Complete the cycle
        return true;
    }
    
    VisitedModules.Add(ModuleName);
    
    FEng_ModuleInfo* Module = FindModule(ModuleName);
    if (!Module)
    {
        return false;
    }
    
    for (const FString& Dependency : Module->Dependencies)
    {
        if (HasCircularDependency(Dependency, VisitedModules))
        {
            return true;
        }
    }
    
    VisitedModules.RemoveAt(VisitedModules.Num() - 1);
    return false;
}

FEng_ModuleInfo* UEng_TechnicalArchitect::FindModule(const FString& ModuleName)
{
    for (FEng_ModuleInfo& Module : RegisteredModules)
    {
        if (Module.ModuleName == ModuleName)
        {
            return &Module;
        }
    }
    return nullptr;
}

void UEng_TechnicalArchitect::InitializeModule(FEng_ModuleInfo& Module)
{
    float StartTime = FPlatformTime::Seconds();
    
    // Module-specific initialization logic would go here
    // For now, we just mark it as initialized
    Module.bIsInitialized = true;
    Module.InitializationTime = FPlatformTime::Seconds() - StartTime;
    
    UE_LOG(LogTemp, Warning, TEXT("Technical Architect: Initialized module '%s' in %.3f seconds"), 
           *Module.ModuleName, Module.InitializationTime);
}

bool UEng_TechnicalArchitect::CheckDependenciesInitialized(const FEng_ModuleInfo& Module)
{
    for (const FString& Dependency : Module.Dependencies)
    {
        if (!IsModuleInitialized(Dependency))
        {
            return false;
        }
    }
    return true;
}