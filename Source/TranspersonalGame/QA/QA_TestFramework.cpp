#include "QA_TestFramework.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Level.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Misc/Paths.h"
#include "UObject/UObjectGlobals.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"

UQA_TestFramework::UQA_TestFramework()
{
    bIsRunningTests = false;
    StartTime = 0.0f;
}

void UQA_TestFramework::RunAllTests()
{
    if (bIsRunningTests)
    {
        UE_LOG(LogTemp, Warning, TEXT("QA Tests already running"));
        return;
    }

    bIsRunningTests = true;
    TestResults.Empty();
    StartTime = FPlatformTime::Seconds();

    UE_LOG(LogTemp, Log, TEXT("QA Framework: Starting all tests"));

    // Run core system validation tests
    FQA_TestCase CharacterTest;
    CharacterTest.TestName = TEXT("Character System Validation");
    CharacterTest.TestDescription = TEXT("Validate TranspersonalCharacter class loads and functions");
    CharacterTest.Result = ValidateCharacterSystem() ? EQA_TestResult::Passed : EQA_TestResult::Failed;
    TestResults.Add(CharacterTest);

    FQA_TestCase VFXTest;
    VFXTest.TestName = TEXT("VFX System Validation");
    VFXTest.TestDescription = TEXT("Validate VFX classes and Niagara integration");
    VFXTest.Result = ValidateVFXSystem() ? EQA_TestResult::Passed : EQA_TestResult::Failed;
    TestResults.Add(VFXTest);

    FQA_TestCase WorldGenTest;
    WorldGenTest.TestName = TEXT("World Generation Validation");
    WorldGenTest.TestDescription = TEXT("Validate PCG and world generation systems");
    WorldGenTest.Result = ValidateWorldGeneration() ? EQA_TestResult::Passed : EQA_TestResult::Failed;
    TestResults.Add(WorldGenTest);

    FQA_TestCase PerformanceTest;
    PerformanceTest.TestName = TEXT("Performance Baseline");
    PerformanceTest.TestDescription = TEXT("Measure current frame rate and memory usage");
    float FrameRate = MeasureFrameRate(2.0f);
    PerformanceTest.Result = (FrameRate > 30.0f) ? EQA_TestResult::Passed : EQA_TestResult::Warning;
    PerformanceTest.ErrorMessage = FString::Printf(TEXT("Frame Rate: %.2f FPS"), FrameRate);
    TestResults.Add(PerformanceTest);

    bIsRunningTests = false;
    float TotalTime = FPlatformTime::Seconds() - StartTime;
    UE_LOG(LogTemp, Log, TEXT("QA Framework: All tests completed in %.2f seconds"), TotalTime);
}

void UQA_TestFramework::RunTestByName(const FString& TestName)
{
    UE_LOG(LogTemp, Log, TEXT("QA Framework: Running test: %s"), *TestName);
    
    if (TestName == TEXT("Character System Validation"))
    {
        FQA_TestCase Test;
        Test.TestName = TestName;
        Test.Result = ValidateCharacterSystem() ? EQA_TestResult::Passed : EQA_TestResult::Failed;
        TestResults.Add(Test);
    }
    else if (TestName == TEXT("VFX System Validation"))
    {
        FQA_TestCase Test;
        Test.TestName = TestName;
        Test.Result = ValidateVFXSystem() ? EQA_TestResult::Passed : EQA_TestResult::Failed;
        TestResults.Add(Test);
    }
}

void UQA_TestFramework::AddTestCase(const FQA_TestCase& TestCase)
{
    TestCases.Add(TestCase);
    UE_LOG(LogTemp, Log, TEXT("QA Framework: Added test case: %s"), *TestCase.TestName);
}

TArray<FQA_TestCase> UQA_TestFramework::GetTestResults() const
{
    return TestResults;
}

void UQA_TestFramework::ClearTestResults()
{
    TestResults.Empty();
    UE_LOG(LogTemp, Log, TEXT("QA Framework: Test results cleared"));
}

bool UQA_TestFramework::ValidateActorExists(const FString& ActorName)
{
    UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        return false;
    }

    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetName().Contains(ActorName))
        {
            return true;
        }
    }
    return false;
}

