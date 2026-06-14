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
    Warning     UMETA(DisplayName = "Warning"),
    Error       UMETA(DisplayName = "Error"),
    Skipped     UMETA(DisplayName = "Skipped")
};

UENUM(BlueprintType)
enum class EQA_TestCategory : uint8
{
    Compilation     UMETA(DisplayName = "Compilation"),
    Performance     UMETA(DisplayName = "Performance"),
    Integration     UMETA(DisplayName = "Integration"),
    VFX             UMETA(DisplayName = "VFX"),
    Lighting        UMETA(DisplayName = "Lighting"),
    Collision       UMETA(DisplayName = "Collision"),
    Regression      UMETA(DisplayName = "Regression"),
    Functional      UMETA(DisplayName = "Functional")
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
        Category = EQA_TestCategory::Functional;
        Result = EQA_TestResult::Skipped;
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
    int32 PassCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Suite")
    int32 FailCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Suite")
    int32 WarningCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Suite")
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
 * QA Test Framework - Automated testing system for TranspersonalGame
 * Validates VFX systems, performance, integration, and regression testing
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQATestFramework : public UObject
{
    GENERATED_BODY()

public:
    UQATestFramework();

    // Core testing functions
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunTestSuite(EQA_TestCategory Category);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestSuite GetTestResults(EQA_TestCategory Category);

    // Specific test categories
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void TestVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void TestPerformance();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void TestIntegration();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void TestRegression();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void TestCompilation();

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void GenerateTestReport();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void ClearTestResults();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateActorCount();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateCampfireLighting();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateImpactTriggers();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Data")
    TMap<EQA_TestCategory, FQA_TestSuite> TestSuites;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Configuration")
    int32 MaxActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Configuration")
    int32 MaxDinosaurCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Configuration")
    float MinFrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Configuration")
    float MaxMemoryUsageMB;

private:
    void AddTestCase(EQA_TestCategory Category, const FQA_TestCase& TestCase);
    void LogTestResult(const FQA_TestCase& TestCase);
    FQA_TestCase CreateTestCase(const FString& Name, EQA_TestCategory Category, const FString& Description);
};

/**
 * QA Test Manager Actor - Manages automated testing in the level
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQATestManager : public AActor
{
    GENERATED_BODY()

public:
    AQATestManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Testing")
    class UQATestFramework* TestFramework;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Configuration")
    bool bAutoRunTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Configuration")
    float TestInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Configuration")
    bool bContinuousMonitoring;

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void StartAutomatedTesting();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void StopAutomatedTesting();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunManualTest(EQA_TestCategory Category);

private:
    float LastTestTime;
    bool bTestingActive;
};