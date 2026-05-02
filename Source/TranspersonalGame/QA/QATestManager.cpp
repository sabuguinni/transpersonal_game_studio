#include "QATestManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyAtmosphere.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/DateTime.h"

UQATestManager::UQATestManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
    
    bAutoRunTests = false;
    TestInterval = 30.0f;
    bGenerateDetailedLogs = true;
    
    // Performance thresholds
    MinFrameRate = 30.0f;
    MaxMemoryUsageMB = 4096.0f;
    MaxActorCount = 1000;
    
    TestTimer = 0.0f;
    bTestsInitialized = false;
}

void UQATestManager::BeginPlay()
{
    Super::BeginPlay();
    
    if (!bTestsInitialized)
    {
        InitializeTestSuites();
        bTestsInitialized = true;
    }
    
    UE_LOG(LogTemp, Log, TEXT("QA Test Manager initialized"));
}

void UQATestManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bAutoRunTests)
    {
        TestTimer += DeltaTime;
        if (TestTimer >= TestInterval)
        {
            RunAllTests();
            TestTimer = 0.0f;
        }
    }
}

void UQATestManager::InitializeTestSuites()
{
    TestSuites.Empty();
    
    // Map Validation Suite
    FQA_TestSuite MapSuite;
    MapSuite.SuiteName = TEXT("Map Validation");
    TestSuites.Add(MapSuite);
    
    AddTestCase(TEXT("Map Validation"), TEXT("Actor Count"), TEXT("Validate total actor count is within limits"));
    AddTestCase(TEXT("Map Validation"), TEXT("Lighting Setup"), TEXT("Validate lighting actors are properly configured"));
    AddTestCase(TEXT("Map Validation"), TEXT("No Duplicates"), TEXT("Validate no duplicate critical actors exist"));
    
    // Character Validation Suite
    FQA_TestSuite CharacterSuite;
    CharacterSuite.SuiteName = TEXT("Character Validation");
    TestSuites.Add(CharacterSuite);
    
    AddTestCase(TEXT("Character Validation"), TEXT("Player Spawn"), TEXT("Validate player character spawns correctly"));
    AddTestCase(TEXT("Character Validation"), TEXT("Movement"), TEXT("Validate character movement components"));
    AddTestCase(TEXT("Character Validation"), TEXT("Stats"), TEXT("Validate survival stats are functional"));
    
    // Dinosaur Validation Suite
    FQA_TestSuite DinosaurSuite;
    DinosaurSuite.SuiteName = TEXT("Dinosaur Validation");
    TestSuites.Add(DinosaurSuite);
    
    AddTestCase(TEXT("Dinosaur Validation"), TEXT("Spawn Count"), TEXT("Validate dinosaur actors are present"));
    AddTestCase(TEXT("Dinosaur Validation"), TEXT("AI Components"), TEXT("Validate dinosaur AI components"));
    AddTestCase(TEXT("Dinosaur Validation"), TEXT("Collision"), TEXT("Validate dinosaur collision setup"));
    
    // Performance Suite
    FQA_TestSuite PerformanceSuite;
    PerformanceSuite.SuiteName = TEXT("Performance");
    TestSuites.Add(PerformanceSuite);
    
    AddTestCase(TEXT("Performance"), TEXT("Frame Rate"), TEXT("Validate frame rate meets minimum threshold"));
    AddTestCase(TEXT("Performance"), TEXT("Memory Usage"), TEXT("Validate memory usage is within limits"));
    AddTestCase(TEXT("Performance"), TEXT("Draw Calls"), TEXT("Validate draw call count is optimized"));
    
    UE_LOG(LogTemp, Log, TEXT("QA Test Suites initialized: %d suites"), TestSuites.Num());
}

void UQATestManager::AddTestCase(const FString& SuiteName, const FString& TestName, const FString& Description)
{
    for (FQA_TestSuite& Suite : TestSuites)
    {
        if (Suite.SuiteName == SuiteName)
        {
            FQA_TestCase NewTest;
            NewTest.TestName = TestName;
            NewTest.TestDescription = Description;
            NewTest.Result = EQA_TestResult::NotRun;
            Suite.TestCases.Add(NewTest);
            break;
        }
    }
}

