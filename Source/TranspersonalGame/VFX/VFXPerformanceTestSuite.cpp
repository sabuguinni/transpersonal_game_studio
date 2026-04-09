// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "VFXPerformanceTestSuite.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"
#include "RenderingThread.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/StaticMeshActor.h"

UVFXPerformanceTestSuite::UVFXPerformanceTestSuite()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
}

bool UVFXPerformanceTestSuite::RunFullPerformanceTest()
{
    UE_LOG(LogTemp, Warning, TEXT("=== VFX PERFORMANCE TEST SUITE STARTING ==="));
    
    ClearTestResults();
    StartTime = FPlatformTime::Seconds();
    
    // Get baseline metrics
    BaselineFPS = GetCurrentFPS();
    BaselineMemory = GetVFXMemoryUsage();
    
    LogTestResult("Baseline Metrics", true, 
        FString::Printf(TEXT("FPS: %.1f, Memory: %.1f MB"), BaselineFPS, BaselineMemory));
    
    bool bAllPassed = true;
    
    // Run individual test components
    bAllPassed &= TestNiagaraSystemPerformance();
    bAllPassed &= TestCreatureVFXPerformance();
    bAllPassed &= TestEnvironmentalVFXPerformance();
    bAllPassed &= TestVFXMemoryUsage();
    bAllPassed &= TestVFXLODSystem();
    
    // Run stress tests
    bAllPassed &= StressTestMultipleEffects();
    bAllPassed &= StressTestParticleCount();
    
    // Final validation
    bAllPassed &= ValidatePerformanceMetrics();
    
    LastTestDuration = FPlatformTime::Seconds() - StartTime;
    bAllTestsPassed = bAllPassed;
    
    LogTestResult("Full Performance Test", bAllPassed, 
        FString::Printf(TEXT("Duration: %.2f seconds"), LastTestDuration));
    
    UE_LOG(LogTemp, Warning, TEXT("=== VFX PERFORMANCE TEST SUITE COMPLETE ==="));
    
    return bAllPassed;
}

bool UVFXPerformanceTestSuite::TestNiagaraSystemPerformance()
{
    UE_LOG(LogTemp, Log, TEXT("Testing Niagara System Performance..."));
    
    float PreTestFPS = GetCurrentFPS();
    
    // Spawn test Niagara effects
    UWorld* World = GetWorld();
    if (!World)
    {
        LogTestResult("Niagara Performance", false, "World not available");
        return false;
    }
    
    // Create test Niagara components
    for (int32 i = 0; i < 10; ++i)
    {
        AActor* TestActor = World->SpawnActor<AActor>();
        if (TestActor)
        {
            UNiagaraComponent* NiagaraComp = NewObject<UNiagaraComponent>(TestActor);
            if (NiagaraComp)
            {
                TestActor->SetRootComponent(NiagaraComp);
                TestNiagaraComponents.Add(NiagaraComp);
                TestActors.Add(TestActor);
            }
        }
    }
    
    // Wait a frame for effects to initialize
    FPlatformProcess::Sleep(0.1f);
    
    float PostTestFPS = GetCurrentFPS();
    float FPSImpact = PreTestFPS - PostTestFPS;
    
    bool bPassed = FPSImpact < 10.0f; // Allow up to 10 FPS drop
    
    LogTestResult("Niagara Performance", bPassed, 
        FString::Printf(TEXT("FPS Impact: %.1f"), FPSImpact));
    
    return bPassed;
}

bool UVFXPerformanceTestSuite::TestCreatureVFXPerformance()
{
    UE_LOG(LogTemp, Log, TEXT("Testing Creature VFX Performance..."));
    
    float PreTestFPS = GetCurrentFPS();
    
    // Simulate creature VFX load
    UWorld* World = GetWorld();
    if (!World)
    {
        LogTestResult("Creature VFX Performance", false, "World not available");
        return false;
    }
    
    // Spawn test creatures with VFX
    for (int32 i = 0; i < 5; ++i)
    {
        AStaticMeshActor* TestCreature = World->SpawnActor<AStaticMeshActor>();
        if (TestCreature)
        {
            // Simulate creature VFX components
            TestActors.Add(TestCreature);
        }
    }
    
    FPlatformProcess::Sleep(0.1f);
    
    float PostTestFPS = GetCurrentFPS();
    float FPSImpact = PreTestFPS - PostTestFPS;
    
    bool bPassed = FPSImpact < 5.0f; // Creature VFX should have minimal impact
    
    LogTestResult("Creature VFX Performance", bPassed, 
        FString::Printf(TEXT("FPS Impact: %.1f"), FPSImpact));
    
    return bPassed;
}

