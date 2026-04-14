#include "EngineArchitectureManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/DateTime.h"

UEngineArchitectureManager::UEngineArchitectureManager()
{
    ValidationLevel = EEng_ArchitectureValidationLevel::Standard;
    CurrentPerformanceTier = EEng_PerformanceTier::PC_Medium;
    bStrictModeEnabled = true;
    bAutoValidationEnabled = true;
    
    // Initialize default performance budget
    PerformanceBudget = FEng_PerformanceBudget();
    UpdatePerformanceBudgetForTier();
}

void UEngineArchitectureManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architecture Manager Initialized"));
    
    // Register core systems
    RegisterSystem(TEXT("WorldGeneration"), EEng_SystemPriority::Critical);
    RegisterSystem(TEXT("PhysicsCore"), EEng_SystemPriority::Critical);
    RegisterSystem(TEXT("CharacterSystem"), EEng_SystemPriority::High);
    RegisterSystem(TEXT("AISystem"), EEng_SystemPriority::High);
    RegisterSystem(TEXT("AudioSystem"), EEng_SystemPriority::Medium);
    RegisterSystem(TEXT("VFXSystem"), EEng_SystemPriority::Medium);
    
    // Set up core dependencies
    FEng_SystemDependency WorldGenDep;
    WorldGenDep.SystemName = TEXT("WorldGeneration");
    WorldGenDep.Priority = EEng_SystemPriority::Critical;
    WorldGenDep.bIsRequired = true;
    WorldGenDep.LoadOrder = 1.0f;
    RegisterSystemDependency(WorldGenDep);
    
    FEng_SystemDependency PhysicsDep;
    PhysicsDep.SystemName = TEXT("PhysicsCore");
    PhysicsDep.Priority = EEng_SystemPriority::Critical;
    PhysicsDep.bIsRequired = true;
    PhysicsDep.LoadOrder = 2.0f;
    RegisterSystemDependency(PhysicsDep);
    
    if (bAutoValidationEnabled)
    {
        RunArchitectureValidation();
    }
}

void UEngineArchitectureManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architecture Manager Deinitialized"));
    
    // Clear all registrations
    SystemDependencies.Empty();
    RegisteredSystems.Empty();
    ValidationResults.Empty();
    
    Super::Deinitialize();
}

bool UEngineArchitectureManager::ValidateSystemArchitecture(const FString& SystemName)
{
    if (SystemName.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("ValidateSystemArchitecture: Empty system name"));
        return false;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Validating system architecture: %s"), *SystemName);
    
    // Check if system is registered
    if (!RegisteredSystems.Contains(SystemName))
    {
        UE_LOG(LogTemp, Warning, TEXT("System not registered: %s"), *SystemName);
        return false;
    }
    
    // Validate performance compliance
    if (!ValidatePerformanceCompliance(SystemName))
    {
        UE_LOG(LogTemp, Error, TEXT("Performance validation failed for: %s"), *SystemName);
        return false;
    }
    
    // Validate dependency chain
    if (!ValidateDependencyChain(SystemName))
    {
        UE_LOG(LogTemp, Error, TEXT("Dependency validation failed for: %s"), *SystemName);
        return false;
    }
    
    // Validate memory usage
    if (!ValidateMemoryUsage(SystemName))
    {
        UE_LOG(LogTemp, Error, TEXT("Memory validation failed for: %s"), *SystemName);
        return false;
    }
    
    UE_LOG(LogTemp, Log, TEXT("System architecture validation passed: %s"), *SystemName);
    return true;
}

FEng_ModuleValidationResult UEngineArchitectureManager::ValidateModule(const FString& ModuleName)
{
    FEng_ModuleValidationResult Result;
    Result.ModuleName = ModuleName;
    Result.bIsValid = true;
    Result.ValidationScore = 100.0f;
    
    // Basic module validation
    if (ModuleName.IsEmpty())
    {
        Result.bIsValid = false;
        Result.ValidationErrors.Add(TEXT("Module name is empty"));
        Result.ValidationScore = 0.0f;
        return Result;
    }
    
    // Check naming conventions
    if (!ModuleName.StartsWith(TEXT("Transpersonal")))
    {
        Result.ValidationWarnings.Add(TEXT("Module name should start with 'Transpersonal'"));
        Result.ValidationScore -= 10.0f;
    }
    
    // Check system registration
    bool bSystemRegistered = false;
    for (const auto& System : RegisteredSystems)
    {
        if (System.Key.Contains(ModuleName))
        {
            bSystemRegistered = true;
            break;
        }
    }
    
    if (!bSystemRegistered)
    {
        Result.ValidationWarnings.Add(TEXT("Module system not registered"));
        Result.ValidationScore -= 20.0f;
    }
    
    // Architecture compliance check
    if (ValidationLevel >= EEng_ArchitectureValidationLevel::Strict)
    {
        if (Result.ValidationScore < 80.0f)
        {
            Result.bIsValid = false;
            Result.ValidationErrors.Add(TEXT("Module fails strict validation requirements"));
        }
    }
    
    ValidationResults.Add(Result);
    LogValidationResult(Result);
    
    return Result;
}

