#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "UObject/NoExportTypes.h"
#include "QATestFramework.generated.h"

UENUM(BlueprintType)
enum class EQA_TestResult : uint8
{
    NotRun      UMETA(DisplayName = "Not Run"),
    Pass        UMETA(DisplayName = "Pass"),
    Fail        UMETA(DisplayName = "Fail"),
    Warning     UMETA(DisplayName = "Warning"),
    Error       UMETA(DisplayName = "Error")
};

UENUM(BlueprintType)
enum class EQA_TestCategory : uint8
{
    Compilation     UMETA(DisplayName = "Compilation"),
    Functionality   UMETA(DisplayName = "Functionality"),
    Performance     UMETA(DisplayName = "Performance"),
    Integration     UMETA(DisplayName = "Integration"),
    Regression      UMETA(DisplayName = "Regression"),
    Gameplay        UMETA(DisplayName = "Gameplay")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_TestCase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Case")
    FString TestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Case")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Case")
    EQA_TestCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Case")
    EQA_TestResult Result;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Case")
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Case")
    float ExecutionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Case")
    FDateTime ExecutionTimestamp;

    FQA_TestCase()
    {
        TestName = TEXT("");
        Description = TEXT("");
        Category = EQA_TestCategory::Functionality;
        Result = EQA_TestResult::NotRun;
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
        ExecutionTimestamp = FDateTime::Now();
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
 * QA Test Framework - Core testing infrastructure for Transpersonal Game
 * Provides automated testing capabilities for all game systems
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQATestFramework : public UObject
{
    GENERATED_BODY()

public:
    UQATestFramework();

    // Core test execution methods
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunTestSuite(const FString& SuiteName);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunSingleTest(const FString& TestName);

    // Test registration methods
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RegisterTestCase(const FQA_TestCase& TestCase);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RegisterTestSuite(const FQA_TestSuite& TestSuite);

    // Validation methods
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateClassExists(const FString& ClassName);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateActorSpawning(UClass* ActorClass);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateComponentFunctionality(UActorComponent* Component);

    // Performance testing
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    float MeasureFrameRate(float Duration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    int32 CountActorsInLevel();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    float MeasureMemoryUsage();

    // Reporting methods
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void GenerateTestReport();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void ExportTestResults(const FString& FilePath);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestSuite GetTestSuiteResults(const FString& SuiteName);

    // Getters
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    TArray<FQA_TestSuite> GetAllTestSuites() const { return TestSuites; }

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    int32 GetTotalTestCount() const;

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    int32 GetPassedTestCount() const;

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    int32 GetFailedTestCount() const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    TArray<FQA_TestSuite> TestSuites;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    bool bAutoRunOnStartup;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    bool bGenerateDetailedLogs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    FString TestResultsDirectory;

private:
    // Internal test execution helpers
    void ExecuteTestCase(FQA_TestCase& TestCase);
    void UpdateTestSuiteStats(FQA_TestSuite& TestSuite);
    void LogTestResult(const FQA_TestCase& TestCase);
    FString GetTestResultString(EQA_TestResult Result);
};

/**
 * QA Test Actor - Spawnable actor for in-world testing scenarios
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQATestActor : public AActor
{
    GENERATED_BODY()

public:
    AQATestActor();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunActorTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void TestMovement();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void TestCollision();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void TestVisibility();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Testing")
    class UStaticMeshComponent* TestMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    bool bRunTestsOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    float TestDuration;

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    float TestTimer;
    bool bTestsCompleted;
};