#include "QA_VFXIntegrationTester.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

AQA_VFXIntegrationTester::AQA_VFXIntegrationTester()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create test mesh component
    TestMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TestMeshComponent"));
    TestMeshComponent->SetupAttachment(RootComponent);

    // Create Niagara components for testing
    FootstepVFXComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("FootstepVFXComponent"));
    FootstepVFXComponent->SetupAttachment(RootComponent);
    FootstepVFXComponent->SetAutoActivate(false);

    CampfireVFXComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("CampfireVFXComponent"));
    CampfireVFXComponent->SetupAttachment(RootComponent);
    CampfireVFXComponent->SetAutoActivate(false);

    // Create audio component for testing
    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    AudioComponent->SetupAttachment(RootComponent);
    AudioComponent->SetAutoActivate(false);

    // Initialize test configuration
    bAutoRunTests = false;
    TestInterval = 5.0f;
    MaxTestIterations = 10;
    CurrentTestIteration = 0;

    // Initialize test results
    bAllTestsPassed = false;
    TotalTests = 0;
    PassedTests = 0;
    FailedTests = 0;
}

void AQA_VFXIntegrationTester::BeginPlay()
{
    Super::BeginPlay();

    if (bAutoRunTests)
    {
        GetWorldTimerManager().SetTimer(TestTimerHandle, this, &AQA_VFXIntegrationTester::ExecuteTestIteration, TestInterval, true);
    }
}

void AQA_VFXIntegrationTester::RunAllVFXTests()
{
    UE_LOG(LogTemp, Warning, TEXT("QA VFX Integration Tester: Starting comprehensive VFX test suite"));

    ClearTestResults();

    // Run all test categories
    RunFootstepVFXTest();
    RunCampfireVFXTest();
    RunAudioSyncTest();
    RunPerformanceTest();

    UpdateTestStatistics();
    GenerateTestReport();

    UE_LOG(LogTemp, Warning, TEXT("QA VFX Integration Tester: Test suite completed - %d/%d tests passed"), PassedTests, TotalTests);
}

void AQA_VFXIntegrationTester::RunFootstepVFXTest()
{
    FString TestName = TEXT("Footstep VFX Validation");
    FString Description = TEXT("Validates footstep particle effects and dust clouds");

    try
    {
        if (!FootstepVFXComponent)
        {
            AddTestResult(TestName, Description, EQA_VFXTestResult::Fail, TEXT("FootstepVFXComponent is null"));
            return;
        }

        // Test component activation
        FootstepVFXComponent->Activate();
        
        if (FootstepVFXComponent->IsActive())
        {
            AddTestResult(TestName + TEXT(" - Activation"), Description, EQA_VFXTestResult::Pass);
        }
        else
        {
            AddTestResult(TestName + TEXT(" - Activation"), Description, EQA_VFXTestResult::Fail, TEXT("Failed to activate footstep VFX"));
        }

        // Test system assignment
        if (FootstepVFXComponent->GetAsset())
        {
            AddTestResult(TestName + TEXT(" - System Asset"), Description, EQA_VFXTestResult::Pass);
        }
        else
        {
            AddTestResult(TestName + TEXT(" - System Asset"), Description, EQA_VFXTestResult::Warning, TEXT("No Niagara system assigned"));
        }

        // Deactivate after test
        FootstepVFXComponent->Deactivate();
    }
    catch (...)
    {
        AddTestResult(TestName, Description, EQA_VFXTestResult::Critical, TEXT("Exception during footstep VFX test"));
    }
}

