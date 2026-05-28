#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
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
struct FDir_AgentTaskData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    int32 AgentNumber = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString CurrentTask;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    EDir_AgentStatus Status = EDir_AgentStatus::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    float ProgressPercentage = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    TArray<FString> Deliverables;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString BlockingReason;

    FDir_AgentTaskData()
    {
        AgentNumber = 0;
        AgentName = TEXT("");
        CurrentTask = TEXT("");
        Status = EDir_AgentStatus::Idle;
        ProgressPercentage = 0.0f;
        BlockingReason = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMilestone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<int32> RequiredAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bCompleted = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float CompletionPercentage = 0.0f;

    FDir_ProductionMilestone()
    {
        MilestoneName = TEXT("");
        Description = TEXT("");
        bCompleted = false;
        CompletionPercentage = 0.0f;
    }
};

/**
 * Studio Director System - Coordinates all 19 AI agents in the production pipeline
 * Translates creative vision into technical tasks and ensures milestone completion
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(TranspersonalGame))
class TRANSPERSONALGAME_API UStudioDirectorSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UStudioDirectorSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Agent Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    TArray<FDir_AgentTaskData> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    TArray<FDir_ProductionMilestone> ProductionMilestones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    FString CurrentCycleID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    int32 TotalActorsInWorld = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    int32 DinosaurActorCount = 0;

    // Core Functions
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeProductionPipeline();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateAgentStatus(int32 AgentNumber, EDir_AgentStatus NewStatus, const FString& TaskDescription);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ValidateMinPlayableMap();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void SpawnDinosaursInBiomes();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_AgentTaskData GetAgentStatus(int32 AgentNumber) const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FString> GetBlockedAgents() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    float GetOverallProgress() const;

    // Milestone Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CheckMilestoneCompletion();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool IsMilestoneCompleted(const FString& MilestoneName) const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CreateMilestone1_WalkAround();

private:
    void SetupAgentTaskList();
    void ValidateWorldState();
    void CountActorsByType();
};