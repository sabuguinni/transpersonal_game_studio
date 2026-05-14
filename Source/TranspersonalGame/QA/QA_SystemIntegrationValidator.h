#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "QA_SystemIntegrationValidator.generated.h"

UENUM(BlueprintType)
enum class EQA_ValidationResult : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    Warning     UMETA(DisplayName = "Warning"),
    Fail        UMETA(DisplayName = "Fail"),
    NotTested   UMETA(DisplayName = "Not Tested")
};

USTRUCT(BlueprintType)
struct FQA_SystemTest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    FString TestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    EQA_ValidationResult Result;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    float ExecutionTimeMs;

    FQA_SystemTest()
    {
        TestName = TEXT("Unnamed Test");
        Result = EQA_ValidationResult::NotTested;
        ErrorMessage = TEXT("");
        ExecutionTimeMs = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FQA_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 ActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 ComponentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float FrameTimeMs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MemoryUsageMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 DrawCalls;

    FQA_PerformanceMetrics()
    {
        ActorCount = 0;
        ComponentCount = 0;
        FrameTimeMs = 0.0f;
        MemoryUsageMB = 0.0f;
        DrawCalls = 0;
    }
};

/**
 * QA System Integration Validator
 * Validates all game systems integration and performance
 */
UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQA_SystemIntegrationValidator : public UActorComponent
{
    GENERATED_BODY()

public:
    UQA_SystemIntegrationValidator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core validation functions
    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateVFXSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateCharacterSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateAudioSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidatePerformance();

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    FQA_PerformanceMetrics GetCurrentPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    bool IsPerformanceWithinLimits();

    // Test execution
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunFullTestSuite();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunQuickValidation();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    TArray<FQA_SystemTest> GetTestResults();

    // Reporting
    UFUNCTION(BlueprintCallable, Category = "QA Reporting")
    FString GenerateQAReport();

    UFUNCTION(BlueprintCallable, Category = "QA Reporting")
    void SaveQAReportToFile(const FString& FilePath);

protected:
    // Test execution helpers
    FQA_SystemTest ExecuteTest(const FString& TestName, TFunction<bool()> TestFunction);
    void LogTestResult(const FQA_SystemTest& Test);
    bool ValidateClassLoading(const FString& ClassName, const FString& ModulePath);

    // Performance helpers
    void UpdatePerformanceMetrics();
    bool CheckFrameRate();
    bool CheckMemoryUsage();
    bool CheckActorCount();

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA State", meta = (AllowPrivateAccess = "true"))
    TArray<FQA_SystemTest> TestResults;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA State", meta = (AllowPrivateAccess = "true"))
    FQA_PerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config", meta = (AllowPrivateAccess = "true"))
    bool bAutoRunTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config", meta = (AllowPrivateAccess = "true"))
    float TestInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config", meta = (AllowPrivateAccess = "true"))
    bool bLogVerbose;

    // Performance thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Thresholds", meta = (AllowPrivateAccess = "true"))
    float MaxFrameTimeMs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Thresholds", meta = (AllowPrivateAccess = "true"))
    float MaxMemoryUsageMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Thresholds", meta = (AllowPrivateAccess = "true"))
    int32 MaxActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Thresholds", meta = (AllowPrivateAccess = "true"))
    int32 MaxDrawCalls;

    float LastTestTime;
    bool bTestsCompleted;
};