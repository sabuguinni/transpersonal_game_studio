#include "Eng_MasterArchitect.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

AEng_MasterArchitect::AEng_MasterArchitect()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize default values
    GlobalPerformanceBudget = 100.0f;
    bEnforceStrictCompliance = true;
    MaxConcurrentSystems = 50;
    
    SetupDefaultArchitecturalRules();
}

void AEng_MasterArchitect::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Master Architect: Initializing architectural framework"));
    InitializeArchitecturalFramework();
    InitializeCoreSystems();
}

void AEng_MasterArchitect::SetupDefaultArchitecturalRules()
{
    CoreArchitecturalRules.Empty();
    
    // Rule 1: All gameplay systems must inherit from proper base classes
    FEng_ArchitecturalRule GameplayInheritanceRule;
    GameplayInheritanceRule.RuleName = TEXT("GameplayInheritanceRule");
    GameplayInheritanceRule.RuleDescription = TEXT("All gameplay actors must inherit from AActor, components from UActorComponent");
    GameplayInheritanceRule.TargetLayer = EEng_ArchitecturalLayer::Gameplay;
    GameplayInheritanceRule.Priority = EEng_SystemPriority::Critical;
    GameplayInheritanceRule.bMandatory = true;
    CoreArchitecturalRules.Add(GameplayInheritanceRule);
    
    // Rule 2: Performance budget enforcement
    FEng_ArchitecturalRule PerformanceRule;
    PerformanceRule.RuleName = TEXT("PerformanceBudgetRule");
    PerformanceRule.RuleDescription = TEXT("No system may exceed 5% of total performance budget without approval");
    PerformanceRule.TargetLayer = EEng_ArchitecturalLayer::Performance;
    PerformanceRule.Priority = EEng_SystemPriority::Critical;
    PerformanceRule.bMandatory = true;
    CoreArchitecturalRules.Add(PerformanceRule);
    
    // Rule 3: Module dependency management
    FEng_ArchitecturalRule DependencyRule;
    DependencyRule.RuleName = TEXT("ModuleDependencyRule");
    DependencyRule.RuleDescription = TEXT("All cross-module dependencies must be declared in Build.cs");
    DependencyRule.TargetLayer = EEng_ArchitecturalLayer::Foundation;
    DependencyRule.Priority = EEng_SystemPriority::Critical;
    DependencyRule.bMandatory = true;
    CoreArchitecturalRules.Add(DependencyRule);
    
    // Rule 4: Blueprint exposure standards
    FEng_ArchitecturalRule BlueprintRule;
    BlueprintRule.RuleName = TEXT("BlueprintExposureRule");
    BlueprintRule.RuleDescription = TEXT("All designer-facing functionality must be Blueprint accessible");
    BlueprintRule.TargetLayer = EEng_ArchitecturalLayer::Interface;
    BlueprintRule.Priority = EEng_SystemPriority::High;
    BlueprintRule.bMandatory = true;
    CoreArchitecturalRules.Add(BlueprintRule);
    
    // Rule 5: Memory management standards
    FEng_ArchitecturalRule MemoryRule;
    MemoryRule.RuleName = TEXT("MemoryManagementRule");
    MemoryRule.RuleDescription = TEXT("All dynamic allocations must use UE5 memory management (UPROPERTY, TArray, etc.)");
    MemoryRule.TargetLayer = EEng_ArchitecturalLayer::Core;
    MemoryRule.Priority = EEng_SystemPriority::Critical;
    MemoryRule.bMandatory = true;
    CoreArchitecturalRules.Add(MemoryRule);
}

