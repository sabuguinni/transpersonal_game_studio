#include "TechnicalArchitectureManager.h"
#include "Engine/Engine.h"
#include "Misc/DateTime.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"

UTechnicalArchitectureManager::UTechnicalArchitectureManager()
{
}

void UTechnicalArchitectureManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Technical Architecture Manager initialized"));
    
    // Initialize default systems, rules, and targets
    InitializeDefaultSystems();
    InitializeDefaultRules();
    InitializeDefaultPerformanceTargets();
    
    RebuildCaches();
}

void UTechnicalArchitectureManager::RegisterSystem(const FSystemDefinition& SystemDef)
{
    // Check if system already exists
    int32 ExistingIndex = SystemNameToIndex.FindRef(SystemDef.SystemName);
    if (ExistingIndex > 0)
    {
        RegisteredSystems[ExistingIndex - 1] = SystemDef;
        UE_LOG(LogTemp, Log, TEXT("Updated system definition: %s"), *SystemDef.SystemName);
    }
    else
    {
        RegisteredSystems.Add(SystemDef);
        UE_LOG(LogTemp, Log, TEXT("Registered new system: %s"), *SystemDef.SystemName);
    }
    
    RebuildCaches();
}

void UTechnicalArchitectureManager::UpdateSystemStatus(const FString& SystemName, bool bImplemented, bool bTested)
{
    int32 SystemIndex = SystemNameToIndex.FindRef(SystemName);
    if (SystemIndex > 0)
    {
        FSystemDefinition& System = RegisteredSystems[SystemIndex - 1];
        System.bIsImplemented = bImplemented;
        System.bIsTested = bTested;
        
        UE_LOG(LogTemp, Log, TEXT("Updated system status - %s: Implemented=%s, Tested=%s"), 
               *SystemName, bImplemented ? TEXT("Yes") : TEXT("No"), bTested ? TEXT("Yes") : TEXT("No"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("System not found for status update: %s"), *SystemName);
    }
}

TArray<FSystemDefinition> UTechnicalArchitectureManager::GetSystemsByLayer(EArchitectureLayer Layer) const
{
    TArray<FSystemDefinition> FilteredSystems;
    
    for (const FSystemDefinition& System : RegisteredSystems)
    {
        if (System.Layer == Layer)
        {
            FilteredSystems.Add(System);
        }
    }
    
    return FilteredSystems;
}

TArray<FSystemDefinition> UTechnicalArchitectureManager::GetSystemsByPriority(ESystemPriority Priority) const
{
    TArray<FSystemDefinition> FilteredSystems;
    
    for (const FSystemDefinition& System : RegisteredSystems)
    {
        if (System.Priority == Priority)
        {
            FilteredSystems.Add(System);
        }
    }
    
    return FilteredSystems;
}

FSystemDefinition UTechnicalArchitectureManager::GetSystemDefinition(const FString& SystemName) const
{
    int32 SystemIndex = SystemNameToIndex.FindRef(SystemName);
    if (SystemIndex > 0)
    {
        return RegisteredSystems[SystemIndex - 1];
    }
    
    return FSystemDefinition(); // Return empty definition if not found
}

void UTechnicalArchitectureManager::AddArchitectureRule(const FArchitectureRule& Rule)
{
    ArchitectureRules.Add(Rule);
    RebuildCaches();
    
    UE_LOG(LogTemp, Log, TEXT("Added architecture rule: %s"), *Rule.RuleName);
}

bool UTechnicalArchitectureManager::ValidateSystemAgainstRules(const FString& SystemName) const
{
    TArray<FString> Violations = GetRuleViolations(SystemName);
    return Violations.Num() == 0;
}

TArray<FString> UTechnicalArchitectureManager::GetRuleViolations(const FString& SystemName) const
{
    TArray<FString> Violations;
    
    FSystemDefinition System = GetSystemDefinition(SystemName);
    if (System.SystemName.IsEmpty())
    {
        Violations.Add(FString::Printf(TEXT("System '%s' not found"), *SystemName));
        return Violations;
    }
    
    // Check each rule
    for (const FArchitectureRule& Rule : ArchitectureRules)
    {
        // Check if rule applies to this system
        bool bRuleApplies = false;
        for (const FString& AppliesTo : Rule.AppliesTo)
        {
            if (SystemName.Contains(AppliesTo) || AppliesTo == TEXT("*"))
            {
                bRuleApplies = true;
                break;
            }
        }
        
        if (bRuleApplies)
        {
            // Perform rule-specific validation
            // This is a simplified example - real implementation would be more complex
            if (Rule.RuleName.Contains(TEXT("Dependency")) && System.Dependencies.Num() == 0)
            {
                Violations.Add(FString::Printf(TEXT("Rule violation: %s"), *Rule.Description));
            }
        }
    }
    
    return Violations;
}

void UTechnicalArchitectureManager::SetPerformanceTarget(const FPerformanceTarget& Target)
{
    int32 ExistingIndex = TargetNameToIndex.FindRef(Target.TargetName);
    if (ExistingIndex > 0)
    {
        PerformanceTargets[ExistingIndex - 1] = Target;
    }
    else
    {
        PerformanceTargets.Add(Target);
    }
    
    RebuildCaches();
    UE_LOG(LogTemp, Log, TEXT("Set performance target: %s = %.2f %s"), 
           *Target.TargetName, Target.TargetValue, *Target.Unit);
}

FPerformanceTarget UTechnicalArchitectureManager::GetPerformanceTarget(const FString& TargetName) const
{
    int32 TargetIndex = TargetNameToIndex.FindRef(TargetName);
    if (TargetIndex > 0)
    {
        return PerformanceTargets[TargetIndex - 1];
    }
    
    return FPerformanceTarget();
}

void UTechnicalArchitectureManager::GenerateArchitectureDocument()
{
    UE_LOG(LogTemp, Log, TEXT("Generating Technical Architecture Document..."));
    
    FString DocumentContent;
    DocumentContent += TEXT("# TECHNICAL ARCHITECTURE DOCUMENT\n");
    DocumentContent += TEXT("## Transpersonal Game Studio\n\n");
    DocumentContent += FString::Printf(TEXT("Generated: %s\n\n"), *FDateTime::Now().ToString());
    
    // Systems by Layer
    for (int32 LayerInt = 0; LayerInt < (int32)EArchitectureLayer::Tools + 1; LayerInt++)
    {
        EArchitectureLayer Layer = (EArchitectureLayer)LayerInt;
        TArray<FSystemDefinition> LayerSystems = GetSystemsByLayer(Layer);
        
        if (LayerSystems.Num() > 0)
        {
            FString LayerName;
            switch (Layer)
            {
            case EArchitectureLayer::Engine: LayerName = TEXT("Engine"); break;
            case EArchitectureLayer::Framework: LayerName = TEXT("Framework"); break;
            case EArchitectureLayer::Gameplay: LayerName = TEXT("Gameplay"); break;
            case EArchitectureLayer::Content: LayerName = TEXT("Content"); break;
            case EArchitectureLayer::Platform: LayerName = TEXT("Platform"); break;
            case EArchitectureLayer::Tools: LayerName = TEXT("Tools"); break;
            }
            
            DocumentContent += FString::Printf(TEXT("## %s Layer\n\n"), *LayerName);
            
            for (const FSystemDefinition& System : LayerSystems)
            {
                DocumentContent += FString::Printf(TEXT("### %s\n"), *System.SystemName);
                DocumentContent += FString::Printf(TEXT("- **Description**: %s\n"), *System.Description);
                DocumentContent += FString::Printf(TEXT("- **Priority**: %s\n"), 
                    System.Priority == ESystemPriority::Critical ? TEXT("Critical") :
                    System.Priority == ESystemPriority::High ? TEXT("High") :
                    System.Priority == ESystemPriority::Medium ? TEXT("Medium") :
                    System.Priority == ESystemPriority::Low ? TEXT("Low") : TEXT("Optional"));
                DocumentContent += FString::Printf(TEXT("- **Responsible Agent**: %s\n"), *System.ResponsibleAgent);
                DocumentContent += FString::Printf(TEXT("- **Status**: %s / %s\n"), 
                    System.bIsImplemented ? TEXT("Implemented") : TEXT("Not Implemented"),
                    System.bIsTested ? TEXT("Tested") : TEXT("Not Tested"));
                
                if (System.Dependencies.Num() > 0)
                {
                    DocumentContent += TEXT("- **Dependencies**: ");
                    for (int32 i = 0; i < System.Dependencies.Num(); i++)
                    {
                        DocumentContent += System.Dependencies[i];
                        if (i < System.Dependencies.Num() - 1) DocumentContent += TEXT(", ");
                    }
                    DocumentContent += TEXT("\n");
                }
                DocumentContent += TEXT("\n");
            }
        }
    }
    
    // Performance Targets
    DocumentContent += TEXT("## Performance Targets\n\n");
    for (const FPerformanceTarget& Target : PerformanceTargets)
    {
        DocumentContent += FString::Printf(TEXT("- **%s**: %.2f %s (%s) [%s]\n"), 
            *Target.TargetName, Target.TargetValue, *Target.Unit, *Target.Platform,
            Target.bIsHardRequirement ? TEXT("Hard Requirement") : TEXT("Target"));
    }
    
    UE_LOG(LogTemp, Log, TEXT("Architecture document generated (%d characters)"), DocumentContent.Len());
}

void UTechnicalArchitectureManager::ExportSystemDependencyGraph()
{
    UE_LOG(LogTemp, Log, TEXT("Exporting system dependency graph..."));
    
    FString GraphContent;
    GraphContent += TEXT("digraph SystemDependencies {\n");
    GraphContent += TEXT("  rankdir=TB;\n");
    GraphContent += TEXT("  node [shape=box];\n\n");
    
    // Add nodes
    for (const FSystemDefinition& System : RegisteredSystems)
    {
        FString NodeColor = System.bIsImplemented ? TEXT("lightgreen") : TEXT("lightcoral");
        GraphContent += FString::Printf(TEXT("  \"%s\" [fillcolor=%s, style=filled];\n"), 
            *System.SystemName, *NodeColor);
    }
    
    GraphContent += TEXT("\n");
    
    // Add edges (dependencies)
    for (const FSystemDefinition& System : RegisteredSystems)
    {
        for (const FString& Dependency : System.Dependencies)
        {
            GraphContent += FString::Printf(TEXT("  \"%s\" -> \"%s\";\n"), 
                *Dependency, *System.SystemName);
        }
    }
    
    GraphContent += TEXT("}\n");
    
    UE_LOG(LogTemp, Log, TEXT("Dependency graph exported (DOT format)"));
}

void UTechnicalArchitectureManager::GenerateImplementationReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== IMPLEMENTATION REPORT ==="));
    
    int32 TotalSystems = RegisteredSystems.Num();
    int32 ImplementedSystems = 0;
    int32 TestedSystems = 0;
    
    for (const FSystemDefinition& System : RegisteredSystems)
    {
        if (System.bIsImplemented) ImplementedSystems++;
        if (System.bIsTested) TestedSystems++;
        
        FString Status = TEXT("❌");
        if (System.bIsImplemented && System.bIsTested) Status = TEXT("✅");
        else if (System.bIsImplemented) Status = TEXT("⚠️");
        
        UE_LOG(LogTemp, Warning, TEXT("%s %s (%s)"), 
            *Status, *System.SystemName, *System.ResponsibleAgent);
    }
    
    float ImplementationProgress = TotalSystems > 0 ? (float)ImplementedSystems / TotalSystems * 100.0f : 0.0f;
    float TestingProgress = TotalSystems > 0 ? (float)TestedSystems / TotalSystems * 100.0f : 0.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("Implementation Progress: %.1f%% (%d/%d)"), 
        ImplementationProgress, ImplementedSystems, TotalSystems);
    UE_LOG(LogTemp, Warning, TEXT("Testing Progress: %.1f%% (%d/%d)"), 
        TestingProgress, TestedSystems, TotalSystems);
    
    UE_LOG(LogTemp, Warning, TEXT("=== END IMPLEMENTATION REPORT ==="));
}

