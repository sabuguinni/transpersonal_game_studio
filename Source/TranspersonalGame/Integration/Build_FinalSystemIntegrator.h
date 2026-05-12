#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Build_FinalSystemIntegrator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemIntegrationStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bCoreSystemsLoaded = false;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bWorldGenerationReady = false;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bCharacterSystemReady = false;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bAISystemsReady = false;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bVFXSystemsReady = false;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bAudioSystemsReady = false;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalActorsInLevel = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 CriticalActorsCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float IntegrationScore = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString LastValidationTimestamp;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> SystemErrors;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> SystemWarnings;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_CycleCompletionMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 CycleNumber = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 AgentsCompleted = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 FilesCreated = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 UE5CommandsExecuted = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float TotalExecutionTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    bool bBuildSuccessful = false;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    bool bAllSystemsIntegrated = false;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    FString CycleCompletionStatus;
};

/**
 * Final System Integrator - Orchestrates all game systems into a cohesive whole
 * This is the master integration class that ensures all agent outputs work together
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_FinalSystemIntegrator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuild_FinalSystemIntegrator();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core Integration Functions
    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool IntegrateCoreSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateCharacterSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateAISystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateAudioSystems();

    // Cycle Management
    UFUNCTION(BlueprintCallable, Category = "Cycle Management")
    void CompleteCycle(int32 CycleNumber);

    UFUNCTION(BlueprintCallable, Category = "Cycle Management")
    FBuild_CycleCompletionMetrics GetCycleMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Cycle Management")
    void GenerateIntegrationReport();

    // System Status
    UFUNCTION(BlueprintCallable, Category = "Status")
    FBuild_SystemIntegrationStatus GetSystemStatus() const;

    UFUNCTION(BlueprintCallable, Category = "Status")
    float CalculateIntegrationScore();

    UFUNCTION(BlueprintCallable, Category = "Status")
    bool IsGamePlayable() const;

    // Error Handling
    UFUNCTION(BlueprintCallable, Category = "Error Handling")
    void ReportSystemError(const FString& SystemName, const FString& ErrorMessage);

    UFUNCTION(BlueprintCallable, Category = "Error Handling")
    void ReportSystemWarning(const FString& SystemName, const FString& WarningMessage);

    UFUNCTION(BlueprintCallable, Category = "Error Handling")
    void ClearSystemErrors();

    // Level Validation
    UFUNCTION(BlueprintCallable, Category = "Level Validation")
    bool ValidateCurrentLevel();

    UFUNCTION(BlueprintCallable, Category = "Level Validation")
    int32 CountActorsByType(const FString& ActorTypeName);

    UFUNCTION(BlueprintCallable, Category = "Level Validation")
    bool HasCriticalActors();

protected:
    // Internal validation functions
    bool ValidateClassLoading();
    bool ValidateActorCounts();
    bool ValidateSystemDependencies();
    void UpdateIntegrationMetrics();

private:
    UPROPERTY()
    FBuild_SystemIntegrationStatus CurrentStatus;

    UPROPERTY()
    FBuild_CycleCompletionMetrics CycleMetrics;

    UPROPERTY()
    TArray<FString> LoadedSystemClasses;

    UPROPERTY()
    TMap<FString, int32> ActorTypeCounts;

    // Integration thresholds
    static constexpr float MIN_INTEGRATION_SCORE = 75.0f;
    static constexpr int32 MIN_CRITICAL_ACTORS = 5;
    static constexpr int32 MIN_TOTAL_ACTORS = 20;
};