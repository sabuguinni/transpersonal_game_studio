#include "QA_TestFramework.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"

UQA_TestFramework::UQA_TestFramework()
{
    PrimaryComponentTick.bCanEverTick = true;
    bAutoRunTests = false;
    TestInterval = 30.0f;
    bLogTestResults = true;
    LastTestTime = 0.0f;
}

void UQA_TestFramework::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoRunTests)
    {
        RunAllTests();
    }
}

void UQA_TestFramework::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bAutoRunTests && TestInterval > 0.0f)
    {
        LastTestTime += DeltaTime;
        if (LastTestTime >= TestInterval)
        {
            RunAllTests();
            LastTestTime = 0.0f;
        }
    }
}

void UQA_TestFramework::RunAllTests()
{
    UE_LOG(LogTemp, Warning, TEXT("QA_TestFramework: Starting comprehensive test suite"));
    
    ClearTestResults();
    
    // System validation tests
    ValidateCharacterSystem();
    ValidateWorldGeneration();
    ValidateVFXSystems();
    ValidateAudioSystems();
    ValidatePerformance();
    
    // Integration tests
    TestCharacterMovement();
    TestDinosaurAI();
    TestEnvironmentalEffects();
    
    // Log summary
    int32 PassedTests = GetPassedTestCount();
    int32 FailedTests = GetFailedTestCount();
    float OverallScore = GetOverallTestScore();
    
    UE_LOG(LogTemp, Warning, TEXT("QA_TestFramework: Test suite complete - Passed: %d, Failed: %d, Score: %.1f%%"), 
           PassedTests, FailedTests, OverallScore);
}

void UQA_TestFramework::RunSingleTest(const FString& TestName)
{
    if (TestName == TEXT("CharacterSystem"))
    {
        ValidateCharacterSystem();
    }
    else if (TestName == TEXT("WorldGeneration"))
    {
        ValidateWorldGeneration();
    }
    else if (TestName == TEXT("VFXSystems"))
    {
        ValidateVFXSystems();
    }
    else if (TestName == TEXT("AudioSystems"))
    {
        ValidateAudioSystems();
    }
    else if (TestName == TEXT("Performance"))
    {
        ValidatePerformance();
    }
    else
    {
        AddTestResult(TestName, TEXT("Single test execution"), EQA_TestResult::Fail, TEXT("Unknown test name"));
    }
}

void UQA_TestFramework::ClearTestResults()
{
    TestResults.Empty();
}

EQA_TestResult UQA_TestFramework::ValidateCharacterSystem()
{
    FString TestName = TEXT("Character System Validation");
    
    try
    {
        // Check if TranspersonalCharacter class exists
        if (!IsClassLoaded(TEXT("/Script/TranspersonalGame.TranspersonalCharacter")))
        {
            AddTestResult(TestName, TEXT("Validate character class loading"), EQA_TestResult::Fail, TEXT("TranspersonalCharacter class not found"));
            return EQA_TestResult::Fail;
        }
        
        // Check for player character in world
        UWorld* World = GetWorld();
        if (World)
        {
            APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
            if (PlayerPawn)
            {
                AddTestResult(TestName, TEXT("Player character exists in world"), EQA_TestResult::Pass);
                return EQA_TestResult::Pass;
            }
            else
            {
                AddTestResult(TestName, TEXT("Player character validation"), EQA_TestResult::Warning, TEXT("No player pawn found in world"));
                return EQA_TestResult::Warning;
            }
        }
        
        AddTestResult(TestName, TEXT("World access validation"), EQA_TestResult::Fail, TEXT("Could not access world"));
        return EQA_TestResult::Fail;
    }
    catch (...)
    {
        AddTestResult(TestName, TEXT("Character system validation"), EQA_TestResult::Fail, TEXT("Exception during validation"));
        return EQA_TestResult::Fail;
    }
}

EQA_TestResult UQA_TestFramework::ValidateWorldGeneration()
{
    FString TestName = TEXT("World Generation Validation");
    
    try
    {
        // Check if PCGWorldGenerator class exists
        if (!IsClassLoaded(TEXT("/Script/TranspersonalGame.PCGWorldGenerator")))
        {
            AddTestResult(TestName, TEXT("PCG World Generator validation"), EQA_TestResult::Fail, TEXT("PCGWorldGenerator class not found"));
            return EQA_TestResult::Fail;
        }
        
        // Check for terrain actors in world
        UWorld* World = GetWorld();
        if (World)
        {
            TArray<AActor*> FoundActors;
            UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), FoundActors);
            
            if (FoundActors.Num() > 10) // Expect at least some terrain/environment actors
            {
                AddTestResult(TestName, TEXT("World content validation"), EQA_TestResult::Pass, FString::Printf(TEXT("Found %d actors in world"), FoundActors.Num()));
                return EQA_TestResult::Pass;
            }
            else
            {
                AddTestResult(TestName, TEXT("World content validation"), EQA_TestResult::Warning, TEXT("Low actor count in world"));
                return EQA_TestResult::Warning;
            }
        }
        
        AddTestResult(TestName, TEXT("World access validation"), EQA_TestResult::Fail, TEXT("Could not access world"));
        return EQA_TestResult::Fail;
    }
    catch (...)
    {
        AddTestResult(TestName, TEXT("World generation validation"), EQA_TestResult::Fail, TEXT("Exception during validation"));
        return EQA_TestResult::Fail;
    }
}

