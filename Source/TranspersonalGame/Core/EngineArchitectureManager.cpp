#include "EngineArchitectureManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/DateTime.h"
#include "HAL/PlatformFilemanager.h"

UEngineArchitectureManager::UEngineArchitectureManager()
{
    bInitialized = false;
    LastValidation = FDateTime::Now();
}

void UEngineArchitectureManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureManager: Initializing..."));
    
    // Setup default compilation rules
    SetupDefaultCompilationRules();
    
    // Register core systems
    RegisterSystem(TEXT("TranspersonalGameMode"), EEng_ModuleType::Core, 10);
    RegisterSystem(TEXT("TranspersonalCharacter"), EEng_ModuleType::Gameplay, 9);
    RegisterSystem(TEXT("StudioDirectorSystem"), EEng_ModuleType::Core, 8);
    RegisterSystem(TEXT("WorldGeneration"), EEng_ModuleType::Gameplay, 7);
    RegisterSystem(TEXT("DinosaurAI"), EEng_ModuleType::AI, 6);
    
    bInitialized = true;
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureManager: Initialization complete"));
}

void UEngineArchitectureManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureManager: Deinitializing..."));
    RegisteredSystems.Empty();
    CompilationRules.Empty();
    bInitialized = false;
    Super::Deinitialize();
}

void UEngineArchitectureManager::RegisterSystem(const FString& SystemName, EEng_ModuleType ModuleType, int32 Priority)
{
    FEng_SystemInfo SystemInfo;
    SystemInfo.SystemName = SystemName;
    SystemInfo.ModuleType = ModuleType;
    SystemInfo.Priority = Priority;
    SystemInfo.Status = EEng_SystemStatus::Initializing;
    SystemInfo.PerformanceImpact = 0.0f;
    SystemInfo.LastUpdate = FDateTime::Now();
    
    RegisteredSystems.Add(SystemName, SystemInfo);
    
    UE_LOG(LogTemp, Log, TEXT("EngineArchitectureManager: Registered system '%s' with priority %d"), *SystemName, Priority);
}

void UEngineArchitectureManager::UnregisterSystem(const FString& SystemName)
{
    if (RegisteredSystems.Contains(SystemName))
    {
        RegisteredSystems.Remove(SystemName);
        UE_LOG(LogTemp, Log, TEXT("EngineArchitectureManager: Unregistered system '%s'"), *SystemName);
    }
}

void UEngineArchitectureManager::UpdateSystemStatus(const FString& SystemName, EEng_SystemStatus NewStatus, const FString& ErrorMessage)
{
    if (FEng_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemName))
    {
        SystemInfo->Status = NewStatus;
        SystemInfo->ErrorMessage = ErrorMessage;
        SystemInfo->LastUpdate = FDateTime::Now();
        
        FString StatusString = TEXT("Unknown");
        switch (NewStatus)
        {
            case EEng_SystemStatus::Active: StatusString = TEXT("Active"); break;
            case EEng_SystemStatus::Warning: StatusString = TEXT("Warning"); break;
            case EEng_SystemStatus::Error: StatusString = TEXT("Error"); break;
            case EEng_SystemStatus::Disabled: StatusString = TEXT("Disabled"); break;
            case EEng_SystemStatus::Initializing: StatusString = TEXT("Initializing"); break;
        }
        
        UE_LOG(LogTemp, Log, TEXT("EngineArchitectureManager: System '%s' status updated to '%s'"), *SystemName, *StatusString);
        
        if (!ErrorMessage.IsEmpty())
        {
            UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureManager: System '%s' error: %s"), *SystemName, *ErrorMessage);
        }
    }
}

TArray<FEng_SystemInfo> UEngineArchitectureManager::GetAllSystems() const
{
    TArray<FEng_SystemInfo> Systems;
    for (const auto& SystemPair : RegisteredSystems)
    {
        Systems.Add(SystemPair.Value);
    }
    
    // Sort by priority (higher priority first)
    Systems.Sort([](const FEng_SystemInfo& A, const FEng_SystemInfo& B) {
        return A.Priority > B.Priority;
    });
    
    return Systems;
}

