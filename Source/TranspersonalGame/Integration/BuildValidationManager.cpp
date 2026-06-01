#include "BuildValidationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/AssetManager.h"

void UBuildValidationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    bValidationInProgress = false;
    OverallStatus = EBuildValidationStatus::Unknown;
    ValidationResults.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("BuildValidationManager initialized"));
}

void UBuildValidationManager::Deinitialize()
{
    ValidationResults.Empty();
    Super::Deinitialize();
}

void UBuildValidationManager::RunFullValidationSuite()
{
    if (bValidationInProgress)
    {
        UE_LOG(LogTemp, Warning, TEXT("Validation already in progress"));
        return;
    }

    bValidationInProgress = true;
    ValidationStartTime = FDateTime::Now();
    ValidationResults.Empty();
    TotalTests = 4;
    CompletedTests = 0;

    UE_LOG(LogTemp, Log, TEXT("Starting full validation suite"));

    // Run all validation tests
    ValidateModuleDependencies();
    ValidateAssetIntegrity();
    ValidateQAResults();
    ValidateCompilationStatus();

    bValidationInProgress = false;
    UpdateOverallStatus();

    FTimespan Duration = FDateTime::Now() - ValidationStartTime;
    UE_LOG(LogTemp, Log, TEXT("Validation suite completed in %.2f seconds"), Duration.GetTotalSeconds());
}

void UBuildValidationManager::ValidateModuleDependencies()
{
    FDateTime StartTime = FDateTime::Now();
    
    // Check if core modules are loaded
    bool bCoreModulesValid = true;
    FString ErrorMessage;

    // Check TranspersonalGame module
    if (!FModuleManager::Get().IsModuleLoaded("TranspersonalGame"))
    {
        bCoreModulesValid = false;
        ErrorMessage += TEXT("TranspersonalGame module not loaded. ");
    }

    // Check Engine modules
    TArray<FString> RequiredModules = {
        TEXT("Engine"),
        TEXT("CoreUObject"),
        TEXT("Core"),
        TEXT("UMG"),
        TEXT("Slate"),
        TEXT("SlateCore")
    };

    for (const FString& ModuleName : RequiredModules)
    {
        if (!FModuleManager::Get().IsModuleLoaded(*ModuleName))
        {
            bCoreModulesValid = false;
            ErrorMessage += FString::Printf(TEXT("%s module not loaded. "), *ModuleName);
        }
    }

    FTimespan Duration = FDateTime::Now() - StartTime;
    EBuildValidationStatus Status = bCoreModulesValid ? EBuildValidationStatus::Passed : EBuildValidationStatus::Failed;
    FString Message = bCoreModulesValid ? TEXT("All required modules loaded") : ErrorMessage;
    
    AddValidationResult(TEXT("Module Dependencies"), Status, Message, Duration.GetTotalSeconds());
    CompletedTests++;
}

void UBuildValidationManager::ValidateAssetIntegrity()
{
    FDateTime StartTime = FDateTime::Now();
    
    // Get asset registry
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

    // Count assets in game content
    TArray<FAssetData> GameAssets;
    AssetRegistry.GetAssetsByPath(FName("/Game"), GameAssets, true);

    bool bAssetsValid = GameAssets.Num() > 0;
    FString Message = FString::Printf(TEXT("Found %d assets in /Game directory"), GameAssets.Num());

    // Check for critical asset types
    int32 MapCount = 0;
    int32 BlueprintCount = 0;
    int32 MaterialCount = 0;

    for (const FAssetData& Asset : GameAssets)
    {
        if (Asset.AssetClass == UWorld::StaticClass()->GetFName())
        {
            MapCount++;
        }
        else if (Asset.AssetClass == FName("Blueprint"))
        {
            BlueprintCount++;
        }
        else if (Asset.AssetClass == FName("Material"))
        {
            MaterialCount++;
        }
    }

    Message += FString::Printf(TEXT(" (Maps: %d, Blueprints: %d, Materials: %d)"), MapCount, BlueprintCount, MaterialCount);

    FTimespan Duration = FDateTime::Now() - StartTime;
    EBuildValidationStatus Status = bAssetsValid ? EBuildValidationStatus::Passed : EBuildValidationStatus::Failed;
    
    AddValidationResult(TEXT("Asset Integrity"), Status, Message, Duration.GetTotalSeconds());
    CompletedTests++;
}

