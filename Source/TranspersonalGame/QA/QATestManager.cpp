#include "QATestManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "Components/ActorComponent.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/DateTime.h"
#include "UObject/UObjectGlobals.h"
#include "Engine/ObjectLibrary.h"

UQA_TestManager::UQA_TestManager()
{
    TestTimeout = 30.0f;
    bVerboseLogging = true;
    bAutoSaveReports = true;
}

FQA_ValidationReport UQA_TestManager::RunFullValidation()
{
    UE_LOG(LogTemp, Warning, TEXT("QA Test Manager: Starting full validation suite"));
    
    CurrentReport = FQA_ValidationReport();
    CurrentReport.ReportTimestamp = FDateTime::Now();
    
    // Core class validation tests
    TArray<FString> CoreClasses = {
        TEXT("TranspersonalCharacter"),
        TEXT("TranspersonalGameMode"),
        TEXT("TranspersonalGameState"),
        TEXT("PCGWorldGenerator"),
        TEXT("FoliageManager"),
        TEXT("CrowdSimulationManager"),
        TEXT("ProceduralWorldManager"),
        TEXT("BuildIntegrationManager")
    };
    
    for (const FString& ClassName : CoreClasses)
    {
        FQA_TestCase TestResult = ValidateClassLoading(ClassName);
        CurrentReport.TestCases.Add(TestResult);
        
        switch (TestResult.Result)
        {
            case EQA_TestResult::Passed:
                CurrentReport.PassedTests++;
                break;
            case EQA_TestResult::Failed:
                CurrentReport.FailedTests++;
                break;
            case EQA_TestResult::Warning:
                CurrentReport.WarningTests++;
                break;
        }
        
        CurrentReport.TotalExecutionTime += TestResult.ExecutionTime;
    }
    
    // Map validation
    FQA_TestCase MapTest = ValidateMinPlayableMap();
    CurrentReport.TestCases.Add(MapTest);
    if (MapTest.Result == EQA_TestResult::Passed) CurrentReport.PassedTests++;
    else if (MapTest.Result == EQA_TestResult::Failed) CurrentReport.FailedTests++;
    else CurrentReport.WarningTests++;
    CurrentReport.TotalExecutionTime += MapTest.ExecutionTime;
    
    // Character movement validation
    FQA_TestCase MovementTest = ValidateCharacterMovement();
    CurrentReport.TestCases.Add(MovementTest);
    if (MovementTest.Result == EQA_TestResult::Passed) CurrentReport.PassedTests++;
    else if (MovementTest.Result == EQA_TestResult::Failed) CurrentReport.FailedTests++;
    else CurrentReport.WarningTests++;
    CurrentReport.TotalExecutionTime += MovementTest.ExecutionTime;
    
    // Performance validation
    FQA_TestCase FrameRateTest = ValidateFrameRate();
    CurrentReport.TestCases.Add(FrameRateTest);
    if (FrameRateTest.Result == EQA_TestResult::Passed) CurrentReport.PassedTests++;
    else if (FrameRateTest.Result == EQA_TestResult::Failed) CurrentReport.FailedTests++;
    else CurrentReport.WarningTests++;
    CurrentReport.TotalExecutionTime += FrameRateTest.ExecutionTime;
    
    // Integration tests
    FQA_TestCase IntegrationTest = ValidatePlayerCharacterIntegration();
    CurrentReport.TestCases.Add(IntegrationTest);
    if (IntegrationTest.Result == EQA_TestResult::Passed) CurrentReport.PassedTests++;
    else if (IntegrationTest.Result == EQA_TestResult::Failed) CurrentReport.FailedTests++;
    else CurrentReport.WarningTests++;
    CurrentReport.TotalExecutionTime += IntegrationTest.ExecutionTime;
    
    UE_LOG(LogTemp, Warning, TEXT("QA Validation Complete: %d Passed, %d Failed, %d Warnings"), 
           CurrentReport.PassedTests, CurrentReport.FailedTests, CurrentReport.WarningTests);
    
    if (bAutoSaveReports)
    {
        FString ReportPath = FPaths::ProjectSavedDir() / TEXT("QA") / TEXT("ValidationReport.txt");
        SaveValidationReport(CurrentReport, ReportPath);
    }
    
    return CurrentReport;
}

FQA_TestCase UQA_TestManager::ValidateClassLoading(const FString& ClassName)
{
    return ExecuteTest(FString::Printf(TEXT("Class Loading: %s"), *ClassName), [this, ClassName]() -> bool
    {
        FString FullClassName = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ClassName);
        
        UClass* LoadedClass = LoadClass<UObject>(nullptr, *FullClassName);
        if (!LoadedClass)
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to load class: %s"), *FullClassName);
            return false;
        }
        
        // Try to get class default object
        UObject* CDO = LoadedClass->GetDefaultObject();
        if (!CDO)
        {
            UE_LOG(LogTemp, Warning, TEXT("Class loaded but CDO unavailable: %s"), *FullClassName);
            return false;
        }
        
        UE_LOG(LogTemp, Log, TEXT("Successfully validated class: %s"), *FullClassName);
        return true;
    });
}

