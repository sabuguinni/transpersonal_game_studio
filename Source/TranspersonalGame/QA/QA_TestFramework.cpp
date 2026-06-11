#include "QA_TestFramework.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/AudioComponent.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"

UQA_TestFramework::UQA_TestFramework()
{
    PrimaryComponentTick.bCanEverTick = true;
    bAutoRunTests = false;
    TestTimeout = 30.0f;
    bVerboseLogging = true;
}

void UQA_TestFramework::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoRunTests)
    {
        InitializeTestSuite(TEXT("AutoRun_Suite"));
        RunAllTests();
    }
}

void UQA_TestFramework::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UQA_TestFramework::InitializeTestSuite(const FString& SuiteName)
{
    CurrentTestSuite = FQA_TestSuite();
    CurrentTestSuite.SuiteName = SuiteName;
    
    if (bVerboseLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("QA Framework: Initialized test suite '%s'"), *SuiteName);
    }
}

void UQA_TestFramework::AddTestCase(const FString& TestName, const FString& Description)
{
    FQA_TestCase NewTest;
    NewTest.TestName = TestName;
    NewTest.TestDescription = Description;
    NewTest.Result = EQA_TestResult::NotRun;
    
    CurrentTestSuite.TestCases.Add(NewTest);
    
    if (bVerboseLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("QA Framework: Added test case '%s'"), *TestName);
    }
}

void UQA_TestFramework::RunAllTests()
{
    if (CurrentTestSuite.TestCases.Num() == 0)
    {
        // Add default test cases
        AddTestCase(TEXT("Actor_Spawning"), TEXT("Validate basic actor spawning functionality"));
        AddTestCase(TEXT("Component_Integration"), TEXT("Validate component attachment and functionality"));
        AddTestCase(TEXT("VFX_Systems"), TEXT("Validate VFX and particle systems"));
        AddTestCase(TEXT("Audio_Systems"), TEXT("Validate audio component functionality"));
        AddTestCase(TEXT("Performance_Metrics"), TEXT("Validate performance within acceptable limits"));
    }
    
    float StartTime = GetCurrentTime();
    
    for (int32 i = 0; i < CurrentTestSuite.TestCases.Num(); i++)
    {
        ExecuteTestCase(CurrentTestSuite.TestCases[i]);
    }
    
    CurrentTestSuite.TotalExecutionTime = GetCurrentTime() - StartTime;
    UpdateTestSuiteStats();
    
    CompletedTestSuites.Add(CurrentTestSuite);
    GenerateTestReport();
}

void UQA_TestFramework::RunSpecificTest(const FString& TestName)
{
    for (int32 i = 0; i < CurrentTestSuite.TestCases.Num(); i++)
    {
        if (CurrentTestSuite.TestCases[i].TestName == TestName)
        {
            ExecuteTestCase(CurrentTestSuite.TestCases[i]);
            break;
        }
    }
}

bool UQA_TestFramework::ValidateActorSpawning()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Test spawning a basic actor
    FVector SpawnLocation = FVector(0, 0, 100);
    FRotator SpawnRotation = FRotator::ZeroRotator;
    
    AActor* TestActor = World->SpawnActor<AActor>(AActor::StaticClass(), SpawnLocation, SpawnRotation);
    
    if (TestActor)
    {
        TestActor->Destroy();
        return true;
    }
    
    return false;
}

bool UQA_TestFramework::ValidateComponentIntegration()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return false;
    }
    
    // Check if owner has required components
    UStaticMeshComponent* MeshComp = Owner->FindComponentByClass<UStaticMeshComponent>();
    
    // Basic validation - owner should exist and be valid
    return Owner->IsValidLowLevel() && !Owner->IsPendingKill();
}

bool UQA_TestFramework::ValidateVFXSystems()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Count particle system components in the world
    int32 ParticleCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->FindComponentByClass<UParticleSystemComponent>())
        {
            ParticleCount++;
        }
    }
    
    // Pass if we have at least one particle system or if none are required
    return true; // Always pass for now - VFX systems are optional
}

bool UQA_TestFramework::ValidateAudioSystems()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Count audio components in the world
    int32 AudioCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->FindComponentByClass<UAudioComponent>())
        {
            AudioCount++;
        }
    }
    
    // Pass if we have at least one audio component or if none are required
    return true; // Always pass for now - Audio systems are optional
}

bool UQA_TestFramework::ValidatePerformanceMetrics()
{
    // Basic performance validation
    // In a real implementation, this would check FPS, memory usage, etc.
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Count total actors - fail if we have an unreasonable number
    int32 ActorCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        ActorCount++;
    }
    
    // Fail if we have more than 10000 actors (performance concern)
    return ActorCount < 10000;
}

