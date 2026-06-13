#include "QA_TestFramework.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "../TranspersonalGameState.h"
#include "../TranspersonalCharacter.h"
#include "../PCGWorldGenerator.h"
#include "../FoliageManager.h"
#include "../VFX/VFX_ParticleManager.h"

AQA_TestFramework::AQA_TestFramework()
{
    PrimaryActorTick.bCanEverTick = true;
    
    bAutoRunTestsOnBeginPlay = false;
    TestExecutionInterval = 30.0f;
    MaxActorCountThreshold = 8000;
    MaxDinosaurCountThreshold = 150;
    LastTestExecutionTime = 0.0f;
}

void AQA_TestFramework::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoRunTestsOnBeginPlay)
    {
        RunAllTests();
    }
}

void AQA_TestFramework::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    LastTestExecutionTime += DeltaTime;
    
    if (LastTestExecutionTime >= TestExecutionInterval)
    {
        RunPerformanceTests();
        LastTestExecutionTime = 0.0f;
    }
}

void AQA_TestFramework::RunAllTests()
{
    ClearTestResults();
    
    UE_LOG(LogTemp, Warning, TEXT("QA_TestFramework: Starting comprehensive test suite"));
    
    RunSystemValidationTests();
    RunPerformanceTests();
    RunIntegrationTests();
    RunVFXValidationTests();
    
    UE_LOG(LogTemp, Warning, TEXT("QA_TestFramework: Test suite complete - %d passed, %d failed"), 
           GetPassedTestCount(), GetFailedTestCount());
}

void AQA_TestFramework::RunSystemValidationTests()
{
    TestCoreSystemsRegistration();
    TestActorSpawning();
    TestGameStateValidation();
    TestWorldGenerationSystems();
}

void AQA_TestFramework::RunPerformanceTests()
{
    TestPerformanceThresholds();
}

void AQA_TestFramework::RunIntegrationTests()
{
    FQA_TestCase IntegrationTest;
    IntegrationTest.TestName = TEXT("System Integration Test");
    IntegrationTest.TestDescription = TEXT("Validate cross-system communication and dependencies");
    
    float StartTime = FPlatformTime::Seconds();
    
    try
    {
        // Test GameState integration
        ATranspersonalGameState* GameState = Cast<ATranspersonalGameState>(GetWorld()->GetGameState());
        if (GameState)
        {
            IntegrationTest.Result = EQA_TestResult::Pass;
            IntegrationTest.ErrorMessage = TEXT("GameState integration validated");
        }
        else
        {
            IntegrationTest.Result = EQA_TestResult::Warning;
            IntegrationTest.ErrorMessage = TEXT("GameState not found in current world");
        }
    }
    catch (...)
    {
        IntegrationTest.Result = EQA_TestResult::Fail;
        IntegrationTest.ErrorMessage = TEXT("Integration test exception occurred");
    }
    
    IntegrationTest.ExecutionTime = FPlatformTime::Seconds() - StartTime;
    AddTestResult(IntegrationTest);
}

void AQA_TestFramework::RunVFXValidationTests()
{
    TestVFXSystemIntegration();
}

void AQA_TestFramework::AddTestResult(const FQA_TestCase& TestCase)
{
    TestResults.Add(TestCase);
    
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
            ResultString = TEXT("PENDING");
            break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("QA Test [%s]: %s - %s (%.3fs)"), 
           *ResultString, *TestCase.TestName, *TestCase.ErrorMessage, TestCase.ExecutionTime);
}

void AQA_TestFramework::ClearTestResults()
{
    TestResults.Empty();
}

TArray<FQA_TestCase> AQA_TestFramework::GetTestResults() const
{
    return TestResults;
}

int32 AQA_TestFramework::GetPassedTestCount() const
{
    return TestResults.FilterByPredicate([](const FQA_TestCase& Test) {
        return Test.Result == EQA_TestResult::Pass;
    }).Num();
}

