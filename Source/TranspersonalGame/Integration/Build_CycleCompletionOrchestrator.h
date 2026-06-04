#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Build_CycleCompletionOrchestrator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_CycleMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Metrics")
    FString CycleID;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Metrics")
    int32 AgentNumber;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Metrics")
    int32 ToolCallsExecuted;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Metrics")
    int32 FilesWritten;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Metrics")
    int32 UE5CommandsExecuted;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Metrics")
    bool bCycleCompleted;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Metrics")
    FString CompletionTimestamp;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Metrics")
    float ExecutionTimeSeconds;

    FBuild_CycleMetrics()
    {
        CycleID = TEXT("");
        AgentNumber = 0;
        ToolCallsExecuted = 0;
        FilesWritten = 0;
        UE5CommandsExecuted = 0;
        bCycleCompleted = false;
        CompletionTimestamp = TEXT("");
        ExecutionTimeSeconds = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ChainStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Chain Status")
    TArray<FBuild_CycleMetrics> AgentCycles;

    UPROPERTY(BlueprintReadOnly, Category = "Chain Status")
    int32 CompletedAgents;

    UPROPERTY(BlueprintReadOnly, Category = "Chain Status")
    int32 TotalAgents;

    UPROPERTY(BlueprintReadOnly, Category = "Chain Status")
    bool bChainCompleted;

    UPROPERTY(BlueprintReadOnly, Category = "Chain Status")
    FString ChainStartTime;

    UPROPERTY(BlueprintReadOnly, Category = "Chain Status")
    FString ChainEndTime;

    FBuild_ChainStatus()
    {
        CompletedAgents = 0;
        TotalAgents = 19;
        bChainCompleted = false;
        ChainStartTime = TEXT("");
        ChainEndTime = TEXT("");
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UBuild_CycleCompletionOrchestrator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cycle Orchestration")
    FBuild_ChainStatus CurrentChainStatus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cycle Orchestration")
    bool bAutoTrackCycles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cycle Orchestration")
    FString CurrentCycleID;

public:
    UFUNCTION(BlueprintCallable, Category = "Cycle Orchestration")
    void StartNewCycle(const FString& CycleID);

    UFUNCTION(BlueprintCallable, Category = "Cycle Orchestration")
    void RecordAgentCompletion(int32 AgentNumber, int32 ToolCalls, int32 FilesWritten, int32 UE5Commands);

    UFUNCTION(BlueprintCallable, Category = "Cycle Orchestration")
    void CompleteCycle();

    UFUNCTION(BlueprintCallable, Category = "Cycle Orchestration")
    FBuild_ChainStatus GetChainStatus() const;

    UFUNCTION(BlueprintCallable, Category = "Cycle Orchestration")
    bool IsChainCompleted() const;

    UFUNCTION(BlueprintCallable, Category = "Cycle Orchestration")
    void ResetChain();

    UFUNCTION(BlueprintCallable, Category = "Cycle Orchestration")
    void GenerateCompletionReport();

    UFUNCTION(BlueprintCallable, Category = "Cycle Orchestration")
    float GetChainCompletionPercentage() const;

    UFUNCTION(BlueprintCallable, Category = "Cycle Orchestration")
    FBuild_CycleMetrics GetAgentMetrics(int32 AgentNumber) const;

    UFUNCTION(BlueprintCallable, Category = "Cycle Orchestration")
    void ValidateChainIntegrity();

private:
    void LogCycleProgress();
    void CheckChainCompletion();
    void NotifyChainCompletion();

    float ChainStartTimeSeconds;
    bool bChainInProgress;
};