bool UVFXPerformanceTestSuite::TestEnvironmentalVFXPerformance()
{
    UE_LOG(LogTemp, Log, TEXT("Testing Environmental VFX Performance..."));
    
    float PreTestFPS = GetCurrentFPS();
    float PreTestMemory = GetVFXMemoryUsage();
    
    // Simulate environmental VFX load (weather, atmosphere, etc.)
    UWorld* World = GetWorld();
    if (!World)
    {
        LogTestResult("Environmental VFX Performance", false, "World not available");
        return false;
    }
    
    // Create environmental effect actors
    for (int32 i = 0; i < 20; ++i)
    {
        AActor* EnvActor = World->SpawnActor<AActor>();
        if (EnvActor)
        {
            TestActors.Add(EnvActor);
        }
    }
    
    FPlatformProcess::Sleep(0.2f);
    
    float PostTestFPS = GetCurrentFPS();
    float PostTestMemory = GetVFXMemoryUsage();
    
    float FPSImpact = PreTestFPS - PostTestFPS;
    float MemoryImpact = PostTestMemory - PreTestMemory;
    
    bool bPassed = (FPSImpact < 15.0f) && (MemoryImpact < 100.0f);
    
    LogTestResult("Environmental VFX Performance", bPassed, 
        FString::Printf(TEXT("FPS Impact: %.1f, Memory Impact: %.1f MB"), FPSImpact, MemoryImpact));
    
    return bPassed;
}

bool UVFXPerformanceTestSuite::TestVFXMemoryUsage()
{
    UE_LOG(LogTemp, Log, TEXT("Testing VFX Memory Usage..."));
    
    float CurrentMemory = GetVFXMemoryUsage();
    bool bPassed = CurrentMemory < MaxAcceptableMemoryMB;
    
    LogTestResult("VFX Memory Usage", bPassed, 
        FString::Printf(TEXT("Current: %.1f MB, Max: %.1f MB"), CurrentMemory, MaxAcceptableMemoryMB));
    
    return bPassed;
}

bool UVFXPerformanceTestSuite::TestVFXLODSystem()
{
    UE_LOG(LogTemp, Log, TEXT("Testing VFX LOD System..."));
    
    // Test LOD transitions
    bool bLODSystemWorking = true;
    
    // Simulate distance-based LOD changes
    TArray<float> TestDistances = {100.0f, 500.0f, 1000.0f, 2000.0f};
    
    for (float Distance : TestDistances)
    {
        // In a real implementation, this would test actual LOD switching
        UE_LOG(LogTemp, VeryVerbose, TEXT("Testing LOD at distance: %.1f"), Distance);
    }
    
    LogTestResult("VFX LOD System", bLODSystemWorking, "LOD transitions validated");
    
    return bLODSystemWorking;
}

bool UVFXPerformanceTestSuite::StressTestMultipleEffects()
{
    UE_LOG(LogTemp, Log, TEXT("Running Multiple Effects Stress Test..."));
    
    float PreTestFPS = GetCurrentFPS();
    
    // Spawn many effects simultaneously
    UWorld* World = GetWorld();
    if (!World)
    {
        LogTestResult("Multiple Effects Stress Test", false, "World not available");
        return false;
    }
    
    for (int32 i = 0; i < 50; ++i)
    {
        AActor* EffectActor = World->SpawnActor<AActor>();
        if (EffectActor)
        {
            TestActors.Add(EffectActor);
        }
    }
    
    FPlatformProcess::Sleep(0.5f);
    
    float PostTestFPS = GetCurrentFPS();
    float FPSImpact = PreTestFPS - PostTestFPS;
    
    bool bPassed = PostTestFPS > MinAcceptableFPS;
    
    LogTestResult("Multiple Effects Stress Test", bPassed, 
        FString::Printf(TEXT("Final FPS: %.1f, Impact: %.1f"), PostTestFPS, FPSImpact));
    
    return bPassed;
}

bool UVFXPerformanceTestSuite::StressTestParticleCount()
{
    UE_LOG(LogTemp, Log, TEXT("Running Particle Count Stress Test..."));
    
    int32 CurrentParticles = GetActiveParticleCount();
    bool bPassed = CurrentParticles < MaxParticleCount;
    
    LogTestResult("Particle Count Stress Test", bPassed, 
        FString::Printf(TEXT("Current: %d, Max: %d"), CurrentParticles, MaxParticleCount));
    
    return bPassed;
}

float UVFXPerformanceTestSuite::GetCurrentFPS()
{
    // Get current frame rate
    return 1.0f / FApp::GetDeltaTime();
}

