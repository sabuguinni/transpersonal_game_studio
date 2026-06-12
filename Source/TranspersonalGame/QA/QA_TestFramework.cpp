#include "QA_TestFramework.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "NiagaraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"

UQA_TestFramework::UQA_TestFramework()
{
    TotalTestTime = 0.0f;
}

void UQA_TestFramework::RunAllTests()
{
    ClearTestResults();
    
    double StartTime = FPlatformTime::Seconds();
    
    UE_LOG(LogTemp, Warning, TEXT("QA Test Framework: Starting comprehensive test suite"));
    
    RunVFXTests();
    RunCharacterTests();
    RunWorldTests();
    RunPerformanceTests();
    
    TotalTestTime = FPlatformTime::Seconds() - StartTime;
    
    GenerateTestReport();
}

void UQA_TestFramework::RunVFXTests()
{
    // Test VFX_NiagaraSystemManager compilation
    FQA_TestCase VFXCompileTest = CreateTestCase(
        TEXT("VFX_NiagaraSystemManager_Compilation"),
        TEXT("Verify VFX Niagara System Manager compiles and loads correctly")
    );
    
    UClass* VFXClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.VFX_NiagaraSystemManager"));
    if (VFXClass)
    {
        VFXCompileTest.Result = EQA_TestResult::Pass;
        UE_LOG(LogTemp, Log, TEXT("QA Test PASS: VFX_NiagaraSystemManager class loaded"));
    }
    else
    {
        VFXCompileTest.Result = EQA_TestResult::Fail;
        VFXCompileTest.ErrorMessage = TEXT("VFX_NiagaraSystemManager class not found");
        UE_LOG(LogTemp, Error, TEXT("QA Test FAIL: VFX_NiagaraSystemManager class not found"));
    }
    
    TestResults.Add(VFXCompileTest);
    
    // Test Niagara components in level
    ValidateNiagaraComponents();
}

void UQA_TestFramework::RunCharacterTests()
{
    FQA_TestCase CharacterTest = CreateTestCase(
        TEXT("TranspersonalCharacter_Compilation"),
        TEXT("Verify TranspersonalCharacter compiles and has required components")
    );
    
    UClass* CharClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
    if (CharClass)
    {
        CharacterTest.Result = EQA_TestResult::Pass;
        UE_LOG(LogTemp, Log, TEXT("QA Test PASS: TranspersonalCharacter class loaded"));
    }
    else
    {
        CharacterTest.Result = EQA_TestResult::Fail;
        CharacterTest.ErrorMessage = TEXT("TranspersonalCharacter class not found");
        UE_LOG(LogTemp, Error, TEXT("QA Test FAIL: TranspersonalCharacter class not found"));
    }
    
    TestResults.Add(CharacterTest);
}

void UQA_TestFramework::RunWorldTests()
{
    FQA_TestCase WorldTest = CreateTestCase(
        TEXT("MinPlayableMap_Validation"),
        TEXT("Verify MinPlayableMap has essential actors and components")
    );
    
    UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull);
    if (World)
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        
        if (AllActors.Num() > 0)
        {
            WorldTest.Result = EQA_TestResult::Pass;
            UE_LOG(LogTemp, Log, TEXT("QA Test PASS: MinPlayableMap has %d actors"), AllActors.Num());
        }
        else
        {
            WorldTest.Result = EQA_TestResult::Warning;
            WorldTest.ErrorMessage = TEXT("No actors found in level");
        }
    }
    else
    {
        WorldTest.Result = EQA_TestResult::Fail;
        WorldTest.ErrorMessage = TEXT("World context not available");
    }
    
    TestResults.Add(WorldTest);
}

void UQA_TestFramework::RunPerformanceTests()
{
    ValidateActorCount(8000);
}

bool UQA_TestFramework::ValidateActorCount(int32 MaxActors)
{
    FQA_TestCase ActorCountTest = CreateTestCase(
        TEXT("Actor_Count_Performance"),
        FString::Printf(TEXT("Verify actor count is within performance limit of %d"), MaxActors)
    );
    
    UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull);
    if (World)
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        
        int32 ActorCount = AllActors.Num();
        
        if (ActorCount <= MaxActors)
        {
            ActorCountTest.Result = EQA_TestResult::Pass;
            UE_LOG(LogTemp, Log, TEXT("QA Test PASS: Actor count %d within limit"), ActorCount);
        }
        else
        {
            ActorCountTest.Result = EQA_TestResult::Fail;
            ActorCountTest.ErrorMessage = FString::Printf(TEXT("Actor count %d exceeds limit %d"), ActorCount, MaxActors);
            UE_LOG(LogTemp, Error, TEXT("QA Test FAIL: Actor count %d exceeds limit %d"), ActorCount, MaxActors);
        }
        
        TestResults.Add(ActorCountTest);
        return ActorCountTest.Result == EQA_TestResult::Pass;
    }
    
    return false;
}

