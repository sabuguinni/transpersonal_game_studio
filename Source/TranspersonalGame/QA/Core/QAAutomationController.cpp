#include "QAAutomationController.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "AutomationControllerManager.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"
#include "TimerManager.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

DEFINE_LOG_CATEGORY(LogQAAutomation);

UQAAutomationController::UQAAutomationController()
{
    bIsRunningTests = false;
    CurrentTestIndex = 0;
}

void UQAAutomationController::Initialize()
{
    UE_LOG(LogQAAutomation, Log, TEXT("Initializing QA Automation Controller"));
    
    // Clear any existing state
    TestResults.Empty();
    PendingTests.Empty();
    bIsRunningTests = false;
    CurrentTestIndex = 0;
    
    // Initialize automation controller if not already done
    if (!FAutomationControllerManager::Get().IsInitialized())
    {
        FAutomationControllerManager::Get().Init();
    }
    
    UE_LOG(LogQAAutomation, Log, TEXT("QA Automation Controller initialized successfully"));
}

bool UQAAutomationController::RunTest(const FString& TestName)
{
    if (bIsRunningTests)
    {
        UE_LOG(LogQAAutomation, Warning, TEXT("Cannot start test '%s' - tests already running"), *TestName);
        return false;
    }
    
    if (!DoesTestExist(TestName))
    {
        UE_LOG(LogQAAutomation, Error, TEXT("Test '%s' does not exist"), *TestName);
        return false;
    }
    
    UE_LOG(LogQAAutomation, Log, TEXT("Starting single test: %s"), *TestName);
    
    PendingTests.Empty();
    PendingTests.Add(TestName);
    CurrentTestIndex = 0;
    bIsRunningTests = true;
    SuiteStartTime = FDateTime::Now();
    
    ExecuteNextTest();
    return true;
}

bool UQAAutomationController::RunTestCategory(const FString& Category)
{
    if (bIsRunningTests)
    {
        UE_LOG(LogQAAutomation, Warning, TEXT("Cannot start category '%s' - tests already running"), *Category);
        return false;
    }
    
    TArray<FString> CategoryTests = GetTestsInCategory(Category);
    if (CategoryTests.Num() == 0)
    {
        UE_LOG(LogQAAutomation, Warning, TEXT("No tests found in category '%s'"), *Category);
        return false;
    }
    
    UE_LOG(LogQAAutomation, Log, TEXT("Starting test category '%s' with %d tests"), *Category, CategoryTests.Num());
    
    PendingTests = CategoryTests;
    CurrentTestIndex = 0;
    bIsRunningTests = true;
    SuiteStartTime = FDateTime::Now();
    
    ExecuteNextTest();
    return true;
}

bool UQAAutomationController::RunTestSuite(const FQATestSuiteConfig& Config)
{
    if (bIsRunningTests)
    {
        UE_LOG(LogQAAutomation, Warning, TEXT("Cannot start test suite '%s' - tests already running"), *Config.SuiteName);
        return false;
    }
    
    CurrentSuiteConfig = Config;
    PendingTests.Empty();
    
    // Collect tests from specified categories
    for (const FString& Category : Config.TestCategories)
    {
        TArray<FString> CategoryTests = GetTestsInCategory(Category);
        PendingTests.Append(CategoryTests);
    }
    
    if (PendingTests.Num() == 0)
    {
        UE_LOG(LogQAAutomation, Warning, TEXT("No tests found for test suite '%s'"), *Config.SuiteName);
        return false;
    }
    
    UE_LOG(LogQAAutomation, Log, TEXT("Starting test suite '%s' with %d tests"), *Config.SuiteName, PendingTests.Num());
    
    CurrentTestIndex = 0;
    bIsRunningTests = true;
    SuiteStartTime = FDateTime::Now();
    
    ExecuteNextTest();
    return true;
}

void UQAAutomationController::StopAllTests()
{
    if (!bIsRunningTests)
    {
        UE_LOG(LogQAAutomation, Warning, TEXT("No tests currently running"));
        return;
    }
    
    UE_LOG(LogQAAutomation, Log, TEXT("Stopping all tests"));
    
    bIsRunningTests = false;
    PendingTests.Empty();
    
    // Clear timer
    if (UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull))
    {
        World->GetTimerManager().ClearTimer(TestExecutionTimer);
    }
    
    // Mark remaining tests as skipped
    for (int32 i = CurrentTestIndex; i < PendingTests.Num(); ++i)
    {
        FQATestResult Result = CreateTestResult(PendingTests[i], EQATestStatus::Skipped, TEXT("Test suite stopped"));
        TestResults.Add(Result);
    }
    
    // Broadcast completion
    OnTestSuiteCompleted.Broadcast(TestResults);
}