FQA_TestCase UQA_TestManager::ValidateActorSpawning(UClass* ActorClass, FVector SpawnLocation)
{
    if (!ActorClass)
    {
        FQA_TestCase FailedTest;
        FailedTest.TestName = TEXT("Actor Spawning");
        FailedTest.Description = TEXT("Validate actor spawning capability");
        FailedTest.Result = EQA_TestResult::Failed;
        FailedTest.ErrorMessage = TEXT("ActorClass is null");
        return FailedTest;
    }
    
    return ExecuteTest(FString::Printf(TEXT("Actor Spawning: %s"), *ActorClass->GetName()), [this, ActorClass, SpawnLocation]() -> bool
    {
        UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull);
        if (!World)
        {
            UE_LOG(LogTemp, Error, TEXT("No valid world context for spawning"));
            return false;
        }
        
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        
        AActor* SpawnedActor = World->SpawnActor<AActor>(ActorClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
        if (!SpawnedActor)
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to spawn actor of class: %s"), *ActorClass->GetName());
            return false;
        }
        
        // Clean up test actor
        SpawnedActor->Destroy();
        UE_LOG(LogTemp, Log, TEXT("Successfully spawned and cleaned up actor: %s"), *ActorClass->GetName());
        return true;
    });
}

FQA_TestCase UQA_TestManager::ValidateMinPlayableMap()
{
    return ExecuteTest(TEXT("MinPlayableMap Validation"), [this]() -> bool
    {
        UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull);
        if (!World)
        {
            UE_LOG(LogTemp, Error, TEXT("No valid world context"));
            return false;
        }
        
        // Count actors in the level
        int32 ActorCount = 0;
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            ActorCount++;
        }
        
        if (ActorCount < 10)
        {
            UE_LOG(LogTemp, Warning, TEXT("MinPlayableMap has fewer actors than expected: %d"), ActorCount);
            return false;
        }
        
        UE_LOG(LogTemp, Log, TEXT("MinPlayableMap validation passed with %d actors"), ActorCount);
        return true;
    });
}

FQA_TestCase UQA_TestManager::ValidateCharacterMovement()
{
    return ExecuteTest(TEXT("Character Movement Validation"), [this]() -> bool
    {
        UClass* CharacterClass = LoadClass<ACharacter>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
        if (!CharacterClass)
        {
            UE_LOG(LogTemp, Error, TEXT("TranspersonalCharacter class not found"));
            return false;
        }
        
        UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull);
        if (!World)
        {
            UE_LOG(LogTemp, Error, TEXT("No valid world context"));
            return false;
        }
        
        // Test character spawning
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        
        ACharacter* TestCharacter = World->SpawnActor<ACharacter>(CharacterClass, FVector(0, 0, 100), FRotator::ZeroRotator, SpawnParams);
        if (!TestCharacter)
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to spawn test character"));
            return false;
        }
        
        // Validate movement component
        if (!TestCharacter->GetCharacterMovement())
        {
            UE_LOG(LogTemp, Error, TEXT("Character has no movement component"));
            TestCharacter->Destroy();
            return false;
        }
        
        // Clean up
        TestCharacter->Destroy();
        UE_LOG(LogTemp, Log, TEXT("Character movement validation passed"));
        return true;
    });
}

FQA_TestCase UQA_TestManager::ValidateFrameRate()
{
    return ExecuteTest(TEXT("Frame Rate Validation"), [this]() -> bool
    {
        // Simple frame rate check - in a real scenario we'd measure over time
        float CurrentFPS = 1.0f / FApp::GetDeltaTime();
        
        if (CurrentFPS < 30.0f)
        {
            UE_LOG(LogTemp, Warning, TEXT("Frame rate below target: %.2f FPS"), CurrentFPS);
            return false;
        }
        
        UE_LOG(LogTemp, Log, TEXT("Frame rate validation passed: %.2f FPS"), CurrentFPS);
        return true;
    });
}

FQA_TestCase UQA_TestManager::ValidatePlayerCharacterIntegration()
{
    return ExecuteTest(TEXT("Player Character Integration"), [this]() -> bool
    {
        UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull);
        if (!World)
        {
            return false;
        }
        
        // Check if we have a valid game mode
        AGameModeBase* GameMode = World->GetAuthGameMode();
        if (!GameMode)
        {
            UE_LOG(LogTemp, Warning, TEXT("No game mode found in world"));
            return false;
        }
        
        UE_LOG(LogTemp, Log, TEXT("Player character integration validation passed"));
        return true;
    });
}

