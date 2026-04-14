#include "QATestFramework.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/DateTime.h"

// UQATestCase Implementation
UQATestCase::UQATestCase()
{
    TestName = TEXT("Unnamed Test");
    Description = TEXT("No description provided");
    Category = EQA_TestCategory::Unit;
    bIsEnabled = true;
    TimeoutSeconds = 30.0f;
    bTestPassed = false;
    LastErrorMessage = TEXT("");
}

FQA_TestReport UQATestCase::ExecuteTest()
{
    FQA_TestReport Report;
    Report.TestName = TestName;
    Report.Category = Category;
    Report.Timestamp = FDateTime::Now();
    
    if (!bIsEnabled)
    {
        Report.Result = EQA_TestResult::Skipped;
        Report.ErrorMessage = TEXT("Test is disabled");
        return Report;
    }

    double StartTime = FPlatformTime::Seconds();
    
    try
    {
        // Reset test state
        bTestPassed = true;
        LastErrorMessage = TEXT("");
        
        // Setup
        OnTestSetup();
        
        // Run the actual test logic
        bool bResult = RunTestLogic();
        
        // Teardown
        OnTestTeardown();
        
        // Determine result
        if (bResult && bTestPassed)
        {
            Report.Result = EQA_TestResult::Passed;
        }
        else
        {
            Report.Result = EQA_TestResult::Failed;
            Report.ErrorMessage = LastErrorMessage.IsEmpty() ? TEXT("Test logic returned false") : LastErrorMessage;
        }
    }
    catch (const std::exception& e)
    {
        Report.Result = EQA_TestResult::Error;
        Report.ErrorMessage = FString::Printf(TEXT("Exception: %s"), ANSI_TO_TCHAR(e.what()));
    }
    
    double EndTime = FPlatformTime::Seconds();
    Report.ExecutionTime = static_cast<float>(EndTime - StartTime);
    
    return Report;
}

void UQATestCase::AssertTrue(bool bCondition, const FString& Message)
{
    if (!bCondition)
    {
        bTestPassed = false;
        LastErrorMessage = Message.IsEmpty() ? TEXT("AssertTrue failed") : Message;
        UE_LOG(LogTemp, Error, TEXT("QA Test Assert Failed: %s"), *LastErrorMessage);
    }
}

void UQATestCase::AssertFalse(bool bCondition, const FString& Message)
{
    if (bCondition)
    {
        bTestPassed = false;
        LastErrorMessage = Message.IsEmpty() ? TEXT("AssertFalse failed") : Message;
        UE_LOG(LogTemp, Error, TEXT("QA Test Assert Failed: %s"), *LastErrorMessage);
    }
}

void UQATestCase::AssertEqual(const FString& Expected, const FString& Actual, const FString& Message)
{
    if (Expected != Actual)
    {
        bTestPassed = false;
        LastErrorMessage = Message.IsEmpty() ? 
            FString::Printf(TEXT("AssertEqual failed: Expected '%s', got '%s'"), *Expected, *Actual) : 
            Message;
        UE_LOG(LogTemp, Error, TEXT("QA Test Assert Failed: %s"), *LastErrorMessage);
    }
}

void UQATestCase::AssertNotNull(UObject* Object, const FString& Message)
{
    if (!IsValid(Object))
    {
        bTestPassed = false;
        LastErrorMessage = Message.IsEmpty() ? TEXT("AssertNotNull failed - object is null") : Message;
        UE_LOG(LogTemp, Error, TEXT("QA Test Assert Failed: %s"), *LastErrorMessage);
    }
}

// UQATestSuite Implementation
UQATestSuite::UQATestSuite()
{
    SuiteName = TEXT("Unnamed Test Suite");
}

