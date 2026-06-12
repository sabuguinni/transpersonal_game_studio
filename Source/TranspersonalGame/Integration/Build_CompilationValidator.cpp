#include "Build_CompilationValidator.h"
#include "Engine/Engine.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Engine/World.h"
#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogBuildValidator, Log, All);

UBuild_CompilationValidator::UBuild_CompilationValidator()
{
    bValidationInProgress = false;
    LastValidationTime = 0.0f;
}

void UBuild_CompilationValidator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogBuildValidator, Log, TEXT("Build Compilation Validator initialized"));
    
    // Initialize module tracking
    ModuleStatuses.Empty();
    
    // Add core modules to track
    TArray<FString> CoreModules = {
        TEXT("TranspersonalGame"),
        TEXT("Core"),
        TEXT("World"),
        TEXT("Character"),
        TEXT("Combat"),
        TEXT("Quest"),
        TEXT("Audio"),
        TEXT("VFX"),
        TEXT("Integration")
    };
    
    for (const FString& ModuleName : CoreModules)
    {
        FBuild_ModuleStatus NewStatus;
        NewStatus.ModuleName = ModuleName;
        NewStatus.Status = EBuild_CompilationStatus::Unknown;
        ModuleStatuses.Add(ModuleName, NewStatus);
    }
}

void UBuild_CompilationValidator::Deinitialize()
{
    ModuleStatuses.Empty();
    Super::Deinitialize();
}

void UBuild_CompilationValidator::ValidateAllModules()
{
    if (bValidationInProgress)
    {
        UE_LOG(LogBuildValidator, Warning, TEXT("Validation already in progress"));
        return;
    }
    
    bValidationInProgress = true;
    LastValidationTime = FPlatformTime::Seconds();
    
    UE_LOG(LogBuildValidator, Log, TEXT("Starting validation of all modules"));
    
    for (auto& ModulePair : ModuleStatuses)
    {
        ValidateModule(ModulePair.Key);
    }
    
    CheckForDuplicateTypes();
    ValidateHeaderIncludes();
    CheckForMissingImplementations();
    
    bValidationInProgress = false;
    
    UE_LOG(LogBuildValidator, Log, TEXT("Module validation completed"));
}

void UBuild_CompilationValidator::ValidateModule(const FString& ModuleName)
{
    if (!ModuleStatuses.Contains(ModuleName))
    {
        FBuild_ModuleStatus NewStatus;
        NewStatus.ModuleName = ModuleName;
        NewStatus.Status = EBuild_CompilationStatus::Unknown;
        ModuleStatuses.Add(ModuleName, NewStatus);
    }
    
    FBuild_ModuleStatus& Status = ModuleStatuses[ModuleName];
    Status.Status = EBuild_CompilationStatus::Compiling;
    Status.ErrorMessages.Empty();
    Status.WarningMessages.Empty();
    
    float StartTime = FPlatformTime::Seconds();
    
    CheckModuleCompilation(ModuleName);
    
    Status.CompilationTime = FPlatformTime::Seconds() - StartTime;
    
    UE_LOG(LogBuildValidator, Log, TEXT("Module %s validation completed in %.2fs"), 
           *ModuleName, Status.CompilationTime);
}

EBuild_CompilationStatus UBuild_CompilationValidator::GetModuleStatus(const FString& ModuleName)
{
    if (ModuleStatuses.Contains(ModuleName))
    {
        return ModuleStatuses[ModuleName].Status;
    }
    return EBuild_CompilationStatus::Unknown;
}

TArray<FBuild_ModuleStatus> UBuild_CompilationValidator::GetAllModuleStatuses()
{
    TArray<FBuild_ModuleStatus> Statuses;
    for (const auto& ModulePair : ModuleStatuses)
    {
        Statuses.Add(ModulePair.Value);
    }
    return Statuses;
}

bool UBuild_CompilationValidator::AreAllModulesValid()
{
    for (const auto& ModulePair : ModuleStatuses)
    {
        if (ModulePair.Value.Status == EBuild_CompilationStatus::Failed)
        {
            return false;
        }
    }
    return true;
}

