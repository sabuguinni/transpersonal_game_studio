#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "ProductionCoordinator.generated.h"

/**
 * Production milestone tracking for development cycles
 */
UENUM(BlueprintType)
enum class EDir_ProductionMilestone : uint8
{
    None            UMETA(DisplayName = "None"),
    Movement        UMETA(DisplayName = "Basic Movement"),
    Dinosaurs       UMETA(DisplayName = "Dinosaur Interaction"),
    Survival        UMETA(DisplayName = "Survival Systems"),
    Complete        UMETA(DisplayName = "Prototype Complete")
};

/**
 * Agent task priority levels
 */
UENUM(BlueprintType)
enum class EDir_TaskPriority : uint8
{
    Low         UMETA(DisplayName = "Low Priority"),
    Medium      UMETA(DisplayName = "Medium Priority"),
    High        UMETA(DisplayName = "High Priority"),
    Critical    UMETA(DisplayName = "Critical Priority")
};

/**
 * Agent task status tracking
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    int32 AgentNumber = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_TaskPriority Priority = EDir_TaskPriority::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    bool bCompleted = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float CompletionPercentage = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString LastUpdate;

    FDir_AgentTask()
    {
        AgentNumber = 0;
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Priority = EDir_TaskPriority::Medium;
        bCompleted = false;
        CompletionPercentage = 0.0f;
        LastUpdate = TEXT("");
    }
};

/**
 * Production milestone data
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_MilestoneData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    EDir_ProductionMilestone MilestoneType = EDir_ProductionMilestone::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bCompleted = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float Progress = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<int32> RequiredAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FVector WorldLocation = FVector::ZeroVector;

    FDir_MilestoneData()
    {
        MilestoneType = EDir_ProductionMilestone::None;
        MilestoneName = TEXT("");
        Description = TEXT("");
        bCompleted = false;
        Progress = 0.0f;
        RequiredAgents.Empty();
        WorldLocation = FVector::ZeroVector;
    }
};

/**
 * Studio Director Production Coordinator
 * Manages the 19-agent development pipeline and milestone tracking
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AProductionCoordinator : public AActor
{
    GENERATED_BODY()

public:
    AProductionCoordinator();

protected:
    virtual void BeginPlay() override;

    // Production milestones
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_MilestoneData> ProductionMilestones;

    // Agent task tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_AgentTask> AgentTasks;

    // Current active milestone
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    EDir_ProductionMilestone CurrentMilestone = EDir_ProductionMilestone::Movement;

    // Production metrics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalCycles = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CompletedTasks = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float OverallProgress = 0.0f;

public:
    // Milestone management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void InitializeMilestones();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateMilestoneProgress(EDir_ProductionMilestone Milestone, float NewProgress);

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool IsMilestoneComplete(EDir_ProductionMilestone Milestone) const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    FDir_MilestoneData GetMilestoneData(EDir_ProductionMilestone Milestone) const;

    // Agent task management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void AssignTaskToAgent(int32 AgentNumber, const FString& TaskDescription, EDir_TaskPriority Priority);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateAgentTaskProgress(int32 AgentNumber, float Progress, const FString& UpdateNote);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void CompleteAgentTask(int32 AgentNumber);

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentTask> GetTasksForAgent(int32 AgentNumber) const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentTask> GetHighPriorityTasks() const;

    // Production metrics
    UFUNCTION(BlueprintCallable, Category = "Production")
    void IncrementCycleCount();

    UFUNCTION(BlueprintCallable, Category = "Production")
    float CalculateOverallProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    FString GenerateProductionReport() const;

    // Coordination utilities
    UFUNCTION(BlueprintCallable, Category = "Production")
    void CreateMilestoneMarkers();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void ValidateMinPlayableMapState();

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<int32> GetNextPriorityAgents() const;
};