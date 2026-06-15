#include "Eng_TechnicalArchitecture.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"

void UEng_TechnicalArchitecture::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Technical Architecture System Initialized"));
    
    // Set default performance targets
    PerformanceTargets.TargetFPS = 60.0f;
    PerformanceTargets.MaxActors = 8000;
    PerformanceTargets.MaxDinosaurs = 150;
    PerformanceTargets.MaxMemoryGB = 4.0f;
    PerformanceTargets.MaxDrawCalls = 2000.0f;
    
    InitializeDefaultModules();
}

void UEng_TechnicalArchitecture::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Technical Architecture System Shutdown"));
    RegisteredModules.Empty();
    ModuleInitializationStatus.Empty();
    
    Super::Deinitialize();
}

void UEng_TechnicalArchitecture::InitializeDefaultModules()
{
    // Core layer - highest priority
    RegisterModule(TEXT("CoreSystems"), {}, 1000);
    RegisterModule(TEXT("PhysicsCore"), {TEXT("CoreSystems")}, 900);
    
    // World layer
    RegisterModule(TEXT("WorldGeneration"), {TEXT("CoreSystems"), TEXT("PhysicsCore")}, 800);
    RegisterModule(TEXT("BiomeSystem"), {TEXT("WorldGeneration")}, 750);
    RegisterModule(TEXT("WeatherSystem"), {TEXT("WorldGeneration")}, 700);
    
    // Character layer
    RegisterModule(TEXT("CharacterSystem"), {TEXT("PhysicsCore")}, 600);
    RegisterModule(TEXT("MovementSystem"), {TEXT("CharacterSystem")}, 550);
    
    // AI layer
    RegisterModule(TEXT("DinosaurAI"), {TEXT("CharacterSystem"), TEXT("BiomeSystem")}, 500);
    RegisterModule(TEXT("NPCBehavior"), {TEXT("CharacterSystem")}, 450);
    RegisterModule(TEXT("CrowdSimulation"), {TEXT("NPCBehavior")}, 400);
    
    // Gameplay layer
    RegisterModule(TEXT("CombatSystem"), {TEXT("CharacterSystem"), TEXT("DinosaurAI")}, 350);
    RegisterModule(TEXT("QuestSystem"), {TEXT("NPCBehavior")}, 300);
    RegisterModule(TEXT("SurvivalSystem"), {TEXT("CharacterSystem")}, 250);
    
    // Audio/VFX layer
    RegisterModule(TEXT("AudioSystem"), {TEXT("CoreSystems")}, 200);
    RegisterModule(TEXT("VFXSystem"), {TEXT("CoreSystems")}, 150);
    
    // UI layer - lowest priority
    RegisterModule(TEXT("UISystem"), {TEXT("CoreSystems")}, 100);
    
    UE_LOG(LogTemp, Warning, TEXT("Registered %d default modules"), RegisteredModules.Num());
}

FEng_ArchitectureValidation UEng_TechnicalArchitecture::ValidateArchitecture()
{
    FEng_ArchitectureValidation Result;
    
    // Check module dependencies
    bool bDependenciesValid = ValidateModuleDependencies();
    
    // Check performance targets
    bool bPerformanceValid = EnforcePerformanceTargets();
    
    // Get current metrics
    Result.ActorCount = GetCurrentActorCount();
    Result.DinosaurCount = 0; // Will be calculated by specific systems
    Result.PerformanceScore = GetCurrentFPS() / PerformanceTargets.TargetFPS;
    
    Result.bIsValid = bDependenciesValid && bPerformanceValid;
    
    if (Result.bIsValid)
    {
        Result.ValidationMessage = TEXT("Architecture validation passed");
    }
    else
    {
        Result.ValidationMessage = TEXT("Architecture validation failed - check dependencies and performance");
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Architecture Validation: %s (Score: %.2f)"), 
           *Result.ValidationMessage, Result.PerformanceScore);
    
    return Result;
}

