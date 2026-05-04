#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "ProductionCoordinator.generated.h"

UENUM(BlueprintType)
enum class EDir_TaskPriority : uint8
{
    Critical    UMETA(DisplayName = "Critical"),
    High        UMETA(DisplayName = "High"), 
    Medium      UMETA(DisplayName = "Medium"),
    Low         UMETA(DisplayName = "Low")
};

UENUM(BlueprintType)
enum class EDir_TaskStatus : uint8
{
    Pending     UMETA(DisplayName = "Pending"),
    InProgress  UMETA(DisplayName = "In Progress"),
    Complete    UMETA(DisplayName = "Complete"),
    Blocked     UMETA(DisplayName = "Blocked")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_ProductionTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString TaskName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_TaskPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_TaskStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString AssignedAgent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TArray<FString> Deliverables;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float ProgressPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FDateTime CreatedTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FDateTime DeadlineTime;

    FDir_ProductionTask()
    {
        TaskName = TEXT("");
        Description = TEXT("");
        Priority = EDir_TaskPriority::Medium;
        Status = EDir_TaskStatus::Pending;
        AssignedAgent = TEXT("");
        ProgressPercentage = 0.0f;
        CreatedTime = FDateTime::Now();
        DeadlineTime = FDateTime::Now() + FTimespan::FromHours(24);
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_MilestoneProgress
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FString> RequiredFeatures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FString> CompletedFeatures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bIsBlocked;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString BlockingReason;

    FDir_MilestoneProgress()
    {
        MilestoneName = TEXT("");
        CompletionPercentage = 0.0f;
        bIsBlocked = false;
        BlockingReason = TEXT("");
    }
};

/**
 * Production Coordinator - Studio Director's central coordination system
 * Manages task distribution, milestone tracking, and agent coordination
 * for the Transpersonal Game Studio development pipeline
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDir_ProductionCoordinator : public UObject
{
    GENERATED_BODY()

public:
    UDir_ProductionCoordinator();

    // Task Management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void CreateTask(const FString& TaskName, const FString& Description, 
                   EDir_TaskPriority Priority, const FString& AssignedAgent);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateTaskStatus(const FString& TaskName, EDir_TaskStatus NewStatus, float Progress = -1.0f);

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_ProductionTask> GetTasksByAgent(const FString& AgentName) const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_ProductionTask> GetTasksByPriority(EDir_TaskPriority Priority) const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_ProductionTask> GetBlockedTasks() const;

    // Milestone Management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void CreateMilestone(const FString& MilestoneName, const TArray<FString>& RequiredFeatures);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateMilestoneProgress(const FString& MilestoneName, const TArray<FString>& CompletedFeatures);

    UFUNCTION(BlueprintCallable, Category = "Production")
    FDir_MilestoneProgress GetMilestoneProgress(const FString& MilestoneName) const;

    // Coordination Functions
    UFUNCTION(BlueprintCallable, Category = "Production")
    void GenerateProductionReport();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void DispatchTasksToAgents();

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool ValidateAgentDeliverables(const FString& AgentName);

    // Critical Path Analysis
    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FString> GetCriticalPathTasks() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    void IdentifyBottlenecks();

    // Studio Status
    UFUNCTION(BlueprintCallable, Category = "Production")
    float GetOverallProjectProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    int32 GetActiveAgentCount() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    void LogProductionMetrics();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Production", meta = (AllowPrivateAccess = "true"))
    TArray<FDir_ProductionTask> ActiveTasks;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Production", meta = (AllowPrivateAccess = "true"))
    TArray<FDir_MilestoneProgress> ProjectMilestones;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Production", meta = (AllowPrivateAccess = "true"))
    TMap<FString, int32> AgentWorkload;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Production", meta = (AllowPrivateAccess = "true"))
    TArray<FString> CriticalPathAgents;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Production", meta = (AllowPrivateAccess = "true"))
    float ProjectStartTime;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Production", meta = (AllowPrivateAccess = "true"))
    bool bProductionActive;

private:
    void InitializeDefaultTasks();
    void InitializeMilestone1();
    void CalculateCriticalPath();
    void UpdateAgentWorkloads();
    FString GenerateTaskID(const FString& TaskName) const;
};

#include "ProductionCoordinator.generated.h"