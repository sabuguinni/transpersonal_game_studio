#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "QATestFramework.generated.h"

// Forward declarations
class UQATestCase;
class UQATestSuite;

UENUM(BlueprintType)
enum class EQA_TestResult : uint8
{
    NotRun      UMETA(DisplayName = "Not Run"),
    Passed      UMETA(DisplayName = "Passed"),
    Failed      UMETA(DisplayName = "Failed"),
    Skipped     UMETA(DisplayName = "Skipped"),
    Error       UMETA(DisplayName = "Error")
};

UENUM(BlueprintType)
enum class EQA_TestCategory : uint8
{
    Unit            UMETA(DisplayName = "Unit Test"),
    Integration     UMETA(DisplayName = "Integration Test"),
    Performance     UMETA(DisplayName = "Performance Test"),
    Functional      UMETA(DisplayName = "Functional Test"),
    Regression      UMETA(DisplayName = "Regression Test"),
    Smoke           UMETA(DisplayName = "Smoke Test")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_TestReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Test Report")
    FString TestName;

    UPROPERTY(BlueprintReadOnly, Category = "Test Report")
    EQA_TestResult Result;

    UPROPERTY(BlueprintReadOnly, Category = "Test Report")
    EQA_TestCategory Category;

    UPROPERTY(BlueprintReadOnly, Category = "Test Report")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Test Report")
    float ExecutionTime;

    UPROPERTY(BlueprintReadOnly, Category = "Test Report")
    FDateTime Timestamp;

    FQA_TestReport()
    {
        TestName = TEXT("");
        Result = EQA_TestResult::NotRun;
        Category = EQA_TestCategory::Unit;
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
        Timestamp = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_ValidationRule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    FString RuleName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    bool bIsEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    EQA_TestCategory Category;

    FQA_ValidationRule()
    {
        RuleName = TEXT("");
        Description = TEXT("");
        bIsEnabled = true;
        Category = EQA_TestCategory::Unit;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FQA_OnTestCompleted, const FQA_TestReport&, TestReport);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FQA_OnTestSuiteCompleted, const FString&, SuiteName, const TArray<FQA_TestReport>&, Results);

/**
 * Base class for all QA test cases
 */
UCLASS(BlueprintType, Blueprintable, Abstract)
class TRANSPERSONALGAME_API UQATestCase : public UObject
{
    GENERATED_BODY()

public:
    UQATestCase();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Case")
    FString TestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Case")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Case")
    EQA_TestCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Case")
    bool bIsEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Case")
    float TimeoutSeconds;

    UFUNCTION(BlueprintCallable, Category = "Test Case")
    virtual FQA_TestReport ExecuteTest();

    UFUNCTION(BlueprintImplementableEvent, Category = "Test Case")
    void OnTestSetup();

    UFUNCTION(BlueprintImplementableEvent, Category = "Test Case")
    void OnTestTeardown();

protected:
    UFUNCTION(BlueprintImplementableEvent, Category = "Test Case")
    bool RunTestLogic();

    UFUNCTION(BlueprintCallable, Category = "Test Case")
    void AssertTrue(bool bCondition, const FString& Message = TEXT(""));

    UFUNCTION(BlueprintCallable, Category = "Test Case")
    void AssertFalse(bool bCondition, const FString& Message = TEXT(""));

    UFUNCTION(BlueprintCallable, Category = "Test Case")
    void AssertEqual(const FString& Expected, const FString& Actual, const FString& Message = TEXT(""));

    UFUNCTION(BlueprintCallable, Category = "Test Case")
    void AssertNotNull(UObject* Object, const FString& Message = TEXT(""));

private:
    bool bTestPassed;
    FString LastErrorMessage;
};

/**
 * Test suite that manages multiple test cases
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQATestSuite : public UObject
{
    GENERATED_BODY()

public:
    UQATestSuite();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Suite")
    FString SuiteName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Suite")
    TArray<TSubclassOf<UQATestCase>> TestCases;

    UPROPERTY(BlueprintAssignable, Category = "Test Suite")
    FQA_OnTestCompleted OnTestCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Test Suite")
    FQA_OnTestSuiteCompleted OnTestSuiteCompleted;

    UFUNCTION(BlueprintCallable, Category = "Test Suite")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "Test Suite")
    void RunTestsByCategory(EQA_TestCategory Category);

    UFUNCTION(BlueprintCallable, Category = "Test Suite")
    TArray<FQA_TestReport> GetLastResults() const { return LastResults; }

    UFUNCTION(BlueprintCallable, Category = "Test Suite")
    void AddTestCase(TSubclassOf<UQATestCase> TestCaseClass);

private:
    TArray<FQA_TestReport> LastResults;
    void ExecuteTestCase(TSubclassOf<UQATestCase> TestCaseClass);
};

/**
 * Main QA Framework subsystem
 */
UCLASS()
class TRANSPERSONALGAME_API UQAFrameworkSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    static UQAFrameworkSubsystem* Get(const UObject* WorldContext);

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void RegisterTestSuite(UQATestSuite* TestSuite);

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void RunAllTestSuites();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void RunTestSuite(const FString& SuiteName);

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void RunSmokeTests();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void RunRegressionTests();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    TArray<FQA_ValidationRule> GetValidationRules() const { return ValidationRules; }

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void AddValidationRule(const FQA_ValidationRule& Rule);

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    bool ValidateGameState();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void GenerateTestReport(const FString& OutputPath);

protected:
    UPROPERTY()
    TArray<UQATestSuite*> RegisteredTestSuites;

    UPROPERTY()
    TArray<FQA_ValidationRule> ValidationRules;

    UPROPERTY()
    TArray<FQA_TestReport> AllTestResults;

private:
    void InitializeDefaultValidationRules();
    void ValidateModuleIntegrity();
    void ValidatePerformanceMetrics();
    void ValidateFunctionalSystems();
};

/**
 * QA Test Actor for in-world testing
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQATestActor : public AActor
{
    GENERATED_BODY()

public:
    AQATestActor();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    FString TestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    bool bAutoRunOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    TSubclassOf<UQATestSuite> TestSuiteClass;

    UFUNCTION(BlueprintCallable, Category = "QA Test")
    void RunTests();

    UFUNCTION(CallInEditor, Category = "QA Test")
    void RunTestsInEditor();

protected:
    virtual void BeginPlay() override;

    UPROPERTY()
    UQATestSuite* TestSuiteInstance;
};