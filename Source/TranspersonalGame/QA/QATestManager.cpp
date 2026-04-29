#include "QATestManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/Character.h"
#include "Components/ActorComponent.h"
#include "Engine/Level.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/DateTime.h"

UQATestManager::UQATestManager()
{
    bAutoRunOnStartup = false;
    bVerboseLogging = true;
    MaxReportHistory = 10;
    CurrentTestName = TEXT("");
    TestStartTime = 0.0;
}

void UQATestManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("QA Test Manager initialized"));
    
    if (bAutoRunOnStartup)
    {
        // Delay auto-run to allow other systems to initialize
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UQATestManager::RunAllTests, 2.0f, false);
    }
}

void UQATestManager::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("QA Test Manager deinitialized"));
    Super::Deinitialize();
}

void UQATestManager::RunAllTests()
{
    UE_LOG(LogTemp, Warning, TEXT("=== QA TEST MANAGER - RUNNING ALL TESTS ==="));
    
    ClearTestResults();
    CurrentReport.ReportTimestamp = FDateTime::Now();
    
    double StartTime = FPlatformTime::Seconds();
    
    // Run all test categories
    TestCoreClasses();
    TestCharacterSystems();
    TestWorldGeneration();
    TestAISystems();
    TestCombatSystems();
    TestAudioSystems();
    TestVFXSystems();
    
    CurrentReport.TotalExecutionTime = FPlatformTime::Seconds() - StartTime;
    
    // Count results
    CurrentReport.PassCount = 0;
    CurrentReport.FailCount = 0;
    CurrentReport.WarningCount = 0;
    
    for (const FQA_TestCase& TestCase : CurrentReport.TestCases)
    {
        switch (TestCase.Result)
        {
            case EQA_TestResult::Pass:
                CurrentReport.PassCount++;
                break;
            case EQA_TestResult::Fail:
                CurrentReport.FailCount++;
                break;
            case EQA_TestResult::Warning:
                CurrentReport.WarningCount++;
                break;
        }
    }
    
    // Add to history
    ReportHistory.Add(CurrentReport);
    if (ReportHistory.Num() > MaxReportHistory)
    {
        ReportHistory.RemoveAt(0);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("QA Tests Complete - Pass: %d, Fail: %d, Warning: %d"), 
           CurrentReport.PassCount, CurrentReport.FailCount, CurrentReport.WarningCount);
}

void UQATestManager::RunModuleTests(const FString& ModuleName)
{
    UE_LOG(LogTemp, Log, TEXT("Running tests for module: %s"), *ModuleName);
    
    if (ModuleName == TEXT("Core"))
    {
        TestCoreClasses();
    }
    else if (ModuleName == TEXT("Characters"))
    {
        TestCharacterSystems();
    }
    else if (ModuleName == TEXT("WorldGeneration"))
    {
        TestWorldGeneration();
    }
    else if (ModuleName == TEXT("AI"))
    {
        TestAISystems();
    }
    else if (ModuleName == TEXT("Combat"))
    {
        TestCombatSystems();
    }
    else if (ModuleName == TEXT("Audio"))
    {
        TestAudioSystems();
    }
    else if (ModuleName == TEXT("VFX"))
    {
        TestVFXSystems();
    }
    else
    {
        LogTestResult(FString::Printf(TEXT("Module_%s"), *ModuleName), false, TEXT("Unknown module"));
    }
}

FQA_ValidationReport UQATestManager::GetLastReport() const
{
    return CurrentReport;
}

void UQATestManager::ClearTestResults()
{
    CurrentReport = FQA_ValidationReport();
}

void UQATestManager::TestCoreClasses()
{
    StartTest(TEXT("Core_GameMode"));
    UWorld* World = GetWorld();
    if (World && World->GetAuthGameMode())
    {
        FinishCurrentTest(true);
    }
    else
    {
        FinishCurrentTest(false, TEXT("No GameMode found"));
    }
    
    StartTest(TEXT("Core_GameState"));
    if (World && World->GetGameState())
    {
        FinishCurrentTest(true);
    }
    else
    {
        FinishCurrentTest(false, TEXT("No GameState found"));
    }
    
    StartTest(TEXT("Core_PlayerController"));
    if (World && World->GetFirstPlayerController())
    {
        FinishCurrentTest(true);
    }
    else
    {
        FinishCurrentTest(false, TEXT("No PlayerController found"));
    }
}

void UQATestManager::TestCharacterSystems()
{
    StartTest(TEXT("Character_PlayerPawn"));
    UWorld* World = GetWorld();
    if (World)
    {
        APlayerController* PC = World->GetFirstPlayerController();
        if (PC && PC->GetPawn())
        {
            FinishCurrentTest(true);
        }
        else
        {
            FinishCurrentTest(false, TEXT("No player pawn found"));
        }
    }
    else
    {
        FinishCurrentTest(false, TEXT("No world context"));
    }
    
    StartTest(TEXT("Character_MovementComponent"));
    // Test character movement component exists
    FinishCurrentTest(true, TEXT("Movement component test placeholder"));
}

