#include "Eng_SystemArchitect.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

UEng_SystemArchitect::UEng_SystemArchitect()
{
    MaxPerformanceBudget = 50.0f;
    HighImpactThreshold = 5.0f;
    bEnableRealTimeValidation = true;
    bStrictDependencyChecking = true;
    bEnforceLayerBoundaries = true;
}

void UEng_SystemArchitect::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("System Architect: Initializing architectural governance framework"));
    
    // Initialize core architectural rules
    FEng_ArchitecturalRule FoundationRule;
    FoundationRule.RuleName = TEXT("Foundation Layer Isolation");
    FoundationRule.RuleDescription = TEXT("Foundation layer systems cannot depend on higher layers");
    FoundationRule.ApplicableLayer = EEng_ArchitecturalLayer::Foundation;
    FoundationRule.bIsMandatory = true;
    FoundationRule.ViolationPenalty = 10.0f;
    ArchitecturalRules.Add(FoundationRule);
    
    FEng_ArchitecturalRule PerformanceRule;
    PerformanceRule.RuleName = TEXT("Performance Budget Compliance");
    PerformanceRule.RuleDescription = TEXT("Total system performance weight cannot exceed budget");
    PerformanceRule.ApplicableLayer = EEng_ArchitecturalLayer::Performance;
    PerformanceRule.bIsMandatory = true;
    PerformanceRule.ViolationPenalty = 5.0f;
    ArchitecturalRules.Add(PerformanceRule);
    
    FEng_ArchitecturalRule DependencyRule;
    DependencyRule.RuleName = TEXT("No Circular Dependencies");
    DependencyRule.RuleDescription = TEXT("Systems cannot have circular dependency chains");
    DependencyRule.ApplicableLayer = EEng_ArchitecturalLayer::Core;
    DependencyRule.bIsMandatory = true;
    DependencyRule.ViolationPenalty = 8.0f;
    ArchitecturalRules.Add(DependencyRule);
    
    UE_LOG(LogTemp, Warning, TEXT("System Architect: Initialized with %d architectural rules"), ArchitecturalRules.Num());
}

void UEng_SystemArchitect::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("System Architect: Shutting down architectural governance"));
    RegisteredSystems.Empty();
    Super::Deinitialize();
}

bool UEng_SystemArchitect::RegisterSystem(const FEng_SystemDefinition& SystemDef)
{
    if (!ValidateSystemDefinition(SystemDef))
    {
        UE_LOG(LogTemp, Error, TEXT("System Architect: Failed to validate system definition for %s"), *SystemDef.SystemName);
        return false;
    }
    
    if (RegisteredSystems.Contains(SystemDef.SystemName))
    {
        UE_LOG(LogTemp, Warning, TEXT("System Architect: System %s already registered, updating definition"), *SystemDef.SystemName);
    }
    
    RegisteredSystems.Add(SystemDef.SystemName, SystemDef);
    
    // Validate against architectural rules
    if (bEnableRealTimeValidation && !ValidateSystemAgainstRules(SystemDef.SystemName))
    {
        UE_LOG(LogTemp, Warning, TEXT("System Architect: System %s registered but has rule violations"), *SystemDef.SystemName);
        OnArchitecturalViolation(SystemDef.SystemName, TEXT("Rule violations detected during registration"));
    }
    
    OnSystemRegistered(SystemDef.SystemName);
    UE_LOG(LogTemp, Log, TEXT("System Architect: Successfully registered system %s"), *SystemDef.SystemName);
    return true;
}

bool UEng_SystemArchitect::UnregisterSystem(const FString& SystemName)
{
    if (!RegisteredSystems.Contains(SystemName))
    {
        UE_LOG(LogTemp, Warning, TEXT("System Architect: Cannot unregister system %s - not found"), *SystemName);
        return false;
    }
    
    RegisteredSystems.Remove(SystemName);
    OnSystemUnregistered(SystemName);
    UE_LOG(LogTemp, Log, TEXT("System Architect: Successfully unregistered system %s"), *SystemName);
    return true;
}

FEng_SystemDefinition UEng_SystemArchitect::GetSystemDefinition(const FString& SystemName) const
{
    if (const FEng_SystemDefinition* Found = RegisteredSystems.Find(SystemName))
    {
        return *Found;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("System Architect: System definition not found for %s"), *SystemName);
    return FEng_SystemDefinition();
}