void UQATestSuite::RunAllTests()
{
    LastResults.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("QA: Running test suite '%s' with %d test cases"), *SuiteName, TestCases.Num());
    
    for (TSubclassOf<UQATestCase> TestCaseClass : TestCases)
    {
        if (TestCaseClass)
        {
            ExecuteTestCase(TestCaseClass);
        }
    }
    
    // Broadcast completion
    OnTestSuiteCompleted.Broadcast(SuiteName, LastResults);
    
    // Log summary
    int32 PassedCount = 0;
    int32 FailedCount = 0;
    int32 SkippedCount = 0;
    int32 ErrorCount = 0;
    
    for (const FQA_TestReport& Report : LastResults)
    {
        switch (Report.Result)
        {
            case EQA_TestResult::Passed: PassedCount++; break;
            case EQA_TestResult::Failed: FailedCount++; break;
            case EQA_TestResult::Skipped: SkippedCount++; break;
            case EQA_TestResult::Error: ErrorCount++; break;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("QA Test Suite '%s' Complete: %d Passed, %d Failed, %d Skipped, %d Errors"), 
           *SuiteName, PassedCount, FailedCount, SkippedCount, ErrorCount);
}

void UQATestSuite::RunTestsByCategory(EQA_TestCategory Category)
{
    LastResults.Empty();
    
    for (TSubclassOf<UQATestCase> TestCaseClass : TestCases)
    {
        if (TestCaseClass)
        {
            UQATestCase* TestCDO = TestCaseClass->GetDefaultObject<UQATestCase>();
            if (TestCDO && TestCDO->Category == Category)
            {
                ExecuteTestCase(TestCaseClass);
            }
        }
    }
    
    OnTestSuiteCompleted.Broadcast(SuiteName, LastResults);
}

void UQATestSuite::AddTestCase(TSubclassOf<UQATestCase> TestCaseClass)
{
    if (TestCaseClass && !TestCases.Contains(TestCaseClass))
    {
        TestCases.Add(TestCaseClass);
    }
}

void UQATestSuite::ExecuteTestCase(TSubclassOf<UQATestCase> TestCaseClass)
{
    if (!TestCaseClass)
    {
        return;
    }
    
    UQATestCase* TestInstance = NewObject<UQATestCase>(this, TestCaseClass);
    if (TestInstance)
    {
        FQA_TestReport Report = TestInstance->ExecuteTest();
        LastResults.Add(Report);
        OnTestCompleted.Broadcast(Report);
        
        // Log individual test result
        FString ResultString;
        switch (Report.Result)
        {
            case EQA_TestResult::Passed: ResultString = TEXT("PASSED"); break;
            case EQA_TestResult::Failed: ResultString = TEXT("FAILED"); break;
            case EQA_TestResult::Skipped: ResultString = TEXT("SKIPPED"); break;
            case EQA_TestResult::Error: ResultString = TEXT("ERROR"); break;
            default: ResultString = TEXT("UNKNOWN"); break;
        }
        
        UE_LOG(LogTemp, Log, TEXT("QA Test: %s - %s (%.3fs)"), *Report.TestName, *ResultString, Report.ExecutionTime);
        
        if (Report.Result == EQA_TestResult::Failed || Report.Result == EQA_TestResult::Error)
        {
            UE_LOG(LogTemp, Error, TEXT("QA Test Error: %s"), *Report.ErrorMessage);
        }
    }
}

// UQAFrameworkSubsystem Implementation
void UQAFrameworkSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("QA Framework Subsystem Initialized"));
    
    InitializeDefaultValidationRules();
}

void UQAFrameworkSubsystem::Deinitialize()
{
    RegisteredTestSuites.Empty();
    ValidationRules.Empty();
    AllTestResults.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("QA Framework Subsystem Deinitialized"));
    
    Super::Deinitialize();
}

UQAFrameworkSubsystem* UQAFrameworkSubsystem::Get(const UObject* WorldContext)
{
    if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull))
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            return GameInstance->GetSubsystem<UQAFrameworkSubsystem>();
        }
    }
    return nullptr;
}

void UQAFrameworkSubsystem::RegisterTestSuite(UQATestSuite* TestSuite)
{
    if (TestSuite && !RegisteredTestSuites.Contains(TestSuite))
    {
        RegisteredTestSuites.Add(TestSuite);
        UE_LOG(LogTemp, Log, TEXT("QA: Registered test suite '%s'"), *TestSuite->SuiteName);
    }
}

