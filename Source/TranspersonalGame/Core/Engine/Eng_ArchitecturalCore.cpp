#include "Eng_ArchitecturalCore.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/DateTime.h"

UEng_ArchitecturalCore::UEng_ArchitecturalCore()
{
    bEnforceStrictCompliance = true;
    MaxPerformanceBudgetTotal = 100.0f;
    MaxSystemsPerLayer = 20;
}

void UEng_ArchitecturalCore::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Eng_ArchitecturalCore: Supreme architectural authority system initialized"));
    
    // Initialize core architectural rules
    RegisteredSystems.Empty();
    ValidationResults.Empty();
    SystemPerformanceMetrics.Empty();
    
    // Register self as the foundational system
    FEng_SystemRegistration SelfRegistration;
    SelfRegistration.SystemName = TEXT("ArchitecturalCore");
    SelfRegistration.Layer = EEng_ArchitecturalLayer::Foundation;
    SelfRegistration.Priority = EEng_SystemPriority::Critical;
    SelfRegistration.PerformanceBudget = 5.0f;
    SelfRegistration.bIsActive = true;
    
    RegisterSystem(SelfRegistration);
    
    LogArchitecturalEvent(TEXT("Architectural Core system established as supreme authority"), EEng_ComplianceStatus::Compliant);
}

void UEng_ArchitecturalCore::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Eng_ArchitecturalCore: Architectural authority system shutting down"));
    
    // Generate final compliance report
    FString FinalReport = GenerateComplianceReport();
    UE_LOG(LogTemp, Warning, TEXT("Final Architectural Report: %s"), *FinalReport);
    
    // Clean up all data
    RegisteredSystems.Empty();
    ValidationResults.Empty();
    SystemPerformanceMetrics.Empty();
    
    Super::Deinitialize();
}

bool UEng_ArchitecturalCore::RegisterSystem(const FEng_SystemRegistration& SystemData)
{
    if (SystemData.SystemName.IsEmpty())
    {
        LogArchitecturalEvent(TEXT("System registration failed: Empty system name"), EEng_ComplianceStatus::Violation);
        return false;
    }
    
    if (RegisteredSystems.Contains(SystemData.SystemName))
    {
        LogArchitecturalEvent(FString::Printf(TEXT("System %s already registered - updating registration"), *SystemData.SystemName), EEng_ComplianceStatus::Warning);
    }
    
    // Validate system before registration
    if (!ValidateSystemInternal(SystemData))
    {
        LogArchitecturalEvent(FString::Printf(TEXT("System %s failed validation - registration denied"), *SystemData.SystemName), EEng_ComplianceStatus::Violation);
        return false;
    }
    
    // Register the system
    RegisteredSystems.Add(SystemData.SystemName, SystemData);
    SystemPerformanceMetrics.Add(SystemData.SystemName, 0.0f);
    
    LogArchitecturalEvent(FString::Printf(TEXT("System %s successfully registered in %s layer"), 
        *SystemData.SystemName, 
        *UEnum::GetValueAsString(SystemData.Layer)), 
        EEng_ComplianceStatus::Compliant);
    
    return true;
}

bool UEng_ArchitecturalCore::UnregisterSystem(const FString& SystemName)
{
    if (!RegisteredSystems.Contains(SystemName))
    {
        LogArchitecturalEvent(FString::Printf(TEXT("Cannot unregister system %s - not found"), *SystemName), EEng_ComplianceStatus::Warning);
        return false;
    }
    
    // Check if other systems depend on this one
    for (const auto& SystemPair : RegisteredSystems)
    {
        if (SystemPair.Value.Dependencies.Contains(SystemName))
        {
            LogArchitecturalEvent(FString::Printf(TEXT("Cannot unregister system %s - other systems depend on it"), *SystemName), EEng_ComplianceStatus::Violation);
            return false;
        }
    }
    
    RegisteredSystems.Remove(SystemName);
    SystemPerformanceMetrics.Remove(SystemName);
    
    LogArchitecturalEvent(FString::Printf(TEXT("System %s successfully unregistered"), *SystemName), EEng_ComplianceStatus::Compliant);
    return true;
}

