#include "Eng_ArchitecturalGovernance.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogArchitecturalGovernance, Log, All);

UEng_ArchitecturalGovernance::UEng_ArchitecturalGovernance()
{
    bStrictComplianceMode = false;
    MinimumComplianceScore = 75.0f;
    bAutoEnforcement = true;
    PerformanceThreshold = 80.0f;
}

void UEng_ArchitecturalGovernance::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogArchitecturalGovernance, Log, TEXT("Architectural Governance System Initialized"));
    
    InitializeDefaultRules();
    ValidateSystemArchitecture();
    
    UE_LOG(LogArchitecturalGovernance, Log, TEXT("Architectural Governance: %d rules loaded, enforcement mode: %s"), 
           ArchitecturalRules.Num(), 
           bAutoEnforcement ? TEXT("AUTO") : TEXT("MANUAL"));
}

void UEng_ArchitecturalGovernance::Deinitialize()
{
    UE_LOG(LogArchitecturalGovernance, Log, TEXT("Architectural Governance System Shutdown"));
    
    // Clear all governance data
    ArchitecturalRules.Empty();
    RegisteredSystems.Empty();
    SystemPriorities.Empty();
    SystemMetrics.Empty();
    BlockedSystems.Empty();
    BlockReasons.Empty();
    
    Super::Deinitialize();
}

void UEng_ArchitecturalGovernance::RegisterArchitecturalRule(const FEng_ArchitecturalRule& Rule)
{
    ArchitecturalRules.Add(Rule);
    
    UE_LOG(LogArchitecturalGovernance, Log, TEXT("Architectural Rule Registered: %s [%s]"), 
           *Rule.RuleName, 
           *UEnum::GetValueAsString(Rule.RuleType));
    
    LogGovernanceAction(TEXT("RULE_REGISTERED"), 
                       FString::Printf(TEXT("Rule: %s, Layer: %s"), 
                                     *Rule.RuleName, 
                                     *UEnum::GetValueAsString(Rule.ApplicableLayer)));
}

bool UEng_ArchitecturalGovernance::ValidateSystemCompliance(const FString& SystemName)
{
    if (!IsSystemRegistered(SystemName))
    {
        UE_LOG(LogArchitecturalGovernance, Warning, TEXT("Cannot validate compliance for unregistered system: %s"), *SystemName);
        return false;
    }
    
    float ComplianceScore = CalculateComplianceScore(SystemName);
    bool bIsCompliant = ComplianceScore >= MinimumComplianceScore;
    
    UE_LOG(LogArchitecturalGovernance, Log, TEXT("System %s compliance: %.2f%% [%s]"), 
           *SystemName, 
           ComplianceScore, 
           bIsCompliant ? TEXT("PASS") : TEXT("FAIL"));
    
    if (!bIsCompliant && bAutoEnforcement)
    {
        BlockSystemActivation(SystemName, FString::Printf(TEXT("Compliance score %.2f%% below threshold %.2f%%"), 
                                                         ComplianceScore, MinimumComplianceScore));
    }
    
    return bIsCompliant;
}

float UEng_ArchitecturalGovernance::GetSystemComplianceScore(const FString& SystemName)
{
    return CalculateComplianceScore(SystemName);
}

TArray<FEng_ArchitecturalRule> UEng_ArchitecturalGovernance::GetRulesForLayer(EEng_ArchitecturalLayer Layer)
{
    TArray<FEng_ArchitecturalRule> LayerRules;
    
    for (const FEng_ArchitecturalRule& Rule : ArchitecturalRules)
    {
        if (Rule.ApplicableLayer == Layer)
        {
            LayerRules.Add(Rule);
        }
    }
    
    return LayerRules;
}

void UEng_ArchitecturalGovernance::RegisterSystem(const FString& SystemName, EEng_ArchitecturalLayer Layer, EEng_SystemPriority Priority)
{
    RegisteredSystems.Add(SystemName, Layer);
    SystemPriorities.Add(SystemName, Priority);
    
    // Initialize default metrics
    FEng_SystemMetrics DefaultMetrics;
    DefaultMetrics.SystemName = SystemName;
    SystemMetrics.Add(SystemName, DefaultMetrics);
    
    UE_LOG(LogArchitecturalGovernance, Log, TEXT("System Registered: %s [Layer: %s, Priority: %s]"), 
           *SystemName, 
           *UEnum::GetValueAsString(Layer), 
           *UEnum::GetValueAsString(Priority));
    
    LogGovernanceAction(TEXT("SYSTEM_REGISTERED"), 
                       FString::Printf(TEXT("System: %s, Layer: %s, Priority: %s"), 
                                     *SystemName, 
                                     *UEnum::GetValueAsString(Layer), 
                                     *UEnum::GetValueAsString(Priority)));
}