TArray<FString> UQAAutomationController::GetAvailableTestCategories() const
{
    TArray<FString> Categories;
    
    // Define standard test categories for Transpersonal Game
    Categories.Add(TEXT("Transpersonal.Core"));
    Categories.Add(TEXT("Transpersonal.Physics"));
    Categories.Add(TEXT("Transpersonal.AI"));
    Categories.Add(TEXT("Transpersonal.World"));
    Categories.Add(TEXT("Transpersonal.Performance"));
    Categories.Add(TEXT("Transpersonal.Gameplay"));
    Categories.Add(TEXT("Transpersonal.Survival"));
    Categories.Add(TEXT("Transpersonal.Combat"));
    Categories.Add(TEXT("Transpersonal.Dinosaurs"));
    Categories.Add(TEXT("Transpersonal.VFX"));
    Categories.Add(TEXT("Transpersonal.Audio"));
    
    return Categories;
}

TArray<FString> UQAAutomationController::GetTestsInCategory(const FString& Category) const
{
    TArray<FString> CategoryTests;
    TArray<FString> AllTests = GetAllAvailableTests();
    
    for (const FString& TestName : AllTests)
    {
        if (GetTestCategory(TestName) == Category)
        {
            CategoryTests.Add(TestName);
        }
    }
    
    return CategoryTests;
}

FString UQAAutomationController::GenerateTestReport(bool bIncludeDetails)
{
    FString Report;
    Report += TEXT("=== QA Automation Test Report ===\n");
    Report += FString::Printf(TEXT("Generated: %s\n"), *FDateTime::Now().ToString());
    Report += FString::Printf(TEXT("Suite: %s\n"), *CurrentSuiteConfig.SuiteName);
    Report += TEXT("\n");
    
    // Calculate statistics
    int32 Passed, Failed, Skipped;
    float TotalTime;
    CalculateTestStatistics(Passed, Failed, Skipped, TotalTime);
    
    Report += TEXT("=== Test Summary ===\n");
    Report += FString::Printf(TEXT("Total Tests: %d\n"), TestResults.Num());
    Report += FString::Printf(TEXT("Passed: %d\n"), Passed);
    Report += FString::Printf(TEXT("Failed: %d\n"), Failed);
    Report += FString::Printf(TEXT("Skipped: %d\n"), Skipped);
    Report += FString::Printf(TEXT("Total Execution Time: %.2f seconds\n"), TotalTime);
    Report += FString::Printf(TEXT("Success Rate: %.1f%%\n"), 
                            TestResults.Num() > 0 ? (float)Passed / TestResults.Num() * 100.0f : 0.0f);
    Report += TEXT("\n");
    
    if (bIncludeDetails)
    {
        Report += TEXT("=== Test Details ===\n");
        for (const FQATestResult& Result : TestResults)
        {
            FString StatusText;
            switch (Result.Status)
            {
                case EQATestStatus::Passed: StatusText = TEXT("PASSED"); break;
                case EQATestStatus::Failed: StatusText = TEXT("FAILED"); break;
                case EQATestStatus::Skipped: StatusText = TEXT("SKIPPED"); break;
                case EQATestStatus::Timeout: StatusText = TEXT("TIMEOUT"); break;
                default: StatusText = TEXT("UNKNOWN"); break;
            }
            
            Report += FString::Printf(TEXT("%s [%s] - %.2fs\n"), *Result.TestName, *StatusText, Result.ExecutionTime);
            if (!Result.ErrorMessage.IsEmpty())
            {
                Report += FString::Printf(TEXT("  Error: %s\n"), *Result.ErrorMessage);
            }
        }
    }
    
    return Report;
}

