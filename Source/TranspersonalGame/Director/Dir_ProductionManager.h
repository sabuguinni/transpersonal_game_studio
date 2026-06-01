#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Dir_ProductionManager.generated.h"

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction,
    PrototypeDevelopment,
    VerticalSlice,
    Production,
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
    Failed
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    int32 AgentNumber = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_AgentStatus Status = EDir_AgentStatus::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float Priority = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TArray<FString> Deliverables;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FDateTime StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FDateTime DeadlineTime;

    FDir_AgentTask()
    {
        StartTime = FDateTime::Now();
        DeadlineTime = FDateTime::Now() + FTimespan::FromHours(2);
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalActorsInMap = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 DinosaurCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CharacterCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CompletedTasks = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 FailedTasks = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float OverallProgress = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    bool bIsPlayable = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    FDateTime LastUpdateTime;

    FDir_ProductionMetrics()
    {
        LastUpdateTime = FDateTime::Now();
    }
};

/**
 * Studio Director Production Manager
 * Coordinates all 19 agents and tracks production progress
 * Ensures the game reaches playable state efficiently
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDir_ProductionManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UDir_ProductionManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Production management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void InitializeProductionPipeline();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateProductionStatus();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AssignTaskToAgent(int32 AgentNumber, const FString& TaskDescription, float Priority = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void CompleteAgentTask(int32 AgentNumber, const TArray<FString>& Deliverables);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void MarkAgentBlocked(int32 AgentNumber, const FString& BlockingReason);

    // Status queries
    UFUNCTION(BlueprintCallable, Category = "Production")
    FDir_ProductionMetrics GetCurrentMetrics();

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentTask> GetActiveTasks();

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentTask> GetBlockedTasks();

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool IsAgentAvailable(int32 AgentNumber);

    // Critical path management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void IdentifyCriticalPath();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void PrioritizePlayablePrototype();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void ValidateMinimumViableProduct();

    // Debug and monitoring
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void LogProductionStatus();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void GenerateProductionReport();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    EDir_ProductionPhase CurrentPhase = EDir_ProductionPhase::PrototypeDevelopment;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_AgentTask> ActiveTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FDir_ProductionMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TMap<int32, FString> AgentNames;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<int32> CriticalPathAgents;

    // Core agent priorities for playable prototype
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<int32> PrototypeAgents = {2, 3, 5, 9, 10, 12};

private:
    void InitializeAgentNames();
    void UpdateMapMetrics();
    void CheckPlayabilityStatus();
    void CalculateOverallProgress();
};