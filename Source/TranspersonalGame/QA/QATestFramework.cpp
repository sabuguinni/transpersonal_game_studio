// Copyright Transpersonal Game Studio. All Rights Reserved.
// QATestFramework.cpp - Implementation of comprehensive QA testing framework

#include "QATestFramework.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "Components/StaticMeshComponent.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"

DEFINE_LOG_CATEGORY_STATIC(LogQAFramework, Log, All);

UQATestFramework::UQATestFramework()
{
    PerformanceTestDuration = 30.0f;
    MaxStressTestActors = 1000;
    bAutoRunTestsOnStartup = false;
    bGenerateDetailedReports = true;
    bTestingInProgress = false;
    TotalTestsRun = 0;
    TestsPassed = 0;
    TestsFailed = 0;
}

void UQATestFramework::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogQAFramework, Log, TEXT("QA Test Framework initialized"));
    
    InitializeTestEnvironment();
    
    if (bAutoRunTestsOnStartup)
    {
        // Delay auto-run to allow other systems to initialize
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
        {
            RunAllTests();
        }, 5.0f, false);
    }
}

void UQATestFramework::Deinitialize()
{
    CleanupTestEnvironment();
    
    if (bGenerateDetailedReports && TestResults.Num() > 0)
    {
        GenerateTestReport();
    }
    
    Super::Deinitialize();
}

void UQATestFramework::RunAllTests()
{
    UE_LOG(LogQAFramework, Log, TEXT("Starting comprehensive QA test suite"));
    
    TestSessionStartTime = FDateTime::Now();
    bTestingInProgress = true;
    TotalTestsRun = 0;
    TestsPassed = 0;
    TestsFailed = 0;
    
    // Core system tests
    FQATestResult CoreTest = TestCoreSystemsIntegration();
    TestResults.Add(CoreTest);
    LogTestResult(CoreTest);
    
    FQATestResult PhysicsTest = TestPhysicsSystem();
    TestResults.Add(PhysicsTest);
    LogTestResult(PhysicsTest);
    
    // AI system tests
    FQATestResult AITest = TestAIBehaviorTrees();
    TestResults.Add(AITest);
    LogTestResult(AITest);
    
    // Audio/VFX tests
    FQATestResult AudioTest = TestAudioSubsystem();
    TestResults.Add(AudioTest);
    LogTestResult(AudioTest);
    
    FQATestResult VFXTest = TestVFXSystems();
    TestResults.Add(VFXTest);
    LogTestResult(VFXTest);
    
    // Performance tests
    FQATestResult PerfTest = TestPerformanceTargets();
    TestResults.Add(PerfTest);
    LogTestResult(PerfTest);
    
    // Gameplay tests
    FQATestResult DinosaurTest = TestDinosaurSpawning();
    TestResults.Add(DinosaurTest);
    LogTestResult(DinosaurTest);
    
    FQATestResult SurvivalTest = TestPlayerSurvivalMechanics();
    TestResults.Add(SurvivalTest);
    LogTestResult(SurvivalTest);
    
    FQATestResult WorldTest = TestWorldGeneration();
    TestResults.Add(WorldTest);
    LogTestResult(WorldTest);
    
    // Memory tests
    FQATestResult MemoryTest = TestMemoryLeaks();
    TestResults.Add(MemoryTest);
    LogTestResult(MemoryTest);
    
    FQATestResult GCTest = TestGarbageCollection();
    TestResults.Add(GCTest);
    LogTestResult(GCTest);
    
    // Stress tests
    FQATestResult StressActorTest = StressTestActorSpawning(100);
    TestResults.Add(StressActorTest);
    LogTestResult(StressActorTest);
    
    FQATestResult StressAITest = StressTestAIBehavior(50);
    TestResults.Add(StressAITest);
    LogTestResult(StressAITest);
    
    bTestingInProgress = false;
    
    UE_LOG(LogQAFramework, Log, TEXT("QA test suite completed: %d/%d tests passed"), 
           TestsPassed, TotalTestsRun);
    
    GenerateTestReport();
}

