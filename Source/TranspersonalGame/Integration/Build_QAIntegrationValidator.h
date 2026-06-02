#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Build_QAIntegrationValidator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_QAValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA Validation")
    bool bValidationPassed = false;

    UPROPERTY(BlueprintReadOnly, Category = "QA Validation")
    FString ValidationMessage;

    UPROPERTY(BlueprintReadOnly, Category = "QA Validation")
    int32 ErrorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "QA Validation")
    int32 WarningCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "QA Validation")
    TArray<FString> CriticalErrors;

    UPROPERTY(BlueprintReadOnly, Category = "QA Validation")
    TArray<FString> Warnings;

    UPROPERTY(BlueprintReadOnly, Category = "QA Validation")
    float ValidationTimestamp = 0.0f;

    FBuild_QAValidationResult()
    {
        bValidationPassed = false;
        ValidationMessage = TEXT("Not validated");
        ErrorCount = 0;
        WarningCount = 0;
        ValidationTimestamp = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_QAIntegrationMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA Metrics")
    int32 TotalActorsInMap = 0;

    UPROPERTY(BlueprintReadOnly, Category = "QA Metrics")
    int32 DinosaurCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "QA Metrics")
    int32 VFXActorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "QA Metrics")
    int32 AudioActorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "QA Metrics")
    float MemoryUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "QA Metrics")
    float FrameRate = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "QA Metrics")
    bool bPerformanceWithinLimits = true;

    UPROPERTY(BlueprintReadOnly, Category = "QA Metrics")
    TArray<FString> PerformanceWarnings;

    FBuild_QAIntegrationMetrics()
    {
        TotalActorsInMap = 0;
        DinosaurCount = 0;
        VFXActorCount = 0;
        AudioActorCount = 0;
        MemoryUsageMB = 0.0f;
        FrameRate = 0.0f;
        bPerformanceWithinLimits = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_QAIntegrationValidator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuild_QAIntegrationValidator();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // QA Integration validation functions
    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    FBuild_QAValidationResult ValidateQAResults();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    FBuild_QAIntegrationMetrics CollectIntegrationMetrics();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    bool ValidateActorLimits();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    bool ValidateDinosaurLimits();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    bool ValidatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    bool ValidateModuleIntegration();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    FString GenerateIntegrationReport();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    void LogValidationResults(const FBuild_QAValidationResult& Results);

    // Critical validation checks
    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    bool CheckCriticalSystemsOnline();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    bool ValidateMapIntegrity();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    bool ValidateAssetReferences();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "QA Integration")
    FBuild_QAValidationResult LastValidationResult;

    UPROPERTY(BlueprintReadOnly, Category = "QA Integration")
    FBuild_QAIntegrationMetrics LastMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "QA Integration")
    bool bValidationInProgress = false;

    UPROPERTY(BlueprintReadOnly, Category = "QA Integration")
    float LastValidationTime = 0.0f;

private:
    // Internal validation helpers
    bool ValidateActorCounts();
    bool ValidateSystemReferences();
    bool ValidatePerformanceLimits();
    void CollectPerformanceData();
    void LogCriticalError(const FString& ErrorMessage);
    void LogWarning(const FString& WarningMessage);
};