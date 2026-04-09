// Copyright Transpersonal Game Studio. All Rights Reserved.
// V42_VerificationTest.h - Comprehensive verification test for v4.2 deployment

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "V42_VerificationTest.generated.h"

/**
 * VERIFICATION TEST RESULT STRUCTURE
 * 
 * Stores the results of individual verification tests
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVerificationTestResult
{
    GENERATED_BODY()

    // Test identification
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Test Result")
    FString TestName;

    // Test outcome
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Test Result")
    bool bPassed;

    // Detailed information about the test
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Test Result")
    TArray<FString> Details;

    // Test execution time
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Test Result")
    float ExecutionTime;

    FVerificationTestResult()
    {
        TestName = TEXT("Unknown Test");
        bPassed = false;
        ExecutionTime = 0.0f;
    }
};

/**
 * V4.2 VERIFICATION TEST COMPONENT
 * 
 * Comprehensive testing system to verify v4.2 deployment integrity
 * 
 * TESTS PERFORMED:
 * - Core subsystem functionality
 * - Studio Director system operation
 * - Engine architecture validation
 * - Module registration verification
 * - Performance baseline measurement
 * - Asset system integrity
 * - Memory management verification
 * - Threading safety checks
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UV42VerificationTest : public UActorComponent
{
    GENERATED_BODY()

public:
    UV42VerificationTest();

protected:
    // Component lifecycle
    virtual void BeginPlay() override;

public:
    /**
     * Run all verification tests
     * This is the main entry point for the verification system
     */
    UFUNCTION(BlueprintCallable, Category = "Verification")
    void RunVerificationTests();

    /**
     * Get the results of all completed tests
     */
    UFUNCTION(BlueprintCallable, Category = "Verification")
    TArray<FVerificationTestResult> GetTestResults() const;

    /**
     * Check if all tests have been completed
     */
    UFUNCTION(BlueprintCallable, Category = "Verification")
    bool AreTestsCompleted() const;

    /**
     * Get the overall success rate as a percentage
     */
    UFUNCTION(BlueprintCallable, Category = "Verification")
    float GetOverallSuccessRate() const;

protected:
    // Individual test methods
    void TestCoreSubsystems();
    void TestStudioDirectorSystem();
    void TestEngineArchitecture();
    void TestModuleRegistration();
    void TestPerformanceBaseline();
    void TestAssetSystem();
    void TestMemoryManagement();
    void TestThreadingSafety();

    // Report generation
    void GenerateVerificationReport();

    // Test results storage
    UPROPERTY(BlueprintReadOnly, Category = "Results")
    TArray<FVerificationTestResult> TestResults;

    // Test state tracking
    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bTestsCompleted;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float TestStartTime;

private:
    // Timer for delayed test execution
    FTimerHandle TestTimerHandle;
};

/**
 * VERIFICATION TEST ACTOR
 * 
 * Actor that can be placed in levels to run verification tests
 */
UCLASS()
class TRANSPERSONALGAME_API AV42VerificationTestActor : public AActor
{
    GENERATED_BODY()

public:
    AV42VerificationTestActor();

protected:
    virtual void BeginPlay() override;

    // Verification component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UV42VerificationTest* VerificationComponent;

public:
    /**
     * Start verification tests manually
     */
    UFUNCTION(BlueprintCallable, Category = "Verification")
    void StartVerificationTests();

    /**
     * Get verification results
     */
    UFUNCTION(BlueprintCallable, Category = "Verification")
    TArray<FVerificationTestResult> GetVerificationResults() const;

    /**
     * Check if verification is complete
     */
    UFUNCTION(BlueprintCallable, Category = "Verification")
    bool IsVerificationComplete() const;

protected:
    // Auto-start verification on begin play
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bAutoStartVerification;

    // Log results to console
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bLogResultsToConsole;
};

/**
 * VERIFICATION TEST SUBSYSTEM
 * 
 * Global subsystem for managing verification tests across the project
 */
UCLASS()
class TRANSPERSONALGAME_API UV42VerificationSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /**
     * Run global verification tests
     */
    UFUNCTION(BlueprintCallable, Category = "Verification")
    void RunGlobalVerificationTests();

    /**
     * Get the last verification results
     */
    UFUNCTION(BlueprintCallable, Category = "Verification")
    TArray<FVerificationTestResult> GetLastVerificationResults() const;

    /**
     * Check if the last verification was successful
     */
    UFUNCTION(BlueprintCallable, Category = "Verification")
    bool WasLastVerificationSuccessful() const;

    /**
     * Get verification statistics
     */
    UFUNCTION(BlueprintCallable, Category = "Verification")
    void GetVerificationStatistics(int32& TotalTests, int32& PassedTests, float& SuccessRate) const;

protected:
    // Last verification results
    UPROPERTY()
    TArray<FVerificationTestResult> LastResults;

    // Verification history
    UPROPERTY()
    TArray<float> VerificationHistory;

private:
    void LogVerificationSummary() const;
};