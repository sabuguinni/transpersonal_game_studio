#include "PhysicsIntegrationTestActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Chaos/ChaosEngineInterface.h"
#include "PhysicsSystemManager.h"
#include "CorePhysicsManager.h"
#include "ChaosPhysicsManager.h"
#include "Core_RagdollComponent.h"
#include "DestructionPhysicsComponent.h"
#include "PhysicsValidationComponent.h"

APhysicsIntegrationTestActor::APhysicsIntegrationTestActor()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
    
    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    
    // Create test mesh component
    TestMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TestMeshComponent"));
    TestMeshComponent->SetupAttachment(RootComponent);
    TestMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    TestMeshComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    TestMeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
    TestMeshComponent->SetSimulatePhysics(true);
    
    // Create collision sphere
    CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
    CollisionSphere->SetupAttachment(TestMeshComponent);
    CollisionSphere->SetSphereRadius(100.0f);
    CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    CollisionSphere->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    CollisionSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
    
    // Create physics validation component
    PhysicsValidationComp = CreateDefaultSubobject<UPhysicsValidationComponent>(TEXT("PhysicsValidationComponent"));
    
    // Create ragdoll component
    RagdollComponent = CreateDefaultSubobject<UCore_RagdollComponent>(TEXT("RagdollComponent"));
    
    // Create destruction component
    DestructionComponent = CreateDefaultSubobject<UDestructionPhysicsComponent>(TEXT("DestructionComponent"));
    
    // Initialize test parameters
    TestDuration = 10.0f;
    TestStartTime = 0.0f;
    bTestRunning = false;
    CurrentTestIndex = 0;
    TestResults.Empty();
    
    // Initialize test configurations
    InitializeTestConfigurations();
}

void APhysicsIntegrationTestActor::BeginPlay()
{
    Super::BeginPlay();
    
    // Bind collision events
    if (CollisionSphere)
    {
        CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &APhysicsIntegrationTestActor::OnOverlapBegin);
        CollisionSphere->OnComponentEndOverlap.AddDynamic(this, &APhysicsIntegrationTestActor::OnOverlapEnd);
    }
    
    // Start automatic testing if enabled
    if (bAutoStartTests)
    {
        StartPhysicsTests();
    }
    
    UE_LOG(LogTemp, Warning, TEXT("PhysicsIntegrationTestActor: BeginPlay completed"));
}

void APhysicsIntegrationTestActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bTestRunning)
    {
        UpdateCurrentTest(DeltaTime);
    }
}