int32 AQA_TestFramework::GetFailedTestCount() const
{
    return TestResults.FilterByPredicate([](const FQA_TestCase& Test) {
        return Test.Result == EQA_TestResult::Fail;
    }).Num();
}

float AQA_TestFramework::GetOverallSuccessRate() const
{
    if (TestResults.Num() == 0)
        return 0.0f;
    
    return (float)GetPassedTestCount() / (float)TestResults.Num() * 100.0f;
}

bool AQA_TestFramework::ValidateActorCount(int32 MaxActors)
{
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    return AllActors.Num() <= MaxActors;
}

bool AQA_TestFramework::ValidateDinosaurCount(int32 MaxDinosaurs)
{
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    int32 DinosaurCount = 0;
    TArray<FString> DinosaurLabels = {TEXT("trex"), TEXT("veloci"), TEXT("tricera"), TEXT("brachi"), 
                                     TEXT("ankylo"), TEXT("parasauro"), TEXT("pachy"), TEXT("proto")};
    
    for (AActor* Actor : AllActors)
    {
        FString ActorLabel = Actor->GetActorLabel().ToLower();
        for (const FString& DinoLabel : DinosaurLabels)
        {
            if (ActorLabel.Contains(DinoLabel))
            {
                DinosaurCount++;
                break;
            }
        }
    }
    
    return DinosaurCount <= MaxDinosaurs;
}

bool AQA_TestFramework::ValidateClassRegistration(const FString& ClassName)
{
    FString FullClassName = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ClassName);
    UClass* TestClass = LoadClass<UObject>(nullptr, *FullClassName);
    
    return TestClass != nullptr;
}

bool AQA_TestFramework::ValidateActorSpawning(UClass* ActorClass)
{
    if (!ActorClass)
        return false;
    
    FVector SpawnLocation = GetActorLocation() + FVector(100, 100, 100);
    AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(ActorClass, SpawnLocation, FRotator::ZeroRotator);
    
    if (SpawnedActor)
    {
        SpawnedActor->Destroy();
        return true;
    }
    
    return false;
}

void AQA_TestFramework::TestCoreSystemsRegistration()
{
    TArray<FString> CoreClasses = {
        TEXT("TranspersonalGameState"),
        TEXT("TranspersonalCharacter"),
        TEXT("PCGWorldGenerator"),
        TEXT("FoliageManager"),
        TEXT("VFX_ParticleManager")
    };
    
    for (const FString& ClassName : CoreClasses)
    {
        FQA_TestCase Test;
        Test.TestName = FString::Printf(TEXT("Class Registration: %s"), *ClassName);
        Test.TestDescription = FString::Printf(TEXT("Validate %s class is registered and loadable"), *ClassName);
        
        float StartTime = FPlatformTime::Seconds();
        
        if (ValidateClassRegistration(ClassName))
        {
            Test.Result = EQA_TestResult::Pass;
            Test.ErrorMessage = TEXT("Class registered successfully");
        }
        else
        {
            Test.Result = EQA_TestResult::Fail;
            Test.ErrorMessage = TEXT("Class not found or failed to load");
        }
        
        Test.ExecutionTime = FPlatformTime::Seconds() - StartTime;
        AddTestResult(Test);
    }
}

void AQA_TestFramework::TestActorSpawning()
{
    FQA_TestCase Test;
    Test.TestName = TEXT("Actor Spawning Test");
    Test.TestDescription = TEXT("Validate core actors can be spawned without errors");
    
    float StartTime = FPlatformTime::Seconds();
    
    UClass* CharacterClass = LoadClass<AActor>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
    
    if (ValidateActorSpawning(CharacterClass))
    {
        Test.Result = EQA_TestResult::Pass;
        Test.ErrorMessage = TEXT("Actor spawning successful");
    }
    else
    {
        Test.Result = EQA_TestResult::Fail;
        Test.ErrorMessage = TEXT("Actor spawning failed");
    }
    
    Test.ExecutionTime = FPlatformTime::Seconds() - StartTime;
    AddTestResult(Test);
}