EQA_TestResult UQA_TestFramework::ValidateVFXSystems()
{
    FString TestName = TEXT("VFX Systems Validation");
    
    try
    {
        // Check if VFX classes exist
        bool bParticleManagerExists = IsClassLoaded(TEXT("/Script/TranspersonalGame.VFX_ParticleManager"));
        bool bEnvironmentalEffectsExists = IsClassLoaded(TEXT("/Script/TranspersonalGame.VFX_EnvironmentalEffects"));
        
        if (bParticleManagerExists && bEnvironmentalEffectsExists)
        {
            AddTestResult(TestName, TEXT("VFX class validation"), EQA_TestResult::Pass, TEXT("All VFX classes loaded successfully"));
            return EQA_TestResult::Pass;
        }
        else if (bParticleManagerExists || bEnvironmentalEffectsExists)
        {
            AddTestResult(TestName, TEXT("VFX class validation"), EQA_TestResult::Warning, TEXT("Some VFX classes missing"));
            return EQA_TestResult::Warning;
        }
        else
        {
            AddTestResult(TestName, TEXT("VFX class validation"), EQA_TestResult::Fail, TEXT("No VFX classes found"));
            return EQA_TestResult::Fail;
        }
    }
    catch (...)
    {
        AddTestResult(TestName, TEXT("VFX systems validation"), EQA_TestResult::Fail, TEXT("Exception during validation"));
        return EQA_TestResult::Fail;
    }
}

EQA_TestResult UQA_TestFramework::ValidateAudioSystems()
{
    FString TestName = TEXT("Audio Systems Validation");
    
    try
    {
        UWorld* World = GetWorld();
        if (World)
        {
            TArray<AActor*> AllActors;
            UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
            
            int32 AudioComponentCount = 0;
            for (AActor* Actor : AllActors)
            {
                if (Actor && Actor->FindComponentByClass<UAudioComponent>())
                {
                    AudioComponentCount++;
                }
            }
            
            if (AudioComponentCount > 0)
            {
                AddTestResult(TestName, TEXT("Audio component validation"), EQA_TestResult::Pass, FString::Printf(TEXT("Found %d audio components"), AudioComponentCount));
                return EQA_TestResult::Pass;
            }
            else
            {
                AddTestResult(TestName, TEXT("Audio component validation"), EQA_TestResult::Warning, TEXT("No audio components found"));
                return EQA_TestResult::Warning;
            }
        }
        
        AddTestResult(TestName, TEXT("Audio systems validation"), EQA_TestResult::Fail, TEXT("Could not access world"));
        return EQA_TestResult::Fail;
    }
    catch (...)
    {
        AddTestResult(TestName, TEXT("Audio systems validation"), EQA_TestResult::Fail, TEXT("Exception during validation"));
        return EQA_TestResult::Fail;
    }
}

EQA_TestResult UQA_TestFramework::ValidatePerformance()
{
    FString TestName = TEXT("Performance Validation");
    
    try
    {
        float CurrentFPS = GetCurrentFPS();
        int32 ActorCount = GetActorCount();
        
        if (CurrentFPS >= 30.0f && ActorCount < 1000)
        {
            AddTestResult(TestName, TEXT("Performance metrics"), EQA_TestResult::Pass, FString::Printf(TEXT("FPS: %.1f, Actors: %d"), CurrentFPS, ActorCount));
            return EQA_TestResult::Pass;
        }
        else if (CurrentFPS >= 20.0f)
        {
            AddTestResult(TestName, TEXT("Performance metrics"), EQA_TestResult::Warning, FString::Printf(TEXT("Low FPS: %.1f, Actors: %d"), CurrentFPS, ActorCount));
            return EQA_TestResult::Warning;
        }
        else
        {
            AddTestResult(TestName, TEXT("Performance metrics"), EQA_TestResult::Fail, FString::Printf(TEXT("Poor performance - FPS: %.1f, Actors: %d"), CurrentFPS, ActorCount));
            return EQA_TestResult::Fail;
        }
    }
    catch (...)
    {
        AddTestResult(TestName, TEXT("Performance validation"), EQA_TestResult::Fail, TEXT("Exception during validation"));
        return EQA_TestResult::Fail;
    }
}

