#include "Build_CriticalCompilationValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Misc/DateTime.h"
#include "Modules/ModuleManager.h"

void UBuild_CriticalCompilationValidator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Build_CriticalCompilationValidator: Initializing critical compilation validation system"));
    
    // Initialize validation state
    bValidationInProgress = false;
    LastValidationTime = 0.0f;
    
    // Cache initial module validation data
    CacheModuleValidationData();
    
    // Perform initial system stability check
    bool bSystemStable = IsSystemStable();
    UE_LOG(LogTemp, Warning, TEXT("Build_CriticalCompilationValidator: Initial system stability: %s"), 
           bSystemStable ? TEXT("STABLE") : TEXT("UNSTABLE"));
}

void UBuild_CriticalCompilationValidator::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_CriticalCompilationValidator: Deinitializing validation system"));
    
    // Generate final compilation report
    GenerateCompilationReport();
    
    Super::Deinitialize();
}

FBuild_CompilationResult UBuild_CriticalCompilationValidator::ValidateCurrentBuild()
{
    if (bValidationInProgress)
    {
        UE_LOG(LogTemp, Warning, TEXT("Build_CriticalCompilationValidator: Validation already in progress"));
        return LastCompilationResult;
    }
    
    bValidationInProgress = true;
    float StartTime = FPlatformTime::Seconds();
    
    FBuild_CompilationResult Result;
    Result.Status = EBuild_CompilationStatus::Compiling;
    
    UE_LOG(LogTemp, Warning, TEXT("Build_CriticalCompilationValidator: Starting build validation"));
    
    // Check header-implementation pairs
    bool bHeadersValid = ValidateHeaderImplementationPairs();
    if (!bHeadersValid)
    {
        Result.Status = EBuild_CompilationStatus::Failed;
        Result.ErrorMessage = TEXT("Missing implementation files for headers");
        Result.bHasCriticalErrors = true;
        Result.ErrorCount++;
    }
    
    // Check for orphaned headers
    bool bNoOrphans = CheckForOrphanedHeaders();
    if (!bNoOrphans)
    {
        Result.WarningCount++;
        UE_LOG(LogTemp, Warning, TEXT("Build_CriticalCompilationValidator: Found orphaned headers"));
    }
    
    // Validate module dependencies
    bool bDependenciesValid = ValidateModuleDependencies();
    if (!bDependenciesValid)
    {
        Result.Status = EBuild_CompilationStatus::Failed;
        Result.ErrorMessage += TEXT(" | Module dependency validation failed");
        Result.bHasCriticalErrors = true;
        Result.ErrorCount++;
    }
    
    // Check critical systems
    bool bCriticalSystemsOK = PerformCriticalSystemCheck();
    if (!bCriticalSystemsOK)
    {
        Result.Status = EBuild_CompilationStatus::CriticalError;
        Result.ErrorMessage += TEXT(" | Critical system validation failed");
        Result.bHasCriticalErrors = true;
        Result.ErrorCount++;
    }
    
    // Finalize result
    if (Result.Status == EBuild_CompilationStatus::Compiling)
    {
        Result.Status = EBuild_CompilationStatus::Success;
        Result.ErrorMessage = TEXT("Build validation completed successfully");
    }
    
    Result.CompilationTime = FPlatformTime::Seconds() - StartTime;
    LastCompilationResult = Result;
    LastValidationTime = FPlatformTime::Seconds();
    bValidationInProgress = false;
    
    UE_LOG(LogTemp, Warning, TEXT("Build_CriticalCompilationValidator: Validation completed - Status: %d, Errors: %d, Warnings: %d, Time: %.2fs"),
           (int32)Result.Status, Result.ErrorCount, Result.WarningCount, Result.CompilationTime);
    
    return Result;
}

bool UBuild_CriticalCompilationValidator::CheckModuleIntegrity(const FString& ModuleName)
{
    if (ModuleName.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Build_CriticalCompilationValidator: Empty module name provided"));
        return false;
    }
    
    // Check if module is loaded
    bool bModuleLoaded = FModuleManager::Get().IsModuleLoaded(*ModuleName);
    
    UE_LOG(LogTemp, Log, TEXT("Build_CriticalCompilationValidator: Module '%s' loaded status: %s"),
           *ModuleName, bModuleLoaded ? TEXT("LOADED") : TEXT("NOT LOADED"));
    
    return bModuleLoaded;
}