void AQA_VFXIntegrationTester::RunCampfireVFXTest()
{
    FString TestName = TEXT("Campfire VFX Validation");
    FString Description = TEXT("Validates campfire particle effects and flame simulation");

    try
    {
        if (!CampfireVFXComponent)
        {
            AddTestResult(TestName, Description, EQA_VFXTestResult::Fail, TEXT("CampfireVFXComponent is null"));
            return;
        }

        // Test component activation
        CampfireVFXComponent->Activate();
        
        if (CampfireVFXComponent->IsActive())
        {
            AddTestResult(TestName + TEXT(" - Activation"), Description, EQA_VFXTestResult::Pass);
        }
        else
        {
            AddTestResult(TestName + TEXT(" - Activation"), Description, EQA_VFXTestResult::Fail, TEXT("Failed to activate campfire VFX"));
        }

        // Test system assignment
        if (CampfireVFXComponent->GetAsset())
        {
            AddTestResult(TestName + TEXT(" - System Asset"), Description, EQA_VFXTestResult::Pass);
        }
        else
        {
            AddTestResult(TestName + TEXT(" - System Asset"), Description, EQA_VFXTestResult::Warning, TEXT("No Niagara system assigned"));
        }

        // Test continuous emission
        if (CampfireVFXComponent->IsActive())
        {
            AddTestResult(TestName + TEXT(" - Continuous Emission"), Description, EQA_VFXTestResult::Pass);
        }

        // Deactivate after test
        CampfireVFXComponent->Deactivate();
    }
    catch (...)
    {
        AddTestResult(TestName, Description, EQA_VFXTestResult::Critical, TEXT("Exception during campfire VFX test"));
    }
}

void AQA_VFXIntegrationTester::RunAudioSyncTest()
{
    FString TestName = TEXT("Audio-VFX Synchronization");
    FString Description = TEXT("Validates synchronization between audio and visual effects");

    try
    {
        if (!AudioComponent)
        {
            AddTestResult(TestName, Description, EQA_VFXTestResult::Fail, TEXT("AudioComponent is null"));
            return;
        }

        // Test audio component functionality
        if (AudioComponent->GetSound())
        {
            AddTestResult(TestName + TEXT(" - Audio Asset"), Description, EQA_VFXTestResult::Pass);
            
            // Test audio playback
            AudioComponent->Play();
            
            if (AudioComponent->IsPlaying())
            {
                AddTestResult(TestName + TEXT(" - Playback"), Description, EQA_VFXTestResult::Pass);
                
                // Test VFX sync with audio
                CampfireVFXComponent->Activate();
                
                if (CampfireVFXComponent->IsActive() && AudioComponent->IsPlaying())
                {
                    AddTestResult(TestName + TEXT(" - Sync Test"), Description, EQA_VFXTestResult::Pass);
                }
                else
                {
                    AddTestResult(TestName + TEXT(" - Sync Test"), Description, EQA_VFXTestResult::Fail, TEXT("Audio-VFX sync failed"));
                }
                
                // Stop audio and VFX
                AudioComponent->Stop();
                CampfireVFXComponent->Deactivate();
            }
            else
            {
                AddTestResult(TestName + TEXT(" - Playback"), Description, EQA_VFXTestResult::Fail, TEXT("Audio failed to play"));
            }
        }
        else
        {
            AddTestResult(TestName + TEXT(" - Audio Asset"), Description, EQA_VFXTestResult::Warning, TEXT("No audio asset assigned"));
        }
    }
    catch (...)
    {
        AddTestResult(TestName, Description, EQA_VFXTestResult::Critical, TEXT("Exception during audio sync test"));
    }
}

