#include "QA_TestFramework.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "UObject/UObjectGlobals.h"
#include "Misc/DateTime.h"
#include "HAL/PlatformFilemanager.h"

UQA_TestFramework::UQA_TestFramework()
{
    PassedTests = 0;
    FailedTests = 0;
    WarningTests = 0;
}

void UQA_TestFramework::RunAllTests()
{
    UE_LOG(LogTemp, Warning, TEXT("QA Framework: Starting comprehensive test suite"));
    
    ClearTestResults();
    
    // Run all test categories
    TestVFXSystems();
    TestCharacterMovement();
    TestDinosaurAI();
    TestWorldGeneration();
    TestAudioSystems();
    TestPerformanceMetrics();
    
    GenerateTestReport();
}

void UQA_TestFramework::RunTestByName(const FString& TestName)
{
    if (TestName == TEXT("VFX"))
    {
        TestVFXSystems();
    }
    else if (TestName == TEXT("Character"))
    {
        TestCharacterMovement();
    }
    else if (TestName == TEXT("Dinosaur"))
    {
        TestDinosaurAI();
    }
    else if (TestName == TEXT("World"))
    {
        TestWorldGeneration();
    }
    else if (TestName == TEXT("Audio"))
    {
        TestAudioSystems();
    }
    else if (TestName == TEXT("Performance"))
    {
        TestPerformanceMetrics();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("QA Framework: Unknown test name: %s"), *TestName);
    }
}

void UQA_TestFramework::AddTestCase(const FQA_TestCase& TestCase)
{
    TestCases.Add(TestCase);
    LogTestResult(TestCase);
}

TArray<FQA_TestCase> UQA_TestFramework::GetTestResults() const
{
    return TestCases;
}

void UQA_TestFramework::ClearTestResults()
{
    TestCases.Empty();
    PassedTests = 0;
    FailedTests = 0;
    WarningTests = 0;
}

void UQA_TestFramework::TestVFXSystems()
{
    FQA_TestCase VFXTest = CreateTestCase(TEXT("VFX_Systems"), TEXT("Validate VFX Niagara systems and particle effects"));
    
    float StartTime = FPlatformTime::Seconds();
    
    try
    {
        // Test VFX class loading
        UClass* VFXLibClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.VFX_NiagaraLibrary"));
        if (!VFXLibClass)
        {
            VFXTest.Result = EQA_TestResult::Fail;
            VFXTest.ErrorMessage = TEXT("Failed to load VFX_NiagaraLibrary class");
        }
        else
        {
            // Test Niagara asset loading
            UObject* CampfireAsset = LoadObject<UObject>(nullptr, TEXT("/Game/VFX/NS_CampfireFlames"));
            UObject* FootstepAsset = LoadObject<UObject>(nullptr, TEXT("/Game/VFX/NS_FootstepImpact"));
            
            if (!CampfireAsset && !FootstepAsset)
            {
                VFXTest.Result = EQA_TestResult::Warning;
                VFXTest.ErrorMessage = TEXT("VFX assets not found - may need to be created");
            }
            else
            {
                VFXTest.Result = EQA_TestResult::Pass;
                VFXTest.ErrorMessage = TEXT("VFX systems validated successfully");
            }
        }
    }
    catch (...)
    {
        VFXTest.Result = EQA_TestResult::Fail;
        VFXTest.ErrorMessage = TEXT("Exception during VFX testing");
    }
    
    VFXTest.ExecutionTime = FPlatformTime::Seconds() - StartTime;
    AddTestCase(VFXTest);
}

void UQA_TestFramework::TestCharacterMovement()
{
    FQA_TestCase CharTest = CreateTestCase(TEXT("Character_Movement"), TEXT("Validate TranspersonalCharacter movement and controls"));
    
    float StartTime = FPlatformTime::Seconds();
    
    try
    {
        UClass* CharClass = LoadClass<AActor>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
        if (!CharClass)
        {
            CharTest.Result = EQA_TestResult::Fail;
            CharTest.ErrorMessage = TEXT("Failed to load TranspersonalCharacter class");
        }
        else
        {
            CharTest.Result = EQA_TestResult::Pass;
            CharTest.ErrorMessage = TEXT("Character class loaded successfully");
        }
    }
    catch (...)
    {
        CharTest.Result = EQA_TestResult::Fail;
        CharTest.ErrorMessage = TEXT("Exception during character testing");
    }
    
    CharTest.ExecutionTime = FPlatformTime::Seconds() - StartTime;
    AddTestCase(CharTest);
}

void UQA_TestFramework::TestDinosaurAI()
{
    FQA_TestCase DinoTest = CreateTestCase(TEXT("Dinosaur_AI"), TEXT("Validate dinosaur behavior and AI systems"));
    
    float StartTime = FPlatformTime::Seconds();
    
    // Test dinosaur AI classes
    DinoTest.Result = EQA_TestResult::Warning;
    DinoTest.ErrorMessage = TEXT("Dinosaur AI systems pending implementation");
    
    DinoTest.ExecutionTime = FPlatformTime::Seconds() - StartTime;
    AddTestCase(DinoTest);
}