void UBuildValidationManager::ValidateQAResults()
{
    FDateTime StartTime = FDateTime::Now();
    
    // Check for QA results file
    FString ProjectDir = FPaths::ProjectDir();
    FString QAResultsPath = FPaths::Combine(ProjectDir, TEXT("Source/TranspersonalGame/QA/asset_requests.json"));
    
    bool bQAResultsExist = FPaths::FileExists(QAResultsPath);
    FString Message;
    EBuildValidationStatus Status;

    if (bQAResultsExist)
    {
        // Try to read file size
        int64 FileSize = IFileManager::Get().FileSize(*QAResultsPath);
        Message = FString::Printf(TEXT("QA results file found (%.2f KB)"), FileSize / 1024.0f);
        Status = EBuildValidationStatus::Passed;
    }
    else
    {
        Message = TEXT("QA results file not found - may indicate incomplete testing");
        Status = EBuildValidationStatus::Warning;
    }

    FTimespan Duration = FDateTime::Now() - StartTime;
    AddValidationResult(TEXT("QA Results"), Status, Message, Duration.GetTotalSeconds());
    CompletedTests++;
}

void UBuildValidationManager::ValidateCompilationStatus()
{
    FDateTime StartTime = FDateTime::Now();
    
    // Check for compiled binaries
    FString ProjectDir = FPaths::ProjectDir();
    FString BinariesDir = FPaths::Combine(ProjectDir, TEXT("Binaries"));
    
    bool bBinariesExist = FPaths::DirectoryExists(BinariesDir);
    FString Message;
    EBuildValidationStatus Status;

    if (bBinariesExist)
    {
        // Count binary files
        TArray<FString> BinaryFiles;
        IFileManager::Get().FindFilesRecursive(BinaryFiles, *BinariesDir, TEXT("*.so"), true, false);
        IFileManager::Get().FindFilesRecursive(BinaryFiles, *BinariesDir, TEXT("*.dll"), true, false);
        IFileManager::Get().FindFilesRecursive(BinaryFiles, *BinariesDir, TEXT("*.dylib"), true, false);

        Message = FString::Printf(TEXT("Found %d compiled binary files"), BinaryFiles.Num());
        Status = BinaryFiles.Num() > 0 ? EBuildValidationStatus::Passed : EBuildValidationStatus::Failed;
    }
    else
    {
        Message = TEXT("Binaries directory not found - compilation may have failed");
        Status = EBuildValidationStatus::Failed;
    }

    FTimespan Duration = FDateTime::Now() - StartTime;
    AddValidationResult(TEXT("Compilation Status"), Status, Message, Duration.GetTotalSeconds());
    CompletedTests++;
}

TArray<FBuildValidationResult> UBuildValidationManager::GetValidationResults() const
{
    return ValidationResults;
}

EBuildValidationStatus UBuildValidationManager::GetOverallStatus() const
{
    return OverallStatus;
}

void UBuildValidationManager::ClearValidationResults()
{
    ValidationResults.Empty();
    OverallStatus = EBuildValidationStatus::Unknown;
    bValidationInProgress = false;
}

bool UBuildValidationManager::IsValidationInProgress() const
{
    return bValidationInProgress;
}

void UBuildValidationManager::AddValidationResult(const FString& TestName, EBuildValidationStatus Status, const FString& Message, float ExecutionTime)
{
    FBuildValidationResult Result;
    Result.TestName = TestName;
    Result.Status = Status;
    Result.Message = Message;
    Result.ExecutionTime = ExecutionTime;
    
    ValidationResults.Add(Result);
    
    UE_LOG(LogTemp, Log, TEXT("Validation Test [%s]: %s - %s (%.3fs)"), 
        *TestName, 
        *UEnum::GetValueAsString(Status), 
        *Message, 
        ExecutionTime);
}

void UBuildValidationManager::UpdateOverallStatus()
{
    if (ValidationResults.Num() == 0)
    {
        OverallStatus = EBuildValidationStatus::Unknown;
        return;
    }

    bool bHasFailed = false;
    bool bHasWarning = false;

    for (const FBuildValidationResult& Result : ValidationResults)
    {
        if (Result.Status == EBuildValidationStatus::Failed)
        {
            bHasFailed = true;
            break;
        }
        else if (Result.Status == EBuildValidationStatus::Warning)
        {
            bHasWarning = true;
        }
    }

    if (bHasFailed)
    {
        OverallStatus = EBuildValidationStatus::Failed;
    }
    else if (bHasWarning)
    {
        OverallStatus = EBuildValidationStatus::Warning;
    }
    else
    {
        OverallStatus = EBuildValidationStatus::Passed;
    }
}