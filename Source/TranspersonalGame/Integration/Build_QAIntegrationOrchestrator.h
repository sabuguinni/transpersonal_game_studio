#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Build_QAIntegrationOrchestrator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_QATestResult
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

    FBuild_QATestResult()
    {
        TestName = TEXT("");
        bPassed = false;
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_IntegrationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_QATestResult> TestResults;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalActors;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 DinosaurCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bActorLimitsValid;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bModuleDependenciesValid;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString BuildStatus;

    FBuild_IntegrationReport()
    {
        TotalActors = 0;
        DinosaurCount = 0;
        bActorLimitsValid = true;
        bModuleDependenciesValid = true;
        BuildStatus = TEXT("UNKNOWN");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_QAIntegrationOrchestrator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuild_QAIntegrationOrchestrator();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    FBuild_IntegrationReport RunIntegrationValidation();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    bool ValidateActorLimits();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    bool ValidateModuleDependencies();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    TArray<FBuild_QATestResult> ParseQATestResults();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    bool ValidateCrossSystemCompatibility();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    void GenerateIntegrationReport(const FBuild_IntegrationReport& Report);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "QA Integration")
    FBuild_IntegrationReport LastIntegrationReport;

    UPROPERTY(BlueprintReadOnly, Category = "QA Integration")
    bool bIntegrationValidationEnabled;

private:
    bool ValidateVFXSystems();
    bool ValidatePerformanceMetrics();
    bool ValidateAudioSystems();
    int32 CountDinosaursInLevel();
    bool CheckModuleLoadStatus();
};