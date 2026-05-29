#include "QATestFramework.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "TimerManager.h"

UQA_TestFramework::UQA_TestFramework()
{
    PrimaryComponentTick.bCanEverTick = false;
    bAutoRunTests = false;
    TestTimeout = 30.0f;
}

void UQA_TestFramework::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize test cases
    AddTestCase(TEXT("ModuleLoading"), TEXT("Test if TranspersonalGame module classes load correctly"));
    AddTestCase(TEXT("ActorSpawning"), TEXT("Test basic actor spawning functionality"));
    AddTestCase(TEXT("VFXIntegration"), TEXT("Test VFX system integration and footstep effects"));
    AddTestCase(TEXT("CharacterSystems"), TEXT("Test character movement and survival stats"));
    AddTestCase(TEXT("WorldGeneration"), TEXT("Test procedural world generation systems"));
    AddTestCase(TEXT("Performance"), TEXT("Test performance metrics and frame rate"));
    
    if (bAutoRunTests)
    {
        // Delay test execution to allow world to fully initialize
        GetWorld()->GetTimerManager().SetTimer(FTimerHandle(), this, &UQA_TestFramework::RunAllTests, 2.0f, false);
    }
}

void UQA_TestFramework::RunAllTests()
{
    UE_LOG(LogTemp, Warning, TEXT("QA Framework: Starting comprehensive test suite"));
    
    // Run each test category
    ValidateVFXSystems();
    ValidateCharacterSystems();
    ValidateWorldGeneration();
    ValidatePerformance();
    
    // Generate final report
    GenerateTestReport();
}

void UQA_TestFramework::RunSingleTest(const FString& TestName)
{
    float StartTime = FPlatformTime::Seconds();
    bool TestResult = false;
    FString ErrorMessage = TEXT("");
    
    if (TestName == TEXT("ModuleLoading"))
    {
        TestResult = TestModuleLoading();
    }
    else if (TestName == TEXT("ActorSpawning"))
    {
        TestResult = TestActorSpawning();
    }
    else if (TestName == TEXT("VFXIntegration"))
    {
        TestResult = TestVFXIntegration();
    }
    else if (TestName == TEXT("CharacterSystems"))
    {
        TestResult = ValidateCharacterSystems();
    }
    else if (TestName == TEXT("WorldGeneration"))
    {
        TestResult = ValidateWorldGeneration();
    }
    else if (TestName == TEXT("Performance"))
    {
        TestResult = ValidatePerformance();
    }
    else
    {
        ErrorMessage = FString::Printf(TEXT("Unknown test: %s"), *TestName);
    }
    
    float ExecutionTime = FPlatformTime::Seconds() - StartTime;
    EQA_TestResult Result = TestResult ? EQA_TestResult::Pass : EQA_TestResult::Fail;
    UpdateTestResult(TestName, Result, ErrorMessage);
    
    // Update execution time
    for (FQA_TestCase& TestCase : TestCases)
    {
        if (TestCase.TestName == TestName)
        {
            TestCase.ExecutionTime = ExecutionTime;
            break;
        }
    }
}

bool UQA_TestFramework::ValidateVFXSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("QA: Validating VFX Systems"));
    
    // Test VFX footstep system
    bool bVFXTestPassed = TestVFXIntegration();
    
    // Check for VFX actors in the world
    UWorld* World = GetWorld();
    if (!World)
    {
        UpdateTestResult(TEXT("VFXIntegration"), EQA_TestResult::Fail, TEXT("World not available"));
        return false;
    }
    
    int32 VFXActorCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetName().Contains(TEXT("VFX")))
        {
            VFXActorCount++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("QA: Found %d VFX actors in world"), VFXActorCount);
    
    bool bResult = bVFXTestPassed && (VFXActorCount > 0);
    UpdateTestResult(TEXT("VFXIntegration"), bResult ? EQA_TestResult::Pass : EQA_TestResult::Warning, 
                    bResult ? TEXT("") : TEXT("VFX systems present but may need validation"));
    
    return bResult;
}