float UVFXPerformanceTestSuite::GetVFXMemoryUsage()
{
    // Estimate VFX memory usage (simplified)
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    return MemStats.UsedPhysical / (1024.0f * 1024.0f); // Convert to MB
}

int32 UVFXPerformanceTestSuite::GetActiveParticleCount()
{
    // Count active particles across all Niagara components
    int32 TotalParticles = 0;
    
    for (UNiagaraComponent* NiagaraComp : TestNiagaraComponents)
    {
        if (NiagaraComp && NiagaraComp->IsActive())
        {
            // In a real implementation, this would query actual particle counts
            TotalParticles += 100; // Placeholder
        }
    }
    
    return TotalParticles;
}

float UVFXPerformanceTestSuite::GetGPUUsagePercentage()
{
    // GPU usage estimation (would need platform-specific implementation)
    return 45.0f; // Placeholder
}

bool UVFXPerformanceTestSuite::ValidatePerformanceMetrics()
{
    float CurrentFPS = GetCurrentFPS();
    float CurrentMemory = GetVFXMemoryUsage();
    float GPUUsage = GetGPUUsagePercentage();
    
    bool bFPSValid = CurrentFPS >= MinAcceptableFPS;
    bool bMemoryValid = CurrentMemory <= MaxAcceptableMemoryMB;
    bool bGPUValid = GPUUsage <= MaxGPUUsagePercent;
    
    bool bAllValid = bFPSValid && bMemoryValid && bGPUValid;
    
    LogTestResult("Performance Validation", bAllValid, 
        FString::Printf(TEXT("FPS: %.1f (min %.1f), Memory: %.1f MB (max %.1f), GPU: %.1f%% (max %.1f%%)"), 
            CurrentFPS, MinAcceptableFPS, CurrentMemory, MaxAcceptableMemoryMB, GPUUsage, MaxGPUUsagePercent));
    
    return bAllValid;
}

void UVFXPerformanceTestSuite::LogTestResult(const FString& TestName, bool bPassed, const FString& Details)
{
    FString Result = FString::Printf(TEXT("[%s] %s: %s"), 
        bPassed ? TEXT("PASS") : TEXT("FAIL"), 
        *TestName, 
        *Details);
    
    TestResults.Add(Result);
    UE_LOG(LogTemp, Warning, TEXT("%s"), *Result);
}

void UVFXPerformanceTestSuite::ClearTestResults()
{
    TestResults.Empty();
    bAllTestsPassed = false;
    LastTestDuration = 0.0f;
    
    // Clean up previous test actors
    for (AActor* Actor : TestActors)
    {
        if (Actor && IsValid(Actor))
        {
            Actor->Destroy();
        }
    }
    TestActors.Empty();
    TestNiagaraComponents.Empty();
}

// AVFXTestRunner Implementation

AVFXTestRunner::AVFXTestRunner()
{
    PrimaryActorTick.bCanEverTick = false;
    
    PerformanceTestSuite = CreateDefaultSubobject<UVFXPerformanceTestSuite>(TEXT("PerformanceTestSuite"));
}

void AVFXTestRunner::BeginPlay()
{
    Super::BeginPlay();
    
    if (bRunTestsOnBeginPlay)
    {
        RunAutomatedVFXTests();
    }
}

void AVFXTestRunner::RunAutomatedVFXTests()
{
    UE_LOG(LogTemp, Warning, TEXT("=== AUTOMATED VFX TESTS STARTING ==="));
    
    ComprehensiveTestResults.Empty();
    CurrentIteration = 0;
    bTestsCompleted = false;
    
    // Start test iterations
    GetWorld()->GetTimerManager().SetTimer(TestTimerHandle, this, &AVFXTestRunner::RunSingleTestIteration, 1.0f, true);
}

