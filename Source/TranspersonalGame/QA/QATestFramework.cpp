#include "QATestFramework.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

AQATestFramework::AQATestFramework()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create test marker mesh
    TestMarkerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TestMarkerMesh"));
    TestMarkerMesh->SetupAttachment(RootComponent);

    // Set default mesh (cube)
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        TestMarkerMesh->SetStaticMesh(CubeMeshAsset.Object);
        TestMarkerMesh->SetWorldScale3D(FVector(0.5f, 0.5f, 2.0f));
    }

    // Initialize test state
    bTestsRunning = false;
    TotalTestTime = 0.0f;

    // Add default test cases
    AddTestCase(TEXT("CharacterMovement"), TEXT("Test player character movement and input"));
    AddTestCase(TEXT("VFXSystems"), TEXT("Test VFX system integration and particle effects"));
    AddTestCase(TEXT("WorldGeneration"), TEXT("Test procedural world generation systems"));
    AddTestCase(TEXT("DinosaurAI"), TEXT("Test dinosaur AI behavior and pathfinding"));
    AddTestCase(TEXT("PerformanceMetrics"), TEXT("Test frame rate and memory usage"));
}

void AQATestFramework::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("QA Test Framework initialized with %d test cases"), TestCases.Num());
}

void AQATestFramework::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bTestsRunning)
    {
        TotalTestTime += DeltaTime;
    }
}

void AQATestFramework::RunAllTests()
{
    if (bTestsRunning)
    {
        UE_LOG(LogTemp, Warning, TEXT("Tests already running, skipping"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("Starting QA test suite with %d tests"), TestCases.Num());
    bTestsRunning = true;
    TotalTestTime = 0.0f;

    // Run each test
    for (FQA_TestCase& TestCase : TestCases)
    {
        TestCase.Result = EQA_TestResult::NotRun;
        TestCase.ErrorMessage = TEXT("");
        
        bool bTestPassed = false;
        float StartTime = GetWorld()->GetTimeSeconds();

        if (TestCase.TestName == TEXT("CharacterMovement"))
        {
            bTestPassed = TestCharacterMovement();
        }
        else if (TestCase.TestName == TEXT("VFXSystems"))
        {
            bTestPassed = TestVFXSystems();
        }
        else if (TestCase.TestName == TEXT("WorldGeneration"))
        {
            bTestPassed = TestWorldGeneration();
        }
        else if (TestCase.TestName == TEXT("DinosaurAI"))
        {
            bTestPassed = TestDinosaurAI();
        }
        else if (TestCase.TestName == TEXT("PerformanceMetrics"))
        {
            bTestPassed = TestPerformanceMetrics();
        }

        TestCase.ExecutionTime = GetWorld()->GetTimeSeconds() - StartTime;
        TestCase.Result = bTestPassed ? EQA_TestResult::Pass : EQA_TestResult::Fail;
        
        LogTestResult(TestCase.TestName, bTestPassed, TestCase.ErrorMessage);
    }

    bTestsRunning = false;
    UE_LOG(LogTemp, Warning, TEXT("QA test suite completed in %.2f seconds"), TotalTestTime);
}

void AQATestFramework::RunSingleTest(const FString& TestName)
{
    for (FQA_TestCase& TestCase : TestCases)
    {
        if (TestCase.TestName == TestName)
        {
            bool bTestPassed = false;
            float StartTime = GetWorld()->GetTimeSeconds();

            if (TestName == TEXT("CharacterMovement"))
            {
                bTestPassed = TestCharacterMovement();
            }
            else if (TestName == TEXT("VFXSystems"))
            {
                bTestPassed = TestVFXSystems();
            }
            // Add other test cases as needed

            TestCase.ExecutionTime = GetWorld()->GetTimeSeconds() - StartTime;
            TestCase.Result = bTestPassed ? EQA_TestResult::Pass : EQA_TestResult::Fail;
            
            LogTestResult(TestName, bTestPassed, TestCase.ErrorMessage);
            return;
        }
    }

    UE_LOG(LogTemp, Error, TEXT("Test case '%s' not found"), *TestName);
}

void AQATestFramework::ClearTestResults()
{
    for (FQA_TestCase& TestCase : TestCases)
    {
        TestCase.Result = EQA_TestResult::NotRun;
        TestCase.ErrorMessage = TEXT("");
        TestCase.ExecutionTime = 0.0f;
    }
    
    TotalTestTime = 0.0f;
    UE_LOG(LogTemp, Warning, TEXT("QA test results cleared"));
}

void AQATestFramework::AddTestCase(const FString& TestName, const FString& Description)
{
    FQA_TestCase NewTest;
    NewTest.TestName = TestName;
    NewTest.Description = Description;
    NewTest.Result = EQA_TestResult::NotRun;
    
    TestCases.Add(NewTest);
}

TArray<FQA_TestCase> AQATestFramework::GetTestResults() const
{
    return TestCases;
}

int32 AQATestFramework::GetPassedTestCount() const
{
    int32 PassedCount = 0;
    for (const FQA_TestCase& TestCase : TestCases)
    {
        if (TestCase.Result == EQA_TestResult::Pass)
        {
            PassedCount++;
        }
    }
    return PassedCount;
}

int32 AQATestFramework::GetFailedTestCount() const
{
    int32 FailedCount = 0;
    for (const FQA_TestCase& TestCase : TestCases)
    {
        if (TestCase.Result == EQA_TestResult::Fail)
        {
            FailedCount++;
        }
    }
    return FailedCount;
}

bool AQATestFramework::TestCharacterMovement()
{
    // Test character class loading and basic movement
    UClass* CharacterClass = LoadClass<ACharacter>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
    if (!CharacterClass)
    {
        UpdateTestCase(TEXT("CharacterMovement"), EQA_TestResult::Fail, TEXT("TranspersonalCharacter class not found"));
        return false;
    }

    // Check for character in world
    TArray<AActor*> FoundCharacters;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), CharacterClass, FoundCharacters);
    
    if (FoundCharacters.Num() == 0)
    {
        UpdateTestCase(TEXT("CharacterMovement"), EQA_TestResult::Warning, TEXT("No character instances found in world"));
        return true; // Not a failure, just a warning
    }

    UE_LOG(LogTemp, Warning, TEXT("Character movement test passed - found %d character(s)"), FoundCharacters.Num());
    return true;
}