void UEng_ArchitecturalGovernance::UnregisterSystem(const FString& SystemName)
{
    RegisteredSystems.Remove(SystemName);
    SystemPriorities.Remove(SystemName);
    SystemMetrics.Remove(SystemName);
    BlockedSystems.Remove(SystemName);
    BlockReasons.Remove(SystemName);
    
    UE_LOG(LogArchitecturalGovernance, Log, TEXT("System Unregistered: %s"), *SystemName);
    
    LogGovernanceAction(TEXT("SYSTEM_UNREGISTERED"), FString::Printf(TEXT("System: %s"), *SystemName));
}

bool UEng_ArchitecturalGovernance::IsSystemRegistered(const FString& SystemName)
{
    return RegisteredSystems.Contains(SystemName);
}

TArray<FString> UEng_ArchitecturalGovernance::GetSystemsInLayer(EEng_ArchitecturalLayer Layer)
{
    TArray<FString> LayerSystems;
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        if (SystemPair.Value == Layer)
        {
            LayerSystems.Add(SystemPair.Key);
        }
    }
    
    return LayerSystems;
}

void UEng_ArchitecturalGovernance::UpdateSystemMetrics(const FString& SystemName, const FEng_SystemMetrics& Metrics)
{
    if (!IsSystemRegistered(SystemName))
    {
        UE_LOG(LogArchitecturalGovernance, Warning, TEXT("Cannot update metrics for unregistered system: %s"), *SystemName);
        return;
    }
    
    SystemMetrics.Add(SystemName, Metrics);
    
    // Check if system health changed
    bool bWasHealthy = SystemMetrics.Contains(SystemName) ? SystemMetrics[SystemName].bIsHealthy : true;
    if (bWasHealthy != Metrics.bIsHealthy)
    {
        UE_LOG(LogArchitecturalGovernance, Warning, TEXT("System %s health changed: %s -> %s"), 
               *SystemName, 
               bWasHealthy ? TEXT("HEALTHY") : TEXT("UNHEALTHY"),
               Metrics.bIsHealthy ? TEXT("HEALTHY") : TEXT("UNHEALTHY"));
    }
    
    // Auto-enforcement for performance issues
    if (bAutoEnforcement && !Metrics.bIsHealthy)
    {
        BlockSystemActivation(SystemName, TEXT("System health check failed"));
    }
}

FEng_SystemMetrics UEng_ArchitecturalGovernance::GetSystemMetrics(const FString& SystemName)
{
    if (SystemMetrics.Contains(SystemName))
    {
        return SystemMetrics[SystemName];
    }
    
    return FEng_SystemMetrics();
}

TArray<FString> UEng_ArchitecturalGovernance::GetUnhealthySystems()
{
    TArray<FString> UnhealthySystems;
    
    for (const auto& MetricsPair : SystemMetrics)
    {
        if (!MetricsPair.Value.bIsHealthy)
        {
            UnhealthySystems.Add(MetricsPair.Key);
        }
    }
    
    return UnhealthySystems;
}

float UEng_ArchitecturalGovernance::GetOverallArchitecturalHealth()
{
    if (SystemMetrics.Num() == 0)
    {
        return 100.0f;
    }
    
    int32 HealthySystems = 0;
    float TotalPerformance = 0.0f;
    
    for (const auto& MetricsPair : SystemMetrics)
    {
        if (MetricsPair.Value.bIsHealthy)
        {
            HealthySystems++;
        }
        TotalPerformance += MetricsPair.Value.PerformanceScore;
    }
    
    float HealthPercentage = (float)HealthySystems / SystemMetrics.Num() * 100.0f;
    float AvgPerformance = TotalPerformance / SystemMetrics.Num();
    
    return (HealthPercentage + AvgPerformance) / 2.0f;
}