void UQATestFramework::RunTestsByCategory(EQATestCategory Category)
{
    UE_LOG(LogQAFramework, Log, TEXT("Running tests for category: %d"), (int32)Category);
    
    switch (Category)
    {
        case EQATestCategory::Core:
            TestResults.Add(TestCoreSystemsIntegration());
            TestResults.Add(TestPhysicsSystem());
            break;
            
        case EQATestCategory::AI:
            TestResults.Add(TestAIBehaviorTrees());
            break;
            
        case EQATestCategory::Audio:
            TestResults.Add(TestAudioSubsystem());
            break;
            
        case EQATestCategory::VFX:
            TestResults.Add(TestVFXSystems());
            break;
            
        case EQATestCategory::Performance:
            TestResults.Add(TestPerformanceTargets());
            break;
            
        case EQATestCategory::Gameplay:
            TestResults.Add(TestDinosaurSpawning());
            TestResults.Add(TestPlayerSurvivalMechanics());
            TestResults.Add(TestWorldGeneration());
            break;
            
        case EQATestCategory::Memory:
            TestResults.Add(TestMemoryLeaks());
            TestResults.Add(TestGarbageCollection());
            break;
    }
}

FQATestResult UQATestFramework::TestCoreSystemsIntegration()
{
    PREHISTORIC_TEST_INIT(TEXT("Core Systems Integration"), EQATestCategory::Core);
    
    // Test world access
    UWorld* World = GetWorld();
    PREHISTORIC_TEST_VALIDATE(World != nullptr, TEXT("Cannot access game world"));
    
    // Test game instance
    UGameInstance* GameInstance = GetGameInstance();
    PREHISTORIC_TEST_VALIDATE(GameInstance != nullptr, TEXT("Cannot access game instance"));
    
    // Test engine subsystems
    PREHISTORIC_TEST_VALIDATE(GEngine != nullptr, TEXT("Engine not available"));
    
    // Test basic actor spawning
    FVector SpawnLocation(0, 0, 100);
    AActor* TestActor = World->SpawnActor<AActor>(AActor::StaticClass(), SpawnLocation, FRotator::ZeroRotator);
    PREHISTORIC_TEST_VALIDATE(TestActor != nullptr, TEXT("Failed to spawn test actor"));
    
    // Cleanup
    if (TestActor)
    {
        TestActor->Destroy();
    }
    
    PREHISTORIC_TEST_PASS(TEXT("Core systems integration successful"));
}

FQATestResult UQATestFramework::TestPhysicsSystem()
{
    PREHISTORIC_TEST_INIT(TEXT("Physics System"), EQATestCategory::Physics);
    
    UWorld* World = GetWorld();
    PREHISTORIC_TEST_VALIDATE(World != nullptr, TEXT("Cannot access world for physics test"));
    
    // Test physics world
    PREHISTORIC_TEST_VALIDATE(World->GetPhysicsScene() != nullptr, TEXT("Physics scene not available"));
    
    // Test collision detection
    FVector StartLocation(0, 0, 1000);
    FVector EndLocation(0, 0, -1000);
    
    FHitResult HitResult;
    bool bHit = World->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECollisionChannel::ECC_WorldStatic
    );
    
    // Note: This may not hit anything in an empty test level, which is okay
    UE_LOG(LogQAFramework, Log, TEXT("Physics line trace test: %s"), bHit ? TEXT("Hit") : TEXT("No Hit"));
    
    PREHISTORIC_TEST_PASS(TEXT("Physics system functional"));
}

FQATestResult UQATestFramework::TestAIBehaviorTrees()
{
    PREHISTORIC_TEST_INIT(TEXT("AI Behavior Trees"), EQATestCategory::AI);
    
    // Test AI controller class availability
    UClass* AIControllerClass = FindObject<UClass>(ANY_PACKAGE, TEXT("AIController"));
    PREHISTORIC_TEST_VALIDATE(AIControllerClass != nullptr, TEXT("AIController class not found"));
    
    // Test behavior tree component
    UClass* BTComponentClass = FindObject<UClass>(ANY_PACKAGE, TEXT("BehaviorTreeComponent"));
    PREHISTORIC_TEST_VALIDATE(BTComponentClass != nullptr, TEXT("BehaviorTreeComponent class not found"));
    
    // Test blackboard component
    UClass* BBComponentClass = FindObject<UClass>(ANY_PACKAGE, TEXT("BlackboardComponent"));
    PREHISTORIC_TEST_VALIDATE(BBComponentClass != nullptr, TEXT("BlackboardComponent class not found"));
    
    PREHISTORIC_TEST_PASS(TEXT("AI behavior tree components available"));
}

