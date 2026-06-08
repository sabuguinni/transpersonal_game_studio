#include "QA_TestFramework.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/DateTime.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"

UQA_TestFramework::UQA_TestFramework()
{
    bIsInitialized = false;
    PerformanceThresholdFPS = 30.0f;
    PerformanceThresholdMemoryMB = 2048.0f;
}

void UQA_TestFramework::InitializeTestFramework()
{
    if (bIsInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("QA Test Framework already initialized"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Initializing QA Test Framework..."));
    
    // Clear existing data
    TestSuites.Empty();
    RegisteredTests.Empty();
    
    // Initialize default test suites
    InitializeDefaultTests();
    
    bIsInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("QA Test Framework initialized successfully"));
}

void UQA_TestFramework::InitializeDefaultTests()
{
    // Core Systems Test Suite
    FQA_TestSuite CoreSuite;
    CoreSuite.SuiteName = TEXT("Core Systems");
    
    FQA_TestCase CharacterTest = CreateTestCase(
        TEXT("TranspersonalCharacter_Loading"),
        TEXT("Validate TranspersonalCharacter class can be loaded"),
        EQA_TestCategory::Core
    );
    CoreSuite.TestCases.Add(CharacterTest);
    
    FQA_TestCase GameStateTest = CreateTestCase(
        TEXT("TranspersonalGameState_Loading"),
        TEXT("Validate TranspersonalGameState class can be loaded"),
        EQA_TestCategory::Core
    );
    CoreSuite.TestCases.Add(GameStateTest);
    
    TestSuites.Add(CoreSuite);
    
    // VFX Test Suite
    FQA_TestSuite VFXSuite;
    VFXSuite.SuiteName = TEXT("VFX Systems");
    
    FQA_TestCase NiagaraTest = CreateTestCase(
        TEXT("VFX_NiagaraLibrary_Loading"),
        TEXT("Validate VFX_NiagaraLibrary class can be loaded"),
        EQA_TestCategory::VFX
    );
    VFXSuite.TestCases.Add(NiagaraTest);
    
    FQA_TestCase EffectManagerTest = CreateTestCase(
        TEXT("VFX_EffectManager_Loading"),
        TEXT("Validate VFX_EffectManager class can be loaded"),
        EQA_TestCategory::VFX
    );
    VFXSuite.TestCases.Add(EffectManagerTest);
    
    TestSuites.Add(VFXSuite);
    
    // Performance Test Suite
    FQA_TestSuite PerformanceSuite;
    PerformanceSuite.SuiteName = TEXT("Performance");
    
    FQA_TestCase FPSTest = CreateTestCase(
        TEXT("Performance_FPS_Check"),
        TEXT("Validate frame rate meets minimum threshold"),
        EQA_TestCategory::Performance
    );
    PerformanceSuite.TestCases.Add(FPSTest);
    
    FQA_TestCase MemoryTest = CreateTestCase(
        TEXT("Performance_Memory_Check"),
        TEXT("Validate memory usage is within acceptable limits"),
        EQA_TestCategory::Performance
    );
    PerformanceSuite.TestCases.Add(MemoryTest);
    
    TestSuites.Add(PerformanceSuite);
}

void UQA_TestFramework::RunAllTests()
{
    if (!bIsInitialized)
    {
        InitializeTestFramework();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Running all QA tests..."));
    
    for (FQA_TestSuite& Suite : TestSuites)
    {
        RunTestSuite(Suite.SuiteName);
    }
    
    GenerateTestReport();
}

void UQA_TestFramework::RunTestSuite(const FString& SuiteName)
{
    FQA_TestSuite* Suite = TestSuites.FindByPredicate([&SuiteName](const FQA_TestSuite& S) {
        return S.SuiteName == SuiteName;
    });
    
    if (!Suite)
    {
        UE_LOG(LogTemp, Error, TEXT("Test suite not found: %s"), *SuiteName);
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Running test suite: %s"), *SuiteName);
    
    Suite->PassCount = 0;
    Suite->FailCount = 0;
    Suite->WarningCount = 0;
    Suite->TotalExecutionTimeMs = 0.0f;
    
    for (FQA_TestCase& TestCase : Suite->TestCases)
    {
        RunSingleTest(TestCase.TestName);
        
        // Update suite counters
        switch (TestCase.Result)
        {
            case EQA_TestResult::Pass:
                Suite->PassCount++;
                break;
            case EQA_TestResult::Fail:
                Suite->FailCount++;
                break;
            case EQA_TestResult::Warning:
                Suite->WarningCount++;
                break;
            default:
                break;
        }
        
        Suite->TotalExecutionTimeMs += TestCase.ExecutionTimeMs;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Test suite %s completed: %d Pass, %d Fail, %d Warning"), 
           *SuiteName, Suite->PassCount, Suite->FailCount, Suite->WarningCount);
}

void UQA_TestFramework::RunSingleTest(const FString& TestName)
{
    double StartTime = FPlatformTime::Seconds();
    
    // Find the test case in all suites
    FQA_TestCase* TestCase = nullptr;
    for (FQA_TestSuite& Suite : TestSuites)
    {
        for (FQA_TestCase& Test : Suite.TestCases)
        {
            if (Test.TestName == TestName)
            {
                TestCase = &Test;
                break;
            }
        }
        if (TestCase) break;
    }
    
    if (!TestCase)
    {
        UE_LOG(LogTemp, Error, TEXT("Test case not found: %s"), *TestName);
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Running test: %s"), *TestName);
    
    // Execute specific test logic
    bool bTestPassed = false;
    FString ErrorMessage = TEXT("");
    
    if (TestName == TEXT("TranspersonalCharacter_Loading"))
    {
        bTestPassed = ValidateClassLoading(TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
    }
    else if (TestName == TEXT("TranspersonalGameState_Loading"))
    {
        bTestPassed = ValidateClassLoading(TEXT("/Script/TranspersonalGame.TranspersonalGameState"));
    }
    else if (TestName == TEXT("VFX_NiagaraLibrary_Loading"))
    {
        bTestPassed = ValidateClassLoading(TEXT("/Script/TranspersonalGame.VFX_NiagaraLibrary"));
    }
    else if (TestName == TEXT("VFX_EffectManager_Loading"))
    {
        bTestPassed = ValidateClassLoading(TEXT("/Script/TranspersonalGame.VFX_EffectManager"));
    }
    else if (TestName == TEXT("Performance_FPS_Check"))
    {
        float CurrentFPS = MeasureFrameRate();
        bTestPassed = CurrentFPS >= PerformanceThresholdFPS;
        if (!bTestPassed)
        {
            ErrorMessage = FString::Printf(TEXT("FPS %f below threshold %f"), CurrentFPS, PerformanceThresholdFPS);
        }
    }
    else if (TestName == TEXT("Performance_Memory_Check"))
    {
        float CurrentMemory = MeasureMemoryUsage();
        bTestPassed = CurrentMemory <= PerformanceThresholdMemoryMB;
        if (!bTestPassed)
        {
            ErrorMessage = FString::Printf(TEXT("Memory %f MB above threshold %f MB"), CurrentMemory, PerformanceThresholdMemoryMB);
        }
    }
    
    // Update test result
    EQA_TestResult Result = bTestPassed ? EQA_TestResult::Pass : EQA_TestResult::Fail;
    UpdateTestResult(*TestCase, Result, ErrorMessage);
    
    double EndTime = FPlatformTime::Seconds();
    TestCase->ExecutionTimeMs = (EndTime - StartTime) * 1000.0f;
    
    LogTestResult(*TestCase);
}

bool UQA_TestFramework::ValidateClassLoading(const FString& ClassName)
{
    UClass* LoadedClass = StaticLoadClass(UObject::StaticClass(), nullptr, *ClassName);
    return LoadedClass != nullptr;
}

bool UQA_TestFramework::ValidateActorSpawning(UClass* ActorClass, const FVector& Location)
{
    if (!ActorClass)
    {
        return false;
    }
    
    UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        return false;
    }
    
    AActor* SpawnedActor = World->SpawnActor<AActor>(ActorClass, Location, FRotator::ZeroRotator);
    if (SpawnedActor)
    {
        SpawnedActor->Destroy();
        return true;
    }
    
    return false;
}

bool UQA_TestFramework::ValidateComponentAttachment(AActor* Actor, UClass* ComponentClass)
{
    if (!Actor || !ComponentClass)
    {
        return false;
    }
    
    UActorComponent* Component = Actor->FindComponentByClass(ComponentClass);
    return Component != nullptr;
}

bool UQA_TestFramework::ValidateAssetLoading(const FString& AssetPath)
{
    UObject* LoadedAsset = StaticLoadObject(UObject::StaticClass(), nullptr, *AssetPath);
    return LoadedAsset != nullptr;
}

float UQA_TestFramework::MeasureFrameRate()
{
    if (GEngine && GEngine->GetGameViewport())
    {
        return GEngine->GetMaxFPS();
    }
    return 0.0f;
}

float UQA_TestFramework::MeasureMemoryUsage()
{
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    return MemStats.UsedPhysical / (1024.0f * 1024.0f); // Convert to MB
}

bool UQA_TestFramework::ValidatePerformanceThresholds()
{
    float CurrentFPS = MeasureFrameRate();
    float CurrentMemory = MeasureMemoryUsage();
    
    return (CurrentFPS >= PerformanceThresholdFPS) && (CurrentMemory <= PerformanceThresholdMemoryMB);
}

bool UQA_TestFramework::ValidateVFXSystem()
{
    bool bNiagaraValid = ValidateClassLoading(TEXT("/Script/TranspersonalGame.VFX_NiagaraLibrary"));
    bool bEffectManagerValid = ValidateClassLoading(TEXT("/Script/TranspersonalGame.VFX_EffectManager"));
    
    return bNiagaraValid && bEffectManagerValid;
}

bool UQA_TestFramework::ValidateNiagaraEffects()
{
    // Check if Niagara system can be loaded
    UNiagaraSystem* TestSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Engine/VFX/Niagara/DefaultAssets/FX_Smoke"));
    return TestSystem != nullptr;
}

bool UQA_TestFramework::ValidateParticlePerformance()
{
    // Basic particle performance check
    return ValidatePerformanceThresholds();
}

bool UQA_TestFramework::ValidateAudioSystem()
{
    // Basic audio system validation
    return GEngine && GEngine->GetAudioDeviceManager();
}

bool UQA_TestFramework::ValidateSoundEffects()
{
    // Check if basic sound cue can be loaded
    USoundCue* TestSound = LoadObject<USoundCue>(nullptr, TEXT("/Engine/EngineSounds/WhiteNoise01"));
    return TestSound != nullptr;
}

void UQA_TestFramework::RegisterTestCase(const FQA_TestCase& TestCase)
{
    RegisteredTests.Add(TestCase.TestName, TestCase);
}

void UQA_TestFramework::RegisterTestSuite(const FQA_TestSuite& TestSuite)
{
    TestSuites.Add(TestSuite);
}

void UQA_TestFramework::GenerateTestReport()
{
    UE_LOG(LogTemp, Log, TEXT("=== QA TEST FRAMEWORK REPORT ==="));
    
    int32 TotalPass = 0;
    int32 TotalFail = 0;
    int32 TotalWarning = 0;
    float TotalTime = 0.0f;
    
    for (const FQA_TestSuite& Suite : TestSuites)
    {
        UE_LOG(LogTemp, Log, TEXT("Suite: %s - Pass: %d, Fail: %d, Warning: %d, Time: %.2fms"),
               *Suite.SuiteName, Suite.PassCount, Suite.FailCount, Suite.WarningCount, Suite.TotalExecutionTimeMs);
        
        TotalPass += Suite.PassCount;
        TotalFail += Suite.FailCount;
        TotalWarning += Suite.WarningCount;
        TotalTime += Suite.TotalExecutionTimeMs;
    }
    
    UE_LOG(LogTemp, Log, TEXT("TOTAL - Pass: %d, Fail: %d, Warning: %d, Time: %.2fms"),
           TotalPass, TotalFail, TotalWarning, TotalTime);
    UE_LOG(LogTemp, Log, TEXT("=== END REPORT ==="));
}

void UQA_TestFramework::ExportTestResults(const FString& FilePath)
{
    FString ReportContent = TEXT("QA Test Framework Report\n");
    ReportContent += FString::Printf(TEXT("Generated: %s\n\n"), *FDateTime::Now().ToString());
    
    for (const FQA_TestSuite& Suite : TestSuites)
    {
        ReportContent += FString::Printf(TEXT("Test Suite: %s\n"), *Suite.SuiteName);
        ReportContent += FString::Printf(TEXT("Pass: %d, Fail: %d, Warning: %d\n"), 
                                       Suite.PassCount, Suite.FailCount, Suite.WarningCount);
        ReportContent += FString::Printf(TEXT("Execution Time: %.2fms\n\n"), Suite.TotalExecutionTimeMs);
        
        for (const FQA_TestCase& TestCase : Suite.TestCases)
        {
            FString ResultStr = TEXT("Unknown");
            switch (TestCase.Result)
            {
                case EQA_TestResult::Pass: ResultStr = TEXT("PASS"); break;
                case EQA_TestResult::Fail: ResultStr = TEXT("FAIL"); break;
                case EQA_TestResult::Warning: ResultStr = TEXT("WARNING"); break;
                case EQA_TestResult::Skipped: ResultStr = TEXT("SKIPPED"); break;
                case EQA_TestResult::InProgress: ResultStr = TEXT("IN_PROGRESS"); break;
            }
            
            ReportContent += FString::Printf(TEXT("  %s: %s (%.2fms)\n"), 
                                           *TestCase.TestName, *ResultStr, TestCase.ExecutionTimeMs);
            
            if (!TestCase.ErrorMessage.IsEmpty())
            {
                ReportContent += FString::Printf(TEXT("    Error: %s\n"), *TestCase.ErrorMessage);
            }
        }
        ReportContent += TEXT("\n");
    }
    
    FFileHelper::SaveStringToFile(ReportContent, *FilePath);
}

FQA_TestSuite UQA_TestFramework::GetTestSuiteResults(const FString& SuiteName)
{
    for (const FQA_TestSuite& Suite : TestSuites)
    {
        if (Suite.SuiteName == SuiteName)
        {
            return Suite;
        }
    }
    
    return FQA_TestSuite();
}

FQA_TestCase UQA_TestFramework::CreateTestCase(const FString& Name, const FString& Description, EQA_TestCategory Category)
{
    FQA_TestCase TestCase;
    TestCase.TestName = Name;
    TestCase.TestDescription = Description;
    TestCase.Category = Category;
    TestCase.Result = EQA_TestResult::InProgress;
    TestCase.bIsAutomated = true;
    return TestCase;
}

void UQA_TestFramework::UpdateTestResult(FQA_TestCase& TestCase, EQA_TestResult Result, const FString& ErrorMessage)
{
    TestCase.Result = Result;
    TestCase.ErrorMessage = ErrorMessage;
}

void UQA_TestFramework::LogTestResult(const FQA_TestCase& TestCase)
{
    FString ResultStr = TEXT("Unknown");
    switch (TestCase.Result)
    {
        case EQA_TestResult::Pass: ResultStr = TEXT("PASS"); break;
        case EQA_TestResult::Fail: ResultStr = TEXT("FAIL"); break;
        case EQA_TestResult::Warning: ResultStr = TEXT("WARNING"); break;
        case EQA_TestResult::Skipped: ResultStr = TEXT("SKIPPED"); break;
        case EQA_TestResult::InProgress: ResultStr = TEXT("IN_PROGRESS"); break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Test %s: %s (%.2fms)"), 
           *TestCase.TestName, *ResultStr, TestCase.ExecutionTimeMs);
    
    if (!TestCase.ErrorMessage.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("  Error: %s"), *TestCase.ErrorMessage);
    }
}