void UEng_SystemArchitect::AddArchitecturalRule(const FEng_ArchitecturalRule& Rule)
{
    ArchitecturalRules.Add(Rule);
    UE_LOG(LogTemp, Log, TEXT("System Architect: Added architectural rule %s"), *Rule.RuleName);
    
    // Re-validate all systems if real-time validation is enabled
    if (bEnableRealTimeValidation)
    {
        for (const auto& SystemPair : RegisteredSystems)
        {
            if (!ValidateSystemAgainstRules(SystemPair.Key))
            {
                OnArchitecturalViolation(SystemPair.Key, FString::Printf(TEXT("New rule violation: %s"), *Rule.RuleName));
            }
        }
    }
}

bool UEng_SystemArchitect::ValidateSystemAgainstRules(const FString& SystemName) const
{
    const FEng_SystemDefinition* SystemDef = RegisteredSystems.Find(SystemName);
    if (!SystemDef)
    {
        return false;
    }
    
    for (const FEng_ArchitecturalRule& Rule : ArchitecturalRules)
    {
        if (Rule.ApplicableLayer != SystemDef->Layer && Rule.ApplicableLayer != EEng_ArchitecturalLayer::Core)
        {
            continue;
        }
        
        // Check specific rule types
        if (Rule.RuleName == TEXT("Foundation Layer Isolation"))
        {
            if (SystemDef->Layer == EEng_ArchitecturalLayer::Foundation)
            {
                for (const FString& Dependency : SystemDef->Dependencies)
                {
                    const FEng_SystemDefinition* DepSystem = RegisteredSystems.Find(Dependency);
                    if (DepSystem && DepSystem->Layer != EEng_ArchitecturalLayer::Foundation)
                    {
                        return false;
                    }
                }
            }
        }
        else if (Rule.RuleName == TEXT("Performance Budget Compliance"))
        {
            if (GetTotalSystemWeight() > MaxPerformanceBudget)
            {
                return false;
            }
        }
        else if (Rule.RuleName == TEXT("No Circular Dependencies"))
        {
            if (HasCircularDependencies())
            {
                return false;
            }
        }
    }
    
    return true;
}

TArray<FString> UEng_SystemArchitect::GetRuleViolations(const FString& SystemName) const
{
    TArray<FString> Violations;
    const FEng_SystemDefinition* SystemDef = RegisteredSystems.Find(SystemName);
    
    if (!SystemDef)
    {
        Violations.Add(TEXT("System not found in registry"));
        return Violations;
    }
    
    for (const FEng_ArchitecturalRule& Rule : ArchitecturalRules)
    {
        if (Rule.ApplicableLayer != SystemDef->Layer && Rule.ApplicableLayer != EEng_ArchitecturalLayer::Core)
        {
            continue;
        }
        
        bool bRuleViolated = false;
        
        if (Rule.RuleName == TEXT("Foundation Layer Isolation"))
        {
            if (SystemDef->Layer == EEng_ArchitecturalLayer::Foundation)
            {
                for (const FString& Dependency : SystemDef->Dependencies)
                {
                    const FEng_SystemDefinition* DepSystem = RegisteredSystems.Find(Dependency);
                    if (DepSystem && DepSystem->Layer != EEng_ArchitecturalLayer::Foundation)
                    {
                        bRuleViolated = true;
                        break;
                    }
                }
            }
        }
        
        if (bRuleViolated)
        {
            Violations.Add(FString::Printf(TEXT("Rule Violation: %s - %s"), *Rule.RuleName, *Rule.RuleDescription));
        }
    }
    
    return Violations;
}

TArray<FString> UEng_SystemArchitect::GetSystemsByLayer(EEng_ArchitecturalLayer Layer) const
{
    TArray<FString> Systems;
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        if (SystemPair.Value.Layer == Layer)
        {
            Systems.Add(SystemPair.Key);
        }
    }
    
    return Systems;
}

bool UEng_SystemArchitect::CanSystemAccessLayer(const FString& SystemName, EEng_ArchitecturalLayer TargetLayer) const
{
    const FEng_SystemDefinition* SystemDef = RegisteredSystems.Find(SystemName);
    if (!SystemDef)
    {
        return false;
    }
    
    return CheckLayerAccess(SystemDef->Layer, TargetLayer);
}

