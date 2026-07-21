#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Dir_ProductionMetrics.generated.h"

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction,
    Prototype,
    Production,
    Alpha,
    Beta,
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

    UPROPERTY(BlueprintReadWrite, Category = "Agent Task")
    int32 AgentNumber = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Agent Task")
    FString AgentName;

    UPROPERTY(BlueprintReadWrite, Category = "Agent Task")
    FString CurrentTask;

    UPROPERTY(BlueprintReadWrite, Category = "Agent Task")
    EDir_AgentStatus Status = EDir_AgentStatus::Idle;

    UPROPERTY(BlueprintReadWrite, Category = "Agent Task")
    float ProgressPercent = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Agent Task")
    FString LastOutput;

    UPROPERTY(BlueprintReadWrite, Category = "Agent Task")
    FDateTime LastUpdate;

    FDir_AgentTask()
    {
        LastUpdate = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Production Stats")
    int32 TotalActors = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Production Stats")
    int32 DinosaurCount = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Production Stats")
    int32 CharacterCount = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Production Stats")
    int32 LandscapeCount = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Production Stats")
    float CurrentFPS = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Production Stats")
    float MemoryUsageGB = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Production Stats")
    EDir_ProductionPhase CurrentPhase = EDir_ProductionPhase::Prototype;

    UPROPERTY(BlueprintReadWrite, Category = "Production Stats")
    FDateTime LastUpdated;

    FDir_ProductionStats()
    {
        LastUpdated = FDateTime::Now();
    }
};

/**
 * Studio Director Production Metrics Subsystem
 * Tracks real-time production metrics and agent coordination
 */
UCLASS()
class TRANSPERSONALGAME_API UDir_ProductionMetrics : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Production metrics
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateProductionStats();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_ProductionStats GetCurrentStats() const { return CurrentStats; }

    // Agent coordination
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateAgentTask(int32 AgentNumber, const FString& TaskDescription, EDir_AgentStatus Status, float Progress = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentTask> GetAllAgentTasks() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_AgentTask GetAgentTask(int32 AgentNumber) const;

    // Production phase management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void SetProductionPhase(EDir_ProductionPhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool IsPhaseComplete(EDir_ProductionPhase Phase) const;

    // Critical path analysis
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<int32> GetBlockedAgents() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<int32> GetCriticalPathAgents() const;

    // Milestone tracking
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool IsWalkAroundMilestoneComplete() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    float GetOverallProgress() const;

protected:
    UPROPERTY()
    FDir_ProductionStats CurrentStats;

    UPROPERTY()
    TMap<int32, FDir_AgentTask> AgentTasks;

    UPROPERTY()
    FTimerHandle MetricsUpdateTimer;

    void InitializeAgentTasks();
    void UpdateMetricsTimer();
    bool ValidateMinimumViablePrototype() const;
};