bool UEng_TechnicalArchitecture::ValidateModuleDependencies()
{
    ValidateCircularDependencies();
    
    // Check if all dependencies are satisfied
    for (const FEng_ModuleDependency& Module : RegisteredModules)
    {
        for (const FString& Dependency : Module.Dependencies)
        {
            bool bDependencyFound = false;
            for (const FEng_ModuleDependency& CheckModule : RegisteredModules)
            {
                if (CheckModule.ModuleName == Dependency)
                {
                    bDependencyFound = true;
                    break;
                }
            }
            
            if (!bDependencyFound)
            {
                UE_LOG(LogTemp, Error, TEXT("Module %s has unresolved dependency: %s"), 
                       *Module.ModuleName, *Dependency);
                return false;
            }
        }
    }
    
    return true;
}

void UEng_TechnicalArchitecture::ValidateCircularDependencies()
{
    // Simple cycle detection using DFS
    TSet<FString> Visiting;
    TSet<FString> Visited;
    
    for (const FEng_ModuleDependency& Module : RegisteredModules)
    {
        if (!Visited.Contains(Module.ModuleName))
        {
            TFunction<bool(const FString&)> DFS = [&](const FString& ModuleName) -> bool
            {
                if (Visiting.Contains(ModuleName))
                {
                    UE_LOG(LogTemp, Error, TEXT("Circular dependency detected involving module: %s"), *ModuleName);
                    return false;
                }
                
                if (Visited.Contains(ModuleName))
                {
                    return true;
                }
                
                Visiting.Add(ModuleName);
                
                // Find module and check its dependencies
                for (const FEng_ModuleDependency& Mod : RegisteredModules)
                {
                    if (Mod.ModuleName == ModuleName)
                    {
                        for (const FString& Dep : Mod.Dependencies)
                        {
                            if (!DFS(Dep))
                            {
                                return false;
                            }
                        }
                        break;
                    }
                }
                
                Visiting.Remove(ModuleName);
                Visited.Add(ModuleName);
                return true;
            };
            
            DFS(Module.ModuleName);
        }
    }
}

bool UEng_TechnicalArchitecture::EnforcePerformanceTargets()
{
    return CheckPerformanceThresholds();
}

bool UEng_TechnicalArchitecture::CheckPerformanceThresholds()
{
    bool bWithinTargets = true;
    
    // Check actor count
    int32 CurrentActors = GetCurrentActorCount();
    if (CurrentActors > PerformanceTargets.MaxActors)
    {
        UE_LOG(LogTemp, Warning, TEXT("Actor count (%d) exceeds target (%d)"), 
               CurrentActors, PerformanceTargets.MaxActors);
        bWithinTargets = false;
    }
    
    // Check FPS
    float CurrentFPS = GetCurrentFPS();
    if (CurrentFPS < PerformanceTargets.TargetFPS * 0.8f) // 80% threshold
    {
        UE_LOG(LogTemp, Warning, TEXT("FPS (%.1f) below target (%.1f)"), 
               CurrentFPS, PerformanceTargets.TargetFPS);
        bWithinTargets = false;
    }
    
    return bWithinTargets;
}

void UEng_TechnicalArchitecture::RegisterModule(const FString& ModuleName, const TArray<FString>& Dependencies, int32 Priority)
{
    FEng_ModuleDependency NewModule;
    NewModule.ModuleName = ModuleName;
    NewModule.Dependencies = Dependencies;
    NewModule.Priority = Priority;
    NewModule.bIsInitialized = false;
    
    RegisteredModules.Add(NewModule);
    ModuleInitializationStatus.Add(ModuleName, false);
    
    UE_LOG(LogTemp, Log, TEXT("Registered module: %s (Priority: %d)"), *ModuleName, Priority);
}

TArray<FEng_ModuleDependency> UEng_TechnicalArchitecture::GetModuleLoadOrder()
{
    TArray<FEng_ModuleDependency> SortedModules = RegisteredModules;
    
    // Sort by priority (highest first)
    SortedModules.Sort([](const FEng_ModuleDependency& A, const FEng_ModuleDependency& B)
    {
        return A.Priority > B.Priority;
    });
    
    return SortedModules;
}

