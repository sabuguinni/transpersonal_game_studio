#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ProductionMetrics.h"
#include "AgentCoordinator.generated.h"

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Working UMETA(DisplayName = "Working"),
    Blocked UMETA(DisplayName = "Blocked"),
    Complete UMETA(DisplayName = "Complete"),
    Error UMETA(DisplayName = "Error")
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Task")
    FString AgentName;

    UPROPERTY(BlueprintReadOnly, Category = "Task")
    FString TaskDescription;

    UPROPERTY(BlueprintReadOnly, Category = "Task")
    EDir_AgentStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "Task")
    int32 Priority;

    UPROPERTY(BlueprintReadOnly, Category = "Task")
    FDateTime Deadline;

    UPROPERTY(BlueprintReadOnly, Category = "Task")
    TArray<FString> Dependencies;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Status = EDir_AgentStatus::Idle;
        Priority = 0;
        Deadline = FDateTime::Now();
    }
};

/**
 * Agent Coordinator System
 * Manages task distribution and dependencies between 19 AI agents
 */
UCLASS()
class TRANSPERSONALGAME_API UAgentCoordinatorSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "Coordination")
    void AssignTask(const FString& AgentName, const FString& TaskDescription, int32 Priority = 0);

    UFUNCTION(BlueprintCallable, Category = "Coordination")
    void UpdateTaskStatus(const FString& AgentName, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Coordination")
    void AddTaskDependency(const FString& AgentName, const FString& DependsOnAgent);

    UFUNCTION(BlueprintCallable, Category = "Coordination")
    TArray<FDir_AgentTask> GetReadyTasks() const;

    UFUNCTION(BlueprintCallable, Category = "Coordination")
    TArray<FDir_AgentTask> GetBlockedTasks() const;

    UFUNCTION(BlueprintCallable, Category = "Coordination")
    bool IsAgentReady(const FString& AgentName) const;

    UFUNCTION(BlueprintCallable, Category = "Coordination")
    void InitializeMilestone1Tasks();

    UFUNCTION(BlueprintCallable, Category = "Coordination")
    float GetMilestone1Progress() const;

private:
    UPROPERTY()
    TArray<FDir_AgentTask> ActiveTasks;

    UPROPERTY()
    UProductionMetricsSubsystem* MetricsSubsystem;

    void UpdateTaskDependencies();
    bool AreTaskDependenciesMet(const FDir_AgentTask& Task) const;
};