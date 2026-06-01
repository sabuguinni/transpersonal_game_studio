#include "QA_TestManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "TranspersonalGame/Character/TranspersonalCharacter.h"
#include "TranspersonalGame/GameMode/TranspersonalGameMode.h"
#include "TranspersonalGame/GameState/TranspersonalGameState.h"

UQA_TestManager::UQA_TestManager()
{
    bTestsRunning = false;
    MaxActorLimit = 8000;
    MaxDinosaurLimit = 150;
    TargetFrameRate = 30.0f;
}

void UQA_TestManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("QA Test Manager initialized"));
    
    // Register default tests
    RegisterTest(TEXT("ClassLoading"), TEXT("Validate all TranspersonalGame classes can be loaded"));
    RegisterTest(TEXT("CDOConstruction"), TEXT("Validate Class Default Objects construct without errors"));
    RegisterTest(TEXT("ActorCounts"), TEXT("Validate actor counts are within limits"));
    RegisterTest(TEXT("Performance"), TEXT("Validate performance metrics meet targets"));
    RegisterTest(TEXT("CharacterSpawn"), TEXT("Validate character spawning works correctly"));
}

void UQA_TestManager::Deinitialize()
{
    ClearTestResults();
    Super::Deinitialize();
}

void UQA_TestManager::RunAllTests()
{
    if (bTestsRunning)
    {
        UE_LOG(LogTemp, Warning, TEXT("Tests already running, skipping"));
        return;
    }

    bTestsRunning = true;
    ClearTestResults();
    
    UE_LOG(LogTemp, Log, TEXT("Starting QA test suite"));

    // Core validation tests
    ExecuteTest(TEXT("ClassLoading"), [this]() { return ValidateClassLoading(); });
    ExecuteTest(TEXT("CDOConstruction"), [this]() { return ValidateCDOConstruction(); });
    ExecuteTest(TEXT("ActorCounts"), [this]() { return ValidateActorCounts(); });
    ExecuteTest(TEXT("Performance"), [this]() { return ValidatePerformanceMetrics(); });

    // Integration tests
    TestCharacterSpawning();
    TestPerformance();
    TestActorLimits();

    bTestsRunning = false;
    
    UE_LOG(LogTemp, Log, TEXT("QA test suite completed. Passed: %d, Failed: %d"), 
           GetPassedTestCount(), GetFailedTestCount());
}

void UQA_TestManager::RunTestByName(const FString& TestName)
{
    if (TestName == TEXT("ClassLoading"))
    {
        ExecuteTest(TestName, [this]() { return ValidateClassLoading(); });
    }
    else if (TestName == TEXT("CDOConstruction"))
    {
        ExecuteTest(TestName, [this]() { return ValidateCDOConstruction(); });
    }
    else if (TestName == TEXT("ActorCounts"))
    {
        ExecuteTest(TestName, [this]() { return ValidateActorCounts(); });
    }
    else if (TestName == TEXT("Performance"))
    {
        ExecuteTest(TestName, [this]() { return ValidatePerformanceMetrics(); });
    }
    else if (TestName == TEXT("CharacterSpawn"))
    {
        TestCharacterSpawning();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Unknown test: %s"), *TestName);
    }
}

void UQA_TestManager::ClearTestResults()
{
    TestResults.Empty();
}

void UQA_TestManager::RegisterTest(const FString& TestName, const FString& Description)
{
    FQA_TestCase NewTest;
    NewTest.TestName = TestName;
    NewTest.TestDescription = Description;
    NewTest.Result = EQA_TestResult::NotRun;
    
    TestResults.Add(NewTest);
}

int32 UQA_TestManager::GetPassedTestCount() const
{
    return TestResults.FilterByPredicate([](const FQA_TestCase& Test) {
        return Test.Result == EQA_TestResult::Pass;
    }).Num();
}

int32 UQA_TestManager::GetFailedTestCount() const
{
    return TestResults.FilterByPredicate([](const FQA_TestCase& Test) {
        return Test.Result == EQA_TestResult::Fail;
    }).Num();
}

float UQA_TestManager::GetOverallSuccessRate() const
{
    if (TestResults.Num() == 0) return 0.0f;
    
    return (float)GetPassedTestCount() / (float)TestResults.Num() * 100.0f;
}

