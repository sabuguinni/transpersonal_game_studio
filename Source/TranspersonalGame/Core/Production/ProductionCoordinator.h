#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "ProductionCoordinator.generated.h"

UENUM(BlueprintType)
enum class EDir_AgentPriority : uint8
{
    LOW = 0,
    MEDIUM = 1,
    HIGH = 2,
    CRITICAL = 3
};

UENUM(BlueprintType)
enum class EDir_TaskStatus : uint8
{
    PENDING = 0,
    IN_PROGRESS = 1,
    COMPLETED = 2,
    BLOCKED = 3,
    FAILED = 4
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_AgentPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString ExpectedDeliverable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_TaskStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float EstimatedHours;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FDateTime AssignedTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FDateTime CompletedTime;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        Priority = EDir_AgentPriority::MEDIUM;
        TaskDescription = TEXT("");
        ExpectedDeliverable = TEXT("");
        Status = EDir_TaskStatus::PENDING;
        EstimatedHours = 1.0f;
        AssignedTime = FDateTime::Now();
        CompletedTime = FDateTime::MinValue();
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalActorsInMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CompletedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 PendingTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 BlockedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float Milestone1Progress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float OverallProductionHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    FDateTime LastUpdateTime;

    FDir_ProductionMetrics()
    {
        TotalActorsInMap = 0;
        CompletedTasks = 0;
        PendingTasks = 0;
        BlockedTasks = 0;
        Milestone1Progress = 0.0f;
        OverallProductionHealth = 100.0f;
        LastUpdateTime = FDateTime::Now();
    }
};

/**
 * Studio Director production coordination system for managing AI agent tasks and tracking development progress
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AProductionCoordinator : public AActor
{
    GENERATED_BODY()
    
public:    
    AProductionCoordinator();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production", meta = (AllowPrivateAccess = "true"))
    TArray<FDir_AgentTask> ActiveTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production", meta = (AllowPrivateAccess = "true"))
    FDir_ProductionMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production", meta = (AllowPrivateAccess = "true"))
    bool bAutoUpdateMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production", meta = (AllowPrivateAccess = "true"))
    float MetricsUpdateInterval;

    FTimerHandle MetricsUpdateTimer;

public:    
    virtual void Tick(float DeltaTime) override;

    // Task Management Functions
    UFUNCTION(BlueprintCallable, Category = "Production")
    void AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription, 
                          const FString& ExpectedDeliverable, EDir_AgentPriority Priority = EDir_AgentPriority::MEDIUM);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateTaskStatus(const FString& AgentName, EDir_TaskStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void CompleteTask(const FString& AgentName);

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentTask> GetTasksByPriority(EDir_AgentPriority Priority);

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentTask> GetTasksByStatus(EDir_TaskStatus Status);

    // Metrics and Analysis Functions
    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Production")
    FDir_ProductionMetrics GetCurrentMetrics() const { return CurrentMetrics; }

    UFUNCTION(BlueprintCallable, Category = "Production")
    float CalculateMilestone1Progress();

    UFUNCTION(BlueprintCallable, Category = "Production")
    float CalculateProductionHealth();

    UFUNCTION(BlueprintCallable, Category = "Production")
    int32 CountActorsByClass(UClass* ActorClass);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void GenerateProductionReport();

    // Editor-only functions for development
    UFUNCTION(CallInEditor, Category = "Production")
    void InitializeDefaultTasks();

    UFUNCTION(CallInEditor, Category = "Production")
    void RefreshMetricsNow();

    UFUNCTION(CallInEditor, Category = "Production")
    void ValidateMapState();

private:
    void UpdateMetricsTimer();
    void AnalyzeMapActors();
    void CalculateTaskProgress();
};

#include "ProductionCoordinator.generated.h"