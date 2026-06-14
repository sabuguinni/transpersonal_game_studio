#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ProductionMetrics.generated.h"

USTRUCT(BlueprintType)
struct FDir_AgentMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    FString AgentName;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 FilesCreated;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 UE5CommandsExecuted;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float ProductionScore;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    FDateTime LastActivity;

    FDir_AgentMetrics()
    {
        AgentName = TEXT("");
        FilesCreated = 0;
        UE5CommandsExecuted = 0;
        ProductionScore = 0.0f;
        LastActivity = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FDir_CycleMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    FString CycleID;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 TotalActorsInLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 DinosaurCount;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 CharacterCount;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float BudgetUsed;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    TArray<FDir_AgentMetrics> AgentMetrics;

    FDir_CycleMetrics()
    {
        CycleID = TEXT("");
        TotalActorsInLevel = 0;
        DinosaurCount = 0;
        CharacterCount = 0;
        BudgetUsed = 0.0f;
    }
};

/**
 * Production Metrics Subsystem
 * Tracks performance and output of all 19 AI agents
 */
UCLASS()
class TRANSPERSONALGAME_API UProductionMetricsSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "Production")
    void RecordAgentActivity(const FString& AgentName, int32 FilesCreated, int32 UE5Commands);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void StartNewCycle(const FString& CycleID);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateLevelMetrics(int32 TotalActors, int32 Dinosaurs, int32 Characters);

    UFUNCTION(BlueprintCallable, Category = "Production")
    FDir_CycleMetrics GetCurrentCycleMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentMetrics> GetTopPerformingAgents(int32 Count = 5) const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    float CalculateOverallProductionScore() const;

private:
    UPROPERTY()
    FDir_CycleMetrics CurrentCycle;

    UPROPERTY()
    TArray<FDir_CycleMetrics> CycleHistory;

    void CalculateAgentScores();
};