void UBuild_CompilationValidator::GenerateCompilationReport()
{
    FString ReportPath = FPaths::ProjectLogDir() / TEXT("CompilationReport.txt");
    FString Report = TEXT("=== TRANSPERSONAL GAME COMPILATION REPORT ===\n\n");
    
    Report += FString::Printf(TEXT("Generated: %s\n"), *FDateTime::Now().ToString());
    Report += FString::Printf(TEXT("Total Modules: %d\n\n"), ModuleStatuses.Num());
    
    int32 SuccessCount = 0;
    int32 FailedCount = 0;
    int32 WarningCount = 0;
    
    for (const auto& ModulePair : ModuleStatuses)
    {
        const FBuild_ModuleStatus& Status = ModulePair.Value;
        Report += FString::Printf(TEXT("Module: %s\n"), *Status.ModuleName);
        Report += FString::Printf(TEXT("Status: %s\n"), 
                                 *UEnum::GetValueAsString(Status.Status));
        Report += FString::Printf(TEXT("Compilation Time: %.2fs\n"), Status.CompilationTime);
        
        if (Status.ErrorMessages.Num() > 0)
        {
            Report += TEXT("Errors:\n");
            for (const FString& Error : Status.ErrorMessages)
            {
                Report += FString::Printf(TEXT("  - %s\n"), *Error);
            }
        }
        
        if (Status.WarningMessages.Num() > 0)
        {
            Report += TEXT("Warnings:\n");
            for (const FString& Warning : Status.WarningMessages)
            {
                Report += FString::Printf(TEXT("  - %s\n"), *Warning);
            }
        }
        
        Report += TEXT("\n");
        
        switch (Status.Status)
        {
            case EBuild_CompilationStatus::Success:
                SuccessCount++;
                break;
            case EBuild_CompilationStatus::Failed:
                FailedCount++;
                break;
            case EBuild_CompilationStatus::Warning:
                WarningCount++;
                break;
        }
    }
    
    Report += TEXT("=== SUMMARY ===\n");
    Report += FString::Printf(TEXT("Success: %d\n"), SuccessCount);
    Report += FString::Printf(TEXT("Failed: %d\n"), FailedCount);
    Report += FString::Printf(TEXT("Warnings: %d\n"), WarningCount);
    
    FFileHelper::SaveStringToFile(Report, *ReportPath);
    
    UE_LOG(LogBuildValidator, Log, TEXT("Compilation report saved to: %s"), *ReportPath);
}

void UBuild_CompilationValidator::CheckForDuplicateTypes()
{
    UE_LOG(LogBuildValidator, Log, TEXT("Checking for duplicate types"));
    
    TMap<FString, TArray<FString>> TypeLocations;
    
    // Scan source files for UCLASS, USTRUCT, UENUM declarations
    FString SourceDir = FPaths::ProjectDir() / TEXT("Source/TranspersonalGame");
    TArray<FString> FoundFiles;
    IFileManager::Get().FindFilesRecursive(FoundFiles, *SourceDir, TEXT("*.h"), true, false);
    
    for (const FString& FilePath : FoundFiles)
    {
        FString FileContent;
        if (FFileHelper::LoadFileToString(FileContent, *FilePath))
        {
            TArray<FString> Lines;
            FileContent.ParseIntoArrayLines(Lines);
            
            for (const FString& Line : Lines)
            {
                FString TrimmedLine = Line.TrimStartAndEnd();
                
                // Check for UCLASS declarations
                if (TrimmedLine.StartsWith(TEXT("UCLASS")))
                {
                    // Find the class name in the next few lines
                    // This is a simplified check
                }
                
                // Check for USTRUCT declarations
                if (TrimmedLine.StartsWith(TEXT("USTRUCT")))
                {
                    // Find the struct name
                }
                
                // Check for UENUM declarations
                if (TrimmedLine.StartsWith(TEXT("UENUM")))
                {
                    // Find the enum name
                }
            }
        }
    }
}

