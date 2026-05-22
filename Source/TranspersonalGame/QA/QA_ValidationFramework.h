#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "QA_ValidationFramework.generated.h"

UENUM(BlueprintType)
enum class EQA_ValidationResult : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    Fail        UMETA(DisplayName = "Fail"),
    Warning     UMETA(DisplayName = "Warning"),
    Pending     UMETA(DisplayName = "Pending")
};

USTRUCT(BlueprintType)
struct FQA_TestResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FString TestName;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    EQA_ValidationResult Result;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FString Message;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    float ExecutionTime;

    FQA_TestResult()
    {
        TestName = TEXT("");
        Result = EQA_ValidationResult::Pending;
        Message = TEXT("");
        ExecutionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FQA_SystemStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    bool bIsOperational;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    int32 ActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    TArray<FString> Issues;

    FQA_SystemStatus()
    {
        SystemName = TEXT("");
        bIsOperational = false;
        ActorCount = 0;
    }
};

/**
 * QA Validation Framework - Automated testing and validation system
 * Provides comprehensive testing for all game systems and integration points
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_ValidationFramework : public UActorComponent
{
    GENERATED_BODY()

public:
    UQA_ValidationFramework();

    // Core validation functions
    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    void RunFullValidationSuite();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    FQA_TestResult ValidateCharacterSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    FQA_TestResult ValidatePhysicsSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    FQA_TestResult ValidateVFXSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    FQA_TestResult ValidateDinosaurAssets();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    FQA_TestResult ValidateBiomePopulation();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    FQA_TestResult ValidatePerformanceMetrics();

    // System status checks
    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    TArray<FQA_SystemStatus> GetSystemStatusReport();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool IsSystemOperational(const FString& SystemName);

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    float GetCurrentMemoryUsage();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    int32 GetTotalActorCount();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    TMap<FString, int32> GetActorCountByType();

    // Integration testing
    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool TestCrossSystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    void GenerateValidationReport();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(BlueprintReadOnly, Category = "QA Validation")
    TArray<FQA_TestResult> TestResults;

    UPROPERTY(BlueprintReadOnly, Category = "QA Validation")
    TArray<FQA_SystemStatus> SystemStatuses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    bool bAutoRunValidationOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    float ValidationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    bool bLogDetailedResults;

private:
    FTimerHandle ValidationTimerHandle;

    // Helper functions
    FQA_TestResult CreateTestResult(const FString& TestName, EQA_ValidationResult Result, const FString& Message, float ExecutionTime = 0.0f);
    void LogTestResult(const FQA_TestResult& Result);
    bool ValidateActorClass(const FString& ClassName);
    int32 CountActorsOfType(const FString& ActorType);
    void RunPeriodicValidation();
};