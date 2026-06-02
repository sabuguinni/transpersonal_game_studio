#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Dir_AgentCoordinator.generated.h"

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle,
    Working,
    Blocked,
    Complete,
    Failed
};

UENUM(BlueprintType)
enum class EDir_MilestonePhase : uint8
{
    WalkAround,
    BasicSurvival,
    DinosaurEncounter,
    CraftingSystem,
    FullPrototype
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    int32 AgentID;

    UPROPERTY(BlueprintReadOnly)
    FString AgentName;

    UPROPERTY(BlueprintReadOnly)
    FString CurrentTask;

    UPROPERTY(BlueprintReadOnly)
    EDir_AgentStatus Status;

    UPROPERTY(BlueprintReadOnly)
    float ProgressPercentage;

    UPROPERTY(BlueprintReadOnly)
    FString BlockingIssue;

    UPROPERTY(BlueprintReadOnly)
    TArray<FString> Dependencies;

    UPROPERTY(BlueprintReadOnly)
    FDateTime LastUpdate;

    FDir_AgentTask()
    {
        AgentID = 0;
        AgentName = TEXT("");
        CurrentTask = TEXT("");
        Status = EDir_AgentStatus::Idle;
        ProgressPercentage = 0.0f;
        BlockingIssue = TEXT("");
        LastUpdate = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FDir_MilestoneRequirement
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString RequirementName;

    UPROPERTY(BlueprintReadOnly)
    TArray<int32> RequiredAgents;

    UPROPERTY(BlueprintReadOnly)
    bool bIsComplete;

    UPROPERTY(BlueprintReadOnly)
    FString ValidationCriteria;

    FDir_MilestoneRequirement()
    {
        RequirementName = TEXT("");
        bIsComplete = false;
        ValidationCriteria = TEXT("");
    }
};

/**
 * Studio Director Agent Coordination System
 * Manages the 19-agent production pipeline for the prehistoric survival game
 */
UCLASS()
class TRANSPERSONALGAME_API UDir_AgentCoordinator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Agent Management
    UFUNCTION(BlueprintCallable, Category = "Agent Coordination")
    void RegisterAgent(int32 AgentID, const FString& AgentName, const FString& Specialization);

    UFUNCTION(BlueprintCallable, Category = "Agent Coordination")
    void UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus, const FString& CurrentTask, float Progress = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Agent Coordination")
    void ReportAgentBlocked(int32 AgentID, const FString& BlockingIssue);

    UFUNCTION(BlueprintCallable, Category = "Agent Coordination")
    TArray<FDir_AgentTask> GetAllAgentTasks() const;

    UFUNCTION(BlueprintCallable, Category = "Agent Coordination")
    FDir_AgentTask GetAgentTask(int32 AgentID) const;

    // Milestone Management
    UFUNCTION(BlueprintCallable, Category = "Milestone Management")
    void SetCurrentMilestone(EDir_MilestonePhase NewMilestone);

    UFUNCTION(BlueprintCallable, Category = "Milestone Management")
    bool ValidateMilestoneCompletion(EDir_MilestonePhase Milestone);

    UFUNCTION(BlueprintCallable, Category = "Milestone Management")
    TArray<FDir_MilestoneRequirement> GetMilestoneRequirements(EDir_MilestonePhase Milestone) const;

    // Critical Path Analysis
    UFUNCTION(BlueprintCallable, Category = "Production Analysis")
    TArray<int32> GetCriticalPathAgents() const;

    UFUNCTION(BlueprintCallable, Category = "Production Analysis")
    TArray<int32> GetBlockedAgents() const;

    UFUNCTION(BlueprintCallable, Category = "Production Analysis")
    float GetOverallProgress() const;

    // Production Reporting
    UFUNCTION(BlueprintCallable, Category = "Production Reporting")
    FString GenerateProductionReport() const;

    UFUNCTION(BlueprintCallable, Category = "Production Reporting")
    void LogProductionEvent(const FString& EventDescription, int32 AgentID = -1);

protected:
    UPROPERTY()
    TMap<int32, FDir_AgentTask> AgentTasks;

    UPROPERTY()
    EDir_MilestonePhase CurrentMilestone;

    UPROPERTY()
    TArray<FString> ProductionLog;

    // Initialize milestone requirements
    void InitializeMilestoneRequirements();
    TMap<EDir_MilestonePhase, TArray<FDir_MilestoneRequirement>> MilestoneRequirements;

    // Agent dependency tracking
    bool CanAgentProceed(int32 AgentID) const;
    TArray<int32> GetAgentDependencies(int32 AgentID) const;
};