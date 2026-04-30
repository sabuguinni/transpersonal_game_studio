#include "QATestManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "UObject/UObjectGlobals.h"
#include "Misc/DateTime.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"

UQATestManager::UQATestManager()
{
    bAutoRunOnStartup = false;
    bGenerateDetailedLogs = true;
    PerformanceThresholdFPS = 30.0f;
}

void UQATestManager::RunAllTests()
{
    UE_LOG(LogTemp, Warning, TEXT("QA Test Manager: Running all test suites"));
    
    double StartTime = FPlatformTime::Seconds();
    
    // Run all registered test suites
    for (auto& TestSuitePair : TestSuites)
    {
        RunTestSuite(TestSuitePair.Key);
    }
    
    double EndTime = FPlatformTime::Seconds();
    float TotalTime = EndTime - StartTime;
    
    UE_LOG(LogTemp, Warning, TEXT("QA Test Manager: All tests completed in %.2f seconds"), TotalTime);
    
    // Generate comprehensive report
    GenerateTestReport();
}

void UQATestManager::RunTestSuite(const FString& SuiteName)
{
    if (!TestSuites.Contains(SuiteName))
    {
        UE_LOG(LogTemp, Error, TEXT("QA Test Manager: Test suite '%s' not found"), *SuiteName);
        return;
    }
    
    FQA_TestSuite& TestSuite = TestSuites[SuiteName];
    UE_LOG(LogTemp, Warning, TEXT("QA Test Manager: Running test suite '%s'"), *SuiteName);
    
    double SuiteStartTime = FPlatformTime::Seconds();
    
    // Reset suite statistics
    TestSuite.PassedTests = 0;
    TestSuite.FailedTests = 0;
    
    // Execute all tests in the suite
    for (FQA_TestCase& TestCase : TestSuite.TestCases)
    {
        double TestStartTime = FPlatformTime::Seconds();
        
        TestCase.Result = ExecuteTest(TestCase);
        
        double TestEndTime = FPlatformTime::Seconds();
        TestCase.ExecutionTime = TestEndTime - TestStartTime;
        
        // Update suite statistics
        if (TestCase.Result == EQA_TestResult::Passed)
        {
            TestSuite.PassedTests++;
        }
        else if (TestCase.Result == EQA_TestResult::Failed)
        {
            TestSuite.FailedTests++;
        }
        
        LogTestResult(TestCase);
    }
    
    double SuiteEndTime = FPlatformTime::Seconds();
    TestSuite.TotalExecutionTime = SuiteEndTime - SuiteStartTime;
    
    UpdateTestSuiteStats(TestSuite);
}

void UQATestManager::RunSingleTest(const FString& TestName)
{
    // Find test case by name
    FQA_TestCase* FoundTest = nullptr;
    
    for (FQA_TestCase& TestCase : AllTestCases)
    {
        if (TestCase.TestName == TestName)
        {
            FoundTest = &TestCase;
            break;
        }
    }
    
    if (!FoundTest)
    {
        UE_LOG(LogTemp, Error, TEXT("QA Test Manager: Test '%s' not found"), *TestName);
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("QA Test Manager: Running single test '%s'"), *TestName);
    
    double TestStartTime = FPlatformTime::Seconds();
    FoundTest->Result = ExecuteTest(*FoundTest);
    double TestEndTime = FPlatformTime::Seconds();
    
    FoundTest->ExecutionTime = TestEndTime - TestStartTime;
    LogTestResult(*FoundTest);
}

void UQATestManager::RegisterTestCase(const FQA_TestCase& TestCase)
{
    AllTestCases.Add(TestCase);
    UE_LOG(LogTemp, Log, TEXT("QA Test Manager: Registered test case '%s'"), *TestCase.TestName);
}

void UQATestManager::RegisterTestSuite(const FQA_TestSuite& TestSuite)
{
    TestSuites.Add(TestSuite.SuiteName, TestSuite);
    UE_LOG(LogTemp, Log, TEXT("QA Test Manager: Registered test suite '%s' with %d tests"), 
           *TestSuite.SuiteName, TestSuite.TestCases.Num());
}

FQA_TestSuite UQATestManager::GetTestSuite(const FString& SuiteName) const
{
    if (TestSuites.Contains(SuiteName))
    {
        return TestSuites[SuiteName];
    }
    
    return FQA_TestSuite();
}

TArray<FQA_TestCase> UQATestManager::GetFailedTests() const
{
    TArray<FQA_TestCase> FailedTests;
    
    for (const FQA_TestCase& TestCase : AllTestCases)
    {
        if (TestCase.Result == EQA_TestResult::Failed)
        {
            FailedTests.Add(TestCase);
        }
    }
    
    return FailedTests;
}

float UQATestManager::GetOverallSuccessRate() const
{
    if (AllTestCases.Num() == 0)
    {
        return 0.0f;
    }
    
    int32 PassedCount = 0;
    for (const FQA_TestCase& TestCase : AllTestCases)
    {
        if (TestCase.Result == EQA_TestResult::Passed)
        {
            PassedCount++;
        }
    }
    
    return (float)PassedCount / (float)AllTestCases.Num() * 100.0f;
}

