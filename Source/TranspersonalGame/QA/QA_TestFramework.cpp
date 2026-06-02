#include "QA_TestFramework.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"

UQA_TestFramework::UQA_TestFramework()
{
    PassedTests = 0;
    FailedTests = 0;
    WarningTests = 0;
}

void UQA_TestFramework::RunAllTests()
{
    ClearTestResults();
    
    UE_LOG(LogTemp, Warning, TEXT("QA Framework: Starting comprehensive test suite"));
    
    RunModuleTests();
    RunPerformanceTests();
    RunVFXTests();
    RunDinosaurTests();
    
    GenerateTestReport();
}

void UQA_TestFramework::RunModuleTests()
{
    // Test module loading
    FQA_TestCase ModuleTest = ValidateModuleLoading();
    AddTestResult(ModuleTest);
    
    // Test actor count
    FQA_TestCase ActorTest = ValidateActorCount();
    AddTestResult(ActorTest);
}

void UQA_TestFramework::RunPerformanceTests()
{
    FQA_TestCase PerfTest = CreateTestCase(TEXT("Performance_ActorLimit"), TEXT("Validate actor count within performance limits"));
    
    UWorld* World = GEngine->GetCurrentPlayWorld();
    if (!World)
    {
        World = GEditor->GetEditorWorldContext().World();
    }
    
    if (World)
    {
        int32 ActorCount = 0;
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            ActorCount++;
        }
        
        if (ActorCount > 8000)
        {
            PerfTest.Result = EQA_TestResult::Fail;
            PerfTest.ErrorMessage = FString::Printf(TEXT("Actor count %d exceeds limit of 8000"), ActorCount);
        }
        else if (ActorCount > 6000)
        {
            PerfTest.Result = EQA_TestResult::Warning;
            PerfTest.ErrorMessage = FString::Printf(TEXT("Actor count %d approaching limit"), ActorCount);
        }
        else
        {
            PerfTest.Result = EQA_TestResult::Pass;
        }
    }
    else
    {
        PerfTest.Result = EQA_TestResult::Fail;
        PerfTest.ErrorMessage = TEXT("No valid world found");
    }
    
    AddTestResult(PerfTest);
}

void UQA_TestFramework::RunVFXTests()
{
    FQA_TestCase VFXTest = ValidateVFXSystems();
    AddTestResult(VFXTest);
}

void UQA_TestFramework::RunDinosaurTests()
{
    FQA_TestCase DinoTest = CreateTestCase(TEXT("Dinosaur_PopulationLimit"), TEXT("Validate dinosaur population within realistic limits"));
    
    UWorld* World = GEngine->GetCurrentPlayWorld();
    if (!World)
    {
        World = GEditor->GetEditorWorldContext().World();
    }
    
    if (World)
    {
        int32 DinosaurCount = 0;
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            FString ActorName = Actor->GetActorLabel().ToLower();
            
            if (ActorName.Contains(TEXT("trex")) || ActorName.Contains(TEXT("veloci")) || 
                ActorName.Contains(TEXT("tricera")) || ActorName.Contains(TEXT("brachi")) ||
                ActorName.Contains(TEXT("ankylo")) || ActorName.Contains(TEXT("parasauro")))
            {
                DinosaurCount++;
            }
        }
        
        if (DinosaurCount > 150)
        {
            DinoTest.Result = EQA_TestResult::Fail;
            DinoTest.ErrorMessage = FString::Printf(TEXT("Dinosaur count %d exceeds realistic limit of 150"), DinosaurCount);
        }
        else if (DinosaurCount > 120)
        {
            DinoTest.Result = EQA_TestResult::Warning;
            DinoTest.ErrorMessage = FString::Printf(TEXT("Dinosaur count %d approaching limit"), DinosaurCount);
        }
        else
        {
            DinoTest.Result = EQA_TestResult::Pass;
        }
    }
    else
    {
        DinoTest.Result = EQA_TestResult::Fail;
        DinoTest.ErrorMessage = TEXT("No valid world found");
    }
    
    AddTestResult(DinoTest);
}

