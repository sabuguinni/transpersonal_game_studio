#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "Dir_ProductionCoordinator.generated.h"

UENUM(BlueprintType)
enum class EDir_MilestoneStatus : uint8
{
    NotStarted      UMETA(DisplayName = "Not Started"),
    InProgress      UMETA(DisplayName = "In Progress"), 
    Completed       UMETA(DisplayName = "Completed"),
    Blocked         UMETA(DisplayName = "Blocked"),
    Failed          UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EDir_AgentPriority : uint8
{
    Critical        UMETA(DisplayName = "Critical"),
    High            UMETA(DisplayName = "High"),
    Normal          UMETA(DisplayName = "Normal"),
    Low             UMETA(DisplayName = "Low"),
    Deferred        UMETA(DisplayName = "Deferred")
};

USTRUCT(BlueprintType)
struct FDir_MilestoneTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString TaskName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_MilestoneStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_AgentPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    int32 AssignedAgentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString RequiredDeliverables;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float EstimatedCycles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float CompletionPercentage;

    FDir_MilestoneTask()
    {
        TaskName = TEXT("");
        Description = TEXT("");
        Status = EDir_MilestoneStatus::NotStarted;
        Priority = EDir_AgentPriority::Normal;
        AssignedAgentID = 0;
        RequiredDeliverables = TEXT("");
        EstimatedCycles = 1.0f;
        CompletionPercentage = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FDir_AgentStatus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    int32 AgentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString CurrentTask;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    float LastCycleProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    TArray<FString> BlockingIssues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    int32 FilesCreatedThisCycle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    int32 UE5CommandsExecuted;

    FDir_AgentStatus()
    {
        AgentID = 0;
        AgentName = TEXT("");
        bIsActive = false;
        CurrentTask = TEXT("");
        LastCycleProgress = 0.0f;
        FilesCreatedThisCycle = 0;
        UE5CommandsExecuted = 0;
    }
};

/**
 * Production Coordinator - Studio Director's central coordination system
 * Tracks Milestone 1 progress and coordinates all 18 agents
 * Ensures gameplay-first approach and prevents architectural drift
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDir_ProductionCoordinator : public UObject
{
    GENERATED_BODY()

public:
    UDir_ProductionCoordinator();

    // Milestone 1 tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone 1")
    TArray<FDir_MilestoneTask> Milestone1Tasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone 1")
    float Milestone1Progress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone 1")
    EDir_MilestoneStatus Milestone1Status;

    // Agent coordination
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agents")
    TArray<FDir_AgentStatus> AgentStatuses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agents")
    int32 CurrentActiveAgent;

    // Critical blockers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blockers")
    TArray<FString> CriticalBlockers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blockers")
    bool bCompilationBlocked;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blockers")
    int32 HeadersWithoutCpp;

    // Production metrics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalFilesCreated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalUE5Commands;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float CycleStartTime;

    // Core coordination functions
    UFUNCTION(BlueprintCallable, Category = "Coordination")
    void InitializeMilestone1Tasks();

    UFUNCTION(BlueprintCallable, Category = "Coordination")
    void UpdateAgentStatus(int32 AgentID, const FString& TaskName, float Progress);

    UFUNCTION(BlueprintCallable, Category = "Coordination")
    void AddCriticalBlocker(const FString& BlockerDescription);

    UFUNCTION(BlueprintCallable, Category = "Coordination")
    void ResolveCriticalBlocker(const FString& BlockerDescription);

    UFUNCTION(BlueprintCallable, Category = "Coordination")
    bool IsAgentBlocked(int32 AgentID) const;

    UFUNCTION(BlueprintCallable, Category = "Coordination")
    TArray<FString> GetNextAgentTasks(int32 AgentID) const;

    UFUNCTION(BlueprintCallable, Category = "Coordination")
    float CalculateMilestone1Progress() const;

    UFUNCTION(BlueprintCallable, Category = "Coordination")
    void ValidatePlayablePrototype();

    // Quality gates
    UFUNCTION(BlueprintCallable, Category = "Quality")
    bool CanAgentProceed(int32 AgentID) const;

    UFUNCTION(BlueprintCallable, Category = "Quality")
    void EnforceGameplayFirst();

    UFUNCTION(BlueprintCallable, Category = "Quality")
    void PreventArchitecturalDrift();

protected:
    void SetupAgentHierarchy();
    void DefineAgentDependencies();
    void EstablishQualityGates();
};