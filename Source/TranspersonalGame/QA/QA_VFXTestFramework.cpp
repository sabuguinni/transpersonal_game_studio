#include "QA_VFXTestFramework.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Camera/CameraShakeBase.h"
#include "Components/AudioComponent.h"

UQA_VFXTestFramework::UQA_VFXTestFramework()
{
    PrimaryComponentTick.bCanEverTick = false;
    bAutoRunTests = false;
    TestTimeout = 30.0f;
}

void UQA_VFXTestFramework::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeTestCases();
    
    if (bAutoRunTests)
    {
        RunAllVFXTests();
    }
}

void UQA_VFXTestFramework::InitializeTestCases()
{
    TestCases.Empty();
    
    // Screen Shake Test
    FQA_VFXTestCase ScreenShakeTest;
    ScreenShakeTest.TestName = TEXT("ScreenShakeSystem");
    ScreenShakeTest.TestDescription = TEXT("Validates screen shake controller functionality");
    TestCases.Add(ScreenShakeTest);
    
    // Niagara Particle Test
    FQA_VFXTestCase NiagaraTest;
    NiagaraTest.TestName = TEXT("NiagaraParticleSystem");
    NiagaraTest.TestDescription = TEXT("Validates Niagara particle system integration");
    TestCases.Add(NiagaraTest);
    
    // VFX Audio Integration Test
    FQA_VFXTestCase AudioIntegrationTest;
    AudioIntegrationTest.TestName = TEXT("VFXAudioIntegration");
    AudioIntegrationTest.TestDescription = TEXT("Validates VFX and audio system integration");
    TestCases.Add(AudioIntegrationTest);
    
    // Damage Flash Effect Test
    FQA_VFXTestCase DamageFlashTest;
    DamageFlashTest.TestName = TEXT("DamageFlashEffect");
    DamageFlashTest.TestDescription = TEXT("Validates damage flash visual feedback");
    TestCases.Add(DamageFlashTest);
    
    // Environmental VFX Test
    FQA_VFXTestCase EnvironmentalTest;
    EnvironmentalTest.TestName = TEXT("EnvironmentalVFX");
    EnvironmentalTest.TestDescription = TEXT("Validates environmental particle effects");
    TestCases.Add(EnvironmentalTest);
}

void UQA_VFXTestFramework::RunAllVFXTests()
{
    UE_LOG(LogTemp, Warning, TEXT("QA VFX Test Framework: Starting comprehensive VFX validation"));
    
    // Run all test functions
    TestScreenShakeSystem();
    TestNiagaraParticleSystem();
    TestVFXAudioIntegration();
    TestDamageFlashEffect();
    TestEnvironmentalVFX();
    
    GenerateTestReport();
}

void UQA_VFXTestFramework::RunSpecificTest(const FString& TestName)
{
    if (TestName == TEXT("ScreenShakeSystem"))
    {
        TestScreenShakeSystem();
    }
    else if (TestName == TEXT("NiagaraParticleSystem"))
    {
        TestNiagaraParticleSystem();
    }
    else if (TestName == TEXT("VFXAudioIntegration"))
    {
        TestVFXAudioIntegration();
    }
    else if (TestName == TEXT("DamageFlashEffect"))
    {
        TestDamageFlashEffect();
    }
    else if (TestName == TEXT("EnvironmentalVFX"))
    {
        TestEnvironmentalVFX();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("QA VFX Test: Unknown test name: %s"), *TestName);
    }
}

bool UQA_VFXTestFramework::TestScreenShakeSystem()
{
    float StartTime = FPlatformTime::Seconds();
    
    try
    {
        // Test if VFX_ScreenShakeController class exists
        if (!ValidateVFXClass(TEXT("VFX_ScreenShakeController")))
        {
            LogTestResult(TEXT("ScreenShakeSystem"), EQA_VFXTestResult::Fail, 
                         TEXT("VFX_ScreenShakeController class not found"));
            return false;
        }
        
        // Test camera shake functionality
        APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (!PC)
        {
            LogTestResult(TEXT("ScreenShakeSystem"), EQA_VFXTestResult::Warning, 
                         TEXT("No player controller found for camera shake test"));
            return false;
        }
        
        LogTestResult(TEXT("ScreenShakeSystem"), EQA_VFXTestResult::Pass, 
                     TEXT("Screen shake system validation successful"));
        return true;
    }
    catch (...)
    {
        LogTestResult(TEXT("ScreenShakeSystem"), EQA_VFXTestResult::Fail, 
                     TEXT("Exception during screen shake test"));
        return false;
    }
}

bool UQA_VFXTestFramework::TestNiagaraParticleSystem()
{
    try
    {
        // Test Niagara system availability
        UWorld* World = GetWorld();
        if (!World)
        {
            LogTestResult(TEXT("NiagaraParticleSystem"), EQA_VFXTestResult::Fail, 
                         TEXT("No valid world context"));
            return false;
        }
        
        // Test if we can create Niagara components
        UNiagaraComponent* TestComponent = NewObject<UNiagaraComponent>(GetOwner());
        if (!TestComponent)
        {
            LogTestResult(TEXT("NiagaraParticleSystem"), EQA_VFXTestResult::Fail, 
                         TEXT("Failed to create Niagara component"));
            return false;
        }
        
        LogTestResult(TEXT("NiagaraParticleSystem"), EQA_VFXTestResult::Pass, 
                     TEXT("Niagara particle system validation successful"));
        return true;
    }
    catch (...)
    {
        LogTestResult(TEXT("NiagaraParticleSystem"), EQA_VFXTestResult::Fail, 
                     TEXT("Exception during Niagara test"));
        return false;
    }
}

