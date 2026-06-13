#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Engine.h"
#include "ProductionCoordinator.generated.h"

/**
 * Studio Director's production coordination system
 * Manages agent task assignments and milestone tracking
 */

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction,
    MinimumViablePrototype,
    CoreGameplay,
    ContentCreation,
    Polish,
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    int32 AgentID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString AgentName = "";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString TaskDescription = "";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_AgentStatus Status = EDir_AgentStatus::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float Priority = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TArray<int32> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FDateTime AssignedTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FDateTime CompletionTime;

    FDir_AgentTask()
    {
        AssignedTime = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalCycles = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CompletedTasks = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 FailedTasks = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float AverageTaskTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float BudgetUsed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float BudgetLimit = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    TArray<FString> CriticalIssues;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDir_ProductionCoordinator : public UObject
{
    GENERATED_BODY()

public:
    UDir_ProductionCoordinator();

    // Production phase management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void SetProductionPhase(EDir_ProductionPhase NewPhase);

    UFUNCTION(BlueprintPure, Category = "Production")
    EDir_ProductionPhase GetCurrentPhase() const { return CurrentPhase; }

    // Agent task management
    UFUNCTION(BlueprintCallable, Category = "Tasks")
    void AssignTask(int32 AgentID, const FString& AgentName, const FString& TaskDescription, float Priority = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Tasks")
    void CompleteTask(int32 AgentID, bool bSuccess = true);

    UFUNCTION(BlueprintCallable, Category = "Tasks")
    void BlockTask(int32 AgentID, const FString& Reason);

    UFUNCTION(BlueprintPure, Category = "Tasks")
    TArray<FDir_AgentTask> GetActiveTasks() const;

    UFUNCTION(BlueprintPure, Category = "Tasks")
    FDir_AgentTask GetTaskByAgent(int32 AgentID) const;

    // Production metrics
    UFUNCTION(BlueprintCallable, Category = "Metrics")
    void UpdateMetrics();

    UFUNCTION(BlueprintPure, Category = "Metrics")
    FDir_ProductionMetrics GetCurrentMetrics() const { return ProductionMetrics; }

    UFUNCTION(BlueprintCallable, Category = "Metrics")
    void AddCriticalIssue(const FString& Issue);

    // Milestone tracking
    UFUNCTION(BlueprintCallable, Category = "Milestones")
    void CheckMilestone1Progress();

    UFUNCTION(BlueprintPure, Category = "Milestones")
    float GetMilestone1Completion() const { return Milestone1Progress; }

    // Debug and logging
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void LogProductionState();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void ResetProduction();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    EDir_ProductionPhase CurrentPhase = EDir_ProductionPhase::MinimumViablePrototype;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tasks")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    FDir_ProductionMetrics ProductionMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestones")
    float Milestone1Progress = 0.0f;

    // Internal methods
    void InitializeAgents();
    void CalculateTaskMetrics();
    bool ValidateTaskDependencies(int32 AgentID) const;
};