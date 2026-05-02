#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "../../SharedTypes.h"
#include "Dir_ProductionCoordinator.generated.h"

/**
 * Studio Director Production Coordinator - Cycle 009
 * Manages the coordination between all 19 agents in the production pipeline
 * Ensures Milestone 1 (Playable Prototype) is achieved through systematic agent task management
 */

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    int32 AgentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString ExpectedDeliverable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    float Priority;

    FDir_AgentTask()
    {
        AgentID = 0;
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        ExpectedDeliverable = TEXT("");
        bIsCompleted = false;
        Priority = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct FDir_MilestoneStatus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FString> RequiredComponents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FString> CompletedComponents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bIsCriticalPath;

    FDir_MilestoneStatus()
    {
        MilestoneName = TEXT("");
        CompletionPercentage = 0.0f;
        bIsCriticalPath = false;
    }
};

UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDir_ProductionCoordinator : public UActorComponent
{
    GENERATED_BODY()

public:
    UDir_ProductionCoordinator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Agent Management
    UFUNCTION(BlueprintCallable, Category = "Production Coordination")
    void InitializeAgentTasks();

    UFUNCTION(BlueprintCallable, Category = "Production Coordination")
    void AssignTaskToAgent(int32 AgentID, const FString& TaskDescription, const FString& ExpectedDeliverable, float Priority = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Production Coordination")
    void MarkTaskCompleted(int32 AgentID, const FString& DeliverableResult);

    UFUNCTION(BlueprintCallable, Category = "Production Coordination")
    TArray<FDir_AgentTask> GetPendingTasks();

    UFUNCTION(BlueprintCallable, Category = "Production Coordination")
    TArray<FDir_AgentTask> GetTasksForAgent(int32 AgentID);

    // Milestone Management
    UFUNCTION(BlueprintCallable, Category = "Production Coordination")
    void InitializeMilestone1Tasks();

    UFUNCTION(BlueprintCallable, Category = "Production Coordination")
    FDir_MilestoneStatus GetMilestone1Status();

    UFUNCTION(BlueprintCallable, Category = "Production Coordination")
    void UpdateMilestoneProgress();

    // Critical Path Management
    UFUNCTION(BlueprintCallable, Category = "Production Coordination")
    TArray<int32> GetCriticalPathAgents();

    UFUNCTION(BlueprintCallable, Category = "Production Coordination")
    void EscalateCriticalIssue(int32 AgentID, const FString& IssueDescription);

    // Production Reporting
    UFUNCTION(BlueprintCallable, Category = "Production Coordination")
    FString GenerateProductionReport();

    UFUNCTION(BlueprintCallable, Category = "Production Coordination")
    void LogProductionStatus();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Management")
    TArray<FDir_AgentTask> ActiveTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Management")
    TArray<FDir_AgentTask> CompletedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone Management")
    FDir_MilestoneStatus Milestone1Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Settings")
    float ProductionUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Settings")
    bool bAutoAssignTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Settings")
    bool bEnableProductionLogging;

private:
    float LastUpdateTime;
    void UpdateAgentTaskPriorities();
    void CheckMilestoneBlockers();
    void ValidateTaskDependencies();
};