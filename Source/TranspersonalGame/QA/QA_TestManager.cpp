#include "QA_TestManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/Character.h"
#include "../TranspersonalGameMode.h"
#include "../TranspersonalCharacter.h"
#include "../TranspersonalGameState.h"

AQA_TestManager::AQA_TestManager()
{
    PrimaryActorTick.bCanEverTick = false;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    bAutoRunOnBeginPlay = true;
    TestInterval = 5.0f;
    PassedTests = 0;
    FailedTests = 0;
    WarningTests = 0;

    // Initialize default test cases
    AddTestCase(TEXT("GameMode_Validation"), TEXT("Validate TranspersonalGameMode is active"));
    AddTestCase(TEXT("Character_Validation"), TEXT("Validate TranspersonalCharacter functionality"));
    AddTestCase(TEXT("GameState_Validation"), TEXT("Validate TranspersonalGameState properties"));
    AddTestCase(TEXT("World_Generation"), TEXT("Validate world generation systems"));
    AddTestCase(TEXT("VFX_Systems"), TEXT("Validate VFX and particle systems"));
}

void AQA_TestManager::BeginPlay()
{
    Super::BeginPlay();

    if (bAutoRunOnBeginPlay)
    {
        GetWorldTimerManager().SetTimer(TestTimerHandle, this, &AQA_TestManager::RunAllTests, TestInterval, false);
    }
}

void AQA_TestManager::RunAllTests()
{
    UE_LOG(LogTemp, Warning, TEXT("QA_TestManager: Starting comprehensive test suite"));
    
    ClearTestResults();
    
    for (FQA_TestCase& TestCase : TestCases)
    {
        ExecuteTest(TestCase);
    }
    
    UpdateTestStatistics();
    GenerateTestReport();
}

void AQA_TestManager::RunSingleTest(const FString& TestName)
{
    for (FQA_TestCase& TestCase : TestCases)
    {
        if (TestCase.TestName == TestName)
        {
            ExecuteTest(TestCase);
            LogTestResult(TestCase);
            return;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("QA_TestManager: Test '%s' not found"), *TestName);
}

void AQA_TestManager::ClearTestResults()
{
    for (FQA_TestCase& TestCase : TestCases)
    {
        TestCase.Result = EQA_TestResult::NotRun;
        TestCase.ErrorMessage = TEXT("");
        TestCase.ExecutionTime = 0.0f;
    }
    
    PassedTests = 0;
    FailedTests = 0;
    WarningTests = 0;
}

FQA_TestCase AQA_TestManager::GetTestResult(const FString& TestName)
{
    for (const FQA_TestCase& TestCase : TestCases)
    {
        if (TestCase.TestName == TestName)
        {
            return TestCase;
        }
    }
    
    return FQA_TestCase();
}

TArray<FQA_TestCase> AQA_TestManager::GetAllTestResults()
{
    return TestCases;
}

void AQA_TestManager::AddTestCase(const FString& Name, const FString& Description)
{
    FQA_TestCase NewTest;
    NewTest.TestName = Name;
    NewTest.TestDescription = Description;
    NewTest.Result = EQA_TestResult::NotRun;
    
    TestCases.Add(NewTest);
}

bool AQA_TestManager::ValidateGameMode()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    AGameModeBase* GameMode = World->GetAuthGameMode();
    if (!GameMode)
    {
        return false;
    }
    
    ATranspersonalGameMode* TPGameMode = Cast<ATranspersonalGameMode>(GameMode);
    return TPGameMode != nullptr;
}

bool AQA_TestManager::ValidateCharacterSystems()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Check if TranspersonalCharacter class exists and can be spawned
    UClass* CharacterClass = ATranspersonalCharacter::StaticClass();
    if (!CharacterClass)
    {
        return false;
    }
    
    // Test spawn location
    FVector TestLocation = GetActorLocation() + FVector(100, 0, 0);
    FRotator TestRotation = FRotator::ZeroRotator;
    
    ATranspersonalCharacter* TestCharacter = World->SpawnActor<ATranspersonalCharacter>(CharacterClass, TestLocation, TestRotation);
    if (TestCharacter)
    {
        // Clean up test character
        TestCharacter->Destroy();
        return true;
    }
    
    return false;
}

