#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Director_CriticalPathManager.generated.h"

UENUM(BlueprintType)
enum class EDir_CriticalPathPhase : uint8
{
    Phase1_CoreSystems     UMETA(DisplayName = "Phase 1: Core Systems"),
    Phase2_Character       UMETA(DisplayName = "Phase 2: Character & Movement"),
    Phase3_Environment     UMETA(DisplayName = "Phase 3: Environment & Terrain"),
    Phase4_AI              UMETA(DisplayName = "Phase 4: AI & Dinosaurs"),
    Phase5_Survival        UMETA(DisplayName = "Phase 5: Survival Systems"),
    Phase6_Integration     UMETA(DisplayName = "Phase 6: Integration & Polish")
};

UENUM(BlueprintType)
enum class EDir_TaskPriority : uint8
{
    Critical    UMETA(DisplayName = "Critical - Blocks Release"),
    High        UMETA(DisplayName = "High - Major Feature"),
    Medium      UMETA(DisplayName = "Medium - Enhancement"),
    Low         UMETA(DisplayName = "Low - Polish")
};

UENUM(BlueprintType)
enum class EDir_TaskStatus : uint8
{
    NotStarted  UMETA(DisplayName = "Not Started"),
    InProgress  UMETA(DisplayName = "In Progress"),
    Testing     UMETA(DisplayName = "Testing"),
    Blocked     UMETA(DisplayName = "Blocked"),
    Complete    UMETA(DisplayName = "Complete")
};

USTRUCT(BlueprintType)
struct FDir_CriticalTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString TaskName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    int32 AgentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_TaskPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_TaskStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_CriticalPathPhase Phase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TArray<int32> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float EstimatedHours;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float ActualHours;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString BlockingIssue;

    FDir_CriticalTask()
    {
        TaskName = TEXT("");
        Description = TEXT("");
        AgentID = 0;
        Priority = EDir_TaskPriority::Medium;
        Status = EDir_TaskStatus::NotStarted;
        Phase = EDir_CriticalPathPhase::Phase1_CoreSystems;
        EstimatedHours = 0.0f;
        ActualHours = 0.0f;
        BlockingIssue = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct FDir_PhaseMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    EDir_CriticalPathPhase Phase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CompletedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 BlockedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float EstimatedRemainingHours;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    bool bCanStartNextPhase;

    FDir_PhaseMetrics()
    {
        Phase = EDir_CriticalPathPhase::Phase1_CoreSystems;
        TotalTasks = 0;
        CompletedTasks = 0;
        BlockedTasks = 0;
        CompletionPercentage = 0.0f;
        EstimatedRemainingHours = 0.0f;
        bCanStartNextPhase = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDir_CriticalPathManager : public UObject
{
    GENERATED_BODY()

public:
    UDir_CriticalPathManager();

    // Core Critical Path Management
    UFUNCTION(BlueprintCallable, Category = "Critical Path")
    void InitializeCriticalPath();

    UFUNCTION(BlueprintCallable, Category = "Critical Path")
    void UpdateTaskStatus(const FString& TaskName, EDir_TaskStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Critical Path")
    void BlockTask(const FString& TaskName, const FString& BlockingReason);

    UFUNCTION(BlueprintCallable, Category = "Critical Path")
    void UnblockTask(const FString& TaskName);

    UFUNCTION(BlueprintCallable, Category = "Critical Path")
    bool CanAdvanceToPhase(EDir_CriticalPathPhase TargetPhase);

    UFUNCTION(BlueprintCallable, Category = "Critical Path")
    void AdvanceToNextPhase();

    // Task Management
    UFUNCTION(BlueprintCallable, Category = "Tasks")
    void AddCriticalTask(const FDir_CriticalTask& NewTask);

    UFUNCTION(BlueprintCallable, Category = "Tasks")
    TArray<FDir_CriticalTask> GetTasksForAgent(int32 AgentID);

    UFUNCTION(BlueprintCallable, Category = "Tasks")
    TArray<FDir_CriticalTask> GetTasksForPhase(EDir_CriticalPathPhase Phase);

    UFUNCTION(BlueprintCallable, Category = "Tasks")
    TArray<FDir_CriticalTask> GetBlockedTasks();

    UFUNCTION(BlueprintCallable, Category = "Tasks")
    TArray<FDir_CriticalTask> GetCriticalTasks();

    // Metrics and Reporting
    UFUNCTION(BlueprintCallable, Category = "Metrics")
    FDir_PhaseMetrics GetPhaseMetrics(EDir_CriticalPathPhase Phase);

    UFUNCTION(BlueprintCallable, Category = "Metrics")
    TArray<FDir_PhaseMetrics> GetAllPhaseMetrics();

    UFUNCTION(BlueprintCallable, Category = "Metrics")
    float GetOverallCompletionPercentage();

    UFUNCTION(BlueprintCallable, Category = "Metrics")
    float GetProjectedCompletionHours();

    // Agent Coordination
    UFUNCTION(BlueprintCallable, Category = "Coordination")
    TArray<int32> GetReadyAgents();

    UFUNCTION(BlueprintCallable, Category = "Coordination")
    TArray<int32> GetBlockedAgents();

    UFUNCTION(BlueprintCallable, Category = "Coordination")
    void AssignTaskToAgent(const FString& TaskName, int32 AgentID);

    UFUNCTION(BlueprintCallable, Category = "Coordination")
    void ReassignBlockedTasks();

    // Critical Path Validation
    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateCriticalPath();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    TArray<FString> GetCriticalPathViolations();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    void GenerateCriticalPathReport();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Critical Path")
    TArray<FDir_CriticalTask> CriticalTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Critical Path")
    EDir_CriticalPathPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Critical Path")
    TArray<FDir_PhaseMetrics> PhaseMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Critical Path")
    float TotalProjectHours;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Critical Path")
    float CompletedProjectHours;

private:
    void CalculatePhaseMetrics(EDir_CriticalPathPhase Phase);
    void UpdateDependencies();
    bool CheckDependenciesComplete(const FDir_CriticalTask& Task);
    void InitializePhase1Tasks();
    void InitializePhase2Tasks();
    void InitializePhase3Tasks();
    void InitializePhase4Tasks();
    void InitializePhase5Tasks();
    void InitializePhase6Tasks();
};