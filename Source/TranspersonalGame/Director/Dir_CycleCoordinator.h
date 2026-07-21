#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/World.h"
#include "Dir_CycleCoordinator.generated.h"

/**
 * Production cycle coordination data for Studio Director
 */
USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Agent Task")
    int32 AgentNumber = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Agent Task")
    FString AgentName;

    UPROPERTY(BlueprintReadWrite, Category = "Agent Task")
    FString TaskDescription;

    UPROPERTY(BlueprintReadWrite, Category = "Agent Task")
    FString Priority; // "Critical", "High", "Medium", "Low"

    UPROPERTY(BlueprintReadWrite, Category = "Agent Task")
    TArray<int32> Dependencies; // Agent numbers this task depends on

    UPROPERTY(BlueprintReadWrite, Category = "Agent Task")
    bool bCompleted = false;

    UPROPERTY(BlueprintReadWrite, Category = "Agent Task")
    FString ExpectedDeliverable;

    FDir_AgentTask()
    {
        AgentNumber = 0;
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Priority = TEXT("Medium");
        bCompleted = false;
        ExpectedDeliverable = TEXT("");
    }
};

/**
 * Current production cycle status
 */
USTRUCT(BlueprintType)
struct FDir_CycleStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Cycle Status")
    FString CycleID;

    UPROPERTY(BlueprintReadWrite, Category = "Cycle Status")
    int32 TotalActors = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Cycle Status")
    int32 DinosaurCount = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Cycle Status")
    int32 CharacterCount = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Cycle Status")
    int32 EnvironmentProps = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Cycle Status")
    bool bHasPlayableCharacter = false;

    UPROPERTY(BlueprintReadWrite, Category = "Cycle Status")
    bool bHasTerrain = false;

    UPROPERTY(BlueprintReadWrite, Category = "Cycle Status")
    bool bHasLighting = false;

    UPROPERTY(BlueprintReadWrite, Category = "Cycle Status")
    FString CriticalBlockers;

    FDir_CycleStatus()
    {
        CycleID = TEXT("");
        TotalActors = 0;
        DinosaurCount = 0;
        CharacterCount = 0;
        EnvironmentProps = 0;
        bHasPlayableCharacter = false;
        bHasTerrain = false;
        bHasLighting = false;
        CriticalBlockers = TEXT("");
    }
};

/**
 * Studio Director cycle coordination system
 * Manages agent task distribution and production pipeline
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDir_CycleCoordinator : public UObject
{
    GENERATED_BODY()

public:
    UDir_CycleCoordinator();

    // Core coordination functions
    UFUNCTION(BlueprintCallable, Category = "Director")
    void InitializeCycle(const FString& CycleID);

    UFUNCTION(BlueprintCallable, Category = "Director")
    void AssignAgentTask(int32 AgentNumber, const FString& TaskDescription, const FString& Priority, const TArray<int32>& Dependencies);

    UFUNCTION(BlueprintCallable, Category = "Director")
    void CompleteAgentTask(int32 AgentNumber, const FString& DeliverableDescription);

    UFUNCTION(BlueprintCallable, Category = "Director")
    FDir_CycleStatus GetCurrentCycleStatus() const;

    UFUNCTION(BlueprintCallable, Category = "Director")
    TArray<FDir_AgentTask> GetPendingTasks() const;

    UFUNCTION(BlueprintCallable, Category = "Director")
    TArray<FDir_AgentTask> GetReadyTasks() const; // Tasks with all dependencies completed

    UFUNCTION(BlueprintCallable, Category = "Director")
    bool IsAgentReady(int32 AgentNumber) const;

    UFUNCTION(BlueprintCallable, Category = "Director")
    void UpdateMapStatus();

    UFUNCTION(BlueprintCallable, Category = "Director")
    FString GenerateNextCycleDirective() const;

    // Production milestone tracking
    UFUNCTION(BlueprintCallable, Category = "Director")
    bool IsMilestone1Complete() const; // Walk Around milestone

    UFUNCTION(BlueprintCallable, Category = "Director")
    TArray<FString> GetMilestone1Requirements() const;

    UFUNCTION(BlueprintCallable, Category = "Director")
    float GetOverallProgress() const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Director")
    FDir_CycleStatus CurrentCycle;

    UPROPERTY(BlueprintReadOnly, Category = "Director")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(BlueprintReadOnly, Category = "Director")
    TMap<int32, FString> AgentNames;

    // Helper functions
    void InitializeAgentNames();
    void UpdateActorCounts();
    bool CheckDependenciesComplete(const TArray<int32>& Dependencies) const;
    FString GetCriticalBlockers() const;
};