bool AQA_TestManager::ValidateWorldGeneration()
{
    // Check if world generation components are present
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Look for world generation actors in the level
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetName().Contains(TEXT("WorldGen")))
        {
            return true;
        }
    }
    
    return false;
}

bool AQA_TestManager::ValidateVFXSystems()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Check for VFX-related actors
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && (Actor->GetName().Contains(TEXT("VFX")) || Actor->GetName().Contains(TEXT("Campfire"))))
        {
            return true;
        }
    }
    
    return false;
}

bool AQA_TestManager::ValidateAudioSystems()
{
    // Basic audio system validation
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Check if audio components exist in the world
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->FindComponentByClass<UAudioComponent>())
        {
            return true;
        }
    }
    
    return false;
}

void AQA_TestManager::GenerateTestReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== QA TEST REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Total Tests: %d"), TestCases.Num());
    UE_LOG(LogTemp, Warning, TEXT("Passed: %d"), PassedTests);
    UE_LOG(LogTemp, Warning, TEXT("Failed: %d"), FailedTests);
    UE_LOG(LogTemp, Warning, TEXT("Warnings: %d"), WarningTests);
    
    for (const FQA_TestCase& TestCase : TestCases)
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
                ResultString = TEXT("NOT_RUN");
                break;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("[%s] %s - %s"), *ResultString, *TestCase.TestName, *TestCase.TestDescription);
        if (!TestCase.ErrorMessage.IsEmpty())
        {
            UE_LOG(LogTemp, Warning, TEXT("    Error: %s"), *TestCase.ErrorMessage);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== END REPORT ==="));
}

void AQA_TestManager::ExecuteTest(FQA_TestCase& TestCase)
{
    double StartTime = FPlatformTime::Seconds();
    
    bool bTestPassed = false;
    FString ErrorMsg = TEXT("");
    
    try
    {
        if (TestCase.TestName == TEXT("GameMode_Validation"))
        {
            bTestPassed = ValidateGameMode();
            if (!bTestPassed)
            {
                ErrorMsg = TEXT("TranspersonalGameMode not found or not active");
            }
        }
        else if (TestCase.TestName == TEXT("Character_Validation"))
        {
            bTestPassed = ValidateCharacterSystems();
            if (!bTestPassed)
            {
                ErrorMsg = TEXT("TranspersonalCharacter validation failed");
            }
        }
        else if (TestCase.TestName == TEXT("GameState_Validation"))
        {
            UWorld* World = GetWorld();
            ATranspersonalGameState* GameState = World ? World->GetGameState<ATranspersonalGameState>() : nullptr;
            bTestPassed = (GameState != nullptr);
            if (!bTestPassed)
            {
                ErrorMsg = TEXT("TranspersonalGameState not found");
            }
        }
        else if (TestCase.TestName == TEXT("World_Generation"))
        {
            bTestPassed = ValidateWorldGeneration();
            if (!bTestPassed)
            {
                ErrorMsg = TEXT("World generation systems not detected");
            }
        }
        else if (TestCase.TestName == TEXT("VFX_Systems"))
        {
            bTestPassed = ValidateVFXSystems();
            if (!bTestPassed)
            {
                ErrorMsg = TEXT("VFX systems not detected");
            }
        }
    }
    catch (...)
    {
        bTestPassed = false;
        ErrorMsg = TEXT("Exception occurred during test execution");
    }
    
    double EndTime = FPlatformTime::Seconds();
    TestCase.ExecutionTime = EndTime - StartTime;
    TestCase.Result = bTestPassed ? EQA_TestResult::Pass : EQA_TestResult::Fail;
    TestCase.ErrorMessage = ErrorMsg;
    
    LogTestResult(TestCase);
}

void AQA_TestManager::LogTestResult(const FQA_TestCase& TestCase)
{
    FString ResultString = (TestCase.Result == EQA_TestResult::Pass) ? TEXT("PASS") : TEXT("FAIL");
    UE_LOG(LogTemp, Warning, TEXT("QA Test [%s]: %s (%.3fs)"), *ResultString, *TestCase.TestName, TestCase.ExecutionTime);
    
    if (!TestCase.ErrorMessage.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("  Error: %s"), *TestCase.ErrorMessage);
    }
}

void AQA_TestManager::UpdateTestStatistics()
{
    PassedTests = 0;
    FailedTests = 0;
    WarningTests = 0;
    
    for (const FQA_TestCase& TestCase : TestCases)
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
        }
    }
}