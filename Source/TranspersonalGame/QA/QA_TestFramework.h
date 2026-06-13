#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "QA_TestFramework.generated.h"

UENUM(BlueprintType)
enum class EQA_TestResult : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    Fail        UMETA(DisplayName = "Fail"),
    Warning     UMETA(DisplayName = "Warning"),
    Error       UMETA(DisplayName = "Error"),
    Skipped     UMETA(DisplayName = "Skipped")
};

USTRUCT(BlueprintType)
struct FQA_TestCase
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
        Result = EQA_TestResult::Skipped;
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FQA_ValidationReport
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Report")
    TArray<FQA_TestCase> TestCases;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Report")
    int32 PassCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Report")
    int32 FailCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Report")
    int32 WarningCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Report")
    int32 ErrorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Report")
    bool bBuildApproved;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Report")
    FString ReportTimestamp;

    FQA_ValidationReport()
    {
        PassCount = 0;
        FailCount = 0;
        WarningCount = 0;
        ErrorCount = 0;
        bBuildApproved = false;
        ReportTimestamp = TEXT("");
    }
};

/**
 * QA Test Framework for comprehensive system validation
 * Handles automated testing, regression detection, and build approval
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_TestFramework : public UObject
{
    GENERATED_BODY()

public:
    UQA_TestFramework();

    // Core testing functions
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_ValidationReport RunFullValidation();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidateSystemIntegration(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase DetectRegressions();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidateActorSpawning(UClass* ActorClass);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ApproveBuildForIntegration(const FQA_ValidationReport& Report);

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void LogTestResult(const FQA_TestCase& TestCase);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void GenerateValidationReport(const FQA_ValidationReport& Report);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    TArray<AActor*> GetActorsByType(const FString& ActorType);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    int32 MaxActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    int32 MaxDinosaurCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    float PerformanceThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    TArray<FString> CriticalSystems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    bool bStrictValidation;

private:
    // Internal validation helpers
    FQA_TestCase ValidateCoreSystem(const FString& SystemName);
    FQA_TestCase CheckMemoryLeaks();
    FQA_TestCase ValidateComponentIntegrity();
    bool IsSystemCritical(const FString& SystemName) const;
    void UpdateTestCounts(FQA_ValidationReport& Report, const FQA_TestCase& TestCase);
};

/**
 * QA Manager Actor for in-world testing and validation
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQA_TestManager : public AActor
{
    GENERATED_BODY()

public:
    AQA_TestManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "QA Manager")
    void StartAutomatedTesting();

    UFUNCTION(BlueprintCallable, Category = "QA Manager")
    void StopAutomatedTesting();

    UFUNCTION(BlueprintCallable, Category = "QA Manager")
    void RunSingleTest(const FString& TestName);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Manager")
    class UQA_TestFramework* TestFramework;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    bool bRunContinuousTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    float TestInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    bool bLogAllResults;

private:
    float LastTestTime;
    bool bTestingActive;
    FQA_ValidationReport CurrentReport;
};