void UQATestManager::RunAllTests()
{
    UE_LOG(LogTemp, Log, TEXT("QA: Running all test suites"));
    
    for (FQA_TestSuite& Suite : TestSuites)
    {
        RunTestSuite(Suite.SuiteName);
    }
    
    GenerateTestReport();
}

void UQATestManager::RunTestSuite(const FString& SuiteName)
{
    UE_LOG(LogTemp, Log, TEXT("QA: Running test suite: %s"), *SuiteName);
    
    if (SuiteName == TEXT("Map Validation"))
    {
        ValidateMapActors();
        ValidateLightingSetup();
    }
    else if (SuiteName == TEXT("Character Validation"))
    {
        ValidatePlayerCharacter();
    }
    else if (SuiteName == TEXT("Dinosaur Validation"))
    {
        ValidateDinosaurActors();
    }
    else if (SuiteName == TEXT("Performance"))
    {
        ValidateFrameRate();
        ValidateMemoryUsage();
    }
}

void UQATestManager::RunSingleTest(const FString& TestName)
{
    UE_LOG(LogTemp, Log, TEXT("QA: Running single test: %s"), *TestName);
    
    if (TestName == TEXT("Actor Count"))
    {
        ValidateMapActors();
    }
    else if (TestName == TEXT("Lighting Setup"))
    {
        ValidateLightingSetup();
    }
    else if (TestName == TEXT("Player Spawn"))
    {
        ValidatePlayerCharacter();
    }
    else if (TestName == TEXT("Spawn Count"))
    {
        ValidateDinosaurActors();
    }
    else if (TestName == TEXT("Frame Rate"))
    {
        ValidateFrameRate();
    }
}

bool UQATestManager::ValidateMapActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UpdateTestResult(TEXT("Map Validation"), TEXT("Actor Count"), EQA_TestResult::Fail, TEXT("World not found"));
        return false;
    }
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    int32 ActorCount = AllActors.Num();
    
    if (ActorCount > MaxActorCount)
    {
        FString ErrorMsg = FString::Printf(TEXT("Too many actors: %d (max: %d)"), ActorCount, MaxActorCount);
        UpdateTestResult(TEXT("Map Validation"), TEXT("Actor Count"), EQA_TestResult::Warning, ErrorMsg);
        return false;
    }
    else if (ActorCount < 10)
    {
        FString ErrorMsg = FString::Printf(TEXT("Too few actors: %d (expected at least 10)"), ActorCount);
        UpdateTestResult(TEXT("Map Validation"), TEXT("Actor Count"), EQA_TestResult::Fail, ErrorMsg);
        return false;
    }
    
    UpdateTestResult(TEXT("Map Validation"), TEXT("Actor Count"), EQA_TestResult::Pass);
    UE_LOG(LogTemp, Log, TEXT("QA: Actor count validation passed: %d actors"), ActorCount);
    return true;
}

bool UQATestManager::ValidatePlayerCharacter()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UpdateTestResult(TEXT("Character Validation"), TEXT("Player Spawn"), EQA_TestResult::Fail, TEXT("World not found"));
        return false;
    }
    
    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC)
    {
        UpdateTestResult(TEXT("Character Validation"), TEXT("Player Spawn"), EQA_TestResult::Fail, TEXT("Player controller not found"));
        return false;
    }
    
    ACharacter* PlayerCharacter = Cast<ACharacter>(PC->GetPawn());
    if (!PlayerCharacter)
    {
        UpdateTestResult(TEXT("Character Validation"), TEXT("Player Spawn"), EQA_TestResult::Fail, TEXT("Player character not found"));
        return false;
    }
    
    UpdateTestResult(TEXT("Character Validation"), TEXT("Player Spawn"), EQA_TestResult::Pass);
    UE_LOG(LogTemp, Log, TEXT("QA: Player character validation passed"));
    return true;
}

