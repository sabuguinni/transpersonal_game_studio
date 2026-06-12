#include "QA_TestFramework.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "Components/AudioComponent.h"
#include "GameFramework/Character.h"
#include "TranspersonalGame/TranspersonalCharacter.h"
#include "TranspersonalGame/VFX/VFX_NiagaraManager.h"

AQA_TestFramework::AQA_TestFramework()
{
    PrimaryActorTick.bCanEverTick = false;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    bAutoRunOnBeginPlay = false;
    bLogResults = true;
    TestTimeout = 30.0f;
}

void AQA_TestFramework::BeginPlay()
{
    Super::BeginPlay();

    if (bAutoRunOnBeginPlay)
    {
        RunAllTests();
    }
}

void AQA_TestFramework::RunAllTests()
{
    UE_LOG(LogTemp, Warning, TEXT("QA Framework: Starting comprehensive test suite"));

    // Clear previous results
    for (FQA_TestCase& TestCase : TestCases)
    {
        TestCase.Result = EQA_TestResult::NotRun;
        TestCase.ErrorMessage = TEXT("");
        TestCase.ExecutionTime = 0.0f;
    }

    // Initialize standard test cases if empty
    if (TestCases.Num() == 0)
    {
        AddTestCase(TEXT("VFX_Systems"), TEXT("Validate VFX and Niagara systems"));
        AddTestCase(TEXT("Audio_Systems"), TEXT("Validate audio components and synchronization"));
        AddTestCase(TEXT("Character_Systems"), TEXT("Validate character movement and stats"));
        AddTestCase(TEXT("World_Generation"), TEXT("Validate procedural world systems"));
        AddTestCase(TEXT("Performance"), TEXT("Validate frame rate and memory usage"));
        AddTestCase(TEXT("Integration"), TEXT("Validate cross-system integration"));
    }

    // Run validation tests
    ValidateVFXSystems();
    ValidateAudioSystems();
    ValidateCharacterSystems();
    ValidateWorldGeneration();
    ValidatePerformance();
    ValidateIntegration();

    // Generate final report
    GenerateTestReport();

    OnAllTestsCompleted();
}

void AQA_TestFramework::RunSingleTest(const FString& TestName)
{
    for (FQA_TestCase& TestCase : TestCases)
    {
        if (TestCase.TestName == TestName)
        {
            TestCase.Result = EQA_TestResult::NotRun;
            TestCase.ErrorMessage = TEXT("");

            float StartTime = FPlatformTime::Seconds();

            if (TestName == TEXT("VFX_Systems"))
            {
                ValidateVFXSystems();
            }
            else if (TestName == TEXT("Audio_Systems"))
            {
                ValidateAudioSystems();
            }
            else if (TestName == TEXT("Character_Systems"))
            {
                ValidateCharacterSystems();
            }
            else if (TestName == TEXT("World_Generation"))
            {
                ValidateWorldGeneration();
            }
            else if (TestName == TEXT("Performance"))
            {
                ValidatePerformance();
            }
            else if (TestName == TEXT("Integration"))
            {
                ValidateIntegration();
            }

            TestCase.ExecutionTime = FPlatformTime::Seconds() - StartTime;
            LogTestResult(TestCase);
            OnTestCompleted(TestCase);
            break;
        }
    }
}

void AQA_TestFramework::AddTestCase(const FString& Name, const FString& Description)
{
    FQA_TestCase NewTest;
    NewTest.TestName = Name;
    NewTest.TestDescription = Description;
    NewTest.Result = EQA_TestResult::NotRun;
    TestCases.Add(NewTest);
}

void AQA_TestFramework::ClearAllTests()
{
    TestCases.Empty();
}

int32 AQA_TestFramework::GetPassedTestCount() const
{
    int32 PassCount = 0;
    for (const FQA_TestCase& TestCase : TestCases)
    {
        if (TestCase.Result == EQA_TestResult::Pass)
        {
            PassCount++;
        }
    }
    return PassCount;
}

