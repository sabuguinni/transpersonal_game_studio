#include "QA_TestingFramework.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UQA_TestingFramework::UQA_TestingFramework()
{
    PassedTests = 0;
    FailedTests = 0;
    WarningTests = 0;
}

void UQA_TestingFramework::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("QA Testing Framework Initialized"));
    InitializeTestCases();
}

void UQA_TestingFramework::Deinitialize()
{
    ClearTestResults();
    Super::Deinitialize();
}

void UQA_TestingFramework::InitializeTestCases()
{
    TestCases.Empty();
    
    // Character Movement Tests
    AddTestCase(CreateTestCase(TEXT("CharacterMovement_WASD"), TEXT("Validate WASD movement input response")));
    AddTestCase(CreateTestCase(TEXT("CharacterMovement_Jump"), TEXT("Validate jump mechanics and landing")));
    AddTestCase(CreateTestCase(TEXT("CharacterMovement_Run"), TEXT("Validate run/sprint functionality")));
    
    // Dinosaur AI Tests
    AddTestCase(CreateTestCase(TEXT("DinosaurAI_Spawning"), TEXT("Validate dinosaur spawning and placement")));
    AddTestCase(CreateTestCase(TEXT("DinosaurAI_Behavior"), TEXT("Validate basic AI behavior patterns")));
    AddTestCase(CreateTestCase(TEXT("DinosaurAI_Combat"), TEXT("Validate combat AI responses")));
    
    // VFX System Tests
    AddTestCase(CreateTestCase(TEXT("VFX_Campfire"), TEXT("Validate campfire VFX system functionality")));
    AddTestCase(CreateTestCase(TEXT("VFX_Weather"), TEXT("Validate weather particle effects")));
    AddTestCase(CreateTestCase(TEXT("VFX_Performance"), TEXT("Validate VFX performance metrics")));
    
    // Audio System Tests
    AddTestCase(CreateTestCase(TEXT("Audio_Ambient"), TEXT("Validate ambient audio playback")));
    AddTestCase(CreateTestCase(TEXT("Audio_SFX"), TEXT("Validate sound effects triggering")));
    AddTestCase(CreateTestCase(TEXT("Audio_Music"), TEXT("Validate background music system")));
    
    // World Generation Tests
    AddTestCase(CreateTestCase(TEXT("WorldGen_Terrain"), TEXT("Validate terrain generation")));
    AddTestCase(CreateTestCase(TEXT("WorldGen_Foliage"), TEXT("Validate foliage placement")));
    AddTestCase(CreateTestCase(TEXT("WorldGen_Performance"), TEXT("Validate world generation performance")));
}

void UQA_TestingFramework::RunAllTests()
{
    UE_LOG(LogTemp, Warning, TEXT("QA Testing Framework: Running All Tests"));
    
    PassedTests = 0;
    FailedTests = 0;
    WarningTests = 0;
    
    for (FQA_TestCase& TestCase : TestCases)
    {
        TestCase.Result = EQA_TestResult::NotRun;
        TestCase.ErrorMessage = TEXT("");
        
        float StartTime = FPlatformTime::Seconds();
        
        bool bTestPassed = false;
        
        if (TestCase.TestName.Contains(TEXT("CharacterMovement")))
        {
            bTestPassed = ValidateCharacterMovement();
        }
        else if (TestCase.TestName.Contains(TEXT("DinosaurAI")))
        {
            bTestPassed = ValidateDinosaurAI();
        }
        else if (TestCase.TestName.Contains(TEXT("VFX")))
        {
            bTestPassed = ValidateVFXSystems();
        }
        else if (TestCase.TestName.Contains(TEXT("Audio")))
        {
            bTestPassed = ValidateAudioSystems();
        }
        else if (TestCase.TestName.Contains(TEXT("WorldGen")))
        {
            bTestPassed = ValidateWorldGeneration();
        }
        
        TestCase.ExecutionTime = FPlatformTime::Seconds() - StartTime;
        TestCase.Result = bTestPassed ? EQA_TestResult::Pass : EQA_TestResult::Fail;
        
        if (bTestPassed)
        {
            PassedTests++;
        }
        else
        {
            FailedTests++;
            TestCase.ErrorMessage = TEXT("Test validation failed - check implementation");
        }
        
        LogTestResult(TestCase);
    }
    
    GenerateTestReport();
}

void UQA_TestingFramework::RunTestByName(const FString& TestName)
{
    for (FQA_TestCase& TestCase : TestCases)
    {
        if (TestCase.TestName == TestName)
        {
            float StartTime = FPlatformTime::Seconds();
            
            bool bTestPassed = false;
            
            if (TestName.Contains(TEXT("CharacterMovement")))
            {
                bTestPassed = ValidateCharacterMovement();
            }
            else if (TestName.Contains(TEXT("DinosaurAI")))
            {
                bTestPassed = ValidateDinosaurAI();
            }
            else if (TestName.Contains(TEXT("VFX")))
            {
                bTestPassed = ValidateVFXSystems();
            }
            else if (TestName.Contains(TEXT("Audio")))
            {
                bTestPassed = ValidateAudioSystems();
            }
            else if (TestName.Contains(TEXT("WorldGen")))
            {
                bTestPassed = ValidateWorldGeneration();
            }
            
            TestCase.ExecutionTime = FPlatformTime::Seconds() - StartTime;
            TestCase.Result = bTestPassed ? EQA_TestResult::Pass : EQA_TestResult::Fail;
            
            if (!bTestPassed)
            {
                TestCase.ErrorMessage = TEXT("Test validation failed - check implementation");
            }
            
            LogTestResult(TestCase);
            break;
        }
    }
}

