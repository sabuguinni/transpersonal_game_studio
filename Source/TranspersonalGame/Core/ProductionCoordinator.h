#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "ProductionCoordinator.generated.h"

/**
 * Studio Director's Production Coordination System
 * Tracks agent progress, milestone completion, and production bottlenecks
 * Ensures all 19 agents work towards the same playable prototype goals
 */

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString Deliverable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    EDir_Priority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    float ProgressPercent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    bool bIsBlocked;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString BlockerReason;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Deliverable = TEXT("");
        Priority = EDir_Priority::Medium;
        ProgressPercent = 0.0f;
        bIsBlocked = false;
        BlockerReason = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_MilestoneStatus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float CompletionPercent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    int32 CriticalBlockers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    int32 AgentsReady;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString EstimatedCompletion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FDir_AgentTask> AgentTasks;

    FDir_MilestoneStatus()
    {
        MilestoneName = TEXT("Milestone 1 - Walk Around");
        CompletionPercent = 25.0f;
        CriticalBlockers = 0;
        AgentsReady = 8;
        EstimatedCompletion = TEXT("2 cycles");
    }
};

/**
 * Production Coordinator - Studio Director's command and control system
 * Manages the 19-agent production pipeline for the playable prototype
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDir_ProductionCoordinator : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UDir_ProductionCoordinator();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Production coordination functions
    UFUNCTION(BlueprintCallable, Category = "Production Coordinator")
    void InitializeMilestone1();

    UFUNCTION(BlueprintCallable, Category = "Production Coordinator")
    void UpdateAgentProgress(const FString& AgentName, float ProgressPercent);

    UFUNCTION(BlueprintCallable, Category = "Production Coordinator")
    void ReportAgentBlocker(const FString& AgentName, const FString& BlockerReason);

    UFUNCTION(BlueprintCallable, Category = "Production Coordinator")
    void ClearAgentBlocker(const FString& AgentName);

    UFUNCTION(BlueprintCallable, Category = "Production Coordinator")
    FDir_MilestoneStatus GetCurrentMilestoneStatus() const;

    UFUNCTION(BlueprintCallable, Category = "Production Coordinator")
    TArray<FDir_AgentTask> GetCriticalTasks() const;

    UFUNCTION(BlueprintCallable, Category = "Production Coordinator")
    TArray<FDir_AgentTask> GetBlockedTasks() const;

    UFUNCTION(BlueprintCallable, Category = "Production Coordinator")
    float CalculateOverallProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Production Coordinator", CallInEditor = true)
    void VerifyLevelState();

    UFUNCTION(BlueprintCallable, Category = "Production Coordinator", CallInEditor = true)
    void CleanupDuplicateActors();

    UFUNCTION(BlueprintCallable, Category = "Production Coordinator")
    void LogProductionStatus() const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production State")
    FDir_MilestoneStatus CurrentMilestone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production State")
    int32 CurrentCycle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production State")
    float ProductionBudgetUsed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production State")
    float ProductionBudgetLimit;

private:
    void SetupMilestone1Tasks();
    void ValidateGameModeSetup();
    void ValidateCharacterSetup();
    void ValidateLevelActors();
    FDir_AgentTask* FindAgentTask(const FString& AgentName);
};