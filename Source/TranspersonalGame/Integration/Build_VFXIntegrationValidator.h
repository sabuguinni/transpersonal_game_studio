#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Build_VFXIntegrationValidator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_VFXValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "VFX Validation")
    bool bNiagaraSystemsValid = false;

    UPROPERTY(BlueprintReadOnly, Category = "VFX Validation")
    bool bParticleSystemsValid = false;

    UPROPERTY(BlueprintReadOnly, Category = "VFX Validation")
    bool bCampfireEffectsValid = false;

    UPROPERTY(BlueprintReadOnly, Category = "VFX Validation")
    bool bPerformanceWithinBudget = false;

    UPROPERTY(BlueprintReadOnly, Category = "VFX Validation")
    int32 TotalVFXActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "VFX Validation")
    float FrameTimeImpact = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "VFX Validation")
    FString ValidationReport;

    FBuild_VFXValidationResult()
    {
        bNiagaraSystemsValid = false;
        bParticleSystemsValid = false;
        bCampfireEffectsValid = false;
        bPerformanceWithinBudget = false;
        TotalVFXActors = 0;
        FrameTimeImpact = 0.0f;
        ValidationReport = TEXT("No validation performed");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_VFXIntegrationValidator : public UActorComponent
{
    GENERATED_BODY()

public:
    UBuild_VFXIntegrationValidator();

    // Core VFX validation functions
    UFUNCTION(BlueprintCallable, Category = "VFX Integration")
    FBuild_VFXValidationResult ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "VFX Integration")
    bool ValidateNiagaraSystems();

    UFUNCTION(BlueprintCallable, Category = "VFX Integration")
    bool ValidateParticleSystems();

    UFUNCTION(BlueprintCallable, Category = "VFX Integration")
    bool ValidateCampfireEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX Integration")
    float MeasureVFXPerformanceImpact();

    UFUNCTION(BlueprintCallable, Category = "VFX Integration")
    void GenerateVFXIntegrationReport();

    // QA integration functions
    UFUNCTION(BlueprintCallable, Category = "VFX Integration")
    bool ValidateQATestActors();

    UFUNCTION(BlueprintCallable, Category = "VFX Integration")
    void ProcessQAValidationResults();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(BlueprintReadOnly, Category = "VFX Integration")
    FBuild_VFXValidationResult LastValidationResult;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Integration")
    float MaxAllowedFrameTimeImpact = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Integration")
    bool bEnableDetailedLogging = true;

private:
    void LogValidationStep(const FString& StepName, bool bSuccess);
    TArray<AActor*> FindVFXActors();
    TArray<AActor*> FindQATestActors();
};