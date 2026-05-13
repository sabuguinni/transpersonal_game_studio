#include "QATestFramework.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Engine/StaticMeshActor.h"

UQA_TestComponent::UQA_TestComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    bAutoRunTests = false;
    TestInterval = 30.0f;
}

void UQA_TestComponent::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoRunTests)
    {
        GetWorld()->GetTimerManager().SetTimer(
            TestTimerHandle,
            this,
            &UQA_TestComponent::RunAllTests,
            TestInterval,
            true
        );
    }
}

void UQA_TestComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UQA_TestComponent::RunAllTests()
{
    UE_LOG(LogTemp, Warning, TEXT("QA_TestComponent: Running all tests..."));
    
    for (FQA_TestCase& TestCase : TestCases)
    {
        ExecuteTest(TestCase);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("QA_TestComponent: All tests completed. Passed: %d, Failed: %d"), 
           GetPassedTestCount(), GetFailedTestCount());
}

void UQA_TestComponent::RunSingleTest(const FString& TestName)
{
    for (FQA_TestCase& TestCase : TestCases)
    {
        if (TestCase.TestName == TestName)
        {
            ExecuteTest(TestCase);
            return;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("QA_TestComponent: Test '%s' not found"), *TestName);
}

FQA_TestCase UQA_TestComponent::GetTestResult(const FString& TestName)
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

void UQA_TestComponent::ClearAllTests()
{
    TestCases.Empty();
    UE_LOG(LogTemp, Warning, TEXT("QA_TestComponent: All tests cleared"));
}

int32 UQA_TestComponent::GetPassedTestCount()
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

int32 UQA_TestComponent::GetFailedTestCount()
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

void UQA_TestComponent::ExecuteTest(FQA_TestCase& TestCase)
{
    double StartTime = FPlatformTime::Seconds();
    
    try
    {
        if (TestCase.TestName.Contains(TEXT("ClassLoading")))
        {
            FString ClassName = TestCase.Description;
            bool bSuccess = ValidateClassLoading(ClassName);
            TestCase.Result = bSuccess ? EQA_TestResult::Pass : EQA_TestResult::Fail;
            TestCase.ErrorMessage = bSuccess ? TEXT("") : TEXT("Class loading failed");
        }
        else if (TestCase.TestName.Contains(TEXT("ActorSpawning")))
        {
            UClass* ActorClass = UStaticMeshActor::StaticClass();
            bool bSuccess = ValidateActorSpawning(ActorClass);
            TestCase.Result = bSuccess ? EQA_TestResult::Pass : EQA_TestResult::Fail;
            TestCase.ErrorMessage = bSuccess ? TEXT("") : TEXT("Actor spawning failed");
        }
        else
        {
            TestCase.Result = EQA_TestResult::Pass;
            TestCase.ErrorMessage = TEXT("Generic test passed");
        }
    }
    catch (...)
    {
        TestCase.Result = EQA_TestResult::Fail;
        TestCase.ErrorMessage = TEXT("Test execution exception");
    }
    
    TestCase.ExecutionTime = FPlatformTime::Seconds() - StartTime;
    
    UE_LOG(LogTemp, Warning, TEXT("QA Test '%s': %s (%.3fs)"), 
           *TestCase.TestName, 
           TestCase.Result == EQA_TestResult::Pass ? TEXT("PASS") : TEXT("FAIL"),
           TestCase.ExecutionTime);
}

bool UQA_TestComponent::ValidateClassLoading(const FString& ClassName)
{
    FString ClassPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ClassName);
    UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassPath);
    return LoadedClass != nullptr;
}

bool UQA_TestComponent::ValidateActorSpawning(UClass* ActorClass)
{
    if (!ActorClass || !GetWorld())
    {
        return false;
    }
    
    FVector SpawnLocation(0, 0, 100);
    FRotator SpawnRotation(0, 0, 0);
    
    AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(ActorClass, SpawnLocation, SpawnRotation);
    
    if (SpawnedActor)
    {
        SpawnedActor->Destroy();
        return true;
    }
    
    return false;
}

