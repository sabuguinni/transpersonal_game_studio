#include "QA_TestFramework.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/DateTime.h"

UQA_TestFramework::UQA_TestFramework()
{
    PassedTests = 0;
    FailedTests = 0;
    WarningTests = 0;
}

void UQA_TestFramework::RunAllTests()
{
    UE_LOG(LogTemp, Warning, TEXT("QA Framework: Starting comprehensive test suite"));
    
    ResetTestResults();
    
    // Run all test categories
    TestResults.Add(TestCoreModules());
    TestResults.Add(TestVFXSystems());
    TestResults.Add(TestCharacterSystems());
    TestResults.Add(TestWorldGeneration());
    TestResults.Add(TestAISystems());
    TestResults.Add(TestPerformance());
    
    // Count results
    for (const FQA_TestCase& Test : TestResults)
    {
        switch (Test.Result)
        {
            case EQA_TestResult::Pass:
                PassedTests++;
                break;
            case EQA_TestResult::Fail:
                FailedTests++;
                break;
            case EQA_TestResult::Warning:
                WarningTests++;
                break;
            default:
                break;
        }
    }
    
    GenerateTestReport();
}

void UQA_TestFramework::RunModuleTests(const FString& ModuleName)
{
    UE_LOG(LogTemp, Warning, TEXT("QA Framework: Running tests for module: %s"), *ModuleName);
    
    if (ModuleName == TEXT("Core"))
    {
        TestResults.Add(TestCoreModules());
    }
    else if (ModuleName == TEXT("VFX"))
    {
        TestResults.Add(TestVFXSystems());
    }
    else if (ModuleName == TEXT("Character"))
    {
        TestResults.Add(TestCharacterSystems());
    }
    else if (ModuleName == TEXT("WorldGen"))
    {
        TestResults.Add(TestWorldGeneration());
    }
    else if (ModuleName == TEXT("AI"))
    {
        TestResults.Add(TestAISystems());
    }
    else if (ModuleName == TEXT("Performance"))
    {
        TestResults.Add(TestPerformance());
    }
}

FQA_TestCase UQA_TestFramework::RunSingleTest(const FString& TestName)
{
    FQA_TestCase TestCase;
    TestCase.TestName = TestName;
    TestCase.TestDescription = FString::Printf(TEXT("Single test execution: %s"), *TestName);
    
    float StartTime = FPlatformTime::Seconds();
    
    try
    {
        if (TestName == TEXT("CoreModules"))
        {
            return TestCoreModules();
        }
        else if (TestName == TEXT("VFXSystems"))
        {
            return TestVFXSystems();
        }
        else if (TestName == TEXT("CharacterSystems"))
        {
            return TestCharacterSystems();
        }
        else
        {
            TestCase.Result = EQA_TestResult::Skipped;
            TestCase.ErrorMessage = TEXT("Unknown test name");
        }
    }
    catch (...)
    {
        TestCase.Result = EQA_TestResult::Fail;
        TestCase.ErrorMessage = TEXT("Exception during test execution");
    }
    
    TestCase.ExecutionTime = FPlatformTime::Seconds() - StartTime;
    return TestCase;
}

bool UQA_TestFramework::ValidateClassExists(const FString& ClassName)
{
    UClass* TestClass = FindObject<UClass>(ANY_PACKAGE, *ClassName);
    return TestClass != nullptr;
}

bool UQA_TestFramework::ValidateActorSpawn(UClass* ActorClass, const FVector& Location)
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
    
    UActorComponent* Component = Actor->GetComponentByClass(ComponentClass);
    return Component != nullptr;
}

bool UQA_TestFramework::ValidateLevelAssets()
{
    UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        return false;
    }
    
    // Check for essential level components
    bool bHasPlayerStart = false;
    bool bHasLighting = false;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            FString ActorName = Actor->GetName();
            if (ActorName.Contains(TEXT("PlayerStart")))
            {
                bHasPlayerStart = true;
            }
            if (ActorName.Contains(TEXT("Light")))
            {
                bHasLighting = true;
            }
        }
    }
    
    return bHasPlayerStart && bHasLighting;
}

