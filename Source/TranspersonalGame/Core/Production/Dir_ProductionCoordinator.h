#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "Dir_ProductionCoordinator.generated.h"

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction,
    Prototype,
    Alpha,
    Beta,
    Polish,
    Release
};

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle,
    Working,
    Completed,
    Blocked,
    Failed
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 EstimatedCycles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CompletedCycles;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Status = EDir_AgentStatus::Idle;
        Priority = 1.0f;
        EstimatedCycles = 1;
        CompletedCycles = 0;
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 DinosaurCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PropCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TerrainComplexity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GameplayCompleteness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PerformanceScore;

    FDir_ProductionMetrics()
    {
        TotalActors = 0;
        DinosaurCount = 0;
        PropCount = 0;
        TerrainComplexity = 0.0f;
        GameplayCompleteness = 0.0f;
        PerformanceScore = 100.0f;
    }
};

/**
 * Production Coordinator - Studio Director's main coordination system
 * Manages agent tasks, tracks production metrics, and coordinates the development pipeline
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDir_ProductionCoordinator : public UActorComponent
{
    GENERATED_BODY()

public:
    UDir_ProductionCoordinator();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FDir_ProductionMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float CycleTimeTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 CurrentCycle;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Agent Management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void AddAgentTask(const FString& AgentName, const FString& TaskDescription, float Priority = 1.0f, int32 EstimatedCycles = 1);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void CompleteAgentTask(const FString& AgentName);

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentTask> GetPendingTasks();

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentTask> GetTasksByStatus(EDir_AgentStatus Status);

    // Production Phase Management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void AdvanceProductionPhase();

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool CanAdvancePhase();

    UFUNCTION(BlueprintCallable, Category = "Production")
    FString GetPhaseDescription();

    // Metrics and Analysis
    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Production")
    float GetOverallProgress();

    UFUNCTION(BlueprintCallable, Category = "Production")
    FDir_ProductionMetrics GetCurrentMetrics() const { return CurrentMetrics; }

    // Coordination Functions
    UFUNCTION(BlueprintCallable, Category = "Production")
    void StartNewCycle();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void GenerateTaskPriorities();

    UFUNCTION(BlueprintCallable, Category = "Production")
    FString GetNextAgentToExecute();

    UFUNCTION(BlueprintCallable, Category = "Production", CallInEditor)
    void InitializeProductionPipeline();

    UFUNCTION(BlueprintCallable, Category = "Production", CallInEditor)
    void AnalyzeCurrentState();

    UFUNCTION(BlueprintCallable, Category = "Production", CallInEditor)
    void GenerateProductionReport();

private:
    void InitializeAgentTasks();
    void CalculateGameplayCompleteness();
    void CalculatePerformanceScore();
    float CalculateTaskProgress(const FDir_AgentTask& Task);
};