bool UQA_TestFramework::ValidateCharacterSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("QA: Validating Character Systems"));
    
    // Test character class loading
    UClass* CharacterClass = LoadClass<APawn>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
    if (!CharacterClass)
    {
        UpdateTestResult(TEXT("CharacterSystems"), EQA_TestResult::Fail, TEXT("TranspersonalCharacter class not found"));
        return false;
    }
    
    // Test character spawning
    UWorld* World = GetWorld();
    if (World)
    {
        FVector SpawnLocation(2000.0f, 2000.0f, 200.0f);
        APawn* TestCharacter = World->SpawnActor<APawn>(CharacterClass, SpawnLocation, FRotator::ZeroRotator);
        if (TestCharacter)
        {
            UE_LOG(LogTemp, Warning, TEXT("QA: Character spawn test PASSED"));
            TestCharacter->Destroy(); // Clean up test actor
            UpdateTestResult(TEXT("CharacterSystems"), EQA_TestResult::Pass, TEXT(""));
            return true;
        }
    }
    
    UpdateTestResult(TEXT("CharacterSystems"), EQA_TestResult::Fail, TEXT("Character spawn test failed"));
    return false;
}

bool UQA_TestFramework::ValidateWorldGeneration()
{
    UE_LOG(LogTemp, Warning, TEXT("QA: Validating World Generation"));
    
    UWorld* World = GetWorld();
    if (!World)
    {
        UpdateTestResult(TEXT("WorldGeneration"), EQA_TestResult::Fail, TEXT("World not available"));
        return false;
    }
    
    // Count landscape and terrain actors
    int32 LandscapeCount = 0;
    int32 TerrainActorCount = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            FString ActorName = Actor->GetName();
            if (ActorName.Contains(TEXT("Landscape")) || ActorName.Contains(TEXT("Terrain")))
            {
                LandscapeCount++;
            }
            if (ActorName.Contains(TEXT("Tree")) || ActorName.Contains(TEXT("Rock")) || ActorName.Contains(TEXT("Foliage")))
            {
                TerrainActorCount++;
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("QA: Found %d landscape actors, %d terrain detail actors"), LandscapeCount, TerrainActorCount);
    
    bool bResult = (LandscapeCount > 0) || (TerrainActorCount > 5);
    UpdateTestResult(TEXT("WorldGeneration"), bResult ? EQA_TestResult::Pass : EQA_TestResult::Warning,
                    bResult ? TEXT("") : TEXT("Limited world generation content detected"));
    
    return bResult;
}

bool UQA_TestFramework::ValidatePerformance()
{
    UE_LOG(LogTemp, Warning, TEXT("QA: Validating Performance"));
    
    UWorld* World = GetWorld();
    if (!World)
    {
        UpdateTestResult(TEXT("Performance"), EQA_TestResult::Fail, TEXT("World not available"));
        return false;
    }
    
    // Count total actors for performance assessment
    int32 TotalActors = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        TotalActors++;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("QA: Performance check - %d total actors in world"), TotalActors);
    
    // Performance thresholds
    bool bPerformanceGood = TotalActors < 10000; // Reasonable actor count
    EQA_TestResult Result = bPerformanceGood ? EQA_TestResult::Pass : EQA_TestResult::Warning;
    FString Message = bPerformanceGood ? TEXT("") : FString::Printf(TEXT("High actor count: %d"), TotalActors);
    
    UpdateTestResult(TEXT("Performance"), Result, Message);
    return bPerformanceGood;
}

void UQA_TestFramework::GenerateTestReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== QA TEST REPORT ==="));
    
    int32 PassedTests = GetPassedTestCount();
    int32 FailedTests = GetFailedTestCount();
    float SuccessRate = GetOverallSuccessRate();
    
    UE_LOG(LogTemp, Warning, TEXT("Tests Passed: %d"), PassedTests);
    UE_LOG(LogTemp, Warning, TEXT("Tests Failed: %d"), FailedTests);
    UE_LOG(LogTemp, Warning, TEXT("Success Rate: %.1f%%"), SuccessRate);
    
    for (const FQA_TestCase& TestCase : TestCases)
    {
        FString ResultString;
        switch (TestCase.Result)
        {
            case EQA_TestResult::Pass: ResultString = TEXT("PASS"); break;
            case EQA_TestResult::Fail: ResultString = TEXT("FAIL"); break;
            case EQA_TestResult::Warning: ResultString = TEXT("WARN"); break;
            default: ResultString = TEXT("NOT_RUN"); break;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("%s: %s (%.2fs) - %s"), 
               *TestCase.TestName, *ResultString, TestCase.ExecutionTime, *TestCase.ErrorMessage);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== END QA REPORT ==="));
}

