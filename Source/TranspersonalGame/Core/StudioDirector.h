#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "../SharedTypes.h"
#include "StudioDirector.generated.h"

/**
 * Studio Director - Central coordination system for the 19-agent production pipeline
 * Manages Milestone 1 execution: playable prototype with character movement and dinosaurs
 * Tracks production state across all agents and ensures deliverable completion
 */

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    int32 AgentID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    bool bIsCompleted = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    float Priority = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    TArray<FString> Dependencies;

    FDir_AgentTask()
    {
        AgentID = 0;
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        bIsCompleted = false;
        Priority = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct FDir_MilestoneProgress
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FDir_AgentTask> RequiredTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    int32 CompletedTasks = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float CompletionPercentage = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bIsMilestoneComplete = false;

    FDir_MilestoneProgress()
    {
        MilestoneName = TEXT("");
        CompletedTasks = 0;
        CompletionPercentage = 0.0f;
        bIsMilestoneComplete = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADir_StudioDirector : public AActor
{
    GENERATED_BODY()

public:
    ADir_StudioDirector();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    // Milestone 1 tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FDir_MilestoneProgress Milestone1Progress;

    // Current production cycle
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString CurrentCycleID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 CurrentAgentID = 1;

    // Production state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    bool bProductionActive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float BudgetUsed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float BudgetLimit = 150.0f;

public:
    // Core coordination functions
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeMilestone1Tasks();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateTaskProgress(int32 AgentID, const FString& TaskName, bool bCompleted);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CalculateMilestoneProgress();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool CanAgentProceed(int32 AgentID) const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentTask> GetPendingTasks() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_AgentTask GetNextCriticalTask() const;

    // Map state verification
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void VerifyMinPlayableMapState();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool IsPlayablePrototypeReady() const;

    // Production pipeline control
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void StartProductionCycle(const FString& CycleID);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CompleteProductionCycle();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void EmergencyStop(const FString& Reason);

    // Debug and monitoring
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void LogProductionState();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void ResetMilestone1Progress();

private:
    void SetupMilestone1Requirements();
    void ValidateAgentDependencies();
    bool CheckCriticalPath() const;
};