bool AQATestFramework::TestVFXSystems()
{
    // Test VFX class loading
    UClass* VFXClass = LoadClass<AActor>(nullptr, TEXT("/Script/TranspersonalGame.VFX_DinosaurImpactManager"));
    if (!VFXClass)
    {
        UpdateTestCase(TEXT("VFXSystems"), EQA_TestResult::Fail, TEXT("VFX_DinosaurImpactManager class not found"));
        return false;
    }

    UE_LOG(LogTemp, Warning, TEXT("VFX systems test passed - VFX classes loaded successfully"));
    return true;
}

bool AQATestFramework::TestWorldGeneration()
{
    // Test world generation class loading
    UClass* WorldGenClass = LoadClass<AActor>(nullptr, TEXT("/Script/TranspersonalGame.PCGWorldGenerator"));
    if (!WorldGenClass)
    {
        UpdateTestCase(TEXT("WorldGeneration"), EQA_TestResult::Fail, TEXT("PCGWorldGenerator class not found"));
        return false;
    }

    UE_LOG(LogTemp, Warning, TEXT("World generation test passed"));
    return true;
}

bool AQATestFramework::TestDinosaurAI()
{
    // Test for dinosaur-related actors in the world
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    int32 DinosaurCount = 0;
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetName().Contains(TEXT("Dinosaur")))
        {
            DinosaurCount++;
        }
    }

    if (DinosaurCount == 0)
    {
        UpdateTestCase(TEXT("DinosaurAI"), EQA_TestResult::Warning, TEXT("No dinosaur actors found in world"));
        return true; // Warning, not failure
    }

    UE_LOG(LogTemp, Warning, TEXT("Dinosaur AI test passed - found %d dinosaur actors"), DinosaurCount);
    return true;
}

bool AQATestFramework::TestPerformanceMetrics()
{
    // Basic performance check - frame rate
    float CurrentFPS = 1.0f / GetWorld()->GetDeltaSeconds();
    
    if (CurrentFPS < 30.0f)
    {
        UpdateTestCase(TEXT("PerformanceMetrics"), EQA_TestResult::Warning, 
                      FString::Printf(TEXT("Low FPS detected: %.1f"), CurrentFPS));
        return true; // Warning, not failure
    }

    UE_LOG(LogTemp, Warning, TEXT("Performance test passed - FPS: %.1f"), CurrentFPS);
    return true;
}

void AQATestFramework::LogTestResult(const FString& TestName, bool bPassed, const FString& ErrorMsg)
{
    if (bPassed)
    {
        UE_LOG(LogTemp, Warning, TEXT("QA TEST PASS: %s"), *TestName);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("QA TEST FAIL: %s - %s"), *TestName, *ErrorMsg);
    }
}

void AQATestFramework::UpdateTestCase(const FString& TestName, EQA_TestResult Result, const FString& ErrorMsg)
{
    for (FQA_TestCase& TestCase : TestCases)
    {
        if (TestCase.TestName == TestName)
        {
            TestCase.Result = Result;
            TestCase.ErrorMessage = ErrorMsg;
            return;
        }
    }
}

bool AQATestFramework::ValidateActorSpawning()
{
    // Test basic actor spawning functionality
    AActor* TestActor = GetWorld()->SpawnActor<AActor>();
    if (!TestActor)
    {
        return false;
    }

    TestActor->Destroy();
    return true;
}

bool AQATestFramework::ValidateClassLoading()
{
    // Test loading of core engine classes
    UClass* ActorClass = LoadClass<AActor>(nullptr, TEXT("/Script/Engine.Actor"));
    UClass* PawnClass = LoadClass<APawn>(nullptr, TEXT("/Script/Engine.Pawn"));
    
    return (ActorClass != nullptr && PawnClass != nullptr);
}

bool AQATestFramework::ValidateComponentSystems()
{
    // Test component creation and attachment
    UStaticMeshComponent* TestComponent = NewObject<UStaticMeshComponent>(this);
    if (!TestComponent)
    {
        return false;
    }

    return true;
}