void APhysicsIntegrationTestActor::StartPhysicsTests()
{
    if (bTestRunning)
    {
        UE_LOG(LogTemp, Warning, TEXT("PhysicsIntegrationTestActor: Tests already running"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("PhysicsIntegrationTestActor: Starting physics integration tests"));
    
    bTestRunning = true;
    CurrentTestIndex = 0;
    TestStartTime = GetWorld()->GetTimeSeconds();
    TestResults.Empty();
    
    // Start first test
    StartCurrentTest();
}

void APhysicsIntegrationTestActor::StopPhysicsTests()
{
    if (!bTestRunning)
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("PhysicsIntegrationTestActor: Stopping physics tests"));
    
    bTestRunning = false;
    
    // Generate test report
    GenerateTestReport();
}

void APhysicsIntegrationTestActor::StartCurrentTest()
{
    if (CurrentTestIndex >= TestConfigurations.Num())
    {
        // All tests completed
        StopPhysicsTests();
        return;
    }
    
    const FCore_PhysicsTestConfig& Config = TestConfigurations[CurrentTestIndex];
    UE_LOG(LogTemp, Warning, TEXT("PhysicsIntegrationTestActor: Starting test %d - %s"), 
           CurrentTestIndex, *Config.TestName);
    
    // Reset test state
    TestStartTime = GetWorld()->GetTimeSeconds();
    
    // Configure test based on type
    switch (Config.TestType)
    {
        case ECore_PhysicsTestType::BasicPhysics:
            StartBasicPhysicsTest(Config);
            break;
            
        case ECore_PhysicsTestType::CollisionDetection:
            StartCollisionTest(Config);
            break;
            
        case ECore_PhysicsTestType::RagdollPhysics:
            StartRagdollTest(Config);
            break;
            
        case ECore_PhysicsTestType::DestructionSystem:
            StartDestructionTest(Config);
            break;
            
        case ECore_PhysicsTestType::PerformanceStress:
            StartPerformanceTest(Config);
            break;
            
        default:
            UE_LOG(LogTemp, Error, TEXT("PhysicsIntegrationTestActor: Unknown test type"));
            NextTest();
            break;
    }
}

void APhysicsIntegrationTestActor::UpdateCurrentTest(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float ElapsedTime = CurrentTime - TestStartTime;
    
    if (ElapsedTime >= TestDuration)
    {
        // Test timeout - move to next test
        CompleteCurrentTest(false, TEXT("Test timeout"));
        return;
    }
    
    // Update test-specific logic
    if (CurrentTestIndex < TestConfigurations.Num())
    {
        const FCore_PhysicsTestConfig& Config = TestConfigurations[CurrentTestIndex];
        
        switch (Config.TestType)
        {
            case ECore_PhysicsTestType::BasicPhysics:
                UpdateBasicPhysicsTest(DeltaTime);
                break;
                
            case ECore_PhysicsTestType::CollisionDetection:
                UpdateCollisionTest(DeltaTime);
                break;
                
            case ECore_PhysicsTestType::RagdollPhysics:
                UpdateRagdollTest(DeltaTime);
                break;
                
            case ECore_PhysicsTestType::DestructionSystem:
                UpdateDestructionTest(DeltaTime);
                break;
                
            case ECore_PhysicsTestType::PerformanceStress:
                UpdatePerformanceTest(DeltaTime);
                break;
        }
    }
}

void APhysicsIntegrationTestActor::CompleteCurrentTest(bool bSuccess, const FString& ResultMessage)
{
    if (CurrentTestIndex >= TestConfigurations.Num())
    {
        return;
    }
    
    const FCore_PhysicsTestConfig& Config = TestConfigurations[CurrentTestIndex];
    
    // Record test result
    FCore_PhysicsTestResult Result;
    Result.TestName = Config.TestName;
    Result.TestType = Config.TestType;
    Result.bSuccess = bSuccess;
    Result.ResultMessage = ResultMessage;
    Result.ExecutionTime = GetWorld()->GetTimeSeconds() - TestStartTime;
    
    TestResults.Add(Result);
    
    UE_LOG(LogTemp, Warning, TEXT("PhysicsIntegrationTestActor: Test %d completed - %s: %s"), 
           CurrentTestIndex, *Config.TestName, bSuccess ? TEXT("PASS") : TEXT("FAIL"));
    
    if (!bSuccess)
    {
        UE_LOG(LogTemp, Error, TEXT("PhysicsIntegrationTestActor: Test failure reason: %s"), *ResultMessage);
    }
    
    // Move to next test
    NextTest();
}

void APhysicsIntegrationTestActor::NextTest()
{
    CurrentTestIndex++;
    
    if (CurrentTestIndex >= TestConfigurations.Num())
    {
        // All tests completed
        StopPhysicsTests();
    }
    else
    {
        // Start next test after a brief delay
        GetWorld()->GetTimerManager().SetTimer(
            TestDelayTimer,
            this,
            &APhysicsIntegrationTestActor::StartCurrentTest,
            1.0f,
            false
        );
    }
}

void APhysicsIntegrationTestActor::InitializeTestConfigurations()
{
    TestConfigurations.Empty();
    
    // Basic Physics Test
    FCore_PhysicsTestConfig BasicTest;
    BasicTest.TestName = TEXT("Basic Physics Simulation");
    BasicTest.TestType = ECore_PhysicsTestType::BasicPhysics;
    BasicTest.ExpectedDuration = 5.0f;
    BasicTest.SuccessCriteria = TEXT("Object falls under gravity and settles");
    TestConfigurations.Add(BasicTest);
    
    // Collision Detection Test
    FCore_PhysicsTestConfig CollisionTest;
    CollisionTest.TestName = TEXT("Collision Detection");
    CollisionTest.TestType = ECore_PhysicsTestType::CollisionDetection;
    CollisionTest.ExpectedDuration = 3.0f;
    CollisionTest.SuccessCriteria = TEXT("Collision events triggered correctly");
    TestConfigurations.Add(CollisionTest);
    
    // Ragdoll Physics Test
    FCore_PhysicsTestConfig RagdollTest;
    RagdollTest.TestName = TEXT("Ragdoll Physics");
    RagdollTest.TestType = ECore_PhysicsTestType::RagdollPhysics;
    RagdollTest.ExpectedDuration = 7.0f;
    RagdollTest.SuccessCriteria = TEXT("Ragdoll activates and simulates correctly");
    TestConfigurations.Add(RagdollTest);
    
    // Destruction System Test
    FCore_PhysicsTestConfig DestructionTest;
    DestructionTest.TestName = TEXT("Destruction System");
    DestructionTest.TestType = ECore_PhysicsTestType::DestructionSystem;
    DestructionTest.ExpectedDuration = 8.0f;
    DestructionTest.SuccessCriteria = TEXT("Destruction triggers and fragments spawn");
    TestConfigurations.Add(DestructionTest);
    
    // Performance Stress Test
    FCore_PhysicsTestConfig PerformanceTest;
    PerformanceTest.TestName = TEXT("Performance Stress Test");
    PerformanceTest.TestType = ECore_PhysicsTestType::PerformanceStress;
    PerformanceTest.ExpectedDuration = 10.0f;
    PerformanceTest.SuccessCriteria = TEXT("Maintains acceptable framerate under load");
    TestConfigurations.Add(PerformanceTest);
}

void APhysicsIntegrationTestActor::StartBasicPhysicsTest(const FCore_PhysicsTestConfig& Config)
{
    if (TestMeshComponent)
    {
        // Reset position and enable physics
        SetActorLocation(FVector(0, 0, 500));
        TestMeshComponent->SetSimulatePhysics(true);
        TestMeshComponent->SetEnableGravity(true);
        
        UE_LOG(LogTemp, Warning, TEXT("PhysicsIntegrationTestActor: Basic physics test started"));
    }
}

void APhysicsIntegrationTestActor::UpdateBasicPhysicsTest(float DeltaTime)
{
    if (TestMeshComponent)
    {
        FVector CurrentLocation = GetActorLocation();
        FVector Velocity = TestMeshComponent->GetComponentVelocity();
        
        // Check if object has settled (low velocity and reasonable height)
        if (Velocity.Size() < 10.0f && CurrentLocation.Z < 200.0f)
        {
            CompleteCurrentTest(true, TEXT("Object settled under gravity"));
        }
    }
}

void APhysicsIntegrationTestActor::StartCollisionTest(const FCore_PhysicsTestConfig& Config)
{
    // Reset collision counters
    CollisionCount = 0;
    OverlapCount = 0;
    
    // Enable collision detection
    if (CollisionSphere)
    {
        CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("PhysicsIntegrationTestActor: Collision test started"));
}

void APhysicsIntegrationTestActor::UpdateCollisionTest(float DeltaTime)
{
    // Test passes if we detect any collisions or overlaps
    if (CollisionCount > 0 || OverlapCount > 0)
    {
        CompleteCurrentTest(true, FString::Printf(TEXT("Collisions: %d, Overlaps: %d"), CollisionCount, OverlapCount));
    }
}

void APhysicsIntegrationTestActor::StartRagdollTest(const FCore_PhysicsTestConfig& Config)
{
    if (RagdollComponent)
    {
        // Activate ragdoll physics
        RagdollComponent->ActivateRagdoll();
        UE_LOG(LogTemp, Warning, TEXT("PhysicsIntegrationTestActor: Ragdoll test started"));
    }
    else
    {
        CompleteCurrentTest(false, TEXT("Ragdoll component not available"));
    }
}

void APhysicsIntegrationTestActor::UpdateRagdollTest(float DeltaTime)
{
    if (RagdollComponent && RagdollComponent->IsRagdollActive())
    {
        CompleteCurrentTest(true, TEXT("Ragdoll activated successfully"));
    }
}

void APhysicsIntegrationTestActor::StartDestructionTest(const FCore_PhysicsTestConfig& Config)
{
    if (DestructionComponent)
    {
        // Trigger destruction
        DestructionComponent->TriggerDestruction(1000.0f, GetActorLocation(), FVector::UpVector);
        UE_LOG(LogTemp, Warning, TEXT("PhysicsIntegrationTestActor: Destruction test started"));
    }
    else
    {
        CompleteCurrentTest(false, TEXT("Destruction component not available"));
    }
}

void APhysicsIntegrationTestActor::UpdateDestructionTest(float DeltaTime)
{
    if (DestructionComponent && DestructionComponent->IsDestroyed())
    {
        CompleteCurrentTest(true, TEXT("Destruction triggered successfully"));
    }
}

void APhysicsIntegrationTestActor::StartPerformanceTest(const FCore_PhysicsTestConfig& Config)
{
    // Record initial framerate
    InitialFrameRate = 1.0f / DeltaTime;
    PerformanceFrameCount = 0;
    PerformanceFrameSum = 0.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("PhysicsIntegrationTestActor: Performance test started"));
}

