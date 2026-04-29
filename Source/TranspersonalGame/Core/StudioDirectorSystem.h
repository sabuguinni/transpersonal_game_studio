#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "StudioDirectorSystem.generated.h"

/**
 * Studio Director System - Central coordination for all game agents and production pipeline
 * Manages agent task distribution, milestone tracking, and production quality assurance
 */

UENUM(BlueprintType)
enum class EDir_AgentPriority : uint8
{
    CRITICAL    UMETA(DisplayName = "Critical"),
    HIGH        UMETA(DisplayName = "High"),
    MEDIUM      UMETA(DisplayName = "Medium"),
    LOW         UMETA(DisplayName = "Low"),
    BLOCKED     UMETA(DisplayName = "Blocked")
};

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    IDLE        UMETA(DisplayName = "Idle"),
    WORKING     UMETA(DisplayName = "Working"),
    COMPLETED   UMETA(DisplayName = "Completed"),
    FAILED      UMETA(DisplayName = "Failed"),
    WAITING     UMETA(DisplayName = "Waiting for Dependencies")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    EDir_AgentPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    int32 CycleAssigned;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    int32 CycleCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    TArray<FString> Deliverables;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Priority = EDir_AgentPriority::MEDIUM;
        Status = EDir_AgentStatus::IDLE;
        CycleAssigned = 0;
        CycleCompleted = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_ProductionMilestone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    int32 TargetCycle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FString> RequiredDeliverables;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float CompletionPercentage;

    FDir_ProductionMilestone()
    {
        MilestoneName = TEXT("");
        Description = TEXT("");
        TargetCycle = 0;
        bIsCompleted = false;
        CompletionPercentage = 0.0f;
    }
};

/**
 * Studio Director Subsystem - Game Instance level coordination system
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UStudioDirectorSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Agent Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void RegisterAgent(const FString& AgentName, EDir_AgentPriority DefaultPriority);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AssignTask(const FString& AgentName, const FString& TaskDescription, EDir_AgentPriority Priority, const TArray<FString>& Dependencies);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CompleteTask(const FString& AgentName, const TArray<FString>& Deliverables);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateTaskStatus(const FString& AgentName, EDir_AgentStatus NewStatus);

    // Milestone Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CreateMilestone(const FString& MilestoneName, const FString& Description, int32 TargetCycle, const TArray<FString>& RequiredDeliverables);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateMilestoneProgress(const FString& MilestoneName, float ProgressPercentage);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool IsMilestoneCompleted(const FString& MilestoneName);

    // Production Coordination
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void StartProductionCycle(int32 CycleNumber);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void EndProductionCycle();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentTask> GetPendingTasks();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentTask> GetTasksByPriority(EDir_AgentPriority Priority);

    // Quality Assurance
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool ValidateAgentDeliverables(const FString& AgentName);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void BlockAgent(const FString& AgentName, const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UnblockAgent(const FString& AgentName);

    // Reporting
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FString GenerateProductionReport();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FString GenerateMilestoneReport();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Studio Director")
    TArray<FDir_AgentTask> ActiveTasks;

    UPROPERTY(BlueprintReadOnly, Category = "Studio Director")
    TArray<FDir_ProductionMilestone> ProductionMilestones;

    UPROPERTY(BlueprintReadOnly, Category = "Studio Director")
    int32 CurrentCycle;

    UPROPERTY(BlueprintReadOnly, Category = "Studio Director")
    TMap<FString, EDir_AgentStatus> AgentStatusMap;

    UPROPERTY(BlueprintReadOnly, Category = "Studio Director")
    TArray<FString> BlockedAgents;

private:
    void InitializeDefaultMilestones();
    void InitializeAgentRegistry();
    bool CheckTaskDependencies(const FDir_AgentTask& Task);
    void UpdateMilestoneCompletion();
};

#include "StudioDirectorSystem.generated.h"