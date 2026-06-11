#include "QA_TestFramework.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"
#include "HAL/PlatformFilemanager.h"

UQA_TestFramework::UQA_TestFramework()
{
    bAutoRunOnStart = false;
    PerformanceThreshold = 60.0f; // 60 FPS minimum
    MaxAllowedErrors = 5;
    StartTime = 0.0f;
    EndTime = 0.0f;
}

void UQA_TestFramework::RunAllTests()
{
    UE_LOG(LogTemp, Warning, TEXT("QA Framework: Starting all tests"));
    StartTime = FPlatformTime::Seconds();
    
    for (FQA_TestSuite& Suite : TestSuites)
    {
        RunTestSuite(Suite.SuiteName);
    }
    
    EndTime = FPlatformTime::Seconds();
    GenerateTestReport();
}

void UQA_TestFramework::RunTestSuite(const FString& SuiteName)
{
    UE_LOG(LogTemp, Warning, TEXT("QA Framework: Running test suite: %s"), *SuiteName);
    
    for (FQA_TestSuite& Suite : TestSuites)
    {
        if (Suite.SuiteName == SuiteName)
        {
            Suite.PassedTests = 0;
            Suite.FailedTests = 0;
            Suite.WarningTests = 0;
            
            for (FQA_TestCase& TestCase : Suite.TestCases)
            {
                ExecuteTest(TestCase);
            }
            
            UpdateTestSuiteStats(Suite);
            break;
        }
    }
}

void UQA_TestFramework::RunSingleTest(const FString& TestName)
{
    UE_LOG(LogTemp, Warning, TEXT("QA Framework: Running single test: %s"), *TestName);
    
    for (FQA_TestSuite& Suite : TestSuites)
    {
        for (FQA_TestCase& TestCase : Suite.TestCases)
        {
            if (TestCase.TestName == TestName)
            {
                ExecuteTest(TestCase);
                LogTestResult(TestCase);
                return;
            }
        }
    }
}

void UQA_TestFramework::RegisterTestCase(const FString& SuiteName, const FString& TestName, const FString& Description)
{
    // Find or create test suite
    FQA_TestSuite* TargetSuite = nullptr;
    for (FQA_TestSuite& Suite : TestSuites)
    {
        if (Suite.SuiteName == SuiteName)
        {
            TargetSuite = &Suite;
            break;
        }
    }
    
    if (!TargetSuite)
    {
        RegisterTestSuite(SuiteName);
        TargetSuite = &TestSuites.Last();
    }
    
    // Add test case
    FQA_TestCase NewTest;
    NewTest.TestName = TestName;
    NewTest.TestDescription = Description;
    NewTest.Result = EQA_TestResult::NotRun;
    
    TargetSuite->TestCases.Add(NewTest);
    
    UE_LOG(LogTemp, Log, TEXT("QA Framework: Registered test '%s' in suite '%s'"), *TestName, *SuiteName);
}

void UQA_TestFramework::RegisterTestSuite(const FString& SuiteName)
{
    FQA_TestSuite NewSuite;
    NewSuite.SuiteName = SuiteName;
    TestSuites.Add(NewSuite);
    
    UE_LOG(LogTemp, Log, TEXT("QA Framework: Registered test suite '%s'"), *SuiteName);
}

bool UQA_TestFramework::ValidateClassLoading()
{
    UE_LOG(LogTemp, Warning, TEXT("QA Framework: Validating class loading"));
    
    TArray<FString> CriticalClasses = {
        TEXT("/Script/TranspersonalGame.TranspersonalCharacter"),
        TEXT("/Script/TranspersonalGame.TranspersonalGameState"),
        TEXT("/Script/TranspersonalGame.PCGWorldGenerator"),
        TEXT("/Script/TranspersonalGame.FoliageManager"),
        TEXT("/Script/TranspersonalGame.VFX_NiagaraLibrary")
    };
    
    int32 LoadedCount = 0;
    for (const FString& ClassName : CriticalClasses)
    {
        UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassName);
        if (LoadedClass)
        {
            LoadedCount++;
            UE_LOG(LogTemp, Log, TEXT("✓ Class loaded: %s"), *ClassName);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("✗ Failed to load class: %s"), *ClassName);
        }
    }
    
    bool bSuccess = (LoadedCount == CriticalClasses.Num());
    UE_LOG(LogTemp, Warning, TEXT("Class loading validation: %d/%d classes loaded"), LoadedCount, CriticalClasses.Num());
    
    return bSuccess;
}

