#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Dir_CycleCoordinator.generated.h"

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
    float Priority = 1.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Agent Task")
    bool bIsCompleted = false;

    UPROPERTY(BlueprintReadWrite, Category = "Agent Task")
    FString ExpectedDeliverable;

    FDir_AgentTask()
    {
        AgentNumber = 0;
        AgentName = TEXT("Unknown");
        TaskDescription = TEXT("");
        Priority = 1.0f;
        bIsCompleted = false;
        ExpectedDeliverable = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct FDir_CycleStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Cycle Status")
    FString CycleID;

    UPROPERTY(BlueprintReadWrite, Category = "Cycle Status")
    int32 TotalAgents = 20;

    UPROPERTY(BlueprintReadWrite, Category = "Cycle Status")
    int32 CompletedTasks = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Cycle Status")
    float CycleProgress = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Cycle Status")
    bool bBridgeOperational = true;

    UPROPERTY(BlueprintReadWrite, Category = "Cycle Status")
    TArray<FDir_AgentTask> ActiveTasks;

    FDir_CycleStatus()
    {
        CycleID = TEXT("UNKNOWN");
        TotalAgents = 20;
        CompletedTasks = 0;
        CycleProgress = 0.0f;
        bBridgeOperational = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADir_CycleCoordinator : public AActor
{
    GENERATED_BODY()

public:
    ADir_CycleCoordinator();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(BlueprintReadWrite, Category = "Coordination")
    FDir_CycleStatus CurrentCycleStatus;

    UPROPERTY(BlueprintReadWrite, Category = "Coordination")
    TArray<FDir_AgentTask> PendingTasks;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Coordination")
    void InitializeCycle(const FString& CycleID);

    UFUNCTION(BlueprintCallable, Category = "Coordination")
    void AssignTaskToAgent(int32 AgentNumber, const FString& TaskDescription, const FString& ExpectedDeliverable, float Priority = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Coordination")
    void MarkTaskCompleted(int32 AgentNumber);

    UFUNCTION(BlueprintCallable, Category = "Coordination")
    float GetCycleProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Coordination")
    bool IsCycleComplete() const;

    UFUNCTION(BlueprintCallable, Category = "Coordination")
    void ValidateBridgeStatus();

    UFUNCTION(BlueprintCallable, Category = "Coordination")
    FString GenerateProductionReport() const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugPrintCycleStatus();

private:
    void UpdateCycleProgress();
    void CheckAgentDeadlines();
    void LogCoordinationEvent(const FString& Event);
};