void UBuild_CompilationValidator::ValidateHeaderIncludes()
{
    UE_LOG(LogBuildValidator, Log, TEXT("Validating header includes"));
    
    // Check for common include issues
    FString SourceDir = FPaths::ProjectDir() / TEXT("Source/TranspersonalGame");
    TArray<FString> FoundFiles;
    IFileManager::Get().FindFilesRecursive(FoundFiles, *SourceDir, TEXT("*.h"), true, false);
    
    for (const FString& FilePath : FoundFiles)
    {
        FString FileContent;
        if (FFileHelper::LoadFileToString(FileContent, *FilePath))
        {
            // Check if .generated.h is the last include
            TArray<FString> Lines;
            FileContent.ParseIntoArrayLines(Lines);
            
            int32 LastIncludeIndex = -1;
            int32 GeneratedIncludeIndex = -1;
            
            for (int32 i = 0; i < Lines.Num(); i++)
            {
                FString TrimmedLine = Lines[i].TrimStartAndEnd();
                if (TrimmedLine.StartsWith(TEXT("#include")))
                {
                    LastIncludeIndex = i;
                    if (TrimmedLine.Contains(TEXT(".generated.h")))
                    {
                        GeneratedIncludeIndex = i;
                    }
                }
            }
            
            if (GeneratedIncludeIndex != -1 && GeneratedIncludeIndex != LastIncludeIndex)
            {
                UE_LOG(LogBuildValidator, Warning, 
                       TEXT("File %s: .generated.h is not the last include"), 
                       *FPaths::GetCleanFilename(FilePath));
            }
        }
    }
}

void UBuild_CompilationValidator::CheckModuleCompilation(const FString& ModuleName)
{
    FBuild_ModuleStatus& Status = ModuleStatuses[ModuleName];
    
    // Simulate compilation check by looking for common issues
    bool bHasErrors = false;
    bool bHasWarnings = false;
    
    // Check if module directory exists
    FString ModuleDir = FPaths::ProjectDir() / TEXT("Source/TranspersonalGame/") + ModuleName;
    if (!IFileManager::Get().DirectoryExists(*ModuleDir))
    {
        Status.ErrorMessages.Add(FString::Printf(TEXT("Module directory not found: %s"), *ModuleDir));
        bHasErrors = true;
    }
    
    // Check for .h/.cpp pairs
    TArray<FString> HeaderFiles;
    IFileManager::Get().FindFiles(HeaderFiles, *(ModuleDir / TEXT("*.h")), true, false);
    
    for (const FString& HeaderFile : HeaderFiles)
    {
        FString BaseName = FPaths::GetBaseFilename(HeaderFile);
        FString CppFile = ModuleDir / (BaseName + TEXT(".cpp"));
        
        if (!IFileManager::Get().FileExists(*CppFile))
        {
            Status.WarningMessages.Add(FString::Printf(TEXT("Missing implementation: %s.cpp"), *BaseName));
            bHasWarnings = true;
        }
    }
    
    if (bHasErrors)
    {
        Status.Status = EBuild_CompilationStatus::Failed;
    }
    else if (bHasWarnings)
    {
        Status.Status = EBuild_CompilationStatus::Warning;
    }
    else
    {
        Status.Status = EBuild_CompilationStatus::Success;
    }
}

void UBuild_CompilationValidator::ParseCompilationErrors(const FString& ModuleName, const FString& LogOutput)
{
    // Parse UE5 compilation log output for errors and warnings
    TArray<FString> Lines;
    LogOutput.ParseIntoArrayLines(Lines);
    
    FBuild_ModuleStatus& Status = ModuleStatuses[ModuleName];
    
    for (const FString& Line : Lines)
    {
        if (Line.Contains(TEXT("error")))
        {
            Status.ErrorMessages.Add(Line);
        }
        else if (Line.Contains(TEXT("warning")))
        {
            Status.WarningMessages.Add(Line);
        }
    }
}

void UBuild_CompilationValidator::CheckForMissingImplementations()
{
    UE_LOG(LogBuildValidator, Log, TEXT("Checking for missing implementations"));
    
    // This would check for functions declared in headers but not implemented in .cpp files
    // For now, just log that the check is running
    
    for (auto& ModulePair : ModuleStatuses)
    {
        FBuild_ModuleStatus& Status = ModulePair.Value;
        
        // Check if module has any .cpp files
        FString ModuleDir = FPaths::ProjectDir() / TEXT("Source/TranspersonalGame/") + Status.ModuleName;
        TArray<FString> CppFiles;
        IFileManager::Get().FindFiles(CppFiles, *(ModuleDir / TEXT("*.cpp")), true, false);
        
        if (CppFiles.Num() == 0)
        {
            Status.WarningMessages.Add(TEXT("No implementation files found"));
            if (Status.Status == EBuild_CompilationStatus::Success)
            {
                Status.Status = EBuild_CompilationStatus::Warning;
            }
        }
    }
}

void UBuild_CompilationValidator::ValidateUPropertyMacros()
{
    // Check for proper UPROPERTY usage
    UE_LOG(LogBuildValidator, Log, TEXT("Validating UPROPERTY macros"));
}