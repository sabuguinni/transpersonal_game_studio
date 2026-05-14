#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "QA_VFXSystemValidator.generated.h"

/**
 * QA Validator for VFX System Integration
 * Validates VFX managers, particle systems, and impact effects
 * Ensures proper integration with character and world systems
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQA_VFXSystemValidator : public AActor
{
    GENERATED_BODY()

public:
    AQA_VFXSystemValidator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    /** Root component for the validator */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Validation")
    class USceneComponent* RootSceneComponent;

    /** Validation status flags */
    UPROPERTY(BlueprintReadOnly, Category = "QA Results")
    bool bVFXManagerValidated;

    UPROPERTY(BlueprintReadOnly, Category = "QA Results")
    bool bImpactEffectsValidated;

    UPROPERTY(BlueprintReadOnly, Category = "QA Results")
    bool bParticleSystemsValidated;

    UPROPERTY(BlueprintReadOnly, Category = "QA Results")
    bool bCharacterIntegrationValidated;

    /** Validation test functions */
    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateVFXManagers();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateImpactEffects();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateParticleSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateCharacterIntegration();

    /** Run complete VFX system validation suite */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "QA Validation")
    void RunCompleteVFXValidation();

    /** Get validation summary report */
    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    FString GetValidationReport();

protected:
    /** Internal validation helpers */
    bool CheckVFXManagerClasses();
    bool CheckImpactEffectComponents();
    bool CheckParticleSystemAssets();
    bool CheckCharacterVFXIntegration();

    /** Validation timer */
    float ValidationTimer;
    bool bValidationComplete;

    /** Test results storage */
    TArray<FString> ValidationResults;
    TArray<FString> ValidationErrors;
};