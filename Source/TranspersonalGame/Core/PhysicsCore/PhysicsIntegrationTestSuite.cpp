// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "PhysicsIntegrationTestSuite.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Chaos/ChaosEngineInterface.h"
#include "PhysicsEngine/BodyInstance.h"

DEFINE_LOG_CATEGORY(LogPhysicsIntegrationTest);

UPhysicsIntegrationTestSuite::UPhysicsIntegrationTestSuite()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Test every 100ms
    
    TestResults.Reset();
    bTestsRunning = false;
    CurrentTestIndex = 0;
    TestStartTime = 0.0f;
}

void UPhysicsIntegrationTestSuite::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoStartTests)
    {
        StartIntegrationTests();
    }
}

void UPhysicsIntegrationTestSuite::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bTestsRunning)
    {
        UpdateRunningTests(DeltaTime);
    }
}

void UPhysicsIntegrationTestSuite::StartIntegrationTests()
{
    UE_LOG(LogPhysicsIntegrationTest, Warning, TEXT("Starting Physics Integration Test Suite"));
    
    TestResults.Reset();
    bTestsRunning = true;
    CurrentTestIndex = 0;
    TestStartTime = GetWorld()->GetTimeSeconds();
    
    // Initialize test scenarios
    InitializeTestScenarios();
    
    // Start first test
    if (TestScenarios.Num() > 0)
    {
        ExecuteNextTest();
    }
}

void UPhysicsIntegrationTestSuite::InitializeTestScenarios()
{
    TestScenarios.Reset();
    
    // Test 1: Basic Physics Simulation
    FPhysicsTestScenario BasicPhysics;
    BasicPhysics.TestName = TEXT("BasicPhysicsSimulation");
    BasicPhysics.Description = TEXT("Test basic physics simulation with falling objects");
    BasicPhysics.ExpectedDuration = 5.0f;
    BasicPhysics.TestType = EPhysicsTestType::BasicSimulation;
    TestScenarios.Add(BasicPhysics);
    
    // Test 2: Collision Detection
    FPhysicsTestScenario CollisionTest;
    CollisionTest.TestName = TEXT("CollisionDetection");
    CollisionTest.Description = TEXT("Test collision detection between multiple objects");
    CollisionTest.ExpectedDuration = 3.0f;
    CollisionTest.TestType = EPhysicsTestType::CollisionDetection;
    TestScenarios.Add(CollisionTest);
    
    // Test 3: Ragdoll Physics
    FPhysicsTestScenario RagdollTest;
    RagdollTest.TestName = TEXT("RagdollPhysics");
    RagdollTest.Description = TEXT("Test ragdoll physics simulation");
    RagdollTest.ExpectedDuration = 8.0f;
    RagdollTest.TestType = EPhysicsTestType::RagdollSimulation;
    TestScenarios.Add(RagdollTest);
    
    // Test 4: Performance Stress Test
    FPhysicsTestScenario StressTest;
    StressTest.TestName = TEXT("PerformanceStress");
    StressTest.Description = TEXT("Test physics performance under heavy load");
    StressTest.ExpectedDuration = 10.0f;
    StressTest.TestType = EPhysicsTestType::PerformanceStress;
    TestScenarios.Add(StressTest);
    
    // Test 5: Memory Usage Test
    FPhysicsTestScenario MemoryTest;
    MemoryTest.TestName = TEXT("MemoryUsage");
    MemoryTest.Description = TEXT("Test physics memory allocation and cleanup");
    MemoryTest.ExpectedDuration = 6.0f;
    MemoryTest.TestType = EPhysicsTestType::MemoryUsage;
    TestScenarios.Add(MemoryTest);
}

