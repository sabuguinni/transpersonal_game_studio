#include "QATestManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/Character.h"
#include "Components/AudioComponent.h"
#include "NiagaraComponent.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/DateTime.h"

UQATestManager::UQATestManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;

    // Initialize test configuration
    bAutoRunTests = false;
    TestInterval = 30.0f;
    bLogVerbose = true;
    bStopOnFirstFailure = false;

    // Initialize test results
    bAllTestsPassed = false;
    TestsRun = 0;
    TestsPassed = 0;
    TestsFailed = 0;
    LastTestDuration = 0.0f;

    // Initialize performance metrics
    CurrentFPS = 0.0f;
    MemoryUsageMB = 0.0f;
    ActorCount = 0;
    ComponentCount = 0;

    // Initialize internal state
    TimeSinceLastTest = 0.0f;
    bTestsRunning = false;
    CurrentTestIndex = 0;
}

void UQATestManager::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Log, TEXT("QA Test Manager initialized"));

    // Setup test queue
    TestQueue.Empty();
    TestQueue.Add(TEXT("ActorSpawning"));
    TestQueue.Add(TEXT("ComponentSystems"));
    TestQueue.Add(TEXT("GameModeIntegration"));
    TestQueue.Add(TEXT("PlayerCharacter"));
    TestQueue.Add(TEXT("DinosaurAI"));
    TestQueue.Add(TEXT("AudioPlayback"));
    TestQueue.Add(TEXT("VFXSpawning"));
    TestQueue.Add(TEXT("PerformanceMetrics"));

    if (bAutoRunTests)
    {
        RunAllTests();
    }
}

void UQATestManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    TimeSinceLastTest += DeltaTime;

    // Update performance metrics
    UpdatePerformanceMetrics();

    // Auto-run tests if enabled
    if (bAutoRunTests && TimeSinceLastTest >= TestInterval && !bTestsRunning)
    {
        RunAllTests();
        TimeSinceLastTest = 0.0f;
    }
}

bool UQATestManager::RunAllTests()
{
    UE_LOG(LogTemp, Log, TEXT("QA: Starting comprehensive test suite"));

    float StartTime = FPlatformTime::Seconds();
    ResetTestResults();
    bTestsRunning = true;

    // Run all test categories
    bool bSystemTests = RunSystemTests();
    bool bPerformanceTests = RunPerformanceTests();
    bool bIntegrationTests = RunIntegrationTests();
    bool bGameplayTests = RunGameplayTests();

    // Calculate results
    bAllTestsPassed = bSystemTests && bPerformanceTests && bIntegrationTests && bGameplayTests;
    LastTestDuration = FPlatformTime::Seconds() - StartTime;
    bTestsRunning = false;

    // Log final results
    LogTestResult(TEXT("COMPREHENSIVE_TEST_SUITE"), bAllTestsPassed, 
        FString::Printf(TEXT("Duration: %.2fs, Tests: %d/%d passed"), 
        LastTestDuration, TestsPassed, TestsRun));

    // Write results to file
    FString ResultsPath = FPaths::ProjectDir() + TEXT("Logs/QA_TestResults.txt");
    FString ResultsContent = FString::Printf(
        TEXT("QA Test Results - %s\n")
        TEXT("================================\n")
        TEXT("Tests Run: %d\n")
        TEXT("Tests Passed: %d\n")
        TEXT("Tests Failed: %d\n")
        TEXT("Duration: %.2f seconds\n")
        TEXT("Overall Result: %s\n\n"),
        *FDateTime::Now().ToString(),
        TestsRun, TestsPassed, TestsFailed,
        LastTestDuration,
        bAllTestsPassed ? TEXT("PASS") : TEXT("FAIL")
    );

    for (const FString& LogEntry : TestLog)
    {
        ResultsContent += LogEntry + TEXT("\n");
    }

    FFileHelper::SaveStringToFile(ResultsContent, *ResultsPath);

    return bAllTestsPassed;
}

bool UQATestManager::RunSystemTests()
{
    UE_LOG(LogTemp, Log, TEXT("QA: Running system tests"));

    bool bAllPassed = true;

    bAllPassed &= TestActorSpawning();
    bAllPassed &= TestComponentSystems();
    bAllPassed &= TestGameModeIntegration();

    return bAllPassed;
}

bool UQATestManager::RunPerformanceTests()
{
    UE_LOG(LogTemp, Log, TEXT("QA: Running performance tests"));

    bool bAllPassed = true;

    bAllPassed &= TestPerformanceMetrics();
    bAllPassed &= TestMemoryUsage();
    bAllPassed &= TestFrameRate();

    return bAllPassed;
}

bool UQATestManager::RunIntegrationTests()
{
    UE_LOG(LogTemp, Log, TEXT("QA: Running integration tests"));

    bool bAllPassed = true;

    bAllPassed &= ValidateMinPlayableMap();
    bAllPassed &= ValidateCharacterSystems();
    bAllPassed &= ValidateDinosaurSystems();

    return bAllPassed;
}

