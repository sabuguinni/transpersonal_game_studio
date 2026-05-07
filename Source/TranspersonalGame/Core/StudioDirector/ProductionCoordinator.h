#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "UObject/NoExportTypes.h"
#include "../../SharedTypes.h"
#include "ProductionCoordinator.generated.h"

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle = 0,
    Working = 1,
    Completed = 2,
    Blocked = 3,
    Failed = 4
};

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    Architecture = 0,
    CoreSystems = 1,
    WorldGeneration = 2,
    CharacterCreation = 3,
    GameplayImplementation = 4,
    Polish = 5
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
    TArray<FString> RequiredDeliverables;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    int32 Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FDateTime Deadline;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString SuccessCriteria;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Status = EDir_AgentStatus::Idle;
        Priority = 0;
        Deadline = FDateTime::Now();
        SuccessCriteria = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct FDir_MilestoneProgress
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FString> Requirements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<bool> CompletionStatus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float ProgressPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bIsBlocking;

    FDir_MilestoneProgress()
    {
        MilestoneName = TEXT("");
        ProgressPercentage = 0.0f;
        bIsBlocking = false;
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalHeaderFiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalCppFiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CompilationErrors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 ActiveAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float WorldSizeKm2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 DinosaurActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    bool bIsPlayable;

    FDir_ProductionMetrics()
    {
        TotalHeaderFiles = 0;
        TotalCppFiles = 0;
        CompilationErrors = 0;
        ActiveAgents = 0;
        WorldSizeKm2 = 0.0f;
        DinosaurActorCount = 0;
        bIsPlayable = false;
    }
};

/**
 * Production Coordinator for Studio Director
 * Manages agent tasks, tracks milestones, and coordinates development pipeline
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDir_ProductionCoordinator : public UObject
{
    GENERATED_BODY()

public:
    UDir_ProductionCoordinator();

    // Task Management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void AssignTaskToAgent(const FString& AgentName, const FDir_AgentTask& Task);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentTask> GetTasksForAgent(const FString& AgentName) const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentTask> GetCriticalPathTasks() const;

    // Milestone Tracking
    UFUNCTION(BlueprintCallable, Category = "Production")
    void InitializeMilestone1();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateMilestoneProgress(const FString& MilestoneName, int32 RequirementIndex, bool bCompleted);

    UFUNCTION(BlueprintCallable, Category = "Production")
    float GetMilestone1Progress() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool IsMilestone1Complete() const;

    // Production Metrics
    UFUNCTION(BlueprintCallable, Category = "Production")
    FDir_ProductionMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateProductionMetrics();

    // Agent Coordination
    UFUNCTION(BlueprintCallable, Category = "Production")
    FString GetNextAgentInChain(const FString& CurrentAgent) const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool CanAgentProceed(const FString& AgentName) const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FString> GetBlockedAgents() const;

    // Critical Path Analysis
    UFUNCTION(BlueprintCallable, Category = "Production")
    void AnalyzeCriticalPath();

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FString> GetCriticalBlockers() const;

    // Production Phase Management
    UFUNCTION(BlueprintCallable, Category = "Production")
    EDir_ProductionPhase GetCurrentPhase() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AdvanceToNextPhase();

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool CanAdvancePhase() const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TMap<FString, TArray<FDir_AgentTask>> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TMap<FString, EDir_AgentStatus> AgentStatusMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_MilestoneProgress> Milestones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FDir_ProductionMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FString> CriticalBlockers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TMap<FString, FString> AgentChain;

private:
    void InitializeAgentChain();
    void ValidateTaskDependencies();
    float CalculateMilestoneProgress(const FDir_MilestoneProgress& Milestone) const;
    bool CheckAgentDependencies(const FString& AgentName) const;
};