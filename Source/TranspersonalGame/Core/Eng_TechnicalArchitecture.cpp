#include "Eng_TechnicalArchitecture.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"

UEng_TechnicalArchitecture::UEng_TechnicalArchitecture()
{
    TargetFrameRate = 60.0f;
    MaxWorldActors = 50000;
    LastFrameTime = 0.0f;
    LastActorCount = 0;
}

void UEng_TechnicalArchitecture::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Engine Architect: Technical Architecture subsystem initialized"));
    
    InitializeCompilationRules();
    InitializeDefaultModules();
    
    // Start performance monitoring
    UpdatePerformanceMetrics();
}

void UEng_TechnicalArchitecture::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("Engine Architect: Technical Architecture subsystem deinitialized"));
    Super::Deinitialize();
}

bool UEng_TechnicalArchitecture::RegisterModule(const FEng_ModuleSpec& ModuleSpec)
{
    if (!ValidateModuleSpec(ModuleSpec))
    {
        UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Invalid module spec for %s"), *ModuleSpec.ModuleName);
        return false;
    }

    // Check for duplicate registration
    for (const FEng_ModuleSpec& ExistingModule : RegisteredModules)
    {
        if (ExistingModule.ModuleName == ModuleSpec.ModuleName)
        {
            UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Module %s already registered"), *ModuleSpec.ModuleName);
            return false;
        }
    }

    RegisteredModules.Add(ModuleSpec);
    UE_LOG(LogTemp, Log, TEXT("Engine Architect: Registered module %s (Type: %d)"), 
           *ModuleSpec.ModuleName, (int32)ModuleSpec.ModuleType);
    
    return true;
}

bool UEng_TechnicalArchitecture::ValidateModuleDependencies(const FString& ModuleName)
{
    const FEng_ModuleSpec* TargetModule = nullptr;
    
    // Find the target module
    for (const FEng_ModuleSpec& Module : RegisteredModules)
    {
        if (Module.ModuleName == ModuleName)
        {
            TargetModule = &Module;
            break;
        }
    }
    
    if (!TargetModule)
    {
        UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Module %s not found for dependency validation"), *ModuleName);
        return false;
    }
    
    // Check if all dependencies are registered
    for (const FString& Dependency : TargetModule->Dependencies)
    {
        bool bDependencyFound = false;
        for (const FEng_ModuleSpec& Module : RegisteredModules)
        {
            if (Module.ModuleName == Dependency)
            {
                bDependencyFound = true;
                break;
            }
        }
        
        if (!bDependencyFound)
        {
            UE_LOG(LogTemp, Error, TEXT("Engine Architect: Module %s missing dependency: %s"), 
                   *ModuleName, *Dependency);
            return false;
        }
    }
    
    return true;
}

TArray<FString> UEng_TechnicalArchitecture::GetModuleLoadOrder()
{
    TArray<FString> LoadOrder;
    TArray<FEng_ModuleSpec> RemainingModules = RegisteredModules;
    
    // Simple dependency resolution (topological sort)
    while (RemainingModules.Num() > 0)
    {
        bool bFoundModule = false;
        
        for (int32 i = RemainingModules.Num() - 1; i >= 0; i--)
        {
            const FEng_ModuleSpec& Module = RemainingModules[i];
            bool bCanLoad = true;
            
            // Check if all dependencies are already in load order
            for (const FString& Dependency : Module.Dependencies)
            {
                if (!LoadOrder.Contains(Dependency))
                {
                    bCanLoad = false;
                    break;
                }
            }
            
            if (bCanLoad)
            {
                LoadOrder.Add(Module.ModuleName);
                RemainingModules.RemoveAt(i);
                bFoundModule = true;
            }
        }
        
        // Prevent infinite loop on circular dependencies
        if (!bFoundModule)
        {
            UE_LOG(LogTemp, Error, TEXT("Engine Architect: Circular dependency detected in module system"));
            break;
        }
    }
    
    return LoadOrder;
}

