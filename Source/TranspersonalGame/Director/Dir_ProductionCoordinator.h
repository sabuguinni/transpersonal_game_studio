#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Dir_ProductionCoordinator.generated.h"

/**
 * Studio Director's Production Coordination System
 * Manages agent task dispatch and milestone tracking for the playable prototype
 */

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
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FVector TaskLocation;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Priority = 1;
        bIsCompleted = false;
        TaskLocation = FVector::ZeroVector;
    }
};

USTRUCT(BlueprintType)
struct FDir_MilestoneStatus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FDir_AgentTask> RequiredTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bIsCriticalPath;

    FDir_MilestoneStatus()
    {
        MilestoneName = TEXT("");
        CompletionPercentage = 0.0f;
        bIsCriticalPath = false;
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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    // Current milestone being tracked
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FDir_MilestoneStatus CurrentMilestone;

    // All active agent tasks
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_AgentTask> ActiveTasks;

    // Production metrics
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Metrics")
    int32 TotalActorsInWorld;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Metrics")
    int32 DinosaurCount;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Metrics")
    int32 EnvironmentPropsCount;

public:
    virtual void Tick(float DeltaTime) override;

    // Task management functions
    UFUNCTION(BlueprintCallable, Category = "Production")
    void DispatchTaskToAgent(const FString& AgentName, const FString& TaskDescription, int32 Priority, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void CompleteAgentTask(const FString& AgentName);

    UFUNCTION(BlueprintCallable, Category = "Production")
    float GetMilestoneProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateWorldMetrics();

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentTask> GetHighPriorityTasks() const;

    // Milestone tracking
    UFUNCTION(BlueprintCallable, Category = "Production")
    void SetCurrentMilestone(const FString& MilestoneName, const TArray<FDir_AgentTask>& RequiredTasks);

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool IsMilestoneComplete() const;

    // Production validation
    UFUNCTION(BlueprintCallable, Category = "Production")
    bool ValidatePlayablePrototype() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    FString GetProductionStatusReport() const;

private:
    // Internal tracking
    float LastMetricsUpdate;
    static const float MetricsUpdateInterval;

    void InitializePlayablePrototypeMilestone();
    void ValidateAgentOutputs();
};