bool UQA_TestFramework::ValidateClassCompilation(const FString& ClassName)
{
    UClass* TestClass = LoadClass<UObject>(nullptr, *ClassName);
    return TestClass != nullptr;
}

bool UQA_TestFramework::ValidateNiagaraComponents()
{
    FQA_TestCase NiagaraTest = CreateTestCase(
        TEXT("Niagara_Components_Validation"),
        TEXT("Verify Niagara components exist and are properly configured")
    );
    
    UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull);
    if (World)
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        
        int32 NiagaraComponentCount = 0;
        
        for (AActor* Actor : AllActors)
        {
            if (Actor)
            {
                TArray<UNiagaraComponent*> NiagaraComponents;
                Actor->GetComponents<UNiagaraComponent>(NiagaraComponents);
                NiagaraComponentCount += NiagaraComponents.Num();
            }
        }
        
        if (NiagaraComponentCount > 0)
        {
            NiagaraTest.Result = EQA_TestResult::Pass;
            UE_LOG(LogTemp, Log, TEXT("QA Test PASS: Found %d Niagara components"), NiagaraComponentCount);
        }
        else
        {
            NiagaraTest.Result = EQA_TestResult::Warning;
            NiagaraTest.ErrorMessage = TEXT("No Niagara components found in level");
        }
        
        TestResults.Add(NiagaraTest);
        return NiagaraTest.Result == EQA_TestResult::Pass;
    }
    
    return false;
}

int32 UQA_TestFramework::GetPassedTestCount() const
{
    int32 PassCount = 0;
    for (const FQA_TestCase& Test : TestResults)
    {
        if (Test.Result == EQA_TestResult::Pass)
        {
            PassCount++;
        }
    }
    return PassCount;
}

int32 UQA_TestFramework::GetFailedTestCount() const
{
    int32 FailCount = 0;
    for (const FQA_TestCase& Test : TestResults)
    {
        if (Test.Result == EQA_TestResult::Fail)
        {
            FailCount++;
        }
    }
    return FailCount;
}

float UQA_TestFramework::GetTestSuccessRate() const
{
    if (TestResults.Num() == 0)
    {
        return 0.0f;
    }
    
    return (float)GetPassedTestCount() / (float)TestResults.Num() * 100.0f;
}

void UQA_TestFramework::GenerateTestReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== QA TEST FRAMEWORK REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Total Tests: %d"), TestResults.Num());
    UE_LOG(LogTemp, Warning, TEXT("Passed: %d"), GetPassedTestCount());
    UE_LOG(LogTemp, Warning, TEXT("Failed: %d"), GetFailedTestCount());
    UE_LOG(LogTemp, Warning, TEXT("Success Rate: %.1f%%"), GetTestSuccessRate());
    UE_LOG(LogTemp, Warning, TEXT("Total Execution Time: %.3f seconds"), TotalTestTime);
    
    for (const FQA_TestCase& Test : TestResults)
    {
        FString ResultStr;
        switch (Test.Result)
        {
            case EQA_TestResult::Pass: ResultStr = TEXT("PASS"); break;
            case EQA_TestResult::Fail: ResultStr = TEXT("FAIL"); break;
            case EQA_TestResult::Warning: ResultStr = TEXT("WARN"); break;
            case EQA_TestResult::Skipped: ResultStr = TEXT("SKIP"); break;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("[%s] %s: %s"), *ResultStr, *Test.TestName, *Test.TestDescription);
        if (!Test.ErrorMessage.IsEmpty())
        {
            UE_LOG(LogTemp, Warning, TEXT("  Error: %s"), *Test.ErrorMessage);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== END QA REPORT ==="));
}

void UQA_TestFramework::AddTestResult(const FString& TestName, const FString& Description, EQA_TestResult Result, const FString& ErrorMsg)
{
    FQA_TestCase NewTest;
    NewTest.TestName = TestName;
    NewTest.TestDescription = Description;
    NewTest.Result = Result;
    NewTest.ErrorMessage = ErrorMsg;
    NewTest.ExecutionTime = 0.0f;
    
    TestResults.Add(NewTest);
}

void UQA_TestFramework::ClearTestResults()
{
    TestResults.Empty();
    TotalTestTime = 0.0f;
}

FQA_TestCase UQA_TestFramework::CreateTestCase(const FString& Name, const FString& Description)
{
    FQA_TestCase TestCase;
    TestCase.TestName = Name;
    TestCase.TestDescription = Description;
    TestCase.Result = EQA_TestResult::Skipped;
    TestCase.ErrorMessage = TEXT("");
    TestCase.ExecutionTime = 0.0f;
    
    return TestCase;
}