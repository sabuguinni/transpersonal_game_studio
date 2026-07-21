#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Build_VFXTribalIntegrationValidator.generated.h"

UENUM(BlueprintType)
enum class EBuild_VFXTribalValidationResult : uint8
{
    NotTested,
    ValidationPassed,
    ValidationFailed,
    CriticalError,
    IntegrationSuccess
};

USTRUCT(BlueprintType)
struct FBuild_VFXTribalTestResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "VFX Tribal Validation")
    FString TestName;

    UPROPERTY(BlueprintReadOnly, Category = "VFX Tribal Validation")
    EBuild_VFXTribalValidationResult Result;

    UPROPERTY(BlueprintReadOnly, Category = "VFX Tribal Validation")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "VFX Tribal Validation")
    float ExecutionTime;

    UPROPERTY(BlueprintReadOnly, Category = "VFX Tribal Validation")
    int32 ActorsAffected;

    FBuild_VFXTribalTestResult()
    {
        TestName = TEXT("Unknown Test");
        Result = EBuild_VFXTribalValidationResult::NotTested;
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
        ActorsAffected = 0;
    }
};

USTRUCT(BlueprintType)
struct FBuild_VFXTribalIntegrationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration Report")
    TArray<FBuild_VFXTribalTestResult> TestResults;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Report")
    int32 TotalTests;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Report")
    int32 PassedTests;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Report")
    int32 FailedTests;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Report")
    float TotalExecutionTime;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Report")
    bool bIntegrationSuccessful;

    FBuild_VFXTribalIntegrationReport()
    {
        TotalTests = 0;
        PassedTests = 0;
        FailedTests = 0;
        TotalExecutionTime = 0.0f;
        bIntegrationSuccessful = false;
    }
};

/**
 * Integration validator for VFX and Tribal Dialogue systems
 * Validates cross-system compatibility between QA Agent #18 outputs and build integration
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABuild_VFXTribalIntegrationValidator : public AActor
{
    GENERATED_BODY()

public:
    ABuild_VFXTribalIntegrationValidator();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* ValidatorMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UParticleSystemComponent* VFXTestComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* TribalAudioComponent;

    // Integration validation properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Tribal Integration")
    bool bEnableVFXValidation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Tribal Integration")
    bool bEnableTribalDialogueValidation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Tribal Integration")
    bool bEnableCrossSystemValidation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Tribal Integration")
    float ValidationTimeout;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Results")
    FBuild_VFXTribalIntegrationReport IntegrationReport;

public:
    // Core validation functions
    UFUNCTION(BlueprintCallable, Category = "VFX Tribal Integration")
    void RunFullIntegrationValidation();

    UFUNCTION(BlueprintCallable, Category = "VFX Tribal Integration")
    bool ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "VFX Tribal Integration")
    bool ValidateTribalDialogueSystems();

    UFUNCTION(BlueprintCallable, Category = "VFX Tribal Integration")
    bool ValidateCrossSystemCompatibility();

    UFUNCTION(BlueprintCallable, Category = "VFX Tribal Integration")
    void ValidateQAAgentOutputs();

    UFUNCTION(BlueprintCallable, Category = "VFX Tribal Integration")
    void ValidateActorLimits();

    UFUNCTION(BlueprintCallable, Category = "VFX Tribal Integration")
    void ValidatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "VFX Tribal Integration")
    FBuild_VFXTribalIntegrationReport GetIntegrationReport() const;

    UFUNCTION(BlueprintCallable, Category = "VFX Tribal Integration")
    void ResetValidationState();

private:
    // Internal validation helpers
    FBuild_VFXTribalTestResult ValidateVFXFootstepEffects();
    FBuild_VFXTribalTestResult ValidateTribalDialogueAudio();
    FBuild_VFXTribalTestResult ValidateVFXTribalCombination();
    FBuild_VFXTribalTestResult ValidateQAValidatorIntegration();
    FBuild_VFXTribalTestResult ValidateSystemPerformance();

    void AddTestResult(const FBuild_VFXTribalTestResult& Result);
    void LogValidationResult(const FString& TestName, bool bSuccess, const FString& Details);
};