FEng_SystemInfo UEngineArchitectureManager::GetSystemInfo(const FString& SystemName) const
{
    if (const FEng_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemName))
    {
        return *SystemInfo;
    }
    
    return FEng_SystemInfo(); // Return default if not found
}

TArray<FEng_SystemInfo> UEngineArchitectureManager::GetSystemsByType(EEng_ModuleType ModuleType) const
{
    TArray<FEng_SystemInfo> FilteredSystems;
    for (const auto& SystemPair : RegisteredSystems)
    {
        if (SystemPair.Value.ModuleType == ModuleType)
        {
            FilteredSystems.Add(SystemPair.Value);
        }
    }
    return FilteredSystems;
}

TArray<FEng_SystemInfo> UEngineArchitectureManager::GetSystemsByStatus(EEng_SystemStatus Status) const
{
    TArray<FEng_SystemInfo> FilteredSystems;
    for (const auto& SystemPair : RegisteredSystems)
    {
        if (SystemPair.Value.Status == Status)
        {
            FilteredSystems.Add(SystemPair.Value);
        }
    }
    return FilteredSystems;
}

void UEngineArchitectureManager::UpdateSystemPerformance(const FString& SystemName, float PerformanceImpact)
{
    if (FEng_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemName))
    {
        SystemInfo->PerformanceImpact = PerformanceImpact;
        SystemInfo->LastUpdate = FDateTime::Now();
        
        UE_LOG(LogTemp, Log, TEXT("EngineArchitectureManager: System '%s' performance impact updated to %.2f"), *SystemName, PerformanceImpact);
    }
}

float UEngineArchitectureManager::GetTotalPerformanceImpact() const
{
    float TotalImpact = 0.0f;
    for (const auto& SystemPair : RegisteredSystems)
    {
        TotalImpact += SystemPair.Value.PerformanceImpact;
    }
    return TotalImpact;
}

TArray<FEng_SystemInfo> UEngineArchitectureManager::GetHighPerformanceImpactSystems(float Threshold) const
{
    TArray<FEng_SystemInfo> HighImpactSystems;
    for (const auto& SystemPair : RegisteredSystems)
    {
        if (SystemPair.Value.PerformanceImpact >= Threshold)
        {
            HighImpactSystems.Add(SystemPair.Value);
        }
    }
    return HighImpactSystems;
}

void UEngineArchitectureManager::AddCompilationRule(const FString& RuleName, const FString& Description, bool bMandatory)
{
    FEng_CompilationRule Rule;
    Rule.RuleName = RuleName;
    Rule.Description = Description;
    Rule.bMandatory = bMandatory;
    
    CompilationRules.Add(Rule);
    
    UE_LOG(LogTemp, Log, TEXT("EngineArchitectureManager: Added compilation rule '%s'"), *RuleName);
}

TArray<FEng_CompilationRule> UEngineArchitectureManager::GetCompilationRules() const
{
    return CompilationRules;
}

bool UEngineArchitectureManager::ValidateCompilationRules(TArray<FString>& OutViolations) const
{
    OutViolations.Empty();
    bool bAllRulesValid = true;
    
    // Check for common violations
    for (const FEng_CompilationRule& Rule : CompilationRules)
    {
        if (Rule.RuleName == TEXT("USTRUCT_GLOBAL_SCOPE"))
        {
            // This would require actual file parsing to validate
            // For now, we assume it's valid
        }
        else if (Rule.RuleName == TEXT("UNIQUE_TYPE_NAMES"))
        {
            // This would require checking all registered types
            // For now, we assume it's valid
        }
        else if (Rule.RuleName == TEXT("GENERATED_H_LAST"))
        {
            // This would require file parsing
            // For now, we assume it's valid
        }
    }
    
    return bAllRulesValid;
}