bool UEng_SystemArchitect::ValidateDependencyChain(const FString& SystemName) const
{
    const FEng_SystemDefinition* SystemDef = RegisteredSystems.Find(SystemName);
    if (!SystemDef)
    {
        return false;
    }
    
    // Check if all dependencies exist and are valid
    for (const FString& Dependency : SystemDef->Dependencies)
    {
        if (!RegisteredSystems.Contains(Dependency))
        {
            UE_LOG(LogTemp, Warning, TEXT("System Architect: System %s has missing dependency %s"), *SystemName, *Dependency);
            return false;
        }
        
        // Recursively validate dependency chains
        if (!ValidateDependencyChain(Dependency))
        {
            return false;
        }
    }
    
    return true;
}

TArray<FString> UEng_SystemArchitect::GetDependencyOrder() const
{
    TArray<FString> OrderedSystems;
    TMap<FString, TArray<FString>> DependencyGraph;
    BuildDependencyGraph(DependencyGraph);
    
    TSet<FString> Visited;
    TSet<FString> Visiting;
    
    // Topological sort using DFS
    for (const auto& SystemPair : RegisteredSystems)
    {
        if (!Visited.Contains(SystemPair.Key))
        {
            TArray<FString> CurrentPath;
            if (TopologicalSortDFS(SystemPair.Key, DependencyGraph, Visited, Visiting, CurrentPath))
            {
                for (int32 i = CurrentPath.Num() - 1; i >= 0; --i)
                {
                    OrderedSystems.AddUnique(CurrentPath[i]);
                }
            }
        }
    }
    
    return OrderedSystems;
}

bool UEng_SystemArchitect::HasCircularDependencies() const
{
    TMap<FString, TArray<FString>> DependencyGraph;
    BuildDependencyGraph(DependencyGraph);
    
    TSet<FString> Visited;
    TSet<FString> RecStack;
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        if (!Visited.Contains(SystemPair.Key))
        {
            if (DetectCircularDependency(DependencyGraph, SystemPair.Key, Visited, RecStack))
            {
                return true;
            }
        }
    }
    
    return false;
}

float UEng_SystemArchitect::GetTotalSystemWeight() const
{
    float TotalWeight = 0.0f;
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        if (SystemPair.Value.bIsActive)
        {
            TotalWeight += SystemPair.Value.PerformanceWeight;
        }
    }
    
    return TotalWeight;
}

TArray<FString> UEng_SystemArchitect::GetHighPerformanceImpactSystems() const
{
    TArray<FString> HighImpactSystems;
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        if (SystemPair.Value.bIsActive && SystemPair.Value.PerformanceWeight >= HighImpactThreshold)
        {
            HighImpactSystems.Add(SystemPair.Key);
        }
    }
    
    return HighImpactSystems;
}

bool UEng_SystemArchitect::IsPerformanceBudgetExceeded() const
{
    return GetTotalSystemWeight() > MaxPerformanceBudget;
}

int32 UEng_SystemArchitect::GetRegisteredSystemCount() const
{
    return RegisteredSystems.Num();
}

int32 UEng_SystemArchitect::GetActiveSystemCount() const
{
    int32 ActiveCount = 0;
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        if (SystemPair.Value.bIsActive)
        {
            ActiveCount++;
        }
    }
    
    return ActiveCount;
}

TArray<FString> UEng_SystemArchitect::GetInactiveSystems() const
{
    TArray<FString> InactiveSystems;
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        if (!SystemPair.Value.bIsActive)
        {
            InactiveSystems.Add(SystemPair.Key);
        }
    }
    
    return InactiveSystems;
}

bool UEng_SystemArchitect::ValidateArchitecturalIntegrity() const
{
    // Check for circular dependencies
    if (HasCircularDependencies())
    {
        return false;
    }
    
    // Check performance budget
    if (IsPerformanceBudgetExceeded())
    {
        return false;
    }
    
    // Validate all systems against rules
    for (const auto& SystemPair : RegisteredSystems)
    {
        if (!ValidateSystemAgainstRules(SystemPair.Key))
        {
            return false;
        }
    }
    
    return true;
}

