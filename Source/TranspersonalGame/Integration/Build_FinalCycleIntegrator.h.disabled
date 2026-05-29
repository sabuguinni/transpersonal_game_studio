#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "Build_FinalCycleIntegrator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_CycleIntegrationMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalActorsInLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 VFXActorsCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 AudioActorsCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 DinosaurActorsCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 QuestActorsCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float PerformanceScore;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bAllSystemsOperational;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString LastValidationTimestamp;

    FBuild_CycleIntegrationMetrics()
    {
        TotalActorsInLevel = 0;
        VFXActorsCount = 0;
        AudioActorsCount = 0;
        DinosaurActorsCount = 0;
        QuestActorsCount = 0;
        PerformanceScore = 0.0f;
        bAllSystemsOperational = false;
        LastValidationTimestamp = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemHealthReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    bool bCoreSystemsHealthy;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    bool bWorldGenerationHealthy;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    bool bVFXSystemsHealthy;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    bool bAudioSystemsHealthy;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    bool bQuestSystemsHealthy;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    TArray<FString> CriticalIssues;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    TArray<FString> Warnings;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    float OverallHealthScore;

    FBuild_SystemHealthReport()
    {
        bCoreSystemsHealthy = false;
        bWorldGenerationHealthy = false;
        bVFXSystemsHealthy = false;
        bAudioSystemsHealthy = false;
        bQuestSystemsHealthy = false;
        OverallHealthScore = 0.0f;
    }
};

/**
 * Final cycle integrator - validates all systems after QA completion
 * Ensures all agent outputs are properly integrated and functional
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_FinalCycleIntegrator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuild_FinalCycleIntegrator();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void PerformFinalIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_CycleIntegrationMetrics GetIntegrationMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_SystemHealthReport GetSystemHealthReport() const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void GenerateFinalReport();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool CheckCriticalSystemsOperational();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateVFXIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateAudioIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateQuestIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void CleanupOrphanedSystems();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FBuild_CycleIntegrationMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FBuild_SystemHealthReport HealthReport;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bIntegrationComplete;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float LastIntegrationTime;

private:
    void ValidateCoreSystemsHealth();
    void ValidateWorldGenerationHealth();
    void CountActorsByType();
    void CalculatePerformanceScore();
    void UpdateHealthReport();
    void LogIntegrationResults();
};