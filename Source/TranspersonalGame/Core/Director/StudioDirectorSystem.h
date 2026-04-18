#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Subsystems/WorldSubsystem.h"
#include "../../SharedTypes.h"
#include "StudioDirectorSystem.generated.h"

/**
 * Studio Director System - Coordinates all game systems and agent outputs
 * Manages production pipeline, agent coordination, and milestone tracking
 */

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    EDir_Priority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    EDir_TaskStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    TArray<FString> Blockers;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Priority = EDir_Priority::Medium;
        Status = EDir_TaskStatus::NotStarted;
        CompletionPercentage = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_MilestoneProgress
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FDir_AgentTask> RequiredTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float OverallProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bIsBlocked;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FString> BlockingIssues;

    FDir_MilestoneProgress()
    {
        MilestoneName = TEXT("");
        OverallProgress = 0.0f;
        bIsBlocked = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalCycles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CompletedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 ActiveAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CriticalBlockers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float PlayabilityScore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float TechnicalDebtLevel;

    FDir_ProductionMetrics()
    {
        TotalCycles = 0;
        CompletedTasks = 0;
        ActiveAgents = 0;
        CriticalBlockers = 0;
        PlayabilityScore = 0.0f;
        TechnicalDebtLevel = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UStudioDirectorSystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core Director Functions
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeProductionPipeline();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void StartNewProductionCycle(int32 CycleNumber);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AssignTaskToAgent(const FString& AgentName, const FDir_AgentTask& Task);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateTaskProgress(const FString& AgentName, const FString& TaskDescription, float Progress);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ReportTaskCompletion(const FString& AgentName, const FString& TaskDescription);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ReportBlocker(const FString& AgentName, const FString& BlockerDescription);

    // Milestone Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CreateMilestone(const FString& MilestoneName, const TArray<FDir_AgentTask>& RequiredTasks);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_MilestoneProgress GetMilestoneProgress(const FString& MilestoneName);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool IsMilestoneComplete(const FString& MilestoneName);

    // Production Metrics
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_ProductionMetrics GetProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    float CalculatePlayabilityScore();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FString> GetCriticalBlockers();

    // Agent Coordination
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentTask> GetTasksForAgent(const FString& AgentName);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FString> GetActiveAgents();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void BroadcastToAllAgents(const FString& Message);

    // Quality Assurance
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool ValidateSystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void RunProductionHealthCheck();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FString> GetSystemWarnings();

    // Debug and Development
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Studio Director")
    void DebugPrintAllTasks();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Studio Director")
    void DebugPrintMilestoneStatus();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Studio Director")
    void DebugResetAllTasks();

protected:
    // Core Data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Studio Director")
    TMap<FString, TArray<FDir_AgentTask>> AgentTasks;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Studio Director")
    TMap<FString, FDir_MilestoneProgress> Milestones;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Studio Director")
    FDir_ProductionMetrics CurrentMetrics;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Studio Director")
    int32 CurrentCycle;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Studio Director")
    TArray<FString> SystemWarnings;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Studio Director")
    TArray<FString> CriticalIssues;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    bool bEnableDebugLogging;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    float PlayabilityThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    int32 MaxCriticalBlockers;

private:
    // Internal Functions
    void InitializeMilestone1();
    void UpdateProductionMetrics();
    void CheckForCriticalBlockers();
    void ValidateAgentOutputs();
    void LogProductionState();
};