FQA_TestCase UQA_TestFramework::ValidateActorCount()
{
    FQA_TestCase TestCase = CreateTestCase(TEXT("World_ActorCount"), TEXT("Validate world actor count"));
    
    UWorld* World = GEngine->GetCurrentPlayWorld();
    if (!World)
    {
        World = GEditor->GetEditorWorldContext().World();
    }
    
    if (World)
    {
        int32 ActorCount = 0;
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            ActorCount++;
        }
        
        TestCase.Result = EQA_TestResult::Pass;
        TestCase.ErrorMessage = FString::Printf(TEXT("Found %d actors in world"), ActorCount);
    }
    else
    {
        TestCase.Result = EQA_TestResult::Fail;
        TestCase.ErrorMessage = TEXT("No valid world found");
    }
    
    return TestCase;
}

FQA_TestCase UQA_TestFramework::ValidateModuleLoading()
{
    FQA_TestCase TestCase = CreateTestCase(TEXT("Module_Loading"), TEXT("Validate TranspersonalGame module classes"));
    
    // Test core classes
    UClass* GameStateClass = FindObject<UClass>(ANY_PACKAGE, TEXT("TranspersonalGameState"));
    UClass* CharacterClass = FindObject<UClass>(ANY_PACKAGE, TEXT("TranspersonalCharacter"));
    
    if (GameStateClass && CharacterClass)
    {
        TestCase.Result = EQA_TestResult::Pass;
        TestCase.ErrorMessage = TEXT("Core module classes loaded successfully");
    }
    else
    {
        TestCase.Result = EQA_TestResult::Fail;
        TestCase.ErrorMessage = TEXT("Failed to load core module classes");
    }
    
    return TestCase;
}

FQA_TestCase UQA_TestFramework::ValidateVFXSystems()
{
    FQA_TestCase TestCase = CreateTestCase(TEXT("VFX_Systems"), TEXT("Validate VFX and Niagara systems"));
    
    UWorld* World = GEngine->GetCurrentPlayWorld();
    if (!World)
    {
        World = GEditor->GetEditorWorldContext().World();
    }
    
    if (World)
    {
        int32 NiagaraCount = 0;
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor->FindComponentByClass<UNiagaraComponent>())
            {
                NiagaraCount++;
            }
        }
        
        TestCase.Result = EQA_TestResult::Pass;
        TestCase.ErrorMessage = FString::Printf(TEXT("Found %d Niagara components"), NiagaraCount);
    }
    else
    {
        TestCase.Result = EQA_TestResult::Fail;
        TestCase.ErrorMessage = TEXT("No valid world found");
    }
    
    return TestCase;
}

void UQA_TestFramework::ClearTestResults()
{
    TestResults.Empty();
    PassedTests = 0;
    FailedTests = 0;
    WarningTests = 0;
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
        FString ResultStr;
        switch (Test.Result)
        {
            case EQA_TestResult::Pass: ResultStr = TEXT("PASS"); break;
            case EQA_TestResult::Fail: ResultStr = TEXT("FAIL"); break;
            case EQA_TestResult::Warning: ResultStr = TEXT("WARN"); break;
            default: ResultStr = TEXT("SKIP"); break;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("[%s] %s: %s"), *ResultStr, *Test.TestName, *Test.ErrorMessage);
    }
}

void UQA_TestFramework::AddTestResult(const FQA_TestCase& TestCase)
{
    TestResults.Add(TestCase);
    
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

FQA_TestCase UQA_TestFramework::CreateTestCase(const FString& Name, const FString& Description)
{
    FQA_TestCase TestCase;
    TestCase.TestName = Name;
    TestCase.Description = Description;
    TestCase.Result = EQA_TestResult::Skipped;
    TestCase.ExecutionTime = 0.0f;
    return TestCase;
}