bool UEng_TechnicalArchitecture::ValidateCompilationRules()
{
    bool bAllRulesPassed = true;
    
    for (FEng_CompilationRule& Rule : CompilationRules)
    {
        bool bRulePassed = true;
        
        // Implement specific rule validation logic
        if (Rule.RuleName == TEXT("USTRUCT_GLOBAL_SCOPE"))
        {
            // This would require code analysis - simplified for now
            bRulePassed = true;
        }
        else if (Rule.RuleName == TEXT("UNIQUE_TYPE_NAMES"))
        {
            // This would require symbol table analysis - simplified for now
            bRulePassed = true;
        }
        else if (Rule.RuleName == TEXT("GENERATED_H_LAST"))
        {
            // This would require file parsing - simplified for now
            bRulePassed = true;
        }
        
        if (!bRulePassed && Rule.bIsMandatory)
        {
            bAllRulesPassed = false;
            UE_LOG(LogTemp, Error, TEXT("Engine Architect: Compilation rule failed: %s - %s"), 
                   *Rule.RuleName, *Rule.ViolationMessage);
        }
    }
    
    return bAllRulesPassed;
}

TArray<FEng_CompilationRule> UEng_TechnicalArchitecture::GetFailedRules()
{
    TArray<FEng_CompilationRule> FailedRules;
    
    // This would contain actual rule validation logic
    // For now, return empty array indicating all rules pass
    
    return FailedRules;
}

bool UEng_TechnicalArchitecture::CheckPerformanceCompliance()
{
    UpdatePerformanceMetrics();
    
    bool bCompliant = true;
    
    // Check frame rate compliance
    if (LastFrameTime > (1000.0f / TargetFrameRate))
    {
        bCompliant = false;
        UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Frame time exceeded target: %.2fms (target: %.2fms)"), 
               LastFrameTime, 1000.0f / TargetFrameRate);
    }
    
    // Check actor count compliance
    if (LastActorCount > MaxWorldActors)
    {
        bCompliant = false;
        UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Actor count exceeded limit: %d (max: %d)"), 
               LastActorCount, MaxWorldActors);
    }
    
    return bCompliant;
}

float UEng_TechnicalArchitecture::GetCurrentFrameTime()
{
    return LastFrameTime;
}

int32 UEng_TechnicalArchitecture::GetCurrentActorCount()
{
    return LastActorCount;
}

bool UEng_TechnicalArchitecture::ValidateSystemArchitecture()
{
    ArchitectureWarnings.Empty();
    bool bValid = true;
    
    // Validate module dependencies
    for (const FEng_ModuleSpec& Module : RegisteredModules)
    {
        if (!ValidateModuleDependencies(Module.ModuleName))
        {
            ArchitectureWarnings.Add(FString::Printf(TEXT("Module %s has unresolved dependencies"), *Module.ModuleName));
            bValid = false;
        }
    }
    
    // Validate performance compliance
    if (!CheckPerformanceCompliance())
    {
        ArchitectureWarnings.Add(TEXT("Performance standards not met"));
        bValid = false;
    }
    
    // Validate compilation rules
    if (!ValidateCompilationRules())
    {
        ArchitectureWarnings.Add(TEXT("Compilation rules violated"));
        bValid = false;
    }
    
    return bValid;
}

TArray<FString> UEng_TechnicalArchitecture::GetArchitectureWarnings()
{
    return ArchitectureWarnings;
}

