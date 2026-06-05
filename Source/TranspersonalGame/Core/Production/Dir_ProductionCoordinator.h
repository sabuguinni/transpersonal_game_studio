#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Engine.h"
#include "Dir_ProductionCoordinator.generated.h"

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle,
    Working,
    Completed,
    Blocked,
    Failed
};

UENUM(BlueprintType)
enum class EDir_MilestoneType : uint8
{
    WalkAround,
    BasicSurvival,
    DinosaurEncounters,
    CombatSystem,
    WorldGeneration
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Task")
    int32 AgentID = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Task")
    FString AgentName;

    UPROPERTY(BlueprintReadWrite, Category = "Task")
    FString TaskDescription;

    UPROPERTY(BlueprintReadWrite, Category = "Task")
    EDir_AgentStatus Status = EDir_AgentStatus::Idle;

    UPROPERTY(BlueprintReadWrite, Category = "Task")
    float Priority = 1.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Task")
    TArray<int32> Dependencies;

    UPROPERTY(BlueprintReadWrite, Category = "Task")
    FString OutputFiles;

    UPROPERTY(BlueprintReadWrite, Category = "Task")
    float EstimatedHours = 1.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Task")
    float ActualHours = 0.0f;
};

USTRUCT(BlueprintType)
struct FDir_ProductionMilestone
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Milestone")
    EDir_MilestoneType Type = EDir_MilestoneType::WalkAround;

    UPROPERTY(BlueprintReadWrite, Category = "Milestone")
    FString Name;

    UPROPERTY(BlueprintReadWrite, Category = "Milestone")
    FString Description;

    UPROPERTY(BlueprintReadWrite, Category = "Milestone")
    TArray<int32> RequiredAgents;

    UPROPERTY(BlueprintReadWrite, Category = "Milestone")
    bool bIsCompleted = false;

    UPROPERTY(BlueprintReadWrite, Category = "Milestone")
    float CompletionPercentage = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Milestone")
    FDateTime Deadline;
};

/**
 * Studio Director Production Coordinator
 * Manages all 19 agents and tracks production milestones
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDir_ProductionCoordinator : public UObject
{
    GENERATED_BODY()

public:
    UDir_ProductionCoordinator();

    // Agent Management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void InitializeAgents();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AssignTask(int32 AgentID, const FString& TaskDescription, float Priority = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool CanAgentStart(int32 AgentID) const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentTask> GetPendingTasks() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    FDir_AgentTask GetAgentTask(int32 AgentID) const;

    // Milestone Management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void CreateMilestone(EDir_MilestoneType Type, const FString& Name, const TArray<int32>& RequiredAgents);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateMilestoneProgress(EDir_MilestoneType Type);

    UFUNCTION(BlueprintCallable, Category = "Production")
    float GetMilestoneCompletion(EDir_MilestoneType Type) const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_ProductionMilestone> GetActiveMilestones() const;

    // Production Metrics
    UFUNCTION(BlueprintCallable, Category = "Production")
    float GetOverallProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    int32 GetActiveAgentCount() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    int32 GetBlockedAgentCount() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    FString GenerateProductionReport() const;

    // Critical Path Analysis
    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<int32> GetCriticalPath() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    void OptimizeTaskSchedule();

    // Quality Gates
    UFUNCTION(BlueprintCallable, Category = "Production")
    bool ValidateAgentOutput(int32 AgentID, const FString& OutputPath);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void TriggerQualityCheck();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Production")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    TArray<FDir_ProductionMilestone> Milestones;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    float TotalEstimatedHours = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    float TotalCompletedHours = 0.0f;

private:
    void InitializeDefaultMilestones();
    void CalculateDependencies();
    bool CheckDependenciesComplete(const TArray<int32>& Dependencies) const;
    void UpdateCriticalPath();
    
    TArray<int32> CriticalPathAgents;
    FDateTime LastUpdateTime;
};

#include "Dir_ProductionCoordinator.generated.h"