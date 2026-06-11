#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "ProductionCoordinator.generated.h"

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction,
    Milestone1_WalkAround,
    Milestone2_BasicSurvival,
    Milestone3_DinosaurInteraction,
    Milestone4_FullGameplay,
    Production,
    Polish
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 AgentID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString CurrentTask;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDir_AgentStatus Status = EDir_AgentStatus::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ProgressPercent = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString LastOutput;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime LastUpdate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> Deliverables;
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalCycles = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CompletedTasks = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ActiveAgents = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 BlockedAgents = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float OverallProgress = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString CurrentMilestone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime ProjectStartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime LastMilestoneComplete;
};

/**
 * Studio Director Production Coordinator
 * Manages the 19-agent production pipeline for Transpersonal Game Studio
 * Tracks agent progress, dependencies, and milestone completion
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UProductionCoordinator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UProductionCoordinator();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Production management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void InitializeProductionPipeline();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus, const FString& TaskDescription);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void SetAgentProgress(int32 AgentID, float ProgressPercent);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void CompleteAgentTask(int32 AgentID, const TArray<FString>& Deliverables);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void BlockAgent(int32 AgentID, const FString& BlockReason);

    // Milestone tracking
    UFUNCTION(BlueprintCallable, Category = "Milestones")
    void SetCurrentMilestone(EDir_ProductionPhase Phase);

    UFUNCTION(BlueprintCallable, Category = "Milestones")
    bool CheckMilestone1Completion() const;

    UFUNCTION(BlueprintCallable, Category = "Milestones")
    void ValidatePlayablePrototype();

    // Agent coordination
    UFUNCTION(BlueprintCallable, Category = "Coordination")
    TArray<FDir_AgentTask> GetAgentQueue() const;

    UFUNCTION(BlueprintCallable, Category = "Coordination")
    FDir_AgentTask GetAgentTask(int32 AgentID) const;

    UFUNCTION(BlueprintCallable, Category = "Coordination")
    TArray<int32> GetBlockedAgents() const;

    UFUNCTION(BlueprintCallable, Category = "Coordination")
    void AssignNextTask(int32 AgentID, const FString& TaskDescription, const TArray<FString>& Dependencies);

    // Production metrics
    UFUNCTION(BlueprintCallable, Category = "Metrics")
    FDir_ProductionMetrics GetProductionMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Metrics")
    void IncrementCycleCount();

    UFUNCTION(BlueprintCallable, Category = "Metrics")
    float CalculateOverallProgress() const;

    // Debug and monitoring
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void PrintProductionStatus();

    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void GenerateProductionReport();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogAgentActivity(int32 AgentID, const FString& Activity);

protected:
    // Agent management
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Production")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Production")
    EDir_ProductionPhase CurrentPhase = EDir_ProductionPhase::Milestone1_WalkAround;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Production")
    FDir_ProductionMetrics ProductionMetrics;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    TMap<int32, FString> AgentNames;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    TMap<int32, TArray<int32>> AgentDependencies;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    float MilestoneCompletionThreshold = 0.8f;

private:
    void InitializeAgentConfiguration();
    void SetupAgentDependencies();
    bool ValidateAgentDependencies(int32 AgentID) const;
    void UpdateProductionMetrics();
    
    FTimerHandle ProductionUpdateTimer;
    void OnProductionUpdate();
};

#include "ProductionCoordinator.generated.h"