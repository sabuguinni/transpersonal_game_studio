#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "../SharedTypes.h"
#include "Build_QAIntegrationOrchestrator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_QATestResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA Test")
    FString TestName;

    UPROPERTY(BlueprintReadOnly, Category = "QA Test")
    bool bPassed;

    UPROPERTY(BlueprintReadOnly, Category = "QA Test")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "QA Test")
    float ExecutionTime;

    UPROPERTY(BlueprintReadOnly, Category = "QA Test")
    int32 ActorsAffected;

    FBuild_QATestResult()
    {
        TestName = TEXT("");
        bPassed = false;
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
        ActorsAffected = 0;
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
    int32 VFXActors;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 AudioActors;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bBuildValid;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString BuildStatus;

    FBuild_IntegrationReport()
    {
        TotalActors = 0;
        DinosaurCount = 0;
        VFXActors = 0;
        AudioActors = 0;
        bBuildValid = false;
        BuildStatus = TEXT("Unknown");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_QAIntegrationOrchestrator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuild_QAIntegrationOrchestrator();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    FBuild_IntegrationReport GenerateIntegrationReport();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    bool ValidateQATestResults();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    bool ValidateActorLimits();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    bool ValidateDinosaurLimits();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    bool ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    bool ValidateAudioSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    bool ValidateBuildIntegrity();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    void CleanupExcessActors();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    TArray<FBuild_QATestResult> ParseQATestResults();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    bool ExecuteIntegrationValidation();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    void LogIntegrationStatus(const FString& Status);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "QA Integration")
    FBuild_IntegrationReport LastReport;

    UPROPERTY(BlueprintReadOnly, Category = "QA Integration")
    TArray<FBuild_QATestResult> CachedTestResults;

    UPROPERTY(BlueprintReadOnly, Category = "QA Integration")
    bool bIntegrationValid;

    UPROPERTY(BlueprintReadOnly, Category = "QA Integration")
    float LastValidationTime;

private:
    void ValidateSystemIntegration();
    void CheckCrossSystemCompatibility();
    void ValidatePerformanceMetrics();
    bool CheckModuleCompilation();
    void GenerateIntegrationMetrics();
};