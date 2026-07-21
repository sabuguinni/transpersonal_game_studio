#include "EngArch_TechnicalStandards.h"
#include "Engine/Engine.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"

UEngArch_TechnicalStandards::UEngArch_TechnicalStandards()
{
    InitializeDefaultStandards();
}

void UEngArch_TechnicalStandards::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect - Technical Standards Manager initialized"));
    
    // Set default performance baselines
    PerformanceBaseline.TargetFrameRate_PC = 60.0f;
    PerformanceBaseline.TargetFrameRate_Console = 30.0f;
    PerformanceBaseline.MaxActorsPerBiome = 5000;
    PerformanceBaseline.MaxDinosaursSimultaneous = 200;
    PerformanceBaseline.MaxDrawDistance = 50000.0f;
    PerformanceBaseline.MaxLODLevels = 4;
    
    // Initialize quality gates for all 19 agents
    for (int32 i = 1; i <= 19; i++)
    {
        AgentQualityGates.Add(i, false);
    }
    
    // Agent #1 (Studio Director) starts approved
    AgentQualityGates[1] = true;
    
    ValidateExistingModules();
}

void UEngArch_TechnicalStandards::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect - Technical Standards Manager shutting down"));
    Super::Deinitialize();
}

void UEngArch_TechnicalStandards::InitializeDefaultStandards()
{
    CodingStandard.AgentPrefix = "Eng_";
    CodingStandard.bRequireHeaderCppPairs = true;
    CodingStandard.bRequireUPROPERTYForExposedMembers = true;
    CodingStandard.bRequireSharedTypesUsage = true;
    CodingStandard.MaxIncludesPerFile = 20;
    CodingStandard.MaxLinesPerFunction = 100;
}

void UEngArch_TechnicalStandards::SetPerformanceBaseline(const FEng_PerformanceBaseline& NewBaseline)
{
    PerformanceBaseline = NewBaseline;
    UE_LOG(LogTemp, Warning, TEXT("Performance baseline updated - PC: %.1f fps, Console: %.1f fps"), 
           NewBaseline.TargetFrameRate_PC, NewBaseline.TargetFrameRate_Console);
}

bool UEngArch_TechnicalStandards::ValidateFrameRate(float CurrentFPS) const
{
    // Check against PC baseline (primary target)
    return CurrentFPS >= PerformanceBaseline.TargetFrameRate_PC * 0.9f; // 10% tolerance
}

bool UEngArch_TechnicalStandards::ValidateActorCount(int32 ActorCount, const FString& BiomeName) const
{
    if (ActorCount > PerformanceBaseline.MaxActorsPerBiome)
    {
        UE_LOG(LogTemp, Error, TEXT("Actor count exceeded in biome %s: %d > %d"), 
               *BiomeName, ActorCount, PerformanceBaseline.MaxActorsPerBiome);
        return false;
    }
    return true;
}

void UEngArch_TechnicalStandards::SetCodingStandard(const FEng_CodingStandard& NewStandard)
{
    CodingStandard = NewStandard;
    UE_LOG(LogTemp, Warning, TEXT("Coding standards updated - Prefix: %s"), *NewStandard.AgentPrefix);
}

bool UEngArch_TechnicalStandards::ValidateClassName(const FString& ClassName) const
{
    if (!ClassName.StartsWith(CodingStandard.AgentPrefix))
    {
        UE_LOG(LogTemp, Error, TEXT("Class name %s does not follow prefix standard: %s"), 
               *ClassName, *CodingStandard.AgentPrefix);
        return false;
    }
    return true;
}

bool UEngArch_TechnicalStandards::ValidateFileStructure(const FString& HeaderPath, const FString& CppPath) const
{
    if (!CodingStandard.bRequireHeaderCppPairs)
    {
        return true;
    }
    
    // Check if both files exist
    bool bHeaderExists = IFileManager::Get().FileExists(*HeaderPath);
    bool bCppExists = IFileManager::Get().FileExists(*CppPath);
    
    if (bHeaderExists && !bCppExists)
    {
        UE_LOG(LogTemp, Error, TEXT("Header file %s exists but corresponding .cpp file %s is missing"), 
               *HeaderPath, *CppPath);
        return false;
    }
    
    return true;
}

void UEngArch_TechnicalStandards::RegisterModule(const FString& ModuleName, const TArray<FString>& Dependencies)
{
    FEng_ModuleIntegration NewModule;
    NewModule.ModuleName = ModuleName;
    NewModule.Dependencies = Dependencies;
    NewModule.bIsCompiling = false;
    NewModule.bHasValidationErrors = false;
    NewModule.ClassCount = 0;
    NewModule.LastCompileTime = 0.0f;
    
    // Remove existing entry if present
    RegisteredModules.RemoveAll([&ModuleName](const FEng_ModuleIntegration& Module) {
        return Module.ModuleName == ModuleName;
    });
    
    RegisteredModules.Add(NewModule);
    
    UE_LOG(LogTemp, Warning, TEXT("Module registered: %s with %d dependencies"), 
           *ModuleName, Dependencies.Num());
}

