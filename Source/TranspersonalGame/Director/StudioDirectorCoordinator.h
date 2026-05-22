#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "../SharedTypes.h"
#include "StudioDirectorCoordinator.generated.h"

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    int32 AgentNumber = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    EDir_TaskPriority Priority = EDir_TaskPriority::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    EDir_TaskStatus Status = EDir_TaskStatus::Pending;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    float EstimatedHours = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString CycleID;

    FDir_AgentTask()
    {
        AgentNumber = 0;
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Priority = EDir_TaskPriority::Medium;
        Status = EDir_TaskStatus::Pending;
        EstimatedHours = 0.0f;
        CycleID = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 TotalActorsInWorld = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 DinosaurActorsCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 TerrainActorsCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 CompletedTasks = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 PendingTasks = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    float MemoryUsagePercent = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    FString BuildStatus;

    FDir_ProductionMetrics()
    {
        TotalActorsInWorld = 0;
        DinosaurActorsCount = 0;
        TerrainActorsCount = 0;
        CompletedTasks = 0;
        PendingTasks = 0;
        MemoryUsagePercent = 0.0f;
        BuildStatus = TEXT("Unknown");
    }
};

UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UStudioDirectorCoordinator : public UActorComponent
{
    GENERATED_BODY()

public:
    UStudioDirectorCoordinator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Agent Task Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CreateAgentTask(int32 AgentNumber, const FString& AgentName, const FString& TaskDescription, EDir_TaskPriority Priority = EDir_TaskPriority::Medium);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateTaskStatus(int32 AgentNumber, EDir_TaskStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentTask> GetTasksForAgent(int32 AgentNumber);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentTask> GetAllTasks();

    // Production Metrics
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_ProductionMetrics GetProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateProductionMetrics();

    // Agent Chain Coordination
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeAgentChain();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void TriggerNextAgent(int32 CurrentAgentNumber);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool ValidateAgentDependencies(int32 AgentNumber);

    // Development Cycle Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void StartNewCycle(const FString& CycleID);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CompleteCycle();

    UFUNCTION(BlueprintCallable, Category = "Studio Director", CallInEditor)
    void ValidateMinPlayableMap();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Studio Director")
    TArray<FDir_AgentTask> ActiveTasks;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Studio Director")
    FDir_ProductionMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    FString CurrentCycleID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    int32 CurrentActiveAgent = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    bool bCycleInProgress = false;

private:
    void LogTaskUpdate(const FDir_AgentTask& Task);
    void ValidateWorldState();
    void CheckAgentChainIntegrity();
};