bool UEng_ArchitecturalCore::IsSystemRegistered(const FString& SystemName) const
{
    return RegisteredSystems.Contains(SystemName);
}

TArray<FEng_SystemRegistration> UEng_ArchitecturalCore::GetRegisteredSystems() const
{
    TArray<FEng_SystemRegistration> Systems;
    RegisteredSystems.GenerateValueArray(Systems);
    return Systems;
}

FEng_ValidationResult UEng_ArchitecturalCore::ValidateSystem(const FString& SystemName)
{
    FEng_ValidationResult Result;
    Result.SystemName = SystemName;
    Result.Timestamp = FDateTime::Now().ToUnixTimestamp();
    
    if (!RegisteredSystems.Contains(SystemName))
    {
        Result.Status = EEng_ComplianceStatus::Violation;
        Result.ValidationMessage = TEXT("System not registered");
        UpdateValidationCache(Result);
        return Result;
    }
    
    const FEng_SystemRegistration& System = RegisteredSystems[SystemName];
    
    if (!ValidateSystemInternal(System))
    {
        Result.Status = EEng_ComplianceStatus::Violation;
        Result.ValidationMessage = TEXT("System failed internal validation");
    }
    else if (!CheckLayerCompliance(System))
    {
        Result.Status = EEng_ComplianceStatus::Warning;
        Result.ValidationMessage = TEXT("Layer compliance issues detected");
    }
    else if (!CheckDependencyCompliance(System))
    {
        Result.Status = EEng_ComplianceStatus::Warning;
        Result.ValidationMessage = TEXT("Dependency compliance issues detected");
    }
    else if (!CheckPerformanceCompliance(System))
    {
        Result.Status = EEng_ComplianceStatus::Warning;
        Result.ValidationMessage = TEXT("Performance compliance issues detected");
    }
    else
    {
        Result.Status = EEng_ComplianceStatus::Compliant;
        Result.ValidationMessage = TEXT("System fully compliant");
    }
    
    UpdateValidationCache(Result);
    return Result;
}

TArray<FEng_ValidationResult> UEng_ArchitecturalCore::ValidateAllSystems()
{
    TArray<FEng_ValidationResult> Results;
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        FEng_ValidationResult Result = ValidateSystem(SystemPair.Key);
        Results.Add(Result);
    }
    
    LogArchitecturalEvent(FString::Printf(TEXT("Validated %d systems"), Results.Num()), EEng_ComplianceStatus::Compliant);
    return Results;
}

bool UEng_ArchitecturalCore::EnforceArchitecturalCompliance()
{
    if (!bEnforceStrictCompliance)
    {
        LogArchitecturalEvent(TEXT("Strict compliance enforcement disabled"), EEng_ComplianceStatus::Warning);
        return true;
    }
    
    TArray<FEng_ValidationResult> Results = ValidateAllSystems();
    bool bAllCompliant = true;
    
    for (const FEng_ValidationResult& Result : Results)
    {
        if (Result.Status == EEng_ComplianceStatus::Violation || Result.Status == EEng_ComplianceStatus::Critical)
        {
            bAllCompliant = false;
            LogArchitecturalEvent(FString::Printf(TEXT("COMPLIANCE VIOLATION: %s - %s"), *Result.SystemName, *Result.ValidationMessage), EEng_ComplianceStatus::Critical);
        }
    }
    
    if (bAllCompliant)
    {
        LogArchitecturalEvent(TEXT("All systems are architecturally compliant"), EEng_ComplianceStatus::Compliant);
    }
    else
    {
        LogArchitecturalEvent(TEXT("ARCHITECTURAL COMPLIANCE FAILURES DETECTED"), EEng_ComplianceStatus::Critical);
    }
    
    return bAllCompliant;
}

