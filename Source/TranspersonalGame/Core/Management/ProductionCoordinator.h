#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "ProductionCoordinator.generated.h"

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Working     UMETA(DisplayName = "Working"),
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed"),
    Blocked     UMETA(DisplayName = "Blocked")
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TArray<FString> Deliverables;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Status = EDir_AgentStatus::Idle;
        Priority = 1.0f;
        StartTime = FDateTime::Now();
        DeadlineTime = FDateTime::Now() + FTimespan::FromHours(24);
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 ActiveAgents;

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
        TotalAgents = 19;
        ActiveAgents = 0;
        CompletedTasks = 0;
        FailedTasks = 0;
        OverallProgress = 0.0f;
        CurrentMilestone = TEXT("Playable Prototype");
    }
};

/**
 * Production Coordinator - Manages the 19-agent development pipeline
 * Tracks task dependencies, deadlines, and production metrics
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UProductionCoordinator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UProductionCoordinator();

    // Subsystem overrides
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Agent Management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void RegisterAgent(const FString& AgentName, int32 AgentNumber);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AssignTask(const FString& AgentName, const FDir_AgentTask& Task);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateTaskStatus(const FString& AgentName, const FString& TaskDescription, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool CanAgentStart(const FString& AgentName) const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentTask> GetPendingTasks() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    FDir_ProductionMetrics GetProductionMetrics() const;

    // Priority Management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void SetHighPriorityTask(const FString& AgentName, const FString& TaskDescription);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void BlockAgent(const FString& AgentName, const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UnblockAgent(const FString& AgentName);

    // Milestone Tracking
    UFUNCTION(BlueprintCallable, Category = "Production")
    void SetCurrentMilestone(const FString& MilestoneName);

    UFUNCTION(BlueprintCallable, Category = "Production")
    float CalculateOverallProgress() const;

    // Debug and Monitoring
    UFUNCTION(BlueprintCallable, Category = "Production", CallInEditor = true)
    void PrintProductionStatus();

    UFUNCTION(BlueprintCallable, Category = "Production", CallInEditor = true)
    void ResetAllTasks();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Production")
    TMap<FString, TArray<FDir_AgentTask>> AgentTasks;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Production")
    TMap<FString, int32> AgentNumbers;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Production")
    TMap<FString, EDir_AgentStatus> AgentStatuses;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Production")
    TMap<FString, FString> BlockedReasons;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Production")
    FDir_ProductionMetrics CurrentMetrics;

private:
    void UpdateMetrics();
    bool CheckDependencies(const FDir_AgentTask& Task) const;
    void LogTaskUpdate(const FString& AgentName, const FString& TaskDescription, EDir_AgentStatus Status);
};