void AQA_TestFramework::TestPerformanceThresholds()
{
    FQA_TestCase Test;
    Test.TestName = TEXT("Performance Thresholds");
    Test.TestDescription = TEXT("Validate actor counts are within performance limits");
    
    float StartTime = FPlatformTime::Seconds();
    
    bool ActorCountValid = ValidateActorCount(MaxActorCountThreshold);
    bool DinosaurCountValid = ValidateDinosaurCount(MaxDinosaurCountThreshold);
    
    if (ActorCountValid && DinosaurCountValid)
    {
        Test.Result = EQA_TestResult::Pass;
        Test.ErrorMessage = TEXT("All performance thresholds within limits");
    }
    else if (!ActorCountValid)
    {
        Test.Result = EQA_TestResult::Warning;
        Test.ErrorMessage = TEXT("Actor count exceeds performance threshold");
    }
    else
    {
        Test.Result = EQA_TestResult::Warning;
        Test.ErrorMessage = TEXT("Dinosaur count exceeds performance threshold");
    }
    
    Test.ExecutionTime = FPlatformTime::Seconds() - StartTime;
    AddTestResult(Test);
}

void AQA_TestFramework::TestVFXSystemIntegration()
{
    FQA_TestCase Test;
    Test.TestName = TEXT("VFX System Integration");
    Test.TestDescription = TEXT("Validate VFX systems are properly integrated and functional");
    
    float StartTime = FPlatformTime::Seconds();
    
    UClass* VFXClass = LoadClass<AActor>(nullptr, TEXT("/Script/TranspersonalGame.VFX_ParticleManager"));
    
    if (VFXClass && ValidateActorSpawning(VFXClass))
    {
        Test.Result = EQA_TestResult::Pass;
        Test.ErrorMessage = TEXT("VFX system integration validated");
    }
    else
    {
        Test.Result = EQA_TestResult::Fail;
        Test.ErrorMessage = TEXT("VFX system integration failed");
    }
    
    Test.ExecutionTime = FPlatformTime::Seconds() - StartTime;
    AddTestResult(Test);
}

void AQA_TestFramework::TestGameStateValidation()
{
    FQA_TestCase Test;
    Test.TestName = TEXT("GameState Validation");
    Test.TestDescription = TEXT("Validate game state systems are functional");
    
    float StartTime = FPlatformTime::Seconds();
    
    AGameStateBase* GameState = GetWorld()->GetGameState();
    
    if (GameState)
    {
        Test.Result = EQA_TestResult::Pass;
        Test.ErrorMessage = TEXT("GameState found and accessible");
    }
    else
    {
        Test.Result = EQA_TestResult::Warning;
        Test.ErrorMessage = TEXT("GameState not found in current world");
    }
    
    Test.ExecutionTime = FPlatformTime::Seconds() - StartTime;
    AddTestResult(Test);
}

void AQA_TestFramework::TestWorldGenerationSystems()
{
    FQA_TestCase Test;
    Test.TestName = TEXT("World Generation Systems");
    Test.TestDescription = TEXT("Validate world generation and foliage systems");
    
    float StartTime = FPlatformTime::Seconds();
    
    bool PCGValid = ValidateClassRegistration(TEXT("PCGWorldGenerator"));
    bool FoliageValid = ValidateClassRegistration(TEXT("FoliageManager"));
    
    if (PCGValid && FoliageValid)
    {
        Test.Result = EQA_TestResult::Pass;
        Test.ErrorMessage = TEXT("World generation systems validated");
    }
    else
    {
        Test.Result = EQA_TestResult::Fail;
        Test.ErrorMessage = TEXT("World generation systems validation failed");
    }
    
    Test.ExecutionTime = FPlatformTime::Seconds() - StartTime;
    AddTestResult(Test);
}