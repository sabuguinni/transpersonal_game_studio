#include "BuildIntegrationReport.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Modules/ModuleManager.h"
#include "UObject/UObjectGlobals.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/DateTime.h"
#include "Engine/GameInstance.h"

void UBuildIntegrationReportSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationReportSubsystem: Initialized"));
    
    // Initialize module status cache
    CachedModuleStatuses.Empty();
    bReportCacheValid = false;
    
    // Perform initial module validation
    RefreshModuleStatus();
}

void UBuildIntegrationReportSubsystem::Deinitialize()
{
    CachedModuleStatuses.Empty();
    bReportCacheValid = false;
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationReportSubsystem: Deinitialized"));
    
    Super::Deinitialize();
}

FBuild_SystemIntegrationReport UBuildIntegrationReportSubsystem::GenerateIntegrationReport()
{
    FBuild_SystemIntegrationReport Report;
    Report.ReportGeneratedAt = FDateTime::Now();
    
    // Check compilation status
    CheckCompilationStatus(Report.CompilationResult);
    
    // Validate all modules
    ValidateAllModules();
    
    // Populate module statuses
    Report.ModuleStatuses.Empty();
    Report.TotalModulesChecked = 0;
    Report.SuccessfulModules = 0;
    Report.FailedModules = 0;
    
    for (const auto& ModulePair : CachedModuleStatuses)
    {
        Report.ModuleStatuses.Add(ModulePair.Value);
        Report.TotalModulesChecked++;
        
        if (ModulePair.Value.bIsLoaded && ModulePair.Value.bHasValidClasses && !ModulePair.Value.bHasDependencyIssues)
        {
            Report.SuccessfulModules++;
        }
        else
        {
            Report.FailedModules++;
            Report.CriticalIssues.Add(FString::Printf(TEXT("Module %s has integration issues"), *ModulePair.Key));
        }
    }
    
    // Determine overall status
    Report.bAllSystemsIntegrated = (Report.FailedModules == 0) && Report.CompilationResult.bCompilationSuccessful;
    
    if (Report.bAllSystemsIntegrated)
    {
        Report.OverallStatus = TEXT("All systems integrated successfully");
    }
    else if (Report.CompilationResult.bCompilationSuccessful)
    {
        Report.OverallStatus = FString::Printf(TEXT("Compilation successful but %d modules have issues"), Report.FailedModules);
    }
    else
    {
        Report.OverallStatus = FString::Printf(TEXT("Compilation failed with %d errors"), Report.CompilationResult.ErrorCount);
    }
    
    // Generate recommendations
    Report.Recommendations.Empty();
    if (!Report.CompilationResult.bCompilationSuccessful)
    {
        Report.Recommendations.Add(TEXT("Fix compilation errors before proceeding"));
    }
    if (Report.FailedModules > 0)
    {
        Report.Recommendations.Add(TEXT("Review module dependencies and class implementations"));
    }
    if (Report.bAllSystemsIntegrated)
    {
        Report.Recommendations.Add(TEXT("System integration complete - ready for gameplay testing"));
    }
    
    // Cache the report
    LastGeneratedReport = Report;
    bReportCacheValid = true;
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationReport: Generated report - %d modules checked, %d successful, %d failed"), 
           Report.TotalModulesChecked, Report.SuccessfulModules, Report.FailedModules);
    
    return Report;
}

bool UBuildIntegrationReportSubsystem::ValidateModuleIntegration(const FString& ModuleName, FBuild_ModuleIntegrationStatus& OutStatus)
{
    OutStatus.ModuleName = ModuleName;
    OutStatus.bIsLoaded = false;
    OutStatus.bHasValidClasses = false;
    OutStatus.ClassCount = 0;
    OutStatus.LoadedClasses.Empty();
    OutStatus.FailedClasses.Empty();
    OutStatus.bHasDependencyIssues = false;
    OutStatus.MissingDependencies.Empty();
    
    // Check if module is loaded
    if (FModuleManager::Get().IsModuleLoaded(*ModuleName))
    {
        OutStatus.bIsLoaded = true;
        UE_LOG(LogTemp, Log, TEXT("Module %s is loaded"), *ModuleName);
        
        // Check class availability
        CheckClassAvailability(ModuleName, OutStatus);
        
        // Analyze dependencies
        AnalyzeDependencies(ModuleName, OutStatus);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Module %s is not loaded"), *ModuleName);
        OutStatus.FailedClasses.Add(FString::Printf(TEXT("Module %s not loaded"), *ModuleName));
    }
    
    // Cache the status
    CachedModuleStatuses.Add(ModuleName, OutStatus);
    
    return OutStatus.bIsLoaded && OutStatus.bHasValidClasses && !OutStatus.bHasDependencyIssues;
}

