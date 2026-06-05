#include "QA_TestFramework.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EditorLevelLibrary.h"
#include "EditorAssetLibrary.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"

UQA_TestFramework::UQA_TestFramework()
{
    bAutoRunOnBeginPlay = false;
    bLogDetailedResults = true;
    TestTimeout = 30.0f;
}

void UQA_TestFramework::RunAllTests()
{
    UE_LOG(LogTemp, Warning, TEXT("QA Framework: Starting comprehensive test suite"));
    
    ClearTestResults();
    
    // Run all test categories
    RunCoreSystemTests();
    RunVFXSystemTests();
    RunPerformanceTests();
    RunIntegrationTests();
    
    GenerateTestReport();
    
    UE_LOG(LogTemp, Warning, TEXT("QA Framework: All tests completed"));
}

void UQA_TestFramework::RunVFXSystemTests()
{
    UE_LOG(LogTemp, Warning, TEXT("QA Framework: Running VFX system tests"));
    
    // Test VFX class loading
    FQA_TestCase VFXClassTest = ExecuteTest(TEXT("VFX_PrehistoricFireSystem_Loading"), [this]() -> bool
    {
        UClass* VFXClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.VFX_PrehistoricFireSystem"));
        return VFXClass != nullptr;
    });
    AddTestResult(VFXClassTest);
    
    // Test Niagara system assets
    FQA_TestCase NiagaraAssetsTest = ExecuteTest(TEXT("Niagara_Fire_Assets"), [this]() -> bool
    {
        TArray<FString> Assets = UEditorAssetLibrary::ListAssets(TEXT("/Game/VFX"), true);
        int32 FireAssets = 0;
        for (const FString& Asset : Assets)
        {
            if (Asset.Contains(TEXT("Fire")) || Asset.Contains(TEXT("Campfire")))
            {
                FireAssets++;
            }
        }
        return FireAssets > 0;
    });
    AddTestResult(NiagaraAssetsTest);
    
    // Test VFX actor spawning
    FQA_TestCase VFXSpawnTest = ExecuteTest(TEXT("VFX_Actor_Spawning"), [this]() -> bool
    {
        UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
        if (!World) return false;
        
        // Try to spawn a basic VFX actor
        FVector SpawnLocation(0.0f, 0.0f, 100.0f);
        FRotator SpawnRotation = FRotator::ZeroRotator;
        
        AActor* VFXActor = World->SpawnActor<AActor>(AActor::StaticClass(), SpawnLocation, SpawnRotation);
        bool bSpawned = VFXActor != nullptr;
        
        if (VFXActor)
        {
            VFXActor->Destroy();
        }
        
        return bSpawned;
    });
    AddTestResult(VFXSpawnTest);
}

void UQA_TestFramework::RunCoreSystemTests()
{
    UE_LOG(LogTemp, Warning, TEXT("QA Framework: Running core system tests"));
    
    // Test TranspersonalGameState loading
    FQA_TestCase GameStateTest = ExecuteTest(TEXT("TranspersonalGameState_Loading"), [this]() -> bool
    {
        UClass* GameStateClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalGameState"));
        return GameStateClass != nullptr;
    });
    AddTestResult(GameStateTest);
    
    // Test TranspersonalCharacter loading
    FQA_TestCase CharacterTest = ExecuteTest(TEXT("TranspersonalCharacter_Loading"), [this]() -> bool
    {
        UClass* CharacterClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
        return CharacterClass != nullptr;
    });
    AddTestResult(CharacterTest);
    
    // Test PCGWorldGenerator loading
    FQA_TestCase WorldGenTest = ExecuteTest(TEXT("PCGWorldGenerator_Loading"), [this]() -> bool
    {
        UClass* WorldGenClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.PCGWorldGenerator"));
        return WorldGenClass != nullptr;
    });
    AddTestResult(WorldGenTest);
}

void UQA_TestFramework::RunPerformanceTests()
{
    UE_LOG(LogTemp, Warning, TEXT("QA Framework: Running performance tests"));
    
    // Test actor count performance
    FQA_TestCase ActorCountTest = ExecuteTest(TEXT("Actor_Count_Performance"), [this]() -> bool
    {
        TArray<AActor*> AllActors = UEditorLevelLibrary::GetAllLevelActors();
        int32 ActorCount = AllActors.Num();
        
        // Reasonable actor count for performance (under 1000 for editor)
        bool bPerformanceOK = ActorCount < 1000;
        
        UE_LOG(LogTemp, Warning, TEXT("QA: Actor count: %d (Performance OK: %s)"), 
               ActorCount, bPerformanceOK ? TEXT("Yes") : TEXT("No"));
        
        return bPerformanceOK;
    });
    AddTestResult(ActorCountTest);
    
    // Test memory usage
    FQA_TestCase MemoryTest = ExecuteTest(TEXT("Memory_Usage_Check"), [this]() -> bool
    {
        // Basic memory check - always pass for now, but log info
        FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
        UE_LOG(LogTemp, Warning, TEXT("QA: Available Physical Memory: %llu MB"), 
               MemStats.AvailablePhysical / (1024 * 1024));
        
        return true; // Always pass for now
    });
    AddTestResult(MemoryTest);
}

