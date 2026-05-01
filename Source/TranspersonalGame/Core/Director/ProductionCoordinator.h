#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "../../SharedTypes.h"
#include "ProductionCoordinator.generated.h"

/**
 * Studio Director's Production Coordinator System
 * Manages agent task coordination and milestone tracking for Transpersonal Game Studio
 */

UENUM(BlueprintType)
enum class EDir_AgentPriority : uint8
{
    Critical = 1,
    High = 2,
    Medium = 3,
    Low = 4
};

UENUM(BlueprintType)
enum class EDir_TaskStatus : uint8
{
    Pending,
    InProgress,
    Completed,
    Blocked,
    Failed
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_AgentPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_TaskStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FVector MarkerLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float EstimatedHours;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TArray<FString> Dependencies;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Priority = EDir_AgentPriority::Medium;
        Status = EDir_TaskStatus::Pending;
        MarkerLocation = FVector::ZeroVector;
        EstimatedHours = 1.0f;
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
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bIsBlocking;

    FDir_MilestoneProgress()
    {
        MilestoneName = TEXT("");
        CompletionPercentage = 0.0f;
        bIsBlocking = false;
    }
};

/**
 * Production Coordinator Actor - Studio Director's main coordination system
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADir_ProductionCoordinator : public AActor
{
    GENERATED_BODY()

public:
    ADir_ProductionCoordinator();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // Current milestone being tracked
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FDir_MilestoneProgress CurrentMilestone;

    // All active agent tasks
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_AgentTask> ActiveTasks;

    // Production metrics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalCyclesCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float BudgetUsed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float BudgetLimit;

public:
    virtual void Tick(float DeltaTime) override;

    // Task management functions
    UFUNCTION(BlueprintCallable, Category = "Production")
    void AddAgentTask(const FDir_AgentTask& NewTask);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateTaskStatus(const FString& AgentName, EDir_TaskStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void CompleteTask(const FString& AgentName);

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentTask> GetTasksByPriority(EDir_AgentPriority Priority);

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentTask> GetPendingTasks();

    // Milestone management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void SetCurrentMilestone(const FDir_MilestoneProgress& NewMilestone);

    UFUNCTION(BlueprintCallable, Category = "Production")
    float CalculateMilestoneProgress();

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool IsMilestoneComplete();

    // Production metrics
    UFUNCTION(BlueprintCallable, Category = "Production")
    void IncrementCycleCount();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateBudgetUsage(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool IsOverBudget();

    // Debug and visualization
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void CreateTaskMarkers();

    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void ClearTaskMarkers();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void PrintProductionReport();

protected:
    // Internal task marker management
    UPROPERTY()
    TArray<class AActor*> TaskMarkerActors;

    void SpawnTaskMarker(const FDir_AgentTask& Task);
    void UpdateTaskMarker(const FDir_AgentTask& Task);
};