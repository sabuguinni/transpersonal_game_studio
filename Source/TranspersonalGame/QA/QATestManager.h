#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "../Core/SharedTypes.h"
#include "QATestManager.generated.h"

UENUM(BlueprintType)
enum class EQA_TestResult : uint8
{
    NotRun      UMETA(DisplayName = "Not Run"),
    Passed      UMETA(DisplayName = "Passed"),
    Failed      UMETA(DisplayName = "Failed"),
    Warning     UMETA(DisplayName = "Warning")
};

USTRUCT(BlueprintType)
struct FQA_TestCase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    FString TestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    EQA_TestResult Result;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    float ExecutionTime;

    FQA_TestCase()
    {
        TestName = TEXT("");
        Description = TEXT("");
        Result = EQA_TestResult::NotRun;
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
    int32 PassedTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Report")
    int32 FailedTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Report")
    int32 WarningTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Report")
    float TotalExecutionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Report")
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
 * QA Test Manager - Handles automated testing and validation of game systems
 * Validates C++ classes, actor spawning, component functionality, and integration scenarios
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_TestManager : public UObject
{
    GENERATED_BODY()

public:
    UQA_TestManager();

    // Core validation functions
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_ValidationReport RunFullValidation();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidateClassLoading(const FString& ClassName);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidateActorSpawning(UClass* ActorClass, FVector SpawnLocation);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidateComponentFunctionality(UActorComponent* Component);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidateMinPlayableMap();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidateCharacterMovement();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidateDinosaurAI();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidateWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidateAudioSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidateVFXSystems();

    // Performance testing
    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    FQA_TestCase ValidateFrameRate();

    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    FQA_TestCase ValidateMemoryUsage();

    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    FQA_TestCase ValidateLoadTimes();

    // Integration testing
    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    FQA_TestCase ValidatePlayerCharacterIntegration();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    FQA_TestCase ValidateWorldActorIntegration();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    FQA_TestCase ValidateCrowdSimulationIntegration();

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "QA Utility")
    void LogTestResult(const FQA_TestCase& TestCase);

    UFUNCTION(BlueprintCallable, Category = "QA Utility")
    void SaveValidationReport(const FQA_ValidationReport& Report, const FString& FilePath);

    UFUNCTION(BlueprintCallable, Category = "QA Utility")
    TArray<FString> GetAllTranspersonalGameClasses();

protected:
    // Internal test execution
    FQA_TestCase ExecuteTest(const FString& TestName, TFunction<bool()> TestFunction);
    
    // Test result tracking
    UPROPERTY()
    FQA_ValidationReport CurrentReport;

    // Test configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    float TestTimeout;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    bool bVerboseLogging;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    bool bAutoSaveReports;

private:
    // Helper functions
    bool IsClassValid(UClass* Class);
    bool CanSpawnActor(UClass* ActorClass);
    float MeasureExecutionTime(TFunction<void()> Function);
};