#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "UObject/NoExportTypes.h"
#include "QA_VFXCampfireValidator.generated.h"

UENUM(BlueprintType)
enum class EQA_VFXCampfireTestResult : uint8
{
    NotTested = 0,
    Pass = 1,
    Fail = 2,
    Warning = 3
};

USTRUCT(BlueprintType)
struct FQA_VFXCampfireTestData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "VFX Validation")
    FString TestName;

    UPROPERTY(BlueprintReadOnly, Category = "VFX Validation")
    EQA_VFXCampfireTestResult Result;

    UPROPERTY(BlueprintReadOnly, Category = "VFX Validation")
    FString Details;

    UPROPERTY(BlueprintReadOnly, Category = "VFX Validation")
    float ExecutionTime;

    FQA_VFXCampfireTestData()
    {
        TestName = TEXT("");
        Result = EQA_VFXCampfireTestResult::NotTested;
        Details = TEXT("");
        ExecutionTime = 0.0f;
    }
};

/**
 * QA Validator for VFX Campfire Effects
 * Validates prehistoric campfire particle systems, fire effects, and smoke simulation
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_VFXCampfireValidator : public UObject
{
    GENERATED_BODY()

public:
    UQA_VFXCampfireValidator();

    // Core validation methods
    UFUNCTION(BlueprintCallable, Category = "QA VFX Validation")
    bool ValidateCampfireEffects();

    UFUNCTION(BlueprintCallable, Category = "QA VFX Validation")
    bool ValidateFireParticleSystem();

    UFUNCTION(BlueprintCallable, Category = "QA VFX Validation")
    bool ValidateSmokeEffects();

    UFUNCTION(BlueprintCallable, Category = "QA VFX Validation")
    bool ValidateEmberEffects();

    UFUNCTION(BlueprintCallable, Category = "QA VFX Validation")
    bool ValidatePerformanceMetrics();

    // Test result access
    UFUNCTION(BlueprintCallable, Category = "QA VFX Validation")
    TArray<FQA_VFXCampfireTestData> GetTestResults() const { return TestResults; }

    UFUNCTION(BlueprintCallable, Category = "QA VFX Validation")
    bool HasPassedAllTests() const;

    UFUNCTION(BlueprintCallable, Category = "QA VFX Validation")
    FString GenerateValidationReport() const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "QA VFX Validation")
    TArray<FQA_VFXCampfireTestData> TestResults;

    UPROPERTY(BlueprintReadOnly, Category = "QA VFX Validation")
    int32 PassedTests;

    UPROPERTY(BlueprintReadOnly, Category = "QA VFX Validation")
    int32 FailedTests;

    UPROPERTY(BlueprintReadOnly, Category = "QA VFX Validation")
    int32 WarningTests;

private:
    void AddTestResult(const FString& TestName, EQA_VFXCampfireTestResult Result, const FString& Details, float ExecutionTime);
    bool ValidateNiagaraSystem(const FString& SystemPath);
    bool ValidateParticleCount(int32 ExpectedMin, int32 ExpectedMax);
    bool ValidateEffectPerformance(float MaxFrameTime);
};