#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "ProductionCoordinator.generated.h"

// Studio Director Production Coordination System
// Manages agent tasks, milestones, and production tracking

UENUM(BlueprintType)
enum class EDir_AgentPriority : uint8
{
    LOW,
    MEDIUM, 
    HIGH,
    CRITICAL
};

UENUM(BlueprintType)
enum class EDir_TaskStatus : uint8
{
    NOT_STARTED,
    IN_PROGRESS,
    BLOCKED,
    COMPLETED,
    FAILED
};

UENUM(BlueprintType)
enum class EDir_MilestoneStatus : uint8
{
    PLANNING,
    IN_PROGRESS,
    TESTING,
    COMPLETED,
    CANCELLED
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString ExpectedDeliverable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_AgentPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_TaskStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    int32 CycleAssigned;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    int32 CycleDeadline;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString Notes;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        ExpectedDeliverable = TEXT("");
        Priority = EDir_AgentPriority::MEDIUM;
        Status = EDir_TaskStatus::NOT_STARTED;
        CycleAssigned = 0;
        CycleDeadline = 0;
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMilestone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    EDir_MilestoneStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FString> Requirements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    int32 TargetCycle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FString> BlockingIssues;

    FDir_ProductionMilestone()
    {
        MilestoneName = TEXT("");
        Description = TEXT("");
        Status = EDir_MilestoneStatus::PLANNING;
        CompletionPercentage = 0.0f;
        TargetCycle = 0;
    }
};

USTRUCT(BlueprintType)
struct FDir_CycleReport
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Report")
    int32 CycleNumber;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Report")
    TArray<FDir_AgentTask> CompletedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Report")
    TArray<FString> FilesCreated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Report")
    TArray<FString> IssuesEncountered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Report")
    float OverallProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Report")
    FString ExecutiveSummary;

    FDir_CycleReport()
    {
        CycleNumber = 0;
        OverallProgress = 0.0f;
        ExecutiveSummary = TEXT("");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDir_ProductionCoordinator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UDir_ProductionCoordinator();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Task Management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription, 
                          const FString& Deliverable, EDir_AgentPriority Priority);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateTaskStatus(const FString& AgentName, const FString& TaskDescription, EDir_TaskStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentTask> GetTasksForAgent(const FString& AgentName);

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentTask> GetTasksByPriority(EDir_AgentPriority Priority);

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentTask> GetTasksByStatus(EDir_TaskStatus Status);

    // Milestone Management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void CreateMilestone(const FString& Name, const FString& Description, 
                        const TArray<FString>& Requirements, int32 TargetCycle);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateMilestoneProgress(const FString& MilestoneName, float CompletionPercentage);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AddMilestoneBlocker(const FString& MilestoneName, const FString& BlockingIssue);

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_ProductionMilestone> GetActiveMilestones();

    // Cycle Management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void StartNewCycle(int32 CycleNumber);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void CompleteCycle(const FString& ExecutiveSummary);

    UFUNCTION(BlueprintCallable, Category = "Production")
    FDir_CycleReport GetCycleReport(int32 CycleNumber);

    UFUNCTION(BlueprintCallable, Category = "Production")
    int32 GetCurrentCycle() const { return CurrentCycle; }

    // Production Analytics
    UFUNCTION(BlueprintCallable, Category = "Production")
    float GetOverallProjectProgress();

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FString> GetCriticalPath();

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FString> GetBlockedAgents();

    // Debug and Editor Tools
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void LogProductionStatus();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void GenerateProductionReport();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void ResetProductionData();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Production")
    TArray<FDir_AgentTask> ActiveTasks;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Production")
    TArray<FDir_ProductionMilestone> ProjectMilestones;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Production")
    TArray<FDir_CycleReport> CycleHistory;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Production")
    int32 CurrentCycle;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Production")
    FDateTime ProjectStartTime;

private:
    void InitializeDefaultMilestones();
    void InitializeDefaultTasks();
    FDir_AgentTask* FindTask(const FString& AgentName, const FString& TaskDescription);
    FDir_ProductionMilestone* FindMilestone(const FString& MilestoneName);
    void UpdateMilestoneDependencies();
    void CalculateProjectProgress();
};