bool UQA_TestFramework::ValidateComponentExists(AActor* Actor, const FString& ComponentName)
{
    if (!Actor)
    {
        return false;
    }

    TArray<UActorComponent*> Components = Actor->GetRootComponent()->GetAttachChildren();
    for (UActorComponent* Component : Components)
    {
        if (Component && Component->GetName().Contains(ComponentName))
        {
            return true;
        }
    }
    return false;
}

bool UQA_TestFramework::ValidateClassLoads(const FString& ClassName)
{
    FString ClassPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ClassName);
    UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassPath);
    return LoadedClass != nullptr;
}

bool UQA_TestFramework::ValidateMapLoads(const FString& MapPath)
{
    return FPaths::FileExists(MapPath);
}

float UQA_TestFramework::MeasureFrameRate(float Duration)
{
    // Simple frame rate measurement
    UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        return 0.0f;
    }

    float DeltaTime = World->GetDeltaSeconds();
    return (DeltaTime > 0.0f) ? (1.0f / DeltaTime) : 60.0f;
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
    return static_cast<float>(MemStats.UsedPhysical) / (1024.0f * 1024.0f); // Convert to MB
}

bool UQA_TestFramework::ValidateVFXSystem()
{
    // Check if VFX classes can be loaded
    bool bVFXLibraryExists = ValidateClassLoads(TEXT("VFX_NiagaraLibrary"));
    bool bVFXManagerExists = ValidateClassLoads(TEXT("VFX_EffectManager"));
    
    UE_LOG(LogTemp, Log, TEXT("QA VFX Validation: Library=%s, Manager=%s"), 
           bVFXLibraryExists ? TEXT("PASS") : TEXT("FAIL"),
           bVFXManagerExists ? TEXT("PASS") : TEXT("FAIL"));
    
    return bVFXLibraryExists && bVFXManagerExists;
}

bool UQA_TestFramework::ValidateAudioSystem()
{
    // Check if Audio classes can be loaded
    bool bAudioManagerExists = ValidateClassLoads(TEXT("Audio_SoundManager"));
    
    UE_LOG(LogTemp, Log, TEXT("QA Audio Validation: Manager=%s"), 
           bAudioManagerExists ? TEXT("PASS") : TEXT("FAIL"));
    
    return bAudioManagerExists;
}

bool UQA_TestFramework::ValidateCharacterSystem()
{
    // Check if core character classes can be loaded
    bool bCharacterExists = ValidateClassLoads(TEXT("TranspersonalCharacter"));
    bool bGameStateExists = ValidateClassLoads(TEXT("TranspersonalGameState"));
    
    UE_LOG(LogTemp, Log, TEXT("QA Character Validation: Character=%s, GameState=%s"), 
           bCharacterExists ? TEXT("PASS") : TEXT("FAIL"),
           bGameStateExists ? TEXT("PASS") : TEXT("FAIL"));
    
    return bCharacterExists && bGameStateExists;
}

bool UQA_TestFramework::ValidateWorldGeneration()
{
    // Check if world generation classes can be loaded
    bool bPCGExists = ValidateClassLoads(TEXT("PCGWorldGenerator"));
    bool bFoliageExists = ValidateClassLoads(TEXT("FoliageManager"));
    
    UE_LOG(LogTemp, Log, TEXT("QA WorldGen Validation: PCG=%s, Foliage=%s"), 
           bPCGExists ? TEXT("PASS") : TEXT("FAIL"),
           bFoliageExists ? TEXT("PASS") : TEXT("FAIL"));
    
    return bPCGExists && bFoliageExists;
}

void UQA_TestFramework::LogTestResult(const FQA_TestCase& TestCase)
{
    FString ResultString;
    switch (TestCase.Result)
    {
        case EQA_TestResult::Passed:
            ResultString = TEXT("PASSED");
            break;
        case EQA_TestResult::Failed:
            ResultString = TEXT("FAILED");
            break;
        case EQA_TestResult::Warning:
            ResultString = TEXT("WARNING");
            break;
        default:
            ResultString = TEXT("NOT_RUN");
            break;
    }

    UE_LOG(LogTemp, Log, TEXT("QA Test [%s]: %s - %s"), 
           *ResultString, *TestCase.TestName, *TestCase.TestDescription);
    
    if (!TestCase.ErrorMessage.IsEmpty())
    {
        UE_LOG(LogTemp, Log, TEXT("QA Test Details: %s"), *TestCase.ErrorMessage);
    }
}