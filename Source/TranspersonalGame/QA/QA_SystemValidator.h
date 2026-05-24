#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "QA_SystemValidator.generated.h"

UENUM(BlueprintType)
enum class EQA_ValidationResult : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    Warning     UMETA(DisplayName = "Warning"),
    Fail        UMETA(DisplayName = "Fail"),
    Critical    UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct FQA_ValidationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FString TestName;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    EQA_ValidationResult Result;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FString Details;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    float ExecutionTime;

    FQA_ValidationReport()
    {
        TestName = TEXT("");
        Result = EQA_ValidationResult::Pass;
        Details = TEXT("");
        ExecutionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FQA_SystemHealthMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    int32 TotalActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    int32 LoadedClassCount;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    int32 FailedClassCount;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    float OverallHealthScore;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    TArray<FQA_ValidationReport> ValidationReports;

    FQA_SystemHealthMetrics()
    {
        TotalActorCount = 0;
        LoadedClassCount = 0;
        FailedClassCount = 0;
        OverallHealthScore = 0.0f;
    }
};

/**
 * QA System Validator - Comprehensive testing and validation system
 * Validates all game systems, classes, and integration points
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_SystemValidator : public UActorComponent
{
    GENERATED_BODY()

public:
    UQA_SystemValidator();

protected:
    virtual void BeginPlay() override;

public:
    // Core validation functions
    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    FQA_SystemHealthMetrics RunComprehensiveValidation();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    FQA_ValidationReport ValidateCharacterSystem();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    FQA_ValidationReport ValidateWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    FQA_ValidationReport ValidateVFXSystem();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    FQA_ValidationReport ValidateAISystem();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    FQA_ValidationReport ValidateAudioSystem();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    FQA_ValidationReport ValidatePerformanceMetrics();

    // Class loading validation
    UFUNCTION(BlueprintCallable, Category = "QA")
    bool ValidateClassLoading(const FString& ClassName);

    // Actor validation
    UFUNCTION(BlueprintCallable, Category = "QA")
    TArray<AActor*> GetActorsOfType(const FString& ActorType);

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "QA")
    float GetCurrentFrameRate();

    UFUNCTION(BlueprintCallable, Category = "QA")
    int32 GetMemoryUsageMB();

protected:
    // Internal validation helpers
    FQA_ValidationReport CreateValidationReport(const FString& TestName, EQA_ValidationResult Result, const FString& Details);
    
    bool IsClassLoadable(const FString& ClassPath);
    
    void LogValidationResult(const FQA_ValidationReport& Report);

private:
    // Core system class names to validate
    UPROPERTY(EditAnywhere, Category = "QA", meta = (AllowPrivateAccess = "true"))
    TArray<FString> CoreSystemClasses;

    // Performance thresholds
    UPROPERTY(EditAnywhere, Category = "QA", meta = (AllowPrivateAccess = "true"))
    int32 MaxActorCountThreshold;

    UPROPERTY(EditAnywhere, Category = "QA", meta = (AllowPrivateAccess = "true"))
    float MinFrameRateThreshold;

    UPROPERTY(EditAnywhere, Category = "QA", meta = (AllowPrivateAccess = "true"))
    int32 MaxMemoryUsageMB;

    // Validation state
    UPROPERTY(BlueprintReadOnly, Category = "QA", meta = (AllowPrivateAccess = "true"))
    FQA_SystemHealthMetrics LastValidationResults;

    UPROPERTY(BlueprintReadOnly, Category = "QA", meta = (AllowPrivateAccess = "true"))
    bool bValidationInProgress;
};