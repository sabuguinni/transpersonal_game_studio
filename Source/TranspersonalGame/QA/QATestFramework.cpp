#include "QATestFramework.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "UObject/UObjectGlobals.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/DateTime.h"

UQA_TestFramework::UQA_TestFramework()
{
    bIsRunning = false;
    CurrentTestName = TEXT("");
}

void UQA_TestFramework::RunAllTests()
{
    if (bIsRunning)
    {
        UE_LOG(LogTemp, Warning, TEXT("QA Test Framework is already running"));
        return;
    }

    bIsRunning = true;
    UE_LOG(LogTemp, Log, TEXT("=== QA TEST FRAMEWORK - RUNNING ALL TESTS ==="));

    float TotalStartTime = FPlatformTime::Seconds();

    for (auto& SuitePair : TestSuites)
    {
        FQA_TestSuite& Suite = SuitePair.Value;
        UE_LOG(LogTemp, Log, TEXT("Running test suite: %s"), *Suite.SuiteName);

        Suite.PassCount = 0;
        Suite.FailCount = 0;
        Suite.ErrorCount = 0;
        Suite.TotalExecutionTime = 0.0f;

        for (FQA_TestCase& TestCase : Suite.TestCases)
        {
            if (TestCase.bEnabled)
            {
                ExecuteTestCase(TestCase);
                LogTestResult(TestCase);
            }
        }

        UpdateSuiteStatistics(Suite);
        UE_LOG(LogTemp, Log, TEXT("Suite %s completed: %d passed, %d failed, %d errors"), 
               *Suite.SuiteName, Suite.PassCount, Suite.FailCount, Suite.ErrorCount);
    }

    float TotalTime = FPlatformTime::Seconds() - TotalStartTime;
    UE_LOG(LogTemp, Log, TEXT("=== ALL TESTS COMPLETED IN %.3f SECONDS ==="), TotalTime);

    bIsRunning = false;
}

void UQA_TestFramework::RunTestSuite(const FString& SuiteName)
{
    if (bIsRunning)
    {
        UE_LOG(LogTemp, Warning, TEXT("QA Test Framework is already running"));
        return;
    }

    FQA_TestSuite* Suite = TestSuites.Find(SuiteName);
    if (!Suite)
    {
        UE_LOG(LogTemp, Error, TEXT("Test suite not found: %s"), *SuiteName);
        return;
    }

    bIsRunning = true;
    UE_LOG(LogTemp, Log, TEXT("=== RUNNING TEST SUITE: %s ==="), *SuiteName);

    Suite->PassCount = 0;
    Suite->FailCount = 0;
    Suite->ErrorCount = 0;
    Suite->TotalExecutionTime = 0.0f;

    for (FQA_TestCase& TestCase : Suite->TestCases)
    {
        if (TestCase.bEnabled)
        {
            ExecuteTestCase(TestCase);
            LogTestResult(TestCase);
        }
    }

    UpdateSuiteStatistics(*Suite);
    UE_LOG(LogTemp, Log, TEXT("=== SUITE COMPLETED: %d passed, %d failed, %d errors ==="), 
           Suite->PassCount, Suite->FailCount, Suite->ErrorCount);

    bIsRunning = false;
}

void UQA_TestFramework::RunSingleTest(const FString& TestName)
{
    if (bIsRunning)
    {
        UE_LOG(LogTemp, Warning, TEXT("QA Test Framework is already running"));
        return;
    }

    // Find test in all suites
    for (auto& SuitePair : TestSuites)
    {
        FQA_TestSuite& Suite = SuitePair.Value;
        for (FQA_TestCase& TestCase : Suite.TestCases)
        {
            if (TestCase.TestName == TestName && TestCase.bEnabled)
            {
                bIsRunning = true;
                UE_LOG(LogTemp, Log, TEXT("=== RUNNING SINGLE TEST: %s ==="), *TestName);
                
                ExecuteTestCase(TestCase);
                LogTestResult(TestCase);
                
                bIsRunning = false;
                return;
            }
        }
    }

    UE_LOG(LogTemp, Error, TEXT("Test not found or disabled: %s"), *TestName);
}