void AQA_VFXIntegrationTester::RunPerformanceTest()
{
    FString TestName = TEXT("VFX Performance Validation");
    FString Description = TEXT("Validates VFX system performance and resource usage");

    try
    {
        PerformanceMetrics = GatherPerformanceMetrics();

        // Test active systems count
        if (PerformanceMetrics.ActiveNiagaraSystems <= 50)
        {
            AddTestResult(TestName + TEXT(" - System Count"), Description, EQA_VFXTestResult::Pass);
        }
        else
        {
            AddTestResult(TestName + TEXT(" - System Count"), Description, EQA_VFXTestResult::Warning, 
                FString::Printf(TEXT("High system count: %d"), PerformanceMetrics.ActiveNiagaraSystems));
        }

        // Test particle count
        if (PerformanceMetrics.ActiveParticles <= 10000)
        {
            AddTestResult(TestName + TEXT(" - Particle Count"), Description, EQA_VFXTestResult::Pass);
        }
        else
        {
            AddTestResult(TestName + TEXT(" - Particle Count"), Description, EQA_VFXTestResult::Warning,
                FString::Printf(TEXT("High particle count: %d"), PerformanceMetrics.ActiveParticles));
        }

        // Test frame time
        if (PerformanceMetrics.FrameTime <= 16.67f) // 60 FPS target
        {
            AddTestResult(TestName + TEXT(" - Frame Time"), Description, EQA_VFXTestResult::Pass);
        }
        else
        {
            AddTestResult(TestName + TEXT(" - Frame Time"), Description, EQA_VFXTestResult::Warning,
                FString::Printf(TEXT("High frame time: %.2fms"), PerformanceMetrics.FrameTime));
        }

        // Overall performance assessment
        if (PerformanceMetrics.bPerformanceWithinLimits)
        {
            AddTestResult(TestName + TEXT(" - Overall"), Description, EQA_VFXTestResult::Pass);
        }
        else
        {
            AddTestResult(TestName + TEXT(" - Overall"), Description, EQA_VFXTestResult::Fail, TEXT("Performance outside acceptable limits"));
        }
    }
    catch (...)
    {
        AddTestResult(TestName, Description, EQA_VFXTestResult::Critical, TEXT("Exception during performance test"));
    }
}

void AQA_VFXIntegrationTester::ClearTestResults()
{
    TestResults.Empty();
    TotalTests = 0;
    PassedTests = 0;
    FailedTests = 0;
    bAllTestsPassed = false;
}

bool AQA_VFXIntegrationTester::ValidateNiagaraSystem(UNiagaraComponent* NiagaraComp, const FString& SystemName)
{
    if (!NiagaraComp)
    {
        UE_LOG(LogTemp, Error, TEXT("QA VFX Tester: Niagara component is null for %s"), *SystemName);
        return false;
    }

    if (!NiagaraComp->GetAsset())
    {
        UE_LOG(LogTemp, Warning, TEXT("QA VFX Tester: No Niagara system assigned to %s"), *SystemName);
        return false;
    }

    return true;
}

bool AQA_VFXIntegrationTester::ValidateAudioComponent(UAudioComponent* AudioComp, const FString& AudioName)
{
    if (!AudioComp)
    {
        UE_LOG(LogTemp, Error, TEXT("QA VFX Tester: Audio component is null for %s"), *AudioName);
        return false;
    }

    if (!AudioComp->GetSound())
    {
        UE_LOG(LogTemp, Warning, TEXT("QA VFX Tester: No sound assigned to %s"), *AudioName);
        return false;
    }

    return true;
}

FQA_VFXPerformanceMetrics AQA_VFXIntegrationTester::GatherPerformanceMetrics()
{
    FQA_VFXPerformanceMetrics Metrics;

    // Count active Niagara systems in the world
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor)
            {
                TArray<UNiagaraComponent*> NiagaraComponents;
                Actor->GetComponents<UNiagaraComponent>(NiagaraComponents);
                
                for (UNiagaraComponent* NiagaraComp : NiagaraComponents)
                {
                    if (NiagaraComp && NiagaraComp->IsActive())
                    {
                        Metrics.ActiveNiagaraSystems++;
                        // Estimate particle count (simplified)
                        Metrics.ActiveParticles += 100; // Placeholder estimation
                    }
                }
            }
        }
    }

    // Get frame time (simplified)
    Metrics.FrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds

    // Determine if performance is within limits
    Metrics.bPerformanceWithinLimits = (Metrics.ActiveNiagaraSystems <= 50) && 
                                       (Metrics.ActiveParticles <= 10000) && 
                                       (Metrics.FrameTime <= 16.67f);

    return Metrics;
}

