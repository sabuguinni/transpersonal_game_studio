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
    Warning     UMETA(DisplayName = "Warning"),
    Critical    UMETA(DisplayName = "Critical")
};

UENUM(BlueprintType)
enum class EQA_TestCategory : uint8
{
    Core        UMETA(DisplayName = "Core Systems"),
    Gameplay    UMETA(DisplayName = "Gameplay"),
    Performance UMETA(DisplayName = "Performance"),
    Audio       UMETA(DisplayName = "Audio"),
    VFX         UMETA(DisplayName = "Visual Effects"),
    Integration UMETA(DisplayName = "Integration"),
    Regression  UMETA(DisplayName = "Regression")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_TestCase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Case")
    FString TestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Case")
    FString TestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Case")
    EQA_TestCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Case")
    EQA_TestResult Result;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Case")
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Case")
    float ExecutionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Case")
    bool bIsAutomated;

    FQA_TestCase()
    {
        TestName = TEXT("");
        TestDescription = TEXT("");
        Category = EQA_TestCategory::Core;
        Result = EQA_TestResult::NotRun;
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
        bIsAutomated = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_ValidationReport
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Report")
    TArray<FQA_TestCase> TestCases;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Report")
    int32 TotalTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Report")
    int32 PassedTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Report")
    int32 FailedTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Report")
    int32 WarningTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Report")
    float TotalExecutionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Report")
    FDateTime ReportTimestamp;

    FQA_ValidationReport()
    {
        TotalTests = 0;
        PassedTests = 0;
        FailedTests = 0;
        WarningTests = 0;
        TotalExecutionTime = 0.0f;
        ReportTimestamp = FDateTime::Now();
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_TestFramework : public UObject
{
    GENERATED_BODY()

public:
    UQA_TestFramework();

    // Core testing functions
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunTestsByCategory(EQA_TestCategory Category);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_ValidationReport GenerateValidationReport();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateGameSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidatePerformance();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateAudioSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateGameplayIntegration();

    // Test case management
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void AddTestCase(const FQA_TestCase& TestCase);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void ClearTestResults();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    TArray<FQA_TestCase> GetFailedTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    float GetTestSuccessRate();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Framework")
    TArray<FQA_TestCase> TestCases;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Framework")
    FQA_ValidationReport CurrentReport;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Framework")
    bool bAutoRunOnStart;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Framework")
    float PerformanceThresholdFPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Framework")
    float MemoryThresholdMB;

private:
    // Internal test execution
    EQA_TestResult ExecuteTestCase(FQA_TestCase& TestCase);
    void LogTestResult(const FQA_TestCase& TestCase);
    void InitializeDefaultTests();
    
    // System validation helpers
    bool ValidateActorSpawning();
    bool ValidateComponentSystems();
    bool ValidateAssetLoading();
    bool ValidateNetworkSystems();
    bool ValidateInputSystems();
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQA_TestActor : public AActor
{
    GENERATED_BODY()

public:
    AQA_TestActor();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunActorTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool TestMovement();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool TestCollision();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool TestComponents();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Testing")
    class UQA_TestFramework* TestFramework;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    bool bRunTestsOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    float TestInterval;

private:
    float LastTestTime;
};