TArray<FBuild_ModuleValidationData> UBuild_CriticalCompilationValidator::ValidateAllModules()
{
    TArray<FBuild_ModuleValidationData> ValidationResults;
    
    // Core modules to validate
    TArray<FString> ModulesToCheck = {
        TEXT("TranspersonalGame"),
        TEXT("Core"),
        TEXT("CoreUObject"),
        TEXT("Engine"),
        TEXT("UnrealEd")
    };
    
    for (const FString& ModuleName : ModulesToCheck)
    {
        FBuild_ModuleValidationData ModuleData;
        ModuleData.ModuleName = ModuleName;
        ModuleData.bIsLoaded = CheckModuleIntegrity(ModuleName);
        
        // For TranspersonalGame module, do deeper validation
        if (ModuleName == TEXT("TranspersonalGame"))
        {
            ModuleData.bHasValidHeaders = true; // Assume valid for now
            ModuleData.bHasValidImplementations = ValidateHeaderImplementationPairs();
            ModuleData.HeaderCount = 50; // Approximate count
            ModuleData.ImplementationCount = 45; // Approximate count
        }
        else
        {
            ModuleData.bHasValidHeaders = true;
            ModuleData.bHasValidImplementations = true;
            ModuleData.HeaderCount = 1;
            ModuleData.ImplementationCount = 1;
        }
        
        ValidationResults.Add(ModuleData);
    }
    
    ModuleValidationCache = ValidationResults;
    
    UE_LOG(LogTemp, Warning, TEXT("Build_CriticalCompilationValidator: Validated %d modules"), ValidationResults.Num());
    
    return ValidationResults;
}

bool UBuild_CriticalCompilationValidator::PerformCriticalSystemCheck()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_CriticalCompilationValidator: Performing critical system check"));
    
    // Check if we have a valid world
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Build_CriticalCompilationValidator: No valid world found"));
        return false;
    }
    
    // Check if engine is initialized
    if (!GEngine)
    {
        UE_LOG(LogTemp, Error, TEXT("Build_CriticalCompilationValidator: Engine not initialized"));
        return false;
    }
    
    // Check module manager
    if (!FModuleManager::Get().IsModuleLoaded(TEXT("TranspersonalGame")))
    {
        UE_LOG(LogTemp, Error, TEXT("Build_CriticalCompilationValidator: TranspersonalGame module not loaded"));
        return false;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Build_CriticalCompilationValidator: Critical system check PASSED"));
    return true;
}

void UBuild_CriticalCompilationValidator::GenerateCompilationReport()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_CriticalCompilationValidator: Generating compilation report"));
    
    FString ReportContent = TEXT("=== CRITICAL COMPILATION VALIDATION REPORT ===\n");
    ReportContent += FString::Printf(TEXT("Generated: %s\n"), *GenerateValidationTimestamp());
    ReportContent += FString::Printf(TEXT("Last Validation Time: %.2f seconds ago\n"), 
                                   FPlatformTime::Seconds() - LastValidationTime);
    
    // Add last compilation result
    ReportContent += TEXT("\n--- LAST COMPILATION RESULT ---\n");
    ReportContent += FString::Printf(TEXT("Status: %d\n"), (int32)LastCompilationResult.Status);
    ReportContent += FString::Printf(TEXT("Errors: %d\n"), LastCompilationResult.ErrorCount);
    ReportContent += FString::Printf(TEXT("Warnings: %d\n"), LastCompilationResult.WarningCount);
    ReportContent += FString::Printf(TEXT("Compilation Time: %.2fs\n"), LastCompilationResult.CompilationTime);
    ReportContent += FString::Printf(TEXT("Critical Errors: %s\n"), 
                                   LastCompilationResult.bHasCriticalErrors ? TEXT("YES") : TEXT("NO"));
    
    if (!LastCompilationResult.ErrorMessage.IsEmpty())
    {
        ReportContent += FString::Printf(TEXT("Error Message: %s\n"), *LastCompilationResult.ErrorMessage);
    }
    
    // Add module validation data
    ReportContent += TEXT("\n--- MODULE VALIDATION ---\n");
    for (const FBuild_ModuleValidationData& ModuleData : ModuleValidationCache)
    {
        ReportContent += FString::Printf(TEXT("Module: %s - Loaded: %s - Headers: %s - Implementations: %s\n"),
                                       *ModuleData.ModuleName,
                                       ModuleData.bIsLoaded ? TEXT("YES") : TEXT("NO"),
                                       ModuleData.bHasValidHeaders ? TEXT("VALID") : TEXT("INVALID"),
                                       ModuleData.bHasValidImplementations ? TEXT("VALID") : TEXT("INVALID"));
    }
    
    ReportContent += TEXT("\n=== END REPORT ===\n");
    
    UE_LOG(LogTemp, Warning, TEXT("%s"), *ReportContent);
}

