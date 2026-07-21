#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "UObject/NoExportTypes.h"
#include "Dir_ProductionCoordinator.generated.h"

UENUM(BlueprintType)
enum class EDir_ProductionMilestone : uint8
{
    None = 0,
    WalkAround = 1,
    BasicSurvival = 2,
    DinosaurEncounters = 3,
    FullGameplay = 4
};

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle = 0,
    Working = 1,
    Blocked = 2,
    Complete = 3
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    int32 Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    float EstimatedHours;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    TArray<FString> Dependencies;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Status = EDir_AgentStatus::Idle;
        Priority = 0;
        EstimatedHours = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 TotalAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 ActiveAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 CompletedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 BlockedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    EDir_ProductionMilestone CurrentMilestone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    float MilestoneProgress;

    FDir_ProductionMetrics()
    {
        TotalAgents = 19;
        ActiveAgents = 0;
        CompletedTasks = 0;
        BlockedTasks = 0;
        CurrentMilestone = EDir_ProductionMilestone::WalkAround;
        MilestoneProgress = 0.0f;
    }
};

/**
 * Studio Director Production Coordination System
 * Manages the 19-agent development pipeline for the prehistoric survival game
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDir_ProductionCoordinator : public UObject
{
    GENERATED_BODY()

public:
    UDir_ProductionCoordinator();

    // === MILESTONE MANAGEMENT ===
    UFUNCTION(BlueprintCallable, Category = "Production Coordination")
    void InitializeProductionPipeline();

    UFUNCTION(BlueprintCallable, Category = "Production Coordination")
    bool ValidateMilestone(EDir_ProductionMilestone Milestone);

    UFUNCTION(BlueprintCallable, Category = "Production Coordination")
    void AdvanceToNextMilestone();

    UFUNCTION(BlueprintCallable, Category = "Production Coordination")
    float CalculateMilestoneProgress();

    // === AGENT TASK MANAGEMENT ===
    UFUNCTION(BlueprintCallable, Category = "Production Coordination")
    void AssignTaskToAgent(const FString& AgentName, const FDir_AgentTask& Task);

    UFUNCTION(BlueprintCallable, Category = "Production Coordination")
    void UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Production Coordination")
    TArray<FDir_AgentTask> GetTasksByPriority();

    UFUNCTION(BlueprintCallable, Category = "Production Coordination")
    TArray<FString> GetBlockedAgents();

    // === PRODUCTION METRICS ===
    UFUNCTION(BlueprintCallable, Category = "Production Coordination")
    FDir_ProductionMetrics GetProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Production Coordination")
    void GenerateProductionReport();

    // === COORDINATION VALIDATION ===
    UFUNCTION(BlueprintCallable, Category = "Production Coordination")
    bool ValidateGameplayElements();

    UFUNCTION(BlueprintCallable, Category = "Production Coordination")
    TArray<FString> GetMissingCriticalSystems();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production State")
    EDir_ProductionMilestone CurrentMilestone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production State")
    TMap<FString, FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production State")
    TMap<FString, EDir_AgentStatus> AgentStatusMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production State")
    FDir_ProductionMetrics ProductionMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production State")
    TArray<FString> CriticalSystemsChecklist;

private:
    void InitializeAgentTasks();
    void InitializeCriticalSystems();
    bool CheckMilestone1Requirements();
    bool CheckMilestone2Requirements();
    bool CheckMilestone3Requirements();
};