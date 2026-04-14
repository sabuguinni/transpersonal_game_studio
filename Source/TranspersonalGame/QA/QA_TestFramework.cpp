#include "QA_TestFramework.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/DateTime.h"

UQA_TestFramework::UQA_TestFramework()
{
    bTestingInProgress = false;
    LastTestRun = FDateTime::MinValue();
}

void UQA_TestFramework::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("QA Test Framework initialized"));
    
    // Initialize default test suites
    InitializeDefaultTests();
}

void UQA_TestFramework::Deinitialize()
{
    TestSuites.Empty();
    AllTestCases.Empty();
    
    Super::Deinitialize();
}

void UQA_TestFramework::InitializeDefaultTests()
{
    // Compilation Test Suite
    FQA_TestSuite CompilationSuite;
    CompilationSuite.SuiteName = TEXT("Compilation Tests");
    
    FQA_TestCase ModuleLoadTest;
    ModuleLoadTest.TestName = TEXT("Module Loading");
    ModuleLoadTest.Description = TEXT("Verify TranspersonalGame module loads correctly");
    ModuleLoadTest.Category = EQA_TestCategory::Compilation;
    ModuleLoadTest.Priority = 10;
    CompilationSuite.TestCases.Add(ModuleLoadTest);
    
    FQA_TestCase ClassLoadTest;
    ClassLoadTest.TestName = TEXT("Class Loading");
    ClassLoadTest.Description = TEXT("Verify all UCLASS types can be loaded");
    ClassLoadTest.Category = EQA_TestCategory::Compilation;
    ClassLoadTest.Priority = 9;
    CompilationSuite.TestCases.Add(ClassLoadTest);
    
    TestSuites.Add(CompilationSuite.SuiteName, CompilationSuite);
    
    // Functional Test Suite
    FQA_TestSuite FunctionalSuite;
    FunctionalSuite.SuiteName = TEXT("Functional Tests");
    
    FQA_TestCase ActorSpawnTest;
    ActorSpawnTest.TestName = TEXT("Actor Spawning");
    ActorSpawnTest.Description = TEXT("Verify custom actors can be spawned");
    ActorSpawnTest.Category = EQA_TestCategory::Functional;
    ActorSpawnTest.Priority = 8;
    FunctionalSuite.TestCases.Add(ActorSpawnTest);
    
    FQA_TestCase ComponentTest;
    ComponentTest.TestName = TEXT("Component Functionality");
    ComponentTest.Description = TEXT("Verify components work correctly");
    ComponentTest.Category = EQA_TestCategory::Functional;
    ComponentTest.Priority = 7;
    FunctionalSuite.TestCases.Add(ComponentTest);
    
    TestSuites.Add(FunctionalSuite.SuiteName, FunctionalSuite);
    
    // Integration Test Suite
    FQA_TestSuite IntegrationSuite;
    IntegrationSuite.SuiteName = TEXT("Integration Tests");
    
    FQA_TestCase GameplayTest;
    GameplayTest.TestName = TEXT("Gameplay Integration");
    GameplayTest.Description = TEXT("Verify gameplay systems work together");
    GameplayTest.Category = EQA_TestCategory::Integration;
    GameplayTest.Priority = 6;
    IntegrationSuite.TestCases.Add(GameplayTest);
    
    TestSuites.Add(IntegrationSuite.SuiteName, IntegrationSuite);
}

void UQA_TestFramework::RunAllTests()
{
    if (bTestingInProgress)
    {
        UE_LOG(LogTemp, Warning, TEXT("Tests already in progress"));
        return;
    }
    
    bTestingInProgress = true;
    LastTestRun = FDateTime::Now();
    
    UE_LOG(LogTemp, Log, TEXT("=== RUNNING ALL QA TESTS ==="));
    
    for (auto& SuitePair : TestSuites)
    {
        RunTestSuite(SuitePair.Key);
    }
    
    GenerateTestReport();
    bTestingInProgress = false;
    
    UE_LOG(LogTemp, Log, TEXT("=== ALL TESTS COMPLETE ==="));
}

void UQA_TestFramework::RunTestSuite(const FString& SuiteName)
{
    if (!TestSuites.Contains(SuiteName))
    {
        UE_LOG(LogTemp, Error, TEXT("Test suite not found: %s"), *SuiteName);
        return;
    }
    
    FQA_TestSuite& TestSuite = TestSuites[SuiteName];
    UE_LOG(LogTemp, Log, TEXT("Running test suite: %s"), *SuiteName);
    
    TestSuite.PassedTests = 0;
    TestSuite.FailedTests = 0;
    TestSuite.WarningTests = 0;
    TestSuite.TotalExecutionTime = 0.0f;
    
    for (FQA_TestCase& TestCase : TestSuite.TestCases)
    {
        ExecuteTestCase(TestCase);
        LogTestResult(TestCase);
        
        switch (TestCase.Result)
        {
            case EQA_TestResult::Pass:
                TestSuite.PassedTests++;
                break;
            case EQA_TestResult::Fail:
            case EQA_TestResult::Critical:
                TestSuite.FailedTests++;
                break;
            case EQA_TestResult::Warning:
                TestSuite.WarningTests++;
                break;
        }
        
        TestSuite.TotalExecutionTime += TestCase.ExecutionTime;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Suite %s complete: %d passed, %d failed, %d warnings"), 
           *SuiteName, TestSuite.PassedTests, TestSuite.FailedTests, TestSuite.WarningTests);
}