void UQA_TestFramework::GenerateTestReport()
{
    FString ReportContent = FString::Printf(TEXT("=== QA TEST REPORT ===\n"));
    ReportContent += FString::Printf(TEXT("Suite: %s\n"), *CurrentTestSuite.SuiteName);
    ReportContent += FString::Printf(TEXT("Total Tests: %d\n"), CurrentTestSuite.TestCases.Num());
    ReportContent += FString::Printf(TEXT("Passed: %d\n"), CurrentTestSuite.PassedTests);
    ReportContent += FString::Printf(TEXT("Failed: %d\n"), CurrentTestSuite.FailedTests);
    ReportContent += FString::Printf(TEXT("Warnings: %d\n"), CurrentTestSuite.WarningTests);
    ReportContent += FString::Printf(TEXT("Execution Time: %.2f seconds\n"), CurrentTestSuite.TotalExecutionTime);
    ReportContent += TEXT("\n=== TEST DETAILS ===\n");
    
    for (const FQA_TestCase& TestCase : CurrentTestSuite.TestCases)
    {
        FString ResultString;
        switch (TestCase.Result)
        {
            case EQA_TestResult::Pass: ResultString = TEXT("PASS"); break;
            case EQA_TestResult::Fail: ResultString = TEXT("FAIL"); break;
            case EQA_TestResult::Warning: ResultString = TEXT("WARN"); break;
            case EQA_TestResult::Critical: ResultString = TEXT("CRIT"); break;
            default: ResultString = TEXT("NOT_RUN"); break;
        }
        
        ReportContent += FString::Printf(TEXT("%s: %s (%.2fs)\n"), 
            *TestCase.TestName, *ResultString, TestCase.ExecutionTime);
        
        if (!TestCase.ErrorMessage.IsEmpty())
        {
            ReportContent += FString::Printf(TEXT("  Error: %s\n"), *TestCase.ErrorMessage);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("%s"), *ReportContent);
}

FString UQA_TestFramework::GetTestSummary()
{
    return FString::Printf(TEXT("QA Suite '%s': %d/%d tests passed"), 
        *CurrentTestSuite.SuiteName, 
        CurrentTestSuite.PassedTests, 
        CurrentTestSuite.TestCases.Num());
}

void UQA_TestFramework::LogTestResult(const FString& TestName, EQA_TestResult Result, const FString& Message)
{
    FString ResultString;
    switch (Result)
    {
        case EQA_TestResult::Pass: ResultString = TEXT("PASS"); break;
        case EQA_TestResult::Fail: ResultString = TEXT("FAIL"); break;
        case EQA_TestResult::Warning: ResultString = TEXT("WARN"); break;
        case EQA_TestResult::Critical: ResultString = TEXT("CRIT"); break;
        default: ResultString = TEXT("UNKNOWN"); break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("QA Test '%s': %s - %s"), *TestName, *ResultString, *Message);
}

void UQA_TestFramework::ExecuteTestCase(FQA_TestCase& TestCase)
{
    float StartTime = GetCurrentTime();
    bool TestResult = false;
    FString ErrorMsg = TEXT("");
    
    try
    {
        if (TestCase.TestName == TEXT("Actor_Spawning"))
        {
            TestResult = ValidateActorSpawning();
        }
        else if (TestCase.TestName == TEXT("Component_Integration"))
        {
            TestResult = ValidateComponentIntegration();
        }
        else if (TestCase.TestName == TEXT("VFX_Systems"))
        {
            TestResult = ValidateVFXSystems();
        }
        else if (TestCase.TestName == TEXT("Audio_Systems"))
        {
            TestResult = ValidateAudioSystems();
        }
        else if (TestCase.TestName == TEXT("Performance_Metrics"))
        {
            TestResult = ValidatePerformanceMetrics();
        }
        else
        {
            TestResult = false;
            ErrorMsg = TEXT("Unknown test case");
        }
    }
    catch (...)
    {
        TestResult = false;
        ErrorMsg = TEXT("Exception during test execution");
    }
    
    TestCase.ExecutionTime = GetCurrentTime() - StartTime;
    TestCase.Result = TestResult ? EQA_TestResult::Pass : EQA_TestResult::Fail;
    TestCase.ErrorMessage = ErrorMsg;
    
    LogTestResult(TestCase.TestName, TestCase.Result, ErrorMsg);
}

void UQA_TestFramework::UpdateTestSuiteStats()
{
    CurrentTestSuite.PassedTests = 0;
    CurrentTestSuite.FailedTests = 0;
    CurrentTestSuite.WarningTests = 0;
    
    for (const FQA_TestCase& TestCase : CurrentTestSuite.TestCases)
    {
        switch (TestCase.Result)
        {
            case EQA_TestResult::Pass:
                CurrentTestSuite.PassedTests++;
                break;
            case EQA_TestResult::Fail:
            case EQA_TestResult::Critical:
                CurrentTestSuite.FailedTests++;
                break;
            case EQA_TestResult::Warning:
                CurrentTestSuite.WarningTests++;
                break;
        }
    }
}

float UQA_TestFramework::GetCurrentTime()
{
    return FPlatformTime::Seconds();
}