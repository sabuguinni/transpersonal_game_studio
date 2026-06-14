#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Dir_ProductionCoordinator.generated.h"

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction,
    CoreSystems,
    WorldBuilding,
    CharacterDevelopment,
    GameplayImplementation,
    PolishAndOptimization,
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
    Failed
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 AgentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FDateTime StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FDateTime EstimatedCompletion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FString> Deliverables;

    FDir_AgentTask()
    {
        AgentID = 0;
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Status = EDir_AgentStatus::Idle;
        Priority = 0.0f;
        StartTime = FDateTime::Now();
        EstimatedCompletion = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalActorsInLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 DinosaurActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CharacterActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 EnvironmentActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CompletedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 PendingTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float OverallProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    FDateTime LastUpdate;

    FDir_ProductionMetrics()
    {
        TotalActorsInLevel = 0;
        DinosaurActors = 0;
        CharacterActors = 0;
        EnvironmentActors = 0;
        CompletedTasks = 0;
        PendingTasks = 0;
        OverallProgress = 0.0f;
        LastUpdate = FDateTime::Now();
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDir_ProductionCoordinator : public UActorComponent
{
    GENERATED_BODY()

public:
    UDir_ProductionCoordinator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Production Phase Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_AgentTask> ActiveTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FDir_ProductionMetrics CurrentMetrics;

    // Core Production Functions
    UFUNCTION(BlueprintCallable, Category = "Production")
    void InitializeProduction();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AssignTaskToAgent(int32 AgentID, const FString& TaskDescription, float Priority);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateTaskStatus(int32 AgentID, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AnalyzeLevelState();

    UFUNCTION(BlueprintCallable, Category = "Production")
    FDir_ProductionMetrics GetProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AdvanceToNextPhase();

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool CanAdvancePhase();

    // Agent Coordination
    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentTask> GetTasksForAgent(int32 AgentID);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void BlockAgent(int32 AgentID, const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UnblockAgent(int32 AgentID);

    // Quality Assurance
    UFUNCTION(BlueprintCallable, Category = "Production")
    bool ValidateMinimumPlayableState();

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FString> GetBlockingIssues();

    // Development Diary
    UFUNCTION(BlueprintCallable, Category = "Production")
    FString GenerateProductionReport();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void LogProductionMilestone(const FString& MilestoneDescription);

private:
    // Internal tracking
    UPROPERTY()
    float LastAnalysisTime;

    UPROPERTY()
    TArray<FString> ProductionLog;

    // Helper functions
    void UpdateMetrics();
    void CheckDependencies();
    void OptimizeTaskQueue();
};

#include "Dir_ProductionCoordinator.generated.h"