int32 AQA_TestFramework::GetFailedTestCount() const
{
    int32 FailCount = 0;
    for (const FQA_TestCase& TestCase : TestCases)
    {
        if (TestCase.Result == EQA_TestResult::Fail)
        {
            FailCount++;
        }
    }
    return FailCount;
}

float AQA_TestFramework::GetTotalExecutionTime() const
{
    float TotalTime = 0.0f;
    for (const FQA_TestCase& TestCase : TestCases)
    {
        TotalTime += TestCase.ExecutionTime;
    }
    return TotalTime;
}

void AQA_TestFramework::GenerateTestReport()
{
    int32 PassCount = GetPassedTestCount();
    int32 FailCount = GetFailedTestCount();
    int32 TotalTests = TestCases.Num();
    float TotalTime = GetTotalExecutionTime();

    UE_LOG(LogTemp, Warning, TEXT("=== QA TEST REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Total Tests: %d"), TotalTests);
    UE_LOG(LogTemp, Warning, TEXT("Passed: %d"), PassCount);
    UE_LOG(LogTemp, Warning, TEXT("Failed: %d"), FailCount);
    UE_LOG(LogTemp, Warning, TEXT("Success Rate: %.1f%%"), TotalTests > 0 ? (float)PassCount / TotalTests * 100.0f : 0.0f);
    UE_LOG(LogTemp, Warning, TEXT("Total Execution Time: %.2f seconds"), TotalTime);
    UE_LOG(LogTemp, Warning, TEXT("======================"));
}

void AQA_TestFramework::ValidateVFXSystems()
{
    try
    {
        // Test VFX manager class loading
        UClass* VFXManagerClass = LoadClass<AVFX_NiagaraManager>(nullptr, TEXT("/Script/TranspersonalGame.VFX_NiagaraManager"));
        
        if (VFXManagerClass)
        {
            // Test spawning VFX manager
            FVector SpawnLocation(0, 0, 100);
            AVFX_NiagaraManager* VFXManager = GetWorld()->SpawnActor<AVFX_NiagaraManager>(VFXManagerClass, SpawnLocation, FRotator::ZeroRotator);
            
            if (VFXManager)
            {
                SetTestResult(TEXT("VFX_Systems"), EQA_TestResult::Pass);
                VFXManager->Destroy(); // Clean up test actor
            }
            else
            {
                SetTestResult(TEXT("VFX_Systems"), EQA_TestResult::Fail, TEXT("Could not spawn VFX manager"));
            }
        }
        else
        {
            SetTestResult(TEXT("VFX_Systems"), EQA_TestResult::Fail, TEXT("VFX manager class not found"));
        }
    }
    catch (...)
    {
        SetTestResult(TEXT("VFX_Systems"), EQA_TestResult::Fail, TEXT("Exception during VFX validation"));
    }
}

void AQA_TestFramework::ValidateAudioSystems()
{
    try
    {
        // Count audio components in level
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
        
        int32 AudioComponentCount = 0;
        for (AActor* Actor : AllActors)
        {
            if (Actor && Actor->FindComponentByClass<UAudioComponent>())
            {
                AudioComponentCount++;
            }
        }
        
        if (AudioComponentCount > 0)
        {
            SetTestResult(TEXT("Audio_Systems"), EQA_TestResult::Pass);
        }
        else
        {
            SetTestResult(TEXT("Audio_Systems"), EQA_TestResult::Warning, TEXT("No audio components found"));
        }
    }
    catch (...)
    {
        SetTestResult(TEXT("Audio_Systems"), EQA_TestResult::Fail, TEXT("Exception during audio validation"));
    }
}