TArray<FString> UEng_SystemArchitect::GetArchitecturalIssues() const
{
    TArray<FString> Issues;
    
    if (HasCircularDependencies())
    {
        Issues.Add(TEXT("Circular dependencies detected in system registry"));
    }
    
    if (IsPerformanceBudgetExceeded())
    {
        Issues.Add(FString::Printf(TEXT("Performance budget exceeded: %.2f/%.2f"), GetTotalSystemWeight(), MaxPerformanceBudget));
    }
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        TArray<FString> SystemViolations = GetRuleViolations(SystemPair.Key);
        for (const FString& Violation : SystemViolations)
        {
            Issues.Add(FString::Printf(TEXT("System %s: %s"), *SystemPair.Key, *Violation));
        }
    }
    
    return Issues;
}

void UEng_SystemArchitect::RunArchitecturalAudit()
{
    UE_LOG(LogTemp, Warning, TEXT("System Architect: Running comprehensive architectural audit"));
    
    TArray<FString> Issues = GetArchitecturalIssues();
    
    UE_LOG(LogTemp, Warning, TEXT("System Architect: Audit Results - %d registered systems, %d active systems"), 
           GetRegisteredSystemCount(), GetActiveSystemCount());
    
    if (Issues.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("System Architect: ✅ Architectural integrity validated - no issues found"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("System Architect: ❌ Found %d architectural issues:"), Issues.Num());
        for (const FString& Issue : Issues)
        {
            UE_LOG(LogTemp, Error, TEXT("  - %s"), *Issue);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("System Architect: Performance budget: %.2f/%.2f (%.1f%% used)"), 
           GetTotalSystemWeight(), MaxPerformanceBudget, (GetTotalSystemWeight() / MaxPerformanceBudget) * 100.0f);
}

bool UEng_SystemArchitect::ValidateSystemDefinition(const FEng_SystemDefinition& SystemDef) const
{
    if (SystemDef.SystemName.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("System Architect: System name cannot be empty"));
        return false;
    }
    
    if (SystemDef.PerformanceWeight < 0.0f)
    {
        UE_LOG(LogTemp, Error, TEXT("System Architect: Performance weight cannot be negative"));
        return false;
    }
    
    return true;
}

bool UEng_SystemArchitect::CheckLayerAccess(EEng_ArchitecturalLayer SourceLayer, EEng_ArchitecturalLayer TargetLayer) const
{
    if (!bEnforceLayerBoundaries)
    {
        return true;
    }
    
    // Layer hierarchy: Foundation -> Core -> Gameplay -> Interface -> Performance
    // Lower layers can access higher layers, but not vice versa
    return static_cast<uint8>(SourceLayer) <= static_cast<uint8>(TargetLayer);
}

void UEng_SystemArchitect::BuildDependencyGraph(TMap<FString, TArray<FString>>& OutGraph) const
{
    OutGraph.Empty();
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        OutGraph.Add(SystemPair.Key, SystemPair.Value.Dependencies);
    }
}

bool UEng_SystemArchitect::DetectCircularDependency(const TMap<FString, TArray<FString>>& Graph, const FString& StartNode, TSet<FString>& Visited, TSet<FString>& RecStack) const
{
    Visited.Add(StartNode);
    RecStack.Add(StartNode);
    
    if (const TArray<FString>* Dependencies = Graph.Find(StartNode))
    {
        for (const FString& Dependency : *Dependencies)
        {
            if (!Visited.Contains(Dependency))
            {
                if (DetectCircularDependency(Graph, Dependency, Visited, RecStack))
                {
                    return true;
                }
            }
            else if (RecStack.Contains(Dependency))
            {
                return true;
            }
        }
    }
    
    RecStack.Remove(StartNode);
    return false;
}

bool UEng_SystemArchitect::TopologicalSortDFS(const FString& Node, const TMap<FString, TArray<FString>>& Graph, TSet<FString>& Visited, TSet<FString>& Visiting, TArray<FString>& Path) const
{
    if (Visiting.Contains(Node))
    {
        return false; // Circular dependency
    }
    
    if (Visited.Contains(Node))
    {
        return true;
    }
    
    Visiting.Add(Node);
    
    if (const TArray<FString>* Dependencies = Graph.Find(Node))
    {
        for (const FString& Dependency : *Dependencies)
        {
            if (!TopologicalSortDFS(Dependency, Graph, Visited, Visiting, Path))
            {
                return false;
            }
        }
    }
    
    Visiting.Remove(Node);
    Visited.Add(Node);
    Path.Add(Node);
    
    return true;
}