void UEng_ArchitecturalGovernance::EnforceArchitecturalStandards()
{
    UE_LOG(LogArchitecturalGovernance, Log, TEXT("Enforcing Architectural Standards across %d systems"), RegisteredSystems.Num());
    
    int32 ViolationsFound = 0;
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        if (!ValidateSystemCompliance(SystemPair.Key))
        {
            ViolationsFound++;
        }
    }
    
    UE_LOG(LogArchitecturalGovernance, Log, TEXT("Architectural Standards Enforcement Complete: %d violations found"), ViolationsFound);
    
    LogGovernanceAction(TEXT("STANDARDS_ENFORCED"), 
                       FString::Printf(TEXT("Systems checked: %d, Violations: %d"), 
                                     RegisteredSystems.Num(), ViolationsFound));
}

bool UEng_ArchitecturalGovernance::CanSystemActivate(const FString& SystemName)
{
    return !BlockedSystems.Contains(SystemName);
}

void UEng_ArchitecturalGovernance::BlockSystemActivation(const FString& SystemName, const FString& Reason)
{
    if (!BlockedSystems.Contains(SystemName))
    {
        BlockedSystems.Add(SystemName);
        BlockReasons.Add(SystemName, Reason);
        
        UE_LOG(LogArchitecturalGovernance, Warning, TEXT("System BLOCKED: %s - Reason: %s"), *SystemName, *Reason);
        
        LogGovernanceAction(TEXT("SYSTEM_BLOCKED"), 
                           FString::Printf(TEXT("System: %s, Reason: %s"), *SystemName, *Reason));
    }
}

void UEng_ArchitecturalGovernance::UnblockSystemActivation(const FString& SystemName)
{
    if (BlockedSystems.Contains(SystemName))
    {
        BlockedSystems.Remove(SystemName);
        BlockReasons.Remove(SystemName);
        
        UE_LOG(LogArchitecturalGovernance, Log, TEXT("System UNBLOCKED: %s"), *SystemName);
        
        LogGovernanceAction(TEXT("SYSTEM_UNBLOCKED"), FString::Printf(TEXT("System: %s"), *SystemName));
    }
}

FString UEng_ArchitecturalGovernance::GenerateArchitecturalReport()
{
    FString Report = TEXT("=== ARCHITECTURAL GOVERNANCE REPORT ===\n\n");
    
    Report += FString::Printf(TEXT("Registered Systems: %d\n"), RegisteredSystems.Num());
    Report += FString::Printf(TEXT("Active Rules: %d\n"), ArchitecturalRules.Num());
    Report += FString::Printf(TEXT("Blocked Systems: %d\n"), BlockedSystems.Num());
    Report += FString::Printf(TEXT("Overall Health: %.2f%%\n\n"), GetOverallArchitecturalHealth());
    
    Report += TEXT("=== SYSTEM BREAKDOWN BY LAYER ===\n");
    for (int32 LayerInt = 0; LayerInt < (int32)EEng_ArchitecturalLayer::Performance + 1; LayerInt++)
    {
        EEng_ArchitecturalLayer Layer = (EEng_ArchitecturalLayer)LayerInt;
        TArray<FString> LayerSystems = GetSystemsInLayer(Layer);
        Report += FString::Printf(TEXT("%s: %d systems\n"), 
                                 *UEnum::GetValueAsString(Layer), 
                                 LayerSystems.Num());
    }
    
    Report += TEXT("\n=== COMPLIANCE VIOLATIONS ===\n");
    TArray<FString> Violations = GetComplianceViolations();
    for (const FString& Violation : Violations)
    {
        Report += FString::Printf(TEXT("- %s\n"), *Violation);
    }
    
    return Report;
}

TArray<FString> UEng_ArchitecturalGovernance::GetComplianceViolations()
{
    TArray<FString> Violations;
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        float ComplianceScore = CalculateComplianceScore(SystemPair.Key);
        if (ComplianceScore < MinimumComplianceScore)
        {
            Violations.Add(FString::Printf(TEXT("%s: %.2f%% compliance (below %.2f%% threshold)"), 
                                         *SystemPair.Key, 
                                         ComplianceScore, 
                                         MinimumComplianceScore));
        }
    }
    
    return Violations;
}