FQA_TestCase UQA_TestManager::ExecuteTest(const FString& TestName, TFunction<bool()> TestFunction)
{
    FQA_TestCase TestCase;
    TestCase.TestName = TestName;
    TestCase.Description = FString::Printf(TEXT("Automated test: %s"), *TestName);
    
    float StartTime = FPlatformTime::Seconds();
    
    try
    {
        bool bTestPassed = TestFunction();
        TestCase.Result = bTestPassed ? EQA_TestResult::Passed : EQA_TestResult::Failed;
        
        if (!bTestPassed)
        {
            TestCase.ErrorMessage = TEXT("Test function returned false");
        }
    }
    catch (...)
    {
        TestCase.Result = EQA_TestResult::Failed;
        TestCase.ErrorMessage = TEXT("Test threw an exception");
        UE_LOG(LogTemp, Error, TEXT("Test %s threw an exception"), *TestName);
    }
    
    TestCase.ExecutionTime = FPlatformTime::Seconds() - StartTime;
    
    if (bVerboseLogging)
    {
        LogTestResult(TestCase);
    }
    
    return TestCase;
}

void UQA_TestManager::LogTestResult(const FQA_TestCase& TestCase)
{
    FString ResultString;
    switch (TestCase.Result)
    {
        case EQA_TestResult::Passed:
            ResultString = TEXT("PASSED");
            break;
        case EQA_TestResult::Failed:
            ResultString = TEXT("FAILED");
            break;
        case EQA_TestResult::Warning:
            ResultString = TEXT("WARNING");
            break;
        default:
            ResultString = TEXT("NOT_RUN");
            break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("QA Test [%s]: %s (%.3fs) - %s"), 
           *ResultString, *TestCase.TestName, TestCase.ExecutionTime, 
           TestCase.ErrorMessage.IsEmpty() ? TEXT("OK") : *TestCase.ErrorMessage);
}

void UQA_TestManager::SaveValidationReport(const FQA_ValidationReport& Report, const FString& FilePath)
{
    FString ReportContent;
    ReportContent += FString::Printf(TEXT("QA Validation Report - %s\n"), *Report.ReportTimestamp.ToString());
    ReportContent += TEXT("=====================================\n\n");
    ReportContent += FString::Printf(TEXT("Summary: %d Passed, %d Failed, %d Warnings\n"), 
                                   Report.PassedTests, Report.FailedTests, Report.WarningTests);
    ReportContent += FString::Printf(TEXT("Total Execution Time: %.3f seconds\n\n"), Report.TotalExecutionTime);
    
    ReportContent += TEXT("Test Details:\n");
    ReportContent += TEXT("-------------\n");
    
    for (const FQA_TestCase& TestCase : Report.TestCases)
    {
        FString ResultString;
        switch (TestCase.Result)
        {
            case EQA_TestResult::Passed: ResultString = TEXT("PASSED"); break;
            case EQA_TestResult::Failed: ResultString = TEXT("FAILED"); break;
            case EQA_TestResult::Warning: ResultString = TEXT("WARNING"); break;
            default: ResultString = TEXT("NOT_RUN"); break;
        }
        
        ReportContent += FString::Printf(TEXT("[%s] %s (%.3fs)\n"), 
                                       *ResultString, *TestCase.TestName, TestCase.ExecutionTime);
        
        if (!TestCase.ErrorMessage.IsEmpty())
        {
            ReportContent += FString::Printf(TEXT("    Error: %s\n"), *TestCase.ErrorMessage);
        }
        ReportContent += TEXT("\n");
    }
    
    // Ensure directory exists
    FString DirectoryPath = FPaths::GetPath(FilePath);
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    if (!PlatformFile.DirectoryExists(*DirectoryPath))
    {
        PlatformFile.CreateDirectoryTree(*DirectoryPath);
    }
    
    if (!FFileHelper::SaveStringToFile(ReportContent, *FilePath))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to save validation report to: %s"), *FilePath);
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Validation report saved to: %s"), *FilePath);
    }
}

// Stub implementations for remaining functions
FQA_TestCase UQA_TestManager::ValidateComponentFunctionality(UActorComponent* Component) { return FQA_TestCase(); }
FQA_TestCase UQA_TestManager::ValidateDinosaurAI() { return FQA_TestCase(); }
FQA_TestCase UQA_TestManager::ValidateWorldGeneration() { return FQA_TestCase(); }
FQA_TestCase UQA_TestManager::ValidateAudioSystems() { return FQA_TestCase(); }
FQA_TestCase UQA_TestManager::ValidateVFXSystems() { return FQA_TestCase(); }
FQA_TestCase UQA_TestManager::ValidateMemoryUsage() { return FQA_TestCase(); }
FQA_TestCase UQA_TestManager::ValidateLoadTimes() { return FQA_TestCase(); }
FQA_TestCase UQA_TestManager::ValidateWorldActorIntegration() { return FQA_TestCase(); }
FQA_TestCase UQA_TestManager::ValidateCrowdSimulationIntegration() { return FQA_TestCase(); }
TArray<FString> UQA_TestManager::GetAllTranspersonalGameClasses() { return TArray<FString>(); }
bool UQA_TestManager::IsClassValid(UClass* Class) { return Class != nullptr; }
bool UQA_TestManager::CanSpawnActor(UClass* ActorClass) { return ActorClass != nullptr; }
float UQA_TestManager::MeasureExecutionTime(TFunction<void()> Function) { return 0.0f; }