#include "Eng_ArchitecturalRules.h"
#include "Engine/Engine.h"
#include "Misc/DateTime.h"

UEng_ArchitecturalRules::UEng_ArchitecturalRules()
{
    bAutoEnforcement = true;
    ValidationInterval = 5.0f; // Check every 5 seconds
}

void UEng_ArchitecturalRules::InitializeDefaultRules()
{
    ArchitecturalRules.Empty();
    ActiveViolations.Empty();
    
    CreateCoreRules();
    CreatePerformanceRules();
    CreateIntegrationRules();
    
    UE_LOG(LogTemp, Warning, TEXT("Architectural Rules: Initialized %d default rules"), ArchitecturalRules.Num());
}

bool UEng_ArchitecturalRules::ValidateRule(const FString& RuleName, const FString& ModuleName)
{
    FEng_ArchitecturalRule Rule = GetRule(RuleName);
    if (Rule.RuleName.IsEmpty() || !Rule.bIsEnforced)
    {
        return true; // Rule doesn't exist or not enforced
    }
    
    // Rule-specific validation logic
    if (RuleName == TEXT("NoCircularDependencies"))
    {
        return ValidateModuleDependencies(ModuleName);
    }
    else if (RuleName == TEXT("PerformanceTargets"))
    {
        return ValidatePerformanceTargets(ModuleName);
    }
    else if (RuleName == TEXT("LayerCompliance"))
    {
        return ValidateLayerCompliance(ModuleName);
    }
    
    return true;
}

TArray<FEng_RuleViolation> UEng_ArchitecturalRules::CheckAllRules(const TArray<FString>& ModuleNames)
{
    TArray<FEng_RuleViolation> Violations;
    
    for (const FString& ModuleName : ModuleNames)
    {
        for (const FEng_ArchitecturalRule& Rule : ArchitecturalRules)
        {
            if (!ValidateRule(Rule.RuleName, ModuleName))
            {
                FEng_RuleViolation Violation;
                Violation.RuleName = Rule.RuleName;
                Violation.Description = Rule.Description;
                Violation.ModuleName = ModuleName;
                Violation.Severity = Rule.Severity;
                Violation.ViolationType = EEng_RuleViolationType::LayerViolation; // Default type
                Violation.DetectionTime = FDateTime::Now();
                
                Violations.Add(Violation);
                ReportViolation(Violation);
            }
        }
    }
    
    return Violations;
}

void UEng_ArchitecturalRules::AddRule(const FEng_ArchitecturalRule& NewRule)
{
    // Remove existing rule with same name
    RemoveRule(NewRule.RuleName);
    
    ArchitecturalRules.Add(NewRule);
    UE_LOG(LogTemp, Log, TEXT("Architectural Rules: Added rule '%s'"), *NewRule.RuleName);
}

void UEng_ArchitecturalRules::RemoveRule(const FString& RuleName)
{
    ArchitecturalRules.RemoveAll([RuleName](const FEng_ArchitecturalRule& Rule)
    {
        return Rule.RuleName == RuleName;
    });
}

FEng_ArchitecturalRule UEng_ArchitecturalRules::GetRule(const FString& RuleName)
{
    for (const FEng_ArchitecturalRule& Rule : ArchitecturalRules)
    {
        if (Rule.RuleName == RuleName)
        {
            return Rule;
        }
    }
    return FEng_ArchitecturalRule(); // Empty rule if not found
}

TArray<FEng_ArchitecturalRule> UEng_ArchitecturalRules::GetRulesByModule(const FString& ModuleName)
{
    TArray<FEng_ArchitecturalRule> ModuleRules;
    
    for (const FEng_ArchitecturalRule& Rule : ArchitecturalRules)
    {
        if (Rule.AffectedModules.Contains(ModuleName) || Rule.AffectedModules.Num() == 0)
        {
            ModuleRules.Add(Rule);
        }
    }
    
    return ModuleRules;
}

void UEng_ArchitecturalRules::ReportViolation(const FEng_RuleViolation& Violation)
{
    // Check if violation already exists
    bool bExists = ActiveViolations.ContainsByPredicate([&Violation](const FEng_RuleViolation& Existing)
    {
        return Existing.RuleName == Violation.RuleName && 
               Existing.ModuleName == Violation.ModuleName &&
               !Existing.bIsResolved;
    });
    
    if (!bExists)
    {
        ActiveViolations.Add(Violation);
        UE_LOG(LogTemp, Warning, TEXT("Architectural Rules: Violation reported - %s in module %s"), 
               *Violation.RuleName, *Violation.ModuleName);
    }
}

