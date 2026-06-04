#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Build_VFXIntegrationValidator.generated.h"

UENUM(BlueprintType)
enum class EBuild_VFXValidationResult : uint8
{
    Passed      UMETA(DisplayName = "Validation Passed"),
    Failed      UMETA(DisplayName = "Validation Failed"),
    Warning     UMETA(DisplayName = "Validation Warning"),
    NotTested   UMETA(DisplayName = "Not Tested")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_VFXValidationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "VFX Validation")
    int32 TotalVFXActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "VFX Validation")
    int32 ValidVFXActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "VFX Validation")
    int32 InvalidVFXActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "VFX Validation")
    TArray<FString> ValidationErrors;

    UPROPERTY(BlueprintReadOnly, Category = "VFX Validation")
    EBuild_VFXValidationResult OverallResult = EBuild_VFXValidationResult::NotTested;

    UPROPERTY(BlueprintReadOnly, Category = "VFX Validation")
    float ValidationTimestamp = 0.0f;
};

/**
 * Build VFX Integration Validator
 * Validates VFX systems integration with build pipeline
 * Ensures Niagara effects work correctly across all modules
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABuild_VFXIntegrationValidator : public AActor
{
    GENERATED_BODY()

public:
    ABuild_VFXIntegrationValidator();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* ValidatorMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UNiagaraComponent* TestVFXComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Validation")
    bool bAutoValidateOnBeginPlay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Validation")
    float ValidationInterval = 5.0f;

    UPROPERTY(BlueprintReadOnly, Category = "VFX Validation")
    FBuild_VFXValidationReport LastValidationReport;

public:
    UFUNCTION(BlueprintCallable, Category = "VFX Validation")
    FBuild_VFXValidationReport ValidateVFXIntegration();

    UFUNCTION(BlueprintCallable, Category = "VFX Validation")
    bool ValidateNiagaraSystem(UNiagaraSystem* NiagaraSystem);

    UFUNCTION(BlueprintCallable, Category = "VFX Validation")
    TArray<AActor*> FindAllVFXActors();

    UFUNCTION(BlueprintCallable, Category = "VFX Validation")
    bool TestVFXPerformance();

    UFUNCTION(BlueprintCallable, Category = "VFX Validation", CallInEditor = true)
    void RunVFXValidationTest();

private:
    FTimerHandle ValidationTimerHandle;
    
    void PerformPeriodicValidation();
    bool ValidateVFXActor(AActor* Actor);
    void LogValidationResult(const FBuild_VFXValidationReport& Report);
};