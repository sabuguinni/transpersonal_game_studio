#include "QA_CompilationValidator.h"
#include "Engine/Engine.h"
#include "Misc/OutputDeviceRedirector.h"
#include "Misc/OutputDeviceHelper.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Modules/ModuleManager.h"
#include "Developer/HotReload/Public/IHotReload.h"

UQA_CompilationValidator::UQA_CompilationValidator()
{
    bIsValidating = false;
    LastValidationTime = 0.0f;
    InitializeModuleList();
}

void UQA_CompilationValidator::InitializeModuleList()
{
    ModulesToValidate.Empty();
    ModulesToValidate.Add(TEXT("TranspersonalGame"));
    ModulesToValidate.Add(TEXT("Core"));
    ModulesToValidate.Add(TEXT("Engine"));
    ModulesToValidate.Add(TEXT("UnrealEd"));
}

bool UQA_CompilationValidator::ValidateAllModules()
{
    if (bIsValidating)
    {
        UE_LOG(LogTemp, Warning, TEXT("QA_CompilationValidator: Validation already in progress"));
        return false;
    }

    bIsValidating = true;
    LastCompilationResults.Empty();

    UE_LOG(LogTemp, Log, TEXT("QA_CompilationValidator: Starting validation of %d modules"), ModulesToValidate.Num());

    for (const FString& ModuleName : ModulesToValidate)
    {
        FQA_CompilationResult Result = ValidateModule(ModuleName);
        LastCompilationResults.Add(Result);
    }

    bIsValidating = false;
    LastValidationTime = FPlatformTime::Seconds();

    // Log summary
    int32 TotalModules, SuccessfulModules, FailedModules;
    GetCompilationStats(TotalModules, SuccessfulModules, FailedModules);
    
    UE_LOG(LogTemp, Log, TEXT("QA_CompilationValidator: Validation complete - %d/%d modules successful"), 
           SuccessfulModules, TotalModules);

    return FailedModules == 0;
}

FQA_CompilationResult UQA_CompilationValidator::ValidateModule(const FString& ModuleName)
{
    FQA_CompilationResult Result;
    Result.ModuleName = ModuleName;
    
    double StartTime = FPlatformTime::Seconds();

    // Check if module is loaded
    if (FModuleManager::Get().IsModuleLoaded(*ModuleName))
    {
        Result.Status = EQA_CompilationStatus::Success;
        UE_LOG(LogTemp, Log, TEXT("QA_CompilationValidator: Module '%s' is loaded and functional"), *ModuleName);
    }
    else if (IsModuleValid(ModuleName))
    {
        // Try to load the module
        if (FModuleManager::Get().LoadModule(*ModuleName))
        {
            Result.Status = EQA_CompilationStatus::Success;
            UE_LOG(LogTemp, Log, TEXT("QA_CompilationValidator: Module '%s' loaded successfully"), *ModuleName);
        }
        else
        {
            Result.Status = EQA_CompilationStatus::Failed;
            Result.ErrorCount = 1;
            Result.ErrorMessages.Add(FString::Printf(TEXT("Failed to load module: %s"), *ModuleName));
            UE_LOG(LogTemp, Error, TEXT("QA_CompilationValidator: Failed to load module '%s'"), *ModuleName);
        }
    }
    else
    {
        Result.Status = EQA_CompilationStatus::Failed;
        Result.ErrorCount = 1;
        Result.ErrorMessages.Add(FString::Printf(TEXT("Module not found: %s"), *ModuleName));
        UE_LOG(LogTemp, Error, TEXT("QA_CompilationValidator: Module '%s' not found"), *ModuleName);
    }

    Result.CompilationTimeSeconds = FPlatformTime::Seconds() - StartTime;
    return Result;
}

bool UQA_CompilationValidator::CheckCompilationErrors()
{
    // Check the output log for compilation errors
    bool bHasErrors = false;
    
    // This is a simplified check - in a real implementation, you would
    // parse the actual build output or use the Hot Reload system
    if (GEngine && GEngine->GetEngineSubsystem<UEngineSubsystem>())
    {
        // Check if any modules failed to load
        for (const FString& ModuleName : ModulesToValidate)
        {
            if (!FModuleManager::Get().IsModuleLoaded(*ModuleName) && IsModuleValid(ModuleName))
            {
                bHasErrors = true;
                UE_LOG(LogTemp, Error, TEXT("QA_CompilationValidator: Module '%s' is not loaded"), *ModuleName);
            }
        }
    }

    return !bHasErrors;
}

void UQA_CompilationValidator::ForceRecompileAllModules()
{
    UE_LOG(LogTemp, Log, TEXT("QA_CompilationValidator: Force recompile requested"));

    // In editor builds, we can trigger hot reload
#if WITH_EDITOR
    if (IHotReloadInterface* HotReload = IHotReloadInterface::GetPtr())
    {
        UE_LOG(LogTemp, Log, TEXT("QA_CompilationValidator: Triggering hot reload"));
        HotReload->DoHotReloadFromIDE();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("QA_CompilationValidator: Hot reload interface not available"));
    }
#else
    UE_LOG(LogTemp, Warning, TEXT("QA_CompilationValidator: Hot reload not available in non-editor builds"));
#endif
}

void UQA_CompilationValidator::GetCompilationStats(int32& TotalModules, int32& SuccessfulModules, int32& FailedModules)
{
    TotalModules = LastCompilationResults.Num();
    SuccessfulModules = 0;
    FailedModules = 0;

    for (const FQA_CompilationResult& Result : LastCompilationResults)
    {
        if (Result.Status == EQA_CompilationStatus::Success)
        {
            SuccessfulModules++;
        }
        else if (Result.Status == EQA_CompilationStatus::Failed)
        {
            FailedModules++;
        }
    }
}

bool UQA_CompilationValidator::IsModuleValid(const FString& ModuleName)
{
    // Check if the module exists in the module manager
    return FModuleManager::Get().ModuleExists(*ModuleName);
}

FQA_CompilationResult UQA_CompilationValidator::ParseCompilationOutput(const FString& ModuleName, const FString& Output)
{
    FQA_CompilationResult Result;
    Result.ModuleName = ModuleName;

    // Simple parsing - count error and warning keywords
    TArray<FString> Lines;
    Output.ParseIntoArrayLines(Lines);

    for (const FString& Line : Lines)
    {
        if (Line.Contains(TEXT("error")) || Line.Contains(TEXT("Error")) || Line.Contains(TEXT("ERROR")))
        {
            Result.ErrorCount++;
            Result.ErrorMessages.Add(Line);
        }
        else if (Line.Contains(TEXT("warning")) || Line.Contains(TEXT("Warning")) || Line.Contains(TEXT("WARNING")))
        {
            Result.WarningCount++;
            Result.WarningMessages.Add(Line);
        }
    }

    // Determine status based on error count
    if (Result.ErrorCount > 0)
    {
        Result.Status = EQA_CompilationStatus::Failed;
    }
    else if (Result.WarningCount > 0)
    {
        Result.Status = EQA_CompilationStatus::Warning;
    }
    else
    {
        Result.Status = EQA_CompilationStatus::Success;
    }

    return Result;
}