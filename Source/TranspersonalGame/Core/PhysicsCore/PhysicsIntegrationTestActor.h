#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "PhysicsIntegrationTestActor.generated.h"

// Forward declarations
class UPhysicsSystemManager;
class UCorePhysicsManager;
class UDestructionPhysicsComponent;
class UCore_RagdollComponent;
class UVehiclePhysicsSystemV2;
class UPhysicsValidationComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogPhysicsIntegrationTest, Log, All);

/**
 * Integration test states for physics systems
 */
UENUM(BlueprintType)
enum class ECore_PhysicsTestState : uint8
{
    NotStarted      UMETA(DisplayName = "Not Started"),
    Initializing    UMETA(DisplayName = "Initializing"),
    Testing         UMETA(DisplayName = "Testing"),
    Validating      UMETA(DisplayName = "Validating"),
    Completed       UMETA(DisplayName = "Completed"),
    Failed          UMETA(DisplayName = "Failed")
};

/**
 * Physics integration test results
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsTestResult
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Test Result")
    FString TestName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Test Result")
    bool bPassed;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Test Result")
    float ExecutionTime;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Test Result")
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Test Result")
    int32 TestPriority;

    FCore_PhysicsTestResult()
    {
        TestName = TEXT("Unknown Test");
        bPassed = false;
        ExecutionTime = 0.0f;
        ErrorMessage = TEXT("");
        TestPriority = 0;
    }
};

/**
 * Comprehensive physics integration test actor that validates all physics systems work together
 * Tests collision, ragdoll, destruction, vehicle physics, and system integration
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APhysicsIntegrationTestActor : public AActor
{
    GENERATED_BODY()

public:
    APhysicsIntegrationTestActor();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

    // Test execution functions
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Physics Integration Test")
    void StartIntegrationTests();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Physics Integration Test")
    void StopIntegrationTests();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Physics Integration Test")
    void ResetTests();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Physics Integration Test")
    ECore_PhysicsTestState GetCurrentTestState() const { return CurrentTestState; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Physics Integration Test")
    TArray<FCore_PhysicsTestResult> GetTestResults() const { return TestResults; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Physics Integration Test")
    float GetOverallTestProgress() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Physics Integration Test")
    bool AreAllTestsPassing() const;

protected:
    // Core components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* TestMeshComponent;

    // Physics system references
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Systems")
    UPhysicsSystemManager* PhysicsSystemManager;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Systems")
    UCorePhysicsManager* CorePhysicsManager;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Systems")
    UDestructionPhysicsComponent* DestructionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Systems")
    UCore_RagdollComponent* RagdollComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Systems")
    UVehiclePhysicsSystemV2* VehiclePhysicsSystem;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Systems")
    UPhysicsValidationComponent* ValidationComponent;

    // Test configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Configuration")
    bool bRunTestsOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Configuration")
    bool bAutoResetAfterTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Configuration")
    float TestTimeout;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Configuration")
    bool bVerboseLogging;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Configuration")
    bool bDrawDebugInfo;

    // Test state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Test State")
    ECore_PhysicsTestState CurrentTestState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Test State")
    TArray<FCore_PhysicsTestResult> TestResults;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Test State")
    int32 CurrentTestIndex;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Test State")
    float TestStartTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Test State")
    float CurrentTestStartTime;

private:
    // Test execution methods
    void InitializeTestSystems();
    void ExecuteNextTest();
    void ValidateTestResults();
    void CompleteTests();
    void FailTests(const FString& Reason);

    // Individual test methods
    bool TestPhysicsSystemInitialization();
    bool TestCollisionDetection();
    bool TestRagdollPhysics();
    bool TestDestructionSystem();
    bool TestVehiclePhysics();
    bool TestSystemIntegration();
    bool TestPerformanceMetrics();
    bool TestMemoryUsage();

    // Utility methods
    void LogTestResult(const FString& TestName, bool bPassed, float ExecutionTime, const FString& ErrorMessage = TEXT(""));
    void DrawDebugInformation();
    bool IsTestTimedOut() const;
    void CleanupTestObjects();

    // Test objects and data
    TArray<AActor*> SpawnedTestActors;
    TMap<FString, float> PerformanceMetrics;
    float TotalTestDuration;
    int32 PassedTestCount;
    int32 FailedTestCount;
};