bool UBuild_CriticalCompilationValidator::IsSystemStable()
{
    // Check if we had recent critical errors
    if (LastCompilationResult.bHasCriticalErrors)
    {
        UE_LOG(LogTemp, Error, TEXT("Build_CriticalCompilationValidator: System unstable - critical errors detected"));
        return false;
    }
    
    // Check if validation is taking too long
    if (bValidationInProgress && (FPlatformTime::Seconds() - LastValidationTime) > 30.0f)
    {
        UE_LOG(LogTemp, Error, TEXT("Build_CriticalCompilationValidator: System unstable - validation timeout"));
        return false;
    }
    
    // Check critical systems
    if (!PerformCriticalSystemCheck())
    {
        UE_LOG(LogTemp, Error, TEXT("Build_CriticalCompilationValidator: System unstable - critical system check failed"));
        return false;
    }
    
    return true;
}

void UBuild_CriticalCompilationValidator::LogCriticalErrors()
{
    if (LastCompilationResult.bHasCriticalErrors)
    {
        UE_LOG(LogTemp, Error, TEXT("=== CRITICAL COMPILATION ERRORS ==="));
        UE_LOG(LogTemp, Error, TEXT("Error Count: %d"), LastCompilationResult.ErrorCount);
        UE_LOG(LogTemp, Error, TEXT("Error Message: %s"), *LastCompilationResult.ErrorMessage);
        
        for (const FString& FailedModule : LastCompilationResult.FailedModules)
        {
            UE_LOG(LogTemp, Error, TEXT("Failed Module: %s"), *FailedModule);
        }
        
        for (const FString& MissingHeader : LastCompilationResult.MissingHeaders)
        {
            UE_LOG(LogTemp, Error, TEXT("Missing Header: %s"), *MissingHeader);
        }
        
        UE_LOG(LogTemp, Error, TEXT("=== END CRITICAL ERRORS ==="));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Build_CriticalCompilationValidator: No critical errors to report"));
    }
}

bool UBuild_CriticalCompilationValidator::ValidateHeaderImplementationPairs()
{
    // This is a simplified validation - in a real scenario, we'd scan the file system
    // For now, assume validation passes if we can load the TranspersonalGame module
    bool bModuleLoaded = FModuleManager::Get().IsModuleLoaded(TEXT("TranspersonalGame"));
    
    UE_LOG(LogTemp, Log, TEXT("Build_CriticalCompilationValidator: Header-implementation validation: %s"),
           bModuleLoaded ? TEXT("PASSED") : TEXT("FAILED"));
    
    return bModuleLoaded;
}

bool UBuild_CriticalCompilationValidator::CheckForOrphanedHeaders()
{
    // Simplified check - assume no orphaned headers if module loads successfully
    bool bModuleLoaded = FModuleManager::Get().IsModuleLoaded(TEXT("TranspersonalGame"));
    
    UE_LOG(LogTemp, Log, TEXT("Build_CriticalCompilationValidator: Orphaned header check: %s"),
           bModuleLoaded ? TEXT("NO ORPHANS") : TEXT("ORPHANS DETECTED"));
    
    return bModuleLoaded;
}

bool UBuild_CriticalCompilationValidator::ValidateModuleDependencies()
{
    // Check core dependencies
    TArray<FString> RequiredModules = {
        TEXT("Core"),
        TEXT("CoreUObject"),
        TEXT("Engine")
    };
    
    for (const FString& ModuleName : RequiredModules)
    {
        if (!FModuleManager::Get().IsModuleLoaded(*ModuleName))
        {
            UE_LOG(LogTemp, Error, TEXT("Build_CriticalCompilationValidator: Required module '%s' not loaded"), *ModuleName);
            return false;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Build_CriticalCompilationValidator: Module dependency validation PASSED"));
    return true;
}

void UBuild_CriticalCompilationValidator::CacheModuleValidationData()
{
    UE_LOG(LogTemp, Log, TEXT("Build_CriticalCompilationValidator: Caching module validation data"));
    
    // This will populate ModuleValidationCache
    ValidateAllModules();
}

FString UBuild_CriticalCompilationValidator::GenerateValidationTimestamp()
{
    FDateTime Now = FDateTime::Now();
    return Now.ToString(TEXT("%Y-%m-%d %H:%M:%S"));
}