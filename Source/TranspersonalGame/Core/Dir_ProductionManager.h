#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Dir_ProductionManager.generated.h"

/**
 * Production milestone tracking for development cycles
 */
UENUM(BlueprintType)
enum class EDir_ProductionMilestone : uint8
{
    None = 0,
    Milestone1_Playable,     // Player can walk around with visible dinosaurs
    Milestone2_Survival,     // Basic survival mechanics functional  
    Milestone3_AI,           // Dinosaur AI and behavior systems
    Milestone4_World,        // Complete world generation and biomes
    Milestone5_Polish        // Final polish and optimization
};

/**
 * Agent execution priority levels
 */
UENUM(BlueprintType)
enum class EDir_AgentPriority : uint8
{
    Critical = 0,    // Must complete before any other agent
    High = 1,        // High priority, blocks dependent agents
    Medium = 2,      // Normal priority
    Low = 3,         // Can be deferred if needed
    Maintenance = 4  // Background tasks only
};

/**
 * Production task status tracking
 */
USTRUCT(BlueprintType)
struct FDir_ProductionTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString ExpectedDeliverable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    EDir_AgentPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    bool bIsBlocked;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString BlockerReason;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float EstimatedCycles;

    FDir_ProductionTask()
    {
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        ExpectedDeliverable = TEXT("");
        Priority = EDir_AgentPriority::Medium;
        bIsCompleted = false;
        bIsBlocked = false;
        BlockerReason = TEXT("");
        EstimatedCycles = 1.0f;
    }
};

/**
 * Milestone progress tracking
 */
USTRUCT(BlueprintType)
struct FDir_MilestoneProgress
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    EDir_ProductionMilestone Milestone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FString> RequiredDeliverables;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FString> CompletedDeliverables;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float ProgressPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    bool bIsCompleted;

    FDir_MilestoneProgress()
    {
        Milestone = EDir_ProductionMilestone::None;
        Description = TEXT("");
        ProgressPercentage = 0.0f;
        bIsCompleted = false;
    }
};

/**
 * Studio Director's Production Management System
 * Coordinates agent tasks, tracks milestones, and manages production pipeline
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(TranspersonalGame))
class TRANSPERSONALGAME_API UDir_ProductionManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UDir_ProductionManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Production Management Functions
    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void InitializeProductionCycle(const FString& CycleID);

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void AddProductionTask(const FDir_ProductionTask& Task);

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void CompleteTask(const FString& AgentName, const FString& DeliverableDescription);

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void BlockTask(const FString& AgentName, const FString& BlockerReason);

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void UnblockTask(const FString& AgentName);

    // Milestone Management
    UFUNCTION(BlueprintCallable, Category = "Milestone Management")
    void SetCurrentMilestone(EDir_ProductionMilestone NewMilestone);

    UFUNCTION(BlueprintCallable, Category = "Milestone Management")
    void UpdateMilestoneProgress();

    UFUNCTION(BlueprintCallable, Category = "Milestone Management")
    float GetMilestoneProgress(EDir_ProductionMilestone Milestone) const;

    // Production Assessment
    UFUNCTION(BlueprintCallable, Category = "Production Assessment")
    void AssessCurrentProductionState();

    UFUNCTION(BlueprintCallable, Category = "Production Assessment")
    TArray<FDir_ProductionTask> GetBlockedTasks() const;

    UFUNCTION(BlueprintCallable, Category = "Production Assessment")
    TArray<FDir_ProductionTask> GetHighPriorityTasks() const;

    UFUNCTION(BlueprintCallable, Category = "Production Assessment")
    int32 GetCompletedTaskCount() const;

    UFUNCTION(BlueprintCallable, Category = "Production Assessment")
    int32 GetTotalTaskCount() const;

    // Agent Coordination
    UFUNCTION(BlueprintCallable, Category = "Agent Coordination")
    FString GetNextAgentToExecute() const;

    UFUNCTION(BlueprintCallable, Category = "Agent Coordination")
    bool CanAgentExecute(const FString& AgentName) const;

    UFUNCTION(BlueprintCallable, Category = "Agent Coordination")
    void GenerateAgentCoordinationReport();

    // Production Metrics
    UFUNCTION(BlueprintCallable, Category = "Production Metrics")
    void LogProductionMetrics();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Production Management")
    void InitializeMilestone1Tasks();

protected:
    // Current production state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production State")
    FString CurrentCycleID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production State")
    EDir_ProductionMilestone CurrentMilestone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production State")
    TArray<FDir_ProductionTask> ProductionTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production State")
    TArray<FDir_MilestoneProgress> MilestoneProgress;

    // Production metrics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    float CycleStartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 TasksCompletedThisCycle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 BlockersEncountered;

    // Internal functions
    void InitializeMilestoneData();
    void UpdateTaskDependencies();
    FDir_MilestoneProgress* FindMilestoneProgress(EDir_ProductionMilestone Milestone);
    bool AreTaskDependenciesMet(const FDir_ProductionTask& Task) const;
};