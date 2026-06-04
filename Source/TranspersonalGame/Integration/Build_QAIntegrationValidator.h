#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Build_QAIntegrationValidator.generated.h"

UENUM(BlueprintType)
enum class EBuild_QAValidationStatus : uint8
{
    Pending UMETA(DisplayName = "Pending"),
    InProgress UMETA(DisplayName = "In Progress"),
    Passed UMETA(DisplayName = "Passed"),
    Failed UMETA(DisplayName = "Failed"),
    Critical UMETA(DisplayName = "Critical Error")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_QAIntegrationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA Integration")
    EBuild_QAValidationStatus ValidationStatus = EBuild_QAValidationStatus::Pending;

    UPROPERTY(BlueprintReadOnly, Category = "QA Integration")
    int32 TotalActorsValidated = 0;

    UPROPERTY(BlueprintReadOnly, Category = "QA Integration")
    int32 QAActorsFound = 0;

    UPROPERTY(BlueprintReadOnly, Category = "QA Integration")
    int32 VFXActorsFound = 0;

    UPROPERTY(BlueprintReadOnly, Category = "QA Integration")
    bool bPerformanceLimitsOK = true;

    UPROPERTY(BlueprintReadOnly, Category = "QA Integration")
    bool bModuleIntegrationOK = true;

    UPROPERTY(BlueprintReadOnly, Category = "QA Integration")
    bool bVFXIntegrationOK = true;

    UPROPERTY(BlueprintReadOnly, Category = "QA Integration")
    FString ValidationReport = TEXT("No validation performed yet");

    FBuild_QAIntegrationResult()
    {
        ValidationStatus = EBuild_QAValidationStatus::Pending;
        TotalActorsValidated = 0;
        QAActorsFound = 0;
        VFXActorsFound = 0;
        bPerformanceLimitsOK = true;
        bModuleIntegrationOK = true;
        bVFXIntegrationOK = true;
        ValidationReport = TEXT("Initialized");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABuild_QAIntegrationValidator : public AActor
{
    GENERATED_BODY()

public:
    ABuild_QAIntegrationValidator();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* ValidatorMesh;

    UPROPERTY(BlueprintReadOnly, Category = "QA Integration")
    FBuild_QAIntegrationResult CurrentValidationResult;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Integration")
    int32 MaxActorLimit = 200;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Integration")
    bool bAutoValidateOnBeginPlay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Integration")
    float ValidationInterval = 30.0f;

public:
    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    FBuild_QAIntegrationResult ValidateQAIntegration();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    bool ValidatePerformanceLimits();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    bool ValidateModuleIntegration();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    bool ValidateVFXIntegration();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    void GenerateValidationReport();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "QA Integration")
    void RunFullValidation();

    UFUNCTION(BlueprintPure, Category = "QA Integration")
    FBuild_QAIntegrationResult GetValidationResult() const { return CurrentValidationResult; }

    UFUNCTION(BlueprintPure, Category = "QA Integration")
    bool IsValidationPassing() const;

private:
    FTimerHandle ValidationTimerHandle;
    
    void PerformPeriodicValidation();
    void UpdateValidationStatus();
    int32 CountActorsByType(const FString& TypeFilter);
};