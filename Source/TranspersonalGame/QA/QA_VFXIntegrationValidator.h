#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "QA_VFXIntegrationValidator.generated.h"

UENUM(BlueprintType)
enum class EQA_VFXValidationResult : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    Warning     UMETA(DisplayName = "Warning"),
    Fail        UMETA(DisplayName = "Fail"),
    Critical    UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct FQA_VFXTestResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FString TestName;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    EQA_VFXValidationResult Result;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FString Details;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    float ExecutionTime;

    FQA_VFXTestResult()
    {
        TestName = TEXT("");
        Result = EQA_VFXValidationResult::Pass;
        Details = TEXT("");
        ExecutionTime = 0.0f;
    }
};

/**
 * QA Validator for VFX systems integration and performance
 * Validates VFX Impact Manager, Niagara systems, and environmental effects
 */
UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQA_VFXIntegrationValidator : public UActorComponent
{
    GENERATED_BODY()

public:
    UQA_VFXIntegrationValidator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core validation functions
    UFUNCTION(BlueprintCallable, Category = "QA|VFX")
    FQA_VFXTestResult ValidateVFXImpactManager();

    UFUNCTION(BlueprintCallable, Category = "QA|VFX")
    FQA_VFXTestResult ValidateNiagaraSystems();

    UFUNCTION(BlueprintCallable, Category = "QA|VFX")
    FQA_VFXTestResult ValidateEnvironmentalEffects();

    UFUNCTION(BlueprintCallable, Category = "QA|VFX")
    FQA_VFXTestResult ValidateParticlePerformance();

    UFUNCTION(BlueprintCallable, Category = "QA|VFX")
    TArray<FQA_VFXTestResult> RunFullVFXValidationSuite();

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "QA|Performance")
    float GetVFXFrameTime();

    UFUNCTION(BlueprintCallable, Category = "QA|Performance")
    int32 GetActiveParticleCount();

    UFUNCTION(BlueprintCallable, Category = "QA|Performance")
    bool IsVFXPerformanceAcceptable();

    // Reporting
    UFUNCTION(BlueprintCallable, Category = "QA|Reporting")
    void GenerateVFXValidationReport();

    UFUNCTION(BlueprintCallable, Category = "QA|Reporting")
    void LogVFXSystemStatus();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "QA")
    TArray<FQA_VFXTestResult> LastValidationResults;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA|Settings")
    float MaxAcceptableFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA|Settings")
    int32 MaxAcceptableParticleCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA|Settings")
    bool bAutoRunValidation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA|Settings")
    float ValidationInterval;

private:
    float LastValidationTime;
    bool bValidationInProgress;

    // Helper functions
    bool ValidateClassExists(const FString& ClassName);
    bool ValidateAssetExists(const FString& AssetPath);
    float MeasureExecutionTime(TFunction<void()> TestFunction);
};