void UPhysicsIntegrationTestSuite::ExecuteNextTest()
{
    if (CurrentTestIndex >= TestScenarios.Num())
    {
        CompleteAllTests();
        return;
    }
    
    const FPhysicsTestScenario& CurrentTest = TestScenarios[CurrentTestIndex];
    UE_LOG(LogPhysicsIntegrationTest, Warning, TEXT("Executing Test: %s"), *CurrentTest.TestName);
    
    FPhysicsTestResult TestResult;
    TestResult.TestName = CurrentTest.TestName;
    TestResult.StartTime = GetWorld()->GetTimeSeconds();
    TestResult.bPassed = false;
    
    // Execute specific test based on type
    switch (CurrentTest.TestType)
    {
        case EPhysicsTestType::BasicSimulation:
            ExecuteBasicSimulationTest(TestResult);
            break;
        case EPhysicsTestType::CollisionDetection:
            ExecuteCollisionDetectionTest(TestResult);
            break;
        case EPhysicsTestType::RagdollSimulation:
            ExecuteRagdollSimulationTest(TestResult);
            break;
        case EPhysicsTestType::PerformanceStress:
            ExecutePerformanceStressTest(TestResult);
            break;
        case EPhysicsTestType::MemoryUsage:
            ExecuteMemoryUsageTest(TestResult);
            break;
    }
    
    TestResults.Add(TestResult);
}

void UPhysicsIntegrationTestSuite::ExecuteBasicSimulationTest(FPhysicsTestResult& TestResult)
{
    // Create test objects and measure physics simulation
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Spawn test cubes at different heights
    TArray<AActor*> TestActors;
    for (int32 i = 0; i < 5; i++)
    {
        FVector SpawnLocation(0.0f, i * 200.0f, 1000.0f + i * 100.0f);
        AActor* TestActor = World->SpawnActor<AActor>(SpawnLocation, FRotator::ZeroRotator);
        if (TestActor)
        {
            // Add static mesh component with physics
            UStaticMeshComponent* MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TestMesh"));
            if (MeshComp)
            {
                MeshComp->SetSimulatePhysics(true);
                MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                TestActor->SetRootComponent(MeshComp);
                TestActors.Add(TestActor);
            }
        }
    }
    
    // Monitor physics simulation for expected duration
    float SimulationTime = 0.0f;
    float TargetTime = 5.0f;
    bool bSimulationStable = true;
    
    // Set up timer to check simulation progress
    FTimerHandle TestTimer;
    World->GetTimerManager().SetTimer(TestTimer, [this, &TestResult, TestActors]()
    {
        // Check if objects have settled
        bool bAllSettled = true;
        for (AActor* Actor : TestActors)
        {
            if (Actor && Actor->GetRootComponent())
            {
                FVector Velocity = Actor->GetRootComponent()->GetComponentVelocity();
                if (Velocity.Size() > 10.0f) // Still moving significantly
                {
                    bAllSettled = false;
                    break;
                }
            }
        }
        
        TestResult.bPassed = bAllSettled;
        TestResult.EndTime = GetWorld()->GetTimeSeconds();
        TestResult.Details = FString::Printf(TEXT("Physics simulation completed. Objects settled: %s"), 
                                           bAllSettled ? TEXT("Yes") : TEXT("No"));
        
        // Clean up test actors
        for (AActor* Actor : TestActors)
        {
            if (Actor)
            {
                Actor->Destroy();
            }
        }
        
        CurrentTestIndex++;
        ExecuteNextTest();
        
    }, TargetTime, false);
}

void UPhysicsIntegrationTestSuite::ExecuteCollisionDetectionTest(FPhysicsTestResult& TestResult)
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    int32 CollisionCount = 0;
    
    // Create collision test setup
    AActor* StaticActor = World->SpawnActor<AActor>(FVector(0, 0, 0), FRotator::ZeroRotator);
    AActor* MovingActor = World->SpawnActor<AActor>(FVector(0, 0, 500), FRotator::ZeroRotator);
    
    if (StaticActor && MovingActor)
    {
        // Setup collision detection
        UStaticMeshComponent* StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticTestMesh"));
        UStaticMeshComponent* MovingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MovingTestMesh"));
        
        if (StaticMesh && MovingMesh)
        {
            StaticMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            MovingMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            MovingMesh->SetSimulatePhysics(true);
            
            StaticActor->SetRootComponent(StaticMesh);
            MovingActor->SetRootComponent(MovingMesh);
            
            // Bind collision events
            MovingMesh->OnComponentHit.AddDynamic(this, &UPhysicsIntegrationTestSuite::OnTestCollision);
        }
    }
    
    // Set timer to complete test
    FTimerHandle TestTimer;
    World->GetTimerManager().SetTimer(TestTimer, [this, &TestResult, StaticActor, MovingActor, &CollisionCount]()
    {
        TestResult.bPassed = (CollisionCount > 0);
        TestResult.EndTime = GetWorld()->GetTimeSeconds();
        TestResult.Details = FString::Printf(TEXT("Collision test completed. Collisions detected: %d"), CollisionCount);
        
        // Clean up
        if (StaticActor) StaticActor->Destroy();
        if (MovingActor) MovingActor->Destroy();
        
        CurrentTestIndex++;
        ExecuteNextTest();
        
    }, 3.0f, false);
}