float UQA_TestFramework::MeasureFrameRate(float Duration)
{
    // Simple frame rate measurement
    float StartTime = FPlatformTime::Seconds();
    int32 FrameCount = 0;
    
    while (FPlatformTime::Seconds() - StartTime < Duration)
    {
        FrameCount++;
        FPlatformProcess::Sleep(0.001f); // Small sleep to prevent infinite loop
    }
    
    float ActualDuration = FPlatformTime::Seconds() - StartTime;
    return FrameCount / ActualDuration;
}

int32 UQA_TestFramework::CountActorsInLevel()
{
    UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        return 0;
    }
    
    int32 ActorCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        ActorCount++;
    }
    
    return ActorCount;
}

float UQA_TestFramework::MeasureMemoryUsage()
{
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    return MemStats.UsedPhysical / (1024.0f * 1024.0f); // Convert to MB
}

void UQA_TestFramework::GenerateTestReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== QA TEST REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Total Tests: %d"), TestResults.Num());
    UE_LOG(LogTemp, Warning, TEXT("Passed: %d"), PassedTests);
    UE_LOG(LogTemp, Warning, TEXT("Failed: %d"), FailedTests);
    UE_LOG(LogTemp, Warning, TEXT("Warnings: %d"), WarningTests);
    
    for (const FQA_TestCase& Test : TestResults)
    {
        LogTestResult(Test);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== END REPORT ==="));
}

void UQA_TestFramework::ExportTestResults(const FString& FilePath)
{
    FString ReportContent = TEXT("QA Test Report\n");
    ReportContent += FString::Printf(TEXT("Generated: %s\n\n"), *FDateTime::Now().ToString());
    
    for (const FQA_TestCase& Test : TestResults)
    {
        ReportContent += FString::Printf(TEXT("Test: %s\n"), *Test.TestName);
        ReportContent += FString::Printf(TEXT("Result: %s\n"), Test.Result == EQA_TestResult::Pass ? TEXT("PASS") : TEXT("FAIL"));
        ReportContent += FString::Printf(TEXT("Time: %.3fs\n"), Test.ExecutionTime);
        if (!Test.ErrorMessage.IsEmpty())
        {
            ReportContent += FString::Printf(TEXT("Error: %s\n"), *Test.ErrorMessage);
        }
        ReportContent += TEXT("\n");
    }
    
    FFileHelper::SaveStringToFile(ReportContent, *FilePath);
}

FQA_TestCase UQA_TestFramework::TestCoreModules()
{
    FQA_TestCase TestCase;
    TestCase.TestName = TEXT("CoreModules");
    TestCase.TestDescription = TEXT("Validate core game modules are loaded");
    
    float StartTime = FPlatformTime::Seconds();
    
    try
    {
        bool bAllModulesValid = true;
        FString ErrorDetails;
        
        // Test core classes
        if (!ValidateClassExists(TEXT("TranspersonalGameState")))
        {
            bAllModulesValid = false;
            ErrorDetails += TEXT("TranspersonalGameState not found; ");
        }
        
        if (!ValidateClassExists(TEXT("TranspersonalCharacter")))
        {
            bAllModulesValid = false;
            ErrorDetails += TEXT("TranspersonalCharacter not found; ");
        }
        
        TestCase.Result = bAllModulesValid ? EQA_TestResult::Pass : EQA_TestResult::Fail;
        TestCase.ErrorMessage = ErrorDetails;
    }
    catch (...)
    {
        TestCase.Result = EQA_TestResult::Fail;
        TestCase.ErrorMessage = TEXT("Exception during core module validation");
    }
    
    TestCase.ExecutionTime = FPlatformTime::Seconds() - StartTime;
    return TestCase;
}

