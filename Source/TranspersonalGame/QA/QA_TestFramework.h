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
    Warning     UMETA(DisplayName = "Warning")
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test Suite")
    FString SuiteName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test Suite")
    TArray<FQA_TestCase> TestCases;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test Suite")
    int32 PassedTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test Suite")
    int32 FailedTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test Suite")
    int32 WarningTests;

    FQA_TestSuite()
    {
        SuiteName = TEXT("");
        PassedTests = 0;
        FailedTests = 0;
        WarningTests = 0;
    }
};

/**
 * QA Test Framework Component
 * Provides automated testing capabilities for game systems
 */
UCLASS(ClassGroup=(QA), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQA_TestFramework : public UActorComponent
{
    GENERATED_BODY()

public:
    UQA_TestFramework();

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

    // Test registration functions
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RegisterTestCase(const FString& SuiteName, const FQA_TestCase& TestCase);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RegisterTestSuite(const FQA_TestSuite& TestSuite);

    // Validation functions
    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateActorCount(int32 MaxActors = 1000);

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateLightCount(int32 MaxLights = 20);

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidatePerformance();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateAssetLoading(const TArray<FString>& AssetPaths);

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateClassCompilation(const TArray<FString>& ClassPaths);

    // Reporting functions
    UFUNCTION(BlueprintCallable, Category = "QA Reporting")
    FString GenerateTestReport();

    UFUNCTION(BlueprintCallable, Category = "QA Reporting")
    void LogTestResults();

    UFUNCTION(BlueprintCallable, Category = "QA Reporting")
    void ExportTestResults(const FString& FilePath);

protected:
    // Test suites storage
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Framework")
    TMap<FString, FQA_TestSuite> TestSuites;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Configuration")
    bool bAutoRunOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Configuration")
    bool bLogDetailedResults;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Configuration")
    float TestTimeout;

    // Performance thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Performance")
    int32 MaxActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Performance")
    int32 MaxLightCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Performance")
    float MaxFrameTime;

private:
    // Internal test execution
    EQA_TestResult ExecuteTest(const FQA_TestCase& TestCase);
    void UpdateTestSuiteStats(FQA_TestSuite& TestSuite);
    
    // Built-in test implementations
    EQA_TestResult TestActorSpawning();
    EQA_TestResult TestAssetLoading();
    EQA_TestResult TestClassCompilation();
    EQA_TestResult TestPerformanceMetrics();
    EQA_TestResult TestGameplaySystems();
};