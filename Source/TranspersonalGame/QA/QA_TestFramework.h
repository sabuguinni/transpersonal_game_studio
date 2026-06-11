#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "QA_TestFramework.generated.h"

UENUM(BlueprintType)
enum class EQA_TestResult : uint8
{
    NotRun      UMETA(DisplayName = "Not Run"),
    Pass        UMETA(DisplayName = "Pass"),
    Fail        UMETA(DisplayName = "Fail"),
    Warning     UMETA(DisplayName = "Warning"),
    Critical    UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_TestCase
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
        Result = EQA_TestResult::NotRun;
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_TestSuite
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Suite")
    FString SuiteName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Suite")
    TArray<FQA_TestCase> TestCases;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Suite")
    int32 PassedTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Suite")
    int32 FailedTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Suite")
    int32 WarningTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Suite")
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

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_TestFramework : public UActorComponent
{
    GENERATED_BODY()

public:
    UQA_TestFramework();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Test Suite Management
    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void InitializeTestSuite(const FString& SuiteName);

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void AddTestCase(const FString& TestName, const FString& Description);

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void RunSpecificTest(const FString& TestName);

    // Test Execution
    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    bool ValidateActorSpawning();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    bool ValidateComponentIntegration();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    bool ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    bool ValidateAudioSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    bool ValidatePerformanceMetrics();

    // Reporting
    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void GenerateTestReport();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    FString GetTestSummary();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void LogTestResult(const FString& TestName, EQA_TestResult Result, const FString& Message = TEXT(""));

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Framework")
    FQA_TestSuite CurrentTestSuite;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Framework")
    TArray<FQA_TestSuite> CompletedTestSuites;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Framework")
    bool bAutoRunTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Framework")
    float TestTimeout;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Framework")
    bool bVerboseLogging;

private:
    void ExecuteTestCase(FQA_TestCase& TestCase);
    void UpdateTestSuiteStats();
    float GetCurrentTime();
};