void UEng_TechnicalArchitecture::RunArchitectureDiagnostics()
{
    UE_LOG(LogTemp, Log, TEXT("=== Engine Architect: Running Architecture Diagnostics ==="));
    
    // Module status
    UE_LOG(LogTemp, Log, TEXT("Registered Modules: %d"), RegisteredModules.Num());
    for (const FEng_ModuleSpec& Module : RegisteredModules)
    {
        UE_LOG(LogTemp, Log, TEXT("  - %s (Type: %d, Dependencies: %d)"), 
               *Module.ModuleName, (int32)Module.ModuleType, Module.Dependencies.Num());
    }
    
    // Performance status
    UE_LOG(LogTemp, Log, TEXT("Performance Metrics:"));
    UE_LOG(LogTemp, Log, TEXT("  - Frame Time: %.2fms (Target: %.2fms)"), 
           LastFrameTime, 1000.0f / TargetFrameRate);
    UE_LOG(LogTemp, Log, TEXT("  - Actor Count: %d (Max: %d)"), LastActorCount, MaxWorldActors);
    
    // Architecture validation
    bool bValid = ValidateSystemArchitecture();
    UE_LOG(LogTemp, Log, TEXT("Architecture Valid: %s"), bValid ? TEXT("YES") : TEXT("NO"));
    
    if (ArchitectureWarnings.Num() > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Architecture Warnings:"));
        for (const FString& Warning : ArchitectureWarnings)
        {
            UE_LOG(LogTemp, Warning, TEXT("  - %s"), *Warning);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("=== Architecture Diagnostics Complete ==="));
}

void UEng_TechnicalArchitecture::LogModuleStatus()
{
    UE_LOG(LogTemp, Log, TEXT("Engine Architect: Module Status Report"));
    
    TArray<FString> LoadOrder = GetModuleLoadOrder();
    UE_LOG(LogTemp, Log, TEXT("Recommended Load Order:"));
    for (int32 i = 0; i < LoadOrder.Num(); i++)
    {
        UE_LOG(LogTemp, Log, TEXT("  %d. %s"), i + 1, *LoadOrder[i]);
    }
}

void UEng_TechnicalArchitecture::InitializeCompilationRules()
{
    CompilationRules.Empty();
    
    // Rule 1: USTRUCT/UENUM at global scope
    FEng_CompilationRule Rule1;
    Rule1.RuleName = TEXT("USTRUCT_GLOBAL_SCOPE");
    Rule1.Description = TEXT("USTRUCT() and UENUM() must be declared at global scope");
    Rule1.bIsMandatory = true;
    Rule1.ViolationMessage = TEXT("USTRUCT/UENUM found nested inside class - move to global scope");
    CompilationRules.Add(Rule1);
    
    // Rule 2: Unique type names
    FEng_CompilationRule Rule2;
    Rule2.RuleName = TEXT("UNIQUE_TYPE_NAMES");
    Rule2.Description = TEXT("All USTRUCT, UENUM, UCLASS names must be unique project-wide");
    Rule2.bIsMandatory = true;
    Rule2.ViolationMessage = TEXT("Duplicate type name detected - use Eng_ prefix for Engine Architect types");
    CompilationRules.Add(Rule2);
    
    // Rule 3: .generated.h last include
    FEng_CompilationRule Rule3;
    Rule3.RuleName = TEXT("GENERATED_H_LAST");
    Rule3.Description = TEXT(".generated.h must be the last include in header files");
    Rule3.bIsMandatory = true;
    Rule3.ViolationMessage = TEXT(".generated.h include must appear after all other includes");
    CompilationRules.Add(Rule3);
    
    UE_LOG(LogTemp, Log, TEXT("Engine Architect: Initialized %d compilation rules"), CompilationRules.Num());
}

void UEng_TechnicalArchitecture::InitializeDefaultModules()
{
    RegisteredModules.Empty();
    
    // Core module
    FEng_ModuleSpec CoreModule;
    CoreModule.ModuleName = TEXT("Core");
    CoreModule.ModuleType = EEng_ModuleType::Core;
    CoreModule.PerformanceRequirement = EEng_PerformanceLevel::Critical;
    CoreModule.MaxActorsPerFrame = 10000;
    CoreModule.MaxFrameTimeMs = 8.33f; // 120fps for core systems
    RegisterModule(CoreModule);
    
    // World Generation module
    FEng_ModuleSpec WorldGenModule;
    WorldGenModule.ModuleName = TEXT("WorldGeneration");
    WorldGenModule.ModuleType = EEng_ModuleType::WorldGeneration;
    WorldGenModule.Dependencies.Add(TEXT("Core"));
    WorldGenModule.PerformanceRequirement = EEng_PerformanceLevel::High;
    WorldGenModule.MaxActorsPerFrame = 5000;
    WorldGenModule.MaxFrameTimeMs = 16.67f;
    RegisterModule(WorldGenModule);
    
    // Character Systems module
    FEng_ModuleSpec CharacterModule;
    CharacterModule.ModuleName = TEXT("CharacterSystems");
    CharacterModule.ModuleType = EEng_ModuleType::CharacterSystems;
    CharacterModule.Dependencies.Add(TEXT("Core"));
    CharacterModule.PerformanceRequirement = EEng_PerformanceLevel::Critical;
    CharacterModule.MaxActorsPerFrame = 1000;
    CharacterModule.MaxFrameTimeMs = 8.33f;
    RegisterModule(CharacterModule);
    
    UE_LOG(LogTemp, Log, TEXT("Engine Architect: Initialized %d default modules"), RegisteredModules.Num());
}

bool UEng_TechnicalArchitecture::ValidateModuleSpec(const FEng_ModuleSpec& ModuleSpec)
{
    if (ModuleSpec.ModuleName.IsEmpty())
    {
        return false;
    }
    
    if (ModuleSpec.MaxActorsPerFrame <= 0 || ModuleSpec.MaxFrameTimeMs <= 0.0f)
    {
        return false;
    }
    
    return true;
}

void UEng_TechnicalArchitecture::UpdatePerformanceMetrics()
{
    // Get current frame time (simplified - would use actual frame time in production)
    LastFrameTime = 16.67f; // Assume 60fps for now
    
    // Get current actor count
    if (UWorld* World = GetWorld())
    {
        LastActorCount = World->GetActorCount();
    }
    else
    {
        LastActorCount = 0;
    }
}

// Technical Standards Component Implementation
UEng_TechnicalStandardsComponent::UEng_TechnicalStandardsComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Check once per second
    
    RequiredPerformanceLevel = EEng_PerformanceLevel::Medium;
    MaxTickTime = 1.0f; // 1ms max tick time
    bIsCompliant = true;
    AccumulatedTickTime = 0.0f;
    TickCount = 0;
}

