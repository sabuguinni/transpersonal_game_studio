#include "EngArch_CompilationValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/DateTime.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"
#include "Developer/HotReload/Public/IHotReload.h"
#include "Modules/ModuleManager.h"

UEngArch_CompilationValidator::UEngArch_CompilationValidator()
{
    bIsValidatingCompilation = false;
    LastValidationTime = 0.0f;
}

void UEngArch_CompilationValidator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("EngArch_CompilationValidator: Initialized"));
    
    // Perform initial validation
    RefreshCompilationStatus();
}

void UEngArch_CompilationValidator::Deinitialize()
{
    ModuleResults.Empty();
    
    Super::Deinitialize();
}

FEng_CompilationResult UEngArch_CompilationValidator::ValidateModuleCompilation(const FString& ModuleName)
{
    FEng_CompilationResult Result;
    Result.ModuleName = ModuleName;
    Result.Status = EEng_CompilationStatus::Unknown;
    
    if (ModuleName.IsEmpty())
    {
        Result.Status = EEng_CompilationStatus::Failed;
        Result.ErrorMessages.Add(TEXT("Module name is empty"));
        Result.ErrorCount = 1;
        return Result;
    }
    
    // Check if module is loaded
    FModuleManager& ModuleManager = FModuleManager::Get();
    if (ModuleManager.IsModuleLoaded(*ModuleName))
    {
        Result.Status = EEng_CompilationStatus::Success;
        UE_LOG(LogTemp, Log, TEXT("EngArch_CompilationValidator: Module %s is loaded and compiled"), *ModuleName);
    }
    else
    {
        // Try to load the module
        if (ModuleManager.ModuleExists(*ModuleName))
        {
            try
            {
                ModuleManager.LoadModule(*ModuleName);
                Result.Status = EEng_CompilationStatus::Success;
                UE_LOG(LogTemp, Log, TEXT("EngArch_CompilationValidator: Successfully loaded module %s"), *ModuleName);
            }
            catch (...)
            {
                Result.Status = EEng_CompilationStatus::Failed;
                Result.ErrorMessages.Add(FString::Printf(TEXT("Failed to load module: %s"), *ModuleName));
                Result.ErrorCount = 1;
                UE_LOG(LogTemp, Error, TEXT("EngArch_CompilationValidator: Failed to load module %s"), *ModuleName);
            }
        }
        else
        {
            Result.Status = EEng_CompilationStatus::Failed;
            Result.ErrorMessages.Add(FString::Printf(TEXT("Module does not exist: %s"), *ModuleName));
            Result.ErrorCount = 1;
        }
    }
    
    return Result;
}

bool UEngArch_CompilationValidator::IsProjectCompiled() const
{
    // Check if TranspersonalGame module is loaded (core requirement)
    FModuleManager& ModuleManager = FModuleManager::Get();
    return ModuleManager.IsModuleLoaded(TEXT("TranspersonalGame"));
}

TArray<FEng_CompilationResult> UEngArch_CompilationValidator::GetAllModuleStatus()
{
    return ModuleResults;
}

void UEngArch_CompilationValidator::RefreshCompilationStatus()
{
    if (bIsValidatingCompilation)
    {
        return;
    }
    
    bIsValidatingCompilation = true;
    LastValidationTime = FPlatformTime::Seconds();
    
    ModuleResults.Empty();
    
    // Validate core modules
    TArray<FString> CoreModules = {
        TEXT("TranspersonalGame"),
        TEXT("Engine"),
        TEXT("Core"),
        TEXT("CoreUObject")
    };
    
    for (const FString& ModuleName : CoreModules)
    {
        FEng_CompilationResult Result = ValidateModuleCompilation(ModuleName);
        ModuleResults.Add(Result);
        BroadcastCompilationResult(Result);
    }
    
    bIsValidatingCompilation = false;
    
    UE_LOG(LogTemp, Log, TEXT("EngArch_CompilationValidator: Refreshed compilation status for %d modules"), ModuleResults.Num());
}

bool UEngArch_CompilationValidator::TriggerHotReload()
{
    // Check if hot reload is available
    IHotReloadInterface* HotReload = IHotReloadInterface::GetPtr();
    if (HotReload)
    {
        UE_LOG(LogTemp, Log, TEXT("EngArch_CompilationValidator: Triggering hot reload"));
        HotReload->DoHotReloadFromIDE();
        return true;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("EngArch_CompilationValidator: Hot reload interface not available"));
    return false;
}

void UEngArch_CompilationValidator::LogCompilationReport()
{
    UE_LOG(LogTemp, Log, TEXT("=== COMPILATION VALIDATION REPORT ==="));
    UE_LOG(LogTemp, Log, TEXT("Project Compiled: %s"), IsProjectCompiled() ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Log, TEXT("Last Validation: %.2f seconds ago"), FPlatformTime::Seconds() - LastValidationTime);
    UE_LOG(LogTemp, Log, TEXT("Modules Validated: %d"), ModuleResults.Num());
    
    int32 SuccessCount = 0;
    int32 FailedCount = 0;
    int32 TotalErrors = 0;
    
    for (const FEng_CompilationResult& Result : ModuleResults)
    {
        FString StatusString;
        switch (Result.Status)
        {
            case EEng_CompilationStatus::Success:
                StatusString = TEXT("SUCCESS");
                SuccessCount++;
                break;
            case EEng_CompilationStatus::Failed:
                StatusString = TEXT("FAILED");
                FailedCount++;
                break;
            case EEng_CompilationStatus::Warning:
                StatusString = TEXT("WARNING");
                break;
            default:
                StatusString = TEXT("UNKNOWN");
                break;
        }
        
        UE_LOG(LogTemp, Log, TEXT("  %s: %s (Errors: %d, Warnings: %d)"), 
               *Result.ModuleName, *StatusString, Result.ErrorCount, Result.WarningCount);
        
        TotalErrors += Result.ErrorCount;
        
        for (const FString& Error : Result.ErrorMessages)
        {
            UE_LOG(LogTemp, Log, TEXT("    ERROR: %s"), *Error);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("SUMMARY: %d Success, %d Failed, %d Total Errors"), SuccessCount, FailedCount, TotalErrors);
    UE_LOG(LogTemp, Log, TEXT("======================================"));
}

void UEngArch_CompilationValidator::ValidateAllModules()
{
    RefreshCompilationStatus();
}

FEng_CompilationResult UEngArch_CompilationValidator::ValidateSpecificModule(const FString& ModuleName)
{
    return ValidateModuleCompilation(ModuleName);
}

void UEngArch_CompilationValidator::BroadcastCompilationResult(const FEng_CompilationResult& Result)
{
    OnCompilationComplete.Broadcast(Result);
}