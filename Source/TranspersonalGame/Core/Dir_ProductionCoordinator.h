#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Engine/TargetPoint.h"
#include "../SharedTypes.h"
#include "Dir_ProductionCoordinator.generated.h"

UENUM(BlueprintType)
enum class EDir_AgentPriority : uint8
{
    Critical = 0,
    High = 1,
    Medium = 2,
    Low = 3
};

UENUM(BlueprintType)
enum class EDir_TaskStatus : uint8
{
    Pending = 0,
    InProgress = 1,
    Completed = 2,
    Blocked = 3,
    Failed = 4
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    int32 AgentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_AgentPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_TaskStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FVector TaskLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float EstimatedDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TArray<FString> Dependencies;

    FDir_AgentTask()
    {
        AgentID = 0;
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Priority = EDir_AgentPriority::Medium;
        Status = EDir_TaskStatus::Pending;
        TaskLocation = FVector::ZeroVector;
        EstimatedDuration = 60.0f;
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CompletedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 PendingTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 BlockedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float OverallProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    FString CurrentCycle;

    FDir_ProductionMetrics()
    {
        TotalTasks = 0;
        CompletedTasks = 0;
        PendingTasks = 0;
        BlockedTasks = 0;
        OverallProgress = 0.0f;
        CurrentCycle = TEXT("PROD_CYCLE_AUTO_007");
    }
};

/**
 * Production Coordinator - Studio Director's central coordination system
 * Manages agent tasks, tracks progress, and coordinates the 19-agent production pipeline
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FDir_ProductionMetrics ProductionMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<AActor*> TaskMarkers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    bool bAutoUpdateMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float MetricsUpdateInterval;

public:
    virtual void Tick(float DeltaTime) override;

    // Task Management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void AddAgentTask(int32 AgentID, const FString& AgentName, const FString& TaskDescription, 
                     EDir_AgentPriority Priority, const FVector& TaskLocation);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateTaskStatus(int32 AgentID, EDir_TaskStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void CompleteTask(int32 AgentID);

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentTask> GetTasksByPriority(EDir_AgentPriority Priority);

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentTask> GetTasksByStatus(EDir_TaskStatus Status);

    // Metrics and Monitoring
    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Production")
    FDir_ProductionMetrics GetProductionMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    float GetProgressPercentage() const;

    // Task Marker Management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void CreateTaskMarker(const FVector& Location, const FString& MarkerName);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void ClearAllTaskMarkers();

    // Agent Coordination
    UFUNCTION(BlueprintCallable, Category = "Production")
    void InitializeAgentTasks();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AssignNextAgentTasks();

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool CanAgentProceed(int32 AgentID) const;

    // Debug and Visualization
    UFUNCTION(BlueprintCallable, Category = "Production", CallInEditor)
    void DebugPrintAllTasks();

    UFUNCTION(BlueprintCallable, Category = "Production", CallInEditor)
    void DebugPrintMetrics();

private:
    void SetupInitialTasks();
    void CheckTaskDependencies();
    FDir_AgentTask* FindTaskByAgentID(int32 AgentID);
    
    float LastMetricsUpdate;
};