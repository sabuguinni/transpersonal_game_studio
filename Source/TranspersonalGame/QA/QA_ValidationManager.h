#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "QA_ValidationManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_ValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    bool bPassed;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    float Score;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FString TestName;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    TArray<FString> Warnings;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    TArray<FString> Errors;

    FQA_ValidationResult()
    {
        bPassed = false;
        Score = 0.0f;
        TestName = TEXT("Unknown");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TotalActors;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 MeshActors;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 LightActors;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ParticleActors;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float ValidationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float OverallScore;

    FQA_PerformanceMetrics()
    {
        TotalActors = 0;
        MeshActors = 0;
        LightActors = 0;
        ParticleActors = 0;
        ValidationTime = 0.0f;
        OverallScore = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_ValidationManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UQA_ValidationManager();

protected:
    virtual void BeginPlay() override;

public:
    // Validation Methods
    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    FQA_ValidationResult ValidatePerformance();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    FQA_ValidationResult ValidateGameplay();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    FQA_ValidationResult ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    FQA_ValidationResult ValidateAudioSystems();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    FQA_ValidationResult RunFullValidation();

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_PerformanceMetrics GetPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "QA")
    bool IsPerformanceAcceptable();

    // Gameplay Validation
    UFUNCTION(BlueprintCallable, Category = "QA")
    bool HasEssentialGameplayElements();

    UFUNCTION(BlueprintCallable, Category = "QA")
    TArray<AActor*> GetMissingCriticalActors();

    // VFX Validation
    UFUNCTION(BlueprintCallable, Category = "QA")
    int32 CountVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "QA")
    bool AreVFXSystemsFunctional();

    // Reporting
    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    void GenerateQAReport();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FString GetLastValidationReport();

protected:
    // Internal validation helpers
    bool ValidateActorCount();
    bool ValidateLightingSetup();
    bool ValidateTerrainSetup();
    bool ValidatePlayerSetup();
    
    // Performance thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    int32 MaxActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    int32 MaxLightCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    int32 MaxParticleSystemCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    float MinPerformanceScore;

    // Validation results storage
    UPROPERTY(BlueprintReadOnly, Category = "QA")
    TArray<FQA_ValidationResult> ValidationHistory;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FString LastReportTimestamp;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    bool bLastValidationPassed;

private:
    // Internal state
    FQA_PerformanceMetrics CachedMetrics;
    bool bMetricsCached;
    float LastValidationTime;
};