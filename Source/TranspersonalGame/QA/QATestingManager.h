#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "QATestingManager.generated.h"

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    FString TestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    FString TestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    EQA_TestResult Result;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
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
struct FQA_SystemLimits
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Limits")
    int32 MaxTotalActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Limits")
    int32 MaxDinosaurs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Limits")
    int32 MaxEnvironmentProps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Limits")
    int32 MaxActorsPerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Limits")
    float MaxMemoryUsageMB;

    FQA_SystemLimits()
    {
        MaxTotalActors = 8000;
        MaxDinosaurs = 150;
        MaxEnvironmentProps = 5000;
        MaxActorsPerBiome = 4000;
        MaxMemoryUsageMB = 4000.0f;
    }
};

/**
 * QA Testing Manager - Validates game systems, performance, and compliance
 * Ensures all systems meet quality standards and performance requirements
 */
UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQATestingManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UQATestingManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // System Validation
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateSystemHealth();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateActorLimits();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateBiomeDistribution();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateGameplayReadiness();

    // Test Execution
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunTestSuite(const FString& SuiteName);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase RunSingleTest(const FString& TestName);

    // Reporting
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FString GenerateQAReport();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void SaveReportToFile(const FString& ReportContent);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    TArray<FQA_TestCase> GetTestResults() const { return TestResults; }

    // Monitoring
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void StartContinuousMonitoring();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void StopContinuousMonitoring();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool IsMonitoringActive() const { return bContinuousMonitoring; }

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Configuration")
    FQA_SystemLimits SystemLimits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Configuration")
    bool bAutoRunTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Configuration")
    float TestInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Configuration")
    bool bLogTestResults;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Configuration")
    bool bBreakOnFailure;

protected:
    // Internal test methods
    FQA_TestCase TestActorCount();
    FQA_TestCase TestDinosaurPopulation();
    FQA_TestCase TestEnvironmentProps();
    FQA_TestCase TestBiomeBalance();
    FQA_TestCase TestMemoryUsage();
    FQA_TestCase TestGameplayElements();
    FQA_TestCase TestVFXSystems();
    FQA_TestCase TestAudioSystems();
    FQA_TestCase TestNavigationMesh();
    FQA_TestCase TestLightingSetup();

    // Utility methods
    int32 CountActorsOfType(const FString& ActorType);
    float EstimateMemoryUsage();
    bool CheckBiomeDistribution();
    void LogTestResult(const FQA_TestCase& TestCase);

private:
    UPROPERTY()
    TArray<FQA_TestCase> TestResults;

    UPROPERTY()
    bool bContinuousMonitoring;

    UPROPERTY()
    float LastTestTime;

    UPROPERTY()
    int32 TestRunCount;

    UPROPERTY()
    UWorld* CachedWorld;
};