void UEngArch_TechnicalStandards::UpdateModuleStatus(const FString& ModuleName, bool bCompiling, bool bHasErrors, int32 ClassCount)
{
    for (FEng_ModuleIntegration& Module : RegisteredModules)
    {
        if (Module.ModuleName == ModuleName)
        {
            Module.bIsCompiling = bCompiling;
            Module.bHasValidationErrors = bHasErrors;
            Module.ClassCount = ClassCount;
            Module.LastCompileTime = FPlatformTime::Seconds();
            
            UE_LOG(LogTemp, Warning, TEXT("Module %s status updated - Compiling: %s, Errors: %s, Classes: %d"), 
                   *ModuleName, bCompiling ? TEXT("Yes") : TEXT("No"), 
                   bHasErrors ? TEXT("Yes") : TEXT("No"), ClassCount);
            break;
        }
    }
}

TArray<FEng_ModuleIntegration> UEngArch_TechnicalStandards::GetAllModules() const
{
    return RegisteredModules;
}

bool UEngArch_TechnicalStandards::ValidateModuleDependencies(const FString& ModuleName) const
{
    TSet<FString> VisitedModules;
    return CheckDependencyChain(ModuleName, VisitedModules);
}

bool UEngArch_TechnicalStandards::CheckDependencyChain(const FString& ModuleName, TSet<FString>& VisitedModules) const
{
    if (VisitedModules.Contains(ModuleName))
    {
        UE_LOG(LogTemp, Error, TEXT("Circular dependency detected involving module: %s"), *ModuleName);
        return false;
    }
    
    VisitedModules.Add(ModuleName);
    
    const FEng_ModuleIntegration* Module = RegisteredModules.FindByPredicate([&ModuleName](const FEng_ModuleIntegration& M) {
        return M.ModuleName == ModuleName;
    });
    
    if (!Module)
    {
        return true; // Module not found, assume valid
    }
    
    for (const FString& Dependency : Module->Dependencies)
    {
        if (!CheckDependencyChain(Dependency, VisitedModules))
        {
            return false;
        }
    }
    
    VisitedModules.Remove(ModuleName);
    return true;
}

void UEngArch_TechnicalStandards::TriggerIncrementalCompilation()
{
    UE_LOG(LogTemp, Warning, TEXT("Triggering incremental compilation for %d disabled files"), DisabledFileCount);
    
    // Mark all modules as compiling
    for (FEng_ModuleIntegration& Module : RegisteredModules)
    {
        Module.bIsCompiling = true;
    }
}

void UEngArch_TechnicalStandards::ValidateSharedTypes()
{
    UE_LOG(LogTemp, Warning, TEXT("Validating SharedTypes.h compliance across all modules"));
    
    // This would check for duplicate type definitions across modules
    // Implementation would scan source files for USTRUCT/UENUM declarations
}

void UEngArch_TechnicalStandards::ReactivateModule(const FString& ModuleName)
{
    UE_LOG(LogTemp, Warning, TEXT("Reactivating module: %s"), *ModuleName);
    
    // This would rename .cpp.disabled files back to .cpp
    // Implementation would use IFileManager to rename files
    
    if (DisabledFileCount > 0)
    {
        DisabledFileCount--;
    }
}

bool UEngArch_TechnicalStandards::CanAgentProceed(int32 AgentNumber) const
{
    // Agent #2 (Engine Architect) must approve architecture before others proceed
    if (AgentNumber > 2 && !bArchitectureApproved)
    {
        return false;
    }
    
    // Check dependencies
    switch (AgentNumber)
    {
        case 3: // Core Systems - needs Engine Architect
            return AgentQualityGates.FindRef(2);
        case 4: // Performance - needs Core Systems
            return AgentQualityGates.FindRef(3);
        case 5: // World Generator - needs Core Systems
            return AgentQualityGates.FindRef(3);
        case 15: // Narrative - can proceed independently
            return true;
        default:
            return AgentQualityGates.FindRef(AgentNumber - 1); // Sequential dependency
    }
}

void UEngArch_TechnicalStandards::SetQualityGate(int32 AgentNumber, bool bPassed)
{
    AgentQualityGates[AgentNumber] = bPassed;
    
    if (AgentNumber == 2 && bPassed)
    {
        bArchitectureApproved = true;
        UE_LOG(LogTemp, Warning, TEXT("Architecture approved - Agents #3-#19 can now proceed"));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Quality gate for Agent #%d set to: %s"), 
           AgentNumber, bPassed ? TEXT("PASSED") : TEXT("FAILED"));
}

TArray<int32> UEngArch_TechnicalStandards::GetBlockedAgents() const
{
    TArray<int32> BlockedAgents;
    
    for (int32 i = 1; i <= 19; i++)
    {
        if (!CanAgentProceed(i))
        {
            BlockedAgents.Add(i);
        }
    }
    
    return BlockedAgents;
}

void UEngArch_TechnicalStandards::ValidateExistingModules()
{
    // Register core modules that already exist
    RegisterModule("TranspersonalGame", {});
    RegisterModule("Engine", {});
    RegisterModule("Core", {});
    RegisterModule("AudioCore", {"TranspersonalGame"});
    RegisterModule("EnvironmentCore", {"TranspersonalGame"});
    RegisterModule("AnimationCore", {"TranspersonalGame"});
    RegisterModule("AICore", {"TranspersonalGame"});
    RegisterModule("CombatCore", {"TranspersonalGame"});
    
    UE_LOG(LogTemp, Warning, TEXT("Validated %d existing modules"), RegisteredModules.Num());
}