bool UEng_TechnicalArchitecture::InitializeModule(const FString& ModuleName)
{
    // Check if dependencies are satisfied
    for (const FEng_ModuleDependency& Module : RegisteredModules)
    {
        if (Module.ModuleName == ModuleName)
        {
            for (const FString& Dependency : Module.Dependencies)
            {
                bool* bInitialized = ModuleInitializationStatus.Find(Dependency);
                if (!bInitialized || !*bInitialized)
                {
                    UE_LOG(LogTemp, Error, TEXT("Cannot initialize %s: dependency %s not initialized"), 
                           *ModuleName, *Dependency);
                    return false;
                }
            }
            break;
        }
    }
    
    ModuleInitializationStatus[ModuleName] = true;
    UE_LOG(LogTemp, Warning, TEXT("Module initialized: %s"), *ModuleName);
    return true;
}

void UEng_TechnicalArchitecture::SetPerformanceTargets(const FEng_PerformanceTargets& NewTargets)
{
    PerformanceTargets = NewTargets;
    UE_LOG(LogTemp, Warning, TEXT("Performance targets updated - FPS: %.1f, Actors: %d"), 
           PerformanceTargets.TargetFPS, PerformanceTargets.MaxActors);
}

float UEng_TechnicalArchitecture::GetCurrentFPS()
{
    // Get FPS from engine stats
    if (GEngine && GEngine->GetWorldContexts().Num() > 0)
    {
        return 1.0f / GEngine->GetWorldContexts()[0].World()->GetDeltaSeconds();
    }
    return 60.0f; // Default fallback
}

int32 UEng_TechnicalArchitecture::GetCurrentActorCount()
{
    if (GEngine && GEngine->GetWorldContexts().Num() > 0)
    {
        UWorld* World = GEngine->GetWorldContexts()[0].World();
        if (World)
        {
            int32 Count = 0;
            for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
            {
                Count++;
            }
            return Count;
        }
    }
    return 0;
}

float UEng_TechnicalArchitecture::GetCurrentMemoryUsageGB()
{
    // Get memory stats (simplified)
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    return static_cast<float>(MemStats.UsedPhysical) / (1024.0f * 1024.0f * 1024.0f);
}

bool UEng_TechnicalArchitecture::ValidateNamingConventions(const FString& ClassName)
{
    // Check for proper prefixes
    if (ClassName.StartsWith(TEXT("A")) || ClassName.StartsWith(TEXT("U")) || 
        ClassName.StartsWith(TEXT("F")) || ClassName.StartsWith(TEXT("E")))
    {
        return true;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Class name %s does not follow UE5 naming conventions"), *ClassName);
    return false;
}

bool UEng_TechnicalArchitecture::ValidateIncludeStructure(const FString& HeaderPath)
{
    // Basic validation - check if .generated.h is present
    return HeaderPath.Contains(TEXT(".generated.h"));
}

TArray<FString> UEng_TechnicalArchitecture::GetCodingStandardsViolations()
{
    TArray<FString> Violations;
    
    // This would be expanded to check actual code files
    // For now, return example violations
    Violations.Add(TEXT("Example: Missing TRANSPERSONALGAME_API on exported class"));
    Violations.Add(TEXT("Example: .generated.h not last include"));
    
    return Violations;
}

// World Architecture Implementation
void UEng_WorldArchitecture::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogTemp, Warning, TEXT("World Architecture Subsystem Initialized"));
}

void UEng_WorldArchitecture::PostInitialize()
{
    Super::PostInitialize();
    
    // Get reference to technical architecture
    if (UGameInstance* GameInstance = GetWorld()->GetGameInstance())
    {
        TechnicalArchitecture = GameInstance->GetSubsystem<UEng_TechnicalArchitecture>();
    }
    
    ValidateWorldState();
}

void UEng_WorldArchitecture::ValidateWorldState()
{
    if (TechnicalArchitecture.IsValid())
    {
        FEng_ArchitectureValidation Validation = TechnicalArchitecture->ValidateArchitecture();
        UE_LOG(LogTemp, Warning, TEXT("World validation: %s"), *Validation.ValidationMessage);
    }
}

bool UEng_WorldArchitecture::EnforceActorLimits()
{
    if (TechnicalArchitecture.IsValid())
    {
        return TechnicalArchitecture->EnforcePerformanceTargets();
    }
    return false;
}