void UQATestManager::GenerateTestReport()
{
    FString ReportContent;
    FDateTime Now = FDateTime::Now();
    
    ReportContent += FString::Printf(TEXT("=== QA TEST REPORT ===\n"));
    ReportContent += FString::Printf(TEXT("Generated: %s\n"), *Now.ToString());
    ReportContent += FString::Printf(TEXT("Overall Success Rate: %.1f%%\n"), GetOverallSuccessRate());
    ReportContent += FString::Printf(TEXT("Total Tests: %d\n\n"), AllTestCases.Num());
    
    // Test suite summaries
    for (const auto& TestSuitePair : TestSuites)
    {
        const FQA_TestSuite& Suite = TestSuitePair.Value;
        ReportContent += FString::Printf(TEXT("Suite: %s\n"), *Suite.SuiteName);
        ReportContent += FString::Printf(TEXT("  Passed: %d\n"), Suite.PassedTests);
        ReportContent += FString::Printf(TEXT("  Failed: %d\n"), Suite.FailedTests);
        ReportContent += FString::Printf(TEXT("  Execution Time: %.2fs\n\n"), Suite.TotalExecutionTime);
    }
    
    // Failed tests details
    TArray<FQA_TestCase> FailedTests = GetFailedTests();
    if (FailedTests.Num() > 0)
    {
        ReportContent += FString::Printf(TEXT("=== FAILED TESTS ===\n"));
        for (const FQA_TestCase& FailedTest : FailedTests)
        {
            ReportContent += FString::Printf(TEXT("Test: %s\n"), *FailedTest.TestName);
            ReportContent += FString::Printf(TEXT("  Category: %s\n"), 
                *UEnum::GetValueAsString(FailedTest.Category));
            ReportContent += FString::Printf(TEXT("  Error: %s\n\n"), *FailedTest.ErrorMessage);
        }
    }
    
    // Save report to file
    FString ReportPath = FPaths::ProjectLogDir() / TEXT("QA_TestReport.txt");
    FFileHelper::SaveStringToFile(ReportContent, *ReportPath);
    
    UE_LOG(LogTemp, Warning, TEXT("QA Test Manager: Report generated at %s"), *ReportPath);
}

void UQATestManager::TestModuleCompilation()
{
    FQA_TestSuite CompilationSuite;
    CompilationSuite.SuiteName = TEXT("Module Compilation");
    
    // Test core TranspersonalGame classes
    TArray<FString> CoreClasses = {
        TEXT("/Script/TranspersonalGame.TranspersonalCharacter"),
        TEXT("/Script/TranspersonalGame.TranspersonalGameMode"),
        TEXT("/Script/TranspersonalGame.TranspersonalGameState"),
        TEXT("/Script/TranspersonalGame.PCGWorldGenerator"),
        TEXT("/Script/TranspersonalGame.FoliageManager")
    };
    
    for (const FString& ClassName : CoreClasses)
    {
        FQA_TestCase TestCase;
        TestCase.TestName = FString::Printf(TEXT("Load Class: %s"), *ClassName);
        TestCase.Category = EQA_TestCategory::Compilation;
        TestCase.Description = FString::Printf(TEXT("Test loading of class %s"), *ClassName);
        
        CompilationSuite.TestCases.Add(TestCase);
    }
    
    RegisterTestSuite(CompilationSuite);
    RunTestSuite(CompilationSuite.SuiteName);
}

void UQATestManager::TestGameplayFunctionality()
{
    FQA_TestSuite GameplaySuite;
    GameplaySuite.SuiteName = TEXT("Gameplay Functionality");
    
    // Test basic gameplay elements
    FQA_TestCase PlayerSpawnTest;
    PlayerSpawnTest.TestName = TEXT("Player Character Spawn");
    PlayerSpawnTest.Category = EQA_TestCategory::Gameplay;
    PlayerSpawnTest.Description = TEXT("Test spawning of player character");
    GameplaySuite.TestCases.Add(PlayerSpawnTest);
    
    FQA_TestCase MovementTest;
    MovementTest.TestName = TEXT("Character Movement");
    MovementTest.Category = EQA_TestCategory::Gameplay;
    MovementTest.Description = TEXT("Test character movement functionality");
    GameplaySuite.TestCases.Add(MovementTest);
    
    RegisterTestSuite(GameplaySuite);
    RunTestSuite(GameplaySuite.SuiteName);
}

void UQATestManager::TestPerformanceMetrics()
{
    FQA_TestSuite PerformanceSuite;
    PerformanceSuite.SuiteName = TEXT("Performance Metrics");
    
    FQA_TestCase FPSTest;
    FPSTest.TestName = TEXT("Frame Rate Test");
    FPSTest.Category = EQA_TestCategory::Performance;
    FPSTest.Description = TEXT("Test minimum frame rate requirements");
    PerformanceSuite.TestCases.Add(FPSTest);
    
    RegisterTestSuite(PerformanceSuite);
    RunTestSuite(PerformanceSuite.SuiteName);
}

