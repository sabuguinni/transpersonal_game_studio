#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Dir_ProductionCoordinator.generated.h"

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction,
    Milestone1_WalkAround,
    Milestone2_Survival,
    Milestone3_Dinosaurs,
    Alpha,
    Beta,
    Release
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime Deadline;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString BlockingReason;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Status = EDir_AgentStatus::Idle;
        Priority = 1.0f;
        Deadline = FDateTime::Now();
        BlockingReason = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct FDir_MilestoneProgress
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> Requirements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<bool> CompletionStatus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float OverallProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime TargetDate;

    FDir_MilestoneProgress()
    {
        MilestoneName = TEXT("");
        OverallProgress = 0.0f;
        TargetDate = FDateTime::Now();
    }
};

/**
 * Studio Director Production Coordinator
 * Manages the 19-agent production pipeline and milestone tracking
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDir_ProductionCoordinator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UDir_ProductionCoordinator();

    // Subsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Production Management
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Production")
    void InitializeProductionPipeline();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Production")
    void UpdateProductionStatus();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Production")
    void GenerateProductionReport();

    // Agent Coordination
    UFUNCTION(BlueprintCallable, Category = "Agent Management")
    void AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription, float Priority);

    UFUNCTION(BlueprintCallable, Category = "Agent Management")
    void UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus, const FString& BlockingReason = TEXT(""));

    UFUNCTION(BlueprintCallable, Category = "Agent Management")
    TArray<FDir_AgentTask> GetAgentTasks() const { return AgentTasks; }

    UFUNCTION(BlueprintCallable, Category = "Agent Management")
    FDir_AgentTask GetAgentTask(const FString& AgentName) const;

    // Milestone Management
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Milestones")
    void CreateMilestone(const FString& Name, const TArray<FString>& Requirements, const FDateTime& TargetDate);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Milestones")
    void UpdateMilestoneProgress(const FString& MilestoneName, int32 RequirementIndex, bool IsComplete);

    UFUNCTION(BlueprintCallable, Category = "Milestones")
    float GetMilestoneProgress(const FString& MilestoneName) const;

    UFUNCTION(BlueprintCallable, Category = "Milestones")
    TArray<FDir_MilestoneProgress> GetAllMilestones() const { return Milestones; }

    // Production Metrics
    UFUNCTION(BlueprintCallable, Category = "Metrics")
    int32 GetActiveAgentCount() const;

    UFUNCTION(BlueprintCallable, Category = "Metrics")
    int32 GetBlockedAgentCount() const;

    UFUNCTION(BlueprintCallable, Category = "Metrics")
    float GetOverallProductionProgress() const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Production")
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Production")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Production")
    TArray<FDir_MilestoneProgress> Milestones;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Production")
    FDateTime ProductionStartTime;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Production")
    int32 CurrentCycleNumber;

private:
    void SetupDefaultAgents();
    void SetupMilestone1Requirements();
    void ValidateProductionState();
    void LogProductionMetrics();
};