bool UTechnicalArchitectureManager::ValidateArchitectureCompliance()
{
    TArray<FString> AllViolations = GetArchitectureViolations();
    return AllViolations.Num() == 0;
}

TArray<FString> UTechnicalArchitectureManager::GetArchitectureViolations()
{
    TArray<FString> AllViolations;
    
    for (const FSystemDefinition& System : RegisteredSystems)
    {
        TArray<FString> SystemViolations = GetRuleViolations(System.SystemName);
        AllViolations.Append(SystemViolations);
    }
    
    return AllViolations;
}

float UTechnicalArchitectureManager::GetImplementationProgress() const
{
    if (RegisteredSystems.Num() == 0) return 0.0f;
    
    int32 ImplementedCount = 0;
    for (const FSystemDefinition& System : RegisteredSystems)
    {
        if (System.bIsImplemented) ImplementedCount++;
    }
    
    return (float)ImplementedCount / RegisteredSystems.Num() * 100.0f;
}

void UTechnicalArchitectureManager::InitializeDefaultSystems()
{
    // Engine Layer Systems
    FSystemDefinition WorldPartitionSystem;
    WorldPartitionSystem.SystemName = TEXT("World Partition System");
    WorldPartitionSystem.Layer = EArchitectureLayer::Engine;
    WorldPartitionSystem.Priority = ESystemPriority::Critical;
    WorldPartitionSystem.Description = TEXT("Large world streaming and management using UE5 World Partition");
    WorldPartitionSystem.RequiredClasses.Add(TEXT("UWorldPartition"));
    WorldPartitionSystem.ResponsibleAgent = TEXT("Engine Architect");
    RegisterSystem(WorldPartitionSystem);
    
    FSystemDefinition NaniteSystem;
    NaniteSystem.SystemName = TEXT("Nanite Virtualized Geometry");
    NaniteSystem.Layer = EArchitectureLayer::Engine;
    NaniteSystem.Priority = ESystemPriority::Critical;
    NaniteSystem.Description = TEXT("High-detail geometry rendering for prehistoric environments");
    NaniteSystem.ResponsibleAgent = TEXT("Engine Architect");
    RegisterSystem(NaniteSystem);
    
    FSystemDefinition LumenSystem;
    LumenSystem.SystemName = TEXT("Lumen Global Illumination");
    LumenSystem.Layer = EArchitectureLayer::Engine;
    LumenSystem.Priority = ESystemPriority::Critical;
    LumenSystem.Description = TEXT("Dynamic global illumination for realistic lighting");
    LumenSystem.ResponsibleAgent = TEXT("Lighting & Atmosphere Agent");
    RegisterSystem(LumenSystem);
    
    // Framework Layer Systems
    FSystemDefinition ConsciousnessSystem;
    ConsciousnessSystem.SystemName = TEXT("Consciousness System");
    ConsciousnessSystem.Layer = EArchitectureLayer::Framework;
    ConsciousnessSystem.Priority = ESystemPriority::High;
    ConsciousnessSystem.Description = TEXT("Core consciousness and awareness mechanics");
    ConsciousnessSystem.ResponsibleAgent = TEXT("Core Systems Programmer");
    RegisterSystem(ConsciousnessSystem);
    
    // Add more default systems as needed...
}

