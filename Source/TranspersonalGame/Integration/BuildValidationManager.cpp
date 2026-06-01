#include "BuildValidationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EditorLevelLibrary.h"
#include "EditorAssetLibrary.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"
#include "Misc/DateTime.h"

void UBuildValidationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    MaxActorCount = 8000;
    MaxDinosaurCount = 150;
    MaxPropsPerBiome = 1000;
    
    UE_LOG(LogTemp, Log, TEXT("BuildValidationManager initialized with limits: Actors=%d, Dinos=%d, Props/Biome=%d"), 
           MaxActorCount, MaxDinosaurCount, MaxPropsPerBiome);
}

void UBuildValidationManager::Deinitialize()
{
    ValidationReports.Empty();
    Super::Deinitialize();
}

void UBuildValidationManager::RunFullValidationSuite()
{
    ValidationStartTime = FDateTime::Now();
    ClearValidationReports();
    
    UE_LOG(LogTemp, Log, TEXT("Starting full validation suite..."));
    
    ValidateActorCounts();
    ValidateModuleDependencies();
    ValidateQAResults();
    ValidateCompilationStatus();
    
    FTimespan Duration = FDateTime::Now() - ValidationStartTime;
    UE_LOG(LogTemp, Log, TEXT("Validation suite completed in %.2f seconds"), Duration.GetTotalSeconds());
    
    bool bAllPassed = IsValidationPassing();
    AddValidationReport(
        bAllPassed ? EBuild_ValidationResult::Pass : EBuild_ValidationResult::Fail,
        TEXT("Full Validation Suite"),
        FString::Printf(TEXT("Completed %d tests in %.2f seconds"), ValidationReports.Num(), Duration.GetTotalSeconds()),
        Duration.GetTotalSeconds()
    );
}

void UBuildValidationManager::ValidateActorCounts()
{
    FDateTime StartTime = FDateTime::Now();
    
    if (UWorld* World = GEngine->GetCurrentPlayWorld())
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        
        int32 ActorCount = AllActors.Num();
        int32 DinosaurCount = 0;
        
        // Count dinosaurs
        for (AActor* Actor : AllActors)
        {
            if (Actor && Actor->GetName().Contains(TEXT("Dino"), ESearchCase::IgnoreCase))
            {
                DinosaurCount++;
            }
        }
        
        FTimespan Duration = FDateTime::Now() - StartTime;
        
        // Validate actor limits
        if (ActorCount > MaxActorCount)
        {
            AddValidationReport(
                EBuild_ValidationResult::Critical,
                TEXT("Actor Count Validation"),
                FString::Printf(TEXT("Actor count %d exceeds limit %d"), ActorCount, MaxActorCount),
                Duration.GetTotalSeconds()
            );
        }
        else if (ActorCount > MaxActorCount * 0.8f)
        {
            AddValidationReport(
                EBuild_ValidationResult::Warning,
                TEXT("Actor Count Validation"),
                FString::Printf(TEXT("Actor count %d approaching limit %d"), ActorCount, MaxActorCount),
                Duration.GetTotalSeconds()
            );
        }
        else
        {
            AddValidationReport(
                EBuild_ValidationResult::Pass,
                TEXT("Actor Count Validation"),
                FString::Printf(TEXT("Actor count %d within limits"), ActorCount),
                Duration.GetTotalSeconds()
            );
        }
        
        // Validate dinosaur limits
        if (DinosaurCount > MaxDinosaurCount)
        {
            AddValidationReport(
                EBuild_ValidationResult::Critical,
                TEXT("Dinosaur Count Validation"),
                FString::Printf(TEXT("Dinosaur count %d exceeds limit %d"), DinosaurCount, MaxDinosaurCount),
                Duration.GetTotalSeconds()
            );
        }
        else
        {
            AddValidationReport(
                EBuild_ValidationResult::Pass,
                TEXT("Dinosaur Count Validation"),
                FString::Printf(TEXT("Dinosaur count %d within limits"), DinosaurCount),
                Duration.GetTotalSeconds()
            );
        }
    }
    else
    {
        AddValidationReport(
            EBuild_ValidationResult::Fail,
            TEXT("Actor Count Validation"),
            TEXT("No valid world found for actor counting"),
            0.0f
        );
    }
}

void UBuildValidationManager::ValidateModuleDependencies()
{
    FDateTime StartTime = FDateTime::Now();
    
    // Check if core modules are loaded
    TArray<FString> RequiredModules = {
        TEXT("TranspersonalGame"),
        TEXT("Core"),
        TEXT("CoreUObject"),
        TEXT("Engine")
    };
    
    int32 LoadedModules = 0;
    for (const FString& ModuleName : RequiredModules)
    {
        if (FModuleManager::Get().IsModuleLoaded(*ModuleName))
        {
            LoadedModules++;
        }
    }
    
    FTimespan Duration = FDateTime::Now() - StartTime;
    
    if (LoadedModules == RequiredModules.Num())
    {
        AddValidationReport(
            EBuild_ValidationResult::Pass,
            TEXT("Module Dependencies"),
            FString::Printf(TEXT("All %d required modules loaded"), RequiredModules.Num()),
            Duration.GetTotalSeconds()
        );
    }
    else
    {
        AddValidationReport(
            EBuild_ValidationResult::Fail,
            TEXT("Module Dependencies"),
            FString::Printf(TEXT("Only %d of %d required modules loaded"), LoadedModules, RequiredModules.Num()),
            Duration.GetTotalSeconds()
        );
    }
}

