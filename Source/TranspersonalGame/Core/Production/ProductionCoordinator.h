#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "ProductionCoordinator.generated.h"

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction,
    Foundation,
    Core,
    Polish,
    Testing,
    Release
};

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle,
    Working,
    Blocked,
    Complete,
    Error
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
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TArray<int32> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FDateTime StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FDateTime EstimatedCompletion;

    FDir_AgentTask()
    {
        AgentID = 0;
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Status = EDir_AgentStatus::Idle;
        Priority = 1.0f;
        StartTime = FDateTime::Now();
        EstimatedCompletion = FDateTime::Now();
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
    int32 BlockedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float OverallProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    FDateTime ProjectStartDate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    FDateTime EstimatedCompletionDate;

    FDir_ProductionMetrics()
    {
        TotalTasks = 0;
        CompletedTasks = 0;
        BlockedTasks = 0;
        OverallProgress = 0.0f;
        ProjectStartDate = FDateTime::Now();
        EstimatedCompletionDate = FDateTime::Now();
    }
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(TranspersonalGame))
class TRANSPERSONALGAME_API UProductionCoordinator : public UActorComponent
{
    GENERATED_BODY()

public:
    UProductionCoordinator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Production Phase Management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void SetProductionPhase(EDir_ProductionPhase NewPhase);

    UFUNCTION(BlueprintPure, Category = "Production")
    EDir_ProductionPhase GetCurrentPhase() const { return CurrentPhase; }

    // Agent Task Management
    UFUNCTION(BlueprintCallable, Category = "Agents")
    void AddAgentTask(const FDir_AgentTask& NewTask);

    UFUNCTION(BlueprintCallable, Category = "Agents")
    void UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Agents")
    bool IsAgentBlocked(int32 AgentID) const;

    UFUNCTION(BlueprintCallable, Category = "Agents")
    TArray<int32> GetBlockedAgents() const;

    UFUNCTION(BlueprintCallable, Category = "Agents")
    void ResolveAgentDependency(int32 AgentID, int32 DependencyAgentID);

    // Production Metrics
    UFUNCTION(BlueprintCallable, Category = "Metrics")
    FDir_ProductionMetrics CalculateProductionMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Metrics")
    float GetOverallProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Metrics")
    TArray<FDir_AgentTask> GetTasksByStatus(EDir_AgentStatus Status) const;

    // Critical Path Analysis
    UFUNCTION(BlueprintCallable, Category = "Planning")
    TArray<int32> GetCriticalPath() const;

    UFUNCTION(BlueprintCallable, Category = "Planning")
    void OptimizeTaskSchedule();

    // Debug and Monitoring
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void PrintProductionStatus() const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void ValidateProductionState();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agents")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    FDir_ProductionMetrics ProductionMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float TaskUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bAutoResolveBlocks;

private:
    float LastUpdateTime;

    void UpdateProductionMetrics();
    void CheckForBlockedTasks();
    void AutoResolveSimpleBlocks();
    bool CanTaskStart(const FDir_AgentTask& Task) const;
    TArray<int32> FindDependencyChain(int32 StartAgentID) const;
};