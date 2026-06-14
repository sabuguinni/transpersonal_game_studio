#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Dir_ProductionManager.generated.h"

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle,
    Working,
    Completed,
    Blocked,
    Error
};

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction,
    CoreSystems,
    WorldBuilding,
    CharacterDevelopment,
    GameplayImplementation,
    Polish,
    Testing
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
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FDateTime StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FDateTime EstimatedCompletion;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Status = EDir_AgentStatus::Idle;
        Priority = 1.0f;
        StartTime = FDateTime::Now();
        EstimatedCompletion = FDateTime::Now();
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
    float OverallProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    EDir_ProductionPhase CurrentPhase;

    FDir_ProductionMetrics()
    {
        TotalAgents = 19;
        ActiveAgents = 0;
        CompletedTasks = 0;
        BlockedTasks = 0;
        OverallProgress = 0.0f;
        CurrentPhase = EDir_ProductionPhase::PreProduction;
    }
};

/**
 * Production Manager - Studio Director's coordination system for managing 19 specialized agents
 * Tracks agent tasks, dependencies, progress metrics, and production pipeline flow
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDir_ProductionManager : public UObject
{
    GENERATED_BODY()

public:
    UDir_ProductionManager();

    // Agent Management
    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void InitializeAgentPipeline();

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription, float Priority = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    bool CanAgentStartTask(const FString& AgentName) const;

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    TArray<FString> GetBlockedAgents() const;

    // Production Flow
    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void AdvanceProductionPhase();

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    FDir_ProductionMetrics GetProductionMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    float CalculateOverallProgress() const;

    // Task Dependencies
    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void AddTaskDependency(const FString& AgentName, const FString& DependentAgent);

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    bool AreDependenciesMet(const FString& AgentName) const;

    // Milestone Tracking
    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void SetMilestone(const FString& MilestoneName, const TArray<FString>& RequiredAgents);

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    bool IsMilestoneComplete(const FString& MilestoneName) const;

    // Debug and Monitoring
    UFUNCTION(BlueprintCallable, Category = "Production Management", CallInEditor)
    void LogProductionStatus();

    UFUNCTION(BlueprintCallable, Category = "Production Management", CallInEditor)
    void ResetAllAgentTasks();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Data")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Data")
    FDir_ProductionMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Data")
    TMap<FString, TArray<FString>> TaskDependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Data")
    TMap<FString, TArray<FString>> ProductionMilestones;

    // Agent Definitions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Configuration")
    TArray<FString> AgentNames;

private:
    void InitializeAgentNames();
    FDir_AgentTask* FindAgentTask(const FString& AgentName);
    const FDir_AgentTask* FindAgentTask(const FString& AgentName) const;
};