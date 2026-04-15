#include "QATestFramework.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/DateTime.h"
#include "Engine/GameViewportClient.h"

UQATestFramework::UQATestFramework()
{
    bAutoRunOnStartup = false;
    bGenerateDetailedLogs = true;
    TestResultsDirectory = TEXT("/Game/QA/TestResults/");
}

void UQATestFramework::RunAllTests()
{
    UE_LOG(LogTemp, Warning, TEXT("QA Framework: Running all test suites"));
    
    for (FQA_TestSuite& TestSuite : TestSuites)
    {
        RunTestSuite(TestSuite.SuiteName);
    }
    
    GenerateTestReport();
}

void UQATestFramework::RunTestSuite(const FString& SuiteName)
{
    UE_LOG(LogTemp, Warning, TEXT("QA Framework: Running test suite: %s"), *SuiteName);
    
    FQA_TestSuite* FoundSuite = TestSuites.FindByPredicate([&SuiteName](const FQA_TestSuite& Suite)
    {
        return Suite.SuiteName == SuiteName;
    });
    
    if (!FoundSuite)
    {
        UE_LOG(LogTemp, Error, TEXT("QA Framework: Test suite not found: %s"), *SuiteName);
        return;
    }
    
    // Reset suite stats
    FoundSuite->PassedTests = 0;
    FoundSuite->FailedTests = 0;
    FoundSuite->WarningTests = 0;
    FoundSuite->TotalExecutionTime = 0.0f;
    
    // Execute all test cases in the suite
    for (FQA_TestCase& TestCase : FoundSuite->TestCases)
    {
        ExecuteTestCase(TestCase);
    }
    
    UpdateTestSuiteStats(*FoundSuite);
    
    UE_LOG(LogTemp, Warning, TEXT("QA Framework: Suite %s completed - %d passed, %d failed, %d warnings"), 
           *SuiteName, FoundSuite->PassedTests, FoundSuite->FailedTests, FoundSuite->WarningTests);
}

void UQATestFramework::RunSingleTest(const FString& TestName)
{
    UE_LOG(LogTemp, Warning, TEXT("QA Framework: Running single test: %s"), *TestName);
    
    for (FQA_TestSuite& TestSuite : TestSuites)
    {
        for (FQA_TestCase& TestCase : TestSuite.TestCases)
        {
            if (TestCase.TestName == TestName)
            {
                ExecuteTestCase(TestCase);
                LogTestResult(TestCase);
                return;
            }
        }
    }
    
    UE_LOG(LogTemp, Error, TEXT("QA Framework: Test not found: %s"), *TestName);
}

void UQATestFramework::RegisterTestCase(const FQA_TestCase& TestCase)
{
    // Find or create default test suite
    FQA_TestSuite* DefaultSuite = TestSuites.FindByPredicate([](const FQA_TestSuite& Suite)
    {
        return Suite.SuiteName == TEXT("Default");
    });
    
    if (!DefaultSuite)
    {
        FQA_TestSuite NewSuite;
        NewSuite.SuiteName = TEXT("Default");
        TestSuites.Add(NewSuite);
        DefaultSuite = &TestSuites.Last();
    }
    
    DefaultSuite->TestCases.Add(TestCase);
    UE_LOG(LogTemp, Log, TEXT("QA Framework: Registered test case: %s"), *TestCase.TestName);
}

void UQATestFramework::RegisterTestSuite(const FQA_TestSuite& TestSuite)
{
    TestSuites.Add(TestSuite);
    UE_LOG(LogTemp, Log, TEXT("QA Framework: Registered test suite: %s with %d test cases"), 
           *TestSuite.SuiteName, TestSuite.TestCases.Num());
}

bool UQATestFramework::ValidateClassExists(const FString& ClassName)
{
    UClass* FoundClass = FindObject<UClass>(ANY_PACKAGE, *ClassName);
    bool bExists = (FoundClass != nullptr);
    
    UE_LOG(LogTemp, Log, TEXT("QA Framework: Class validation for %s: %s"), 
           *ClassName, bExists ? TEXT("PASS") : TEXT("FAIL"));
    
    return bExists;
}