EQA_TestResult UQA_TestFramework::TestCharacterMovement()
{
    FString TestName = TEXT("Character Movement Test");
    
    try
    {
        UWorld* World = GetWorld();
        if (World)
        {
            APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
            if (PlayerPawn)
            {
                FVector InitialLocation = PlayerPawn->GetActorLocation();
                // Simple movement validation - check if character can be moved
                PlayerPawn->SetActorLocation(InitialLocation + FVector(10, 0, 0));
                FVector NewLocation = PlayerPawn->GetActorLocation();
                
                if (!InitialLocation.Equals(NewLocation, 1.0f))
                {
                    PlayerPawn->SetActorLocation(InitialLocation); // Reset position
                    AddTestResult(TestName, TEXT("Character movement capability"), EQA_TestResult::Pass);
                    return EQA_TestResult::Pass;
                }
                else
                {
                    AddTestResult(TestName, TEXT("Character movement capability"), EQA_TestResult::Fail, TEXT("Character could not be moved"));
                    return EQA_TestResult::Fail;
                }
            }
            else
            {
                AddTestResult(TestName, TEXT("Character movement test"), EQA_TestResult::Fail, TEXT("No player pawn found"));
                return EQA_TestResult::Fail;
            }
        }
        
        AddTestResult(TestName, TEXT("Character movement test"), EQA_TestResult::Fail, TEXT("Could not access world"));
        return EQA_TestResult::Fail;
    }
    catch (...)
    {
        AddTestResult(TestName, TEXT("Character movement test"), EQA_TestResult::Fail, TEXT("Exception during test"));
        return EQA_TestResult::Fail;
    }
}

EQA_TestResult UQA_TestFramework::TestDinosaurAI()
{
    FString TestName = TEXT("Dinosaur AI Test");
    
    // Placeholder for dinosaur AI testing
    AddTestResult(TestName, TEXT("Dinosaur AI behavior validation"), EQA_TestResult::Skipped, TEXT("Dinosaur AI system not yet implemented"));
    return EQA_TestResult::Skipped;
}

EQA_TestResult UQA_TestFramework::TestEnvironmentalEffects()
{
    FString TestName = TEXT("Environmental Effects Test");
    
    try
    {
        // Check if environmental effects are present
        if (IsClassLoaded(TEXT("/Script/TranspersonalGame.VFX_EnvironmentalEffects")))
        {
            AddTestResult(TestName, TEXT("Environmental effects availability"), EQA_TestResult::Pass);
            return EQA_TestResult::Pass;
        }
        else
        {
            AddTestResult(TestName, TEXT("Environmental effects test"), EQA_TestResult::Warning, TEXT("Environmental effects class not found"));
            return EQA_TestResult::Warning;
        }
    }
    catch (...)
    {
        AddTestResult(TestName, TEXT("Environmental effects test"), EQA_TestResult::Fail, TEXT("Exception during test"));
        return EQA_TestResult::Fail;
    }
}

float UQA_TestFramework::GetCurrentFPS()
{
    if (GEngine && GEngine->GetGameViewport())
    {
        return 1.0f / GEngine->GetMaxTickRate(0.0f, false);
    }
    return 60.0f; // Default assumption
}

int32 UQA_TestFramework::GetActorCount()
{
    UWorld* World = GetWorld();
    if (World)
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        return AllActors.Num();
    }
    return 0;
}

float UQA_TestFramework::GetMemoryUsage()
{
    // Simplified memory usage estimation
    return 0.0f; // Placeholder
}

int32 UQA_TestFramework::GetPassedTestCount() const
{
    int32 Count = 0;
    for (const FQA_TestCase& TestCase : TestResults)
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
    for (const FQA_TestCase& TestCase : TestResults)
    {
        if (TestCase.Result == EQA_TestResult::Fail)
        {
            Count++;
        }
    }
    return Count;
}

float UQA_TestFramework::GetOverallTestScore() const
{
    if (TestResults.Num() == 0)
    {
        return 0.0f;
    }
    
    int32 PassedTests = GetPassedTestCount();
    return (float)PassedTests / (float)TestResults.Num() * 100.0f;
}

void UQA_TestFramework::AddTestResult(const FString& TestName, const FString& Description, EQA_TestResult Result, const FString& ErrorMessage)
{
    FQA_TestCase NewTestCase;
    NewTestCase.TestName = TestName;
    NewTestCase.TestDescription = Description;
    NewTestCase.Result = Result;
    NewTestCase.ErrorMessage = ErrorMessage;
    NewTestCase.ExecutionTime = FPlatformTime::Seconds();
    
    TestResults.Add(NewTestCase);
    
    if (bLogTestResults)
    {
        LogTestResult(NewTestCase);
    }
}

void UQA_TestFramework::LogTestResult(const FQA_TestCase& TestCase)
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
        case EQA_TestResult::Skipped:
            ResultString = TEXT("SKIP");
            break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("QA_Test [%s] %s: %s - %s"), 
           *ResultString, *TestCase.TestName, *TestCase.TestDescription, *TestCase.ErrorMessage);
}

bool UQA_TestFramework::IsClassLoaded(const FString& ClassName)
{
    UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassName);
    return LoadedClass != nullptr;
}

AActor* UQA_TestFramework::SpawnTestActor(UClass* ActorClass, const FVector& Location)
{
    UWorld* World = GetWorld();
    if (World && ActorClass)
    {
        return World->SpawnActor<AActor>(ActorClass, Location, FRotator::ZeroRotator);
    }
    return nullptr;
}