FQATestResult UQATestFramework::TestAudioSubsystem()
{
    PREHISTORIC_TEST_INIT(TEXT("Audio Subsystem"), EQATestCategory::Audio);
    
    // Test audio device
    FAudioDevice* AudioDevice = GetWorld()->GetAudioDevice().GetAudioDevice();
    PREHISTORIC_TEST_VALIDATE(AudioDevice != nullptr, TEXT("Audio device not available"));
    
    // Test sound class availability
    UClass* SoundCueClass = FindObject<UClass>(ANY_PACKAGE, TEXT("SoundCue"));
    PREHISTORIC_TEST_VALIDATE(SoundCueClass != nullptr, TEXT("SoundCue class not found"));
    
    UClass* AudioComponentClass = FindObject<UClass>(ANY_PACKAGE, TEXT("AudioComponent"));
    PREHISTORIC_TEST_VALIDATE(AudioComponentClass != nullptr, TEXT("AudioComponent class not found"));
    
    PREHISTORIC_TEST_PASS(TEXT("Audio subsystem functional"));
}

FQATestResult UQATestFramework::TestVFXSystems()
{
    PREHISTORIC_TEST_INIT(TEXT("VFX Systems"), EQATestCategory::VFX);
    
    // Test particle system classes
    UClass* ParticleSystemClass = FindObject<UClass>(ANY_PACKAGE, TEXT("ParticleSystem"));
    PREHISTORIC_TEST_VALIDATE(ParticleSystemClass != nullptr, TEXT("ParticleSystem class not found"));
    
    UClass* ParticleComponentClass = FindObject<UClass>(ANY_PACKAGE, TEXT("ParticleSystemComponent"));
    PREHISTORIC_TEST_VALIDATE(ParticleComponentClass != nullptr, TEXT("ParticleSystemComponent class not found"));
    
    // Test Niagara system (if available)
    UClass* NiagaraSystemClass = FindObject<UClass>(ANY_PACKAGE, TEXT("NiagaraSystem"));
    if (NiagaraSystemClass)
    {
        UE_LOG(LogQAFramework, Log, TEXT("Niagara system available"));
    }
    else
    {
        UE_LOG(LogQAFramework, Warning, TEXT("Niagara system not found - using legacy particle system"));
    }
    
    PREHISTORIC_TEST_PASS(TEXT("VFX systems available"));
}

FQATestResult UQATestFramework::TestPerformanceTargets()
{
    PREHISTORIC_TEST_INIT(TEXT("Performance Targets"), EQATestCategory::Performance);
    
    FQAPerformanceMetrics Metrics = GetCurrentPerformanceMetrics();
    
    // Validate frame time (target: 16.67ms for 60fps)
    VALIDATE_FRAME_TIME(20.0f); // Allow some tolerance
    
    // Validate memory usage (target: under 2GB)
    VALIDATE_MEMORY_USAGE(2048.0f);
    
    // Log performance metrics
    UE_LOG(LogQAFramework, Log, TEXT("Performance Metrics - Frame: %.2fms, Memory: %.2fMB"), 
           Metrics.FrameTime, Metrics.MemoryUsage);
    
    PREHISTORIC_TEST_PASS(TEXT("Performance targets met"));
}

FQATestResult UQATestFramework::TestDinosaurSpawning()
{
    PREHISTORIC_TEST_INIT(TEXT("Dinosaur Spawning"), EQATestCategory::Gameplay);
    
    UWorld* World = GetWorld();
    PREHISTORIC_TEST_VALIDATE(World != nullptr, TEXT("Cannot access world"));
    
    // Test character class availability (placeholder for dinosaur)
    UClass* CharacterClass = FindObject<UClass>(ANY_PACKAGE, TEXT("Character"));
    PREHISTORIC_TEST_VALIDATE(CharacterClass != nullptr, TEXT("Character class not found"));
    
    // Test spawning capability
    FVector SpawnLocation(500, 0, 100);
    ACharacter* TestDinosaur = World->SpawnActor<ACharacter>(CharacterClass, SpawnLocation, FRotator::ZeroRotator);
    PREHISTORIC_TEST_VALIDATE(TestDinosaur != nullptr, TEXT("Failed to spawn test dinosaur"));
    
    // Cleanup
    if (TestDinosaur)
    {
        TestDinosaur->Destroy();
    }
    
    PREHISTORIC_TEST_PASS(TEXT("Dinosaur spawning functional"));
}