bool UEng_ArchitecturalCore::SetSystemPerformanceBudget(const FString& SystemName, float Budget)
{
    if (!RegisteredSystems.Contains(SystemName))
    {
        LogArchitecturalEvent(FString::Printf(TEXT("Cannot set performance budget for unregistered system %s"), *SystemName), EEng_ComplianceStatus::Violation);
        return false;
    }
    
    if (Budget < 0.0f || Budget > MaxPerformanceBudgetTotal)
    {
        LogArchitecturalEvent(FString::Printf(TEXT("Invalid performance budget %f for system %s"), Budget, *SystemName), EEng_ComplianceStatus::Violation);
        return false;
    }
    
    RegisteredSystems[SystemName].PerformanceBudget = Budget;
    LogArchitecturalEvent(FString::Printf(TEXT("Performance budget set to %f for system %s"), Budget, *SystemName), EEng_ComplianceStatus::Compliant);
    
    return true;
}

float UEng_ArchitecturalCore::GetSystemPerformanceBudget(const FString& SystemName) const
{
    if (RegisteredSystems.Contains(SystemName))
    {
        return RegisteredSystems[SystemName].PerformanceBudget;
    }
    return 0.0f;
}

bool UEng_ArchitecturalCore::ValidatePerformanceBudgets()
{
    float TotalBudget = 0.0f;
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        TotalBudget += SystemPair.Value.PerformanceBudget;
    }
    
    if (TotalBudget > MaxPerformanceBudgetTotal)
    {
        LogArchitecturalEvent(FString::Printf(TEXT("Total performance budget %f exceeds maximum %f"), TotalBudget, MaxPerformanceBudgetTotal), EEng_ComplianceStatus::Violation);
        return false;
    }
    
    LogArchitecturalEvent(FString::Printf(TEXT("Performance budgets validated - total: %f/%f"), TotalBudget, MaxPerformanceBudgetTotal), EEng_ComplianceStatus::Compliant);
    return true;
}

TArray<FString> UEng_ArchitecturalCore::GetSystemsInLayer(EEng_ArchitecturalLayer Layer) const
{
    TArray<FString> SystemsInLayer;
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        if (SystemPair.Value.Layer == Layer)
        {
            SystemsInLayer.Add(SystemPair.Key);
        }
    }
    
    return SystemsInLayer;
}

bool UEng_ArchitecturalCore::ValidateLayerDependencies()
{
    // Foundation layer should have no dependencies
    TArray<FString> FoundationSystems = GetSystemsInLayer(EEng_ArchitecturalLayer::Foundation);
    for (const FString& SystemName : FoundationSystems)
    {
        if (RegisteredSystems[SystemName].Dependencies.Num() > 0)
        {
            LogArchitecturalEvent(FString::Printf(TEXT("Foundation layer system %s has dependencies - VIOLATION"), *SystemName), EEng_ComplianceStatus::Violation);
            return false;
        }
    }
    
    LogArchitecturalEvent(TEXT("Layer dependencies validated successfully"), EEng_ComplianceStatus::Compliant);
    return true;
}

bool UEng_ArchitecturalCore::SetSystemPriority(const FString& SystemName, EEng_SystemPriority Priority)
{
    if (!RegisteredSystems.Contains(SystemName))
    {
        LogArchitecturalEvent(FString::Printf(TEXT("Cannot set priority for unregistered system %s"), *SystemName), EEng_ComplianceStatus::Violation);
        return false;
    }
    
    RegisteredSystems[SystemName].Priority = Priority;
    LogArchitecturalEvent(FString::Printf(TEXT("Priority set to %s for system %s"), *UEnum::GetValueAsString(Priority), *SystemName), EEng_ComplianceStatus::Compliant);
    
    return true;
}

EEng_SystemPriority UEng_ArchitecturalCore::GetSystemPriority(const FString& SystemName) const
{
    if (RegisteredSystems.Contains(SystemName))
    {
        return RegisteredSystems[SystemName].Priority;
    }
    return EEng_SystemPriority::Low;
}