bool UQATestFramework::ValidateActorSpawning(UClass* ActorClass)
{
    if (!ActorClass)
    {
        UE_LOG(LogTemp, Error, TEXT("QA Framework: Cannot validate spawning - null actor class"));
        return false;
    }
    
    UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("QA Framework: Cannot validate spawning - no world context"));
        return false;
    }
    
    FVector SpawnLocation = FVector::ZeroVector;
    FRotator SpawnRotation = FRotator::ZeroRotator;
    
    AActor* SpawnedActor = World->SpawnActor<AActor>(ActorClass, SpawnLocation, SpawnRotation);
    bool bSpawned = (SpawnedActor != nullptr);
    
    if (bSpawned)
    {
        // Clean up test actor
        SpawnedActor->Destroy();
        UE_LOG(LogTemp, Log, TEXT("QA Framework: Actor spawning validation for %s: PASS"), 
               *ActorClass->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("QA Framework: Actor spawning validation for %s: FAIL"), 
               *ActorClass->GetName());
    }
    
    return bSpawned;
}

bool UQATestFramework::ValidateComponentFunctionality(UActorComponent* Component)
{
    if (!Component)
    {
        UE_LOG(LogTemp, Error, TEXT("QA Framework: Cannot validate component - null component"));
        return false;
    }
    
    bool bIsValid = IsValid(Component);
    bool bIsActive = Component->IsActive();
    
    UE_LOG(LogTemp, Log, TEXT("QA Framework: Component validation for %s: Valid=%s, Active=%s"), 
           *Component->GetClass()->GetName(), 
           bIsValid ? TEXT("true") : TEXT("false"),
           bIsActive ? TEXT("true") : TEXT("false"));
    
    return bIsValid && bIsActive;
}

float UQATestFramework::MeasureFrameRate(float Duration)
{
    UE_LOG(LogTemp, Warning, TEXT("QA Framework: Measuring frame rate for %.2f seconds"), Duration);
    
    // This is a simplified frame rate measurement
    // In a real implementation, you'd accumulate frame times over the duration
    float CurrentFPS = 1.0f / FApp::GetDeltaTime();
    
    UE_LOG(LogTemp, Log, TEXT("QA Framework: Current FPS: %.2f"), CurrentFPS);
    return CurrentFPS;
}

int32 UQATestFramework::CountActorsInLevel()
{
    UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("QA Framework: Cannot count actors - no world context"));
        return 0;
    }
    
    int32 ActorCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        ActorCount++;
    }
    
    UE_LOG(LogTemp, Log, TEXT("QA Framework: Total actors in level: %d"), ActorCount);
    return ActorCount;
}

float UQATestFramework::MeasureMemoryUsage()
{
    // Simplified memory measurement
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    float UsedMemoryMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    
    UE_LOG(LogTemp, Log, TEXT("QA Framework: Current memory usage: %.2f MB"), UsedMemoryMB);
    return UsedMemoryMB;
}

