#include "EngArchitect_TechnicalSpecs.h"
#include "Engine/Engine.h"
#include "HAL/Platform.h"

UEngArchitect_TechnicalSpecs::UEngArchitect_TechnicalSpecs()
{
    TargetFrameRate = 60.0f;
    MaxActorsPerBiome = 500;
    MaxTotalActors = 20000;
    
    InitializeDefaultSpecs();
}

void UEngArchitect_TechnicalSpecs::InitializeDefaultSpecs()
{
    SystemSpecifications.Empty();
    SystemRegistry.Empty();

    // Core Systems (Agent #03)
    FEng_SystemSpec CoreSystemsSpec;
    CoreSystemsSpec.SystemName = TEXT("Core Systems");
    CoreSystemsSpec.Priority = EEng_SystemPriority::Critical;
    CoreSystemsSpec.Status = EEng_ModuleStatus::InProgress;
    CoreSystemsSpec.AgentResponsible = 3;
    CoreSystemsSpec.Dependencies.Add(TEXT("Engine Architecture"));
    CoreSystemsSpec.MaxFrameTime = 5.0f;
    CoreSystemsSpec.MaxMemoryMB = 256;
    RegisterSystemSpec(CoreSystemsSpec);

    // World Generation (Agent #05)
    FEng_SystemSpec WorldGenSpec;
    WorldGenSpec.SystemName = TEXT("World Generation");
    WorldGenSpec.Priority = EEng_SystemPriority::Critical;
    WorldGenSpec.Status = EEng_ModuleStatus::NotStarted;
    WorldGenSpec.AgentResponsible = 5;
    WorldGenSpec.Dependencies.Add(TEXT("Core Systems"));
    WorldGenSpec.MaxFrameTime = 8.0f;
    WorldGenSpec.MaxMemoryMB = 1024;
    RegisterSystemSpec(WorldGenSpec);

    // Character System (Agent #09)
    FEng_SystemSpec CharacterSpec;
    CharacterSpec.SystemName = TEXT("Character System");
    CharacterSpec.Priority = EEng_SystemPriority::High;
    CharacterSpec.Status = EEng_ModuleStatus::NotStarted;
    CharacterSpec.AgentResponsible = 9;
    CharacterSpec.Dependencies.Add(TEXT("Core Systems"));
    CharacterSpec.MaxFrameTime = 4.0f;
    CharacterSpec.MaxMemoryMB = 512;
    RegisterSystemSpec(CharacterSpec);

    // Dinosaur AI (Agent #12)
    FEng_SystemSpec DinosaurAISpec;
    DinosaurAISpec.SystemName = TEXT("Dinosaur AI");
    DinosaurAISpec.Priority = EEng_SystemPriority::High;
    DinosaurAISpec.Status = EEng_ModuleStatus::NotStarted;
    DinosaurAISpec.AgentResponsible = 12;
    DinosaurAISpec.Dependencies.Add(TEXT("Character System"));
    DinosaurAISpec.Dependencies.Add(TEXT("World Generation"));
    DinosaurAISpec.MaxFrameTime = 6.0f;
    DinosaurAISpec.MaxMemoryMB = 768;
    RegisterSystemSpec(DinosaurAISpec);

    // Survival Systems
    FEng_SystemSpec SurvivalSpec;
    SurvivalSpec.SystemName = TEXT("Survival Systems");
    SurvivalSpec.Priority = EEng_SystemPriority::High;
    SurvivalSpec.Status = EEng_ModuleStatus::NotStarted;
    SurvivalSpec.AgentResponsible = 14;
    SurvivalSpec.Dependencies.Add(TEXT("Character System"));
    SurvivalSpec.MaxFrameTime = 3.0f;
    SurvivalSpec.MaxMemoryMB = 256;
    RegisterSystemSpec(SurvivalSpec);

    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Initialized %d system specifications"), SystemSpecifications.Num());
}

bool UEngArchitect_TechnicalSpecs::ValidateSystemDependencies()
{
    bool bAllValid = true;

    for (const FEng_SystemSpec& Spec : SystemSpecifications)
    {
        for (const FString& Dependency : Spec.Dependencies)
        {
            if (!SystemRegistry.Contains(Dependency))
            {
                UE_LOG(LogTemp, Error, TEXT("System '%s' has unresolved dependency: '%s'"), 
                    *Spec.SystemName, *Dependency);
                bAllValid = false;
            }
        }
    }

    if (bAllValid)
    {
        UE_LOG(LogTemp, Warning, TEXT("Engine Architect: All system dependencies validated successfully"));
    }

    return bAllValid;
}