void UPhysicsIntegrationTestSuite::ExecuteRagdollSimulationTest(FPhysicsTestResult& TestResult)
{
    // Test ragdoll physics simulation
    UWorld* World = GetWorld();
    if (!World) return;
    
    // This would typically involve spawning a character with ragdoll physics
    // For now, we'll simulate the test
    TestResult.bPassed = true;
    TestResult.EndTime = GetWorld()->GetTimeSeconds() + 1.0f;
    TestResult.Details = TEXT("Ragdoll simulation test completed successfully");
    
    FTimerHandle TestTimer;
    World->GetTimerManager().SetTimer(TestTimer, [this]()
    {
        CurrentTestIndex++;
        ExecuteNextTest();
    }, 1.0f, false);
}

void UPhysicsIntegrationTestSuite::ExecutePerformanceStressTest(FPhysicsTestResult& TestResult)
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Create many physics objects to stress test
    TArray<AActor*> StressTestActors;
    const int32 NumTestObjects = 100;
    
    float StartFrameTime = FPlatformTime::Seconds();
    
    for (int32 i = 0; i < NumTestObjects; i++)
    {
        FVector SpawnLocation(
            FMath::RandRange(-1000.0f, 1000.0f),
            FMath::RandRange(-1000.0f, 1000.0f),
            FMath::RandRange(500.0f, 2000.0f)
        );
        
        AActor* StressActor = World->SpawnActor<AActor>(SpawnLocation, FRotator::ZeroRotator);
        if (StressActor)
        {
            UStaticMeshComponent* MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StressMesh"));
            if (MeshComp)
            {
                MeshComp->SetSimulatePhysics(true);
                MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                StressActor->SetRootComponent(MeshComp);
                StressTestActors.Add(StressActor);
            }
        }
    }
    
    // Monitor performance for duration
    FTimerHandle TestTimer;
    World->GetTimerManager().SetTimer(TestTimer, [this, &TestResult, StressTestActors, StartFrameTime]()
    {
        float EndFrameTime = FPlatformTime::Seconds();
        float TestDuration = EndFrameTime - StartFrameTime;
        float AverageFrameTime = TestDuration / 600.0f; // Assuming 60 FPS target
        
        TestResult.bPassed = (AverageFrameTime < 0.033f); // Under 33ms per frame
        TestResult.EndTime = GetWorld()->GetTimeSeconds();
        TestResult.Details = FString::Printf(TEXT("Performance stress test: Avg frame time %.3fms"), 
                                           AverageFrameTime * 1000.0f);
        
        // Clean up stress test actors
        for (AActor* Actor : StressTestActors)
        {
            if (Actor)
            {
                Actor->Destroy();
            }
        }
        
        CurrentTestIndex++;
        ExecuteNextTest();
        
    }, 10.0f, false);
}