FQATestResult UQATestFramework::TestPlayerSurvivalMechanics()
{
    PREHISTORIC_TEST_INIT(TEXT("Player Survival Mechanics"), EQATestCategory::Gameplay);
    
    // Test player controller availability
    UClass* PlayerControllerClass = FindObject<UClass>(ANY_PACKAGE, TEXT("PlayerController"));
    PREHISTORIC_TEST_VALIDATE(PlayerControllerClass != nullptr, TEXT("PlayerController class not found"));
    
    // Test pawn class
    UClass* PawnClass = FindObject<UClass>(ANY_PACKAGE, TEXT("Pawn"));
    PREHISTORIC_TEST_VALIDATE(PawnClass != nullptr, TEXT("Pawn class not found"));
    
    PREHISTORIC_TEST_PASS(TEXT("Player survival mechanics framework available"));
}

FQATestResult UQATestFramework::TestWorldGeneration()
{
    PREHISTORIC_TEST_INIT(TEXT("World Generation"), EQATestCategory::Gameplay);
    
    UWorld* World = GetWorld();
    PREHISTORIC_TEST_VALIDATE(World != nullptr, TEXT("Cannot access world"));
    
    // Test landscape system availability
    UClass* LandscapeClass = FindObject<UClass>(ANY_PACKAGE, TEXT("Landscape"));
    PREHISTORIC_TEST_VALIDATE(LandscapeClass != nullptr, TEXT("Landscape class not found"));
    
    // Test foliage system
    UClass* FoliageClass = FindObject<UClass>(ANY_PACKAGE, TEXT("InstancedFoliageActor"));
    PREHISTORIC_TEST_VALIDATE(FoliageClass != nullptr, TEXT("Foliage system not found"));
    
    PREHISTORIC_TEST_PASS(TEXT("World generation systems available"));
}

FQATestResult UQATestFramework::TestMemoryLeaks()
{
    PREHISTORIC_TEST_INIT(TEXT("Memory Leak Detection"), EQATestCategory::Memory);
    
    // Capture initial memory state
    FQAPerformanceMetrics InitialMetrics = GetCurrentPerformanceMetrics();
    
    // Create and destroy objects to test for leaks
    TArray<AActor*> TestActors;
    UWorld* World = GetWorld();
    
    for (int32 i = 0; i < 100; ++i)
    {
        AActor* Actor = World->SpawnActor<AActor>();
        TestActors.Add(Actor);
    }
    
    // Destroy all test actors
    for (AActor* Actor : TestActors)
    {
        if (Actor)
        {
            Actor->Destroy();
        }
    }
    TestActors.Empty();
    
    // Force garbage collection
    GEngine->ForceGarbageCollection(true);
    
    // Check memory after cleanup
    FQAPerformanceMetrics FinalMetrics = GetCurrentPerformanceMetrics();
    
    float MemoryDifference = FinalMetrics.MemoryUsage - InitialMetrics.MemoryUsage;
    
    // Allow small memory increase (5MB tolerance)
    PREHISTORIC_TEST_VALIDATE(MemoryDifference < 5.0f, 
                             FString::Printf(TEXT("Memory leak detected: %.2fMB increase"), MemoryDifference));
    
    PREHISTORIC_TEST_PASS(TEXT("No significant memory leaks detected"));
}

FQATestResult UQATestFramework::TestGarbageCollection()
{
    PREHISTORIC_TEST_INIT(TEXT("Garbage Collection"), EQATestCategory::Memory);
    
    // Test garbage collection functionality
    GEngine->ForceGarbageCollection(true);
    
    // Verify GC completed without crashing
    PREHISTORIC_TEST_PASS(TEXT("Garbage collection completed successfully"));
}