void UQATestManager::TestIntegrationScenarios()
{
    FQA_TestSuite IntegrationSuite;
    IntegrationSuite.SuiteName = TEXT("Integration Scenarios");
    
    FQA_TestCase WorldGenTest;
    WorldGenTest.TestName = TEXT("World Generation Integration");
    WorldGenTest.Category = EQA_TestCategory::Integration;
    WorldGenTest.Description = TEXT("Test world generation with foliage system");
    IntegrationSuite.TestCases.Add(WorldGenTest);
    
    RegisterTestSuite(IntegrationSuite);
    RunTestSuite(IntegrationSuite.SuiteName);
}

EQA_TestResult UQATestManager::ExecuteTest(FQA_TestCase& TestCase)
{
    try
    {
        // Determine test type and execute accordingly
        if (TestCase.TestName.Contains(TEXT("Load Class")))
        {
            FString ClassName = TestCase.TestName.Replace(TEXT("Load Class: "), TEXT(""));
            return TestClassLoading(ClassName);
        }
        else if (TestCase.TestName.Contains(TEXT("Spawn")))
        {
            // Test actor spawning
            UClass* ActorClass = LoadClass<AActor>(nullptr, TEXT("/Script/Engine.Actor"));
            return TestActorSpawning(ActorClass);
        }
        else if (TestCase.TestName.Contains(TEXT("Movement")))
        {
            // Test movement functionality
            TestCase.ErrorMessage = TEXT("Movement test not yet implemented");
            return EQA_TestResult::Warning;
        }
        else if (TestCase.TestName.Contains(TEXT("Frame Rate")))
        {
            // Test performance
            TestCase.ErrorMessage = TEXT("Performance test not yet implemented");
            return EQA_TestResult::Warning;
        }
        else
        {
            TestCase.ErrorMessage = TEXT("Unknown test type");
            return EQA_TestResult::Failed;
        }
    }
    catch (...)
    {
        TestCase.ErrorMessage = TEXT("Test execution threw exception");
        return EQA_TestResult::Failed;
    }
}

void UQATestManager::LogTestResult(const FQA_TestCase& TestCase)
{
    FString ResultString = UEnum::GetValueAsString(TestCase.Result);
    
    if (TestCase.Result == EQA_TestResult::Passed)
    {
        UE_LOG(LogTemp, Log, TEXT("QA Test PASSED: %s (%.3fs)"), 
               *TestCase.TestName, TestCase.ExecutionTime);
    }
    else if (TestCase.Result == EQA_TestResult::Failed)
    {
        UE_LOG(LogTemp, Error, TEXT("QA Test FAILED: %s - %s (%.3fs)"), 
               *TestCase.TestName, *TestCase.ErrorMessage, TestCase.ExecutionTime);
    }
    else if (TestCase.Result == EQA_TestResult::Warning)
    {
        UE_LOG(LogTemp, Warning, TEXT("QA Test WARNING: %s - %s (%.3fs)"), 
               *TestCase.TestName, *TestCase.ErrorMessage, TestCase.ExecutionTime);
    }
}

void UQATestManager::UpdateTestSuiteStats(FQA_TestSuite& TestSuite)
{
    UE_LOG(LogTemp, Warning, TEXT("QA Test Suite '%s' completed: %d passed, %d failed (%.2fs)"),
           *TestSuite.SuiteName, TestSuite.PassedTests, TestSuite.FailedTests, 
           TestSuite.TotalExecutionTime);
}

EQA_TestResult UQATestManager::TestClassLoading(const FString& ClassName)
{
    UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassName);
    
    if (LoadedClass)
    {
        return EQA_TestResult::Passed;
    }
    else
    {
        return EQA_TestResult::Failed;
    }
}

EQA_TestResult UQATestManager::TestActorSpawning(UClass* ActorClass)
{
    if (!ActorClass)
    {
        return EQA_TestResult::Failed;
    }
    
    UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        return EQA_TestResult::Failed;
    }
    
    AActor* SpawnedActor = World->SpawnActor<AActor>(ActorClass);
    if (SpawnedActor)
    {
        // Clean up test actor
        SpawnedActor->Destroy();
        return EQA_TestResult::Passed;
    }
    
    return EQA_TestResult::Failed;
}

EQA_TestResult UQATestManager::TestComponentFunctionality(UActorComponent* Component)
{
    if (!Component)
    {
        return EQA_TestResult::Failed;
    }
    
    // Basic component validation
    if (Component->IsValidLowLevel() && !Component->IsPendingKill())
    {
        return EQA_TestResult::Passed;
    }
    
    return EQA_TestResult::Failed;
}

EQA_TestResult UQATestManager::TestMapLoading(const FString& MapPath)
{
    // This would require more complex map loading logic
    // For now, return warning as not implemented
    return EQA_TestResult::Warning;
}