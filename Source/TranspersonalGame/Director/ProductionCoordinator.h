#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "../SharedTypes.h"
#include "ProductionCoordinator.generated.h"

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Production")
    int32 AgentNumber;

    UPROPERTY(BlueprintReadWrite, Category = "Production")
    FString TaskDescription;

    UPROPERTY(BlueprintReadWrite, Category = "Production")
    EDir_Priority Priority;

    UPROPERTY(BlueprintReadWrite, Category = "Production")
    FString RequiredDeliverables;

    UPROPERTY(BlueprintReadWrite, Category = "Production")
    bool bIsCompleted;

    UPROPERTY(BlueprintReadWrite, Category = "Production")
    float EstimatedHours;

    FDir_AgentTask()
    {
        AgentNumber = 0;
        TaskDescription = TEXT("");
        Priority = EDir_Priority::Medium;
        RequiredDeliverables = TEXT("");
        bIsCompleted = false;
        EstimatedHours = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Metrics")
    int32 TotalFilesCreated;

    UPROPERTY(BlueprintReadWrite, Category = "Metrics")
    int32 TotalUE5Commands;

    UPROPERTY(BlueprintReadWrite, Category = "Metrics")
    int32 ActiveDinosaurs;

    UPROPERTY(BlueprintReadWrite, Category = "Metrics")
    float TerrainCoverage;

    UPROPERTY(BlueprintReadWrite, Category = "Metrics")
    bool bPlayablePrototype;

    FDir_ProductionMetrics()
    {
        TotalFilesCreated = 0;
        TotalUE5Commands = 0;
        ActiveDinosaurs = 0;
        TerrainCoverage = 0.0f;
        bPlayablePrototype = false;
    }
};

/**
 * Production Coordinator for Studio Director
 * Manages the 19-agent production pipeline and ensures milestone delivery
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDir_ProductionCoordinator : public UActorComponent
{
    GENERATED_BODY()

public:
    UDir_ProductionCoordinator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Production Management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void InitializeProductionPipeline();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AssignTaskToAgent(int32 AgentNumber, const FString& TaskDescription, EDir_Priority Priority);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void MarkTaskCompleted(int32 AgentNumber);

    UFUNCTION(BlueprintCallable, Category = "Production")
    FDir_ProductionMetrics GetProductionMetrics() const;

    // Milestone Tracking
    UFUNCTION(BlueprintCallable, Category = "Milestones")
    bool IsMilestone1Complete() const; // Walk Around prototype

    UFUNCTION(BlueprintCallable, Category = "Milestones")
    void ValidatePlayablePrototype();

    // Agent Coordination
    UFUNCTION(BlueprintCallable, Category = "Coordination")
    TArray<FDir_AgentTask> GetPendingTasks() const;

    UFUNCTION(BlueprintCallable, Category = "Coordination")
    void TriggerNextAgent();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugPrintProductionStatus();

protected:
    UPROPERTY(BlueprintReadWrite, Category = "Production")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(BlueprintReadWrite, Category = "Production")
    FDir_ProductionMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadWrite, Category = "Production")
    int32 CurrentCycleNumber;

    UPROPERTY(BlueprintReadWrite, Category = "Production")
    bool bPipelineActive;

private:
    void UpdateProductionMetrics();
    void CheckMilestoneProgress();
    void LogProductionStatus();
};