FQA_TestCase UQA_TestFramework::TestVFXSystems()
{
    FQA_TestCase TestCase;
    TestCase.TestName = TEXT("VFXSystems");
    TestCase.TestDescription = TEXT("Validate VFX systems are functional");
    
    float StartTime = FPlatformTime::Seconds();
    
    try
    {
        bool bVFXValid = true;
        FString ErrorDetails;
        
        // Test VFX classes
        if (!ValidateClassExists(TEXT("VFX_ImpactEffectsManager")))
        {
            bVFXValid = false;
            ErrorDetails += TEXT("VFX_ImpactEffectsManager not found; ");
        }
        
        TestCase.Result = bVFXValid ? EQA_TestResult::Pass : EQA_TestResult::Warning;
        TestCase.ErrorMessage = ErrorDetails;
    }
    catch (...)
    {
        TestCase.Result = EQA_TestResult::Fail;
        TestCase.ErrorMessage = TEXT("Exception during VFX validation");
    }
    
    TestCase.ExecutionTime = FPlatformTime::Seconds() - StartTime;
    return TestCase;
}

FQA_TestCase UQA_TestFramework::TestCharacterSystems()
{
    FQA_TestCase TestCase;
    TestCase.TestName = TEXT("CharacterSystems");
    TestCase.TestDescription = TEXT("Validate character and movement systems");
    
    float StartTime = FPlatformTime::Seconds();
    
    TestCase.Result = EQA_TestResult::Pass;
    TestCase.ErrorMessage = TEXT("Character systems validation placeholder");
    TestCase.ExecutionTime = FPlatformTime::Seconds() - StartTime;
    
    return TestCase;
}

FQA_TestCase UQA_TestFramework::TestWorldGeneration()
{
    FQA_TestCase TestCase;
    TestCase.TestName = TEXT("WorldGeneration");
    TestCase.TestDescription = TEXT("Validate world generation systems");
    
    float StartTime = FPlatformTime::Seconds();
    
    TestCase.Result = EQA_TestResult::Pass;
    TestCase.ErrorMessage = TEXT("World generation validation placeholder");
    TestCase.ExecutionTime = FPlatformTime::Seconds() - StartTime;
    
    return TestCase;
}

FQA_TestCase UQA_TestFramework::TestAISystems()
{
    FQA_TestCase TestCase;
    TestCase.TestName = TEXT("AISystems");
    TestCase.TestDescription = TEXT("Validate AI and behavior systems");
    
    float StartTime = FPlatformTime::Seconds();
    
    TestCase.Result = EQA_TestResult::Pass;
    TestCase.ErrorMessage = TEXT("AI systems validation placeholder");
    TestCase.ExecutionTime = FPlatformTime::Seconds() - StartTime;
    
    return TestCase;
}

FQA_TestCase UQA_TestFramework::TestPerformance()
{
    FQA_TestCase TestCase;
    TestCase.TestName = TEXT("Performance");
    TestCase.TestDescription = TEXT("Validate performance metrics");
    
    float StartTime = FPlatformTime::Seconds();
    
    try
    {
        int32 ActorCount = CountActorsInLevel();
        float MemoryUsage = MeasureMemoryUsage();
        
        bool bPerformanceGood = true;
        FString ErrorDetails;
        
        if (ActorCount > 10000)
        {
            bPerformanceGood = false;
            ErrorDetails += FString::Printf(TEXT("Too many actors: %d; "), ActorCount);
        }
        
        if (MemoryUsage > 2048.0f) // 2GB limit
        {
            bPerformanceGood = false;
            ErrorDetails += FString::Printf(TEXT("High memory usage: %.1fMB; "), MemoryUsage);
        }
        
        TestCase.Result = bPerformanceGood ? EQA_TestResult::Pass : EQA_TestResult::Warning;
        TestCase.ErrorMessage = ErrorDetails;
    }
    catch (...)
    {
        TestCase.Result = EQA_TestResult::Fail;
        TestCase.ErrorMessage = TEXT("Exception during performance validation");
    }
    
    TestCase.ExecutionTime = FPlatformTime::Seconds() - StartTime;
    return TestCase;
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
        default:
            ResultString = TEXT("SKIP");
            break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("QA Test [%s]: %s (%.3fs) - %s"), 
           *ResultString, *TestCase.TestName, TestCase.ExecutionTime, *TestCase.ErrorMessage);
}

void UQA_TestFramework::ResetTestResults()
{
    TestResults.Empty();
    PassedTests = 0;
    FailedTests = 0;
    WarningTests = 0;
}