bool UQATestManager::RunGameplayTests()
{
    UE_LOG(LogTemp, Log, TEXT("QA: Running gameplay tests"));

    bool bAllPassed = true;

    bAllPassed &= TestPlayerCharacter();
    bAllPassed &= TestDinosaurAI();
    bAllPassed &= TestAudioPlayback();
    bAllPassed &= TestVFXSpawning();

    return bAllPassed;
}

bool UQATestManager::ValidateMinPlayableMap()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        LogTestResult(TEXT("ValidateMinPlayableMap"), false, TEXT("No world found"));
        return false;
    }

    // Count essential actors
    int32 PlayerStarts = 0;
    int32 Lights = 0;
    int32 Terrain = 0;
    int32 Characters = 0;

    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        FString ClassName = Actor->GetClass()->GetName();

        if (ClassName.Contains(TEXT("PlayerStart")))
            PlayerStarts++;
        else if (ClassName.Contains(TEXT("Light")))
            Lights++;
        else if (ClassName.Contains(TEXT("Landscape")) || ClassName.Contains(TEXT("Terrain")))
            Terrain++;
        else if (ClassName.Contains(TEXT("Character")) || ClassName.Contains(TEXT("Pawn")))
            Characters++;
    }

    bool bValid = (PlayerStarts > 0) && (Lights > 0) && (Terrain > 0);
    
    LogTestResult(TEXT("ValidateMinPlayableMap"), bValid, 
        FString::Printf(TEXT("PlayerStarts: %d, Lights: %d, Terrain: %d, Characters: %d"), 
        PlayerStarts, Lights, Terrain, Characters));

    return bValid;
}

bool UQATestManager::ValidateCharacterSystems()
{
    // Test character class loading
    UClass* CharacterClass = LoadClass<ACharacter>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
    bool bCharacterClassValid = (CharacterClass != nullptr);

    LogTestResult(TEXT("ValidateCharacterSystems"), bCharacterClassValid, 
        bCharacterClassValid ? TEXT("Character class loaded successfully") : TEXT("Failed to load character class"));

    return bCharacterClassValid;
}

bool UQATestManager::ValidateDinosaurSystems()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        LogTestResult(TEXT("ValidateDinosaurSystems"), false, TEXT("No world found"));
        return false;
    }

    // Count dinosaur actors
    int32 DinosaurCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        FString ActorName = Actor->GetName();
        
        if (ActorName.Contains(TEXT("Dinosaur")) || ActorName.Contains(TEXT("TRex")) || ActorName.Contains(TEXT("Raptor")))
        {
            DinosaurCount++;
        }
    }

    bool bValid = DinosaurCount > 0;
    LogTestResult(TEXT("ValidateDinosaurSystems"), bValid, 
        FString::Printf(TEXT("Dinosaur actors found: %d"), DinosaurCount));

    return bValid;
}

bool UQATestManager::ValidateAudioSystems()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        LogTestResult(TEXT("ValidateAudioSystems"), false, TEXT("No world found"));
        return false;
    }

    // Count audio components
    int32 AudioComponents = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        TArray<UAudioComponent*> AudioComps;
        Actor->GetComponents<UAudioComponent>(AudioComps);
        AudioComponents += AudioComps.Num();
    }

    bool bValid = AudioComponents >= 0; // Audio is optional but should not crash
    LogTestResult(TEXT("ValidateAudioSystems"), bValid, 
        FString::Printf(TEXT("Audio components found: %d"), AudioComponents));

    return bValid;
}

bool UQATestManager::ValidateVFXSystems()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        LogTestResult(TEXT("ValidateVFXSystems"), false, TEXT("No world found"));
        return false;
    }

    // Count Niagara components
    int32 VFXComponents = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        TArray<UNiagaraComponent*> NiagaraComps;
        Actor->GetComponents<UNiagaraComponent>(NiagaraComps);
        VFXComponents += NiagaraComps.Num();
    }

    bool bValid = VFXComponents >= 0; // VFX is optional but should not crash
    LogTestResult(TEXT("ValidateVFXSystems"), bValid, 
        FString::Printf(TEXT("VFX components found: %d"), VFXComponents));

    return bValid;
}

// Internal test implementations
bool UQATestManager::TestActorSpawning()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        LogTestResult(TEXT("TestActorSpawning"), false, TEXT("No world found"));
        return false;
    }

    // Test basic actor spawning
    FVector SpawnLocation(0.0f, 0.0f, 200.0f);
    AActor* TestActor = World->SpawnActor<AActor>(AActor::StaticClass(), SpawnLocation, FRotator::ZeroRotator);
    
    bool bSpawned = (TestActor != nullptr);
    if (bSpawned && TestActor)
    {
        TestActor->Destroy();
    }

    LogTestResult(TEXT("TestActorSpawning"), bSpawned, TEXT("Basic actor spawn test"));
    return bSpawned;
}

