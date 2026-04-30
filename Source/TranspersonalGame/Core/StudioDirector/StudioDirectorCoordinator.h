#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/Engine.h"
#include "../../SharedTypes.h"
#include "StudioDirectorCoordinator.generated.h"

// Studio Director coordination enums
UENUM(BlueprintType)
enum class EDir_AgentPriority : uint8
{
    CRITICAL    UMETA(DisplayName = "Critical"),
    HIGH        UMETA(DisplayName = "High"),
    MEDIUM      UMETA(DisplayName = "Medium"),
    LOW         UMETA(DisplayName = "Low"),
    INACTIVE    UMETA(DisplayName = "Inactive")
};

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    WAITING     UMETA(DisplayName = "Waiting"),
    ACTIVE      UMETA(DisplayName = "Active"),
    COMPLETED   UMETA(DisplayName = "Completed"),
    BLOCKED     UMETA(DisplayName = "Blocked"),
    ERROR       UMETA(DisplayName = "Error")
};

UENUM(BlueprintType)
enum class EDir_MilestoneType : uint8
{
    PLAYABLE_PROTOTYPE      UMETA(DisplayName = "Playable Prototype"),
    CORE_SYSTEMS           UMETA(DisplayName = "Core Systems"),
    WORLD_GENERATION       UMETA(DisplayName = "World Generation"),
    CHARACTER_SYSTEMS      UMETA(DisplayName = "Character Systems"),
    AI_BEHAVIOR            UMETA(DisplayName = "AI Behavior"),
    SURVIVAL_MECHANICS     UMETA(DisplayName = "Survival Mechanics"),
    POLISH_PHASE           UMETA(DisplayName = "Polish Phase")
};

// Agent task structure
USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    EDir_AgentPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString ExpectedDeliverable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    float EstimatedCycles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    int32 CycleAssigned;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    int32 CycleCompleted;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Priority = EDir_AgentPriority::MEDIUM;
        Status = EDir_AgentStatus::WAITING;
        ExpectedDeliverable = TEXT("");
        EstimatedCycles = 1.0f;
        CycleAssigned = 0;
        CycleCompleted = 0;
    }
};

// Milestone tracking structure
USTRUCT(BlueprintType)
struct FDir_Milestone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    EDir_MilestoneType Type;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FString> RequiredDeliverables;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FString> CompletedDeliverables;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float ProgressPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    int32 TargetCycle;

    FDir_Milestone()
    {
        MilestoneName = TEXT("");
        Type = EDir_MilestoneType::PLAYABLE_PROTOTYPE;
        ProgressPercentage = 0.0f;
        bIsCompleted = false;
        TargetCycle = 0;
    }
};

// Production metrics structure
USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CurrentCycle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalFilesCreated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalUE5CommandsExecuted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 ActiveAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 BlockedAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float OverallProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    FString CurrentMilestone;

    FDir_ProductionMetrics()
    {
        CurrentCycle = 0;
        TotalFilesCreated = 0;
        TotalUE5CommandsExecuted = 0;
        ActiveAgents = 0;
        BlockedAgents = 0;
        OverallProgress = 0.0f;
        CurrentMilestone = TEXT("Playable Prototype");
    }
};

/**
 * Studio Director Coordinator - Central coordination system for all 19 agents
 * Manages task assignment, milestone tracking, and production metrics
 * Ensures agents work in proper sequence and dependencies are met
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UStudioDirectorCoordinator : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UStudioDirectorCoordinator();

    // Subsystem overrides
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Agent management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void RegisterAgent(const FString& AgentName, EDir_AgentPriority DefaultPriority);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AssignTask(const FString& AgentName, const FString& TaskDescription, 
                   EDir_AgentPriority Priority, const FString& ExpectedDeliverable);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CompleteAgentTask(const FString& AgentName, const FString& DeliverableDescription);

    // Milestone management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CreateMilestone(const FString& MilestoneName, EDir_MilestoneType Type, 
                        const TArray<FString>& RequiredDeliverables, int32 TargetCycle);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateMilestoneProgress(const FString& MilestoneName, const FString& CompletedDeliverable);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool IsMilestoneCompleted(const FString& MilestoneName) const;

    // Production metrics
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void IncrementCycle();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void RecordFileCreated();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void RecordUE5CommandExecuted();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_ProductionMetrics GetProductionMetrics() const;

    // Query functions
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentTask> GetTasksByPriority(EDir_AgentPriority Priority) const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentTask> GetBlockedTasks() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_AgentTask GetAgentTask(const FString& AgentName) const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_Milestone> GetActiveMilestones() const;

    // Coordination functions
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ExecuteProductionCycle();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FString GenerateProductionReport() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ResolveAgentConflict(const FString& Agent1, const FString& Agent2, const FString& Resolution);

protected:
    // Agent tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Studio Director")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Studio Director")
    TMap<FString, EDir_AgentStatus> AgentStatusMap;

    // Milestone tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Studio Director")
    TArray<FDir_Milestone> Milestones;

    // Production metrics
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Studio Director")
    FDir_ProductionMetrics ProductionMetrics;

    // Internal functions
    void InitializeDefaultAgents();
    void InitializeDefaultMilestones();
    void UpdateProductionMetrics();
    float CalculateOverallProgress() const;
    void LogProductionState() const;
};