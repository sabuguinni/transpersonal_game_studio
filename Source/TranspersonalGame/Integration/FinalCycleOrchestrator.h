#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "FinalCycleOrchestrator.generated.h"

/**
 * Final Cycle Orchestrator - Integration Agent #19
 * Manages production cycle completion, system validation, and build orchestration
 * Ensures all agent outputs are properly integrated and validated
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UFinalCycleOrchestrator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UFinalCycleOrchestrator();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core orchestration functions
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void GenerateFinalCycleReport();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateBuildIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void CheckModuleDependencies();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateGameplayElements();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void PerformSystemHealthCheck();

    // System validation properties
    UPROPERTY(BlueprintReadOnly, Category = "Integration Status")
    bool bAllSystemsValidated;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Status")
    bool bBuildIntegrityConfirmed;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Status")
    bool bModuleDependenciesResolved;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Status")
    bool bGameplayElementsValidated;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Status")
    int32 TotalActorsInLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Status")
    int32 ValidatedSystemsCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Status")
    FString LastValidationTimestamp;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Status")
    TArray<FString> ValidationErrors;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Status")
    TArray<FString> ValidationWarnings;

    // Critical system references
    UPROPERTY(BlueprintReadOnly, Category = "System References")
    TArray<AActor*> CriticalGameplayActors;

    UPROPERTY(BlueprintReadOnly, Category = "System References")
    TArray<UObject*> LoadedModules;

    UPROPERTY(BlueprintReadOnly, Category = "System References")
    FString CurrentCycleID;

    UPROPERTY(BlueprintReadOnly, Category = "System References")
    int32 AgentExecutionCount;

private:
    // Internal validation methods
    void ValidateWorldState();
    void ValidateActorIntegrity();
    void ValidateComponentSystems();
    void ValidateGameModeConfiguration();
    void ValidatePlayerCharacterSystems();
    void ValidateEnvironmentalSystems();
    void ValidateAISystems();
    void ValidateVFXSystems();
    void ValidateAudioSystems();
    void ValidateQASystems();

    // Build validation methods
    void CheckCompilationStatus();
    void ValidateModuleLoading();
    void CheckDependencyChain();
    void ValidateAssetIntegrity();

    // Reporting methods
    void LogValidationResults();
    void GenerateSystemReport();
    void CreateIntegrationSummary();

    // Internal state
    bool bInitialized;
    FDateTime LastValidationTime;
    TMap<FString, bool> SystemValidationResults;
    TArray<FString> CriticalErrors;
    TArray<FString> SystemWarnings;
};