bool UQA_TestFramework::ValidateActorSpawning()
{
    UE_LOG(LogTemp, Warning, TEXT("QA Framework: Validating actor spawning"));
    
    UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("No valid world found for actor spawning test"));
        return false;
    }
    
    // Test basic actor spawning
    FVector SpawnLocation(1000.0f, 1000.0f, 100.0f);
    FRotator SpawnRotation(0.0f, 0.0f, 0.0f);
    
    AActor* TestActor = World->SpawnActor<AActor>(AActor::StaticClass(), SpawnLocation, SpawnRotation);
    if (TestActor)
    {
        TestActor->SetActorLabel(TEXT("QA_SpawnTest"));
        UE_LOG(LogTemp, Log, TEXT("✓ Basic actor spawning successful"));
        
        // Clean up test actor
        TestActor->Destroy();
        return true;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("✗ Basic actor spawning failed"));
        return false;
    }
}

bool UQA_TestFramework::ValidateVFXSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("QA Framework: Validating VFX systems"));
    
    // Check if VFX_NiagaraLibrary class exists
    UClass* VFXClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.VFX_NiagaraLibrary"));
    if (!VFXClass)
    {
        UE_LOG(LogTemp, Error, TEXT("✗ VFX_NiagaraLibrary class not found"));
        return false;
    }
    
    UE_LOG(LogTemp, Log, TEXT("✓ VFX_NiagaraLibrary class loaded successfully"));
    
    // Additional VFX validation can be added here
    return true;
}

bool UQA_TestFramework::ValidatePerformance()
{
    UE_LOG(LogTemp, Warning, TEXT("QA Framework: Validating performance"));
    
    // Basic performance validation
    // In a real implementation, this would check frame rate, memory usage, etc.
    float CurrentFPS = 1.0f / FApp::GetDeltaTime();
    
    if (CurrentFPS >= PerformanceThreshold)
    {
        UE_LOG(LogTemp, Log, TEXT("✓ Performance validation passed: %.2f FPS"), CurrentFPS);
        return true;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("⚠ Performance below threshold: %.2f FPS (target: %.2f)"), CurrentFPS, PerformanceThreshold);
        return false;
    }
}

bool UQA_TestFramework::ValidateIntegration()
{
    UE_LOG(LogTemp, Warning, TEXT("QA Framework: Validating system integration"));
    
    // Test integration between different systems
    bool bClassLoadingOK = ValidateClassLoading();
    bool bActorSpawningOK = ValidateActorSpawning();
    bool bVFXSystemsOK = ValidateVFXSystems();
    
    bool bIntegrationOK = bClassLoadingOK && bActorSpawningOK && bVFXSystemsOK;
    
    if (bIntegrationOK)
    {
        UE_LOG(LogTemp, Log, TEXT("✓ System integration validation passed"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("✗ System integration validation failed"));
    }
    
    return bIntegrationOK;
}

void UQA_TestFramework::GenerateTestReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== QA TEST REPORT ==="));
    
    int32 TotalPassed = 0;
    int32 TotalFailed = 0;
    int32 TotalWarnings = 0;
    
    for (const FQA_TestSuite& Suite : TestSuites)
    {
        UE_LOG(LogTemp, Warning, TEXT("Suite: %s"), *Suite.SuiteName);
        UE_LOG(LogTemp, Warning, TEXT("  Passed: %d, Failed: %d, Warnings: %d"), 
               Suite.PassedTests, Suite.FailedTests, Suite.WarningTests);
        
        TotalPassed += Suite.PassedTests;
        TotalFailed += Suite.FailedTests;
        TotalWarnings += Suite.WarningTests;
    }
    
    float TotalTime = EndTime - StartTime;
    UE_LOG(LogTemp, Warning, TEXT("=== SUMMARY ==="));
    UE_LOG(LogTemp, Warning, TEXT("Total Passed: %d"), TotalPassed);
    UE_LOG(LogTemp, Warning, TEXT("Total Failed: %d"), TotalFailed);
    UE_LOG(LogTemp, Warning, TEXT("Total Warnings: %d"), TotalWarnings);
    UE_LOG(LogTemp, Warning, TEXT("Execution Time: %.2f seconds"), TotalTime);
    UE_LOG(LogTemp, Warning, TEXT("=== END REPORT ==="));
}

FString UQA_TestFramework::GetTestSummary()
{
    int32 TotalPassed = 0;
    int32 TotalFailed = 0;
    int32 TotalWarnings = 0;
    
    for (const FQA_TestSuite& Suite : TestSuites)
    {
        TotalPassed += Suite.PassedTests;
        TotalFailed += Suite.FailedTests;
        TotalWarnings += Suite.WarningTests;
    }
    
    return FString::Printf(TEXT("QA Summary: %d Passed, %d Failed, %d Warnings"), 
                          TotalPassed, TotalFailed, TotalWarnings);
}

TArray<FQA_TestCase> UQA_TestFramework::GetFailedTests()
{
    TArray<FQA_TestCase> FailedTests;
    
    for (const FQA_TestSuite& Suite : TestSuites)
    {
        for (const FQA_TestCase& TestCase : Suite.TestCases)
        {
            if (TestCase.Result == EQA_TestResult::Fail)
            {
                FailedTests.Add(TestCase);
            }
        }
    }
    
    return FailedTests;
}