bool UEng_ArchitecturalCore::AddSystemDependency(const FString& SystemName, const FString& DependencyName)
{
    if (!RegisteredSystems.Contains(SystemName) || !RegisteredSystems.Contains(DependencyName))
    {
        LogArchitecturalEvent(FString::Printf(TEXT("Cannot add dependency - system not found: %s -> %s"), *SystemName, *DependencyName), EEng_ComplianceStatus::Violation);
        return false;
    }
    
    RegisteredSystems[SystemName].Dependencies.AddUnique(DependencyName);
    LogArchitecturalEvent(FString::Printf(TEXT("Dependency added: %s -> %s"), *SystemName, *DependencyName), EEng_ComplianceStatus::Compliant);
    
    return true;
}

bool UEng_ArchitecturalCore::RemoveSystemDependency(const FString& SystemName, const FString& DependencyName)
{
    if (!RegisteredSystems.Contains(SystemName))
    {
        LogArchitecturalEvent(FString::Printf(TEXT("Cannot remove dependency from unregistered system %s"), *SystemName), EEng_ComplianceStatus::Violation);
        return false;
    }
    
    RegisteredSystems[SystemName].Dependencies.Remove(DependencyName);
    LogArchitecturalEvent(FString::Printf(TEXT("Dependency removed: %s -> %s"), *SystemName, *DependencyName), EEng_ComplianceStatus::Compliant);
    
    return true;
}

TArray<FString> UEng_ArchitecturalCore::GetSystemDependencies(const FString& SystemName) const
{
    if (RegisteredSystems.Contains(SystemName))
    {
        return RegisteredSystems[SystemName].Dependencies;
    }
    return TArray<FString>();
}

bool UEng_ArchitecturalCore::ValidateDependencyChain(const FString& SystemName)
{
    if (!RegisteredSystems.Contains(SystemName))
    {
        return false;
    }
    
    TSet<FString> VisitedSystems;
    TArray<FString> DependencyStack;
    
    // Check for circular dependencies using DFS
    return ValidateDependencyChainRecursive(SystemName, VisitedSystems, DependencyStack);
}

FString UEng_ArchitecturalCore::GenerateArchitecturalReport()
{
    FString Report = TEXT("=== ARCHITECTURAL AUTHORITY REPORT ===\n");
    Report += FString::Printf(TEXT("Generated: %s\n"), *FDateTime::Now().ToString());
    Report += FString::Printf(TEXT("Total Registered Systems: %d\n"), RegisteredSystems.Num());
    Report += FString::Printf(TEXT("Strict Compliance: %s\n"), bEnforceStrictCompliance ? TEXT("ENABLED") : TEXT("DISABLED"));
    Report += TEXT("\n=== SYSTEM REGISTRY ===\n");
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        const FEng_SystemRegistration& System = SystemPair.Value;
        Report += FString::Printf(TEXT("System: %s\n"), *System.SystemName);
        Report += FString::Printf(TEXT("  Layer: %s\n"), *UEnum::GetValueAsString(System.Layer));
        Report += FString::Printf(TEXT("  Priority: %s\n"), *UEnum::GetValueAsString(System.Priority));
        Report += FString::Printf(TEXT("  Performance Budget: %.2f\n"), System.PerformanceBudget);
        Report += FString::Printf(TEXT("  Dependencies: %d\n"), System.Dependencies.Num());
        Report += FString::Printf(TEXT("  Active: %s\n"), System.bIsActive ? TEXT("YES") : TEXT("NO"));
        Report += TEXT("\n");
    }
    
    return Report;
}

