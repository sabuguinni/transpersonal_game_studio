#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "QA_VFXIntegrationReport.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_VFXTestResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FString TestName;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    bool bPassed;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    float ExecutionTime;

    FQA_VFXTestResult()
    {
        TestName = TEXT("");
        bPassed = false;
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_VFXSystemStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    bool bVFXManagerLoaded;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    bool bNiagaraSystemsValid;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    bool bImpactEffectsWorking;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    int32 TotalVFXActors;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    TArray<FQA_VFXTestResult> TestResults;

    FQA_VFXSystemStatus()
    {
        bVFXManagerLoaded = false;
        bNiagaraSystemsValid = false;
        bImpactEffectsWorking = false;
        TotalVFXActors = 0;
    }
};

/**
 * QA VFX Integration Report - Validates VFX systems from Agent #17
 * Tests VFX_ImpactManager integration and Niagara system functionality
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_VFXIntegrationReport : public UActorComponent
{
    GENERATED_BODY()

public:
    UQA_VFXIntegrationReport();

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FQA_VFXSystemStatus CurrentStatus;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FDateTime LastValidationTime;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    bool bCriticalVFXFailure;

    UFUNCTION(BlueprintCallable, Category = "QA")
    void RunVFXIntegrationTests();

    UFUNCTION(BlueprintCallable, Category = "QA")
    bool ValidateVFXManagerClass();

    UFUNCTION(BlueprintCallable, Category = "QA")
    bool TestNiagaraSystemLoading();

    UFUNCTION(BlueprintCallable, Category = "QA")
    bool ValidateImpactEffects();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FString GenerateVFXReport();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void LogCriticalVFXIssue(const FString& Issue);

protected:
    virtual void BeginPlay() override;

private:
    void AddTestResult(const FString& TestName, bool bPassed, const FString& ErrorMessage = TEXT(""), float ExecutionTime = 0.0f);
    void ResetTestResults();
};