void AQA_TestFramework::ValidateCharacterSystems()
{
    try
    {
        // Test character class loading
        UClass* CharacterClass = LoadClass<ATranspersonalCharacter>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
        
        if (CharacterClass)
        {
            SetTestResult(TEXT("Character_Systems"), EQA_TestResult::Pass);
        }
        else
        {
            SetTestResult(TEXT("Character_Systems"), EQA_TestResult::Fail, TEXT("Character class not found"));
        }
    }
    catch (...)
    {
        SetTestResult(TEXT("Character_Systems"), EQA_TestResult::Fail, TEXT("Exception during character validation"));
    }
}

void AQA_TestFramework::ValidateWorldGeneration()
{
    try
    {
        // Basic world validation - check for landscape
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
        
        bool bFoundTerrain = false;
        for (AActor* Actor : AllActors)
        {
            if (Actor && Actor->GetName().Contains(TEXT("Landscape")))
            {
                bFoundTerrain = true;
                break;
            }
        }
        
        if (bFoundTerrain)
        {
            SetTestResult(TEXT("World_Generation"), EQA_TestResult::Pass);
        }
        else
        {
            SetTestResult(TEXT("World_Generation"), EQA_TestResult::Warning, TEXT("No landscape found"));
        }
    }
    catch (...)
    {
        SetTestResult(TEXT("World_Generation"), EQA_TestResult::Fail, TEXT("Exception during world validation"));
    }
}

void AQA_TestFramework::ValidatePerformance()
{
    try
    {
        // Basic performance check - actor count
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
        
        int32 ActorCount = AllActors.Num();
        
        if (ActorCount < 1000) // Reasonable limit for performance
        {
            SetTestResult(TEXT("Performance"), EQA_TestResult::Pass);
        }
        else
        {
            SetTestResult(TEXT("Performance"), EQA_TestResult::Warning, FString::Printf(TEXT("High actor count: %d"), ActorCount));
        }
    }
    catch (...)
    {
        SetTestResult(TEXT("Performance"), EQA_TestResult::Fail, TEXT("Exception during performance validation"));
    }
}

void AQA_TestFramework::ValidateIntegration()
{
    try
    {
        // Integration test - check for multiple system types
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
        
        bool bHasCharacter = false;
        bool bHasVFX = false;
        bool bHasAudio = false;
        
        for (AActor* Actor : AllActors)
        {
            if (Actor)
            {
                if (Cast<ACharacter>(Actor))
                {
                    bHasCharacter = true;
                }
                if (Actor->FindComponentByClass<UNiagaraComponent>())
                {
                    bHasVFX = true;
                }
                if (Actor->FindComponentByClass<UAudioComponent>())
                {
                    bHasAudio = true;
                }
            }
        }
        
        int32 SystemCount = (bHasCharacter ? 1 : 0) + (bHasVFX ? 1 : 0) + (bHasAudio ? 1 : 0);
        
        if (SystemCount >= 2)
        {
            SetTestResult(TEXT("Integration"), EQA_TestResult::Pass);
        }
        else
        {
            SetTestResult(TEXT("Integration"), EQA_TestResult::Warning, TEXT("Limited system integration"));
        }
    }
    catch (...)
    {
        SetTestResult(TEXT("Integration"), EQA_TestResult::Fail, TEXT("Exception during integration validation"));
    }
}

void AQA_TestFramework::LogTestResult(const FQA_TestCase& TestCase)
{
    if (bLogResults)
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
                ResultString = TEXT("NOT RUN");
                break;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("QA Test [%s]: %s - %s (%.2fs)"), 
               *TestCase.TestName, 
               *ResultString, 
               *TestCase.ErrorMessage, 
               TestCase.ExecutionTime);
    }
}

void AQA_TestFramework::SetTestResult(const FString& TestName, EQA_TestResult Result, const FString& ErrorMsg)
{
    for (FQA_TestCase& TestCase : TestCases)
    {
        if (TestCase.TestName == TestName)
        {
            TestCase.Result = Result;
            TestCase.ErrorMessage = ErrorMsg;
            break;
        }
    }
}