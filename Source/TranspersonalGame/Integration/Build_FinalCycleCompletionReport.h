#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Build_FinalCycleCompletionReport.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_FinalCycleMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 TotalActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 QATestActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 VFXTestActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 TotalAssets = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 MaterialAssets = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 BlueprintAssets = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 MeshAssets = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    bool bCoreClassesLoaded = false;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    bool bQAValidatorLoaded = false;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float CompletionPercentage = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    EBuildValidationStatus ValidationStatus = EBuildValidationStatus::Pending;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_AgentDeliverable
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Deliverable")
    FString AgentName;

    UPROPERTY(BlueprintReadOnly, Category = "Deliverable")
    int32 AgentNumber = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Deliverable")
    TArray<FString> FilesCreated;

    UPROPERTY(BlueprintReadOnly, Category = "Deliverable")
    TArray<FString> UE5CommandsExecuted;

    UPROPERTY(BlueprintReadOnly, Category = "Deliverable")
    bool bCompilationSuccess = false;

    UPROPERTY(BlueprintReadOnly, Category = "Deliverable")
    bool bFunctionalValidation = false;

    UPROPERTY(BlueprintReadOnly, Category = "Deliverable")
    FString CycleID;
};

/**
 * Final Cycle Completion Report - Comprehensive integration status for PROD_CYCLE_AUTO_20260604_005
 * Tracks all agent deliverables, compilation status, and final build validation
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_FinalCycleCompletionReport : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuild_FinalCycleCompletionReport();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Final Cycle")
    void GenerateFinalReport();

    UFUNCTION(BlueprintCallable, Category = "Final Cycle")
    FBuild_FinalCycleMetrics GetFinalMetrics() const { return FinalMetrics; }

    UFUNCTION(BlueprintCallable, Category = "Final Cycle")
    TArray<FBuild_AgentDeliverable> GetAgentDeliverables() const { return AgentDeliverables; }

    UFUNCTION(BlueprintCallable, Category = "Final Cycle")
    bool ValidateAllAgentDeliverables();

    UFUNCTION(BlueprintCallable, Category = "Final Cycle")
    void RecordAgentDeliverable(const FBuild_AgentDeliverable& Deliverable);

    UFUNCTION(BlueprintCallable, Category = "Final Cycle")
    float CalculateOverallCompletionPercentage() const;

    UFUNCTION(BlueprintCallable, Category = "Final Cycle")
    EBuildValidationStatus GetFinalValidationStatus() const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Final Cycle")
    void ExportFinalReportToJSON();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Final Cycle")
    FBuild_FinalCycleMetrics FinalMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Final Cycle")
    TArray<FBuild_AgentDeliverable> AgentDeliverables;

    UPROPERTY(BlueprintReadOnly, Category = "Final Cycle")
    FString CycleID;

    UPROPERTY(BlueprintReadOnly, Category = "Final Cycle")
    FDateTime CompletionTimestamp;

private:
    void CollectSystemMetrics();
    void ValidateAgentOutputs();
    void CalculateFinalScores();
};