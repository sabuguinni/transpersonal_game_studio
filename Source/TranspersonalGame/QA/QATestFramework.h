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
struct TRANSPERSONALGAME_API FQA_ValidationReport
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Report")
    TArray<FQA_TestCase> TestCases;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Report")
    int32 PassedTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Report")
    int32 FailedTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Report")
    int32 WarningTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Report")
    float TotalExecutionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Report")
    FDateTime ReportTimestamp;

    FQA_ValidationReport()
    {
        PassedTests = 0;
        FailedTests = 0;
        WarningTests = 0;
        TotalExecutionTime = 0.0f;
        ReportTimestamp = FDateTime::Now();
    }
};

/**
 * QA Test Framework for automated validation of game systems
 * Provides comprehensive testing capabilities for all game modules
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

    // Core testing functions
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunCoreSystemTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunVFXSystemTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunPerformanceTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunGameplayTests();

    // Individual test categories
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase TestCharacterSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase TestWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase TestVFXIntegration();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase TestAudioSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase TestLevelIntegrity();

    // Validation utilities
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateClassExists(const FString& ClassName);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateActorSpawning(UClass* ActorClass);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    int32 CountActorsOfType(UClass* ActorClass);

    // Report generation
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_ValidationReport GenerateValidationReport();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void SaveReportToFile(const FQA_ValidationReport& Report, const FString& FilePath);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void LogTestResults(const FQA_ValidationReport& Report);

protected:
    // Test configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    bool bAutoRunTestsOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    bool bLogDetailedResults;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    float PerformanceTestDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    int32 MaxAllowedActorCount;

    // Test state
    UPROPERTY(BlueprintReadOnly, Category = "QA State")
    FQA_ValidationReport CurrentReport;

    UPROPERTY(BlueprintReadOnly, Category = "QA State")
    bool bTestsRunning;

    UPROPERTY(BlueprintReadOnly, Category = "QA State")
    float TestStartTime;

private:
    // Internal test helpers
    FQA_TestCase CreateTestCase(const FString& Name, const FString& Description);
    void CompleteTestCase(FQA_TestCase& TestCase, EQA_TestResult Result, const FString& ErrorMsg = TEXT(""));
    void AddTestToReport(const FQA_TestCase& TestCase);
    
    // Performance monitoring
    void StartPerformanceMonitoring();
    void StopPerformanceMonitoring();
    float GetCurrentFrameRate();
    int32 GetCurrentActorCount();
};