void APhysicsIntegrationTestActor::UpdatePerformanceTest(float DeltaTime)
{
    // Track framerate
    PerformanceFrameCount++;
    PerformanceFrameSum += 1.0f / DeltaTime;
    
    if (PerformanceFrameCount >= 60) // Test over 60 frames
    {
        float AverageFrameRate = PerformanceFrameSum / PerformanceFrameCount;
        bool bPerformanceAcceptable = AverageFrameRate > 30.0f; // Minimum 30 FPS
        
        FString ResultMsg = FString::Printf(TEXT("Average FPS: %.1f"), AverageFrameRate);
        CompleteCurrentTest(bPerformanceAcceptable, ResultMsg);
    }
}

void APhysicsIntegrationTestActor::GenerateTestReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PHYSICS INTEGRATION TEST REPORT ==="));
    
    int32 PassedTests = 0;
    int32 TotalTests = TestResults.Num();
    
    for (const FCore_PhysicsTestResult& Result : TestResults)
    {
        if (Result.bSuccess)
        {
            PassedTests++;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Test: %s - %s (%.2fs) - %s"), 
               *Result.TestName,
               Result.bSuccess ? TEXT("PASS") : TEXT("FAIL"),
               Result.ExecutionTime,
               *Result.ResultMessage);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== TEST SUMMARY: %d/%d PASSED ==="), PassedTests, TotalTests);
    
    // Broadcast test completion
    if (OnTestsCompleted.IsBound())
    {
        OnTestsCompleted.Broadcast(PassedTests, TotalTests, TestResults);
    }
}

void APhysicsIntegrationTestActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    OverlapCount++;
    UE_LOG(LogTemp, Log, TEXT("PhysicsIntegrationTestActor: Overlap begin with %s"), OtherActor ? *OtherActor->GetName() : TEXT("Unknown"));
}

void APhysicsIntegrationTestActor::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
    UE_LOG(LogTemp, Log, TEXT("PhysicsIntegrationTestActor: Overlap end with %s"), OtherActor ? *OtherActor->GetName() : TEXT("Unknown"));
}

void APhysicsIntegrationTestActor::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
    CollisionCount++;
    UE_LOG(LogTemp, Log, TEXT("PhysicsIntegrationTestActor: Hit with %s"), OtherActor ? *OtherActor->GetName() : TEXT("Unknown"));
}