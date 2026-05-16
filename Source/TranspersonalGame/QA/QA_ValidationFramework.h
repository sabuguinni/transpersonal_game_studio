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
    Skipped     UMETA(DisplayName = "Skipped")
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
        Result = EQA_ValidationResult::Skipped;
        Message = TEXT("");
        ExecutionTime = 0.0f;
    }

    FQA_TestResult(const FString& InTestName, EQA_ValidationResult InResult, const FString& InMessage, float InExecutionTime)
        : TestName(InTestName), Result(InResult), Message(InMessage), ExecutionTime(InExecutionTime)
    {
    }
};

USTRUCT(BlueprintType)
struct FQA_SystemHealth
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    int32 TotalActors;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    int32 CharacterActors;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    int32 DinosaurActors;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    int32 LightingActors;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    int32 EnvironmentActors;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    float MemoryUsagePercent;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    float CPUUsagePercent;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    float OverallHealthScore;

    FQA_SystemHealth()
    {
        TotalActors = 0;
        CharacterActors = 0;
        DinosaurActors = 0;
        LightingActors = 0;
        EnvironmentActors = 0;
        MemoryUsagePercent = 0.0f;
        CPUUsagePercent = 0.0f;
        OverallHealthScore = 0.0f;
    }
};

/**
 * QA Validation Framework for comprehensive system testing
 * Provides automated testing capabilities for all game systems
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_ValidationFramework : public UObject
{
    GENERATED_BODY()

public:
    UQA_ValidationFramework();

    // Core validation functions
    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    TArray<FQA_TestResult> RunAllValidationTests();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    FQA_TestResult ValidateCharacterSystems();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    FQA_TestResult ValidateDinosaurAI();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    FQA_TestResult ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    FQA_TestResult ValidatePhysicsSystems();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    FQA_TestResult ValidateLightingSystems();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    FQA_TestResult ValidateAssetPipeline();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    FQA_TestResult ValidateBiomePopulation();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    FQA_SystemHealth GetSystemHealthReport();

    // Performance testing
    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    FQA_TestResult RunPerformanceTest();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    FQA_TestResult ValidateMemoryUsage();

    // Integration testing
    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    FQA_TestResult TestCrossSystemIntegration();

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "QA")
    void LogTestResult(const FQA_TestResult& TestResult);

    UFUNCTION(BlueprintCallable, Category = "QA")
    void GenerateQAReport();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "QA")
    TArray<FQA_TestResult> TestResults;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FQA_SystemHealth CurrentSystemHealth;

    // Helper functions
    FQA_TestResult CreateTestResult(const FString& TestName, EQA_ValidationResult Result, const FString& Message);
    bool IsClassLoaded(const FString& ClassName);
    int32 CountActorsOfType(const FString& ActorType);
    float MeasureExecutionTime(TFunction<void()> TestFunction);
};