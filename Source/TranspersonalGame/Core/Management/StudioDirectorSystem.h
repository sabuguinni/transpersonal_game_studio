#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "StudioDirectorSystem.generated.h"

USTRUCT(BlueprintType)
struct FDir_ProductionTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString TaskName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 AgentNumber;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    bool bCompleted;

    FDir_ProductionTask()
    {
        TaskName = TEXT("");
        AgentNumber = 0;
        Priority = TEXT("Medium");
        bCompleted = false;
    }
};

USTRUCT(BlueprintType)
struct FDir_AgentStatus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    int32 AgentNumber;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString CurrentTask;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    bool bActive;

    FDir_AgentStatus()
    {
        AgentNumber = 0;
        AgentName = TEXT("");
        CurrentTask = TEXT("");
        CompletionPercentage = 0.0f;
        bActive = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AStudioDirectorSystem : public AActor
{
    GENERATED_BODY()

public:
    AStudioDirectorSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_ProductionTask> ActiveTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_AgentStatus> AgentStatuses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString CurrentCycleID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 TotalAgents;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Production")
    void InitializeProductionCycle(const FString& CycleID);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AssignTaskToAgent(int32 AgentNumber, const FString& TaskDescription, const FString& Priority);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateAgentStatus(int32 AgentNumber, const FString& CurrentTask, float Completion);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void CompleteTask(const FString& TaskName);

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_ProductionTask> GetPendingTasks() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    FDir_AgentStatus GetAgentStatus(int32 AgentNumber) const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    float GetOverallProgress() const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Production")
    void SpawnDinosaursInBiomes();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Production")
    void ValidateMinPlayableMap();

private:
    void SetupDefaultTasks();
    void InitializeAgentStatuses();
};