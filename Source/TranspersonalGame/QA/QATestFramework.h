#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
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
struct FQA_SystemStatus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA System")
    FString SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA System")
    bool bIsLoaded;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA System")
    bool bIsFunctional;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA System")
    int32 ActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA System")
    float PerformanceScore;

    FQA_SystemStatus()
    {
        SystemName = TEXT("");
        bIsLoaded = false;
        bIsFunctional = false;
        ActorCount = 0;
        PerformanceScore = 0.0f;
    }
};

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

    // Test execution functions
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunSystemTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunPerformanceTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunIntegrationTests();

    // Individual test functions
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase TestCharacterSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase TestWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase TestAudioSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase TestVFXSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase TestDinosaurAI();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase TestPerformanceMetrics();

    // System validation functions
    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    FQA_SystemStatus ValidateSystem(const FString& SystemName, const FString& ClassPath);

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    TArray<FQA_SystemStatus> GetAllSystemStatus();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    float CalculateIntegrationScore();

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    int32 GetTotalActorCount();

    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    float GetEstimatedMemoryUsage();

    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    bool IsPerformanceAcceptable();

    // Report generation
    UFUNCTION(BlueprintCallable, Category = "QA Reporting")
    void GenerateTestReport();

    UFUNCTION(BlueprintCallable, Category = "QA Reporting")
    FString GetTestSummary();

protected:
    // Test data storage
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Data")
    TArray<FQA_TestCase> TestResults;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Data")
    TArray<FQA_SystemStatus> SystemStatuses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    bool bAutoRunTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    float TestInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    int32 MaxActorThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    float MemoryThresholdMB;

private:
    // Internal test helpers
    bool ValidateClassLoading(const FString& ClassPath);
    int32 CountActorsByLabel(const FString& LabelFilter);
    float MeasureTestExecutionTime(TFunction<void()> TestFunction);
    void LogTestResult(const FQA_TestCase& TestCase);

    // Performance tracking
    float LastTestTime;
    int32 TestRunCount;
};