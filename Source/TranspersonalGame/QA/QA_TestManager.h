#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "QA_TestManager.generated.h"

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

    FQA_TestSuite()
    {
        SuiteName = TEXT("");
        PassCount = 0;
        FailCount = 0;
        WarningCount = 0;
    }
};

/**
 * QA Test Manager - Automated testing system for Transpersonal Game
 * Validates game systems, actors, and integration points
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

    // Test execution
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunTestSuite(const FString& SuiteName);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunSingleTest(const FString& TestName);

    // Test validation functions
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateMapActors();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateLightingSetup();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidatePlayerCharacter();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateDinosaurActors();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateAudioSystems();

    // Performance tests
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateFrameRate();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateMemoryUsage();

    // Integration tests
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateCharacterMovement();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateWorldGeneration();

    // Reporting
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void GenerateTestReport();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FString GetTestSummary();

protected:
    // Test suites
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Configuration")
    TArray<FQA_TestSuite> TestSuites;

    // Test settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Configuration")
    bool bAutoRunOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Configuration")
    bool bGenerateReportOnComplete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Configuration")
    float TestTimeout;

    // Performance thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MinFrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxMemoryUsageMB;

    // Test state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Test State")
    bool bTestsRunning;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Test State")
    int32 TotalTestsRun;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Test State")
    int32 TotalTestsPassed;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Test State")
    int32 TotalTestsFailed;

private:
    // Helper functions
    void InitializeTestSuites();
    void AddTestCase(const FString& SuiteName, const FString& TestName, const FString& Description);
    void UpdateTestResult(const FString& TestName, EQA_TestResult Result, const FString& ErrorMessage = TEXT(""));
    FQA_TestCase* FindTestCase(const FString& TestName);
    void LogTestResult(const FQA_TestCase& TestCase);
};