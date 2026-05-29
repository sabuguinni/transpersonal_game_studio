#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Director_ProductionPipeline.generated.h"

// Production phase tracking
UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction,
    CoreSystems,
    WorldGeneration,
    CharacterSystems,
    GameplayMechanics,
    Polish,
    Testing,
    Release
};

// Agent status tracking
UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle,
    Working,
    Blocked,
    Complete,
    Error
};

// Critical milestone definitions
USTRUCT(BlueprintType)
struct FDir_Milestone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<int32> RequiredAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bIsComplete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float Priority;

    FDir_Milestone()
    {
        Name = TEXT("");
        Description = TEXT("");
        bIsComplete = false;
        Priority = 1.0f;
    }
};

// Agent task definition
USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    int32 AgentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString TaskName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TArray<FString> Deliverables;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float EstimatedHours;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TArray<int32> Dependencies;

    FDir_AgentTask()
    {
        AgentID = 0;
        TaskName = TEXT("");
        Description = TEXT("");
        Status = EDir_AgentStatus::Idle;
        EstimatedHours = 1.0f;
    }
};

// Production metrics tracking
USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalCycles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CompletedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 ActiveAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float OverallProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    TArray<FString> CriticalBlocks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float BudgetUsed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float BudgetRemaining;

    FDir_ProductionMetrics()
    {
        TotalCycles = 0;
        CompletedTasks = 0;
        ActiveAgents = 0;
        OverallProgress = 0.0f;
        BudgetUsed = 0.0f;
        BudgetRemaining = 100.0f;
    }
};

/**
 * Production Pipeline Manager - Studio Director's main coordination system
 * Manages the 19-agent production chain and ensures milestone delivery
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDir_ProductionPipeline : public UActorComponent
{
    GENERATED_BODY()

public:
    UDir_ProductionPipeline();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core pipeline management
    UFUNCTION(BlueprintCallable, Category = "Production Pipeline")
    void InitializeProductionChain();

    UFUNCTION(BlueprintCallable, Category = "Production Pipeline")
    void AdvanceToNextPhase();

    UFUNCTION(BlueprintCallable, Category = "Production Pipeline")
    bool ValidateMilestone(const FString& MilestoneName);

    UFUNCTION(BlueprintCallable, Category = "Production Pipeline")
    void AssignTaskToAgent(int32 AgentID, const FDir_AgentTask& Task);

    UFUNCTION(BlueprintCallable, Category = "Production Pipeline")
    void UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Production Pipeline")
    TArray<FDir_AgentTask> GetCriticalPath();

    UFUNCTION(BlueprintCallable, Category = "Production Pipeline")
    void ResolveAgentConflict(int32 AgentA, int32 AgentB, const FString& Resolution);

    // Milestone management
    UFUNCTION(BlueprintCallable, Category = "Milestones")
    void RegisterMilestone(const FDir_Milestone& Milestone);

    UFUNCTION(BlueprintCallable, Category = "Milestones")
    void CompleteMilestone(const FString& MilestoneName);

    UFUNCTION(BlueprintCallable, Category = "Milestones")
    TArray<FDir_Milestone> GetPendingMilestones();

    UFUNCTION(BlueprintCallable, Category = "Milestones")
    float GetMilestoneProgress(const FString& MilestoneName);

    // Production metrics
    UFUNCTION(BlueprintCallable, Category = "Metrics")
    FDir_ProductionMetrics GetProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Metrics")
    void UpdateBudgetTracking(float CostDelta);

    UFUNCTION(BlueprintCallable, Category = "Metrics")
    TArray<FString> GetProductionReport();

    // Critical system validation
    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateMinimumViablePrototype();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateCharacterMovement();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateDinosaurAI();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateTerrainSystem();

protected:
    // Core production state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production State")
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production State")
    TArray<FDir_Milestone> Milestones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production State")
    TArray<FDir_AgentTask> ActiveTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production State")
    TMap<int32, EDir_AgentStatus> AgentStatuses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production State")
    FDir_ProductionMetrics Metrics;

    // Critical milestone tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Critical Milestones")
    bool bCharacterMovementComplete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Critical Milestones")
    bool bSurvivalSystemComplete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Critical Milestones")
    bool bDinosaurAIComplete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Critical Milestones")
    bool bTerrainSystemComplete;

    // Agent coordination
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Coordination")
    TArray<FString> AgentNames;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Coordination")
    TMap<int32, TArray<int32>> AgentDependencies;

    // Production timing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Timing")
    float CycleStartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Timing")
    float TargetCycleTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Timing")
    int32 CurrentCycleNumber;

private:
    // Internal pipeline management
    void InitializeAgentChain();
    void ValidateCriticalPath();
    void UpdateProductionMetrics();
    bool CheckMilestoneDependencies(const FDir_Milestone& Milestone);
    void TriggerEmergencyProtocol(const FString& Reason);
};

#include "Director_ProductionPipeline.generated.h"