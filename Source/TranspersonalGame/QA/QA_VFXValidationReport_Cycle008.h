#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Engine.h"
#include "QA_VFXValidationReport_Cycle008.generated.h"

UENUM(BlueprintType)
enum class EQA_ValidationStatus : uint8
{
    NotStarted      UMETA(DisplayName = "Not Started"),
    InProgress      UMETA(DisplayName = "In Progress"),
    Passed          UMETA(DisplayName = "Passed"),
    PassedWithWarnings UMETA(DisplayName = "Passed with Warnings"),
    Failed          UMETA(DisplayName = "Failed")
};

USTRUCT(BlueprintType)
struct FQA_ValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString TestName;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString Category;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bPassed = false;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString Message;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FDateTime Timestamp;

    FQA_ValidationResult()
    {
        TestName = TEXT("");
        Category = TEXT("");
        bPassed = false;
        Message = TEXT("");
        Timestamp = FDateTime::Now();
    }
};

/**
 * QA VFX Validation Report for Cycle 008
 * Comprehensive validation of VFX systems and critical game components
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_VFXValidationReport_Cycle008 : public UObject
{
    GENERATED_BODY()

public:
    UQA_VFXValidationReport_Cycle008();

    virtual void BeginDestroy() override;

    // Validation Control
    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    void InitializeValidation();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    void FinalizeValidation();

    // VFX System Validation
    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateVFXNiagaraLibrary();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateVFXEffectManager();

    // Critical System Validation
    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateCriticalSystems();

    // Performance Validation
    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidatePerformanceMetrics();

    // Report Generation
    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    FString GetValidationStatusString() const;

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    FString GenerateDetailedReport() const;

    // Validation Status
    UPROPERTY(BlueprintReadOnly, Category = "Validation Status")
    EQA_ValidationStatus ValidationStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Validation Status")
    FDateTime ReportTimestamp;

    UPROPERTY(BlueprintReadOnly, Category = "Validation Metrics")
    int32 CriticalIssuesFound;

    UPROPERTY(BlueprintReadOnly, Category = "Validation Metrics")
    int32 WarningsFound;

    UPROPERTY(BlueprintReadOnly, Category = "Validation Metrics")
    int32 TestsPassed;

    UPROPERTY(BlueprintReadOnly, Category = "Validation Metrics")
    int32 TestsFailed;

    UPROPERTY(BlueprintReadOnly, Category = "Validation Results")
    TArray<FQA_ValidationResult> ValidationResults;
};