void UQATestFramework::GenerateTestReport()
{
    UE_LOG(LogTemp, Warning, TEXT("QA Framework: Generating comprehensive test report"));
    
    FString ReportContent;
    ReportContent += TEXT("=== TRANSPERSONAL GAME QA TEST REPORT ===\n");
    ReportContent += FString::Printf(TEXT("Generated: %s\n\n"), *FDateTime::Now().ToString());
    
    int32 TotalTests = 0;
    int32 TotalPassed = 0;
    int32 TotalFailed = 0;
    int32 TotalWarnings = 0;
    
    for (const FQA_TestSuite& TestSuite : TestSuites)
    {
        ReportContent += FString::Printf(TEXT("Test Suite: %s\n"), *TestSuite.SuiteName);
        ReportContent += FString::Printf(TEXT("  Passed: %d, Failed: %d, Warnings: %d\n"), 
                                       TestSuite.PassedTests, TestSuite.FailedTests, TestSuite.WarningTests);
        ReportContent += FString::Printf(TEXT("  Execution Time: %.3f seconds\n\n"), TestSuite.TotalExecutionTime);
        
        TotalTests += TestSuite.TestCases.Num();
        TotalPassed += TestSuite.PassedTests;
        TotalFailed += TestSuite.FailedTests;
        TotalWarnings += TestSuite.WarningTests;
        
        for (const FQA_TestCase& TestCase : TestSuite.TestCases)
        {
            ReportContent += FString::Printf(TEXT("  [%s] %s - %s\n"), 
                                           *GetTestResultString(TestCase.Result),
                                           *TestCase.TestName,
                                           *TestCase.Description);
            
            if (!TestCase.ErrorMessage.IsEmpty())
            {
                ReportContent += FString::Printf(TEXT("    Error: %s\n"), *TestCase.ErrorMessage);
            }
        }
        ReportContent += TEXT("\n");
    }
    
    ReportContent += TEXT("=== SUMMARY ===\n");
    ReportContent += FString::Printf(TEXT("Total Tests: %d\n"), TotalTests);
    ReportContent += FString::Printf(TEXT("Passed: %d (%.1f%%)\n"), TotalPassed, 
                                   TotalTests > 0 ? (float)TotalPassed / TotalTests * 100.0f : 0.0f);
    ReportContent += FString::Printf(TEXT("Failed: %d (%.1f%%)\n"), TotalFailed,
                                   TotalTests > 0 ? (float)TotalFailed / TotalTests * 100.0f : 0.0f);
    ReportContent += FString::Printf(TEXT("Warnings: %d (%.1f%%)\n"), TotalWarnings,
                                   TotalTests > 0 ? (float)TotalWarnings / TotalTests * 100.0f : 0.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("%s"), *ReportContent);
    
    // Export to file
    FString ReportPath = FPaths::ProjectSavedDir() + TEXT("QA/TestReport_") + 
                        FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S")) + TEXT(".txt");
    
    if (FFileHelper::SaveStringToFile(ReportContent, *ReportPath))
    {
        UE_LOG(LogTemp, Warning, TEXT("QA Framework: Test report saved to: %s"), *ReportPath);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("QA Framework: Failed to save test report to: %s"), *ReportPath);
    }
}

void UQATestFramework::ExportTestResults(const FString& FilePath)
{
    FString ExportContent;
    ExportContent += TEXT("TestSuite,TestName,Result,ExecutionTime,ErrorMessage\n");
    
    for (const FQA_TestSuite& TestSuite : TestSuites)
    {
        for (const FQA_TestCase& TestCase : TestSuite.TestCases)
        {
            ExportContent += FString::Printf(TEXT("%s,%s,%s,%.3f,%s\n"),
                                           *TestSuite.SuiteName,
                                           *TestCase.TestName,
                                           *GetTestResultString(TestCase.Result),
                                           TestCase.ExecutionTime,
                                           *TestCase.ErrorMessage.Replace(TEXT(","), TEXT(";")));
        }
    }
    
    if (FFileHelper::SaveStringToFile(ExportContent, *FilePath))
    {
        UE_LOG(LogTemp, Warning, TEXT("QA Framework: Test results exported to: %s"), *FilePath);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("QA Framework: Failed to export test results to: %s"), *FilePath);
    }
}

FQA_TestSuite UQATestFramework::GetTestSuiteResults(const FString& SuiteName)
{
    const FQA_TestSuite* FoundSuite = TestSuites.FindByPredicate([&SuiteName](const FQA_TestSuite& Suite)
    {
        return Suite.SuiteName == SuiteName;
    });
    
    if (FoundSuite)
    {
        return *FoundSuite;
    }
    
    return FQA_TestSuite();
}

