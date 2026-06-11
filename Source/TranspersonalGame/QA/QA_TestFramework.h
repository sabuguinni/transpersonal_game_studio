#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "QA_TestFramework.generated.h"

UENUM(BlueprintType)
enum class EQA_TestResult : uint8
{
    NotRun      UMETA(DisplayName = "Not Run"),
    Pass        UMETA(DisplayName = "Pass"),
    Fail        UMETA(DisplayName = "Fail"),
    Warning     UMETA(DisplayName = "Warning")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_TestCase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    FString TestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    FString TestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    EQA_TestResult Result;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
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

    FQA_TestSuite()
    {
        SuiteName = TEXT("");
        PassedTests = 0;
        FailedTests = 0;
        WarningTests = 0;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_TestFramework : public UObject
{
    GENERATED_BODY()

public:
    UQA_TestFramework();

    // Test execution functions
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunTestSuite(const FString& SuiteName);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunSingleTest(const FString& TestName);

    // Test registration functions
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RegisterTestCase(const FString& SuiteName, const FString& TestName, const FString& Description);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RegisterTestSuite(const FString& SuiteName);

    // Validation functions
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateClassLoading();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateActorSpawning();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidatePerformance();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateIntegration();

    // Reporting functions
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void GenerateTestReport();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FString GetTestSummary();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    TArray<FQA_TestCase> GetFailedTests();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Data")
    TArray<FQA_TestSuite> TestSuites;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Data")
    bool bAutoRunOnStart;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Data")
    float PerformanceThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Data")
    int32 MaxAllowedErrors;

private:
    // Internal test execution
    void ExecuteTest(FQA_TestCase& TestCase);
    void UpdateTestSuiteStats(FQA_TestSuite& Suite);
    void LogTestResult(const FQA_TestCase& TestCase);
    
    // Performance monitoring
    float StartTime;
    float EndTime;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQA_TestActor : public AActor
{
    GENERATED_BODY()

public:
    AQA_TestActor();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Testing")
    class UQA_TestFramework* TestFramework;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    bool bRunTestsOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    TArray<FString> TestSuitesToRun;

public:
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void StartTesting();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void StopTesting();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool IsTestingComplete();
};

#include "QA_TestFramework.generated.h"