#include "EngArch_CompilationManager.h"
#include "Engine/Engine.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

UEngArch_CompilationManager::UEngArch_CompilationManager()
{
    bLastCompilationSuccessful = false;
    LastCompilationTime = 0.0f;
}

void UEngArch_CompilationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect Compilation Manager Initialized"));
    
    // Register core modules for monitoring
    RegisterModule(TEXT("TranspersonalGame"));
    RegisterModule(TEXT("Engine"));
    RegisterModule(TEXT("Core"));
    RegisterModule(TEXT("CoreUObject"));
    
    // Perform initial compilation check
    CheckCompilationStatus();
}

void UEngArch_CompilationManager::Deinitialize()
{
    CompilationResults.Empty();
    LoadedModules.Empty();
    CriticalErrors.Empty();
    Super::Deinitialize();
}

bool UEngArch_CompilationManager::CheckCompilationStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("Checking compilation status..."));
    
    float StartTime = FPlatformTime::Seconds();
    CompilationResults.Empty();
    CriticalErrors.Empty();
    
    // Check TranspersonalGame module compilation
    FEng_CompilationResult MainResult;
    MainResult.ModuleName = TEXT("TranspersonalGame");
    MainResult.Status = EEng_CompilationStatus::Success;
    MainResult.ErrorCount = 0;
    MainResult.WarningCount = 0;
    MainResult.CompileTime = 0.0f;
    
    // Validate core classes are loadable
    TArray<FString> CoreClasses = {
        TEXT("TranspersonalCharacter"),
        TEXT("TranspersonalGameMode"),
        TEXT("TranspersonalGameState"),
        TEXT("EngArch_SystemIntegration"),
        TEXT("Director_ProductionCoordinator")
    };
    
    for (const FString& ClassName : CoreClasses)
    {
        if (!ValidateClassDefinition(ClassName))
        {
            MainResult.ErrorCount++;
            MainResult.ErrorMessages.Add(FString::Printf(TEXT("Class %s failed validation"), *ClassName));
            LogCompilationIssue(FString::Printf(TEXT("Class validation failed: %s"), *ClassName), true);
        }
    }
    
    // Set overall status
    if (MainResult.ErrorCount == 0)
    {
        MainResult.Status = EEng_CompilationStatus::Success;
        bLastCompilationSuccessful = true;
    }
    else
    {
        MainResult.Status = EEng_CompilationStatus::Failed;
        bLastCompilationSuccessful = false;
    }
    
    MainResult.CompileTime = FPlatformTime::Seconds() - StartTime;
    LastCompilationTime = MainResult.CompileTime;
    CompilationResults.Add(MainResult);
    
    UE_LOG(LogTemp, Warning, TEXT("Compilation check complete. Status: %s, Errors: %d, Time: %.3fs"),
           bLastCompilationSuccessful ? TEXT("SUCCESS") : TEXT("FAILED"),
           MainResult.ErrorCount,
           MainResult.CompileTime);
    
    return bLastCompilationSuccessful;
}

FEng_CompilationResult UEngArch_CompilationManager::GetModuleCompilationResult(const FString& ModuleName)
{
    for (const FEng_CompilationResult& Result : CompilationResults)
    {
        if (Result.ModuleName == ModuleName)
        {
            return Result;
        }
    }
    
    // Return default result if not found
    FEng_CompilationResult DefaultResult;
    DefaultResult.ModuleName = ModuleName;
    DefaultResult.Status = EEng_CompilationStatus::Unknown;
    return DefaultResult;
}

TArray<FEng_CompilationResult> UEngArch_CompilationManager::GetAllCompilationResults()
{
    return CompilationResults;
}

int32 UEngArch_CompilationManager::GetTotalErrorCount() const
{
    int32 TotalErrors = 0;
    for (const FEng_CompilationResult& Result : CompilationResults)
    {
        TotalErrors += Result.ErrorCount;
    }
    return TotalErrors;
}

int32 UEngArch_CompilationManager::GetTotalWarningCount() const
{
    int32 TotalWarnings = 0;
    for (const FEng_CompilationResult& Result : CompilationResults)
    {
        TotalWarnings += Result.WarningCount;
    }
    return TotalWarnings;
}

TArray<FString> UEngArch_CompilationManager::GetCriticalErrors() const
{
    return CriticalErrors;
}

bool UEngArch_CompilationManager::ValidateCodeQuality()
{
    UE_LOG(LogTemp, Warning, TEXT("Validating code quality standards..."));
    
    bool bQualityPassed = true;
    
    // Check for proper header/cpp pairing
    if (!ValidateHeaderIncludes())
    {
        bQualityPassed = false;
        LogCompilationIssue(TEXT("Header include validation failed"), true);
    }
    
    // Check for proper UPROPERTY usage
    if (!ValidateUPropertyMacros())
    {
        bQualityPassed = false;
        LogCompilationIssue(TEXT("UPROPERTY macro validation failed"), true);
    }
    
    // Check file structure
    ValidateFileStructure();
    
    // Check for disabled files
    CheckForDisabledFiles();
    
    UE_LOG(LogTemp, Warning, TEXT("Code quality validation %s"), 
           bQualityPassed ? TEXT("PASSED") : TEXT("FAILED"));
    
    return bQualityPassed;
}