TArray<FEng_RuleViolation> UEng_ArchitecturalRules::GetActiveViolations()
{
    return ActiveViolations.FilterByPredicate([](const FEng_RuleViolation& Violation)
    {
        return !Violation.bIsResolved;
    });
}

void UEng_ArchitecturalRules::ResolveViolation(const FString& RuleName, const FString& ModuleName)
{
    for (FEng_RuleViolation& Violation : ActiveViolations)
    {
        if (Violation.RuleName == RuleName && Violation.ModuleName == ModuleName)
        {
            Violation.bIsResolved = true;
            UE_LOG(LogTemp, Log, TEXT("Architectural Rules: Resolved violation %s in module %s"), 
                   *RuleName, *ModuleName);
            break;
        }
    }
}

int32 UEng_ArchitecturalRules::GetViolationCount() const
{
    return ActiveViolations.FilterByPredicate([](const FEng_RuleViolation& Violation)
    {
        return !Violation.bIsResolved;
    }).Num();
}

void UEng_ArchitecturalRules::CreateCoreRules()
{
    // No circular dependencies rule
    FEng_ArchitecturalRule CircularRule;
    CircularRule.RuleName = TEXT("NoCircularDependencies");
    CircularRule.Description = TEXT("Modules must not have circular dependencies");
    CircularRule.bIsEnforced = true;
    CircularRule.Severity = 10; // Critical
    AddRule(CircularRule);
    
    // Layer compliance rule
    FEng_ArchitecturalRule LayerRule;
    LayerRule.RuleName = TEXT("LayerCompliance");
    LayerRule.Description = TEXT("Modules must only depend on lower or same layer modules");
    LayerRule.bIsEnforced = true;
    LayerRule.Severity = 8;
    AddRule(LayerRule);
    
    // Compilation requirement rule
    FEng_ArchitecturalRule CompileRule;
    CompileRule.RuleName = TEXT("CompilationRequired");
    CompileRule.Description = TEXT("All modules must compile without errors");
    CompileRule.bIsEnforced = true;
    CompileRule.Severity = 10;
    AddRule(CompileRule);
}

void UEng_ArchitecturalRules::CreatePerformanceRules()
{
    // Performance targets rule
    FEng_ArchitecturalRule PerfRule;
    PerfRule.RuleName = TEXT("PerformanceTargets");
    PerfRule.Description = TEXT("Modules must meet performance targets (60 FPS, 8000 actors max)");
    PerfRule.bIsEnforced = true;
    PerfRule.Severity = 7;
    AddRule(PerfRule);
    
    // Memory usage rule
    FEng_ArchitecturalRule MemoryRule;
    MemoryRule.RuleName = TEXT("MemoryLimits");
    MemoryRule.Description = TEXT("Modules must not exceed 4GB memory usage");
    MemoryRule.bIsEnforced = true;
    MemoryRule.Severity = 8;
    AddRule(MemoryRule);
}

void UEng_ArchitecturalRules::CreateIntegrationRules()
{
    // Integration testing rule
    FEng_ArchitecturalRule IntegrationRule;
    IntegrationRule.RuleName = TEXT("IntegrationTesting");
    IntegrationRule.Description = TEXT("Modules must pass integration tests with other systems");
    IntegrationRule.bIsEnforced = true;
    IntegrationRule.Severity = 6;
    AddRule(IntegrationRule);
    
    // API compatibility rule
    FEng_ArchitecturalRule APIRule;
    APIRule.RuleName = TEXT("APICompatibility");
    APIRule.Description = TEXT("Modules must maintain backward compatibility");
    APIRule.bIsEnforced = true;
    APIRule.Severity = 5;
    AddRule(APIRule);
}

bool UEng_ArchitecturalRules::ValidateModuleDependencies(const FString& ModuleName)
{
    // Simplified dependency validation
    // In a real implementation, this would check actual module dependencies
    return true;
}

bool UEng_ArchitecturalRules::ValidatePerformanceTargets(const FString& ModuleName)
{
    // Simplified performance validation
    // In a real implementation, this would check actual performance metrics
    return true;
}

bool UEng_ArchitecturalRules::ValidateLayerCompliance(const FString& ModuleName)
{
    // Simplified layer validation
    // In a real implementation, this would check actual layer dependencies
    return true;
}