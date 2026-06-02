#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Dir_ProductionCoordinator.generated.h"

UENUM(BlueprintType)
enum class EDir_MilestoneStatus : uint8
{
    NotStarted UMETA(DisplayName = "Not Started"),
    InProgress UMETA(DisplayName = "In Progress"),
    Completed UMETA(DisplayName = "Completed"),
    Blocked UMETA(DisplayName = "Blocked")
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
    EDir_MilestoneStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    int32 Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float EstimatedHours;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Status = EDir_MilestoneStatus::NotStarted;
        Priority = 1;
        EstimatedHours = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADir_ProductionCoordinator : public AActor
{
    GENERATED_BODY()

public:
    ADir_ProductionCoordinator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_AgentTask> CurrentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    EDir_MilestoneStatus WalkAroundMilestoneStatus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 CurrentCycleID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float ProductionBudgetUsed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float ProductionBudgetTotal;

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription, int32 Priority);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateTaskStatus(const FString& AgentName, EDir_MilestoneStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void GenerateProductionReport();

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool IsWalkAroundMilestoneComplete();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Production")
    void InitializeProductionTasks();

private:
    void SetupMilestone1Tasks();
    void ValidateAgentProgress();
    float CalculateOverallProgress();
};