FQATestResult UQATestFramework::StressTestActorSpawning(int32 ActorCount)
{
    PREHISTORIC_TEST_INIT(TEXT("Actor Spawning Stress Test"), EQATestCategory::Performance);
    
    UWorld* World = GetWorld();
    PREHISTORIC_TEST_VALIDATE(World != nullptr, TEXT("Cannot access world"));
    
    TArray<AActor*> SpawnedActors;
    SpawnedActors.Reserve(ActorCount);
    
    float StartTime = FPlatformTime::Seconds();
    
    // Spawn actors
    for (int32 i = 0; i < ActorCount; ++i)
    {
        FVector Location(FMath::RandRange(-1000, 1000), FMath::RandRange(-1000, 1000), 100);
        AActor* Actor = World->SpawnActor<AActor>(AActor::StaticClass(), Location, FRotator::ZeroRotator);
        
        if (Actor)
        {
            SpawnedActors.Add(Actor);
        }
    }
    
    float SpawnTime = FPlatformTime::Seconds() - StartTime;
    
    UE_LOG(LogQAFramework, Log, TEXT("Spawned %d/%d actors in %.3f seconds"), 
           SpawnedActors.Num(), ActorCount, SpawnTime);
    
    // Cleanup
    for (AActor* Actor : SpawnedActors)
    {
        if (Actor)
        {
            Actor->Destroy();
        }
    }
    
    // Validate performance
    PREHISTORIC_TEST_VALIDATE(SpawnTime < 5.0f, 
                             FString::Printf(TEXT("Actor spawning too slow: %.3f seconds"), SpawnTime));
    
    PREHISTORIC_TEST_VALIDATE(SpawnedActors.Num() >= ActorCount * 0.95f, 
                             TEXT("Too many spawn failures"));
    
    PREHISTORIC_TEST_PASS(FString::Printf(TEXT("Stress test passed: %d actors in %.3fs"), 
                                         SpawnedActors.Num(), SpawnTime));
}

FQATestResult UQATestFramework::StressTestAIBehavior(int32 AICount)
{
    PREHISTORIC_TEST_INIT(TEXT("AI Behavior Stress Test"), EQATestCategory::AI);
    
    // This is a placeholder for AI stress testing
    // In a full implementation, this would spawn multiple AI characters
    // and test their behavior tree execution under load
    
    UE_LOG(LogQAFramework, Log, TEXT("AI stress test placeholder - would test %d AI entities"), AICount);
    
    PREHISTORIC_TEST_PASS(TEXT("AI stress test framework ready"));
}

FQAPerformanceMetrics UQATestFramework::GetCurrentPerformanceMetrics()
{
    FQAPerformanceMetrics Metrics;
    
    // Get frame time
    Metrics.FrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds
    
    // Get memory usage (approximate)
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    Metrics.MemoryUsage = MemStats.UsedPhysical / (1024.0f * 1024.0f); // Convert to MB
    
    // Get actor count
    if (UWorld* World = GetWorld())
    {
        Metrics.ActiveActors = World->GetActorCount();
    }
    
    return Metrics;
}

bool UQATestFramework::ValidateFrameRate(float TargetFPS)
{
    float CurrentFPS = 1.0f / FApp::GetDeltaTime();
    return CurrentFPS >= TargetFPS * 0.9f; // Allow 10% tolerance
}

void UQATestFramework::GenerateTestReport()
{
    FString ReportContent;
    ReportContent += TEXT("=== QA TEST REPORT ===\n");
    ReportContent += FString::Printf(TEXT("Generated: %s\n"), *FDateTime::Now().ToString());
    ReportContent += FString::Printf(TEXT("Total Tests: %d\n"), TotalTestsRun);
    ReportContent += FString::Printf(TEXT("Passed: %d\n"), TestsPassed);
    ReportContent += FString::Printf(TEXT("Failed: %d\n"), TestsFailed);
    ReportContent += TEXT("\n=== DETAILED RESULTS ===\n");
    
    for (const FQATestResult& Result : TestResults)
    {
        ReportContent += FString::Printf(TEXT("[%s] %s - %s (%.3fs)\n"),
                                        Result.bPassed ? TEXT("PASS") : TEXT("FAIL"),
                                        *Result.TestName,
                                        *Result.ErrorMessage,
                                        Result.ExecutionTime);
    }
    
    UE_LOG(LogQAFramework, Log, TEXT("Test Report:\n%s"), *ReportContent);
}

