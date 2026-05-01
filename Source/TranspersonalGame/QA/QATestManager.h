#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "../SharedTypes.h"
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
struct FQA_ValidationReport
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
 * QA Test Manager - Comprehensive testing and validation system
 * Validates all game systems, integration points, and agent outputs
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQATestManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQATestManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core validation functions
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_ValidationReport RunFullValidation();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_ValidationReport RunSystemValidation();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_ValidationReport RunIntegrationTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_ValidationReport RunAgentOutputValidation();

    // Individual test categories
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidateCharacterSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidateWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidateDinosaurAI();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidateAudioSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidateVFXSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidateNarrativeSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidateMinPlayableMap();

    // Performance testing
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidatePerformance();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidateMemoryUsage();

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void SaveValidationReport(const FQA_ValidationReport& Report, const FString& Filename);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_ValidationReport LoadValidationReport(const FString& Filename);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void LogTestResults(const FQA_ValidationReport& Report);

    // Editor-only testing functions
    UFUNCTION(CallInEditor, Category = "QA Testing")
    void RunQuickValidation();

    UFUNCTION(CallInEditor, Category = "QA Testing")
    void RunFullValidationSuite();

protected:
    // Current validation report
    UPROPERTY(BlueprintReadOnly, Category = "QA Testing")
    FQA_ValidationReport CurrentReport;

    // Test configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    bool bEnablePerformanceTesting;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    bool bEnableMemoryTesting;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    bool bVerboseLogging;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    float MaxAcceptableFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    int32 MaxAcceptableMemoryMB;

private:
    // Helper functions
    FQA_TestCase CreateTestCase(const FString& Name, const FString& Description);
    void SetTestResult(FQA_TestCase& TestCase, EQA_TestResult Result, const FString& ErrorMessage = TEXT(""));
    bool ValidateClassExists(const FString& ClassName);
    bool ValidateActorExists(const FString& ActorName);
    bool ValidateAssetExists(const FString& AssetPath);
    
    // Performance monitoring
    float MeasureFrameTime();
    int32 MeasureMemoryUsage();
    
    // Test execution timing
    double TestStartTime;
    void StartTestTimer();
    float EndTestTimer();
};