FString UEng_ArchitecturalCore::GenerateComplianceReport()
{
    FString Report = TEXT("=== ARCHITECTURAL COMPLIANCE REPORT ===\n");
    Report += FString::Printf(TEXT("Generated: %s\n"), *FDateTime::Now().ToString());
    
    TArray<FEng_ValidationResult> Results = ValidateAllSystems();
    
    int32 CompliantCount = 0;
    int32 WarningCount = 0;
    int32 ViolationCount = 0;
    int32 CriticalCount = 0;
    
    for (const FEng_ValidationResult& Result : Results)
    {
        switch (Result.Status)
        {
            case EEng_ComplianceStatus::Compliant: CompliantCount++; break;
            case EEng_ComplianceStatus::Warning: WarningCount++; break;
            case EEng_ComplianceStatus::Violation: ViolationCount++; break;
            case EEng_ComplianceStatus::Critical: CriticalCount++; break;
        }
    }
    
    Report += FString::Printf(TEXT("Compliant Systems: %d\n"), CompliantCount);
    Report += FString::Printf(TEXT("Warning Systems: %d\n"), WarningCount);
    Report += FString::Printf(TEXT("Violation Systems: %d\n"), ViolationCount);
    Report += FString::Printf(TEXT("Critical Systems: %d\n"), CriticalCount);
    
    if (ViolationCount > 0 || CriticalCount > 0)
    {
        Report += TEXT("\n=== COMPLIANCE ISSUES ===\n");
        for (const FEng_ValidationResult& Result : Results)
        {
            if (Result.Status == EEng_ComplianceStatus::Violation || Result.Status == EEng_ComplianceStatus::Critical)
            {
                Report += FString::Printf(TEXT("%s: %s - %s\n"), *Result.SystemName, *UEnum::GetValueAsString(Result.Status), *Result.ValidationMessage);
            }
        }
    }
    
    return Report;
}

FString UEng_ArchitecturalCore::GeneratePerformanceReport()
{
    FString Report = TEXT("=== PERFORMANCE BUDGET REPORT ===\n");
    Report += FString::Printf(TEXT("Generated: %s\n"), *FDateTime::Now().ToString());
    
    float TotalBudget = 0.0f;
    for (const auto& SystemPair : RegisteredSystems)
    {
        TotalBudget += SystemPair.Value.PerformanceBudget;
    }
    
    Report += FString::Printf(TEXT("Total Budget Used: %.2f / %.2f (%.1f%%)\n"), TotalBudget, MaxPerformanceBudgetTotal, (TotalBudget / MaxPerformanceBudgetTotal) * 100.0f);
    Report += TEXT("\n=== SYSTEM BUDGETS ===\n");
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        const FEng_SystemRegistration& System = SystemPair.Value;
        float Percentage = (System.PerformanceBudget / MaxPerformanceBudgetTotal) * 100.0f;
        Report += FString::Printf(TEXT("%s: %.2f (%.1f%%)\n"), *System.SystemName, System.PerformanceBudget, Percentage);
    }
    
    return Report;
}

bool UEng_ArchitecturalCore::ValidateSystemInternal(const FEng_SystemRegistration& System)
{
    // Basic validation rules
    if (System.SystemName.IsEmpty())
    {
        return false;
    }
    
    if (System.PerformanceBudget < 0.0f)
    {
        return false;
    }
    
    // Check if dependencies exist
    for (const FString& Dependency : System.Dependencies)
    {
        if (!RegisteredSystems.Contains(Dependency))
        {
            LogArchitecturalEvent(FString::Printf(TEXT("System %s has unregistered dependency: %s"), *System.SystemName, *Dependency), EEng_ComplianceStatus::Warning);
        }
    }
    
    return true;
}

bool UEng_ArchitecturalCore::CheckLayerCompliance(const FEng_SystemRegistration& System)
{
    // Count systems in the same layer
    int32 SystemsInLayer = GetSystemsInLayer(System.Layer).Num();
    
    if (SystemsInLayer > MaxSystemsPerLayer)
    {
        LogArchitecturalEvent(FString::Printf(TEXT("Layer %s has too many systems: %d > %d"), *UEnum::GetValueAsString(System.Layer), SystemsInLayer, MaxSystemsPerLayer), EEng_ComplianceStatus::Warning);
        return false;
    }
    
    return true;
}