void AEng_MasterArchitect::InitializeCoreSystems()
{
    RegisteredSystems.Empty();
    
    // Register Core Game Systems
    FEng_SystemSpecification GameStateSystem;
    GameStateSystem.SystemName = TEXT("TranspersonalGameState");
    GameStateSystem.SystemDescription = TEXT("Core game state management system");
    GameStateSystem.Layer = EEng_ArchitecturalLayer::Core;
    GameStateSystem.Priority = EEng_SystemPriority::Critical;
    GameStateSystem.RequiredModules.Add(TEXT("CoreMinimal"));
    GameStateSystem.RequiredModules.Add(TEXT("Engine"));
    GameStateSystem.PerformanceBudget = 2.0f;
    RegisterNewSystem(GameStateSystem);
    
    // Register Character System
    FEng_SystemSpecification CharacterSystem;
    CharacterSystem.SystemName = TEXT("TranspersonalCharacter");
    CharacterSystem.SystemDescription = TEXT("Player character and movement system");
    CharacterSystem.Layer = EEng_ArchitecturalLayer::Gameplay;
    CharacterSystem.Priority = EEng_SystemPriority::Critical;
    CharacterSystem.RequiredModules.Add(TEXT("CoreMinimal"));
    CharacterSystem.RequiredModules.Add(TEXT("Engine"));
    CharacterSystem.RequiredModules.Add(TEXT("InputCore"));
    CharacterSystem.PerformanceBudget = 3.0f;
    RegisterNewSystem(CharacterSystem);
    
    // Register Biome System
    FEng_SystemSpecification BiomeSystem;
    BiomeSystem.SystemName = TEXT("BiomeManager");
    BiomeSystem.SystemDescription = TEXT("World biome generation and management");
    BiomeSystem.Layer = EEng_ArchitecturalLayer::Gameplay;
    BiomeSystem.Priority = EEng_SystemPriority::High;
    BiomeSystem.RequiredModules.Add(TEXT("CoreMinimal"));
    BiomeSystem.RequiredModules.Add(TEXT("Engine"));
    BiomeSystem.RequiredModules.Add(TEXT("PCG"));
    BiomeSystem.PerformanceBudget = 8.0f;
    RegisterNewSystem(BiomeSystem);
    
    // Register Performance Monitoring System
    FEng_SystemSpecification PerformanceSystem;
    PerformanceSystem.SystemName = TEXT("PerformanceProfiler");
    PerformanceSystem.SystemDescription = TEXT("Real-time performance monitoring and optimization");
    PerformanceSystem.Layer = EEng_ArchitecturalLayer::Performance;
    PerformanceSystem.Priority = EEng_SystemPriority::High;
    PerformanceSystem.RequiredModules.Add(TEXT("CoreMinimal"));
    PerformanceSystem.RequiredModules.Add(TEXT("Engine"));
    PerformanceSystem.PerformanceBudget = 1.5f;
    RegisterNewSystem(PerformanceSystem);
}

void AEng_MasterArchitect::InitializeArchitecturalFramework()
{
    UE_LOG(LogTemp, Warning, TEXT("Master Architect: Initializing architectural framework with %d rules"), CoreArchitecturalRules.Num());
    
    // Initialize system tracking
    SystemActiveStates.Empty();
    SystemPerformanceMetrics.Empty();
    CriticalSystemFailures.Empty();
    
    // Validate all registered systems
    ValidateAllSystems();
    
    UE_LOG(LogTemp, Warning, TEXT("Master Architect: Framework initialization complete"));
}