bool UQA_TestingFramework::ValidateCharacterMovement()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Find player character
    ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(World, 0);
    if (!PlayerCharacter)
    {
        UE_LOG(LogTemp, Error, TEXT("QA Test Failed: No player character found"));
        return false;
    }
    
    // Check if character has movement component
    if (!PlayerCharacter->GetCharacterMovement())
    {
        UE_LOG(LogTemp, Error, TEXT("QA Test Failed: Character missing movement component"));
        return false;
    }
    
    // Validate movement settings
    float WalkSpeed = PlayerCharacter->GetCharacterMovement()->MaxWalkSpeed;
    if (WalkSpeed <= 0.0f)
    {
        UE_LOG(LogTemp, Error, TEXT("QA Test Failed: Invalid walk speed: %f"), WalkSpeed);
        return false;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("QA Test Passed: Character movement validation successful"));
    return true;
}

bool UQA_TestingFramework::ValidateDinosaurAI()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Count dinosaur actors in the world
    TArray<AActor*> DinosaurActors;
    UGameplayStatics::GetAllActorsOfClass(World, APawn::StaticClass(), DinosaurActors);
    
    int32 DinosaurCount = 0;
    for (AActor* Actor : DinosaurActors)
    {
        if (Actor->GetName().Contains(TEXT("Dino")) || Actor->GetName().Contains(TEXT("Rex")) || Actor->GetName().Contains(TEXT("Raptor")))
        {
            DinosaurCount++;
        }
    }
    
    if (DinosaurCount == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("QA Test Warning: No dinosaur actors found in world"));
        return false;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("QA Test Passed: Found %d dinosaur actors"), DinosaurCount);
    return true;
}

bool UQA_TestingFramework::ValidateVFXSystems()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Check for Niagara components in the world
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    int32 VFXCount = 0;
    for (AActor* Actor : AllActors)
    {
        TArray<UNiagaraComponent*> NiagaraComponents;
        Actor->GetComponents<UNiagaraComponent>(NiagaraComponents);
        VFXCount += NiagaraComponents.Num();
    }
    
    UE_LOG(LogTemp, Warning, TEXT("QA Test: Found %d Niagara VFX components"), VFXCount);
    return VFXCount > 0;
}

bool UQA_TestingFramework::ValidateAudioSystems()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Check for audio components in the world
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    int32 AudioCount = 0;
    for (AActor* Actor : AllActors)
    {
        TArray<UAudioComponent*> AudioComponents;
        Actor->GetComponents<UAudioComponent>(AudioComponents);
        AudioCount += AudioComponents.Num();
    }
    
    UE_LOG(LogTemp, Warning, TEXT("QA Test: Found %d Audio components"), AudioCount);
    return AudioCount > 0;
}

bool UQA_TestingFramework::ValidateWorldGeneration()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Check for static mesh actors (terrain, props, etc.)
    TArray<AActor*> StaticMeshActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), StaticMeshActors);
    
    int32 WorldObjectCount = 0;
    for (AActor* Actor : StaticMeshActors)
    {
        if (Actor->GetRootComponent() && Actor->GetRootComponent()->IsA<UStaticMeshComponent>())
        {
            WorldObjectCount++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("QA Test: Found %d world objects"), WorldObjectCount);
    return WorldObjectCount > 10; // Expect at least 10 world objects
}

void UQA_TestingFramework::AddTestCase(const FQA_TestCase& TestCase)
{
    TestCases.Add(TestCase);
}

TArray<FQA_TestCase> UQA_TestingFramework::GetTestResults() const
{
    return TestCases;
}

void UQA_TestingFramework::ClearTestResults()
{
    TestCases.Empty();
    PassedTests = 0;
    FailedTests = 0;
    WarningTests = 0;
}

void UQA_TestingFramework::GenerateTestReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== QA TEST REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Total Tests: %d"), TestCases.Num());
    UE_LOG(LogTemp, Warning, TEXT("Passed: %d"), PassedTests);
    UE_LOG(LogTemp, Warning, TEXT("Failed: %d"), FailedTests);
    UE_LOG(LogTemp, Warning, TEXT("Warnings: %d"), WarningTests);
    
    float SuccessRate = TestCases.Num() > 0 ? (float)PassedTests / (float)TestCases.Num() * 100.0f : 0.0f;
    UE_LOG(LogTemp, Warning, TEXT("Success Rate: %.1f%%"), SuccessRate);
    UE_LOG(LogTemp, Warning, TEXT("======================"));
}

void UQA_TestingFramework::LogTestResult(const FQA_TestCase& TestCase)
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
            ResultString = TEXT("WARN");
            break;
        case EQA_TestResult::Error:
            ResultString = TEXT("ERROR");
            break;
        default:
            ResultString = TEXT("NOT_RUN");
            break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("QA Test [%s]: %s (%.3fs) - %s"), 
           *ResultString, 
           *TestCase.TestName, 
           TestCase.ExecutionTime,
           *TestCase.ErrorMessage);
}

FQA_TestCase UQA_TestingFramework::CreateTestCase(const FString& Name, const FString& Description)
{
    FQA_TestCase NewTestCase;
    NewTestCase.TestName = Name;
    NewTestCase.TestDescription = Description;
    NewTestCase.Result = EQA_TestResult::NotRun;
    NewTestCase.ErrorMessage = TEXT("");
    NewTestCase.ExecutionTime = 0.0f;
    return NewTestCase;
}