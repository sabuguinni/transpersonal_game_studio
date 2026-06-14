#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
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
        Result = EQA_TestResult::Skipped;
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
 * QA Test Framework Component for automated testing of game systems
 * Provides comprehensive validation of VFX, audio, lighting, and performance
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
    void RunVFXTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunAudioTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunLightingTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunPerformanceTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunIntegrationTests();

    // Test result functions
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestSuite GetTestResults() const { return TestSuite; }

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void ClearTestResults();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void ExportTestResults(const FString& FilePath);

    // Validation functions
    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateWeatherSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateParticleEffects();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateAudioIntegration();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateLightingSetup();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidatePerformanceMetrics();

protected:
    // Test suite data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Test Results")
    FQA_TestSuite TestSuite;

    // Test configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Configuration")
    bool bAutoRunTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Configuration")
    float TestInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Configuration")
    bool bLogTestResults;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Configuration")
    int32 MaxActorCountThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Configuration")
    float TargetFrameRate;

private:
    // Internal test functions
    void AddTestCase(const FString& TestName, const FString& Description, EQA_TestResult Result, const FString& ErrorMsg = TEXT(""));
    void LogTestResult(const FQA_TestCase& TestCase);
    void CalculateTestSuiteStats();

    // Timer for automatic testing
    float TestTimer;
    bool bTestsCompleted;
};