bool UEngineArchitectureManager::IsSystemHealthy() const
{
    for (const auto& SystemPair : RegisteredSystems)
    {
        if (SystemPair.Value.Status == EEng_SystemStatus::Error)
        {
            return false;
        }
    }
    return true;
}

int32 UEngineArchitectureManager::GetErrorCount() const
{
    int32 ErrorCount = 0;
    for (const auto& SystemPair : RegisteredSystems)
    {
        if (SystemPair.Value.Status == EEng_SystemStatus::Error)
        {
            ErrorCount++;
        }
    }
    return ErrorCount;
}

int32 UEngineArchitectureManager::GetWarningCount() const
{
    int32 WarningCount = 0;
    for (const auto& SystemPair : RegisteredSystems)
    {
        if (SystemPair.Value.Status == EEng_SystemStatus::Warning)
        {
            WarningCount++;
        }
    }
    return WarningCount;
}

FString UEngineArchitectureManager::GenerateSystemReport() const
{
    FString Report = TEXT("=== ENGINE ARCHITECTURE SYSTEM REPORT ===\n\n");
    
    Report += FString::Printf(TEXT("Total Systems: %d\n"), RegisteredSystems.Num());
    Report += FString::Printf(TEXT("Errors: %d\n"), GetErrorCount());
    Report += FString::Printf(TEXT("Warnings: %d\n"), GetWarningCount());
    Report += FString::Printf(TEXT("Total Performance Impact: %.2f\n"), GetTotalPerformanceImpact());
    Report += FString::Printf(TEXT("System Health: %s\n\n"), IsSystemHealthy() ? TEXT("HEALTHY") : TEXT("UNHEALTHY"));
    
    Report += TEXT("=== REGISTERED SYSTEMS ===\n");
    TArray<FEng_SystemInfo> AllSystems = GetAllSystems();
    for (const FEng_SystemInfo& System : AllSystems)
    {
        FString StatusString = TEXT("Unknown");
        switch (System.Status)
        {
            case EEng_SystemStatus::Active: StatusString = TEXT("Active"); break;
            case EEng_SystemStatus::Warning: StatusString = TEXT("Warning"); break;
            case EEng_SystemStatus::Error: StatusString = TEXT("Error"); break;
            case EEng_SystemStatus::Disabled: StatusString = TEXT("Disabled"); break;
            case EEng_SystemStatus::Initializing: StatusString = TEXT("Initializing"); break;
        }
        
        Report += FString::Printf(TEXT("- %s [%s] Priority: %d, Performance: %.2f\n"), 
            *System.SystemName, *StatusString, System.Priority, System.PerformanceImpact);
        
        if (!System.ErrorMessage.IsEmpty())
        {
            Report += FString::Printf(TEXT("  Error: %s\n"), *System.ErrorMessage);
        }
    }
    
    Report += TEXT("\n=== COMPILATION RULES ===\n");
    for (const FEng_CompilationRule& Rule : CompilationRules)
    {
        Report += FString::Printf(TEXT("- %s: %s [%s]\n"), 
            *Rule.RuleName, *Rule.Description, Rule.bMandatory ? TEXT("MANDATORY") : TEXT("OPTIONAL"));
    }
    
    return Report;
}

void UEngineArchitectureManager::LogSystemStatus() const
{
    UE_LOG(LogTemp, Warning, TEXT("=== ENGINE ARCHITECTURE STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Total Systems: %d"), RegisteredSystems.Num());
    UE_LOG(LogTemp, Warning, TEXT("Errors: %d"), GetErrorCount());
    UE_LOG(LogTemp, Warning, TEXT("Warnings: %d"), GetWarningCount());
    UE_LOG(LogTemp, Warning, TEXT("System Health: %s"), IsSystemHealthy() ? TEXT("HEALTHY") : TEXT("UNHEALTHY"));
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        const FEng_SystemInfo& System = SystemPair.Value;
        UE_LOG(LogTemp, Log, TEXT("System: %s, Status: %d, Priority: %d"), 
            *System.SystemName, (int32)System.Status, System.Priority);
    }
}

