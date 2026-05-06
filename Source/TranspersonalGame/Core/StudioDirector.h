#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Engine.h"
#include "StudioDirector.generated.h"

// Forward declarations
class UTranspersonalGameInstance;

/**
 * Production priority levels for agent task coordination
 */
UENUM(BlueprintType)
enum class EDir_ProductionPriority : uint8
{
    Critical    UMETA(DisplayName = "Critical - Blocks all other work"),
    High        UMETA(DisplayName = "High - Must complete this cycle"),
    Medium      UMETA(DisplayName = "Medium - Complete if time allows"),
    Low         UMETA(DisplayName = "Low - Future cycle priority")
};

/**
 * Milestone tracking for production cycles
 */
UENUM(BlueprintType)
enum class EDir_MilestoneStatus : uint8
{
    NotStarted  UMETA(DisplayName = "Not Started"),
    InProgress  UMETA(DisplayName = "In Progress"),
    Blocked     UMETA(DisplayName = "Blocked - Waiting for dependency"),
    Complete    UMETA(DisplayName = "Complete"),
    Failed      UMETA(DisplayName = "Failed - Needs rework")
};

/**
 * Agent task assignment structure
 */
USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString AgentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_ProductionPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TArray<FString> Deliverables;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float EstimatedCycleTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_MilestoneStatus Status;

    FDir_AgentTask()
    {
        Priority = EDir_ProductionPriority::Medium;
        EstimatedCycleTime = 1.0f;
        Status = EDir_MilestoneStatus::NotStarted;
    }
};

/**
 * Production milestone tracking
 */
USTRUCT(BlueprintType)
struct FDir_ProductionMilestone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FString> SuccessCriteria;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FDir_AgentTask> RequiredTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    EDir_MilestoneStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FDateTime TargetDate;

    FDir_ProductionMilestone()
    {
        Status = EDir_MilestoneStatus::NotStarted;
        CompletionPercentage = 0.0f;
    }
};

/**
 * Studio Director - Production coordination and agent management system
 * 
 * The Studio Director acts as the central coordination hub for all 19 agents,
 * translating creative vision into technical tasks, tracking milestone progress,
 * and ensuring the production pipeline flows smoothly from concept to playable game.
 * 
 * Key responsibilities:
 * - Receive creative direction from Miguel and translate to agent tasks
 * - Coordinate agent dependencies and resolve conflicts
 * - Track milestone progress and identify blockers
 * - Maintain production quality standards
 * - Report final results back to creative leadership
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UStudioDirector : public UObject
{
    GENERATED_BODY()

public:
    UStudioDirector();

    // === CORE COORDINATION FUNCTIONS ===

    /**
     * Initialize a new production cycle with creative direction
     * @param CycleID Unique identifier for this production cycle
     * @param CreativeDirection High-level creative goals from Miguel
     */
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeProductionCycle(const FString& CycleID, const FString& CreativeDirection);

    /**
     * Assign tasks to specific agents based on current milestone needs
     * @param AgentID Target agent identifier (e.g., "Agent_02_Engine_Architect")
     * @param Task Task specification with priority and deliverables
     */
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AssignAgentTask(const FString& AgentID, const FDir_AgentTask& Task);

    /**
     * Update milestone progress based on agent completions
     * @param MilestoneName Name of the milestone to update
     * @param CompletedTasks List of completed task IDs
     */
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateMilestoneProgress(const FString& MilestoneName, const TArray<FString>& CompletedTasks);

    /**
     * Validate current production state and identify blockers
     * @return Array of critical issues that must be resolved
     */
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FString> ValidateProductionState();

    /**
     * Generate final production report for Miguel
     * @return Comprehensive status of all milestones and agent outputs
     */
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FString GenerateProductionReport();

    // === MILESTONE MANAGEMENT ===

    /**
     * Define a new production milestone
     * @param Milestone Complete milestone specification
     */
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void DefineMilestone(const FDir_ProductionMilestone& Milestone);

    /**
     * Get current milestone progress
     * @param MilestoneName Name of milestone to query
     * @return Current milestone status and completion percentage
     */
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_ProductionMilestone GetMilestoneStatus(const FString& MilestoneName);

    /**
     * Check if milestone is blocked by dependencies
     * @param MilestoneName Milestone to check
     * @return True if milestone is blocked and cannot proceed
     */
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool IsMilestoneBlocked(const FString& MilestoneName);

    // === AGENT COORDINATION ===

    /**
     * Get next agent in the production chain
     * @param CurrentAgentID Current agent completing work
     * @return ID of next agent to receive tasks
     */
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FString GetNextAgent(const FString& CurrentAgentID);

    /**
     * Resolve conflicts between agents
     * @param ConflictDescription Description of the conflict
     * @param InvolvedAgents List of agents involved in conflict
     * @return Resolution decision and instructions
     */
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FString ResolveAgentConflict(const FString& ConflictDescription, const TArray<FString>& InvolvedAgents);

    // === QUALITY ASSURANCE ===

    /**
     * Validate that agent output meets quality standards
     * @param AgentID Agent that produced the output
     * @param OutputDescription Description of what was produced
     * @return True if output meets standards, false if rework needed
     */
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool ValidateAgentOutput(const FString& AgentID, const FString& OutputDescription);

protected:
    // === PRODUCTION STATE ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production State")
    FString CurrentCycleID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production State")
    FString CreativeDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production State")
    TArray<FDir_ProductionMilestone> ActiveMilestones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production State")
    TMap<FString, FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production State")
    TArray<FString> ProductionBlockers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production State")
    FDateTime CycleStartTime;

    // === AGENT CHAIN CONFIGURATION ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Chain")
    TArray<FString> AgentExecutionOrder;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Chain")
    TMap<FString, FString> AgentDependencies;

    // === QUALITY METRICS ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality Metrics")
    int32 CompilationErrors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality Metrics")
    int32 MissingImplementations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality Metrics")
    float PerformanceFPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality Metrics")
    int32 PlayableFeatures;

private:
    // === INTERNAL COORDINATION LOGIC ===

    void InitializeAgentChain();
    void SetupMilestone1();
    bool ValidateCompilationState();
    void CleanupDuplicateActors();
    FString GenerateAgentInstructions(const FString& AgentID, const FDir_AgentTask& Task);
};