#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "QA_VFXTribalDialogueValidator.generated.h"

/**
 * QA Validator for VFX Tribal Dialogue System
 * Validates integration between tribal NPCs and dialogue VFX effects
 * Tests Niagara particle systems for speech bubbles and interaction feedback
 */
UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQA_VFXTribalDialogueValidator : public UActorComponent
{
    GENERATED_BODY()

public:
    UQA_VFXTribalDialogueValidator();

    // Validation methods
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "QA Validation")
    bool ValidateTribalDialogueVFX();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "QA Validation")
    bool ValidateNiagaraDialogueEffects();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "QA Validation")
    bool ValidateSpeechBubbleSystem();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "QA Validation")
    bool ValidateInteractionFeedbackVFX();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "QA Validation")
    void RunComprehensiveVFXValidation();

    // Test creation methods
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "QA Testing")
    void CreateVFXTestScenario();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "QA Testing")
    void SpawnTribalDialogueTestActors();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "QA Testing")
    void CleanupVFXTestActors();

protected:
    // Validation properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    bool bEnableDetailedLogging = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    float ValidationTimeout = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    int32 MaxTestActors = 10;

    // Test tracking
    UPROPERTY(BlueprintReadOnly, Category = "QA Status")
    int32 PassedTests = 0;

    UPROPERTY(BlueprintReadOnly, Category = "QA Status")
    int32 FailedTests = 0;

    UPROPERTY(BlueprintReadOnly, Category = "QA Status")
    TArray<FString> ValidationErrors;

    UPROPERTY(BlueprintReadOnly, Category = "QA Status")
    TArray<AActor*> TestActors;

private:
    // Internal validation helpers
    bool ValidateNiagaraSystemExists(const FString& SystemPath);
    bool ValidateVFXActorSpawning();
    bool ValidateParticleEffectTriggers();
    void LogValidationResult(const FString& TestName, bool bPassed, const FString& Details = "");
    void ResetValidationCounters();
};