void UBuildValidationManager::ValidateQAResults()
{
    FDateTime StartTime = FDateTime::Now();
    
    FString QAResultsPath = FPaths::ProjectDir() / TEXT("Source/TranspersonalGame/QA/asset_requests.json");
    
    if (FPlatformFileManager::Get().GetPlatformFile().FileExists(*QAResultsPath))
    {
        FString FileContent;
        if (FFileHelper::LoadFileToString(FileContent, *QAResultsPath))
        {
            // Basic validation - check if file is not empty and contains valid JSON structure
            if (FileContent.Len() > 10 && FileContent.Contains(TEXT("{")))
            {
                AddValidationReport(
                    EBuild_ValidationResult::Pass,
                    TEXT("QA Results Validation"),
                    TEXT("QA results file found and appears valid"),
                    (FDateTime::Now() - StartTime).GetTotalSeconds()
                );
            }
            else
            {
                AddValidationReport(
                    EBuild_ValidationResult::Warning,
                    TEXT("QA Results Validation"),
                    TEXT("QA results file found but appears empty or invalid"),
                    (FDateTime::Now() - StartTime).GetTotalSeconds()
                );
            }
        }
        else
        {
            AddValidationReport(
                EBuild_ValidationResult::Fail,
                TEXT("QA Results Validation"),
                TEXT("QA results file exists but cannot be read"),
                (FDateTime::Now() - StartTime).GetTotalSeconds()
            );
        }
    }
    else
    {
        AddValidationReport(
            EBuild_ValidationResult::Warning,
            TEXT("QA Results Validation"),
            TEXT("No QA results file found - may be first run"),
            (FDateTime::Now() - StartTime).GetTotalSeconds()
        );
    }
}

void UBuildValidationManager::ValidateCompilationStatus()
{
    FDateTime StartTime = FDateTime::Now();
    
    FString BinariesPath = FPaths::ProjectDir() / TEXT("Binaries");
    
    if (FPlatformFileManager::Get().GetPlatformFile().DirectoryExists(*BinariesPath))
    {
        TArray<FString> BinaryFiles;
        FPlatformFileManager::Get().GetPlatformFile().FindFilesRecursively(BinaryFiles, *BinariesPath, TEXT(".so"));
        
        if (BinaryFiles.Num() > 0)
        {
            AddValidationReport(
                EBuild_ValidationResult::Pass,
                TEXT("Compilation Status"),
                FString::Printf(TEXT("Found %d compiled binary files"), BinaryFiles.Num()),
                (FDateTime::Now() - StartTime).GetTotalSeconds()
            );
        }
        else
        {
            AddValidationReport(
                EBuild_ValidationResult::Fail,
                TEXT("Compilation Status"),
                TEXT("No compiled binary files found"),
                (FDateTime::Now() - StartTime).GetTotalSeconds()
            );
        }
    }
    else
    {
        AddValidationReport(
            EBuild_ValidationResult::Fail,
            TEXT("Compilation Status"),
            TEXT("Binaries directory not found"),
            (FDateTime::Now() - StartTime).GetTotalSeconds()
        );
    }
}

bool UBuildValidationManager::IsValidationPassing() const
{
    for (const FBuild_ValidationReport& Report : ValidationReports)
    {
        if (Report.Result == EBuild_ValidationResult::Critical || Report.Result == EBuild_ValidationResult::Fail)
        {
            return false;
        }
    }
    return true;
}

void UBuildValidationManager::AddValidationReport(EBuild_ValidationResult Result, const FString& TestName, const FString& Details, float ExecutionTime)
{
    FBuild_ValidationReport Report;
    Report.Result = Result;
    Report.TestName = TestName;
    Report.Details = Details;
    Report.ExecutionTime = ExecutionTime;
    
    ValidationReports.Add(Report);
    
    FString ResultString;
    switch (Result)
    {
        case EBuild_ValidationResult::Pass: ResultString = TEXT("PASS"); break;
        case EBuild_ValidationResult::Warning: ResultString = TEXT("WARN"); break;
        case EBuild_ValidationResult::Fail: ResultString = TEXT("FAIL"); break;
        case EBuild_ValidationResult::Critical: ResultString = TEXT("CRIT"); break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("[%s] %s: %s (%.3fs)"), *ResultString, *TestName, *Details, ExecutionTime);
}