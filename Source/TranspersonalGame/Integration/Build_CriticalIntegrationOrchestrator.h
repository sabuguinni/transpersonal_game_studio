#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Build_CriticalIntegrationOrchestrator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_IntegrationStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bModulesLoaded = false;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bCoreSystemsActive = false;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bBuildHealthy = false;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 LoadedActorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 ActiveSystemCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString LastValidationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> CriticalErrors;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> SystemWarnings;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ModuleValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bIsLoaded = false;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bHasErrors = false;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 ClassCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FString> LoadedClasses;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FString> ValidationErrors;
};

/**
 * Critical Integration Orchestrator - Agent #19 Final System
 * Orchestrates all integration validation and build health monitoring
 * Ensures all agent outputs are properly integrated and functional
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_CriticalIntegrationOrchestrator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuild_CriticalIntegrationOrchestrator();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core Integration Functions
    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    FBuild_IntegrationStatus ValidateFullSystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    bool ValidateAllModulesLoaded();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    TArray<FBuild_ModuleValidationResult> ValidateModuleHealth();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    bool ValidateCoreSystemsActive();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    bool ValidateBuildIntegrity();

    // Critical System Validation
    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    bool ValidateWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    bool ValidateCharacterSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    bool ValidateAISystems();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    bool ValidateVFXSystems();

    // Integration Reporting
    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    FString GenerateIntegrationReport();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    void LogCriticalIntegrationStatus();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    bool SaveIntegrationReport(const FString& ReportPath);

    // Emergency Recovery
    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    bool AttemptSystemRecovery();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    void TriggerEmergencyValidation();

protected:
    // Internal validation helpers
    bool ValidateModuleClasses(const FString& ModuleName, TArray<FString>& OutErrors);
    bool ValidateSystemDependencies();
    bool ValidateActorIntegrity();
    void CollectSystemMetrics();
    void UpdateIntegrationStatus();

private:
    UPROPERTY()
    FBuild_IntegrationStatus CurrentStatus;

    UPROPERTY()
    TArray<FBuild_ModuleValidationResult> ModuleResults;

    UPROPERTY()
    FDateTime LastValidationTime;

    UPROPERTY()
    bool bEmergencyMode = false;

    UPROPERTY()
    int32 ValidationCycleCount = 0;

    // Critical system references
    UPROPERTY()
    TArray<TWeakObjectPtr<UObject>> TrackedSystems;

    UPROPERTY()
    TMap<FString, bool> SystemHealthMap;
};