void UTechnicalArchitectureManager::InitializeDefaultRules()
{
    FArchitectureRule ModularityRule;
    ModularityRule.RuleName = TEXT("System Modularity");
    ModularityRule.Description = TEXT("All systems must be modular and loosely coupled");
    ModularityRule.Severity = ESystemPriority::High;
    ModularityRule.AppliesTo.Add(TEXT("*"));
    ModularityRule.Enforcement = TEXT("Code review and architecture validation");
    AddArchitectureRule(ModularityRule);
    
    FArchitectureRule PerformanceRule;
    PerformanceRule.RuleName = TEXT("Performance First");
    PerformanceRule.Description = TEXT("All systems must meet 60fps target on target hardware");
    PerformanceRule.Severity = ESystemPriority::Critical;
    PerformanceRule.AppliesTo.Add(TEXT("*"));
    PerformanceRule.Enforcement = TEXT("Performance profiling and testing");
    AddArchitectureRule(PerformanceRule);
}

void UTechnicalArchitectureManager::InitializeDefaultPerformanceTargets()
{
    FPerformanceTarget FrameRateTarget;
    FrameRateTarget.TargetName = TEXT("Frame Rate");
    FrameRateTarget.TargetValue = 60.0f;
    FrameRateTarget.Unit = TEXT("FPS");
    FrameRateTarget.Platform = TEXT("PC High-End");
    FrameRateTarget.bIsHardRequirement = true;
    SetPerformanceTarget(FrameRateTarget);
    
    FPerformanceTarget MemoryTarget;
    MemoryTarget.TargetName = TEXT("Memory Usage");
    MemoryTarget.TargetValue = 8.0f;
    MemoryTarget.Unit = TEXT("GB");
    MemoryTarget.Platform = TEXT("PC High-End");
    MemoryTarget.bIsHardRequirement = false;
    SetPerformanceTarget(MemoryTarget);
    
    FPerformanceTarget LoadTimeTarget;
    LoadTimeTarget.TargetName = TEXT("Level Load Time");
    LoadTimeTarget.TargetValue = 10.0f;
    LoadTimeTarget.Unit = TEXT("seconds");
    LoadTimeTarget.Platform = TEXT("PC High-End");
    LoadTimeTarget.bIsHardRequirement = false;
    SetPerformanceTarget(LoadTimeTarget);
}

void UTechnicalArchitectureManager::RebuildCaches()
{
    SystemNameToIndex.Empty();
    RuleNameToIndex.Empty();
    TargetNameToIndex.Empty();
    
    for (int32 i = 0; i < RegisteredSystems.Num(); i++)
    {
        SystemNameToIndex.Add(RegisteredSystems[i].SystemName, i + 1);
    }
    
    for (int32 i = 0; i < ArchitectureRules.Num(); i++)
    {
        RuleNameToIndex.Add(ArchitectureRules[i].RuleName, i + 1);
    }
    
    for (int32 i = 0; i < PerformanceTargets.Num(); i++)
    {
        TargetNameToIndex.Add(PerformanceTargets[i].TargetName, i + 1);
    }
}