bool UEng_ArchitecturalCore::CheckDependencyCompliance(const FEng_SystemRegistration& System)
{
    // Foundation layer systems should have no dependencies
    if (System.Layer == EEng_ArchitecturalLayer::Foundation && System.Dependencies.Num() > 0)
    {
        return false;
    }
    
    // Check for circular dependencies
    return ValidateDependencyChain(System.SystemName);
}

bool UEng_ArchitecturalCore::CheckPerformanceCompliance(const FEng_SystemRegistration& System)
{
    if (System.PerformanceBudget > MaxPerformanceBudgetTotal * 0.5f)
    {
        LogArchitecturalEvent(FString::Printf(TEXT("System %s uses excessive performance budget: %.2f"), *System.SystemName, System.PerformanceBudget), EEng_ComplianceStatus::Warning);
        return false;
    }
    
    return true;
}

void UEng_ArchitecturalCore::LogArchitecturalEvent(const FString& Message, EEng_ComplianceStatus Severity)
{
    FString LogMessage = FString::Printf(TEXT("[ARCHITECTURAL CORE] %s"), *Message);
    
    switch (Severity)
    {
        case EEng_ComplianceStatus::Compliant:
            UE_LOG(LogTemp, Log, TEXT("%s"), *LogMessage);
            break;
        case EEng_ComplianceStatus::Warning:
            UE_LOG(LogTemp, Warning, TEXT("%s"), *LogMessage);
            break;
        case EEng_ComplianceStatus::Violation:
        case EEng_ComplianceStatus::Critical:
            UE_LOG(LogTemp, Error, TEXT("%s"), *LogMessage);
            break;
        default:
            UE_LOG(LogTemp, Log, TEXT("%s"), *LogMessage);
            break;
    }
}

void UEng_ArchitecturalCore::UpdateValidationCache(const FEng_ValidationResult& Result)
{
    // Remove old results for the same system
    ValidationResults.RemoveAll([&Result](const FEng_ValidationResult& ExistingResult)
    {
        return ExistingResult.SystemName == Result.SystemName;
    });
    
    // Add new result
    ValidationResults.Add(Result);
    
    // Clean up old results if cache is too large
    if (ValidationResults.Num() > 1000)
    {
        CleanupValidationCache();
    }
}

void UEng_ArchitecturalCore::CleanupValidationCache()
{
    // Remove results older than 1 hour
    float CurrentTime = FDateTime::Now().ToUnixTimestamp();
    float OneHourAgo = CurrentTime - 3600.0f;
    
    ValidationResults.RemoveAll([OneHourAgo](const FEng_ValidationResult& Result)
    {
        return Result.Timestamp < OneHourAgo;
    });
}

bool UEng_ArchitecturalCore::ValidateDependencyChainRecursive(const FString& SystemName, TSet<FString>& VisitedSystems, TArray<FString>& DependencyStack)
{
    if (DependencyStack.Contains(SystemName))
    {
        // Circular dependency detected
        LogArchitecturalEvent(FString::Printf(TEXT("Circular dependency detected in chain: %s"), *FString::Join(DependencyStack, TEXT(" -> "))), EEng_ComplianceStatus::Violation);
        return false;
    }
    
    if (VisitedSystems.Contains(SystemName))
    {
        // Already validated this branch
        return true;
    }
    
    VisitedSystems.Add(SystemName);
    DependencyStack.Add(SystemName);
    
    if (RegisteredSystems.Contains(SystemName))
    {
        const FEng_SystemRegistration& System = RegisteredSystems[SystemName];
        for (const FString& Dependency : System.Dependencies)
        {
            if (!ValidateDependencyChainRecursive(Dependency, VisitedSystems, DependencyStack))
            {
                return false;
            }
        }
    }
    
    DependencyStack.Remove(SystemName);
    return true;
}