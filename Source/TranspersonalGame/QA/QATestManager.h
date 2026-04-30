#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "QATestManager.generated.h"

UENUM(BlueprintType)
enum class EQA_TestResult : uint8
{
    NotRun      UMETA(DisplayName = "Not Run"),
    Passed      UMETA(DisplayName = "Passed"),
    Failed      UMETA(DisplayName = "Failed"),
    Warning     UMETA(DisplayName = "Warning")
};

UENUM(BlueprintType)
enum class EQA_TestCategory : uint8
{
    Compilation     UMETA(DisplayName = "Compilation"),
    Functionality   UMETA(DisplayName = "Functionality"),
    Performance     UMETA(DisplayName = "Performance"),
    Integration     UMETA(DisplayName = "Integration"),
    Gameplay        UMETA(DisplayName = "Gameplay")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_TestCase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    FString TestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    EQA_TestCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    EQA_TestResult Result;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    float ExecutionTime;

    FQA_TestCase()
    {
        TestName = TEXT("");
        Category = EQA_TestCategory::Functionality;
        Result = EQA_TestResult::NotRun;
        Description = TEXT("");
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
    float TotalExecutionTime;

    FQA_TestSuite()
    {
        SuiteName = TEXT("");
        PassedTests = 0;
        FailedTests = 0;
        TotalExecutionTime = 0.0f;
    }
};

/**
 * QA Test Manager - Handles automated testing and validation
 * Ensures game quality and catches regressions early
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQATestManager : public UObject
{
    GENERATED_BODY()

public:
    UQATestManager();

    // Core test execution
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunTestSuite(const FString& SuiteName);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunSingleTest(const FString& TestName);

    // Test registration
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RegisterTestCase(const FQA_TestCase& TestCase);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RegisterTestSuite(const FQA_TestSuite& TestSuite);

    // Results and reporting
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestSuite GetTestSuite(const FString& SuiteName) const;

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    TArray<FQA_TestCase> GetFailedTests() const;

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    float GetOverallSuccessRate() const;

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void GenerateTestReport();

    // Specific test categories
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void TestModuleCompilation();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void TestGameplayFunctionality();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void TestPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void TestIntegrationScenarios();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Data")
    TMap<FString, FQA_TestSuite> TestSuites;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Data")
    TArray<FQA_TestCase> AllTestCases;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bAutoRunOnStartup;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bGenerateDetailedLogs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float PerformanceThresholdFPS;

private:
    // Internal test execution
    EQA_TestResult ExecuteTest(FQA_TestCase& TestCase);
    void LogTestResult(const FQA_TestCase& TestCase);
    void UpdateTestSuiteStats(FQA_TestSuite& TestSuite);
    
    // Specific test implementations
    EQA_TestResult TestClassLoading(const FString& ClassName);
    EQA_TestResult TestActorSpawning(UClass* ActorClass);
    EQA_TestResult TestComponentFunctionality(UActorComponent* Component);
    EQA_TestResult TestMapLoading(const FString& MapPath);
};