#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "QA_VFXEffectManagerValidator.generated.h"

UENUM(BlueprintType)
enum class EQA_VFXValidationResult : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    Fail        UMETA(DisplayName = "Fail"),
    Warning     UMETA(DisplayName = "Warning"),
    Critical    UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct FQA_VFXValidationData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA Validation")
    FString TestName;

    UPROPERTY(BlueprintReadOnly, Category = "QA Validation")
    EQA_VFXValidationResult Result;

    UPROPERTY(BlueprintReadOnly, Category = "QA Validation")
    FString Details;

    UPROPERTY(BlueprintReadOnly, Category = "QA Validation")
    float ExecutionTime;

    FQA_VFXValidationData()
    {
        TestName = TEXT("");
        Result = EQA_VFXValidationResult::Pass;
        Details = TEXT("");
        ExecutionTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQA_VFXEffectManagerValidator : public UActorComponent
{
    GENERATED_BODY()

public:
    UQA_VFXEffectManagerValidator();

    UFUNCTION(BlueprintCallable, Category = "QA Validation", CallInEditor)
    void RunVFXValidationSuite();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateVFXEffectManager();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateNiagaraSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateFootstepEffects();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateCampfireEffects();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    TArray<FQA_VFXValidationData> GetValidationResults() const;

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    void GenerateValidationReport();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "QA Validation")
    TArray<FQA_VFXValidationData> ValidationResults;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    float MaxAllowedFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    int32 MaxParticleCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    bool bEnableDetailedLogging;

private:
    void AddValidationResult(const FString& TestName, EQA_VFXValidationResult Result, const FString& Details, float ExecutionTime);
    bool TestNiagaraSystemLoad(const FString& SystemPath);
    bool TestEffectSpawning(UNiagaraSystem* System, const FVector& Location);
    float MeasureFrameTime();
};