bool UQAAutomationController::ExportTestResults(const FString& FilePath, const FString& Format)
{
    FString Content;
    
    if (Format.ToUpper() == TEXT("JSON"))
    {
        Content = GenerateJSONReport();
    }
    else if (Format.ToUpper() == TEXT("HTML"))
    {
        Content = GenerateHTMLReport();
    }
    else if (Format.ToUpper() == TEXT("CSV"))
    {
        Content = GenerateCSVReport();
    }
    else
    {
        UE_LOG(LogQAAutomation, Error, TEXT("Unsupported export format: %s"), *Format);
        return false;
    }
    
    bool bSuccess = FFileHelper::SaveStringToFile(Content, *FilePath);
    
    UE_LOG(LogQAAutomation, Log, TEXT("Test results export %s: %s"), 
           bSuccess ? TEXT("successful") : TEXT("failed"), *FilePath);
    
    return bSuccess;
}

void UQAAutomationController::ClearTestResults()
{
    TestResults.Empty();
    UE_LOG(LogQAAutomation, Log, TEXT("Test results cleared"));
}

bool UQAAutomationController::RunSmokeTests()
{
    FQATestSuiteConfig SmokeConfig;
    SmokeConfig.SuiteName = TEXT("Smoke Tests");
    SmokeConfig.TestCategories.Add(TEXT("Transpersonal.Core"));
    SmokeConfig.bRunPerformanceTests = false;
    SmokeConfig.bRunStressTests = false;
    SmokeConfig.TestTimeout = 60.0f; // 1 minute timeout for smoke tests
    
    return RunTestSuite(SmokeConfig);
}

bool UQAAutomationController::RunPerformanceTests()
{
    FQATestSuiteConfig PerfConfig;
    PerfConfig.SuiteName = TEXT("Performance Tests");
    PerfConfig.TestCategories.Add(TEXT("Transpersonal.Performance"));
    PerfConfig.bRunPerformanceTests = true;
    PerfConfig.TestTimeout = 600.0f; // 10 minute timeout for performance tests
    
    return RunTestSuite(PerfConfig);
}

bool UQAAutomationController::RunFunctionalTests()
{
    FQATestSuiteConfig FuncConfig;
    FuncConfig.SuiteName = TEXT("Functional Tests");
    FuncConfig.TestCategories.Add(TEXT("Transpersonal.Gameplay"));
    FuncConfig.TestCategories.Add(TEXT("Transpersonal.AI"));
    FuncConfig.TestCategories.Add(TEXT("Transpersonal.Combat"));
    FuncConfig.bRunFunctionalTests = true;
    FuncConfig.TestTimeout = 300.0f; // 5 minute timeout
    
    return RunTestSuite(FuncConfig);
}

bool UQAAutomationController::RunStressTests()
{
    FQATestSuiteConfig StressConfig;
    StressConfig.SuiteName = TEXT("Stress Tests");
    StressConfig.TestCategories.Add(TEXT("Transpersonal.Performance"));
    StressConfig.TestCategories.Add(TEXT("Transpersonal.World"));
    StressConfig.bRunStressTests = true;
    StressConfig.TestTimeout = 1800.0f; // 30 minute timeout for stress tests
    
    return RunTestSuite(StressConfig);
}

bool UQAAutomationController::ValidateBuildIntegrity()
{
    UE_LOG(LogQAAutomation, Log, TEXT("Validating build integrity"));
    
    // Run core system validation
    bool bCoreValid = RunTestCategory(TEXT("Transpersonal.Core"));
    
    // Additional build validation logic would go here
    // - Check asset integrity
    // - Validate module loading
    // - Check configuration files
    
    return bCoreValid;
}

void UQAAutomationController::ExecuteNextTest()
{
    if (CurrentTestIndex >= PendingTests.Num())
    {
        // All tests completed
        bIsRunningTests = false;
        
        UE_LOG(LogQAAutomation, Log, TEXT("Test suite completed. %d tests executed"), TestResults.Num());
        
        // Generate report if requested
        if (CurrentSuiteConfig.bGenerateReport && !CurrentSuiteConfig.ReportOutputPath.IsEmpty())
        {
            ExportTestResults(CurrentSuiteConfig.ReportOutputPath, TEXT("HTML"));
        }
        
        OnTestSuiteCompleted.Broadcast(TestResults);
        return;
    }
    
    FString CurrentTestName = PendingTests[CurrentTestIndex];
    UE_LOG(LogQAAutomation, Log, TEXT("Executing test %d/%d: %s"), 
           CurrentTestIndex + 1, PendingTests.Num(), *CurrentTestName);
    
    // Create test result entry
    FQATestResult Result = CreateTestResult(CurrentTestName, EQATestStatus::Running);
    TestResults.Add(Result);
    
    // Execute the test (simplified - would integrate with actual automation framework)
    bool bTestPassed = true; // Placeholder - would run actual test
    FString ErrorMessage;
    
    // Simulate test execution time
    if (UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull))
    {
        World->GetTimerManager().SetTimer(
            TestExecutionTimer,
            [this, CurrentTestName, bTestPassed, ErrorMessage]()
            {
                OnTestExecutionCompleted(CurrentTestName, bTestPassed, ErrorMessage);
            },
            FMath::RandRange(1.0f, 5.0f), // Random execution time for simulation
            false
        );
    }
}