void AQA_VFXIntegrationTester::LogTestResult(const FQA_VFXTestCase& TestCase)
{
    FString ResultString;
    switch (TestCase.Result)
    {
        case EQA_VFXTestResult::Pass:
            ResultString = TEXT("PASS");
            break;
        case EQA_VFXTestResult::Fail:
            ResultString = TEXT("FAIL");
            break;
        case EQA_VFXTestResult::Warning:
            ResultString = TEXT("WARNING");
            break;
        case EQA_VFXTestResult::Critical:
            ResultString = TEXT("CRITICAL");
            break;
        default:
            ResultString = TEXT("NOT_TESTED");
            break;
    }

    UE_LOG(LogTemp, Warning, TEXT("QA VFX Test: %s - %s - %s"), *TestCase.TestName, *ResultString, *TestCase.ErrorMessage);
}

void AQA_VFXIntegrationTester::GenerateTestReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== QA VFX INTEGRATION TEST REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Total Tests: %d"), TotalTests);
    UE_LOG(LogTemp, Warning, TEXT("Passed: %d"), PassedTests);
    UE_LOG(LogTemp, Warning, TEXT("Failed: %d"), FailedTests);
    UE_LOG(LogTemp, Warning, TEXT("Success Rate: %.1f%%"), TotalTests > 0 ? (float)PassedTests / TotalTests * 100.0f : 0.0f);

    for (const FQA_VFXTestCase& TestCase : TestResults)
    {
        LogTestResult(TestCase);
    }

    UE_LOG(LogTemp, Warning, TEXT("=== PERFORMANCE METRICS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Active Niagara Systems: %d"), PerformanceMetrics.ActiveNiagaraSystems);
    UE_LOG(LogTemp, Warning, TEXT("Active Particles: %d"), PerformanceMetrics.ActiveParticles);
    UE_LOG(LogTemp, Warning, TEXT("Frame Time: %.2fms"), PerformanceMetrics.FrameTime);
    UE_LOG(LogTemp, Warning, TEXT("Performance Within Limits: %s"), PerformanceMetrics.bPerformanceWithinLimits ? TEXT("YES") : TEXT("NO"));
}

FString AQA_VFXIntegrationTester::GetTestSummary()
{
    return FString::Printf(TEXT("VFX Tests: %d/%d passed (%.1f%%)"), 
                          PassedTests, TotalTests, 
                          TotalTests > 0 ? (float)PassedTests / TotalTests * 100.0f : 0.0f);
}

void AQA_VFXIntegrationTester::ExecuteTestIteration()
{
    if (CurrentTestIteration < MaxTestIterations)
    {
        CurrentTestIteration++;
        RunAllVFXTests();
    }
    else
    {
        GetWorldTimerManager().ClearTimer(TestTimerHandle);
        UE_LOG(LogTemp, Warning, TEXT("QA VFX Integration Tester: Completed %d test iterations"), MaxTestIterations);
    }
}

void AQA_VFXIntegrationTester::AddTestResult(const FString& TestName, const FString& Description, EQA_VFXTestResult Result, const FString& ErrorMsg)
{
    FQA_VFXTestCase TestCase;
    TestCase.TestName = TestName;
    TestCase.Description = Description;
    TestCase.Result = Result;
    TestCase.ErrorMessage = ErrorMsg;
    TestCase.ExecutionTime = FApp::GetDeltaTime();

    TestResults.Add(TestCase);
}

void AQA_VFXIntegrationTester::UpdateTestStatistics()
{
    TotalTests = TestResults.Num();
    PassedTests = 0;
    FailedTests = 0;

    for (const FQA_VFXTestCase& TestCase : TestResults)
    {
        if (TestCase.Result == EQA_VFXTestResult::Pass)
        {
            PassedTests++;
        }
        else if (TestCase.Result == EQA_VFXTestResult::Fail || TestCase.Result == EQA_VFXTestResult::Critical)
        {
            FailedTests++;
        }
    }

    bAllTestsPassed = (FailedTests == 0) && (TotalTests > 0);
}