void UQAFrameworkSubsystem::RunAllTestSuites()
{
    AllTestResults.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("QA: Running all test suites (%d registered)"), RegisteredTestSuites.Num());
    
    for (UQATestSuite* TestSuite : RegisteredTestSuites)
    {
        if (TestSuite)
        {
            TestSuite->RunAllTests();
            AllTestResults.Append(TestSuite->GetLastResults());
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("QA: All test suites completed. Total tests: %d"), AllTestResults.Num());
}

void UQAFrameworkSubsystem::RunTestSuite(const FString& SuiteName)
{
    for (UQATestSuite* TestSuite : RegisteredTestSuites)
    {
        if (TestSuite && TestSuite->SuiteName == SuiteName)
        {
            TestSuite->RunAllTests();
            return;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("QA: Test suite '%s' not found"), *SuiteName);
}

void UQAFrameworkSubsystem::RunSmokeTests()
{
    UE_LOG(LogTemp, Log, TEXT("QA: Running smoke tests"));
    
    for (UQATestSuite* TestSuite : RegisteredTestSuites)
    {
        if (TestSuite)
        {
            TestSuite->RunTestsByCategory(EQA_TestCategory::Smoke);
        }
    }
}

void UQAFrameworkSubsystem::RunRegressionTests()
{
    UE_LOG(LogTemp, Log, TEXT("QA: Running regression tests"));
    
    for (UQATestSuite* TestSuite : RegisteredTestSuites)
    {
        if (TestSuite)
        {
            TestSuite->RunTestsByCategory(EQA_TestCategory::Regression);
        }
    }
}

void UQAFrameworkSubsystem::AddValidationRule(const FQA_ValidationRule& Rule)
{
    ValidationRules.Add(Rule);
    UE_LOG(LogTemp, Log, TEXT("QA: Added validation rule '%s'"), *Rule.RuleName);
}

bool UQAFrameworkSubsystem::ValidateGameState()
{
    UE_LOG(LogTemp, Log, TEXT("QA: Validating game state with %d rules"), ValidationRules.Num());
    
    bool bAllValid = true;
    
    for (const FQA_ValidationRule& Rule : ValidationRules)
    {
        if (Rule.bIsEnabled)
        {
            // This would contain specific validation logic based on the rule
            // For now, we'll just log that we're checking it
            UE_LOG(LogTemp, Log, TEXT("QA: Checking rule '%s'"), *Rule.RuleName);
        }
    }
    
    ValidateModuleIntegrity();
    ValidatePerformanceMetrics();
    ValidateFunctionalSystems();
    
    return bAllValid;
}

void UQAFrameworkSubsystem::GenerateTestReport(const FString& OutputPath)
{
    FString ReportContent;
    ReportContent += TEXT("# QA Test Report\n\n");
    ReportContent += FString::Printf(TEXT("Generated: %s\n\n"), *FDateTime::Now().ToString());
    
    // Summary
    int32 TotalTests = AllTestResults.Num();
    int32 PassedTests = 0;
    int32 FailedTests = 0;
    int32 SkippedTests = 0;
    int32 ErrorTests = 0;
    
    for (const FQA_TestReport& Report : AllTestResults)
    {
        switch (Report.Result)
        {
            case EQA_TestResult::Passed: PassedTests++; break;
            case EQA_TestResult::Failed: FailedTests++; break;
            case EQA_TestResult::Skipped: SkippedTests++; break;
            case EQA_TestResult::Error: ErrorTests++; break;
        }
    }
    
    ReportContent += FString::Printf(TEXT("## Summary\n"));
    ReportContent += FString::Printf(TEXT("- Total Tests: %d\n"), TotalTests);
    ReportContent += FString::Printf(TEXT("- Passed: %d\n"), PassedTests);
    ReportContent += FString::Printf(TEXT("- Failed: %d\n"), FailedTests);
    ReportContent += FString::Printf(TEXT("- Skipped: %d\n"), SkippedTests);
    ReportContent += FString::Printf(TEXT("- Errors: %d\n\n"), ErrorTests);
    
    // Detailed results
    ReportContent += TEXT("## Detailed Results\n\n");
    
    for (const FQA_TestReport& Report : AllTestResults)
    {
        FString ResultString;
        switch (Report.Result)
        {
            case EQA_TestResult::Passed: ResultString = TEXT("✅ PASSED"); break;
            case EQA_TestResult::Failed: ResultString = TEXT("❌ FAILED"); break;
            case EQA_TestResult::Skipped: ResultString = TEXT("⏭️ SKIPPED"); break;
            case EQA_TestResult::Error: ResultString = TEXT("💥 ERROR"); break;
        }
        
        ReportContent += FString::Printf(TEXT("### %s - %s\n"), *Report.TestName, *ResultString);
        ReportContent += FString::Printf(TEXT("- Category: %s\n"), *UEnum::GetValueAsString(Report.Category));
        ReportContent += FString::Printf(TEXT("- Execution Time: %.3fs\n"), Report.ExecutionTime);
        ReportContent += FString::Printf(TEXT("- Timestamp: %s\n"), *Report.Timestamp.ToString());
        
        if (!Report.ErrorMessage.IsEmpty())
        {
            ReportContent += FString::Printf(TEXT("- Error: %s\n"), *Report.ErrorMessage);
        }
        
        ReportContent += TEXT("\n");
    }
    
    // Save to file
    FFileHelper::SaveStringToFile(ReportContent, *OutputPath);
    UE_LOG(LogTemp, Log, TEXT("QA: Test report saved to %s"), *OutputPath);
}

void UQAFrameworkSubsystem::InitializeDefaultValidationRules()
{
    // Module integrity rules
    FQA_ValidationRule ModuleRule;
    ModuleRule.RuleName = TEXT("Module Integrity Check");
    ModuleRule.Description = TEXT("Verify all TranspersonalGame modules are loaded and functional");
    ModuleRule.Category = EQA_TestCategory::Smoke;
    ModuleRule.bIsEnabled = true;
    AddValidationRule(ModuleRule);
    
    // Performance rules
    FQA_ValidationRule PerformanceRule;
    PerformanceRule.RuleName = TEXT("Performance Baseline Check");
    PerformanceRule.Description = TEXT("Ensure game maintains target framerate");
    PerformanceRule.Category = EQA_TestCategory::Performance;
    PerformanceRule.bIsEnabled = true;
    AddValidationRule(PerformanceRule);
    
    // Functional rules
    FQA_ValidationRule FunctionalRule;
    FunctionalRule.RuleName = TEXT("Core Systems Functional Check");
    FunctionalRule.Description = TEXT("Verify core game systems are operational");
    FunctionalRule.Category = EQA_TestCategory::Functional;
    FunctionalRule.bIsEnabled = true;
    AddValidationRule(FunctionalRule);
}

void UQAFrameworkSubsystem::ValidateModuleIntegrity()
{
    UE_LOG(LogTemp, Log, TEXT("QA: Validating module integrity"));
    // Implementation would check if all required modules are loaded
}

void UQAFrameworkSubsystem::ValidatePerformanceMetrics()
{
    UE_LOG(LogTemp, Log, TEXT("QA: Validating performance metrics"));
    // Implementation would check framerate, memory usage, etc.
}

void UQAFrameworkSubsystem::ValidateFunctionalSystems()
{
    UE_LOG(LogTemp, Log, TEXT("QA: Validating functional systems"));
    // Implementation would test core game functionality
}

// AQATestActor Implementation
AQATestActor::AQATestActor()
{
    PrimaryActorTick.bCanEverTick = false;
    
    TestDescription = TEXT("QA Test Actor");
    bAutoRunOnBeginPlay = false;
    TestSuiteInstance = nullptr;
}

void AQATestActor::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoRunOnBeginPlay)
    {
        RunTests();
    }
}

void AQATestActor::RunTests()
{
    if (TestSuiteClass)
    {
        TestSuiteInstance = NewObject<UQATestSuite>(this, TestSuiteClass);
        if (TestSuiteInstance)
        {
            UE_LOG(LogTemp, Log, TEXT("QA Test Actor: Running tests for '%s'"), *TestDescription);
            TestSuiteInstance->RunAllTests();
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("QA Test Actor: No test suite class assigned"));
    }
}

void AQATestActor::RunTestsInEditor()
{
    RunTests();
}