void UQA_TestFramework::RunSingleTest(const FString& TestName)
{
    for (auto& SuitePair : TestSuites)
    {
        for (FQA_TestCase& TestCase : SuitePair.Value.TestCases)
        {
            if (TestCase.TestName == TestName)
            {
                ExecuteTestCase(TestCase);
                LogTestResult(TestCase);
                return;
            }
        }
    }
    
    UE_LOG(LogTemp, Error, TEXT("Test not found: %s"), *TestName);
}

void UQA_TestFramework::ExecuteTestCase(FQA_TestCase& TestCase)
{
    double StartTime = FPlatformTime::Seconds();
    TestCase.Timestamp = FDateTime::Now();
    TestCase.Result = EQA_TestResult::NotRun;
    TestCase.ErrorMessage = TEXT("");
    
    try
    {
        if (TestCase.TestName == TEXT("Module Loading"))
        {
            ValidateModuleCompilation();
            TestCase.Result = EQA_TestResult::Pass;
        }
        else if (TestCase.TestName == TEXT("Class Loading"))
        {
            ValidateClassLoading();
            TestCase.Result = EQA_TestResult::Pass;
        }
        else if (TestCase.TestName == TEXT("Actor Spawning"))
        {
            ValidateActorSpawning();
            TestCase.Result = EQA_TestResult::Pass;
        }
        else if (TestCase.TestName == TEXT("Component Functionality"))
        {
            ValidateComponentFunctionality();
            TestCase.Result = EQA_TestResult::Pass;
        }
        else if (TestCase.TestName == TEXT("Gameplay Integration"))
        {
            ValidateGameplayIntegration();
            TestCase.Result = EQA_TestResult::Pass;
        }
        else
        {
            TestCase.Result = EQA_TestResult::Warning;
            TestCase.ErrorMessage = TEXT("Test not implemented");
        }
    }
    catch (...)
    {
        TestCase.Result = EQA_TestResult::Critical;
        TestCase.ErrorMessage = TEXT("Test crashed with exception");
    }
    
    double EndTime = FPlatformTime::Seconds();
    TestCase.ExecutionTime = static_cast<float>(EndTime - StartTime);
}

void UQA_TestFramework::ValidateModuleCompilation()
{
    // Check if we can access basic UE5 functionality
    UWorld* World = GetWorld();
    if (!World)
    {
        throw std::runtime_error("Cannot access world");
    }
    
    // Check if TranspersonalGame module classes are available
    UClass* CharacterClass = FindObject<UClass>(ANY_PACKAGE, TEXT("TranspersonalCharacter"));
    if (!CharacterClass)
    {
        throw std::runtime_error("TranspersonalCharacter class not found");
    }
}

void UQA_TestFramework::ValidateClassLoading()
{
    TArray<FString> TestClasses = {
        TEXT("TranspersonalCharacter"),
        TEXT("TranspersonalGameState"),
        TEXT("PCGWorldGenerator"),
        TEXT("FoliageManager")
    };
    
    for (const FString& ClassName : TestClasses)
    {
        UClass* TestClass = FindObject<UClass>(ANY_PACKAGE, *ClassName);
        if (!TestClass)
        {
            FString ErrorMsg = FString::Printf(TEXT("Class not found: %s"), *ClassName);
            throw std::runtime_error(TCHAR_TO_UTF8(*ErrorMsg));
        }
    }
}

void UQA_TestFramework::ValidateActorSpawning()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        throw std::runtime_error("Cannot access world for actor spawning");
    }
    
    // Try to spawn a basic static mesh actor
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
    AStaticMeshActor* TestActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
    
    if (!TestActor)
    {
        throw std::runtime_error("Failed to spawn test actor");
    }
    
    // Clean up
    TestActor->Destroy();
}

void UQA_TestFramework::ValidateComponentFunctionality()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        throw std::runtime_error("Cannot access world for component testing");
    }
    
    // Create a test actor with components
    FActorSpawnParameters SpawnParams;
    AActor* TestActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
    
    if (!TestActor)
    {
        throw std::runtime_error("Failed to spawn test actor for component testing");
    }
    
    // Add a static mesh component
    UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(TestActor);
    if (!MeshComp)
    {
        TestActor->Destroy();
        throw std::runtime_error("Failed to create static mesh component");
    }
    
    TestActor->SetRootComponent(MeshComp);
    
    // Clean up
    TestActor->Destroy();
}