int32 UQA_TestFramework::GetPassedTestCount() const
{
    int32 Count = 0;
    for (const FQA_TestCase& TestCase : TestCases)
    {
        if (TestCase.Result == EQA_TestResult::Pass)
        {
            Count++;
        }
    }
    return Count;
}

int32 UQA_TestFramework::GetFailedTestCount() const
{
    int32 Count = 0;
    for (const FQA_TestCase& TestCase : TestCases)
    {
        if (TestCase.Result == EQA_TestResult::Fail)
        {
            Count++;
        }
    }
    return Count;
}

float UQA_TestFramework::GetOverallSuccessRate() const
{
    if (TestCases.Num() == 0) return 0.0f;
    
    int32 PassedCount = GetPassedTestCount();
    return (float)PassedCount / (float)TestCases.Num() * 100.0f;
}

void UQA_TestFramework::AddTestCase(const FString& Name, const FString& Description)
{
    FQA_TestCase NewTest;
    NewTest.TestName = Name;
    NewTest.TestDescription = Description;
    NewTest.Result = EQA_TestResult::NotRun;
    TestCases.Add(NewTest);
}

void UQA_TestFramework::UpdateTestResult(const FString& TestName, EQA_TestResult Result, const FString& ErrorMsg)
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

bool UQA_TestFramework::TestActorSpawning()
{
    UWorld* World = GetWorld();
    if (!World) return false;
    
    // Test basic actor spawning
    FVector SpawnLocation(1500.0f, 1500.0f, 200.0f);
    AActor* TestActor = World->SpawnActor<AActor>(AActor::StaticClass(), SpawnLocation, FRotator::ZeroRotator);
    
    if (TestActor)
    {
        TestActor->Destroy();
        return true;
    }
    
    return false;
}

bool UQA_TestFramework::TestModuleLoading()
{
    // Test loading core TranspersonalGame classes
    UClass* GameStateClass = LoadClass<AGameStateBase>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalGameState"));
    UClass* CharacterClass = LoadClass<APawn>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
    
    return (GameStateClass != nullptr) && (CharacterClass != nullptr);
}

bool UQA_TestFramework::TestVFXIntegration()
{
    // Test VFX footstep system class loading
    UClass* VFXClass = LoadClass<AActor>(nullptr, TEXT("/Script/TranspersonalGame.VFX_FootstepImpactSystem"));
    
    if (VFXClass)
    {
        UWorld* World = GetWorld();
        if (World)
        {
            FVector SpawnLocation(1800.0f, 1800.0f, 200.0f);
            AActor* VFXActor = World->SpawnActor<AActor>(VFXClass, SpawnLocation, FRotator::ZeroRotator);
            if (VFXActor)
            {
                VFXActor->Destroy(); // Clean up test
                return true;
            }
        }
    }
    
    return false;
}

// AQA_TestActor implementation
AQA_TestActor::AQA_TestActor()
{
    PrimaryActorTick.bCanEverTick = false;
    
    TestFramework = CreateDefaultSubobject<UQA_TestFramework>(TEXT("TestFramework"));
    bRunTestsOnBeginPlay = true;
}

void AQA_TestActor::BeginPlay()
{
    Super::BeginPlay();
    
    if (bRunTestsOnBeginPlay && TestFramework)
    {
        // Delay test execution
        GetWorld()->GetTimerManager().SetTimer(FTimerHandle(), this, &AQA_TestActor::StartQATests, 3.0f, false);
    }
}

void AQA_TestActor::StartQATests()
{
    if (TestFramework)
    {
        UE_LOG(LogTemp, Warning, TEXT("QA Test Actor: Starting automated test suite"));
        TestFramework->RunAllTests();
    }
}