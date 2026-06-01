#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "QA_TestFramework.generated.h"

UENUM(BlueprintType)
enum class EQA_TestResult : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    Fail        UMETA(DisplayName = "Fail"),
    Warning     UMETA(DisplayName = "Warning"),
    Skipped     UMETA(DisplayName = "Skipped")
};

USTRUCT(BlueprintType)
struct FQA_TestCase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    FString TestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    FString TestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    EQA_TestResult Result;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    float ExecutionTime;

    FQA_TestCase()
    {
        TestName = TEXT("");
        TestDescription = TEXT("");
        Result = EQA_TestResult::Skipped;
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FQA_TestSuite
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test Suite")
    FString SuiteName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test Suite")
    TArray<FQA_TestCase> TestCases;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test Suite")
    int32 PassCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test Suite")
    int32 FailCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test Suite")
    int32 WarningCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test Suite")
    float TotalExecutionTime;

    FQA_TestSuite()
    {
        SuiteName = TEXT("");
        PassCount = 0;
        FailCount = 0;
        WarningCount = 0;
        TotalExecutionTime = 0.0f;
    }
};

/**
 * QA Test Framework for automated testing and validation
 * Provides comprehensive testing capabilities for all game systems
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_TestFramework : public UObject
{
    GENERATED_BODY()

public:
    UQA_TestFramework();

    // Core test execution methods
    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void RunTestSuite(const FString& SuiteName);

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    FQA_TestCase RunSingleTest(const FString& TestName);

    // Test registration
    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void RegisterTestCase(const FString& SuiteName, const FString& TestName, const FString& Description);

    // Validation methods
    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    FQA_TestCase ValidateActorCount();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    FQA_TestCase ValidateDinosaurLimits();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    FQA_TestCase ValidateModuleClasses();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    FQA_TestCase ValidateBiomeDistribution();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    FQA_TestCase ValidatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    FQA_TestCase ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    FQA_TestCase ValidateCharacterMovement();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    FQA_TestCase ValidateGameplayMechanics();

    // Reporting methods
    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void GenerateTestReport();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void ExportTestResults(const FString& FilePath);

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    FString GetTestSummary();

    // Utility methods
    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void ClearTestResults();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    bool IsTestPassing(const FString& TestName);

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    TArray<FString> GetFailedTests();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Framework")
    TArray<FQA_TestSuite> TestSuites;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Framework")
    bool bAutoRunOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Framework")
    bool bLogTestResults;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Framework")
    float TestTimeout;

private:
    // Helper methods
    FQA_TestCase CreateTestCase(const FString& Name, const FString& Description);
    void LogTestResult(const FQA_TestCase& TestCase);
    float GetCurrentTime();
    void UpdateTestSuiteStats(FQA_TestSuite& Suite);
};

/**
 * QA Test Actor for in-world testing scenarios
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQA_TestActor : public AActor
{
    GENERATED_BODY()

public:
    AQA_TestActor();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Test Actor")
    class UQA_TestFramework* TestFramework;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test Actor")
    bool bRunTestsOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test Actor")
    TArray<FString> TestSuitesToRun;

public:
    UFUNCTION(BlueprintCallable, Category = "QA Test Actor")
    void ExecuteTests();

    UFUNCTION(BlueprintCallable, Category = "QA Test Actor")
    void ShowTestResults();
};