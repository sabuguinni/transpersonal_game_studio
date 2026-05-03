#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "../../SharedTypes.h"
#include "ProductionCoordinator.generated.h"

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle,
    Working,
    Completed,
    Failed,
    Blocked
};

UENUM(BlueprintType)
enum class EDir_MilestonePhase : uint8
{
    Planning,
    InProgress,
    Testing,
    Completed,
    Failed
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ExpectedOutput;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EstimatedDuration;

    FDir_AgentTask()
    {
        Status = EDir_AgentStatus::Idle;
        Priority = 1.0f;
        EstimatedDuration = 300.0f; // 5 minutes default
    }
};

USTRUCT(BlueprintType)
struct FDir_MilestoneProgress
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDir_MilestonePhase Phase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FDir_AgentTask> Tasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString BlockingIssues;

    FDir_MilestoneProgress()
    {
        Phase = EDir_MilestonePhase::Planning;
        CompletionPercentage = 0.0f;
    }
};

/**
 * Production Coordinator - Studio Director's main coordination system
 * Manages agent tasks, milestone tracking, and production pipeline
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UProductionCoordinator : public UActorComponent
{
    GENERATED_BODY()

public:
    UProductionCoordinator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Milestone 1: "Walk Around" tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone 1")
    FDir_MilestoneProgress Milestone1Progress;

    // Agent coordination
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coordination")
    TArray<FDir_AgentTask> ActiveTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coordination")
    TArray<FString> CompletedAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coordination")
    FString CurrentCycleID;

    // Production metrics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalCppFiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalHeaderFiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 VisibleActorsInMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    bool bPlayerCanMove;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    bool bDinosaursVisible;

    // Coordination functions
    UFUNCTION(BlueprintCallable, Category = "Coordination")
    void InitializeMilestone1();

    UFUNCTION(BlueprintCallable, Category = "Coordination")
    void UpdateAgentTask(const FString& AgentName, EDir_AgentStatus NewStatus, const FString& Output = "");

    UFUNCTION(BlueprintCallable, Category = "Coordination")
    void CheckMilestone1Completion();

    UFUNCTION(BlueprintCallable, Category = "Coordination")
    FString GetNextPriorityAgent();

    UFUNCTION(BlueprintCallable, Category = "Coordination")
    void LogProductionStatus();

    UFUNCTION(BlueprintCallable, Category = "Coordination")
    bool VerifyMinPlayableMapState();

    UFUNCTION(BlueprintCallable, Category = "Coordination")
    void GenerateAgentTaskList();

    // Debug functions
    UFUNCTION(CallInEditor, Category = "Debug")
    void DebugPrintAllTasks();

    UFUNCTION(CallInEditor, Category = "Debug")
    void DebugCheckMapActors();

private:
    void UpdateProductionMetrics();
    void ValidateAgentDependencies();
    FString GenerateProductionReport();
};