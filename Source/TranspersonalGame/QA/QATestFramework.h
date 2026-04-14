#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "QATestFramework.generated.h"

UENUM(BlueprintType)
enum class EQA_TestResult : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    Fail        UMETA(DisplayName = "Fail"),
    Error       UMETA(DisplayName = "Error"),
    Skipped     UMETA(DisplayName = "Skipped"),
    InProgress  UMETA(DisplayName = "In Progress")
};

UENUM(BlueprintType)
enum class EQA_TestCategory : uint8
{
    Core            UMETA(DisplayName = "Core Systems"),
    Integration     UMETA(DisplayName = "Integration"),
    Performance     UMETA(DisplayName = "Performance"),
    Gameplay        UMETA(DisplayName = "Gameplay"),
    Audio           UMETA(DisplayName = "Audio"),
    Visual          UMETA(DisplayName = "Visual"),
    Network         UMETA(DisplayName = "Network"),
    Memory          UMETA(DisplayName = "Memory"),
    Stability       UMETA(DisplayName = "Stability")
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Case")
    FDateTime ExecutionTimestamp;

    FQA_TestCase()
    {
        TestName = TEXT("");
        TestDescription = TEXT("");
        Category = EQA_TestCategory::Core;
        Result = EQA_TestResult::InProgress;
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
    int32 ErrorTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Suite")
    float TotalExecutionTime;

    FQA_TestSuite()
    {
        SuiteName = TEXT("");
        PassedTests = 0;
        FailedTests = 0;
        ErrorTests = 0;
        TotalExecutionTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_TestFramework : public UObject
{
    GENERATED_BODY()

public:
    UQA_TestFramework();

    // Core test execution functions
    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void RunTestSuite(const FString& SuiteName);

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void RunSingleTest(const FString& TestName);

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    FQA_TestSuite GetTestSuiteResults(const FString& SuiteName);

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void GenerateTestReport();

    // Test registration functions
    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void RegisterTestCase(const FQA_TestCase& TestCase);

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void RegisterTestSuite(const FQA_TestSuite& TestSuite);

    // Validation functions
    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    bool ValidateClassExists(const FString& ClassName);

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    bool ValidateActorSpawning(UClass* ActorClass);

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    bool ValidateComponentFunctionality(UActorComponent* Component);

    // Performance testing
    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    float MeasureFrameRate();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    float MeasureMemoryUsage();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void RunPerformanceBenchmark();

    // Integration testing
    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    bool TestSystemIntegration(const FString& SystemA, const FString& SystemB);

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void RunIntegrationTests();

    // Automated testing
    UFUNCTION(BlueprintCallable, Category = "QA Framework", CallInEditor = true)
    void RunAutomatedTestSuite();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void SchedulePeriodicTests(float IntervalSeconds);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Framework")
    TMap<FString, FQA_TestSuite> TestSuites;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Framework")
    TArray<FQA_TestCase> RegisteredTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Framework")
    bool bAutomatedTestingEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Framework")
    float TestTimeoutSeconds;

private:
    void InitializeDefaultTests();
    void ExecuteTestCase(FQA_TestCase& TestCase);
    void UpdateTestSuiteStats(FQA_TestSuite& TestSuite);
    void LogTestResult(const FQA_TestCase& TestCase);
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQA_TestActor : public AActor
{
    GENERATED_BODY()

public:
    AQA_TestActor();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Test Actor")
    class UQA_TestFramework* TestFramework;

    UFUNCTION(BlueprintCallable, Category = "QA Test Actor")
    void StartAutomatedTesting();

    UFUNCTION(BlueprintCallable, Category = "QA Test Actor")
    void StopAutomatedTesting();

    UFUNCTION(BlueprintCallable, Category = "QA Test Actor")
    void RunQuickValidation();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test Actor")
    bool bRunTestsOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test Actor")
    float TestInterval;

private:
    FTimerHandle TestTimerHandle;
    void PeriodicTestExecution();
};