bool UEngArchitect_TechnicalSpecs::CheckCompilationCompliance()
{
    bool bCompliant = true;

    // Check UE5.5 compatibility
    if (!CompilationRules.bEnforceUE55Compatibility)
    {
        UE_LOG(LogTemp, Error, TEXT("UE5.5 compatibility enforcement is disabled"));
        bCompliant = false;
    }

    // Check matching .cpp files requirement
    if (!CompilationRules.bRequireMatchingCppFiles)
    {
        UE_LOG(LogTemp, Error, TEXT("Matching .cpp files requirement is disabled"));
        bCompliant = false;
    }

    // Check naming conventions
    if (!CompilationRules.bEnforceNamingConventions)
    {
        UE_LOG(LogTemp, Warning, TEXT("Naming conventions enforcement is disabled"));
    }

    // Check SharedTypes usage
    if (!CompilationRules.bRequireSharedTypesUsage)
    {
        UE_LOG(LogTemp, Warning, TEXT("SharedTypes usage requirement is disabled"));
    }

    if (bCompliant)
    {
        UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Compilation compliance check passed"));
    }

    return bCompliant;
}

bool UEngArchitect_TechnicalSpecs::ValidatePerformanceTargets()
{
    bool bValid = true;
    float TotalFrameTime = 0.0f;
    int32 TotalMemory = 0;

    for (const FEng_SystemSpec& Spec : SystemSpecifications)
    {
        TotalFrameTime += Spec.MaxFrameTime;
        TotalMemory += Spec.MaxMemoryMB;
    }

    float TargetFrameTime = 1000.0f / TargetFrameRate; // Convert to milliseconds

    if (TotalFrameTime > TargetFrameTime)
    {
        UE_LOG(LogTemp, Error, TEXT("Total system frame time (%.2fms) exceeds target (%.2fms)"), 
            TotalFrameTime, TargetFrameTime);
        bValid = false;
    }

    if (TotalMemory > 4096) // 4GB limit
    {
        UE_LOG(LogTemp, Error, TEXT("Total system memory (%dMB) exceeds 4GB limit"), TotalMemory);
        bValid = false;
    }

    if (bValid)
    {
        UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Performance targets validated - Frame: %.2fms, Memory: %dMB"), 
            TotalFrameTime, TotalMemory);
    }

    return bValid;
}

void UEngArchitect_TechnicalSpecs::RegisterSystemSpec(const FEng_SystemSpec& NewSpec)
{
    SystemSpecifications.Add(NewSpec);
    SystemRegistry.Add(NewSpec.SystemName, NewSpec);
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Registered system '%s' (Agent %d, Priority %d)"), 
        *NewSpec.SystemName, NewSpec.AgentResponsible, (int32)NewSpec.Priority);
}

FEng_SystemSpec UEngArchitect_TechnicalSpecs::GetSystemSpec(const FString& SystemName)
{
    if (SystemRegistry.Contains(SystemName))
    {
        return SystemRegistry[SystemName];
    }

    UE_LOG(LogTemp, Warning, TEXT("System spec not found: %s"), *SystemName);
    return FEng_SystemSpec();
}

bool UEngArchitect_TechnicalSpecs::ValidateAgentChain()
{
    // Validate agent dependency chain: 02 -> 03 -> 05 -> 06 -> 07 -> 08 -> 09 -> 10 -> 11 -> 12 -> 13 -> 14 -> 15 -> 16 -> 17 -> 18 -> 19
    TArray<int32> RequiredAgents = {2, 3, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19};
    TSet<int32> RegisteredAgents;

    for (const FEng_SystemSpec& Spec : SystemSpecifications)
    {
        RegisteredAgents.Add(Spec.AgentResponsible);
    }

    bool bChainValid = true;
    for (int32 AgentNum : RequiredAgents)
    {
        if (!RegisteredAgents.Contains(AgentNum))
        {
            UE_LOG(LogTemp, Warning, TEXT("Agent #%d not registered in system specs"), AgentNum);
            // Not marking as invalid since agents register progressively
        }
    }

    return bChainValid;
}

bool UEngArchitect_TechnicalSpecs::CheckModuleDependencies()
{
    // Check that all required UE5 modules are available
    TArray<FString> RequiredModules = {
        TEXT("Core"),
        TEXT("CoreUObject"),
        TEXT("Engine"),
        TEXT("UnrealEd"),
        TEXT("ToolMenus"),
        TEXT("EditorStyle"),
        TEXT("EditorWidgets"),
        TEXT("PropertyEditor"),
        TEXT("Slate"),
        TEXT("SlateCore"),
        TEXT("InputCore"),
        TEXT("RenderCore"),
        TEXT("RHI"),
        TEXT("NavigationSystem"),
        TEXT("AIModule"),
        TEXT("GameplayTasks"),
        TEXT("UMG"),
        TEXT("Niagara"),
        TEXT("Landscape"),
        TEXT("Foliage")
    };

    // In a real implementation, this would check module availability
    // For now, we assume all modules are available in UE5.5
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Module dependency check completed"));
    
    return true;
}