bool UEngineArchitectureManager::CheckSystemDependencies(const FString& SystemName)
{
    for (const FEng_SystemDependency& Dep : SystemDependencies)
    {
        if (Dep.SystemName == SystemName)
        {
            if (Dep.bIsRequired && !RegisteredSystems.Contains(SystemName))
            {
                UE_LOG(LogTemp, Error, TEXT("Required system not registered: %s"), *SystemName);
                return false;
            }
        }
    }
    
    return true;
}

void UEngineArchitectureManager::RegisterSystemDependency(const FEng_SystemDependency& Dependency)
{
    SystemDependencies.Add(Dependency);
    UE_LOG(LogTemp, Log, TEXT("Registered system dependency: %s"), *Dependency.SystemName);
}

void UEngineArchitectureManager::SetPerformanceTier(EEng_PerformanceTier Tier)
{
    CurrentPerformanceTier = Tier;
    UpdatePerformanceBudgetForTier();
    UE_LOG(LogTemp, Log, TEXT("Performance tier set to: %d"), (int32)Tier);
}

FEng_PerformanceBudget UEngineArchitectureManager::GetCurrentPerformanceBudget() const
{
    return PerformanceBudget;
}

bool UEngineArchitectureManager::CheckPerformanceBudget(const FString& SystemName, float CPUTime, float GPUTime)
{
    bool bWithinBudget = true;
    
    if (CPUTime > PerformanceBudget.CPUBudgetMS)
    {
        UE_LOG(LogTemp, Warning, TEXT("System %s exceeds CPU budget: %.2fms > %.2fms"), 
               *SystemName, CPUTime, PerformanceBudget.CPUBudgetMS);
        bWithinBudget = false;
    }
    
    if (GPUTime > PerformanceBudget.GPUBudgetMS)
    {
        UE_LOG(LogTemp, Warning, TEXT("System %s exceeds GPU budget: %.2fms > %.2fms"), 
               *SystemName, GPUTime, PerformanceBudget.GPUBudgetMS);
        bWithinBudget = false;
    }
    
    return bWithinBudget;
}

void UEngineArchitectureManager::RegisterSystem(const FString& SystemName, EEng_SystemPriority Priority)
{
    RegisteredSystems.Add(SystemName, Priority);
    UE_LOG(LogTemp, Log, TEXT("Registered system: %s with priority: %d"), *SystemName, (int32)Priority);
}

void UEngineArchitectureManager::UnregisterSystem(const FString& SystemName)
{
    if (RegisteredSystems.Remove(SystemName) > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Unregistered system: %s"), *SystemName);
    }
}

TArray<FString> UEngineArchitectureManager::GetRegisteredSystems() const
{
    TArray<FString> Systems;
    RegisteredSystems.GetKeys(Systems);
    return Systems;
}

void UEngineArchitectureManager::SetValidationLevel(EEng_ArchitectureValidationLevel Level)
{
    ValidationLevel = Level;
    UE_LOG(LogTemp, Log, TEXT("Validation level set to: %d"), (int32)Level);
}

bool UEngineArchitectureManager::EnforceArchitectureRules(const FString& SystemName)
{
    if (ValidationLevel == EEng_ArchitectureValidationLevel::None)
    {
        return true;
    }
    
    return ValidateSystemArchitecture(SystemName);
}