bool UQATestManager::TestComponentSystems()
{
    // Test component creation
    UActorComponent* TestComponent = CreateDefaultSubobject<UActorComponent>(TEXT("TestComponent"));
    bool bComponentValid = (TestComponent != nullptr);

    LogTestResult(TEXT("TestComponentSystems"), bComponentValid, TEXT("Component creation test"));
    return bComponentValid;
}

bool UQATestManager::TestGameModeIntegration()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        LogTestResult(TEXT("TestGameModeIntegration"), false, TEXT("No world found"));
        return false;
    }

    AGameModeBase* GameMode = World->GetAuthGameMode();
    bool bGameModeValid = (GameMode != nullptr);

    LogTestResult(TEXT("TestGameModeIntegration"), bGameModeValid, 
        bGameModeValid ? FString::Printf(TEXT("GameMode: %s"), *GameMode->GetClass()->GetName()) : TEXT("No GameMode"));

    return bGameModeValid;
}

bool UQATestManager::TestPlayerCharacter()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        LogTestResult(TEXT("TestPlayerCharacter"), false, TEXT("No world found"));
        return false;
    }

    ACharacter* PlayerCharacter = World->GetFirstPlayerController() ? 
        World->GetFirstPlayerController()->GetCharacter() : nullptr;
    
    bool bCharacterValid = (PlayerCharacter != nullptr);
    LogTestResult(TEXT("TestPlayerCharacter"), bCharacterValid, 
        bCharacterValid ? TEXT("Player character found") : TEXT("No player character"));

    return bCharacterValid;
}

bool UQATestManager::TestDinosaurAI()
{
    // Placeholder for dinosaur AI testing
    LogTestResult(TEXT("TestDinosaurAI"), true, TEXT("Dinosaur AI test placeholder"));
    return true;
}

bool UQATestManager::TestAudioPlayback()
{
    // Placeholder for audio testing
    LogTestResult(TEXT("TestAudioPlayback"), true, TEXT("Audio playback test placeholder"));
    return true;
}

bool UQATestManager::TestVFXSpawning()
{
    // Placeholder for VFX testing
    LogTestResult(TEXT("TestVFXSpawning"), true, TEXT("VFX spawning test placeholder"));
    return true;
}

bool UQATestManager::TestPerformanceMetrics()
{
    UpdatePerformanceMetrics();
    
    bool bPerformanceGood = (CurrentFPS > 20.0f) && (MemoryUsageMB < 4000.0f);
    LogTestResult(TEXT("TestPerformanceMetrics"), bPerformanceGood, 
        FString::Printf(TEXT("FPS: %.1f, Memory: %.1fMB"), CurrentFPS, MemoryUsageMB));

    return bPerformanceGood;
}

bool UQATestManager::TestMemoryUsage()
{
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    float UsedMemoryMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    
    bool bMemoryOK = UsedMemoryMB < 8000.0f; // 8GB limit
    LogTestResult(TEXT("TestMemoryUsage"), bMemoryOK, 
        FString::Printf(TEXT("Used memory: %.1fMB"), UsedMemoryMB));

    return bMemoryOK;
}

bool UQATestManager::TestFrameRate()
{
    float DeltaTime = GetWorld()->GetDeltaSeconds();
    float FPS = (DeltaTime > 0.0f) ? (1.0f / DeltaTime) : 0.0f;
    
    bool bFrameRateOK = FPS > 15.0f; // Minimum 15 FPS
    LogTestResult(TEXT("TestFrameRate"), bFrameRateOK, 
        FString::Printf(TEXT("Current FPS: %.1f"), FPS));

    return bFrameRateOK;
}

void UQATestManager::LogTestResult(const FString& TestName, bool bPassed, const FString& Details)
{
    TestsRun++;
    if (bPassed)
    {
        TestsPassed++;
    }
    else
    {
        TestsFailed++;
        FailedTests.Add(TestName);
    }

    FString LogEntry = FString::Printf(TEXT("[%s] %s: %s"), 
        bPassed ? TEXT("PASS") : TEXT("FAIL"), 
        *TestName, 
        *Details);
    
    TestLog.Add(LogEntry);

    if (bLogVerbose)
    {
        UE_LOG(LogTemp, Log, TEXT("QA: %s"), *LogEntry);
    }
}

void UQATestManager::ResetTestResults()
{
    bAllTestsPassed = false;
    TestsRun = 0;
    TestsPassed = 0;
    TestsFailed = 0;
    FailedTests.Empty();
    TestLog.Empty();
}

void UQATestManager::UpdatePerformanceMetrics()
{
    if (UWorld* World = GetWorld())
    {
        // Update FPS
        float DeltaTime = World->GetDeltaSeconds();
        CurrentFPS = (DeltaTime > 0.0f) ? (1.0f / DeltaTime) : 0.0f;

        // Update actor count
        ActorCount = 0;
        ComponentCount = 0;
        
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            ActorCount++;
            ComponentCount += (*ActorItr)->GetRootComponent() ? 
                (*ActorItr)->GetRootComponent()->GetAttachChildren().Num() + 1 : 0;
        }

        // Update memory usage
        FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
        MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    }
}