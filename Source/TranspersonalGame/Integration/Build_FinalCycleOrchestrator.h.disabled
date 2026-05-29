#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Build_FinalCycleOrchestrator.generated.h"

UENUM(BlueprintType)
enum class EBuild_CyclePhase : uint8
{
    PreValidation,
    SystemIntegration,
    QualityAssurance,
    FinalValidation,
    CycleComplete,
    CycleError
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_CycleMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Metrics")
    int32 TotalAgentsExecuted = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Metrics")
    int32 SuccessfulAgents = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Metrics")
    int32 FailedAgents = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Metrics")
    int32 TotalFilesCreated = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Metrics")
    int32 TotalUE5Commands = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Metrics")
    float CycleExecutionTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Metrics")
    bool bCompilationSuccessful = false;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Metrics")
    int32 OrphanedHeaders = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Metrics")
    int32 ActiveSystems = 0;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemHealth
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "System Health")
    bool bCoreSystemsOnline = false;

    UPROPERTY(BlueprintReadOnly, Category = "System Health")
    bool bWorldGenerationActive = false;

    UPROPERTY(BlueprintReadOnly, Category = "System Health")
    bool bCharacterSystemReady = false;

    UPROPERTY(BlueprintReadOnly, Category = "System Health")
    bool bCombatSystemFunctional = false;

    UPROPERTY(BlueprintReadOnly, Category = "System Health")
    bool bQuestSystemOperational = false;

    UPROPERTY(BlueprintReadOnly, Category = "System Health")
    bool bNPCSystemActive = false;

    UPROPERTY(BlueprintReadOnly, Category = "System Health")
    bool bAudioSystemReady = false;

    UPROPERTY(BlueprintReadOnly, Category = "System Health")
    bool bVFXSystemOnline = false;

    UPROPERTY(BlueprintReadOnly, Category = "System Health")
    bool bQAValidationPassed = false;

    UPROPERTY(BlueprintReadOnly, Category = "System Health")
    bool bIntegrationComplete = false;
};

/**
 * Final Cycle Orchestrator - Master integration system for cycle completion
 * Coordinates all agent outputs and validates final build state
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_FinalCycleOrchestrator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuild_FinalCycleOrchestrator();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core orchestration functions
    UFUNCTION(BlueprintCallable, Category = "Cycle Orchestration")
    void InitializeCycleValidation();

    UFUNCTION(BlueprintCallable, Category = "Cycle Orchestration")
    void ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Cycle Orchestration")
    void GenerateFinalReport();

    UFUNCTION(BlueprintCallable, Category = "Cycle Orchestration")
    void CompleteCycle();

    // System validation
    UFUNCTION(BlueprintCallable, Category = "System Validation")
    bool ValidateCoreSystemIntegrity();

    UFUNCTION(BlueprintCallable, Category = "System Validation")
    bool ValidateWorldSystemHealth();

    UFUNCTION(BlueprintCallable, Category = "System Validation")
    bool ValidateCharacterSystemStatus();

    UFUNCTION(BlueprintCallable, Category = "System Validation")
    bool ValidateCombatSystemReadiness();

    UFUNCTION(BlueprintCallable, Category = "System Validation")
    bool ValidateQuestSystemOperational();

    UFUNCTION(BlueprintCallable, Category = "System Validation")
    bool ValidateNPCSystemActive();

    UFUNCTION(BlueprintCallable, Category = "System Validation")
    bool ValidateAudioSystemReady();

    UFUNCTION(BlueprintCallable, Category = "System Validation")
    bool ValidateVFXSystemOnline();

    // Metrics and reporting
    UFUNCTION(BlueprintCallable, Category = "Metrics")
    FBuild_CycleMetrics GetCycleMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Metrics")
    FBuild_SystemHealth GetSystemHealth() const;

    UFUNCTION(BlueprintCallable, Category = "Metrics")
    EBuild_CyclePhase GetCurrentPhase() const;

    // File system validation
    UFUNCTION(BlueprintCallable, Category = "File Validation")
    int32 CountOrphanedHeaders();

    UFUNCTION(BlueprintCallable, Category = "File Validation")
    int32 CountImplementedSystems();

    UFUNCTION(BlueprintCallable, Category = "File Validation")
    bool ValidateCompilationStatus();

    // Emergency recovery
    UFUNCTION(BlueprintCallable, Category = "Emergency")
    void TriggerEmergencyRecovery();

    UFUNCTION(BlueprintCallable, Category = "Emergency")
    void CreateRecoveryCheckpoint();

    UFUNCTION(BlueprintCallable, Category = "Emergency")
    bool RestoreFromCheckpoint();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "State")
    EBuild_CyclePhase CurrentPhase;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    FBuild_CycleMetrics CycleMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    FBuild_SystemHealth SystemHealth;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bValidationInProgress;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bCycleCompleted;

    UPROPERTY(BlueprintReadOnly, Category = "Emergency")
    bool bEmergencyMode;

private:
    void UpdateSystemHealth();
    void CalculateMetrics();
    void LogValidationResults();
    void NotifyStudioDirector();
    
    FString GenerateStatusReport();
    void SaveCycleReport();
    void ArchiveCycleData();
};