void UQAAutomationController::OnTestExecutionCompleted(const FString& TestName, bool bSuccess, const FString& ErrorMessage)
{
    // Update test result
    if (TestResults.Num() > CurrentTestIndex)
    {
        TestResults[CurrentTestIndex].Status = bSuccess ? EQATestStatus::Passed : EQATestStatus::Failed;
        TestResults[CurrentTestIndex].EndTime = FDateTime::Now();
        TestResults[CurrentTestIndex].ExecutionTime = (TestResults[CurrentTestIndex].EndTime - TestResults[CurrentTestIndex].StartTime).GetTotalSeconds();
        TestResults[CurrentTestIndex].ErrorMessage = ErrorMessage;
        
        // Broadcast individual test completion
        OnTestCompleted.Broadcast(TestResults[CurrentTestIndex]);
    }
    
    CurrentTestIndex++;
    ExecuteNextTest();
}

TArray<FString> UQAAutomationController::GetAllAvailableTests() const
{
    TArray<FString> AllTests;
    
    // This would integrate with the actual automation framework to get real test names
    // For now, return some example test names
    AllTests.Add(TEXT("Transpersonal.Core.WorldInitialization"));
    AllTests.Add(TEXT("Transpersonal.Core.PhysicsSystem"));
    AllTests.Add(TEXT("Transpersonal.Performance.FrameRate"));
    AllTests.Add(TEXT("Transpersonal.AI.DinosaurBehavior"));
    AllTests.Add(TEXT("Transpersonal.Gameplay.CraftingSystem"));
    
    return AllTests;
}

bool UQAAutomationController::DoesTestExist(const FString& TestName) const
{
    TArray<FString> AllTests = GetAllAvailableTests();
    return AllTests.Contains(TestName);
}

FString UQAAutomationController::GetTestCategory(const FString& TestName) const
{
    // Extract category from test name (assumes format: Category.Subcategory.TestName)
    TArray<FString> Parts;
    TestName.ParseIntoArray(Parts, TEXT("."));
    
    if (Parts.Num() >= 2)
    {
        return Parts[0] + TEXT(".") + Parts[1];
    }
    
    return TEXT("Unknown");
}

FQATestResult UQAAutomationController::CreateTestResult(const FString& TestName, EQATestStatus Status, const FString& ErrorMessage)
{
    FQATestResult Result;
    Result.TestName = TestName;
    Result.Status = Status;
    Result.ErrorMessage = ErrorMessage;
    Result.Category = GetTestCategory(TestName);
    Result.StartTime = FDateTime::Now();
    Result.EndTime = Result.StartTime;
    Result.ExecutionTime = 0.0f;
    
    return Result;
}