void UQA_TestFramework::TestWorldGeneration()
{
    FQA_TestCase WorldTest = CreateTestCase(TEXT("World_Generation"), TEXT("Validate PCG world generation systems"));
    
    float StartTime = FPlatformTime::Seconds();
    
    try
    {
        UClass* WorldGenClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.PCGWorldGenerator"));
        if (!WorldGenClass)
        {
            WorldTest.Result = EQA_TestResult::Fail;
            WorldTest.ErrorMessage = TEXT("Failed to load PCGWorldGenerator class");
        }
        else
        {
            WorldTest.Result = EQA_TestResult::Pass;
            WorldTest.ErrorMessage = TEXT("World generation class loaded successfully");
        }
    }
    catch (...)
    {
        WorldTest.Result = EQA_TestResult::Fail;
        WorldTest.ErrorMessage = TEXT("Exception during world generation testing");
    }
    
    WorldTest.ExecutionTime = FPlatformTime::Seconds() - StartTime;
    AddTestCase(WorldTest);
}

void UQA_TestFramework::TestAudioSystems()
{
    FQA_TestCase AudioTest = CreateTestCase(TEXT("Audio_Systems"), TEXT("Validate audio and MetaSounds systems"));
    
    float StartTime = FPlatformTime::Seconds();
    
    // Test audio systems
    AudioTest.Result = EQA_TestResult::Warning;
    AudioTest.ErrorMessage = TEXT("Audio systems pending validation");
    
    AudioTest.ExecutionTime = FPlatformTime::Seconds() - StartTime;
    AddTestCase(AudioTest);
}

void UQA_TestFramework::TestPerformanceMetrics()
{
    FQA_TestCase PerfTest = CreateTestCase(TEXT("Performance_Metrics"), TEXT("Validate performance and optimization systems"));
    
    float StartTime = FPlatformTime::Seconds();
    
    // Basic performance validation
    if (GEngine)
    {
        PerfTest.Result = EQA_TestResult::Pass;
        PerfTest.ErrorMessage = TEXT("Engine performance systems accessible");
    }
    else
    {
        PerfTest.Result = EQA_TestResult::Fail;
        PerfTest.ErrorMessage = TEXT("Engine not accessible for performance testing");
    }
    
    PerfTest.ExecutionTime = FPlatformTime::Seconds() - StartTime;
    AddTestCase(PerfTest);
}

bool UQA_TestFramework::ValidateActorCount(int32 ExpectedCount, const FString& ActorType)
{
    if (UWorld* World = GEngine->GetCurrentPlayWorld())
    {
        int32 ActorCount = 0;
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor->GetName().Contains(ActorType))
            {
                ActorCount++;
            }
        }
        return ActorCount >= ExpectedCount;
    }
    return false;
}

bool UQA_TestFramework::ValidateClassLoading(const FString& ClassName)
{
    UClass* TestClass = LoadClass<UObject>(nullptr, *ClassName);
    return TestClass != nullptr;
}

bool UQA_TestFramework::ValidateAssetLoading(const FString& AssetPath)
{
    UObject* TestAsset = LoadObject<UObject>(nullptr, *AssetPath);
    return TestAsset != nullptr;
}

void UQA_TestFramework::GenerateTestReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== QA TEST REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Total Tests: %d"), TestCases.Num());
    UE_LOG(LogTemp, Warning, TEXT("Passed: %d"), PassedTests);
    UE_LOG(LogTemp, Warning, TEXT("Failed: %d"), FailedTests);
    UE_LOG(LogTemp, Warning, TEXT("Warnings: %d"), WarningTests);
    
    for (const FQA_TestCase& TestCase : TestCases)
    {
        FString ResultStr;
        switch (TestCase.Result)
        {
            case EQA_TestResult::Pass: ResultStr = TEXT("PASS"); break;
            case EQA_TestResult::Fail: ResultStr = TEXT("FAIL"); break;
            case EQA_TestResult::Warning: ResultStr = TEXT("WARN"); break;
            default: ResultStr = TEXT("NONE"); break;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("[%s] %s: %s (%.3fs)"), 
               *ResultStr, *TestCase.TestName, *TestCase.ErrorMessage, TestCase.ExecutionTime);
    }
    UE_LOG(LogTemp, Warning, TEXT("=== END QA REPORT ==="));
}

void UQA_TestFramework::LogTestResult(const FQA_TestCase& TestCase)
{
    switch (TestCase.Result)
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

FQA_TestCase UQA_TestFramework::CreateTestCase(const FString& Name, const FString& Description)
{
    FQA_TestCase TestCase;
    TestCase.TestName = Name;
    TestCase.TestDescription = Description;
    TestCase.Result = EQA_TestResult::NotRun;
    TestCase.ErrorMessage = TEXT("");
    TestCase.ExecutionTime = 0.0f;
    return TestCase;
}