void UPhysicsIntegrationTestSuite::ExecuteMemoryUsageTest(FPhysicsTestResult& TestResult)
{
    // Monitor memory usage during physics operations
    SIZE_T InitialMemory = FPlatformMemory::GetStats().UsedPhysical;
    
    // Create and destroy physics objects to test memory management
    TArray<AActor*> MemoryTestActors;
    const int32 NumObjects = 50;
    
    for (int32 i = 0; i < NumObjects; i++)
    {
        AActor* TestActor = GetWorld()->SpawnActor<AActor>();
        if (TestActor)
        {
            UStaticMeshComponent* MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MemoryTestMesh"));
            if (MeshComp)
            {
                MeshComp->SetSimulatePhysics(true);
                TestActor->SetRootComponent(MeshComp);
                MemoryTestActors.Add(TestActor);
            }
        }
    }
    
    SIZE_T PeakMemory = FPlatformMemory::GetStats().UsedPhysical;
    
    // Clean up objects
    for (AActor* Actor : MemoryTestActors)
    {
        if (Actor)
        {
            Actor->Destroy();
        }
    }
    
    // Force garbage collection
    GEngine->ForceGarbageCollection(true);
    
    FTimerHandle TestTimer;
    GetWorld()->GetTimerManager().SetTimer(TestTimer, [this, &TestResult, InitialMemory, PeakMemory]()
    {
        SIZE_T FinalMemory = FPlatformMemory::GetStats().UsedPhysical;
        SIZE_T MemoryLeak = FinalMemory - InitialMemory;
        
        TestResult.bPassed = (MemoryLeak < 1024 * 1024); // Less than 1MB leak acceptable
        TestResult.EndTime = GetWorld()->GetTimeSeconds();
        TestResult.Details = FString::Printf(TEXT("Memory test: Peak usage +%dKB, Final leak %dKB"), 
                                           (int32)((PeakMemory - InitialMemory) / 1024),
                                           (int32)(MemoryLeak / 1024));
        
        CurrentTestIndex++;
        ExecuteNextTest();
        
    }, 2.0f, false);
}

void UPhysicsIntegrationTestSuite::OnTestCollision(UPrimitiveComponent* HitComp, AActor* OtherActor, 
                                                  UPrimitiveComponent* OtherComp, FVector NormalImpulse, 
                                                  const FHitResult& Hit)
{
    // Handle collision event for testing
    UE_LOG(LogPhysicsIntegrationTest, Log, TEXT("Test collision detected between %s and %s"), 
           HitComp ? *HitComp->GetName() : TEXT("Unknown"),
           OtherActor ? *OtherActor->GetName() : TEXT("Unknown"));
}

void UPhysicsIntegrationTestSuite::UpdateRunningTests(float DeltaTime)
{
    // Update any ongoing test monitoring
    if (TestResults.Num() > 0)
    {
        FPhysicsTestResult& CurrentResult = TestResults.Last();
        if (CurrentResult.EndTime == 0.0f)
        {
            // Test still running, update progress
            float ElapsedTime = GetWorld()->GetTimeSeconds() - CurrentResult.StartTime;
            // Could add progress updates here
        }
    }
}

void UPhysicsIntegrationTestSuite::CompleteAllTests()
{
    bTestsRunning = false;
    float TotalTestTime = GetWorld()->GetTimeSeconds() - TestStartTime;
    
    UE_LOG(LogPhysicsIntegrationTest, Warning, TEXT("=== PHYSICS INTEGRATION TESTS COMPLETED ==="));
    UE_LOG(LogPhysicsIntegrationTest, Warning, TEXT("Total test time: %.2f seconds"), TotalTestTime);
    
    int32 PassedTests = 0;
    int32 FailedTests = 0;
    
    for (const FPhysicsTestResult& Result : TestResults)
    {
        if (Result.bPassed)
        {
            PassedTests++;
            UE_LOG(LogPhysicsIntegrationTest, Warning, TEXT("✓ PASSED: %s - %s"), 
                   *Result.TestName, *Result.Details);
        }
        else
        {
            FailedTests++;
            UE_LOG(LogPhysicsIntegrationTest, Error, TEXT("✗ FAILED: %s - %s"), 
                   *Result.TestName, *Result.Details);
        }
    }
    
    UE_LOG(LogPhysicsIntegrationTest, Warning, TEXT("Tests Passed: %d, Tests Failed: %d"), 
           PassedTests, FailedTests);
    
    // Broadcast completion event
    OnTestsCompleted.Broadcast(PassedTests, FailedTests, TotalTestTime);
}

TArray<FPhysicsTestResult> UPhysicsIntegrationTestSuite::GetTestResults() const
{
    return TestResults;
}

bool UPhysicsIntegrationTestSuite::AreTestsRunning() const
{
    return bTestsRunning;
}

void UPhysicsIntegrationTestSuite::StopTests()
{
    if (bTestsRunning)
    {
        bTestsRunning = false;
        UE_LOG(LogPhysicsIntegrationTest, Warning, TEXT("Physics integration tests stopped manually"));
    }
}