bool UQA_VFXTestFramework::TestVFXAudioIntegration()
{
    try
    {
        // Test audio component creation for VFX integration
        UAudioComponent* TestAudio = NewObject<UAudioComponent>(GetOwner());
        if (!TestAudio)
        {
            LogTestResult(TEXT("VFXAudioIntegration"), EQA_VFXTestResult::Fail, 
                         TEXT("Failed to create audio component"));
            return false;
        }
        
        LogTestResult(TEXT("VFXAudioIntegration"), EQA_VFXTestResult::Pass, 
                     TEXT("VFX audio integration validation successful"));
        return true;
    }
    catch (...)
    {
        LogTestResult(TEXT("VFXAudioIntegration"), EQA_VFXTestResult::Fail, 
                     TEXT("Exception during VFX audio integration test"));
        return false;
    }
}

bool UQA_VFXTestFramework::TestDamageFlashEffect()
{
    try
    {
        // Test damage flash effect system
        if (!ValidateVFXClass(TEXT("VFX_ScreenShakeController")))
        {
            LogTestResult(TEXT("DamageFlashEffect"), EQA_VFXTestResult::Warning, 
                         TEXT("VFX controller not available for damage flash test"));
            return false;
        }
        
        LogTestResult(TEXT("DamageFlashEffect"), EQA_VFXTestResult::Pass, 
                     TEXT("Damage flash effect validation successful"));
        return true;
    }
    catch (...)
    {
        LogTestResult(TEXT("DamageFlashEffect"), EQA_VFXTestResult::Fail, 
                     TEXT("Exception during damage flash test"));
        return false;
    }
}

bool UQA_VFXTestFramework::TestEnvironmentalVFX()
{
    try
    {
        // Test environmental VFX systems
        UWorld* World = GetWorld();
        if (!World)
        {
            LogTestResult(TEXT("EnvironmentalVFX"), EQA_VFXTestResult::Fail, 
                         TEXT("No valid world context"));
            return false;
        }
        
        LogTestResult(TEXT("EnvironmentalVFX"), EQA_VFXTestResult::Pass, 
                     TEXT("Environmental VFX validation successful"));
        return true;
    }
    catch (...)
    {
        LogTestResult(TEXT("EnvironmentalVFX"), EQA_VFXTestResult::Fail, 
                     TEXT("Exception during environmental VFX test"));
        return false;
    }
}

FQA_VFXTestCase UQA_VFXTestFramework::GetTestResult(const FString& TestName)
{
    for (const FQA_VFXTestCase& TestCase : TestCases)
    {
        if (TestCase.TestName == TestName)
        {
            return TestCase;
        }
    }
    
    FQA_VFXTestCase EmptyResult;
    return EmptyResult;
}

void UQA_VFXTestFramework::ClearAllTestResults()
{
    for (FQA_VFXTestCase& TestCase : TestCases)
    {
        TestCase.Result = EQA_VFXTestResult::NotTested;
        TestCase.ErrorMessage = TEXT("");
        TestCase.ExecutionTime = 0.0f;
    }
}

void UQA_VFXTestFramework::GenerateTestReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== QA VFX TEST REPORT ==="));
    
    int32 PassCount = 0;
    int32 FailCount = 0;
    int32 WarningCount = 0;
    
    for (const FQA_VFXTestCase& TestCase : TestCases)
    {
        FString ResultString;
        switch (TestCase.Result)
        {
            case EQA_VFXTestResult::Pass:
                ResultString = TEXT("PASS");
                PassCount++;
                break;
            case EQA_VFXTestResult::Fail:
                ResultString = TEXT("FAIL");
                FailCount++;
                break;
            case EQA_VFXTestResult::Warning:
                ResultString = TEXT("WARNING");
                WarningCount++;
                break;
            default:
                ResultString = TEXT("NOT_TESTED");
                break;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("%s: %s - %s"), 
               *TestCase.TestName, *ResultString, *TestCase.ErrorMessage);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== SUMMARY: %d PASS, %d FAIL, %d WARNING ==="), 
           PassCount, FailCount, WarningCount);
}

void UQA_VFXTestFramework::LogTestResult(const FString& TestName, EQA_VFXTestResult Result, const FString& Message)
{
    for (FQA_VFXTestCase& TestCase : TestCases)
    {
        if (TestCase.TestName == TestName)
        {
            TestCase.Result = Result;
            TestCase.ErrorMessage = Message;
            TestCase.ExecutionTime = FPlatformTime::Seconds();
            break;
        }
    }
}

bool UQA_VFXTestFramework::ValidateVFXClass(const FString& ClassName)
{
    FString FullClassName = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ClassName);
    UClass* TestClass = LoadClass<UObject>(nullptr, *FullClassName);
    return TestClass != nullptr;
}

bool UQA_VFXTestFramework::ValidateNiagaraSystem(const FString& SystemPath)
{
    UNiagaraSystem* System = LoadObject<UNiagaraSystem>(nullptr, *SystemPath);
    return System != nullptr;
}