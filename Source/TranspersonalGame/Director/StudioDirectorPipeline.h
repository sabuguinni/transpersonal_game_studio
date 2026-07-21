#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "StudioDirectorPipeline.generated.h"

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle,
    Working,
    Completed,
    Blocked,
    Failed
};

UENUM(BlueprintType)
enum class EDir_MilestoneType : uint8
{
    WalkAround,
    BasicSurvival,
    DinosaurEncounter,
    CraftingSystem,
    FullPrototype
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Task")
    int32 AgentID = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Task")
    FString AgentName;

    UPROPERTY(BlueprintReadWrite, Category = "Task")
    FString TaskDescription;

    UPROPERTY(BlueprintReadWrite, Category = "Task")
    EDir_AgentStatus Status = EDir_AgentStatus::Idle;

    UPROPERTY(BlueprintReadWrite, Category = "Task")
    TArray<int32> Dependencies;

    UPROPERTY(BlueprintReadWrite, Category = "Task")
    float Priority = 1.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Task")
    FDateTime StartTime;

    UPROPERTY(BlueprintReadWrite, Category = "Task")
    FDateTime Deadline;

    UPROPERTY(BlueprintReadWrite, Category = "Task")
    FString ExpectedDeliverables;

    FDir_AgentTask()
    {
        StartTime = FDateTime::Now();
        Deadline = StartTime + FTimespan::FromHours(2);
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_ProductionMilestone
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Milestone")
    EDir_MilestoneType Type = EDir_MilestoneType::WalkAround;

    UPROPERTY(BlueprintReadWrite, Category = "Milestone")
    FString Name;

    UPROPERTY(BlueprintReadWrite, Category = "Milestone")
    FString Description;

    UPROPERTY(BlueprintReadWrite, Category = "Milestone")
    TArray<int32> RequiredAgents;

    UPROPERTY(BlueprintReadWrite, Category = "Milestone")
    bool bIsCompleted = false;

    UPROPERTY(BlueprintReadWrite, Category = "Milestone")
    float CompletionPercentage = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Milestone")
    TArray<FString> CompletionCriteria;

    UPROPERTY(BlueprintReadWrite, Category = "Milestone")
    FDateTime TargetDate;
};

/**
 * Studio Director Pipeline - Coordinates the 19-agent production chain
 * Ensures proper sequencing, dependency management, and milestone tracking
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UStudioDirectorPipeline : public UActorComponent
{
    GENERATED_BODY()

public:
    UStudioDirectorPipeline();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core Pipeline Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeProductionPipeline();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void StartProductionCycle();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool ValidateMilestone(EDir_MilestoneType MilestoneType);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void BlockProductionPipeline(const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ResumeProductionPipeline();

    // Agent Coordination
    UFUNCTION(BlueprintCallable, Category = "Agent Management")
    void AssignTaskToAgent(int32 AgentID, const FString& TaskDescription, const TArray<int32>& Dependencies);

    UFUNCTION(BlueprintCallable, Category = "Agent Management")
    void UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Agent Management")
    TArray<FDir_AgentTask> GetPendingTasks();

    UFUNCTION(BlueprintCallable, Category = "Agent Management")
    bool CanAgentStart(int32 AgentID);

    // Milestone Tracking
    UFUNCTION(BlueprintCallable, Category = "Milestones")
    void CreateMilestone(EDir_MilestoneType Type, const FString& Name, const TArray<int32>& RequiredAgents);

    UFUNCTION(BlueprintCallable, Category = "Milestones")
    void UpdateMilestoneProgress(EDir_MilestoneType Type, float Progress);

    UFUNCTION(BlueprintCallable, Category = "Milestones")
    FDir_ProductionMilestone GetCurrentMilestone();

    // Emergency Protocols
    UFUNCTION(BlueprintCallable, Category = "Emergency")
    void TriggerEmergencyStop(const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "Emergency")
    void InitiateRollback(int32 CyclesBack);

    UFUNCTION(BlueprintCallable, Category = "Emergency")
    void ForceAgentRestart(int32 AgentID);

    // Production Status
    UFUNCTION(BlueprintCallable, Category = "Status")
    float GetOverallProgress();

    UFUNCTION(BlueprintCallable, Category = "Status")
    FString GetProductionReport();

    UFUNCTION(BlueprintCallable, Category = "Status")
    bool IsProductionOnSchedule();

protected:
    // Agent Registry
    UPROPERTY(BlueprintReadWrite, Category = "Agents")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(BlueprintReadWrite, Category = "Agents")
    TMap<int32, FString> AgentNames;

    // Milestone System
    UPROPERTY(BlueprintReadWrite, Category = "Milestones")
    TArray<FDir_ProductionMilestone> ProductionMilestones;

    UPROPERTY(BlueprintReadWrite, Category = "Milestones")
    EDir_MilestoneType CurrentMilestone = EDir_MilestoneType::WalkAround;

    // Production State
    UPROPERTY(BlueprintReadWrite, Category = "Production")
    bool bProductionActive = false;

    UPROPERTY(BlueprintReadWrite, Category = "Production")
    bool bEmergencyStop = false;

    UPROPERTY(BlueprintReadWrite, Category = "Production")
    FDateTime CycleStartTime;

    UPROPERTY(BlueprintReadWrite, Category = "Production")
    int32 CurrentCycleNumber = 0;

    // Performance Tracking
    UPROPERTY(BlueprintReadWrite, Category = "Performance")
    float AverageCycleTime = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Performance")
    int32 CompletedCycles = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Performance")
    TArray<float> CycleTimes;

private:
    void InitializeAgentRegistry();
    void InitializeDefaultMilestones();
    void UpdateCycleMetrics();
    bool ValidateAgentDependencies(int32 AgentID);
    void NotifyMilestoneCompletion(EDir_MilestoneType CompletedMilestone);
};