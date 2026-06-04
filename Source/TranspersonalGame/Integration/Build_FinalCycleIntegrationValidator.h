#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Build_FinalCycleIntegrationValidator.generated.h"

UENUM(BlueprintType)
enum class EBuild_IntegrationStatus : uint8
{
    Unknown,
    Validating,
    Pass,
    Fail,
    Warning
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_QAIntegrationResults
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA Integration")
    bool bVFXFrameworkLoaded = false;

    UPROPERTY(BlueprintReadOnly, Category = "QA Integration")
    bool bSystemValidatorActive = false;

    UPROPERTY(BlueprintReadOnly, Category = "QA Integration")
    bool bBuildValidationComplete = false;

    UPROPERTY(BlueprintReadOnly, Category = "QA Integration")
    int32 TotalTestsPassed = 0;

    UPROPERTY(BlueprintReadOnly, Category = "QA Integration")
    int32 TotalTestsFailed = 0;

    UPROPERTY(BlueprintReadOnly, Category = "QA Integration")
    FString LastValidationTimestamp;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ModuleStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Module Status")
    int32 CoreModulesLoaded = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Module Status")
    int32 IntegrationModulesLoaded = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Module Status")
    int32 QAModulesLoaded = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Module Status")
    int32 TotalActorsInLevel = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Module Status")
    EBuild_IntegrationStatus OverallStatus = EBuild_IntegrationStatus::Unknown;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_FinalCycleIntegrationValidator : public UActorComponent
{
    GENERATED_BODY()

public:
    UBuild_FinalCycleIntegrationValidator();

protected:
    virtual void BeginPlay() override;

public:
    UFUNCTION(BlueprintCallable, Category = "Integration Validation")
    bool ValidateQAIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration Validation")
    bool ValidateModuleStatus();

    UFUNCTION(BlueprintCallable, Category = "Integration Validation")
    bool ValidateVFXSystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration Validation")
    FBuild_QAIntegrationResults GetQAResults() const { return QAResults; }

    UFUNCTION(BlueprintCallable, Category = "Integration Validation")
    FBuild_ModuleStatus GetModuleStatus() const { return ModuleStatus; }

    UFUNCTION(BlueprintCallable, Category = "Integration Validation")
    EBuild_IntegrationStatus GetOverallIntegrationStatus() const;

    UFUNCTION(BlueprintCallable, Category = "Integration Validation")
    void RunFullValidationSuite();

    UFUNCTION(BlueprintCallable, Category = "Integration Validation")
    bool CheckActorCountLimits();

    UFUNCTION(BlueprintCallable, Category = "Integration Validation")
    void GenerateIntegrationReport();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Validation Results")
    FBuild_QAIntegrationResults QAResults;

    UPROPERTY(BlueprintReadOnly, Category = "Validation Results")
    FBuild_ModuleStatus ModuleStatus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Settings")
    int32 MaxActorCountLimit = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Settings")
    bool bAutoValidateOnBeginPlay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Settings")
    float ValidationInterval = 30.0f;

private:
    bool ValidateCoreModules();
    bool ValidateIntegrationModules();
    bool ValidateQAModules();
    void UpdateValidationTimestamp();

    FTimerHandle ValidationTimerHandle;
};