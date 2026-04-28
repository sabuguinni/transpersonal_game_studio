// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "PhysicsSystemValidator.h"
#include "EnhancedRagdollSystemV43.h"
#include "PhysicsIntegrationTestSuite.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPhysicsIntegrationTest, Log, All);

// Physics test scenario types
UENUM(BlueprintType)
enum class EPhysicsTestScenario : uint8
{
    BasicRigidBody = 0,        // Simple falling objects
    CollisionResponse = 1,     // Object-to-object collision
    RagdollActivation = 2,     // Character ragdoll physics
    DestructionTest = 3,       // Geometry collection destruction
    ConstraintTest = 4,        // Physics constraints and joints
    PerformanceStress = 5,     // High object count performance
    DinosaurPhysics = 6,       // Large creature physics simulation
    EnvironmentalPhysics = 7,  // Trees, rocks, environmental objects
    FluidInteraction = 8,      // Physics interaction with fluids
    NetworkedPhysics = 9       // Multiplayer physics synchronization
};

// Test result data
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPhysicsTestResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    EPhysicsTestScenario TestScenario;

    UPROPERTY(BlueprintReadOnly)
    bool bTestPassed = false;

    UPROPERTY(BlueprintReadOnly)
    float ExecutionTime = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    int32 ObjectsSimulated = 0;

    UPROPERTY(BlueprintReadOnly)
    float AverageFramerate = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float PhysicsTickTime = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly)
    FDateTime TestTimestamp;

    FPhysicsTestResult()
        : TestScenario(EPhysicsTestScenario::BasicRigidBody)
        , TestTimestamp(FDateTime::Now())
    {}
};

// Delegate for test completion
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPhysicsTestComplete, 
    EPhysicsTestScenario, TestScenario, const FPhysicsTestResult&, Result);

/**
 * Physics Integration Test Suite
 * Comprehensive testing system for all physics scenarios in the dinosaur survival game
 * Tests realistic game scenarios: dinosaur collisions, environmental destruction, character ragdoll
 * Validates performance under stress conditions and ensures 60fps target
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APhysicsIntegrationTestSuite : public AActor
{
    GENERATED_BODY()

public:
    APhysicsIntegrationTestSuite();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime);

    /** Run all physics integration tests */
    UFUNCTION(BlueprintCallable, Category = "Physics Testing")
    void RunAllIntegrationTests();

    /** Run specific test scenario */
    UFUNCTION(BlueprintCallable, Category = "Physics Testing")
    void RunTestScenario(EPhysicsTestScenario TestScenario);

    /** Get test results */
    UFUNCTION(BlueprintPure, Category = "Physics Testing")
    const TArray<FPhysicsTestResult>& GetTestResults() const { return TestResults; }

    /** Clear all test results */
    UFUNCTION(BlueprintCallable, Category = "Physics Testing")
    void ClearTestResults();

    /** Generate test report */
    UFUNCTION(BlueprintCallable, Category = "Physics Testing")
    FString GenerateTestReport() const;

    /** Check if all tests passed */
    UFUNCTION(BlueprintPure, Category = "Physics Testing")
    bool AllTestsPassed() const;

    /** Get test count by status */
    UFUNCTION(BlueprintPure, Category = "Physics Testing")
    int32 GetPassedTestCount() const;

    /** Cleanup all test objects */
    UFUNCTION(BlueprintCallable, Category = "Physics Testing")
    void CleanupTestObjects();

    /** Test completion event */
    UPROPERTY(BlueprintAssignable, Category = "Physics Events")
    FOnPhysicsTestComplete OnTestComplete;

protected:
    /** Physics system validator component */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UPhysicsSystemValidator> PhysicsValidator;

    /** Test results array */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Test Results")
    TArray<FPhysicsTestResult> TestResults;

    /** Auto-run tests on begin play */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Settings")
    bool bAutoRunTests = true;

    /** Test duration for each scenario (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Settings")
    float TestDuration = 10.0f;

    /** Maximum objects for stress testing */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Settings")
    int32 MaxStressTestObjects = 100;

    /** Target framerate for performance validation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFramerate = 60.0f;

    /** Test area bounds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Settings")
    FVector TestAreaSize = FVector(2000, 2000, 1000);

    /** Test area center */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Settings")
    FVector TestAreaCenter = FVector::ZeroVector;

private:
    // Test scenario implementations
    FPhysicsTestResult TestBasicRigidBody();
    FPhysicsTestResult TestCollisionResponse();
    FPhysicsTestResult TestRagdollActivation();
    FPhysicsTestResult TestDestruction();
    FPhysicsTestResult TestConstraints();
    FPhysicsTestResult TestPerformanceStress();
    FPhysicsTestResult TestDinosaurPhysics();
    FPhysicsTestResult TestEnvironmentalPhysics();
    FPhysicsTestResult TestFluidInteraction();
    FPhysicsTestResult TestNetworkedPhysics();

    // Helper methods
    AActor* CreateTestRigidBody(FVector Location, FVector Scale = FVector::OneVector);
    AActor* CreateTestCharacter(FVector Location);
    AActor* CreateTestDestruction(FVector Location);
    AActor* CreateTestConstraint(FVector Location);
    
    void SpawnTestObjects(int32 Count, TArray<AActor*>& OutActors);
    void CleanupTestActors(TArray<AActor*>& Actors);
    
    float MeasureFramerate(float Duration);
    float MeasurePhysicsTickTime();
    bool ValidatePerformance(float Framerate, float PhysicsTime);
    
    FVector GetRandomLocationInTestArea() const;
    FVector GetRandomVelocity() const;
    FRotator GetRandomRotation() const;

    // Test state
    bool bTestsRunning = false;
    float CurrentTestStartTime = 0.0f;
    EPhysicsTestScenario CurrentTestScenario;
    TArray<AActor*> CurrentTestActors;
    
    // Performance monitoring
    TArray<float> FrameTimeHistory;
    TArray<float> PhysicsTimeHistory;
    int32 PerformanceSampleCount = 0;
    
    // Test object templates (set in Blueprint or code)
    UPROPERTY(EditAnywhere, Category = "Test Objects")
    TSubclassOf<AActor> TestCubeClass;
    
    UPROPERTY(EditAnywhere, Category = "Test Objects")
    TSubclassOf<AActor> TestSphereClass;
    
    UPROPERTY(EditAnywhere, Category = "Test Objects")
    TSubclassOf<ACharacter> TestCharacterClass;
    
    UPROPERTY(EditAnywhere, Category = "Test Objects")
    TObjectPtr<UStaticMesh> TestCubeMesh;
    
    UPROPERTY(EditAnywhere, Category = "Test Objects")
    TObjectPtr<UStaticMesh> TestSphereMesh;
};