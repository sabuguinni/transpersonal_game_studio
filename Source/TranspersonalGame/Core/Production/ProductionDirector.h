#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "ProductionDirector.generated.h"

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    int32 Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    bool bCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    float CompletionPercentage;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Priority = 1;
        bCompleted = false;
        CompletionPercentage = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMilestone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FString> RequiredAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    int32 MinActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bAchieved;

    FDir_ProductionMilestone()
    {
        MilestoneName = TEXT("");
        MinActorCount = 0;
        bAchieved = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AProductionDirector : public AActor
{
    GENERATED_BODY()

public:
    AProductionDirector();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Management")
    TArray<FDir_AgentTask> ActiveTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Management")
    TArray<FDir_ProductionMilestone> ProductionMilestones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 CurrentCycleNumber;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    float PlayablePrototypeScore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 TotalActorsInLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 TerrainActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 CharacterActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 DinosaurActorCount;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void InitializeProductionPipeline();

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription, int32 Priority);

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void UpdateTaskProgress(const FString& AgentName, float CompletionPercentage);

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void CompleteAgentTask(const FString& AgentName);

    UFUNCTION(BlueprintCallable, Category = "Production Metrics")
    void AnalyzeProductionState();

    UFUNCTION(BlueprintCallable, Category = "Production Metrics")
    float CalculatePlayableScore();

    UFUNCTION(BlueprintCallable, Category = "Production Metrics")
    void ValidateMilestones();

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    TArray<FDir_AgentTask> GetPendingTasks();

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    bool IsMilestoneAchieved(const FString& MilestoneName);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Production Tools")
    void GenerateProductionReport();

private:
    void SetupDefaultMilestones();
    void CountLevelActors();
    void LogProductionStatus();
};

#include "ProductionDirector.generated.h"