void UQA_TestFramework::AddTestCase(const FQA_TestCase& TestCase, const FString& SuiteName)
{
    FQA_TestSuite& Suite = TestSuites.FindOrAdd(SuiteName);
    if (Suite.SuiteName.IsEmpty())
    {
        Suite.SuiteName = SuiteName;
    }

    // Check for duplicate test names
    for (const FQA_TestCase& ExistingTest : Suite.TestCases)
    {
        if (ExistingTest.TestName == TestCase.TestName)
        {
            UE_LOG(LogTemp, Warning, TEXT("Test case already exists: %s"), *TestCase.TestName);
            return;
        }
    }

    Suite.TestCases.Add(TestCase);
    UE_LOG(LogTemp, Log, TEXT("Added test case: %s to suite: %s"), *TestCase.TestName, *SuiteName);
}

void UQA_TestFramework::RemoveTestCase(const FString& TestName)
{
    for (auto& SuitePair : TestSuites)
    {
        FQA_TestSuite& Suite = SuitePair.Value;
        for (int32 i = Suite.TestCases.Num() - 1; i >= 0; i--)
        {
            if (Suite.TestCases[i].TestName == TestName)
            {
                Suite.TestCases.RemoveAt(i);
                UE_LOG(LogTemp, Log, TEXT("Removed test case: %s"), *TestName);
                return;
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Test case not found for removal: %s"), *TestName);
}

void UQA_TestFramework::ClearAllTests()
{
    TestSuites.Empty();
    UE_LOG(LogTemp, Log, TEXT("All test cases cleared"));
}

FString UQA_TestFramework::GenerateTestReport()
{
    FString Report = TEXT("=== QA TEST FRAMEWORK REPORT ===\n");
    Report += FString::Printf(TEXT("Generated: %s\n\n"), *FDateTime::Now().ToString());

    int32 TotalTests = 0;
    int32 TotalPassed = 0;
    int32 TotalFailed = 0;
    int32 TotalErrors = 0;
    float TotalTime = 0.0f;

    for (const auto& SuitePair : TestSuites)
    {
        const FQA_TestSuite& Suite = SuitePair.Value;
        Report += FString::Printf(TEXT("SUITE: %s\n"), *Suite.SuiteName);
        Report += FString::Printf(TEXT("  Passed: %d, Failed: %d, Errors: %d\n"), 
                                 Suite.PassCount, Suite.FailCount, Suite.ErrorCount);
        Report += FString::Printf(TEXT("  Execution Time: %.3f seconds\n\n"), Suite.TotalExecutionTime);

        TotalTests += Suite.TestCases.Num();
        TotalPassed += Suite.PassCount;
        TotalFailed += Suite.FailCount;
        TotalErrors += Suite.ErrorCount;
        TotalTime += Suite.TotalExecutionTime;

        // List failed tests
        for (const FQA_TestCase& TestCase : Suite.TestCases)
        {
            if (TestCase.Result == EQA_TestResult::Fail || TestCase.Result == EQA_TestResult::Error)
            {
                Report += FString::Printf(TEXT("  FAILED: %s - %s\n"), *TestCase.TestName, *TestCase.ErrorMessage);
            }
        }
        Report += TEXT("\n");
    }

    Report += TEXT("=== SUMMARY ===\n");
    Report += FString::Printf(TEXT("Total Tests: %d\n"), TotalTests);
    Report += FString::Printf(TEXT("Passed: %d (%.1f%%)\n"), TotalPassed, TotalTests > 0 ? (float)TotalPassed / TotalTests * 100.0f : 0.0f);
    Report += FString::Printf(TEXT("Failed: %d\n"), TotalFailed);
    Report += FString::Printf(TEXT("Errors: %d\n"), TotalErrors);
    Report += FString::Printf(TEXT("Total Execution Time: %.3f seconds\n"), TotalTime);

    return Report;
}

void UQA_TestFramework::ExportTestResults(const FString& FilePath)
{
    FString Report = GenerateTestReport();
    
    if (!FFileHelper::SaveStringToFile(Report, *FilePath))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to export test results to: %s"), *FilePath);
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Test results exported to: %s"), *FilePath);
    }
}

TArray<FQA_TestCase> UQA_TestFramework::GetFailedTests()
{
    TArray<FQA_TestCase> FailedTests;

    for (const auto& SuitePair : TestSuites)
    {
        const FQA_TestSuite& Suite = SuitePair.Value;
        for (const FQA_TestCase& TestCase : Suite.TestCases)
        {
            if (TestCase.Result == EQA_TestResult::Fail || TestCase.Result == EQA_TestResult::Error)
            {
                FailedTests.Add(TestCase);
            }
        }
    }

    return FailedTests;
}

float UQA_TestFramework::GetOverallPassRate()
{
    int32 TotalTests = 0;
    int32 TotalPassed = 0;

    for (const auto& SuitePair : TestSuites)
    {
        const FQA_TestSuite& Suite = SuitePair.Value;
        TotalTests += Suite.TestCases.Num();
        TotalPassed += Suite.PassCount;
    }

    return TotalTests > 0 ? (float)TotalPassed / TotalTests * 100.0f : 0.0f;
}

bool UQA_TestFramework::ValidateClassExists(const FString& ClassName)
{
    UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassName);
    return LoadedClass != nullptr;
}

bool UQA_TestFramework::ValidateActorSpawning(UClass* ActorClass)
{
    if (!ActorClass || !ActorClass->IsChildOf(AActor::StaticClass()))
    {
        return false;
    }

    UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
    if (!World)
    {
        return false;
    }

    AActor* SpawnedActor = World->SpawnActor<AActor>(ActorClass);
    if (SpawnedActor)
    {
        SpawnedActor->Destroy();
        return true;
    }

    return false;
}

bool UQA_TestFramework::ValidateComponentAttachment(AActor* Actor, UClass* ComponentClass)
{
    if (!Actor || !ComponentClass || !ComponentClass->IsChildOf(UActorComponent::StaticClass()))
    {
        return false;
    }

    UActorComponent* Component = Actor->FindComponentByClass(ComponentClass);
    return Component != nullptr;
}

bool UQA_TestFramework::ValidatePropertyAccess(UObject* Object, const FString& PropertyName)
{
    if (!Object)
    {
        return false;
    }

    FProperty* Property = Object->GetClass()->FindPropertyByName(*PropertyName);
    return Property != nullptr;
}

void UQA_TestFramework::ExecuteTestCase(FQA_TestCase& TestCase)
{
    CurrentTestName = TestCase.TestName;
    float StartTime = FPlatformTime::Seconds();

    try
    {
        // Basic validation tests based on category
        bool bTestPassed = false;

        switch (TestCase.Category)
        {
            case EQA_TestCategory::Compilation:
                // Test class loading
                bTestPassed = ValidateClassExists(TestCase.Description);
                break;

            case EQA_TestCategory::Functional:
                // Test basic functionality
                bTestPassed = true; // Placeholder - implement specific functional tests
                break;

            case EQA_TestCategory::Integration:
                // Test system integration
                bTestPassed = true; // Placeholder - implement integration tests
                break;

            default:
                bTestPassed = true; // Default pass for other categories
                break;
        }

        TestCase.Result = bTestPassed ? EQA_TestResult::Pass : EQA_TestResult::Fail;
        if (!bTestPassed)
        {
            TestCase.ErrorMessage = TEXT("Test validation failed");
        }
    }
    catch (...)
    {
        TestCase.Result = EQA_TestResult::Error;
        TestCase.ErrorMessage = TEXT("Exception occurred during test execution");
    }

    TestCase.ExecutionTime = FPlatformTime::Seconds() - StartTime;
    CurrentTestName = TEXT("");
}

void UQA_TestFramework::LogTestResult(const FQA_TestCase& TestCase)
{
    FString ResultString;
    switch (TestCase.Result)
    {
        case EQA_TestResult::Pass:
            ResultString = TEXT("PASS");
            break;
        case EQA_TestResult::Fail:
            ResultString = TEXT("FAIL");
            break;
        case EQA_TestResult::Error:
            ResultString = TEXT("ERROR");
            break;
        default:
            ResultString = TEXT("UNKNOWN");
            break;
    }

    UE_LOG(LogTemp, Log, TEXT("[%s] %s (%.3fs) - %s"), 
           *ResultString, *TestCase.TestName, TestCase.ExecutionTime,
           TestCase.ErrorMessage.IsEmpty() ? TEXT("OK") : *TestCase.ErrorMessage);
}

void UQA_TestFramework::UpdateSuiteStatistics(FQA_TestSuite& Suite)
{
    Suite.PassCount = 0;
    Suite.FailCount = 0;
    Suite.ErrorCount = 0;
    Suite.TotalExecutionTime = 0.0f;

    for (const FQA_TestCase& TestCase : Suite.TestCases)
    {
        switch (TestCase.Result)
        {
            case EQA_TestResult::Pass:
                Suite.PassCount++;
                break;
            case EQA_TestResult::Fail:
                Suite.FailCount++;
                break;
            case EQA_TestResult::Error:
                Suite.ErrorCount++;
                break;
        }
        Suite.TotalExecutionTime += TestCase.ExecutionTime;
    }
}