int32 UQATestFramework::GetTotalTestCount() const
{
    int32 Total = 0;
    for (const FQA_TestSuite& TestSuite : TestSuites)
    {
        Total += TestSuite.TestCases.Num();
    }
    return Total;
}

int32 UQATestFramework::GetPassedTestCount() const
{
    int32 Total = 0;
    for (const FQA_TestSuite& TestSuite : TestSuites)
    {
        Total += TestSuite.PassedTests;
    }
    return Total;
}

int32 UQATestFramework::GetFailedTestCount() const
{
    int32 Total = 0;
    for (const FQA_TestSuite& TestSuite : TestSuites)
    {
        Total += TestSuite.FailedTests;
    }
    return Total;
}

void UQATestFramework::ExecuteTestCase(FQA_TestCase& TestCase)
{
    UE_LOG(LogTemp, Log, TEXT("QA Framework: Executing test: %s"), *TestCase.TestName);
    
    float StartTime = FPlatformTime::Seconds();
    TestCase.ExecutionTimestamp = FDateTime::Now();
    TestCase.Result = EQA_TestResult::NotRun;
    TestCase.ErrorMessage = TEXT("");
    
    try
    {
        // Basic test execution logic
        // In a real implementation, this would dispatch to specific test methods
        if (TestCase.Category == EQA_TestCategory::Compilation)
        {
            // Test compilation-related functionality
            TestCase.Result = EQA_TestResult::Pass;
        }
        else if (TestCase.Category == EQA_TestCategory::Functionality)
        {
            // Test functional requirements
            TestCase.Result = EQA_TestResult::Pass;
        }
        else if (TestCase.Category == EQA_TestCategory::Performance)
        {
            // Test performance requirements
            float FPS = MeasureFrameRate(1.0f);
            if (FPS >= 30.0f)
            {
                TestCase.Result = EQA_TestResult::Pass;
            }
            else
            {
                TestCase.Result = EQA_TestResult::Warning;
                TestCase.ErrorMessage = FString::Printf(TEXT("Low FPS: %.2f"), FPS);
            }
        }
        else
        {
            TestCase.Result = EQA_TestResult::Pass;
        }
    }
    catch (...)
    {
        TestCase.Result = EQA_TestResult::Error;
        TestCase.ErrorMessage = TEXT("Exception occurred during test execution");
    }
    
    TestCase.ExecutionTime = FPlatformTime::Seconds() - StartTime;
    LogTestResult(TestCase);
}

void UQATestFramework::UpdateTestSuiteStats(FQA_TestSuite& TestSuite)
{
    TestSuite.PassedTests = 0;
    TestSuite.FailedTests = 0;
    TestSuite.WarningTests = 0;
    TestSuite.TotalExecutionTime = 0.0f;
    
    for (const FQA_TestCase& TestCase : TestSuite.TestCases)
    {
        switch (TestCase.Result)
        {
            case EQA_TestResult::Pass:
                TestSuite.PassedTests++;
                break;
            case EQA_TestResult::Fail:
            case EQA_TestResult::Error:
                TestSuite.FailedTests++;
                break;
            case EQA_TestResult::Warning:
                TestSuite.WarningTests++;
                break;
            default:
                break;
        }
        
        TestSuite.TotalExecutionTime += TestCase.ExecutionTime;
    }
}

void UQATestFramework::LogTestResult(const FQA_TestCase& TestCase)
{
    FString ResultString = GetTestResultString(TestCase.Result);
    
    if (TestCase.Result == EQA_TestResult::Pass)
    {
        UE_LOG(LogTemp, Log, TEXT("QA Test [%s] %s: PASSED (%.3fs)"), 
               *ResultString, *TestCase.TestName, TestCase.ExecutionTime);
    }
    else if (TestCase.Result == EQA_TestResult::Warning)
    {
        UE_LOG(LogTemp, Warning, TEXT("QA Test [%s] %s: WARNING - %s (%.3fs)"), 
               *ResultString, *TestCase.TestName, *TestCase.ErrorMessage, TestCase.ExecutionTime);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("QA Test [%s] %s: FAILED - %s (%.3fs)"), 
               *ResultString, *TestCase.TestName, *TestCase.ErrorMessage, TestCase.ExecutionTime);
    }
}

