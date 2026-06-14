#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "ProductionDirector.generated.h"

/**
 * Studio Director Agent #1 - Production Coordination System
 * Manages the 19-agent production pipeline for the prehistoric survival game
 * Tracks milestones, coordinates agent tasks, and ensures gameplay-first development
 */

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction UMETA(DisplayName = "Pre-Production"),
    MinimalPlayable UMETA(DisplayName = "Minimal Playable Prototype"), 
    CoreGameplay UMETA(DisplayName = "Core Gameplay Systems"),
    ContentCreation UMETA(DisplayName = "Content Creation Phase"),
    Polish UMETA(DisplayName = "Polish and Optimization"),
    Release UMETA(DisplayName = "Release Preparation")
};

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Active UMETA(DisplayName = "Active"),
    Blocked UMETA(DisplayName = "Blocked"),
    Complete UMETA(DisplayName = "Complete"),
    Failed UMETA(DisplayName = "Failed")
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString Deliverables;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Status = EDir_AgentStatus::Idle;
        Priority = 0;
        EstimatedHours = 0.0f;
        Deliverables = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMilestone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FString> RequiredDeliverables;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bIsComplete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    EDir_ProductionPhase Phase;

    FDir_ProductionMilestone()
    {
        MilestoneName = TEXT("");
        Description = TEXT("");
        bIsComplete = false;
        CompletionPercentage = 0.0f;
        Phase = EDir_ProductionPhase::PreProduction;
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalActorsInScene;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 DinosaurCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CompletedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 BlockedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float OverallProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    bool bMinimalPlayableReached;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    FString CurrentBottleneck;

    FDir_ProductionMetrics()
    {
        TotalActorsInScene = 0;
        DinosaurCount = 0;
        CompletedTasks = 0;
        BlockedTasks = 0;
        OverallProgress = 0.0f;
        bMinimalPlayableReached = false;
        CurrentBottleneck = TEXT("None");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UProductionDirector : public UObject
{
    GENERATED_BODY()

public:
    UProductionDirector();

    // Core Production Management
    UFUNCTION(BlueprintCallable, Category = "Production Director")
    void InitializeProductionPipeline();

    UFUNCTION(BlueprintCallable, Category = "Production Director")
    void UpdateProductionState();

    UFUNCTION(BlueprintCallable, Category = "Production Director")
    FDir_ProductionMetrics GetCurrentMetrics() const;

    // Agent Task Management
    UFUNCTION(BlueprintCallable, Category = "Production Director")
    void AssignTaskToAgent(const FString& AgentName, const FDir_AgentTask& Task);

    UFUNCTION(BlueprintCallable, Category = "Production Director")
    void UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Production Director")
    TArray<FDir_AgentTask> GetTasksForAgent(const FString& AgentName) const;

    UFUNCTION(BlueprintCallable, Category = "Production Director")
    TArray<FDir_AgentTask> GetBlockedTasks() const;

    // Milestone Management
    UFUNCTION(BlueprintCallable, Category = "Production Director")
    void CreateMilestone(const FDir_ProductionMilestone& Milestone);

    UFUNCTION(BlueprintCallable, Category = "Production Director")
    void UpdateMilestoneProgress(const FString& MilestoneName, float Progress);

    UFUNCTION(BlueprintCallable, Category = "Production Director")
    bool IsMilestoneComplete(const FString& MilestoneName) const;

    UFUNCTION(BlueprintCallable, Category = "Production Director")
    FDir_ProductionMilestone GetCurrentMilestone() const;

    // Gameplay-First Validation
    UFUNCTION(BlueprintCallable, Category = "Production Director")
    bool ValidateMinimalPlayablePrototype() const;

    UFUNCTION(BlueprintCallable, Category = "Production Director")
    TArray<FString> GetMissingPlayableElements() const;

    // Agent Coordination
    UFUNCTION(BlueprintCallable, Category = "Production Director")
    FString GetNextAgentToActivate() const;

    UFUNCTION(BlueprintCallable, Category = "Production Director")
    void ResolveAgentConflict(const FString& Agent1, const FString& Agent2, const FString& Resolution);

    // Production Reporting
    UFUNCTION(BlueprintCallable, Category = "Production Director")
    FString GenerateProductionReport() const;

    UFUNCTION(BlueprintCallable, Category = "Production Director")
    void LogProductionEvent(const FString& Event, const FString& Details);

protected:
    // Production State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production State")
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production State")
    TArray<FDir_AgentTask> AllTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production State")
    TArray<FDir_ProductionMilestone> Milestones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production State")
    FDir_ProductionMetrics CurrentMetrics;

    // Agent Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Management")
    TMap<FString, EDir_AgentStatus> AgentStatusMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Management")
    TArray<FString> AgentExecutionOrder;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Management")
    FString CurrentActiveAgent;

    // Production Events Log
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Events")
    TArray<FString> ProductionLog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Events")
    int32 MaxLogEntries;

private:
    // Internal helper functions
    void InitializeAgentOrder();
    void InitializeMilestones();
    void AnalyzeSceneState();
    void UpdateAgentDependencies();
    bool CheckPlayablePrototypeRequirements() const;
    void PrioritizeTasksByGameplayImpact();
};