void UEng_ArchitecturalGovernance::LogArchitecturalDecision(const FString& Decision, const FString& Justification)
{
    UE_LOG(LogArchitecturalGovernance, Log, TEXT("ARCHITECTURAL DECISION: %s - %s"), *Decision, *Justification);
    
    LogGovernanceAction(TEXT("ARCHITECTURAL_DECISION"), 
                       FString::Printf(TEXT("Decision: %s, Justification: %s"), *Decision, *Justification));
}

void UEng_ArchitecturalGovernance::InitializeDefaultRules()
{
    // Foundation Layer Rules
    FEng_ArchitecturalRule FoundationRule;
    FoundationRule.RuleName = TEXT("Foundation System Stability");
    FoundationRule.RuleDescription = TEXT("All foundation systems must maintain 99%+ uptime");
    FoundationRule.RuleType = EEng_ArchitecturalRule::MandatoryCompliance;
    FoundationRule.ApplicableLayer = EEng_ArchitecturalLayer::Foundation;
    FoundationRule.bIsEnforced = true;
    FoundationRule.ComplianceScore = 100.0f;
    RegisterArchitecturalRule(FoundationRule);
    
    // Core Layer Rules
    FEng_ArchitecturalRule CoreRule;
    CoreRule.RuleName = TEXT("Core System Performance");
    CoreRule.RuleDescription = TEXT("Core systems must not exceed 60fps performance budget");
    CoreRule.RuleType = EEng_ArchitecturalRule::MandatoryCompliance;
    CoreRule.ApplicableLayer = EEng_ArchitecturalLayer::Core;
    CoreRule.bIsEnforced = true;
    CoreRule.ComplianceScore = 95.0f;
    RegisterArchitecturalRule(CoreRule);
    
    // Gameplay Layer Rules
    FEng_ArchitecturalRule GameplayRule;
    GameplayRule.RuleName = TEXT("Gameplay System Integration");
    GameplayRule.RuleDescription = TEXT("Gameplay systems must use shared interfaces for cross-system communication");
    GameplayRule.RuleType = EEng_ArchitecturalRule::StrongRecommendation;
    GameplayRule.ApplicableLayer = EEng_ArchitecturalLayer::Gameplay;
    GameplayRule.bIsEnforced = false;
    GameplayRule.ComplianceScore = 85.0f;
    RegisterArchitecturalRule(GameplayRule);
}

void UEng_ArchitecturalGovernance::ValidateSystemArchitecture()
{
    UE_LOG(LogArchitecturalGovernance, Log, TEXT("Validating system architecture..."));
    
    // This would contain more complex validation logic in a full implementation
    // For now, we just log that validation occurred
    
    UE_LOG(LogArchitecturalGovernance, Log, TEXT("System architecture validation complete"));
}

float UEng_ArchitecturalGovernance::CalculateComplianceScore(const FString& SystemName)
{
    if (!IsSystemRegistered(SystemName))
    {
        return 0.0f;
    }
    
    float TotalScore = 0.0f;
    int32 ApplicableRules = 0;
    
    EEng_ArchitecturalLayer SystemLayer = RegisteredSystems[SystemName];
    
    for (const FEng_ArchitecturalRule& Rule : ArchitecturalRules)
    {
        if (Rule.ApplicableLayer == SystemLayer)
        {
            ApplicableRules++;
            if (CheckRuleCompliance(SystemName, Rule))
            {
                TotalScore += Rule.ComplianceScore;
            }
        }
    }
    
    return ApplicableRules > 0 ? TotalScore / ApplicableRules : 100.0f;
}

bool UEng_ArchitecturalGovernance::CheckRuleCompliance(const FString& SystemName, const FEng_ArchitecturalRule& Rule)
{
    // This would contain specific compliance checking logic for each rule type
    // For now, we use system metrics to determine compliance
    
    if (SystemMetrics.Contains(SystemName))
    {
        const FEng_SystemMetrics& Metrics = SystemMetrics[SystemName];
        
        // Simple compliance check based on system health and performance
        if (!Metrics.bIsHealthy)
        {
            return false;
        }
        
        if (Metrics.PerformanceScore < PerformanceThreshold)
        {
            return false;
        }
    }
    
    return true;
}

void UEng_ArchitecturalGovernance::LogGovernanceAction(const FString& Action, const FString& Details)
{
    UE_LOG(LogArchitecturalGovernance, Log, TEXT("GOVERNANCE ACTION: %s - %s"), *Action, *Details);
}