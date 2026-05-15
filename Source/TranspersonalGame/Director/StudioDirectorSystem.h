#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "../SharedTypes.h"
#include "StudioDirectorSystem.generated.h"

/**
 * Agent Task Priority Levels for Studio Director coordination
 */
UENUM(BlueprintType)
enum class EDir_AgentTaskPriority : uint8
{
    Critical    UMETA(DisplayName = "Critical"),
    High        UMETA(DisplayName = "High"), 
    Medium      UMETA(DisplayName = "Medium"),
    Low         UMETA(DisplayName = "Low"),
    Deferred    UMETA(DisplayName = "Deferred")
};

/**
 * Agent Task Status for tracking completion
 */
UENUM(BlueprintType)
enum class EDir_AgentTaskStatus : uint8
{
    Pending     UMETA(DisplayName = "Pending"),
    InProgress  UMETA(DisplayName = "In Progress"),
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed"),
    Blocked     UMETA(DisplayName = "Blocked")
};

/**
 * Studio Director Task Definition
 */
USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    int32 AgentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_AgentTaskPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_AgentTaskStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FVector WorldPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float EstimatedDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TArray<FString> Deliverables;

    FDir_AgentTask()
    {
        AgentID = 0;
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Priority = EDir_AgentTaskPriority::Medium;
        Status = EDir_AgentTaskStatus::Pending;
        WorldPosition = FVector::ZeroVector;
        EstimatedDuration = 0.0f;
    }
};

/**
 * Production Cycle Metrics
 */
USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CycleNumber;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float TotalExecutionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CompletedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 FailedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 FilesCreated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 UE5CommandsExecuted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float BudgetUsed;

    FDir_ProductionMetrics()
    {
        CycleNumber = 0;
        TotalExecutionTime = 0.0f;
        CompletedTasks = 0;
        FailedTasks = 0;
        FilesCreated = 0;
        UE5CommandsExecuted = 0;
        BudgetUsed = 0.0f;
    }
};

/**
 * Studio Director System Component
 * Coordinates all 19 agents and tracks production pipeline
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UStudioDirectorSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UStudioDirectorSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Agent Task Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CreateAgentTask(int32 AgentID, const FString& AgentName, const FString& TaskDescription, 
                        EDir_AgentTaskPriority Priority, const FVector& WorldPosition);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateTaskStatus(int32 AgentID, EDir_AgentTaskStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentTask> GetTasksForAgent(int32 AgentID);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentTask> GetTasksByPriority(EDir_AgentTaskPriority Priority);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentTask> GetTasksByStatus(EDir_AgentTaskStatus Status);

    // Production Pipeline Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void StartProductionCycle(int32 CycleNumber);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void EndProductionCycle();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_ProductionMetrics GetCurrentCycleMetrics();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void LogAgentAction(int32 AgentID, const FString& Action, bool bSuccess);

    // Milestone Tracking
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CheckMilestone1Progress();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool IsMilestone1Complete();

    // Critical Agent Coordination
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AssignCriticalTasks();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ValidateAgentDependencies();

protected:
    // Current agent tasks
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tasks")
    TArray<FDir_AgentTask> AgentTasks;

    // Current production cycle
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FDir_ProductionMetrics CurrentCycle;

    // Milestone 1 requirements
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestones")
    bool bCharacterMovementComplete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestones")
    bool bTerrainComplete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestones")
    bool bDinosaurActorsComplete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestones")
    bool bLightingComplete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestones")
    bool bSurvivalUIComplete;

private:
    void InitializeAgentTasks();
    void UpdateCycleMetrics();
    bool ValidateTaskDependencies(const FDir_AgentTask& Task);
};