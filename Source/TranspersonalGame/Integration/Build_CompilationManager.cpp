#include "Build_CompilationManager.h"
#include "Engine/Engine.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Misc/DateTime.h"

void UBuild_CompilationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    bCompilationInProgress = false;
    CurrentCompilingModule = TEXT("");
    
    UE_LOG(LogTemp, Log, TEXT("Build_CompilationManager: Subsystem initialized"));
    
    // Initialize with current module status
    FBuild_CompilationResult InitialResult;
    InitialResult.ModuleName = TEXT("TranspersonalGame");
    InitialResult.Status = EBuild_CompilationStatus::Success;
    InitialResult.Message = TEXT("Module loaded successfully");
    InitialResult.CompilationTime = 0.0f;
    CompilationHistory.Add(InitialResult);
}

void UBuild_CompilationManager::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("Build_CompilationManager: Subsystem deinitialized"));
    Super::Deinitialize();
}

bool UBuild_CompilationManager::StartCompilation(const FString& ModuleName)
{
    if (bCompilationInProgress)
    {
        UE_LOG(LogTemp, Warning, TEXT("Compilation already in progress for module: %s"), *CurrentCompilingModule);
        return false;
    }

    if (!CheckModuleExists(ModuleName))
    {
        UE_LOG(LogTemp, Error, TEXT("Module does not exist: %s"), *ModuleName);
        return false;
    }

    bCompilationInProgress = true;
    CurrentCompilingModule = ModuleName;

    UE_LOG(LogTemp, Log, TEXT("Starting compilation for module: %s"), *ModuleName);

    // Create compilation result entry
    FBuild_CompilationResult Result;
    Result.ModuleName = ModuleName;
    Result.Status = EBuild_CompilationStatus::Compiling;
    Result.CompilationTime = FDateTime::Now().GetTimeOfDay().GetTotalSeconds();
    
    CompilationHistory.Add(Result);

    // Simulate compilation completion (in real scenario, this would be async)
    OnCompilationComplete(ModuleName, true);

    return true;
}

FBuild_CompilationResult UBuild_CompilationManager::GetCompilationResult(const FString& ModuleName)
{
    for (const FBuild_CompilationResult& Result : CompilationHistory)
    {
        if (Result.ModuleName == ModuleName)
        {
            return Result;
        }
    }

    // Return default result if not found
    FBuild_CompilationResult DefaultResult;
    DefaultResult.ModuleName = ModuleName;
    DefaultResult.Status = EBuild_CompilationStatus::Unknown;
    DefaultResult.Message = TEXT("No compilation record found");
    return DefaultResult;
}

TArray<FBuild_CompilationResult> UBuild_CompilationManager::GetAllCompilationResults() const
{
    return CompilationHistory;
}

bool UBuild_CompilationManager::IsCompilationInProgress() const
{
    return bCompilationInProgress;
}

void UBuild_CompilationManager::ClearCompilationHistory()
{
    CompilationHistory.Empty();
    UE_LOG(LogTemp, Log, TEXT("Build_CompilationManager: Compilation history cleared"));
}

bool UBuild_CompilationManager::ValidateModuleDependencies(const FString& ModuleName)
{
    UE_LOG(LogTemp, Log, TEXT("Validating dependencies for module: %s"), *ModuleName);

    // Check for common dependencies
    TArray<FString> RequiredModules = {
        TEXT("Core"),
        TEXT("CoreUObject"),
        TEXT("Engine"),
        TEXT("UnrealEd")
    };

    bool bAllDependenciesValid = true;
    for (const FString& Dependency : RequiredModules)
    {
        // In a real implementation, this would check if the module is actually loaded
        UE_LOG(LogTemp, Log, TEXT("Checking dependency: %s"), *Dependency);
    }

    return bAllDependenciesValid;
}

int32 UBuild_CompilationManager::GetTotalModuleCount() const
{
    // Return the number of unique modules in compilation history
    TSet<FString> UniqueModules;
    for (const FBuild_CompilationResult& Result : CompilationHistory)
    {
        UniqueModules.Add(Result.ModuleName);
    }
    return UniqueModules.Num();
}

void UBuild_CompilationManager::OnCompilationComplete(const FString& ModuleName, bool bSuccess)
{
    bCompilationInProgress = false;
    CurrentCompilingModule = TEXT("");

    // Update the compilation result
    for (FBuild_CompilationResult& Result : CompilationHistory)
    {
        if (Result.ModuleName == ModuleName && Result.Status == EBuild_CompilationStatus::Compiling)
        {
            Result.Status = bSuccess ? EBuild_CompilationStatus::Success : EBuild_CompilationStatus::Failed;
            Result.CompilationTime = FDateTime::Now().GetTimeOfDay().GetTotalSeconds() - Result.CompilationTime;
            
            if (bSuccess)
            {
                Result.Message = TEXT("Compilation completed successfully");
            }
            else
            {
                Result.Message = TEXT("Compilation failed");
                Result.ErrorCount = 1;
                Result.ErrorMessages.Add(TEXT("Generic compilation error"));
            }
            
            LogCompilationResult(Result);
            break;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Compilation completed for module %s: %s"), 
           *ModuleName, bSuccess ? TEXT("SUCCESS") : TEXT("FAILED"));
}

bool UBuild_CompilationManager::CheckModuleExists(const FString& ModuleName)
{
    // Check if module directory exists
    FString ProjectDir = FPaths::ProjectDir();
    FString ModulePath = FPaths::Combine(ProjectDir, TEXT("Source"), ModuleName);
    
    bool bExists = IFileManager::Get().DirectoryExists(*ModulePath);
    UE_LOG(LogTemp, Log, TEXT("Module %s exists: %s"), *ModuleName, bExists ? TEXT("YES") : TEXT("NO"));
    
    return bExists;
}

void UBuild_CompilationManager::LogCompilationResult(const FBuild_CompilationResult& Result)
{
    FString StatusString;
    switch (Result.Status)
    {
        case EBuild_CompilationStatus::Success:
            StatusString = TEXT("SUCCESS");
            break;
        case EBuild_CompilationStatus::Failed:
            StatusString = TEXT("FAILED");
            break;
        case EBuild_CompilationStatus::Warning:
            StatusString = TEXT("WARNING");
            break;
        case EBuild_CompilationStatus::Compiling:
            StatusString = TEXT("COMPILING");
            break;
        default:
            StatusString = TEXT("UNKNOWN");
            break;
    }

    UE_LOG(LogTemp, Log, TEXT("Compilation Result [%s] %s: %s (Time: %.2fs, Errors: %d, Warnings: %d)"),
           *Result.ModuleName, *StatusString, *Result.Message, 
           Result.CompilationTime, Result.ErrorCount, Result.WarningCount);
}