void UQA_TestManager::TestPerformance()
{
    float StartTime = FPlatformTime::Seconds();
    
    UWorld* World = GetWorld();
    if (!World)
    {
        LogTestResult(TEXT("Performance"), EQA_TestResult::Fail, TEXT("No valid world"));
        return;
    }

    // Check frame rate
    float CurrentFPS = 1.0f / World->GetDeltaSeconds();
    bool bFrameRateOK = CurrentFPS >= TargetFrameRate;
    
    // Check actor count
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    bool bActorCountOK = AllActors.Num() <= MaxActorLimit;
    
    float ExecutionTime = FPlatformTime::Seconds() - StartTime;
    
    if (bFrameRateOK && bActorCountOK)
    {
        LogTestResult(TEXT("Performance"), EQA_TestResult::Pass, 
                     FString::Printf(TEXT("FPS: %.1f, Actors: %d"), CurrentFPS, AllActors.Num()));
    }
    else
    {
        LogTestResult(TEXT("Performance"), EQA_TestResult::Fail,
                     FString::Printf(TEXT("FPS: %.1f (target: %.1f), Actors: %d (limit: %d)"), 
                                   CurrentFPS, TargetFrameRate, AllActors.Num(), MaxActorLimit));
    }
}

void UQA_TestManager::TestActorLimits()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        LogTestResult(TEXT("ActorLimits"), EQA_TestResult::Fail, TEXT("No valid world"));
        return;
    }

    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    // Count dinosaurs
    int32 DinosaurCount = 0;
    for (AActor* Actor : AllActors)
    {
        FString ActorName = Actor->GetName().ToLower();
        if (ActorName.Contains(TEXT("trex")) || ActorName.Contains(TEXT("veloci")) || 
            ActorName.Contains(TEXT("tricera")) || ActorName.Contains(TEXT("brachi")))
        {
            DinosaurCount++;
        }
    }
    
    bool bWithinLimits = AllActors.Num() <= MaxActorLimit && DinosaurCount <= MaxDinosaurLimit;
    
    if (bWithinLimits)
    {
        LogTestResult(TEXT("ActorLimits"), EQA_TestResult::Pass,
                     FString::Printf(TEXT("Actors: %d/%d, Dinosaurs: %d/%d"), 
                                   AllActors.Num(), MaxActorLimit, DinosaurCount, MaxDinosaurLimit));
    }
    else
    {
        LogTestResult(TEXT("ActorLimits"), EQA_TestResult::Fail,
                     FString::Printf(TEXT("EXCEEDED - Actors: %d/%d, Dinosaurs: %d/%d"), 
                                   AllActors.Num(), MaxActorLimit, DinosaurCount, MaxDinosaurLimit));
    }
}

void UQA_TestManager::TestMemoryUsage()
{
    // Basic memory usage test
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    
    float UsedMemoryMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    float AvailableMemoryMB = MemStats.AvailablePhysical / (1024.0f * 1024.0f);
    
    // Consider memory usage acceptable if under 4GB
    bool bMemoryOK = UsedMemoryMB < 4096.0f;
    
    if (bMemoryOK)
    {
        LogTestResult(TEXT("MemoryUsage"), EQA_TestResult::Pass,
                     FString::Printf(TEXT("Used: %.1f MB, Available: %.1f MB"), UsedMemoryMB, AvailableMemoryMB));
    }
    else
    {
        LogTestResult(TEXT("MemoryUsage"), EQA_TestResult::Warning,
                     FString::Printf(TEXT("High usage: %.1f MB, Available: %.1f MB"), UsedMemoryMB, AvailableMemoryMB));
    }
}

void UQA_TestManager::TestCharacterSpawning()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        LogTestResult(TEXT("CharacterSpawn"), EQA_TestResult::Fail, TEXT("No valid world"));
        return;
    }

    // Try to spawn a test character
    FVector SpawnLocation(1000.0f, 1000.0f, 200.0f);
    FRotator SpawnRotation = FRotator::ZeroRotator;
    
    ATranspersonalCharacter* TestCharacter = World->SpawnActor<ATranspersonalCharacter>(
        ATranspersonalCharacter::StaticClass(), SpawnLocation, SpawnRotation);
    
    if (TestCharacter)
    {
        LogTestResult(TEXT("CharacterSpawn"), EQA_TestResult::Pass, TEXT("Character spawned successfully"));
        
        // Clean up test character
        TestCharacter->Destroy();
    }
    else
    {
        LogTestResult(TEXT("CharacterSpawn"), EQA_TestResult::Fail, TEXT("Failed to spawn character"));
    }
}

