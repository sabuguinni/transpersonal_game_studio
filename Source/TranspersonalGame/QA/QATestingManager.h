#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "QATestingManager.generated.h"

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
struct FQA_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 TotalActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 StaticMeshActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 SkeletalMeshActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 LightActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 DinosaurActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float FrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MemoryUsageMB;

    FQA_PerformanceMetrics()
    {
        TotalActors = 0;
        StaticMeshActors = 0;
        SkeletalMeshActors = 0;
        LightActors = 0;
        DinosaurActors = 0;
        FrameRate = 0.0f;
        MemoryUsageMB = 0.0f;
    }
};

/**
 * QA Testing Manager - Handles automated testing and validation
 * Ensures game quality and performance standards
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQATestingManager : public AActor
{
    GENERATED_BODY()

public:
    AQATestingManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Test execution functions
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunPerformanceTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunActorValidationTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunBiomeDistributionTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunDinosaurValidationTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunVFXValidationTests();

    // Validation functions
    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateActorCount();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateDinosaurCount();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateBiomeDistribution();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidatePerformanceMetrics();

    // Reporting functions
    UFUNCTION(BlueprintCallable, Category = "QA Reporting")
    void GenerateTestReport();

    UFUNCTION(BlueprintCallable, Category = "QA Reporting")
    void LogTestResults();

    UFUNCTION(BlueprintCallable, Category = "QA Reporting")
    FString GetTestSummary();

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FQA_PerformanceMetrics GetCurrentPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopPerformanceMonitoring();

protected:
    // Test data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    TArray<FQA_TestCase> TestCases;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FQA_PerformanceMetrics CurrentMetrics;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    int32 MaxActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    int32 MaxDinosaurCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    float MinFrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    float MaxMemoryUsageMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    bool bAutoRunTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    float TestInterval;

    // Internal state
    UPROPERTY()
    float LastTestTime;

    UPROPERTY()
    bool bIsMonitoring;

    UPROPERTY()
    int32 TestsRun;

    UPROPERTY()
    int32 TestsPassed;

    UPROPERTY()
    int32 TestsFailed;

private:
    // Helper functions
    void InitializeTestCases();
    void ExecuteTestCase(FQA_TestCase& TestCase);
    void UpdatePerformanceMetrics();
    void CheckActorLimits();
    void ValidateGameSystems();
    void LogPerformanceWarnings();
};