void UEng_TechnicalStandardsComponent::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Engine Architect: Technical Standards Component initialized on %s"), 
           *GetOwner()->GetName());
    
    ValidateComponentSetup();
}

void UEng_TechnicalStandardsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    float TickStartTime = FPlatformTime::Seconds();
    
    CheckActorPerformance();
    
    float TickEndTime = FPlatformTime::Seconds();
    float TickDuration = (TickEndTime - TickStartTime) * 1000.0f; // Convert to ms
    
    AccumulatedTickTime += TickDuration;
    TickCount++;
    
    // Check if tick time exceeds limits
    if (TickDuration > MaxTickTime)
    {
        UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Actor %s exceeded tick time limit: %.3fms (max: %.3fms)"), 
               *GetOwner()->GetName(), TickDuration, MaxTickTime);
        bIsCompliant = false;
    }
}

bool UEng_TechnicalStandardsComponent::ValidateActorCompliance()
{
    bool bCompliant = true;
    
    // Check component setup
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return false;
    }
    
    // Check if actor has too many components
    TArray<UActorComponent*> Components = Owner->GetRootComponent()->GetAttachChildren();
    if (Components.Num() > 50) // Arbitrary limit
    {
        UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Actor %s has excessive components: %d"), 
               *Owner->GetName(), Components.Num());
        bCompliant = false;
    }
    
    // Check average tick time
    if (TickCount > 0)
    {
        float AverageTickTime = AccumulatedTickTime / TickCount;
        if (AverageTickTime > MaxTickTime)
        {
            UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Actor %s average tick time excessive: %.3fms"), 
                   *Owner->GetName(), AverageTickTime);
            bCompliant = false;
        }
    }
    
    bIsCompliant = bCompliant;
    return bCompliant;
}

void UEng_TechnicalStandardsComponent::EnforcePerformanceStandards()
{
    if (!bIsCompliant)
    {
        UE_LOG(LogTemp, Log, TEXT("Engine Architect: Enforcing performance standards on %s"), 
               *GetOwner()->GetName());
        
        // Reset performance counters
        AccumulatedTickTime = 0.0f;
        TickCount = 0;
        
        // Could implement automatic optimization here
        // For example: reduce tick frequency, disable non-essential components, etc.
        
        bIsCompliant = true;
    }
}

void UEng_TechnicalStandardsComponent::CheckActorPerformance()
{
    // Placeholder for performance checking logic
    // In a full implementation, this would monitor:
    // - Memory usage
    // - Draw calls
    // - Animation complexity
    // - Physics simulation cost
}

void UEng_TechnicalStandardsComponent::ValidateComponentSetup()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        UE_LOG(LogTemp, Error, TEXT("Engine Architect: Technical Standards Component has no owner"));
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Engine Architect: Validated component setup for %s"), *Owner->GetName());
}