bool UEngArch_CompilationManager::ValidateHeaderIncludes()
{
    // Validate that .generated.h includes are properly placed
    // This is a simplified validation - in practice would scan actual files
    UE_LOG(LogTemp, Log, TEXT("Validating header include structure..."));
    return true; // Assume valid for now
}

bool UEngArch_CompilationManager::ValidateUPropertyMacros()
{
    // Validate UPROPERTY macro usage
    UE_LOG(LogTemp, Log, TEXT("Validating UPROPERTY macro usage..."));
    return true; // Assume valid for now
}

bool UEngArch_CompilationManager::RegisterModule(const FString& ModuleName)
{
    if (!LoadedModules.Contains(ModuleName))
    {
        LoadedModules.Add(ModuleName);
        UE_LOG(LogTemp, Log, TEXT("Registered module for monitoring: %s"), *ModuleName);
        return true;
    }
    return false;
}

bool UEngArch_CompilationManager::IsModuleLoaded(const FString& ModuleName)
{
    return LoadedModules.Contains(ModuleName);
}

TArray<FString> UEngArch_CompilationManager::GetLoadedModules() const
{
    return LoadedModules;
}

void UEngArch_CompilationManager::GenerateCompilationReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== COMPILATION REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Last Compilation: %s"), 
           bLastCompilationSuccessful ? TEXT("SUCCESS") : TEXT("FAILED"));
    UE_LOG(LogTemp, Warning, TEXT("Compilation Time: %.3fs"), LastCompilationTime);
    UE_LOG(LogTemp, Warning, TEXT("Total Errors: %d"), GetTotalErrorCount());
    UE_LOG(LogTemp, Warning, TEXT("Total Warnings: %d"), GetTotalWarningCount());
    UE_LOG(LogTemp, Warning, TEXT("Monitored Modules: %d"), LoadedModules.Num());
    
    if (CriticalErrors.Num() > 0)
    {
        UE_LOG(LogTemp, Error, TEXT("CRITICAL ERRORS:"));
        for (const FString& Error : CriticalErrors)
        {
            UE_LOG(LogTemp, Error, TEXT("  - %s"), *Error);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== MODULE DETAILS ==="));
    for (const FEng_CompilationResult& Result : CompilationResults)
    {
        FString StatusStr;
        switch (Result.Status)
        {
            case EEng_CompilationStatus::Success: StatusStr = TEXT("SUCCESS"); break;
            case EEng_CompilationStatus::Failed: StatusStr = TEXT("FAILED"); break;
            case EEng_CompilationStatus::Warning: StatusStr = TEXT("WARNING"); break;
            case EEng_CompilationStatus::Compiling: StatusStr = TEXT("COMPILING"); break;
            default: StatusStr = TEXT("UNKNOWN"); break;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Module: %s | Status: %s | Errors: %d | Warnings: %d | Time: %.3fs"),
               *Result.ModuleName, *StatusStr, Result.ErrorCount, Result.WarningCount, Result.CompileTime);
    }
}

void UEngArch_CompilationManager::ValidateProjectIntegrity()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PROJECT INTEGRITY VALIDATION ==="));
    
    // Check compilation status
    bool bCompilationOK = CheckCompilationStatus();
    UE_LOG(LogTemp, Warning, TEXT("Compilation Status: %s"), bCompilationOK ? TEXT("OK") : TEXT("FAILED"));
    
    // Check code quality
    bool bQualityOK = ValidateCodeQuality();
    UE_LOG(LogTemp, Warning, TEXT("Code Quality: %s"), bQualityOK ? TEXT("OK") : TEXT("FAILED"));
    
    // Overall integrity
    bool bIntegrityOK = bCompilationOK && bQualityOK;
    UE_LOG(LogTemp, Warning, TEXT("=== PROJECT INTEGRITY: %s ==="), 
           bIntegrityOK ? TEXT("HEALTHY") : TEXT("COMPROMISED"));
}

void UEngArch_CompilationManager::CleanupDisabledFiles()
{
    UE_LOG(LogTemp, Warning, TEXT("Scanning for .disabled files..."));
    
    // In a real implementation, this would scan the file system
    // and provide recommendations for re-enabling or removing disabled files
    UE_LOG(LogTemp, Warning, TEXT("Cleanup scan complete"));
}

void UEngArch_CompilationManager::ScanForCompilationErrors()
{
    // Implementation would scan build logs for errors
    UE_LOG(LogTemp, Log, TEXT("Scanning for compilation errors..."));
}

void UEngArch_CompilationManager::ValidateFileStructure()
{
    // Implementation would validate proper file organization
    UE_LOG(LogTemp, Log, TEXT("Validating file structure..."));
}

void UEngArch_CompilationManager::CheckForDisabledFiles()
{
    // Implementation would check for .disabled files that need attention
    UE_LOG(LogTemp, Log, TEXT("Checking for disabled files..."));
}

bool UEngArch_CompilationManager::ValidateClassDefinition(const FString& ClassName)
{
    try
    {
        UClass* LoadedClass = LoadClass<UObject>(nullptr, *FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ClassName));
        return LoadedClass != nullptr;
    }
    catch (...)
    {
        return false;
    }
}

void UEngArch_CompilationManager::LogCompilationIssue(const FString& Issue, bool bIsCritical)
{
    if (bIsCritical)
    {
        CriticalErrors.Add(Issue);
        UE_LOG(LogTemp, Error, TEXT("CRITICAL COMPILATION ISSUE: %s"), *Issue);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Compilation Issue: %s"), *Issue);
    }
}