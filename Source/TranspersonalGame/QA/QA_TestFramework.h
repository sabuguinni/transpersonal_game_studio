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
    Passed      UMETA(DisplayName = "Passed"),
    Failed      UMETA(DisplayName = "Failed"),
    Warning     UMETA(DisplayName = "Warning"),
    Error       UMETA(DisplayName = "Error")
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
struct TRANSPERSONALGAME_API FQA_ValidationReport
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Report")
    FString CycleID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Report")
    FDateTime Timestamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Report")
    TArray<FQA_TestCase> TestCases;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Report")
    int32 PassedTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Report")
    int32 FailedTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Report")
    int32 WarningTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Report")
    bool bBuildBlocked;

    FQA_ValidationReport()
    {
        CycleID = TEXT("");
        Timestamp = FDateTime::Now();
        PassedTests = 0;
        FailedTests = 0;
        WarningTests = 0;
        bBuildBlocked = false;
    }
};

/**
 * QA Test Framework - Comprehensive testing and validation system
 * Provides automated testing capabilities for all game systems
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQA_TestFramework : public AActor
{
    GENERATED_BODY()

public:
    AQA_TestFramework();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core Testing Functions
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunClassValidationTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunPerformanceTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunIntegrationTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunMapValidationTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_ValidationReport GenerateValidationReport();

    // Test Management
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void AddTestCase(const FString& TestName, const FString& Description);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void SetTestResult(const FString& TestName, EQA_TestResult Result, const FString& ErrorMessage = TEXT(""));

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool IsTestPassed(const FString& TestName) const;

    // Build Validation
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateBuildIntegrity();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool BlockBuild(const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void UnblockBuild();

protected:
    // Test Data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Testing")
    TArray<FQA_TestCase> TestCases;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Testing")
    FQA_ValidationReport CurrentReport;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    bool bAutoRunTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    float TestInterval;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Testing")
    bool bBuildBlocked;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Testing")
    FString BuildBlockReason;

private:
    // Internal test functions
    void ValidateTranspersonalGameClasses();
    void ValidateVFXSystem();
    void ValidateWorldGeneration();
    void ValidateCharacterSystem();
    void ValidateAISystem();
    void ValidatePerformanceMetrics();
    void ValidateMapContent();
    void ValidateCrossSystemIntegration();

    // Utility functions
    FQA_TestCase* FindTestCase(const FString& TestName);
    void LogTestResult(const FQA_TestCase& TestCase);
    void SaveReportToFile(const FQA_ValidationReport& Report);

    float LastTestTime;
};