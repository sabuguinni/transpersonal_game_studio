#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "QATestManager.generated.h"

UENUM(BlueprintType)
enum class EQA_TestResult : uint8
{
    NotRun      UMETA(DisplayName = "Not Run"),
    Pass        UMETA(DisplayName = "Pass"),
    Fail        UMETA(DisplayName = "Fail"),
    Warning     UMETA(DisplayName = "Warning")
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
        Result = EQA_TestResult::NotRun;
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

    FQA_TestSuite()
    {
        SuiteName = TEXT("");
        PassCount = 0;
        FailCount = 0;
        WarningCount = 0;
    }
};

/**
 * QA Test Manager - Manages automated testing and validation
 * Ensures game quality and prevents regressions
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQATestManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UQATestManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Test execution functions
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunTestSuite(const FString& SuiteName);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunSingleTest(const FString& TestName);

    // Test validation functions
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateMapActors();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidatePlayerCharacter();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateDinosaurActors();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateLightingSetup();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateVFXSystems();

    // Performance testing
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateFrameRate();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateMemoryUsage();

    // Reporting functions
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void GenerateTestReport();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FString GetTestSummary();

    // Test result accessors
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    TArray<FQA_TestSuite> GetTestSuites() const { return TestSuites; }

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    int32 GetTotalPassCount() const;

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    int32 GetTotalFailCount() const;

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    int32 GetTotalWarningCount() const;

protected:
    // Test data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    TArray<FQA_TestSuite> TestSuites;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    bool bAutoRunTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    float TestInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    bool bGenerateDetailedLogs;

    // Performance thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Performance")
    float MinFrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Performance")
    float MaxMemoryUsageMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Performance")
    int32 MaxActorCount;

private:
    // Internal test functions
    void InitializeTestSuites();
    void AddTestCase(const FString& SuiteName, const FString& TestName, const FString& Description);
    void UpdateTestResult(const FString& SuiteName, const FString& TestName, EQA_TestResult Result, const FString& ErrorMessage = TEXT(""));
    void LogTestResult(const FQA_TestCase& TestCase);

    // Timer for auto-testing
    float TestTimer;
    bool bTestsInitialized;
};