#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "QA_TestFramework.generated.h"

UENUM(BlueprintType)
enum class EQA_TestStatus : uint8
{
    NotRun      UMETA(DisplayName = "Not Run"),
    Running     UMETA(DisplayName = "Running"),
    Pass        UMETA(DisplayName = "Pass"),
    Fail        UMETA(DisplayName = "Fail"),
    Warning     UMETA(DisplayName = "Warning")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_TestResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA Test")
    FString TestName;

    UPROPERTY(BlueprintReadOnly, Category = "QA Test")
    EQA_TestStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "QA Test")
    FString Description;

    UPROPERTY(BlueprintReadOnly, Category = "QA Test")
    TArray<FString> Details;

    UPROPERTY(BlueprintReadOnly, Category = "QA Test")
    float ExecutionTime;

    FQA_TestResult()
    {
        TestName = TEXT("");
        Status = EQA_TestStatus::NotRun;
        Description = TEXT("");
        ExecutionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_TestSession
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA Session")
    FString SessionId;

    UPROPERTY(BlueprintReadOnly, Category = "QA Session")
    FDateTime Timestamp;

    UPROPERTY(BlueprintReadOnly, Category = "QA Session")
    TArray<FQA_TestResult> TestResults;

    UPROPERTY(BlueprintReadOnly, Category = "QA Session")
    int32 PassedTests;

    UPROPERTY(BlueprintReadOnly, Category = "QA Session")
    int32 FailedTests;

    UPROPERTY(BlueprintReadOnly, Category = "QA Session")
    int32 WarningTests;

    UPROPERTY(BlueprintReadOnly, Category = "QA Session")
    float OverallScore;

    FQA_TestSession()
    {
        SessionId = TEXT("");
        Timestamp = FDateTime::Now();
        PassedTests = 0;
        FailedTests = 0;
        WarningTests = 0;
        OverallScore = 0.0f;
    }
};

/**
 * QA Test Framework - Automated testing and validation system
 * Provides comprehensive testing capabilities for all game systems
 */
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
    void RunTestByName(const FString& TestName);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestResult RunSystemConnectivityTest();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestResult RunClassLoadingTest();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestResult RunActorSpawningTest();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestResult RunPerformanceTest();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestResult RunIntegrationTest();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestResult RunAssetValidationTest();

    // Results and reporting
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestSession GetCurrentSession() const { return CurrentSession; }

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void GenerateReport();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void SaveResultsToFile(const FString& FilePath);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    float CalculateOverallScore() const;

    // Validation helpers
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateClass(const FString& ClassName);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    TArray<FString> GetSystemErrors();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "QA Testing")
    FQA_TestSession CurrentSession;

    UPROPERTY(BlueprintReadOnly, Category = "QA Testing")
    TArray<FString> RegisteredTestNames;

    UPROPERTY(BlueprintReadOnly, Category = "QA Testing")
    bool bTestingInProgress;

    // Internal test execution
    void InitializeTestSession();
    void FinalizeTestSession();
    FQA_TestResult CreateTestResult(const FString& TestName, EQA_TestStatus Status, const FString& Description);
    void AddTestDetail(FQA_TestResult& TestResult, const FString& Detail);
    void LogTestResult(const FQA_TestResult& TestResult);

    // Specific test implementations
    bool TestWorldState();
    bool TestGameModeSetup();
    bool TestCharacterFunctionality();
    bool TestVFXSystems();
    bool TestAudioSystems();
    bool TestMemoryUsage();
    bool TestFrameRate();
};

/**
 * QA Test Manager Actor - Manages QA testing in the world
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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Testing")
    class UQA_TestFramework* TestFramework;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    bool bAutoRunTestsOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    float TestInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    bool bContinuousMonitoring;

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void StartTesting();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void StopTesting();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunSingleTest();

private:
    float LastTestTime;
    bool bTestingActive;
};