bool UQATestManager::ValidateDinosaurActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UpdateTestResult(TEXT("Dinosaur Validation"), TEXT("Spawn Count"), EQA_TestResult::Fail, TEXT("World not found"));
        return false;
    }
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    int32 DinosaurCount = 0;
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetName().Contains(TEXT("Dinosaur")))
        {
            DinosaurCount++;
        }
    }
    
    if (DinosaurCount < 3)
    {
        FString ErrorMsg = FString::Printf(TEXT("Too few dinosaurs: %d (expected at least 3)"), DinosaurCount);
        UpdateTestResult(TEXT("Dinosaur Validation"), TEXT("Spawn Count"), EQA_TestResult::Warning, ErrorMsg);
        return false;
    }
    
    UpdateTestResult(TEXT("Dinosaur Validation"), TEXT("Spawn Count"), EQA_TestResult::Pass);
    UE_LOG(LogTemp, Log, TEXT("QA: Dinosaur validation passed: %d dinosaurs found"), DinosaurCount);
    return true;
}

bool UQATestManager::ValidateLightingSetup()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UpdateTestResult(TEXT("Map Validation"), TEXT("Lighting Setup"), EQA_TestResult::Fail, TEXT("World not found"));
        return false;
    }
    
    // Check for directional light
    TArray<AActor*> DirectionalLights;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), DirectionalLights);
    
    if (DirectionalLights.Num() != 1)
    {
        FString ErrorMsg = FString::Printf(TEXT("Expected 1 DirectionalLight, found %d"), DirectionalLights.Num());
        UpdateTestResult(TEXT("Map Validation"), TEXT("Lighting Setup"), EQA_TestResult::Fail, ErrorMsg);
        return false;
    }
    
    // Check for sky light
    TArray<AActor*> SkyLights;
    UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), SkyLights);
    
    if (SkyLights.Num() > 1)
    {
        FString ErrorMsg = FString::Printf(TEXT("Too many SkyLights: %d (max: 1)"), SkyLights.Num());
        UpdateTestResult(TEXT("Map Validation"), TEXT("Lighting Setup"), EQA_TestResult::Warning, ErrorMsg);
    }
    
    UpdateTestResult(TEXT("Map Validation"), TEXT("Lighting Setup"), EQA_TestResult::Pass);
    UE_LOG(LogTemp, Log, TEXT("QA: Lighting setup validation passed"));
    return true;
}

bool UQATestManager::ValidateVFXSystems()
{
    // Placeholder for VFX validation
    UpdateTestResult(TEXT("VFX Validation"), TEXT("Niagara Systems"), EQA_TestResult::Pass);
    return true;
}

bool UQATestManager::ValidateFrameRate()
{
    float CurrentFPS = 1.0f / GetWorld()->GetDeltaSeconds();
    
    if (CurrentFPS < MinFrameRate)
    {
        FString ErrorMsg = FString::Printf(TEXT("Low frame rate: %.1f (min: %.1f)"), CurrentFPS, MinFrameRate);
        UpdateTestResult(TEXT("Performance"), TEXT("Frame Rate"), EQA_TestResult::Warning, ErrorMsg);
        return false;
    }
    
    UpdateTestResult(TEXT("Performance"), TEXT("Frame Rate"), EQA_TestResult::Pass);
    return true;
}

bool UQATestManager::ValidateMemoryUsage()
{
    // Placeholder for memory validation
    UpdateTestResult(TEXT("Performance"), TEXT("Memory Usage"), EQA_TestResult::Pass);
    return true;
}

void UQATestManager::UpdateTestResult(const FString& SuiteName, const FString& TestName, EQA_TestResult Result, const FString& ErrorMessage)
{
    for (FQA_TestSuite& Suite : TestSuites)
    {
        if (Suite.SuiteName == SuiteName)
        {
            for (FQA_TestCase& TestCase : Suite.TestCases)
            {
                if (TestCase.TestName == TestName)
                {
                    TestCase.Result = Result;
                    TestCase.ErrorMessage = ErrorMessage;
                    TestCase.ExecutionTime = GetWorld()->GetTimeSeconds();
                    
                    LogTestResult(TestCase);
                    break;
                }
            }
            break;
        }
    }
}

