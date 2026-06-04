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
        Result = EQA_TestResult::Skipped;
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FQA_TestSuite
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test Suite")
    FString SuiteName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test Suite")
    TArray<FQA_TestCase> TestCases;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test Suite")
    int32 PassCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test Suite")
    int32 FailCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test Suite")
    int32 WarningCount;

    FQA_TestSuite()
    {
        SuiteName = TEXT("");
        PassCount = 0;
        FailCount = 0;
        WarningCount = 0;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQATestFramework : public UActorComponent
{
    GENERATED_BODY()

public:
    UQATestFramework();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Test execution functions
    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void RunTestSuite(const FString& SuiteName);

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    FQA_TestCase RunSingleTest(const FString& TestName);

    // VFX System Tests
    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    FQA_TestCase TestVFXSystemIntegrity();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    FQA_TestCase TestNiagaraSystemsLoading();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    FQA_TestCase TestAudioSyncedVFX();

    // Combat System Tests
    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    FQA_TestCase TestCombatVFXManager();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    FQA_TestCase TestScreenShakeSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    FQA_TestCase TestDamageVFXTriggers();

    // Audio System Tests
    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    FQA_TestCase TestAudioManagerIntegration();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    FQA_TestCase TestMetaSoundsPlayback();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    FQA_TestCase TestSpatialAudioSystem();

    // Performance Tests
    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    FQA_TestCase TestFrameRateStability();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    FQA_TestCase TestMemoryUsage();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    FQA_TestCase TestLODSystemPerformance();

    // Integration Tests
    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    FQA_TestCase TestVFXAudioIntegration();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    FQA_TestCase TestCombatSystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    FQA_TestCase TestWorldGenerationIntegration();

    // Reporting functions
    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void GenerateTestReport();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void ExportTestResults(const FString& FilePath);

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void LogTestResults();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Framework")
    TArray<FQA_TestSuite> TestSuites;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Framework")
    bool bAutoRunTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Framework")
    float TestInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Framework")
    bool bLogVerbose;

private:
    void InitializeTestSuites();
    FQA_TestCase CreateTestCase(const FString& Name, const FString& Description);
    void UpdateTestSuiteStats(FQA_TestSuite& Suite);
    float CurrentTestStartTime;
};