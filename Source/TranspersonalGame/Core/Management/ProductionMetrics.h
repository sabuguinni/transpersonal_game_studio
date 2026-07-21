#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "ProductionMetrics.generated.h"

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction,
    Prototyping,
    Production,
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
    Blocked,
    Complete,
    Error
};

USTRUCT(BlueprintType)
struct FDir_AgentMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    FString AgentName;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    EDir_AgentStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 TasksCompleted;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 FilesCreated;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float CycleTime;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float SuccessRate;

    FDir_AgentMetrics()
    {
        AgentName = TEXT("");
        Status = EDir_AgentStatus::Idle;
        TasksCompleted = 0;
        FilesCreated = 0;
        CycleTime = 0.0f;
        SuccessRate = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    int32 TotalAgents;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    int32 ActiveAgents;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    int32 CompletedTasks;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    float OverallProgress;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    FString CurrentCycle;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    TArray<FDir_AgentMetrics> AgentMetrics;

    FDir_ProductionStats()
    {
        CurrentPhase = EDir_ProductionPhase::Prototyping;
        TotalAgents = 20;
        ActiveAgents = 0;
        CompletedTasks = 0;
        OverallProgress = 0.0f;
        CurrentCycle = TEXT("PROD_CYCLE_AUTO_20260613_006");
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UProductionMetrics : public UActorComponent
{
    GENERATED_BODY()

public:
    UProductionMetrics();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(BlueprintReadOnly, Category = "Production Metrics")
    FDir_ProductionStats ProductionStats;

    UPROPERTY(BlueprintReadOnly, Category = "Production Metrics")
    TMap<FString, FDir_AgentMetrics> AgentRegistry;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Production Metrics")
    void UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Production Metrics")
    void IncrementTasksCompleted(const FString& AgentName);

    UFUNCTION(BlueprintCallable, Category = "Production Metrics")
    void RecordFileCreated(const FString& AgentName);

    UFUNCTION(BlueprintCallable, Category = "Production Metrics")
    FDir_ProductionStats GetProductionStats() const;

    UFUNCTION(BlueprintCallable, Category = "Production Metrics")
    TArray<FDir_AgentMetrics> GetAgentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Production Metrics")
    float CalculateOverallProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Production Metrics")
    void SetProductionPhase(EDir_ProductionPhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Production Metrics")
    void LogProductionState() const;

private:
    void InitializeAgentRegistry();
    void UpdateProductionStats();
    float CalculateAgentSuccessRate(const FDir_AgentMetrics& Metrics) const;
};

#include "ProductionMetrics.generated.h"