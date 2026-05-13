#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Build_FinalIntegrationOrchestrator.generated.h"

UENUM(BlueprintType)
enum class EBuild_IntegrationPhase : uint8
{
    PreIntegration      UMETA(DisplayName = "Pre-Integration"),
    CoreSystemsCheck    UMETA(DisplayName = "Core Systems Check"),
    VFXIntegration      UMETA(DisplayName = "VFX Integration"),
    QAValidation        UMETA(DisplayName = "QA Validation"),
    FinalOrchestration  UMETA(DisplayName = "Final Orchestration"),
    BuildComplete       UMETA(DisplayName = "Build Complete"),
    IntegrationFailed   UMETA(DisplayName = "Integration Failed")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemIntegrationMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration Metrics")
    int32 TotalActorsInScene;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Metrics")
    int32 CoreSystemsLoaded;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Metrics")
    int32 VFXSystemsIntegrated;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Metrics")
    int32 QAValidationsPassed;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Metrics")
    float IntegrationCompletionPercentage;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Metrics")
    bool bBuildReadyForDeployment;

    FBuild_SystemIntegrationMetrics()
    {
        TotalActorsInScene = 0;
        CoreSystemsLoaded = 0;
        VFXSystemsIntegrated = 0;
        QAValidationsPassed = 0;
        IntegrationCompletionPercentage = 0.0f;
        bBuildReadyForDeployment = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_CrossSystemValidation
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Cross-System Validation")
    bool bCoreGameSystemsValid;

    UPROPERTY(BlueprintReadOnly, Category = "Cross-System Validation")
    bool bVFXSystemsOperational;

    UPROPERTY(BlueprintReadOnly, Category = "Cross-System Validation")
    bool bQAFrameworkActive;

    UPROPERTY(BlueprintReadOnly, Category = "Cross-System Validation")
    bool bPerformanceWithinLimits;

    UPROPERTY(BlueprintReadOnly, Category = "Cross-System Validation")
    bool bMemoryUsageAcceptable;

    UPROPERTY(BlueprintReadOnly, Category = "Cross-System Validation")
    TArray<FString> ValidationErrors;

    FBuild_CrossSystemValidation()
    {
        bCoreGameSystemsValid = false;
        bVFXSystemsOperational = false;
        bQAFrameworkActive = false;
        bPerformanceWithinLimits = false;
        bMemoryUsageAcceptable = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_FinalIntegrationComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UBuild_FinalIntegrationComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Status")
    EBuild_IntegrationPhase CurrentIntegrationPhase;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Metrics")
    FBuild_SystemIntegrationMetrics IntegrationMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Cross-System Validation")
    FBuild_CrossSystemValidation CrossSystemValidation;

    UPROPERTY(BlueprintReadWrite, Category = "Integration Settings")
    float IntegrationUpdateInterval;

    UPROPERTY(BlueprintReadWrite, Category = "Integration Settings")
    bool bAutoProgressPhases;

    UPROPERTY(BlueprintReadWrite, Category = "Integration Settings")
    bool bDetailedLogging;

    UFUNCTION(BlueprintCallable, Category = "Integration Control")
    void StartFinalIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration Control")
    void AdvanceIntegrationPhase();

    UFUNCTION(BlueprintCallable, Category = "Integration Validation")
    bool ValidateCoreGameSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration Validation")
    bool ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration Validation")
    bool ValidateQAFramework();

    UFUNCTION(BlueprintCallable, Category = "Integration Validation")
    bool ValidatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Integration Validation")
    void RunCrossSystemValidation();

    UFUNCTION(BlueprintCallable, Category = "Integration Metrics")
    void UpdateIntegrationMetrics();

    UFUNCTION(BlueprintCallable, Category = "Integration Control")
    void CompleteFinalIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration Control")
    void HandleIntegrationFailure(const FString& FailureReason);

private:
    float LastIntegrationUpdate;
    int32 IntegrationStepCounter;
    TArray<FString> IntegrationLog;

    void LogIntegrationStep(const FString& StepDescription);
    void ValidateSystemDependencies();
    void CheckMemoryAndPerformance();
    void FinalizeIntegrationProcess();
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABuild_FinalIntegrationOrchestrator : public AActor
{
    GENERATED_BODY()

public:
    ABuild_FinalIntegrationOrchestrator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBuild_FinalIntegrationComponent* IntegrationComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Orchestration Status")
    bool bOrchestrationActive;

    UPROPERTY(BlueprintReadOnly, Category = "Orchestration Status")
    float OrchestrationProgress;

    UPROPERTY(BlueprintReadWrite, Category = "Orchestration Settings")
    bool bAutoStartOrchestration;

    UFUNCTION(BlueprintCallable, Category = "Orchestration Control")
    void StartFinalOrchestration();

    UFUNCTION(BlueprintCallable, Category = "Orchestration Control")
    void StopOrchestration();

    UFUNCTION(BlueprintCallable, Category = "Orchestration Status")
    bool IsIntegrationComplete() const;

    UFUNCTION(BlueprintCallable, Category = "Orchestration Status")
    FString GetOrchestrationStatusReport() const;
};