void UQATestManager::TestWorldGeneration()
{
    StartTest(TEXT("World_LevelLoaded"));
    UWorld* World = GetWorld();
    if (World && World->GetCurrentLevel())
    {
        FinishCurrentTest(true);
    }
    else
    {
        FinishCurrentTest(false, TEXT("No level loaded"));
    }
    
    StartTest(TEXT("World_ActorCount"));
    if (World)
    {
        int32 ActorCount = World->GetCurrentLevel()->Actors.Num();
        if (ActorCount > 0)
        {
            FinishCurrentTest(true, FString::Printf(TEXT("Found %d actors"), ActorCount));
        }
        else
        {
            FinishCurrentTest(false, TEXT("No actors in level"));
        }
    }
    else
    {
        FinishCurrentTest(false, TEXT("No world context"));
    }
}

void UQATestManager::TestAISystems()
{
    StartTest(TEXT("AI_SystemsPlaceholder"));
    FinishCurrentTest(true, TEXT("AI systems test placeholder"));
}

void UQATestManager::TestCombatSystems()
{
    StartTest(TEXT("Combat_SystemsPlaceholder"));
    FinishCurrentTest(true, TEXT("Combat systems test placeholder"));
}

void UQATestManager::TestAudioSystems()
{
    StartTest(TEXT("Audio_SystemsPlaceholder"));
    FinishCurrentTest(true, TEXT("Audio systems test placeholder"));
}

void UQATestManager::TestVFXSystems()
{
    StartTest(TEXT("VFX_SystemsPlaceholder"));
    FinishCurrentTest(true, TEXT("VFX systems test placeholder"));
}

bool UQATestManager::ValidateActorInLevel(TSubclassOf<AActor> ActorClass, const FString& TestName)
{
    UWorld* World = GetWorld();
    if (!World || !ActorClass)
    {
        LogTestResult(TestName, false, TEXT("Invalid world or actor class"));
        return false;
    }
    
    for (TActorIterator<AActor> ActorItr(World, ActorClass); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && IsValid(Actor))
        {
            LogTestResult(TestName, true, FString::Printf(TEXT("Found actor: %s"), *Actor->GetName()));
            return true;
        }
    }
    
    LogTestResult(TestName, false, TEXT("No valid actors of specified class found"));
    return false;
}

bool UQATestManager::ValidateComponentOnActor(AActor* Actor, TSubclassOf<UActorComponent> ComponentClass, const FString& TestName)
{
    if (!Actor || !ComponentClass)
    {
        LogTestResult(TestName, false, TEXT("Invalid actor or component class"));
        return false;
    }
    
    UActorComponent* Component = Actor->GetComponentByClass(ComponentClass);
    if (Component && IsValid(Component))
    {
        LogTestResult(TestName, true, FString::Printf(TEXT("Found component: %s"), *Component->GetName()));
        return true;
    }
    
    LogTestResult(TestName, false, TEXT("Component not found on actor"));
    return false;
}

void UQATestManager::LogTestResult(const FString& TestName, bool bPassed, const FString& ErrorMessage)
{
    EQA_TestResult Result = bPassed ? EQA_TestResult::Pass : EQA_TestResult::Fail;
    AddTestCase(TestName, TEXT("Manual test result"), Result, ErrorMessage, 0.0f);
    
    if (bVerboseLogging)
    {
        if (bPassed)
        {
            UE_LOG(LogTemp, Log, TEXT("QA TEST PASS: %s"), *TestName);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("QA TEST FAIL: %s - %s"), *TestName, *ErrorMessage);
        }
    }
}

void UQATestManager::AddTestCase(const FString& TestName, const FString& Description, EQA_TestResult Result, const FString& ErrorMessage, float ExecutionTime)
{
    FQA_TestCase TestCase;
    TestCase.TestName = TestName;
    TestCase.Description = Description;
    TestCase.Result = Result;
    TestCase.ErrorMessage = ErrorMessage;
    TestCase.ExecutionTime = ExecutionTime;
    
    CurrentReport.TestCases.Add(TestCase);
}

void UQATestManager::StartTest(const FString& TestName)
{
    CurrentTestName = TestName;
    TestStartTime = FPlatformTime::Seconds();
    
    if (bVerboseLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("QA TEST START: %s"), *TestName);
    }
}

void UQATestManager::FinishCurrentTest(bool bPassed, const FString& ErrorMessage)
{
    if (CurrentTestName.IsEmpty())
    {
        return;
    }
    
    float ExecutionTime = FPlatformTime::Seconds() - TestStartTime;
    EQA_TestResult Result = bPassed ? EQA_TestResult::Pass : EQA_TestResult::Fail;
    
    AddTestCase(CurrentTestName, TEXT("Automated test"), Result, ErrorMessage, ExecutionTime);
    
    if (bVerboseLogging)
    {
        if (bPassed)
        {
            UE_LOG(LogTemp, Log, TEXT("QA TEST PASS: %s (%.3fs)"), *CurrentTestName, ExecutionTime);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("QA TEST FAIL: %s - %s (%.3fs)"), *CurrentTestName, *ErrorMessage, ExecutionTime);
        }
    }
    
    CurrentTestName = TEXT("");
    TestStartTime = 0.0;
}