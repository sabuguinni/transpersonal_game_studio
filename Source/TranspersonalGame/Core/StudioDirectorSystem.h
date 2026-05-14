#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "StudioDirectorSystem.generated.h"

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle,
    Working,
    Completed,
    Failed,
    Blocked
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
    FString OutputPath;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Status = EDir_AgentStatus::Idle;
        Priority = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionCycle
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Cycle")
    FString CycleID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Cycle")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Cycle")
    float StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Cycle")
    float EndTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Cycle")
    bool bCycleCompleted;

    FDir_ProductionCycle()
    {
        CycleID = TEXT("");
        StartTime = 0.0f;
        EndTime = 0.0f;
        bCycleCompleted = false;
    }
};

/**
 * Studio Director System - Coordinates all 19 AI agents in the production pipeline
 * Manages task distribution, dependency tracking, and production cycle coordination
 */
UCLASS()
class TRANSPERSONALGAME_API UStudioDirectorSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UStudioDirectorSystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Production Cycle Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void StartProductionCycle(const FString& CycleID);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CompleteProductionCycle();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool IsProductionCycleActive() const;

    // Agent Task Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription, float Priority = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    EDir_AgentStatus GetAgentStatus(const FString& AgentName) const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentTask> GetAllAgentTasks() const;

    // Dependency Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AddTaskDependency(const FString& AgentName, const FString& DependencyAgent);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool CanAgentStart(const FString& AgentName) const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FString> GetBlockedAgents() const;

    // Production Monitoring
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    float GetCycleProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    int32 GetCompletedTaskCount() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    int32 GetTotalTaskCount() const;

    // Milestone Tracking
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void SetMilestone(const FString& MilestoneName, bool bCompleted);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool IsMilestoneCompleted(const FString& MilestoneName) const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FString> GetCompletedMilestones() const;

protected:
    UPROPERTY()
    FDir_ProductionCycle CurrentCycle;

    UPROPERTY()
    TMap<FString, bool> Milestones;

    UPROPERTY()
    TArray<FDir_ProductionCycle> CycleHistory;

private:
    void InitializeAgentTasks();
    void ValidateTaskDependencies();
    FDir_AgentTask* FindAgentTask(const FString& AgentName);
    const FDir_AgentTask* FindAgentTask(const FString& AgentName) const;
};