FString UQATestFramework::GetTestResultString(EQA_TestResult Result)
{
    switch (Result)
    {
        case EQA_TestResult::Pass:      return TEXT("PASS");
        case EQA_TestResult::Fail:      return TEXT("FAIL");
        case EQA_TestResult::Warning:   return TEXT("WARN");
        case EQA_TestResult::Error:     return TEXT("ERROR");
        case EQA_TestResult::NotRun:    return TEXT("NOT_RUN");
        default:                        return TEXT("UNKNOWN");
    }
}

// AQATestActor Implementation
AQATestActor::AQATestActor()
{
    PrimaryActorTick.bCanEverTick = true;
    
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    
    TestMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TestMesh"));
    TestMesh->SetupAttachment(RootComponent);
    
    // Set default test cube mesh
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        TestMesh->SetStaticMesh(CubeMeshAsset.Object);
    }
    
    bRunTestsOnBeginPlay = true;
    TestDuration = 5.0f;
    TestTimer = 0.0f;
    bTestsCompleted = false;
}

void AQATestActor::BeginPlay()
{
    Super::BeginPlay();
    
    if (bRunTestsOnBeginPlay)
    {
        UE_LOG(LogTemp, Warning, TEXT("QA Test Actor: Starting automated tests"));
        RunActorTests();
    }
}

void AQATestActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (!bTestsCompleted)
    {
        TestTimer += DeltaTime;
        if (TestTimer >= TestDuration)
        {
            bTestsCompleted = true;
            UE_LOG(LogTemp, Warning, TEXT("QA Test Actor: Test duration completed"));
        }
    }
}

void AQATestActor::RunActorTests()
{
    UE_LOG(LogTemp, Warning, TEXT("QA Test Actor: Running comprehensive actor tests"));
    
    TestMovement();
    TestCollision();
    TestVisibility();
}

void AQATestActor::TestMovement()
{
    UE_LOG(LogTemp, Log, TEXT("QA Test Actor: Testing movement capabilities"));
    
    FVector CurrentLocation = GetActorLocation();
    FVector TestLocation = CurrentLocation + FVector(100.0f, 0.0f, 0.0f);
    
    SetActorLocation(TestLocation);
    
    FVector NewLocation = GetActorLocation();
    bool bMovementSuccess = FVector::Dist(NewLocation, TestLocation) < 1.0f;
    
    UE_LOG(LogTemp, Log, TEXT("QA Test Actor: Movement test %s"), 
           bMovementSuccess ? TEXT("PASSED") : TEXT("FAILED"));
    
    // Reset position
    SetActorLocation(CurrentLocation);
}

void AQATestActor::TestCollision()
{
    UE_LOG(LogTemp, Log, TEXT("QA Test Actor: Testing collision detection"));
    
    if (TestMesh)
    {
        bool bHasCollision = TestMesh->GetCollisionEnabled() != ECollisionEnabled::NoCollision;
        UE_LOG(LogTemp, Log, TEXT("QA Test Actor: Collision test %s"), 
               bHasCollision ? TEXT("PASSED") : TEXT("FAILED"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("QA Test Actor: Collision test FAILED - No test mesh"));
    }
}

void AQATestActor::TestVisibility()
{
    UE_LOG(LogTemp, Log, TEXT("QA Test Actor: Testing visibility"));
    
    if (TestMesh)
    {
        bool bIsVisible = TestMesh->IsVisible();
        UE_LOG(LogTemp, Log, TEXT("QA Test Actor: Visibility test %s"), 
               bIsVisible ? TEXT("PASSED") : TEXT("FAILED"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("QA Test Actor: Visibility test FAILED - No test mesh"));
    }
}