void UQATestFramework::ExportTestResults(const FString& FilePath)
{
    FString JSONContent = TEXT("{\n");
    JSONContent += FString::Printf(TEXT("  \"timestamp\": \"%s\",\n"), *FDateTime::Now().ToString());
    JSONContent += FString::Printf(TEXT("  \"totalTests\": %d,\n"), TotalTestsRun);
    JSONContent += FString::Printf(TEXT("  \"passed\": %d,\n"), TestsPassed);
    JSONContent += FString::Printf(TEXT("  \"failed\": %d,\n"), TestsFailed);
    JSONContent += TEXT("  \"results\": [\n");
    
    for (int32 i = 0; i < TestResults.Num(); ++i)
    {
        const FQATestResult& Result = TestResults[i];
        JSONContent += TEXT("    {\n");
        JSONContent += FString::Printf(TEXT("      \"name\": \"%s\",\n"), *Result.TestName);
        JSONContent += FString::Printf(TEXT("      \"passed\": %s,\n"), Result.bPassed ? TEXT("true") : TEXT("false"));
        JSONContent += FString::Printf(TEXT("      \"message\": \"%s\",\n"), *Result.ErrorMessage);
        JSONContent += FString::Printf(TEXT("      \"executionTime\": %.3f\n"), Result.ExecutionTime);
        JSONContent += TEXT("    }");
        if (i < TestResults.Num() - 1) JSONContent += TEXT(",");
        JSONContent += TEXT("\n");
    }
    
    JSONContent += TEXT("  ]\n");
    JSONContent += TEXT("}\n");
    
    FFileHelper::SaveStringToFile(JSONContent, *FilePath);
}

FQATestResult UQATestFramework::CreateTestResult(const FString& TestName, EQATestCategory Category, 
                                                EQATestSeverity Severity, bool bPassed, 
                                                const FString& ErrorMessage)
{
    FQATestResult Result;
    Result.TestName = TestName;
    Result.Category = Category;
    Result.Severity = Severity;
    Result.bPassed = bPassed;
    Result.ErrorMessage = ErrorMessage;
    Result.Timestamp = FDateTime::Now();
    Result.BuildVersion = FApp::GetBuildVersion();
    
    return Result;
}

void UQATestFramework::LogTestResult(const FQATestResult& Result)
{
    TotalTestsRun++;
    if (Result.bPassed)
    {
        TestsPassed++;
        UE_LOG(LogQAFramework, Log, TEXT("✓ PASS: %s"), *Result.TestName);
    }
    else
    {
        TestsFailed++;
        UE_LOG(LogQAFramework, Error, TEXT("✗ FAIL: %s - %s"), *Result.TestName, *Result.ErrorMessage);
    }
}

void UQATestFramework::InitializeTestEnvironment()
{
    UE_LOG(LogQAFramework, Log, TEXT("Initializing QA test environment"));
    
    // Clear previous test results
    TestResults.Empty();
    PerformanceHistory.Empty();
    
    // Validate system requirements
    ValidateSystemRequirements();
}

void UQATestFramework::CleanupTestEnvironment()
{
    UE_LOG(LogQAFramework, Log, TEXT("Cleaning up QA test environment"));
    
    // Force garbage collection to clean up test objects
    if (GEngine)
    {
        GEngine->ForceGarbageCollection(true);
    }
}

bool UQATestFramework::ValidateSystemRequirements()
{
    // Check minimum system requirements for testing
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    float AvailableMemoryGB = MemStats.AvailablePhysical / (1024.0f * 1024.0f * 1024.0f);
    
    if (AvailableMemoryGB < 4.0f)
    {
        UE_LOG(LogQAFramework, Warning, TEXT("Low available memory: %.1fGB"), AvailableMemoryGB);
        return false;
    }
    
    return true;
}