bool AEng_MasterArchitect::ValidateSystemCompliance(const FString& SystemName)
{
    if (SystemName.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Master Architect: Cannot validate empty system name"));
        return false;
    }
    
    // Find system specification
    FEng_SystemSpecification* SystemSpec = RegisteredSystems.FindByPredicate([&SystemName](const FEng_SystemSpecification& Spec)
    {
        return Spec.SystemName == SystemName;
    });
    
    if (!SystemSpec)
    {
        UE_LOG(LogTemp, Warning, TEXT("Master Architect: System %s not found in registry"), *SystemName);
        return false;
    }
    
    // Validate against architectural rules
    for (const FEng_ArchitecturalRule& Rule : CoreArchitecturalRules)
    {
        if (Rule.TargetLayer == SystemSpec->Layer && Rule.bMandatory)
        {
            // Perform rule-specific validation
            if (Rule.RuleName == TEXT("PerformanceBudgetRule"))
            {
                if (SystemSpec->PerformanceBudget > 5.0f)
                {
                    LogArchitecturalViolation(FString::Printf(TEXT("System %s exceeds performance budget: %.2f%%"), 
                        *SystemName, SystemSpec->PerformanceBudget));
                    return false;
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Master Architect: System %s passed compliance validation"), *SystemName);
    return true;
}

bool AEng_MasterArchitect::RegisterNewSystem(const FEng_SystemSpecification& SystemSpec)
{
    if (SystemSpec.SystemName.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Master Architect: Cannot register system with empty name"));
        return false;
    }
    
    // Check if system already exists
    bool bSystemExists = RegisteredSystems.ContainsByPredicate([&SystemSpec](const FEng_SystemSpecification& ExistingSpec)
    {
        return ExistingSpec.SystemName == SystemSpec.SystemName;
    });
    
    if (bSystemExists)
    {
        UE_LOG(LogTemp, Warning, TEXT("Master Architect: System %s already registered"), *SystemSpec.SystemName);
        return false;
    }
    
    // Validate system specification
    if (!ValidateSystemCompliance(SystemSpec.SystemName))
    {
        UE_LOG(LogTemp, Error, TEXT("Master Architect: System %s failed compliance validation"), *SystemSpec.SystemName);
        return false;
    }
    
    // Register the system
    RegisteredSystems.Add(SystemSpec);
    SystemActiveStates.Add(SystemSpec.SystemName, SystemSpec.bIsActive);
    SystemPerformanceMetrics.Add(SystemSpec.SystemName, 0.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("Master Architect: Successfully registered system %s"), *SystemSpec.SystemName);
    return true;
}

bool AEng_MasterArchitect::EnforceArchitecturalRule(const FEng_ArchitecturalRule& Rule)
{
    if (Rule.RuleName.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Master Architect: Cannot enforce rule with empty name"));
        return false;
    }
    
    // Check if rule already exists
    bool bRuleExists = CoreArchitecturalRules.ContainsByPredicate([&Rule](const FEng_ArchitecturalRule& ExistingRule)
    {
        return ExistingRule.RuleName == Rule.RuleName;
    });
    
    if (bRuleExists)
    {
        UE_LOG(LogTemp, Warning, TEXT("Master Architect: Rule %s already exists"), *Rule.RuleName);
        return false;
    }
    
    // Add the new rule
    CoreArchitecturalRules.Add(Rule);
    
    // Re-validate all systems against the new rule
    ValidateAllSystems();
    
    UE_LOG(LogTemp, Warning, TEXT("Master Architect: Successfully enforced new rule %s"), *Rule.RuleName);
    return true;
}

bool AEng_MasterArchitect::ActivateSystem(const FString& SystemName)
{
    if (SystemName.IsEmpty())
    {
        return false;
    }
    
    bool* SystemState = SystemActiveStates.Find(SystemName);
    if (SystemState)
    {
        *SystemState = true;
        UE_LOG(LogTemp, Log, TEXT("Master Architect: Activated system %s"), *SystemName);
        return true;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Master Architect: Cannot activate unregistered system %s"), *SystemName);
    return false;
}

bool AEng_MasterArchitect::DeactivateSystem(const FString& SystemName)
{
    if (SystemName.IsEmpty())
    {
        return false;
    }
    
    bool* SystemState = SystemActiveStates.Find(SystemName);
    if (SystemState)
    {
        *SystemState = false;
        UE_LOG(LogTemp, Log, TEXT("Master Architect: Deactivated system %s"), *SystemName);
        return true;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Master Architect: Cannot deactivate unregistered system %s"), *SystemName);
    return false;
}

void AEng_MasterArchitect::ValidateAllSystems()
{
    CriticalSystemFailures.Empty();
    
    for (const FEng_SystemSpecification& SystemSpec : RegisteredSystems)
    {
        if (!ValidateSystemCompliance(SystemSpec.SystemName))
        {
            CriticalSystemFailures.Add(SystemSpec.SystemName);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Master Architect: System validation complete. %d failures detected"), 
        CriticalSystemFailures.Num());
}

TArray<FString> AEng_MasterArchitect::GetFailedSystems()
{
    return CriticalSystemFailures;
}

float AEng_MasterArchitect::GetSystemPerformanceImpact(const FString& SystemName)
{
    float* PerformanceMetric = SystemPerformanceMetrics.Find(SystemName);
    return PerformanceMetric ? *PerformanceMetric : 0.0f;
}

bool AEng_MasterArchitect::IsPerformanceBudgetExceeded()
{
    float TotalPerformanceUsage = 0.0f;
    
    for (const auto& Metric : SystemPerformanceMetrics)
    {
        TotalPerformanceUsage += Metric.Value;
    }
    
    return TotalPerformanceUsage > GlobalPerformanceBudget;
}

void AEng_MasterArchitect::OptimizeSystemPerformance()
{
    UE_LOG(LogTemp, Warning, TEXT("Master Architect: Initiating system performance optimization"));
    
    // Update performance metrics
    UpdatePerformanceMetrics();
    
    // Identify systems exceeding budget
    TArray<FString> OverBudgetSystems;
    for (const auto& Metric : SystemPerformanceMetrics)
    {
        if (Metric.Value > 5.0f) // Systems using more than 5% of budget
        {
            OverBudgetSystems.Add(Metric.Key);
        }
    }
    
    // Log optimization recommendations
    for (const FString& SystemName : OverBudgetSystems)
    {
        UE_LOG(LogTemp, Warning, TEXT("Master Architect: System %s requires performance optimization"), *SystemName);
    }
}

FString AEng_MasterArchitect::GenerateArchitecturalReport()
{
    FString Report = TEXT("=== MASTER ARCHITECT REPORT ===\n");
    Report += FString::Printf(TEXT("Total Registered Systems: %d\n"), RegisteredSystems.Num());
    Report += FString::Printf(TEXT("Active Systems: %d\n"), SystemActiveStates.Num());
    Report += FString::Printf(TEXT("Failed Systems: %d\n"), CriticalSystemFailures.Num());
    Report += FString::Printf(TEXT("Architectural Rules: %d\n"), CoreArchitecturalRules.Num());
    
    float TotalPerformanceUsage = 0.0f;
    for (const auto& Metric : SystemPerformanceMetrics)
    {
        TotalPerformanceUsage += Metric.Value;
    }
    Report += FString::Printf(TEXT("Performance Usage: %.2f%% of %.2f%%\n"), TotalPerformanceUsage, GlobalPerformanceBudget);
    
    Report += TEXT("\n=== SYSTEM DETAILS ===\n");
    for (const FEng_SystemSpecification& SystemSpec : RegisteredSystems)
    {
        bool* IsActive = SystemActiveStates.Find(SystemSpec.SystemName);
        float* PerformanceMetric = SystemPerformanceMetrics.Find(SystemSpec.SystemName);
        
        Report += FString::Printf(TEXT("- %s: %s (%.2f%% performance)\n"), 
            *SystemSpec.SystemName,
            (IsActive && *IsActive) ? TEXT("ACTIVE") : TEXT("INACTIVE"),
            PerformanceMetric ? *PerformanceMetric : 0.0f);
    }
    
    return Report;
}

TArray<FString> AEng_MasterArchitect::GetSystemDependencies(const FString& SystemName)
{
    TArray<FString> Dependencies;
    
    FEng_SystemSpecification* SystemSpec = RegisteredSystems.FindByPredicate([&SystemName](const FEng_SystemSpecification& Spec)
    {
        return Spec.SystemName == SystemName;
    });
    
    if (SystemSpec)
    {
        Dependencies = SystemSpec->RequiredModules;
        Dependencies.Append(SystemSpec->OptionalModules);
    }
    
    return Dependencies;
}

void AEng_MasterArchitect::LogArchitecturalViolation(const FString& Violation)
{
    UE_LOG(LogTemp, Error, TEXT("ARCHITECTURAL VIOLATION: %s"), *Violation);
    
    // Add to critical failures if not already present
    if (!CriticalSystemFailures.Contains(Violation))
    {
        CriticalSystemFailures.Add(Violation);
    }
}

void AEng_MasterArchitect::ValidateProjectArchitecture()
{
    UE_LOG(LogTemp, Warning, TEXT("Master Architect: Starting project-wide architecture validation"));
    
    ValidateAllSystems();
    
    FString Report = GenerateArchitecturalReport();
    UE_LOG(LogTemp, Warning, TEXT("%s"), *Report);
    
    if (CriticalSystemFailures.Num() > 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Master Architect: %d critical failures detected!"), CriticalSystemFailures.Num());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Master Architect: All systems passed architectural validation"));
    }
}

void AEng_MasterArchitect::GenerateSystemDependencyGraph()
{
    UE_LOG(LogTemp, Warning, TEXT("Master Architect: Generating system dependency graph"));
    
    for (const FEng_SystemSpecification& SystemSpec : RegisteredSystems)
    {
        UE_LOG(LogTemp, Log, TEXT("System: %s"), *SystemSpec.SystemName);
        for (const FString& Dependency : SystemSpec.RequiredModules)
        {
            UE_LOG(LogTemp, Log, TEXT("  -> Requires: %s"), *Dependency);
        }
        for (const FString& OptionalDep : SystemSpec.OptionalModules)
        {
            UE_LOG(LogTemp, Log, TEXT("  -> Optional: %s"), *OptionalDep);
        }
    }
}

void AEng_MasterArchitect::AuditPerformanceBudgets()
{
    UE_LOG(LogTemp, Warning, TEXT("Master Architect: Auditing performance budgets"));
    
    UpdatePerformanceMetrics();
    
    float TotalUsage = 0.0f;
    for (const auto& Metric : SystemPerformanceMetrics)
    {
        TotalUsage += Metric.Value;
        UE_LOG(LogTemp, Log, TEXT("System %s: %.2f%% performance"), *Metric.Key, Metric.Value);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Total Performance Usage: %.2f%% of %.2f%% budget"), TotalUsage, GlobalPerformanceBudget);
    
    if (TotalUsage > GlobalPerformanceBudget)
    {
        UE_LOG(LogTemp, Error, TEXT("PERFORMANCE BUDGET EXCEEDED! Optimization required."));
    }
}

bool AEng_MasterArchitect::CheckSystemDependencies(const FString& SystemName)
{
    // Implementation for checking if all system dependencies are satisfied
    return true; // Simplified for now
}

void AEng_MasterArchitect::UpdatePerformanceMetrics()
{
    // Update performance metrics for all systems
    // This would typically interface with UE5's profiling systems
    for (auto& Metric : SystemPerformanceMetrics)
    {
        // Simulate performance data - in real implementation, this would query actual metrics
        Metric.Value = FMath::RandRange(0.5f, 4.0f);
    }
}