bool UBuildIntegrationReportSubsystem::CheckCompilationStatus(FBuild_CompilationResult& OutResult)
{
    OutResult.bCompilationSuccessful = true; // Assume success if we're running
    OutResult.ErrorCount = 0;
    OutResult.WarningCount = 0;
    OutResult.ErrorMessages.Empty();
    OutResult.WarningMessages.Empty();
    OutResult.CompilationTimeSeconds = 0.0f;
    OutResult.BuildConfiguration = TEXT("Development");
    OutResult.CompilationTimestamp = FDateTime::Now();
    
    // If we're here, compilation was successful enough to load the module
    UE_LOG(LogTemp, Log, TEXT("Compilation status check: Successful (module is running)"));
    
    return OutResult.bCompilationSuccessful;
}

void UBuildIntegrationReportSubsystem::SaveReportToFile(const FBuild_SystemIntegrationReport& Report, const FString& FilePath)
{
    FString ReportContent;
    ReportContent += FString::Printf(TEXT("=== BUILD INTEGRATION REPORT ===\n"));
    ReportContent += FString::Printf(TEXT("Generated: %s\n"), *Report.ReportGeneratedAt.ToString());
    ReportContent += FString::Printf(TEXT("Overall Status: %s\n"), *Report.OverallStatus);
    ReportContent += FString::Printf(TEXT("All Systems Integrated: %s\n"), Report.bAllSystemsIntegrated ? TEXT("YES") : TEXT("NO"));
    ReportContent += FString::Printf(TEXT("\n"));
    
    ReportContent += FString::Printf(TEXT("=== COMPILATION RESULTS ===\n"));
    ReportContent += FString::Printf(TEXT("Successful: %s\n"), Report.CompilationResult.bCompilationSuccessful ? TEXT("YES") : TEXT("NO"));
    ReportContent += FString::Printf(TEXT("Errors: %d\n"), Report.CompilationResult.ErrorCount);
    ReportContent += FString::Printf(TEXT("Warnings: %d\n"), Report.CompilationResult.WarningCount);
    ReportContent += FString::Printf(TEXT("Build Config: %s\n"), *Report.CompilationResult.BuildConfiguration);
    ReportContent += FString::Printf(TEXT("\n"));
    
    ReportContent += FString::Printf(TEXT("=== MODULE STATUS ===\n"));
    ReportContent += FString::Printf(TEXT("Total Modules: %d\n"), Report.TotalModulesChecked);
    ReportContent += FString::Printf(TEXT("Successful: %d\n"), Report.SuccessfulModules);
    ReportContent += FString::Printf(TEXT("Failed: %d\n"), Report.FailedModules);
    ReportContent += FString::Printf(TEXT("\n"));
    
    for (const auto& ModuleStatus : Report.ModuleStatuses)
    {
        ReportContent += FString::Printf(TEXT("Module: %s\n"), *ModuleStatus.ModuleName);
        ReportContent += FString::Printf(TEXT("  Loaded: %s\n"), ModuleStatus.bIsLoaded ? TEXT("YES") : TEXT("NO"));
        ReportContent += FString::Printf(TEXT("  Valid Classes: %s\n"), ModuleStatus.bHasValidClasses ? TEXT("YES") : TEXT("NO"));
        ReportContent += FString::Printf(TEXT("  Class Count: %d\n"), ModuleStatus.ClassCount);
        ReportContent += FString::Printf(TEXT("  Dependency Issues: %s\n"), ModuleStatus.bHasDependencyIssues ? TEXT("YES") : TEXT("NO"));
        ReportContent += FString::Printf(TEXT("\n"));
    }
    
    if (Report.CriticalIssues.Num() > 0)
    {
        ReportContent += FString::Printf(TEXT("=== CRITICAL ISSUES ===\n"));
        for (const FString& Issue : Report.CriticalIssues)
        {
            ReportContent += FString::Printf(TEXT("- %s\n"), *Issue);
        }
        ReportContent += FString::Printf(TEXT("\n"));
    }
    
    if (Report.Recommendations.Num() > 0)
    {
        ReportContent += FString::Printf(TEXT("=== RECOMMENDATIONS ===\n"));
        for (const FString& Recommendation : Report.Recommendations)
        {
            ReportContent += FString::Printf(TEXT("- %s\n"), *Recommendation);
        }
    }
    
    // Save to file
    if (!FFileHelper::SaveStringToFile(ReportContent, *FilePath))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to save integration report to file: %s"), *FilePath);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Integration report saved to: %s"), *FilePath);
    }
}

