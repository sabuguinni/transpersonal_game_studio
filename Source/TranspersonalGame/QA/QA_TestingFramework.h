#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "QA_TestingFramework.generated.h"

UENUM(BlueprintType)
enum class EQA_TestResult : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    Fail        UMETA(DisplayName = "Fail"),
    Warning     UMETA(DisplayName = "Warning"),
    Skipped     UMETA(DisplayName = "Skipped")
};

UENUM(BlueprintType)
enum class EQA_TestCategory : uint8
{
    SystemValidation    UMETA(DisplayName = "System Validation"),
    Performance         UMETA(DisplayName = "Performance"),
    Integration         UMETA(DisplayName = "Integration"),
    Regression          UMETA(DisplayName = "Regression"),
    Functional          UMETA(DisplayName = "Functional")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_TestCase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Case")
    FString TestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Case")
    FString TestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Case")
    EQA_TestCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Case")
    EQA_TestResult Result;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Case")
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Case")
    float ExecutionTime;

    FQA_TestCase()
    {
        TestName = TEXT("");
        TestDescription = TEXT("");
        Category = EQA_TestCategory::SystemValidation;
        Result = EQA_TestResult::Skipped;
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_TestSuite
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Suite")
    FString SuiteName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Suite")
    TArray<FQA_TestCase> TestCases;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Suite")
    int32 PassedTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Suite")
    int32 FailedTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Suite")
    int32 WarningTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Suite")
    float TotalExecutionTime;

    FQA_TestSuite()
    {
        SuiteName = TEXT("");
        PassedTests = 0;
        FailedTests = 0;
        WarningTests = 0;
        TotalExecutionTime = 0.0f;
    }
};

/**
 * QA Testing Framework - Comprehensive testing system for TranspersonalGame
 * Provides automated testing capabilities for system validation, performance monitoring,
 * integration testing, and regression detection.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQA_TestingFramework : public AActor
{
    GENERATED_BODY()

public:
    AQA_TestingFramework();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    TArray<FQA_TestSuite> TestSuites;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    bool bAutoRunOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    bool bLogDetailedResults;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    float PerformanceThresholdMS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    int32 MaxActorCountThreshold;

public:
    // Core testing functions
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunTestSuite(const FString& SuiteName);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase RunSingleTest(const FString& TestName);

    // System validation tests
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidateGameState();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidateCharacterSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidateWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidateVFXSystem();

    // Performance tests
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase CheckActorCount();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase CheckFrameRate();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase CheckMemoryUsage();

    // Integration tests
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase TestDinosaurVFXIntegration();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase TestPlayerMovement();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase TestLevelEssentials();

    // Regression tests
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase CheckCriticalSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidateAgentDeliverables();

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void GenerateTestReport();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    float GetOverallPassRate() const;

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void ClearTestResults();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "QA Testing")
    void RunEditorTests();

private:
    // Helper functions
    FQA_TestCase CreateTestCase(const FString& Name, const FString& Description, EQA_TestCategory Category);
    void LogTestResult(const FQA_TestCase& TestCase);
    bool IsClassLoadable(const FString& ClassName);
    int32 CountActorsOfType(const FString& ActorType);
    void AddTestToSuite(const FString& SuiteName, const FQA_TestCase& TestCase);
    void InitializeTestSuites();
};