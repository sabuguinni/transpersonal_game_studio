#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "QATestFramework.generated.h"

UENUM(BlueprintType)
enum class EQA_TestResult : uint8
{
    Pass,
    Fail,
    Warning,
    NotRun
};

UENUM(BlueprintType)
enum class EQA_TestCategory : uint8
{
    Core,
    Gameplay,
    Performance,
    Rendering,
    Audio,
    AI,
    Physics,
    UI,
    Integration
};

USTRUCT(BlueprintType)
struct FQA_TestCase
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString TestName;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString Description;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    EQA_TestCategory Category;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    EQA_TestResult Result;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString ErrorMessage;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float ExecutionTime;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    bool bIsCritical;

    FQA_TestCase()
    {
        TestName = TEXT("");
        Description = TEXT("");
        Category = EQA_TestCategory::Core;
        Result = EQA_TestResult::NotRun;
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
        bIsCritical = false;
    }
};

USTRUCT(BlueprintType)
struct FQA_ValidationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FDateTime Timestamp;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString CycleID;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TArray<FQA_TestCase> TestCases;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int32 PassedTests;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int32 FailedTests;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int32 WarningTests;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    bool bHasCriticalFailures;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString OverallStatus;

    FQA_ValidationReport()
    {
        Timestamp = FDateTime::Now();
        CycleID = TEXT("");
        PassedTests = 0;
        FailedTests = 0;
        WarningTests = 0;
        bHasCriticalFailures = false;
        OverallStatus = TEXT("Not Run");
    }
};

/**
 * QA Test Framework - Automated testing and validation system
 * Validates game systems, performance, and integration
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQATestFramework : public AActor
{
    GENERATED_BODY()

public:
    AQATestFramework();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Test execution
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "QA Testing")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "QA Testing")
    void RunTestCategory(EQA_TestCategory Category);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "QA Testing")
    void RunSingleTest(const FString& TestName);

    // Core system tests
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidateModuleLoading();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidateClassRegistration();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidateLevelContent();

    // Gameplay tests
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidateCharacterMovement();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidatePlayerInput();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidateWorldGeneration();

    // Performance tests
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidateFrameRate();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidateMemoryUsage();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidateLODSystem();

    // AI tests
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidateAIBehavior();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidateCrowdSimulation();

    // Integration tests
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidateSystemIntegration();

    // Report generation
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "QA Testing")
    void GenerateValidationReport();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_ValidationReport GetLastReport() const { return LastValidationReport; }

    // Configuration
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "QA Settings")
    bool bAutoRunOnBeginPlay;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "QA Settings")
    bool bLogDetailedResults;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "QA Settings")
    float PerformanceTestDuration;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "QA Settings")
    float TargetFrameRate;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "QA Settings")
    float MaxMemoryUsageMB;

protected:
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "QA State")
    FQA_ValidationReport LastValidationReport;

    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "QA State")
    bool bTestsRunning;

    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "QA State")
    float TestStartTime;

private:
    // Helper functions
    FQA_TestCase CreateTestCase(const FString& Name, const FString& Description, EQA_TestCategory Category, bool bCritical = false);
    void LogTestResult(const FQA_TestCase& TestCase);
    void UpdateReportStatistics();
    bool ValidateActorClass(const FString& ClassName);
    bool CheckMinimumPlayableRequirements();
};