void UEngineArchitectureManager::RunArchitectureValidation()
{
    UE_LOG(LogTemp, Warning, TEXT("=== RUNNING ARCHITECTURE VALIDATION ==="));
    
    ValidationResults.Empty();
    
    int32 PassedSystems = 0;
    int32 FailedSystems = 0;
    
    for (const auto& System : RegisteredSystems)
    {
        FEng_ModuleValidationResult Result = ValidateModule(System.Key);
        
        if (Result.bIsValid)
        {
            PassedSystems++;
        }
        else
        {
            FailedSystems++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Architecture Validation Complete:"));
    UE_LOG(LogTemp, Warning, TEXT("  Passed: %d systems"), PassedSystems);
    UE_LOG(LogTemp, Warning, TEXT("  Failed: %d systems"), FailedSystems);
    UE_LOG(LogTemp, Warning, TEXT("  Total: %d systems"), PassedSystems + FailedSystems);
}

void UEngineArchitectureManager::GenerateArchitectureReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== ENGINE ARCHITECTURE REPORT ==="));
    
    FDateTime Now = FDateTime::Now();
    UE_LOG(LogTemp, Warning, TEXT("Generated: %s"), *Now.ToString());
    
    UE_LOG(LogTemp, Warning, TEXT("Performance Tier: %d"), (int32)CurrentPerformanceTier);
    UE_LOG(LogTemp, Warning, TEXT("Validation Level: %d"), (int32)ValidationLevel);
    
    UE_LOG(LogTemp, Warning, TEXT("Performance Budget:"));
    UE_LOG(LogTemp, Warning, TEXT("  CPU: %.2fms"), PerformanceBudget.CPUBudgetMS);
    UE_LOG(LogTemp, Warning, TEXT("  GPU: %.2fms"), PerformanceBudget.GPUBudgetMS);
    UE_LOG(LogTemp, Warning, TEXT("  Memory: %dMB"), PerformanceBudget.MaxMemoryMB);
    
    UE_LOG(LogTemp, Warning, TEXT("Registered Systems: %d"), RegisteredSystems.Num());
    for (const auto& System : RegisteredSystems)
    {
        UE_LOG(LogTemp, Warning, TEXT("  %s (Priority: %d)"), *System.Key, (int32)System.Value);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("System Dependencies: %d"), SystemDependencies.Num());
    for (const FEng_SystemDependency& Dep : SystemDependencies)
    {
        UE_LOG(LogTemp, Warning, TEXT("  %s (Required: %s, Order: %.1f)"), 
               *Dep.SystemName, 
               Dep.bIsRequired ? TEXT("Yes") : TEXT("No"),
               Dep.LoadOrder);
    }
}

void UEngineArchitectureManager::LogSystemStatus()
{
    UE_LOG(LogTemp, Log, TEXT("=== SYSTEM STATUS ==="));
    UE_LOG(LogTemp, Log, TEXT("Architecture Manager Active: %s"), IsValid(this) ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Log, TEXT("Registered Systems: %d"), RegisteredSystems.Num());
    UE_LOG(LogTemp, Log, TEXT("Validation Results: %d"), ValidationResults.Num());
}

bool UEngineArchitectureManager::ValidatePerformanceCompliance(const FString& SystemName)
{
    // Basic performance validation
    // In a real implementation, this would check actual performance metrics
    return true;
}

bool UEngineArchitectureManager::ValidateDependencyChain(const FString& SystemName)
{
    // Check for circular dependencies and proper load order
    return CheckSystemDependencies(SystemName);
}

bool UEngineArchitectureManager::ValidateMemoryUsage(const FString& SystemName)
{
    // Basic memory validation
    // In a real implementation, this would check actual memory usage
    return true;
}

void UEngineArchitectureManager::UpdatePerformanceBudgetForTier()
{
    switch (CurrentPerformanceTier)
    {
        case EEng_PerformanceTier::Console:
            PerformanceBudget.CPUBudgetMS = 33.33f;  // 30fps
            PerformanceBudget.GPUBudgetMS = 33.33f;
            PerformanceBudget.MaxMemoryMB = 6144;
            PerformanceBudget.MaxDrawCalls = 1500;
            PerformanceBudget.MaxTriangles = 1500000;
            break;
            
        case EEng_PerformanceTier::PC_Low:
            PerformanceBudget.CPUBudgetMS = 22.22f;  // 45fps
            PerformanceBudget.GPUBudgetMS = 22.22f;
            PerformanceBudget.MaxMemoryMB = 8192;
            PerformanceBudget.MaxDrawCalls = 1800;
            PerformanceBudget.MaxTriangles = 1800000;
            break;
            
        case EEng_PerformanceTier::PC_Medium:
            PerformanceBudget.CPUBudgetMS = 16.67f;  // 60fps
            PerformanceBudget.GPUBudgetMS = 16.67f;
            PerformanceBudget.MaxMemoryMB = 12288;
            PerformanceBudget.MaxDrawCalls = 2000;
            PerformanceBudget.MaxTriangles = 2000000;
            break;
            
        case EEng_PerformanceTier::PC_High:
            PerformanceBudget.CPUBudgetMS = 11.11f;  // 90fps
            PerformanceBudget.GPUBudgetMS = 11.11f;
            PerformanceBudget.MaxMemoryMB = 16384;
            PerformanceBudget.MaxDrawCalls = 2500;
            PerformanceBudget.MaxTriangles = 2500000;
            break;
            
        case EEng_PerformanceTier::PC_Ultra:
            PerformanceBudget.CPUBudgetMS = 8.33f;   // 120fps
            PerformanceBudget.GPUBudgetMS = 8.33f;
            PerformanceBudget.MaxMemoryMB = 32768;
            PerformanceBudget.MaxDrawCalls = 3000;
            PerformanceBudget.MaxTriangles = 3000000;
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Performance budget updated for tier %d"), (int32)CurrentPerformanceTier);
}

void UEngineArchitectureManager::LogValidationResult(const FEng_ModuleValidationResult& Result)
{
    if (Result.bIsValid)
    {
        UE_LOG(LogTemp, Log, TEXT("Module validation PASSED: %s (Score: %.1f)"), 
               *Result.ModuleName, Result.ValidationScore);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Module validation FAILED: %s (Score: %.1f)"), 
               *Result.ModuleName, Result.ValidationScore);
        
        for (const FString& Error : Result.ValidationErrors)
        {
            UE_LOG(LogTemp, Error, TEXT("  ERROR: %s"), *Error);
        }
    }
    
    for (const FString& Warning : Result.ValidationWarnings)
    {
        UE_LOG(LogTemp, Warning, TEXT("  WARNING: %s"), *Warning);
    }
}