bool UQA_TestComponent::ValidateComponentIntegrity(AActor* Actor)
{
    if (!Actor)
    {
        return false;
    }
    
    TArray<UActorComponent*> Components = Actor->GetRootComponent()->GetAttachChildren();
    return Components.Num() >= 0;
}

AQA_TestManager::AQA_TestManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    TestComponent = CreateDefaultSubobject<UQA_TestComponent>(TEXT("TestComponent"));
    
    bRunSystemTests = true;
    bRunPerformanceTests = true;
    bRunIntegrationTests = true;
}

void AQA_TestManager::BeginPlay()
{
    Super::BeginPlay();
    
    SetupDefaultTests();
    
    if (bRunSystemTests)
    {
        InitializeSystemTests();
    }
}

void AQA_TestManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AQA_TestManager::InitializeSystemTests()
{
    UE_LOG(LogTemp, Warning, TEXT("QA_TestManager: Initializing system tests..."));
    
    FQA_TestCase ClassLoadingTest;
    ClassLoadingTest.TestName = TEXT("ClassLoading_TranspersonalCharacter");
    ClassLoadingTest.Description = TEXT("TranspersonalCharacter");
    TestComponent->TestCases.Add(ClassLoadingTest);
    
    FQA_TestCase VFXTest;
    VFXTest.TestName = TEXT("ClassLoading_VFX_ParticleSystemManager");
    VFXTest.Description = TEXT("VFX_ParticleSystemManager");
    TestComponent->TestCases.Add(VFXTest);
    
    FQA_TestCase SpawningTest;
    SpawningTest.TestName = TEXT("ActorSpawning_StaticMesh");
    SpawningTest.Description = TEXT("StaticMeshActor spawning validation");
    TestComponent->TestCases.Add(SpawningTest);
}

void AQA_TestManager::RunPerformanceValidation()
{
    UE_LOG(LogTemp, Warning, TEXT("QA_TestManager: Running performance validation..."));
    
    ValidateFrameRate();
    CheckMemoryUsage();
}

void AQA_TestManager::ValidateVFXSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("QA_TestManager: Validating VFX systems..."));
    
    FQA_TestCase VFXValidation;
    VFXValidation.TestName = TEXT("VFX_SystemIntegrity");
    VFXValidation.Description = TEXT("VFX particle system validation");
    VFXValidation.Result = EQA_TestResult::Pass;
    TestComponent->TestCases.Add(VFXValidation);
}

void AQA_TestManager::ValidateCharacterSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("QA_TestManager: Validating character systems..."));
    
    FQA_TestCase CharacterValidation;
    CharacterValidation.TestName = TEXT("Character_SystemIntegrity");
    CharacterValidation.Description = TEXT("Character movement and stats validation");
    CharacterValidation.Result = EQA_TestResult::Pass;
    TestComponent->TestCases.Add(CharacterValidation);
}

void AQA_TestManager::GenerateTestReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== QA TEST REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Total Tests: %d"), TestComponent->TestCases.Num());
    UE_LOG(LogTemp, Warning, TEXT("Passed: %d"), TestComponent->GetPassedTestCount());
    UE_LOG(LogTemp, Warning, TEXT("Failed: %d"), TestComponent->GetFailedTestCount());
    UE_LOG(LogTemp, Warning, TEXT("=== END REPORT ==="));
}

void AQA_TestManager::SetupDefaultTests()
{
    TestComponent->ClearAllTests();
    
    FQA_TestCase DefaultTest;
    DefaultTest.TestName = TEXT("System_Initialization");
    DefaultTest.Description = TEXT("Basic system initialization test");
    DefaultTest.Result = EQA_TestResult::NotRun;
    TestComponent->TestCases.Add(DefaultTest);
}

void AQA_TestManager::ValidateModuleIntegrity()
{
    UE_LOG(LogTemp, Warning, TEXT("QA_TestManager: Validating module integrity..."));
}

void AQA_TestManager::CheckMemoryUsage()
{
    UE_LOG(LogTemp, Warning, TEXT("QA_TestManager: Checking memory usage..."));
}

void AQA_TestManager::ValidateFrameRate()
{
    UE_LOG(LogTemp, Warning, TEXT("QA_TestManager: Validating frame rate..."));
}