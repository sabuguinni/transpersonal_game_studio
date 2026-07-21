#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/ActorComponent.h"
#include "QA_VFXParticleValidator.generated.h"

UENUM(BlueprintType)
enum class EQA_VFXValidationResult : uint8
{
    Pass,
    Warning,
    Critical,
    SystemFailure
};

USTRUCT(BlueprintType)
struct FQA_ParticleValidationData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString ParticleSystemName;

    UPROPERTY(BlueprintReadOnly)
    EQA_VFXValidationResult ValidationResult;

    UPROPERTY(BlueprintReadOnly)
    FString ValidationMessage;

    UPROPERTY(BlueprintReadOnly)
    float PerformanceImpact;

    UPROPERTY(BlueprintReadOnly)
    int32 ParticleCount;

    FQA_ParticleValidationData()
    {
        ParticleSystemName = TEXT("");
        ValidationResult = EQA_VFXValidationResult::Pass;
        ValidationMessage = TEXT("");
        PerformanceImpact = 0.0f;
        ParticleCount = 0;
    }
};

/**
 * QA Validator for VFX Particle Systems
 * Validates Niagara systems, performance impact, and visual quality
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_VFXParticleValidator : public UActorComponent
{
    GENERATED_BODY()

public:
    UQA_VFXParticleValidator();

    UFUNCTION(BlueprintCallable, Category = "QA|VFX")
    bool ValidateAllParticleSystems();

    UFUNCTION(BlueprintCallable, Category = "QA|VFX")
    FQA_ParticleValidationData ValidateNiagaraSystem(UNiagaraSystem* NiagaraSystem);

    UFUNCTION(BlueprintCallable, Category = "QA|VFX")
    bool ValidateFootstepEffects();

    UFUNCTION(BlueprintCallable, Category = "QA|VFX")
    bool ValidateCombatEffects();

    UFUNCTION(BlueprintCallable, Category = "QA|VFX")
    bool ValidateWeatherEffects();

    UFUNCTION(BlueprintCallable, Category = "QA|VFX")
    TArray<FQA_ParticleValidationData> GetValidationResults() const;

    UFUNCTION(BlueprintCallable, Category = "QA|VFX")
    void GenerateVFXValidationReport();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "QA|VFX")
    TArray<FQA_ParticleValidationData> ValidationResults;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA|VFX")
    float MaxAllowedPerformanceImpact;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA|VFX")
    int32 MaxParticleCountPerSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA|VFX")
    bool bEnableDetailedLogging;

private:
    bool ValidateParticlePerformance(UNiagaraSystem* System, FQA_ParticleValidationData& OutData);
    bool ValidateParticleVisuals(UNiagaraSystem* System, FQA_ParticleValidationData& OutData);
    void LogValidationResult(const FQA_ParticleValidationData& Data);
};