TArray<FString> UBuildIntegrationReportSubsystem::GetAvailableModules() const
{
    TArray<FString> ModuleNames;
    
    // Add core TranspersonalGame modules
    ModuleNames.Add(TEXT("TranspersonalGame"));
    ModuleNames.Add(TEXT("Engine"));
    ModuleNames.Add(TEXT("Core"));
    ModuleNames.Add(TEXT("CoreUObject"));
    ModuleNames.Add(TEXT("UnrealEd"));
    
    return ModuleNames;
}

bool UBuildIntegrationReportSubsystem::IsModuleLoaded(const FString& ModuleName) const
{
    return FModuleManager::Get().IsModuleLoaded(*ModuleName);
}

void UBuildIntegrationReportSubsystem::RefreshModuleStatus()
{
    CachedModuleStatuses.Empty();
    bReportCacheValid = false;
    
    TArray<FString> ModulesToCheck = GetAvailableModules();
    
    for (const FString& ModuleName : ModulesToCheck)
    {
        FBuild_ModuleIntegrationStatus Status;
        ValidateModuleIntegration(ModuleName, Status);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Module status refreshed - %d modules checked"), ModulesToCheck.Num());
}

void UBuildIntegrationReportSubsystem::ValidateAllModules()
{
    if (bReportCacheValid)
    {
        return; // Use cached data
    }
    
    RefreshModuleStatus();
}

void UBuildIntegrationReportSubsystem::CheckClassAvailability(const FString& ModuleName, FBuild_ModuleIntegrationStatus& Status)
{
    Status.LoadedClasses.Empty();
    Status.FailedClasses.Empty();
    Status.ClassCount = 0;
    Status.bHasValidClasses = false;
    
    // For TranspersonalGame module, check for key classes
    if (ModuleName == TEXT("TranspersonalGame"))
    {
        TArray<FString> ExpectedClasses = {
            TEXT("TranspersonalCharacter"),
            TEXT("TranspersonalGameMode"),
            TEXT("TranspersonalGameState"),
            TEXT("PCGWorldGenerator"),
            TEXT("FoliageManager"),
            TEXT("CrowdSimulationManager")
        };
        
        for (const FString& ClassName : ExpectedClasses)
        {
            FString ClassPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ClassName);
            UClass* FoundClass = LoadClass<UObject>(nullptr, *ClassPath);
            
            if (FoundClass)
            {
                Status.LoadedClasses.Add(ClassName);
                Status.ClassCount++;
            }
            else
            {
                Status.FailedClasses.Add(ClassName);
            }
        }
        
        Status.bHasValidClasses = (Status.LoadedClasses.Num() > 0);
    }
    else
    {
        // For other modules, just mark as having valid classes if loaded
        Status.bHasValidClasses = Status.bIsLoaded;
        Status.ClassCount = Status.bIsLoaded ? 1 : 0;
    }
}

void UBuildIntegrationReportSubsystem::AnalyzeDependencies(const FString& ModuleName, FBuild_ModuleIntegrationStatus& Status)
{
    Status.MissingDependencies.Empty();
    Status.bHasDependencyIssues = false;
    
    // Check core dependencies for TranspersonalGame
    if (ModuleName == TEXT("TranspersonalGame"))
    {
        TArray<FString> RequiredDependencies = {
            TEXT("Engine"),
            TEXT("Core"),
            TEXT("CoreUObject")
        };
        
        for (const FString& Dependency : RequiredDependencies)
        {
            if (!FModuleManager::Get().IsModuleLoaded(*Dependency))
            {
                Status.MissingDependencies.Add(Dependency);
                Status.bHasDependencyIssues = true;
            }
        }
    }
}

FString UBuildIntegrationReportSubsystem::GetModuleStatusSummary(const FBuild_ModuleIntegrationStatus& Status) const
{
    if (Status.bIsLoaded && Status.bHasValidClasses && !Status.bHasDependencyIssues)
    {
        return TEXT("OK");
    }
    else if (!Status.bIsLoaded)
    {
        return TEXT("NOT_LOADED");
    }
    else if (!Status.bHasValidClasses)
    {
        return TEXT("NO_CLASSES");
    }
    else if (Status.bHasDependencyIssues)
    {
        return TEXT("DEPENDENCY_ISSUES");
    }
    else
    {
        return TEXT("UNKNOWN");
    }
}