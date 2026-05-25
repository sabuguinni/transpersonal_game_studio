#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "ProductionCoordinator.generated.h"

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Working     UMETA(DisplayName = "Working"),
    Completed   UMETA(DisplayName = "Completed"),
    Blocked     UMETA(DisplayName = "Blocked"),
    Failed      UMETA(DisplayName = "Failed")
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
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FDateTime StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FDateTime DeadlineTime;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Status = EDir_AgentStatus::Idle;
        Priority = 1.0f;
        StartTime = FDateTime::Now();
        DeadlineTime = FDateTime::Now() + FTimespan::FromHours(1);
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CompletedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 FailedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float OverallProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    FString CurrentMilestone;

    FDir_ProductionMetrics()
    {
        TotalTasks = 0;
        CompletedTasks = 0;
        FailedTasks = 0;
        OverallProgress = 0.0f;
        CurrentMilestone = TEXT("Milestone 1 - Walk Around");
    }
};

/**
 * Production Coordinator - Studio Director's central command system
 * Tracks all 19 agents, manages task dependencies, and ensures milestone delivery
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UProductionCoordinator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UProductionCoordinator();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Task Management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void AssignTask(const FString& AgentName, const FString& TaskDescription, float Priority = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateTaskStatus(const FString& AgentName, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentTask> GetTasksForAgent(const FString& AgentName);

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentTask> GetAllTasks();

    // Metrics and Monitoring
    UFUNCTION(BlueprintCallable, Category = "Production")
    FDir_ProductionMetrics GetProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void LogProductionEvent(const FString& EventDescription);

    // Milestone Management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void SetCurrentMilestone(const FString& MilestoneName);

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool IsMilestoneComplete(const FString& MilestoneName);

    // Agent Chain Coordination
    UFUNCTION(BlueprintCallable, Category = "Production")
    void TriggerNextAgent(const FString& CurrentAgent);

    UFUNCTION(BlueprintCallable, Category = "Production")
    FString GetNextAgentInChain(const FString& CurrentAgent);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Production")
    TArray<FDir_AgentTask> ActiveTasks;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    FDir_ProductionMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    TMap<FString, int32> AgentChainOrder;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    TArray<FString> ProductionLog;

private:
    void InitializeAgentChain();
    void UpdateMetrics();
    void ValidateTaskDependencies();
};