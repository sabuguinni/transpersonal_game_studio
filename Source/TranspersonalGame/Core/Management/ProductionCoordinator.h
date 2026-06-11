#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "ProductionCoordinator.generated.h"

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    Planning,
    PreProduction,
    Production,
    Alpha,
    Beta,
    Gold
};

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle,
    Working,
    Blocked,
    Complete,
    Failed
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    int32 AgentID = 0;

    UPROPERTY(BlueprintReadWrite)
    FString AgentName;

    UPROPERTY(BlueprintReadWrite)
    FString TaskDescription;

    UPROPERTY(BlueprintReadWrite)
    EDir_AgentStatus Status = EDir_AgentStatus::Idle;

    UPROPERTY(BlueprintReadWrite)
    float Priority = 1.0f;

    UPROPERTY(BlueprintReadWrite)
    FString Dependencies;

    UPROPERTY(BlueprintReadWrite)
    FDateTime StartTime;

    UPROPERTY(BlueprintReadWrite)
    FDateTime EstimatedCompletion;

    FDir_AgentTask()
    {
        StartTime = FDateTime::Now();
        EstimatedCompletion = StartTime + FTimespan::FromHours(2);
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    int32 CycleNumber = 0;

    UPROPERTY(BlueprintReadWrite)
    int32 CompletedTasks = 0;

    UPROPERTY(BlueprintReadWrite)
    int32 ActiveAgents = 0;

    UPROPERTY(BlueprintReadWrite)
    int32 BlockedAgents = 0;

    UPROPERTY(BlueprintReadWrite)
    float OverallProgress = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    FString CurrentMilestone;

    UPROPERTY(BlueprintReadWrite)
    TArray<FString> CompletedDeliverables;

    UPROPERTY(BlueprintReadWrite)
    TArray<FString> CriticalIssues;
};

/**
 * Studio Director's Production Coordinator
 * Manages the 19-agent production pipeline and tracks deliverables
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UProductionCoordinator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UProductionCoordinator();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Production Phase Management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void SetProductionPhase(EDir_ProductionPhase NewPhase);

    UFUNCTION(BlueprintPure, Category = "Production")
    EDir_ProductionPhase GetCurrentPhase() const { return CurrentPhase; }

    // Agent Task Management
    UFUNCTION(BlueprintCallable, Category = "Agent Management")
    void AssignTask(int32 AgentID, const FString& TaskDescription, float Priority = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Agent Management")
    void UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Agent Management")
    void CompleteTask(int32 AgentID, const FString& Deliverable);

    UFUNCTION(BlueprintCallable, Category = "Agent Management")
    void BlockAgent(int32 AgentID, const FString& BlockingIssue);

    // Production Metrics
    UFUNCTION(BlueprintCallable, Category = "Metrics")
    FDir_ProductionMetrics GetProductionMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Metrics")
    void IncrementCycle();

    UFUNCTION(BlueprintCallable, Category = "Metrics")
    void SetMilestone(const FString& MilestoneName);

    // Critical Path Analysis
    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<int32> GetCriticalPathAgents() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentTask> GetBlockedTasks() const;

    // Milestone Tracking
    UFUNCTION(BlueprintCallable, Category = "Milestones")
    void MarkMilestoneComplete(const FString& MilestoneName);

    UFUNCTION(BlueprintCallable, Category = "Milestones")
    bool IsMilestoneComplete(const FString& MilestoneName) const;

    // Emergency Protocols
    UFUNCTION(BlueprintCallable, Category = "Emergency", CallInEditor)
    void TriggerProductionHalt(const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "Emergency", CallInEditor)
    void ResumeProduction();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Production")
    EDir_ProductionPhase CurrentPhase = EDir_ProductionPhase::Production;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    FDir_ProductionMetrics ProductionMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    TArray<FString> CompletedMilestones;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    bool bProductionHalted = false;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    FString HaltReason;

private:
    void InitializeAgentTasks();
    void UpdateProductionMetrics();
    FDir_AgentTask* FindAgentTask(int32 AgentID);
    void LogProductionEvent(const FString& Event);
};