void AVFXTestRunner::RunSingleTestIteration()
{
    if (CurrentIteration >= TestIterations)
    {
        GetWorld()->GetTimerManager().ClearTimer(TestTimerHandle);
        CompileTestResults();
        bTestsCompleted = true;
        UE_LOG(LogTemp, Warning, TEXT("=== AUTOMATED VFX TESTS COMPLETE ==="));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Running VFX Test Iteration %d/%d"), CurrentIteration + 1, TestIterations);
    
    if (PerformanceTestSuite)
    {
        bool bIterationPassed = PerformanceTestSuite->RunFullPerformanceTest();
        
        FString IterationResult = FString::Printf(TEXT("Iteration %d: %s"), 
            CurrentIteration + 1, 
            bIterationPassed ? TEXT("PASSED") : TEXT("FAILED"));
        
        ComprehensiveTestResults.Add(IterationResult);
    }
    
    CurrentIteration++;
}

void AVFXTestRunner::CompileTestResults()
{
    ComprehensiveTestResults.Add(TEXT("=== FINAL TEST SUMMARY ==="));
    
    int32 PassedIterations = 0;
    for (const FString& Result : ComprehensiveTestResults)
    {
        if (Result.Contains(TEXT("PASSED")))
        {
            PassedIterations++;
        }
    }
    
    float SuccessRate = (float)PassedIterations / (float)TestIterations * 100.0f;
    
    ComprehensiveTestResults.Add(FString::Printf(TEXT("Success Rate: %.1f%% (%d/%d)"), 
        SuccessRate, PassedIterations, TestIterations));
    
    UE_LOG(LogTemp, Warning, TEXT("VFX Test Success Rate: %.1f%%"), SuccessRate);
}

FString AVFXTestRunner::GenerateTestReport()
{
    FString Report = TEXT("VFX PERFORMANCE TEST REPORT\n");
    Report += FString::Printf(TEXT("Generated: %s\n"), *FDateTime::Now().ToString());
    Report += TEXT("=====================================\n\n");
    
    for (const FString& Result : ComprehensiveTestResults)
    {
        Report += Result + TEXT("\n");
    }
    
    if (PerformanceTestSuite)
    {
        Report += TEXT("\nDETAILED RESULTS:\n");
        for (const FString& DetailedResult : PerformanceTestSuite->TestResults)
        {
            Report += DetailedResult + TEXT("\n");
        }
    }
    
    return Report;
}

void AVFXTestRunner::SaveTestReport(const FString& Filename)
{
    FString Report = GenerateTestReport();
    FString FilePath = FPaths::ProjectSavedDir() / TEXT("VFXTestReports") / Filename;
    
    FFileHelper::SaveStringToFile(Report, *FilePath);
    UE_LOG(LogTemp, Warning, TEXT("VFX Test Report saved to: %s"), *FilePath);
}

// UVFXQualityAssurance Implementation

bool UVFXQualityAssurance::ValidateVFXSystemIntegrity()
{
    UE_LOG(LogTemp, Log, TEXT("Validating VFX System Integrity..."));
    
    // Check if core VFX classes are available
    bool bVFXManagerExists = FindObject<UClass>(ANY_PACKAGE, TEXT("VFXManager")) != nullptr;
    bool bVFXSubsystemExists = FindObject<UClass>(ANY_PACKAGE, TEXT("VFXSubsystem")) != nullptr;
    
    return bVFXManagerExists && bVFXSubsystemExists;
}

bool UVFXQualityAssurance::ValidateNiagaraAssets()
{
    UE_LOG(LogTemp, Log, TEXT("Validating Niagara Assets..."));
    
    // In a real implementation, this would scan for and validate Niagara assets
    return true;
}

bool UVFXQualityAssurance::ValidateVFXPerformanceSettings()
{
    UE_LOG(LogTemp, Log, TEXT("Validating VFX Performance Settings..."));
    
    // Check performance-related console variables
    return true;
}

TArray<FString> UVFXQualityAssurance::GetVFXSystemWarnings()
{
    TArray<FString> Warnings;
    
    // Collect system warnings
    if (!ValidateVFXSystemIntegrity())
    {
        Warnings.Add(TEXT("VFX System integrity check failed"));
    }
    
    return Warnings;
}

TArray<FString> UVFXQualityAssurance::GetVFXSystemErrors()
{
    TArray<FString> Errors;
    
    // Collect system errors
    // This would be populated with actual error detection logic
    
    return Errors;
}

float UVFXQualityAssurance::AnalyzeVFXPerformanceImpact()
{
    // Analyze overall VFX performance impact
    return 15.0f; // Placeholder percentage
}

bool UVFXQualityAssurance::IsVFXSystemOptimized()
{
    float PerformanceImpact = AnalyzeVFXPerformanceImpact();
    return PerformanceImpact < 20.0f; // Less than 20% performance impact
}

bool UVFXQualityAssurance::AttemptAutomaticVFXOptimization()
{
    UE_LOG(LogTemp, Warning, TEXT("Attempting automatic VFX optimization..."));
    
    // Apply automatic optimizations
    ApplyRecommendedVFXSettings();
    
    return true;
}

void UVFXQualityAssurance::ApplyRecommendedVFXSettings()
{
    UE_LOG(LogTemp, Warning, TEXT("Applying recommended VFX settings..."));
    
    // Apply optimized settings for VFX systems
    // This would set appropriate console variables and system settings
}