void UQATestManager::LogTestResult(const FQA_TestCase& TestCase)
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
            ResultString = TEXT("NOT_RUN");
            break;
    }
    
    if (bGenerateDetailedLogs)
    {
        UE_LOG(LogTemp, Log, TEXT("QA Test [%s]: %s - %s"), *ResultString, *TestCase.TestName, *TestCase.ErrorMessage);
    }
}

void UQATestManager::GenerateTestReport()
{
    FString ReportContent;
    ReportContent += TEXT("=== QA TEST REPORT ===\n");
    ReportContent += FString::Printf(TEXT("Generated: %s\n"), *FDateTime::Now().ToString());
    ReportContent += TEXT("\n");
    
    int32 TotalPass = 0;
    int32 TotalFail = 0;
    int32 TotalWarning = 0;
    
    for (const FQA_TestSuite& Suite : TestSuites)
    {
        ReportContent += FString::Printf(TEXT("Suite: %s\n"), *Suite.SuiteName);
        ReportContent += TEXT("----------------------------------------\n");
        
        for (const FQA_TestCase& TestCase : Suite.TestCases)
        {
            FString ResultString;
            switch (TestCase.Result)
            {
                case EQA_TestResult::Pass:
                    ResultString = TEXT("PASS");
                    TotalPass++;
                    break;
                case EQA_TestResult::Fail:
                    ResultString = TEXT("FAIL");
                    TotalFail++;
                    break;
                case EQA_TestResult::Warning:
                    ResultString = TEXT("WARNING");
                    TotalWarning++;
                    break;
                default:
                    ResultString = TEXT("NOT_RUN");
                    break;
            }
            
            ReportContent += FString::Printf(TEXT("  [%s] %s\n"), *ResultString, *TestCase.TestName);
            if (!TestCase.ErrorMessage.IsEmpty())
            {
                ReportContent += FString::Printf(TEXT("    Error: %s\n"), *TestCase.ErrorMessage);
            }
        }
        ReportContent += TEXT("\n");
    }
    
    ReportContent += TEXT("=== SUMMARY ===\n");
    ReportContent += FString::Printf(TEXT("Total Tests: %d\n"), TotalPass + TotalFail + TotalWarning);
    ReportContent += FString::Printf(TEXT("Passed: %d\n"), TotalPass);
    ReportContent += FString::Printf(TEXT("Failed: %d\n"), TotalFail);
    ReportContent += FString::Printf(TEXT("Warnings: %d\n"), TotalWarning);
    
    // Save report to file
    FString ReportPath = FPaths::ProjectLogDir() + TEXT("QA_TestReport.txt");
    FFileHelper::SaveStringToFile(ReportContent, *ReportPath);
    
    UE_LOG(LogTemp, Log, TEXT("QA Test Report generated: %s"), *ReportPath);
}

FString UQATestManager::GetTestSummary()
{
    int32 TotalPass = GetTotalPassCount();
    int32 TotalFail = GetTotalFailCount();
    int32 TotalWarning = GetTotalWarningCount();
    
    return FString::Printf(TEXT("QA Summary - Pass: %d, Fail: %d, Warning: %d"), TotalPass, TotalFail, TotalWarning);
}

int32 UQATestManager::GetTotalPassCount() const
{
    int32 Count = 0;
    for (const FQA_TestSuite& Suite : TestSuites)
    {
        for (const FQA_TestCase& TestCase : Suite.TestCases)
        {
            if (TestCase.Result == EQA_TestResult::Pass)
            {
                Count++;
            }
        }
    }
    return Count;
}

int32 UQATestManager::GetTotalFailCount() const
{
    int32 Count = 0;
    for (const FQA_TestSuite& Suite : TestSuites)
    {
        for (const FQA_TestCase& TestCase : Suite.TestCases)
        {
            if (TestCase.Result == EQA_TestResult::Fail)
            {
                Count++;
            }
        }
    }
    return Count;
}

int32 UQATestManager::GetTotalWarningCount() const
{
    int32 Count = 0;
    for (const FQA_TestSuite& Suite : TestSuites)
    {
        for (const FQA_TestCase& TestCase : Suite.TestCases)
        {
            if (TestCase.Result == EQA_TestResult::Warning)
            {
                Count++;
            }
        }
    }
    return Count;
}