void UQA_TestManager::TestDinosaurAI()
{
    // Placeholder for dinosaur AI testing
    LogTestResult(TEXT("DinosaurAI"), EQA_TestResult::Warning, TEXT("Dinosaur AI tests not implemented"));
}

void UQA_TestManager::TestVFXSystems()
{
    // Placeholder for VFX system testing
    LogTestResult(TEXT("VFXSystems"), EQA_TestResult::Warning, TEXT("VFX system tests not implemented"));
}

void UQA_TestManager::TestWorldGeneration()
{
    // Placeholder for world generation testing
    LogTestResult(TEXT("WorldGeneration"), EQA_TestResult::Warning, TEXT("World generation tests not implemented"));
}

void UQA_TestManager::ExecuteTest(const FString& TestName, TFunction<bool()> TestFunction)
{
    float StartTime = FPlatformTime::Seconds();
    
    try
    {
        bool bTestPassed = TestFunction();
        float ExecutionTime = FPlatformTime::Seconds() - StartTime;
        
        EQA_TestResult Result = bTestPassed ? EQA_TestResult::Pass : EQA_TestResult::Fail;
        LogTestResult(TestName, Result, TEXT(""));
        
        // Update execution time
        for (FQA_TestCase& Test : TestResults)
        {
            if (Test.TestName == TestName)
            {
                Test.ExecutionTime = ExecutionTime;
                break;
            }
        }
    }
    catch (...)
    {
        LogTestResult(TestName, EQA_TestResult::Fail, TEXT("Exception during test execution"));
    }
}

void UQA_TestManager::LogTestResult(const FString& TestName, EQA_TestResult Result, const FString& Message)
{
    // Find existing test or create new one
    FQA_TestCase* ExistingTest = TestResults.FindByPredicate([&TestName](const FQA_TestCase& Test) {
        return Test.TestName == TestName;
    });
    
    if (ExistingTest)
    {
        ExistingTest->Result = Result;
        ExistingTest->ErrorMessage = Message;
    }
    else
    {
        FQA_TestCase NewTest;
        NewTest.TestName = TestName;
        NewTest.Result = Result;
        NewTest.ErrorMessage = Message;
        TestResults.Add(NewTest);
    }
    
    // Log to console
    FString ResultString;
    switch (Result)
    {
        case EQA_TestResult::Pass: ResultString = TEXT("PASS"); break;
        case EQA_TestResult::Fail: ResultString = TEXT("FAIL"); break;
        case EQA_TestResult::Warning: ResultString = TEXT("WARN"); break;
        default: ResultString = TEXT("UNKNOWN"); break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("QA Test [%s] %s: %s"), *TestName, *ResultString, *Message);
}

bool UQA_TestManager::ValidateClassLoading()
{
    // Test loading core TranspersonalGame classes
    TArray<FString> ClassesToTest = {
        TEXT("/Script/TranspersonalGame.TranspersonalCharacter"),
        TEXT("/Script/TranspersonalGame.TranspersonalGameMode"),
        TEXT("/Script/TranspersonalGame.TranspersonalGameState")
    };
    
    for (const FString& ClassName : ClassesToTest)
    {
        UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassName);
        if (!LoadedClass)
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to load class: %s"), *ClassName);
            return false;
        }
    }
    
    return true;
}

bool UQA_TestManager::ValidateCDOConstruction()
{
    // Test CDO construction for core classes
    UClass* CharacterClass = ATranspersonalCharacter::StaticClass();
    if (!CharacterClass || !CharacterClass->GetDefaultObject())
    {
        return false;
    }
    
    UClass* GameModeClass = ATranspersonalGameMode::StaticClass();
    if (!GameModeClass || !GameModeClass->GetDefaultObject())
    {
        return false;
    }
    
    return true;
}

bool UQA_TestManager::ValidateActorCounts()
{
    UWorld* World = GetWorld();
    if (!World) return false;
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    return AllActors.Num() <= MaxActorLimit;
}

bool UQA_TestManager::ValidatePerformanceMetrics()
{
    UWorld* World = GetWorld();
    if (!World) return false;
    
    // Basic performance validation
    float CurrentFPS = 1.0f / World->GetDeltaSeconds();
    return CurrentFPS >= TargetFrameRate;
}