FString UQAAutomationController::GenerateHTMLReport() const
{
    FString HTML;
    HTML += TEXT("<!DOCTYPE html><html><head><title>QA Test Report</title>");
    HTML += TEXT("<style>body{font-family:Arial,sans-serif;margin:20px;}");
    HTML += TEXT("table{border-collapse:collapse;width:100%;}");
    HTML += TEXT("th,td{border:1px solid #ddd;padding:8px;text-align:left;}");
    HTML += TEXT("th{background-color:#f2f2f2;}.passed{color:green;}.failed{color:red;}</style>");
    HTML += TEXT("</head><body>");
    HTML += TEXT("<h1>QA Automation Test Report</h1>");
    
    // Summary
    int32 Passed, Failed, Skipped;
    float TotalTime;
    CalculateTestStatistics(Passed, Failed, Skipped, TotalTime);
    
    HTML += TEXT("<h2>Summary</h2>");
    HTML += FString::Printf(TEXT("<p>Total Tests: %d | Passed: %d | Failed: %d | Skipped: %d</p>"), 
                          TestResults.Num(), Passed, Failed, Skipped);
    
    // Test details table
    HTML += TEXT("<h2>Test Results</h2>");
    HTML += TEXT("<table><tr><th>Test Name</th><th>Status</th><th>Time (s)</th><th>Error</th></tr>");
    
    for (const FQATestResult& Result : TestResults)
    {
        FString StatusClass = (Result.Status == EQATestStatus::Passed) ? TEXT("passed") : TEXT("failed");
        FString StatusText = (Result.Status == EQATestStatus::Passed) ? TEXT("PASSED") : TEXT("FAILED");
        
        HTML += FString::Printf(TEXT("<tr><td>%s</td><td class=\"%s\">%s</td><td>%.2f</td><td>%s</td></tr>"),
                              *Result.TestName, *StatusClass, *StatusText, Result.ExecutionTime, *Result.ErrorMessage);
    }
    
    HTML += TEXT("</table></body></html>");
    return HTML;
}

FString UQAAutomationController::GenerateJSONReport() const
{
    TSharedPtr<FJsonObject> RootObject = MakeShareable(new FJsonObject);
    
    // Summary
    int32 Passed, Failed, Skipped;
    float TotalTime;
    CalculateTestStatistics(Passed, Failed, Skipped, TotalTime);
    
    TSharedPtr<FJsonObject> SummaryObject = MakeShareable(new FJsonObject);
    SummaryObject->SetNumberField(TEXT("total"), TestResults.Num());
    SummaryObject->SetNumberField(TEXT("passed"), Passed);
    SummaryObject->SetNumberField(TEXT("failed"), Failed);
    SummaryObject->SetNumberField(TEXT("skipped"), Skipped);
    SummaryObject->SetNumberField(TEXT("totalTime"), TotalTime);
    
    RootObject->SetObjectField(TEXT("summary"), SummaryObject);
    
    // Test results
    TArray<TSharedPtr<FJsonValue>> TestArray;
    for (const FQATestResult& Result : TestResults)
    {
        TSharedPtr<FJsonObject> TestObject = MakeShareable(new FJsonObject);
        TestObject->SetStringField(TEXT("name"), Result.TestName);
        TestObject->SetStringField(TEXT("status"), Result.Status == EQATestStatus::Passed ? TEXT("passed") : TEXT("failed"));
        TestObject->SetNumberField(TEXT("executionTime"), Result.ExecutionTime);
        TestObject->SetStringField(TEXT("error"), Result.ErrorMessage);
        
        TestArray.Add(MakeShareable(new FJsonValueObject(TestObject)));
    }
    
    RootObject->SetArrayField(TEXT("tests"), TestArray);
    
    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);
    
    return OutputString;
}

FString UQAAutomationController::GenerateCSVReport() const
{
    FString CSV;
    CSV += TEXT("TestName,Status,ExecutionTime,Category,Error\n");
    
    for (const FQATestResult& Result : TestResults)
    {
        FString StatusText = (Result.Status == EQATestStatus::Passed) ? TEXT("PASSED") : TEXT("FAILED");
        CSV += FString::Printf(TEXT("%s,%s,%.2f,%s,\"%s\"\n"),
                             *Result.TestName, *StatusText, Result.ExecutionTime, 
                             *Result.Category, *Result.ErrorMessage);
    }
    
    return CSV;
}

void UQAAutomationController::CalculateTestStatistics(int32& OutPassed, int32& OutFailed, int32& OutSkipped, float& OutTotalTime) const
{
    OutPassed = 0;
    OutFailed = 0;
    OutSkipped = 0;
    OutTotalTime = 0.0f;
    
    for (const FQATestResult& Result : TestResults)
    {
        switch (Result.Status)
        {
            case EQATestStatus::Passed:
                OutPassed++;
                break;
            case EQATestStatus::Failed:
            case EQATestStatus::Timeout:
                OutFailed++;
                break;
            case EQATestStatus::Skipped:
                OutSkipped++;
                break;
        }
        
        OutTotalTime += Result.ExecutionTime;
    }
}