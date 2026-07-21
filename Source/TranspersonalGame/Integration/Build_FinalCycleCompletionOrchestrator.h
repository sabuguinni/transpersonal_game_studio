#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "../SharedTypes.h"
#include "Build_FinalCycleCompletionOrchestrator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_FinalCycleCompletionData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Final Cycle")
    int32 TotalActorsValidated;

    UPROPERTY(BlueprintReadOnly, Category = "Final Cycle")
    int32 DinosaurCount;

    UPROPERTY(BlueprintReadOnly, Category = "Final Cycle")
    int32 VFXSystemsActive;

    UPROPERTY(BlueprintReadOnly, Category = "Final Cycle")
    int32 AudioSystemsActive;

    UPROPERTY(BlueprintReadOnly, Category = "Final Cycle")
    int32 PerformanceSystemsActive;

    UPROPERTY(BlueprintReadOnly, Category = "Final Cycle")
    bool bQAValidationPassed;

    UPROPERTY(BlueprintReadOnly, Category = "Final Cycle")
    bool bBuildIntegrationComplete;

    UPROPERTY(BlueprintReadOnly, Category = "Final Cycle")
    bool bCrossSystemCompatibilityVerified;

    UPROPERTY(BlueprintReadOnly, Category = "Final Cycle")
    float CompletionPercentage;

    UPROPERTY(BlueprintReadOnly, Category = "Final Cycle")
    FString CompletionStatus;

    FBuild_FinalCycleCompletionData()
    {
        TotalActorsValidated = 0;
        DinosaurCount = 0;
        VFXSystemsActive = 0;
        AudioSystemsActive = 0;
        PerformanceSystemsActive = 0;
        bQAValidationPassed = false;
        bBuildIntegrationComplete = false;
        bCrossSystemCompatibilityVerified = false;
        CompletionPercentage = 0.0f;
        CompletionStatus = TEXT("Initializing");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemIntegrationMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration Metrics")
    int32 ModulesLoaded;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Metrics")
    int32 ClassesRegistered;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Metrics")
    int32 ComponentsActive;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Metrics")
    int32 SubsystemsInitialized;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Metrics")
    bool bCoreSystemsOnline;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Metrics")
    bool bGameplaySystemsOnline;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Metrics")
    bool bRenderingSystemsOnline;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Metrics")
    float SystemHealthScore;

    FBuild_SystemIntegrationMetrics()
    {
        ModulesLoaded = 0;
        ClassesRegistered = 0;
        ComponentsActive = 0;
        SubsystemsInitialized = 0;
        bCoreSystemsOnline = false;
        bGameplaySystemsOnline = false;
        bRenderingSystemsOnline = false;
        SystemHealthScore = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_FinalCycleCompletionOrchestrator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuild_FinalCycleCompletionOrchestrator();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Final Cycle Completion")
    void InitializeFinalCycleCompletion();

    UFUNCTION(BlueprintCallable, Category = "Final Cycle Completion")
    void ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Final Cycle Completion")
    void ValidateQAIntegration();

    UFUNCTION(BlueprintCallable, Category = "Final Cycle Completion")
    void ValidateBuildIntegration();

    UFUNCTION(BlueprintCallable, Category = "Final Cycle Completion")
    void ValidateCrossSystemCompatibility();

    UFUNCTION(BlueprintCallable, Category = "Final Cycle Completion")
    void GenerateFinalCompletionReport();

    UFUNCTION(BlueprintCallable, Category = "Final Cycle Completion")
    FBuild_FinalCycleCompletionData GetCompletionData() const { return CompletionData; }

    UFUNCTION(BlueprintCallable, Category = "Final Cycle Completion")
    FBuild_SystemIntegrationMetrics GetIntegrationMetrics() const { return IntegrationMetrics; }

    UFUNCTION(BlueprintCallable, Category = "Final Cycle Completion")
    bool IsFinalCycleComplete() const;

    UFUNCTION(BlueprintCallable, Category = "Final Cycle Completion")
    float GetOverallCompletionPercentage() const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Final Cycle Completion")
    FBuild_FinalCycleCompletionData CompletionData;

    UPROPERTY(BlueprintReadOnly, Category = "Final Cycle Completion")
    FBuild_SystemIntegrationMetrics IntegrationMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Final Cycle Completion")
    bool bInitialized;

    UPROPERTY(BlueprintReadOnly, Category = "Final Cycle Completion")
    bool bValidationInProgress;

    void ValidateActorLimits();
    void ValidateSystemHealth();
    void ValidateModuleIntegration();
    void CalculateCompletionMetrics();
    void UpdateCompletionStatus();
};