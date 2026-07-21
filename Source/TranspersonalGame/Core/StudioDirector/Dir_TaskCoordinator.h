#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Dir_TaskCoordinator.generated.h"

/**
 * Studio Director Task Coordination System
 * Manages agent task assignments and progress tracking across development cycles
 */

UENUM(BlueprintType)
enum class EDir_AgentPriority : uint8
{
    Critical    UMETA(DisplayName = "Critical"),
    High        UMETA(DisplayName = "High"),
    Medium      UMETA(DisplayName = "Medium"),
    Low         UMETA(DisplayName = "Low")
};

UENUM(BlueprintType)
enum class EDir_TaskStatus : uint8
{
    Pending     UMETA(DisplayName = "Pending"),
    InProgress  UMETA(DisplayName = "In Progress"),
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed"),
    Blocked     UMETA(DisplayName = "Blocked")
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
    FString RequiredDeliverables;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    int32 CycleAssigned;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    int32 CycleCompleted;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Priority = EDir_AgentPriority::Medium;
        Status = EDir_TaskStatus::Pending;
        MarkerLocation = FVector::ZeroVector;
        RequiredDeliverables = TEXT("");
        CycleAssigned = 0;
        CycleCompleted = 0;
    }
};

USTRUCT(BlueprintType)
struct FDir_CycleMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CycleNumber;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TasksAssigned;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TasksCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 FilesCreated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 UE5CommandsExecuted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float TotalExecutionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    bool bPlayablePrototypeReady;

    FDir_CycleMetrics()
    {
        CycleNumber = 0;
        TasksAssigned = 0;
        TasksCompleted = 0;
        FilesCreated = 0;
        UE5CommandsExecuted = 0;
        TotalExecutionTime = 0.0f;
        bPlayablePrototypeReady = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADir_TaskCoordinator : public AActor
{
    GENERATED_BODY()

public:
    ADir_TaskCoordinator();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* CoordinatorMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task Management")
    TArray<FDir_AgentTask> ActiveTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    TArray<FDir_CycleMetrics> CycleHistory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    int32 CurrentCycle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bDebugMode;

public:
    virtual void Tick(float DeltaTime) override;

    // Task Management Functions
    UFUNCTION(BlueprintCallable, Category = "Task Management")
    void AssignTask(const FString& AgentName, const FString& TaskDescription, 
                   EDir_AgentPriority Priority, const FVector& MarkerLocation);

    UFUNCTION(BlueprintCallable, Category = "Task Management")
    void UpdateTaskStatus(const FString& AgentName, EDir_TaskStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Task Management")
    void CompleteTask(const FString& AgentName, const FString& Deliverables);

    UFUNCTION(BlueprintCallable, Category = "Task Management")
    TArray<FDir_AgentTask> GetTasksByPriority(EDir_AgentPriority Priority);

    UFUNCTION(BlueprintCallable, Category = "Task Management")
    TArray<FDir_AgentTask> GetTasksByStatus(EDir_TaskStatus Status);

    // Cycle Management Functions
    UFUNCTION(BlueprintCallable, Category = "Cycle Management")
    void StartNewCycle();

    UFUNCTION(BlueprintCallable, Category = "Cycle Management")
    void EndCurrentCycle();

    UFUNCTION(BlueprintCallable, Category = "Cycle Management")
    FDir_CycleMetrics GetCurrentCycleMetrics();

    UFUNCTION(BlueprintCallable, Category = "Cycle Management")
    bool IsPlayablePrototypeReady();

    // Coordination Functions
    UFUNCTION(BlueprintCallable, Category = "Coordination")
    void SpawnTaskMarker(const FString& TaskName, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Coordination")
    void RemoveTaskMarker(const FString& TaskName);

    UFUNCTION(BlueprintCallable, Category = "Coordination")
    void ValidateAgentProgress();

    // Debug Functions
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugPrintAllTasks();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugValidatePlayableState();

private:
    // Internal helper functions
    void InitializeTaskCoordinator();
    void UpdateCycleMetrics();
    bool ValidateMinPlayableMap();
    void LogTaskProgress();
};