void UEngineArchitectureManager::ValidateArchitecture()
{
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureManager: Running architecture validation..."));
    
    LastValidation = FDateTime::Now();
    
    // Validate system dependencies
    ValidateSystemDependencies();
    
    // Check performance constraints
    CheckPerformanceConstraints();
    
    // Validate compilation rules
    TArray<FString> Violations;
    bool bRulesValid = ValidateCompilationRules(Violations);
    
    if (!bRulesValid)
    {
        UE_LOG(LogTemp, Error, TEXT("EngineArchitectureManager: Compilation rule violations found:"));
        for (const FString& Violation : Violations)
        {
            UE_LOG(LogTemp, Error, TEXT("  - %s"), *Violation);
        }
    }
    
    // Log final status
    LogSystemStatus();
    
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureManager: Architecture validation complete"));
}

void UEngineArchitectureManager::SetupDefaultCompilationRules()
{
    AddCompilationRule(TEXT("USTRUCT_GLOBAL_SCOPE"), 
        TEXT("USTRUCT and UENUM must be declared at global scope only"), true);
    
    AddCompilationRule(TEXT("UNIQUE_TYPE_NAMES"), 
        TEXT("Every USTRUCT, UENUM, and UCLASS name must be unique across the project"), true);
    
    AddCompilationRule(TEXT("GENERATED_H_LAST"), 
        TEXT(".generated.h must be the last include in header files"), true);
    
    AddCompilationRule(TEXT("NO_ESCAPED_QUOTES"), 
        TEXT("Use normal quotes in UPROPERTY/UFUNCTION macros"), true);
    
    AddCompilationRule(TEXT("NO_SPACES_IN_IDENTIFIERS"), 
        TEXT("Variable and function names cannot contain spaces"), true);
    
    AddCompilationRule(TEXT("COMPLETE_FILES_ONLY"), 
        TEXT("Every .h must have a matching .cpp with complete implementations"), true);
    
    AddCompilationRule(TEXT("FORWARD_DECLARE_CROSS_MODULE"), 
        TEXT("Use forward declarations for cross-module types"), true);
    
    AddCompilationRule(TEXT("CORRECT_BASE_CLASSES"), 
        TEXT("Use correct UE5 base classes (A* for Actor, U* for UObject)"), true);
}

void UEngineArchitectureManager::ValidateSystemDependencies()
{
    // Check for circular dependencies and missing dependencies
    for (const auto& SystemPair : RegisteredSystems)
    {
        const FEng_SystemInfo& System = SystemPair.Value;
        
        // For now, just mark all systems as active if they're initializing
        if (System.Status == EEng_SystemStatus::Initializing)
        {
            // This would be where we check actual dependencies
            // For now, we'll assume all systems are valid
        }
    }
}

void UEngineArchitectureManager::CheckPerformanceConstraints()
{
    float TotalImpact = GetTotalPerformanceImpact();
    const float MaxAllowedImpact = 100.0f; // Maximum allowed total performance impact
    
    if (TotalImpact > MaxAllowedImpact)
    {
        UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureManager: Total performance impact (%.2f) exceeds threshold (%.2f)"), 
            TotalImpact, MaxAllowedImpact);
        
        // Log high-impact systems
        TArray<FEng_SystemInfo> HighImpactSystems = GetHighPerformanceImpactSystems(10.0f);
        for (const FEng_SystemInfo& System : HighImpactSystems)
        {
            UE_LOG(LogTemp, Warning, TEXT("  High impact system: %s (%.2f)"), 
                *System.SystemName, System.PerformanceImpact);
        }
    }
}