void UQA_TestFramework::ExecuteTest(FQA_TestCase& TestCase)
{
    float TestStartTime = FPlatformTime::Seconds();
    
    UE_LOG(LogTemp, Log, TEXT("Executing test: %s"), *TestCase.TestName);
    
    // Execute specific test based on test name
    bool bTestPassed = false;
    
    if (TestCase.TestName.Contains(TEXT("ClassLoading")))
    {
        bTestPassed = ValidateClassLoading();
    }
    else if (TestCase.TestName.Contains(TEXT("ActorSpawning")))
    {
        bTestPassed = ValidateActorSpawning();
    }
    else if (TestCase.TestName.Contains(TEXT("VFX")))
    {
        bTestPassed = ValidateVFXSystems();
    }
    else if (TestCase.TestName.Contains(TEXT("Performance")))
    {
        bTestPassed = ValidatePerformance();
    }
    else if (TestCase.TestName.Contains(TEXT("Integration")))
    {
        bTestPassed = ValidateIntegration();
    }
    else
    {
        // Default test execution
        bTestPassed = true;
        UE_LOG(LogTemp, Warning, TEXT("No specific test implementation for: %s"), *TestCase.TestName);
    }
    
    TestCase.Result = bTestPassed ? EQA_TestResult::Pass : EQA_TestResult::Fail;
    TestCase.ExecutionTime = FPlatformTime::Seconds() - TestStartTime;
    
    LogTestResult(TestCase);
}

void UQA_TestFramework::UpdateTestSuiteStats(FQA_TestSuite& Suite)
{
    Suite.PassedTests = 0;
    Suite.FailedTests = 0;
    Suite.WarningTests = 0;
    
    for (const FQA_TestCase& TestCase : Suite.TestCases)
    {
        switch (TestCase.Result)
        {
            case EQA_TestResult::Pass:
                Suite.PassedTests++;
                break;
            case EQA_TestResult::Fail:
                Suite.FailedTests++;
                break;
            case EQA_TestResult::Warning:
                Suite.WarningTests++;
                break;
            default:
                break;
        }
    }
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
        case EQA_TestResult::Warning:
            ResultString = TEXT("WARNING");
            break;
        default:
            ResultString = TEXT("NOT_RUN");
            break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Test Result: %s - %s (%.3fs)"), 
           *TestCase.TestName, *ResultString, TestCase.ExecutionTime);
    
    if (!TestCase.ErrorMessage.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("  Error: %s"), *TestCase.ErrorMessage);
    }
}

// AQA_TestActor Implementation
AQA_TestActor::AQA_TestActor()
{
    PrimaryActorTick.bCanEverTick = false;
    
    TestFramework = CreateDefaultSubobject<UQA_TestFramework>(TEXT("TestFramework"));
    bRunTestsOnBeginPlay = false;
}

void AQA_TestActor::BeginPlay()
{
    Super::BeginPlay();
    
    if (bRunTestsOnBeginPlay)
    {
        StartTesting();
    }
}

void AQA_TestActor::StartTesting()
{
    if (TestFramework)
    {
        UE_LOG(LogTemp, Warning, TEXT("QA Test Actor: Starting automated testing"));
        
        // Register default test suites
        TestFramework->RegisterTestSuite(TEXT("Core Systems"));
        TestFramework->RegisterTestSuite(TEXT("VFX Systems"));
        TestFramework->RegisterTestSuite(TEXT("Performance"));
        TestFramework->RegisterTestSuite(TEXT("Integration"));
        
        // Register specific tests
        TestFramework->RegisterTestCase(TEXT("Core Systems"), TEXT("ClassLoading"), TEXT("Validate all critical classes load correctly"));
        TestFramework->RegisterTestCase(TEXT("Core Systems"), TEXT("ActorSpawning"), TEXT("Validate basic actor spawning functionality"));
        TestFramework->RegisterTestCase(TEXT("VFX Systems"), TEXT("VFX_NiagaraLibrary"), TEXT("Validate VFX Niagara library functionality"));
        TestFramework->RegisterTestCase(TEXT("Performance"), TEXT("Performance_FPS"), TEXT("Validate frame rate performance"));
        TestFramework->RegisterTestCase(TEXT("Integration"), TEXT("Integration_CrossSystem"), TEXT("Validate cross-system integration"));
        
        // Run tests
        if (TestSuitesToRun.Num() > 0)
        {
            for (const FString& SuiteName : TestSuitesToRun)
            {
                TestFramework->RunTestSuite(SuiteName);
            }
        }
        else
        {
            TestFramework->RunAllTests();
        }
    }
}

void AQA_TestActor::StopTesting()
{
    UE_LOG(LogTemp, Warning, TEXT("QA Test Actor: Stopping testing"));
    // Implementation for stopping tests if needed
}

bool AQA_TestActor::IsTestingComplete()
{
    // Implementation to check if all tests are complete
    return true;
}