void UQA_TestFramework::ValidateGameplayIntegration()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        throw std::runtime_error("Cannot access world for gameplay testing");
    }
    
    // Check if game mode exists
    AGameModeBase* GameMode = World->GetAuthGameMode();
    if (!GameMode)
    {
        throw std::runtime_error("No game mode found");
    }
    
    // Check if we can access gameplay statics
    UGameplayStatics* GameplayStatics = UGameplayStatics::StaticClass()->GetDefaultObject<UGameplayStatics>();
    if (!GameplayStatics)
    {
        throw std::runtime_error("Cannot access gameplay statics");
    }
}

void UQA_TestFramework::RegisterTestCase(const FQA_TestCase& TestCase)
{
    AllTestCases.Add(TestCase);
    
    // Add to appropriate test suite or create new one
    FString SuiteName = UEnum::GetValueAsString(TestCase.Category);
    if (!TestSuites.Contains(SuiteName))
    {
        FQA_TestSuite NewSuite;
        NewSuite.SuiteName = SuiteName;
        TestSuites.Add(SuiteName, NewSuite);
    }
    
    TestSuites[SuiteName].TestCases.Add(TestCase);
}

void UQA_TestFramework::RegisterTestSuite(const FQA_TestSuite& TestSuite)
{
    TestSuites.Add(TestSuite.SuiteName, TestSuite);
    
    for (const FQA_TestCase& TestCase : TestSuite.TestCases)
    {
        AllTestCases.Add(TestCase);
    }
}

FQA_TestSuite UQA_TestFramework::GetTestSuite(const FString& SuiteName) const
{
    if (TestSuites.Contains(SuiteName))
    {
        return TestSuites[SuiteName];
    }
    
    return FQA_TestSuite();
}

TArray<FQA_TestCase> UQA_TestFramework::GetFailedTests() const
{
    TArray<FQA_TestCase> FailedTests;
    
    for (const FQA_TestCase& TestCase : AllTestCases)
    {
        if (TestCase.Result == EQA_TestResult::Fail || TestCase.Result == EQA_TestResult::Critical)
        {
            FailedTests.Add(TestCase);
        }
    }
    
    return FailedTests;
}

void UQA_TestFramework::GenerateTestReport()
{
    FString ReportContent = TEXT("=== QA TEST REPORT ===\n");
    ReportContent += FString::Printf(TEXT("Generated: %s\n\n"), *FDateTime::Now().ToString());
    
    int32 TotalPassed = 0;
    int32 TotalFailed = 0;
    int32 TotalWarnings = 0;
    float TotalTime = 0.0f;
    
    for (const auto& SuitePair : TestSuites)
    {
        const FQA_TestSuite& Suite = SuitePair.Value;
        ReportContent += FString::Printf(TEXT("Suite: %s\n"), *Suite.SuiteName);
        ReportContent += FString::Printf(TEXT("  Passed: %d, Failed: %d, Warnings: %d\n"), 
                                       Suite.PassedTests, Suite.FailedTests, Suite.WarningTests);
        ReportContent += FString::Printf(TEXT("  Execution Time: %.3fs\n\n"), Suite.TotalExecutionTime);
        
        TotalPassed += Suite.PassedTests;
        TotalFailed += Suite.FailedTests;
        TotalWarnings += Suite.WarningTests;
        TotalTime += Suite.TotalExecutionTime;
    }
    
    ReportContent += FString::Printf(TEXT("TOTAL: %d passed, %d failed, %d warnings\n"), 
                                   TotalPassed, TotalFailed, TotalWarnings);
    ReportContent += FString::Printf(TEXT("Total execution time: %.3fs\n"), TotalTime);
    
    // Log the report
    UE_LOG(LogTemp, Log, TEXT("%s"), *ReportContent);
    
    // Save to file
    FString ReportPath = FPaths::ProjectLogDir() / TEXT("QA_TestReport.txt");
    FFileHelper::SaveStringToFile(ReportContent, *ReportPath);
}

bool UQA_TestFramework::HasCriticalFailures() const
{
    for (const FQA_TestCase& TestCase : AllTestCases)
    {
        if (TestCase.Result == EQA_TestResult::Critical)
        {
            return true;
        }
    }
    
    return false;
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
            ResultString = TEXT("WARN");
            break;
        case EQA_TestResult::Critical:
            ResultString = TEXT("CRITICAL");
            break;
        default:
            ResultString = TEXT("NOT_RUN");
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("[%s] %s (%.3fs) - %s"), 
           *ResultString, *TestCase.TestName, TestCase.ExecutionTime, 
           TestCase.ErrorMessage.IsEmpty() ? TEXT("OK") : *TestCase.ErrorMessage);
}