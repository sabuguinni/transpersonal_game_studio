#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/World.h"
#include "StudioDirectorTaskManager.generated.h"

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle = 0,
    Working = 1,
    Completed = 2,
    Blocked = 3,
    Failed = 4
};

UENUM(BlueprintType)
enum class EDir_TaskPriority : uint8
{
    Critical = 0,
    High = 1,
    Medium = 2,
    Low = 3
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    int32 AgentID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_TaskPriority Priority = EDir_TaskPriority::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_AgentStatus Status = EDir_AgentStatus::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TArray<int32> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float EstimatedTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString ExpectedDeliverables;

    FDir_AgentTask()
    {
        AgentID = 0;
        TaskDescription = TEXT("");
        Priority = EDir_TaskPriority::Medium;
        Status = EDir_AgentStatus::Idle;
        EstimatedTime = 0.0f;
        ExpectedDeliverables = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct FDir_MilestoneTracker
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<int32> RequiredAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FString> RequiredDeliverables;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bIsCompleted = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float CompletionPercentage = 0.0f;

    FDir_MilestoneTracker()
    {
        MilestoneName = TEXT("");
        bIsCompleted = false;
        CompletionPercentage = 0.0f;
    }
};

/**
 * Studio Director Task Management System
 * Coordinates the 19-agent production pipeline for Transpersonal Game Studio
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UStudioDirectorTaskManager : public UObject
{
    GENERATED_BODY()

public:
    UStudioDirectorTaskManager();

    // Task Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeAgentPipeline();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AssignTaskToAgent(int32 AgentID, const FString& TaskDescription, EDir_TaskPriority Priority);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool CanAgentStart(int32 AgentID) const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentTask> GetBlockedTasks() const;

    // Milestone Tracking
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CreateMilestone(const FString& MilestoneName, const TArray<int32>& RequiredAgents);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    float GetMilestoneProgress(const FString& MilestoneName) const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool IsMilestoneComplete(const FString& MilestoneName) const;

    // Critical Path Analysis
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<int32> GetCriticalPathAgents() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ReportAgentDeliverable(int32 AgentID, const FString& DeliverableName);

    // Emergency Protocols
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void TriggerEmergencyReallocation();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void BlockAgent(int32 AgentID, const FString& Reason);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    TArray<FDir_MilestoneTracker> Milestones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    TMap<int32, EDir_AgentStatus> AgentStatusMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    TMap<int32, TArray<FString>> AgentDeliverables;

private:
    void SetupDefaultMilestones();
    void ValidateAgentDependencies();
    bool AreAgentDependenciesMet(int32 AgentID) const;
};

#include "StudioDirectorTaskManager.generated.h"