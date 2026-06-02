#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/World.h"
#include "Dir_ProductionManager.generated.h"

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction   UMETA(DisplayName = "Pre-Production"),
    Production      UMETA(DisplayName = "Production"),
    Alpha          UMETA(DisplayName = "Alpha"),
    Beta           UMETA(DisplayName = "Beta"),
    Release        UMETA(DisplayName = "Release")
};

UENUM(BlueprintType)
enum class EDir_AgentPriority : uint8
{
    Critical       UMETA(DisplayName = "Critical"),
    High          UMETA(DisplayName = "High"),
    Medium        UMETA(DisplayName = "Medium"),
    Low           UMETA(DisplayName = "Low"),
    Blocked       UMETA(DisplayName = "Blocked")
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    int32 AgentNumber;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_AgentPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    bool bCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float EstimatedHours;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TArray<int32> Dependencies;

    FDir_AgentTask()
    {
        AgentNumber = 0;
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Priority = EDir_AgentPriority::Medium;
        bCompleted = false;
        EstimatedHours = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 DinosaurCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CharacterCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TerrainActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CriticalIssues;

    FDir_ProductionMetrics()
    {
        TotalActors = 0;
        DinosaurCount = 0;
        CharacterCount = 0;
        TerrainActors = 0;
        CompletionPercentage = 0.0f;
        CriticalIssues = 0;
    }
};

/**
 * Studio Director Production Management System
 * Coordinates all 19 agents and tracks production progress
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDir_ProductionManager : public UObject
{
    GENERATED_BODY()

public:
    UDir_ProductionManager();

    // Production Phase Management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void SetProductionPhase(EDir_ProductionPhase NewPhase);

    UFUNCTION(BlueprintPure, Category = "Production")
    EDir_ProductionPhase GetCurrentPhase() const { return CurrentPhase; }

    // Agent Task Management
    UFUNCTION(BlueprintCallable, Category = "Agents")
    void AssignTaskToAgent(int32 AgentNumber, const FString& TaskDescription, EDir_AgentPriority Priority, float EstimatedHours);

    UFUNCTION(BlueprintCallable, Category = "Agents")
    void CompleteAgentTask(int32 AgentNumber);

    UFUNCTION(BlueprintCallable, Category = "Agents")
    TArray<FDir_AgentTask> GetPendingTasks();

    UFUNCTION(BlueprintCallable, Category = "Agents")
    TArray<FDir_AgentTask> GetTasksForAgent(int32 AgentNumber);

    // Production Metrics
    UFUNCTION(BlueprintCallable, Category = "Metrics")
    FDir_ProductionMetrics CalculateProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Metrics")
    void UpdateMetrics();

    // Critical Path Analysis
    UFUNCTION(BlueprintCallable, Category = "Analysis")
    TArray<int32> GetCriticalPathAgents();

    UFUNCTION(BlueprintCallable, Category = "Analysis")
    bool IsAgentBlocked(int32 AgentNumber);

    // Milestone Tracking
    UFUNCTION(BlueprintCallable, Category = "Milestones")
    void SetMilestone(const FString& MilestoneName, float TargetCompletion);

    UFUNCTION(BlueprintPure, Category = "Milestones")
    float GetMilestoneProgress(const FString& MilestoneName);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tasks")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    FDir_ProductionMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestones")
    TMap<FString, float> Milestones;

private:
    void InitializeAgentTasks();
    void ValidateTaskDependencies();
    int32 CountActorsByType(const FString& TypeKeyword);
};