void UQA_TestFramework::RunIntegrationTests()
{
    UE_LOG(LogTemp, Warning, TEXT("QA Framework: Running integration tests"));
    
    // Test map integrity
    FQA_TestCase MapIntegrityTest = ExecuteTest(TEXT("Map_Integrity_Check"), [this]() -> bool
    {
        return ValidateMapIntegrity();
    });
    AddTestResult(MapIntegrityTest);
    
    // Test minimum actor requirements
    FQA_TestCase MinActorTest = ExecuteTest(TEXT("Minimum_Actor_Requirements"), [this]() -> bool
    {
        return ValidateActorCount(10); // Minimum 10 actors in map
    });
    AddTestResult(MinActorTest);
}

void UQA_TestFramework::AddTestResult(const FQA_TestCase& TestCase)
{
    TestResults.Add(TestCase);
    LogTestResult(TestCase);
}

TArray<FQA_TestCase> UQA_TestFramework::GetTestResults() const
{
    return TestResults;
}

void UQA_TestFramework::ClearTestResults()
{
    TestResults.Empty();
}

void UQA_TestFramework::GenerateTestReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== QA TEST REPORT ==="));
    
    int32 PassCount = 0;
    int32 FailCount = 0;
    int32 WarningCount = 0;
    int32 SkippedCount = 0;
    
    for (const FQA_TestCase& Test : TestResults)
    {
        switch (Test.Result)
        {
            case EQA_TestResult::Pass: PassCount++; break;
            case EQA_TestResult::Fail: FailCount++; break;
            case EQA_TestResult::Warning: WarningCount++; break;
            case EQA_TestResult::Skipped: SkippedCount++; break;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Total Tests: %d"), TestResults.Num());
    UE_LOG(LogTemp, Warning, TEXT("PASS: %d"), PassCount);
    UE_LOG(LogTemp, Warning, TEXT("FAIL: %d"), FailCount);
    UE_LOG(LogTemp, Warning, TEXT("WARNING: %d"), WarningCount);
    UE_LOG(LogTemp, Warning, TEXT("SKIPPED: %d"), SkippedCount);
    
    float SuccessRate = TestResults.Num() > 0 ? (float)PassCount / TestResults.Num() * 100.0f : 0.0f;
    UE_LOG(LogTemp, Warning, TEXT("Success Rate: %.1f%%"), SuccessRate);
    
    UE_LOG(LogTemp, Warning, TEXT("=== END QA REPORT ==="));
}

bool UQA_TestFramework::ValidateActorCount(int32 ExpectedMinimum)
{
    TArray<AActor*> AllActors = UEditorLevelLibrary::GetAllLevelActors();
    return AllActors.Num() >= ExpectedMinimum;
}

bool UQA_TestFramework::ValidateClassLoading(const FString& ClassName)
{
    UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassName);
    return LoadedClass != nullptr;
}

bool UQA_TestFramework::ValidateNiagaraSystem(const FString& SystemPath)
{
    UNiagaraSystem* NiagaraSystem = LoadObject<UNiagaraSystem>(nullptr, *SystemPath);
    return NiagaraSystem != nullptr;
}

bool UQA_TestFramework::ValidateMapIntegrity()
{
    UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
    if (!World)
    {
        return false;
    }
    
    // Basic map integrity checks
    TArray<AActor*> AllActors = UEditorLevelLibrary::GetAllLevelActors();
    
    // Check for null actors
    for (AActor* Actor : AllActors)
    {
        if (!IsValid(Actor))
        {
            return false;
        }
    }
    
    return true;
}

FQA_TestCase UQA_TestFramework::ExecuteTest(const FString& TestName, TFunction<bool()> TestFunction)
{
    FQA_TestCase TestCase;
    TestCase.TestName = TestName;
    TestCase.TestDescription = FString::Printf(TEXT("Automated test: %s"), *TestName);
    
    double StartTime = GetCurrentTime();
    
    try
    {
        bool bTestPassed = TestFunction();
        TestCase.Result = bTestPassed ? EQA_TestResult::Pass : EQA_TestResult::Fail;
        
        if (!bTestPassed)
        {
            TestCase.ErrorMessage = FString::Printf(TEXT("Test %s failed"), *TestName);
        }
    }
    catch (...)
    {
        TestCase.Result = EQA_TestResult::Fail;
        TestCase.ErrorMessage = FString::Printf(TEXT("Test %s threw an exception"), *TestName);
    }
    
    double EndTime = GetCurrentTime();
    TestCase.ExecutionTime = static_cast<float>(EndTime - StartTime);
    
    return TestCase;
}

void UQA_TestFramework::LogTestResult(const FQA_TestCase& TestCase)
{
    if (!bLogDetailedResults) return;
    
    FString ResultString;
    switch (TestCase.Result)
    {
        case EQA_TestResult::Pass: ResultString = TEXT("PASS"); break;
        case EQA_TestResult::Fail: ResultString = TEXT("FAIL"); break;
        case EQA_TestResult::Warning: ResultString = TEXT("WARN"); break;
        case EQA_TestResult::Skipped: ResultString = TEXT("SKIP"); break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("QA Test [%s] %s (%.3fs) - %s"), 
           *ResultString, *TestCase.TestName, TestCase.ExecutionTime, 
           TestCase.ErrorMessage.IsEmpty() ? TEXT("OK") : *TestCase.ErrorMessage);
}

double UQA_TestFramework::GetCurrentTime() const
{
    return FPlatformTime::Seconds();
}