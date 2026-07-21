#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Dir_ProductionCoordinator.generated.h"

/**
 * Production Coordinator Component
 * Manages agent task coordination and milestone tracking for the playable prototype
 * Used by Studio Director to monitor and coordinate development progress
 */

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Working     UMETA(DisplayName = "Working"),
    Completed   UMETA(DisplayName = "Completed"),
    Blocked     UMETA(DisplayName = "Blocked"),
    Failed      UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EDir_MilestonePhase : uint8
{
    Infrastructure  UMETA(DisplayName = "Infrastructure"),
    WalkAround     UMETA(DisplayName = "Walk Around"),
    BasicGameplay  UMETA(DisplayName = "Basic Gameplay"),
    Polish         UMETA(DisplayName = "Polish"),
    Complete       UMETA(DisplayName = "Complete")
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    int32 Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    int32 CycleAssigned;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    int32 CycleDeadline;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString Dependencies;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Status = EDir_AgentStatus::Idle;
        Priority = 0;
        CycleAssigned = 0;
        CycleDeadline = 0;
        Dependencies = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct FDir_MilestoneProgress
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    EDir_MilestonePhase Phase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    int32 TargetCycle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FString> RequiredDeliverables;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FString> CompletedDeliverables;

    FDir_MilestoneProgress()
    {
        MilestoneName = TEXT("");
        Phase = EDir_MilestonePhase::Infrastructure;
        CompletionPercentage = 0.0f;
        TargetCycle = 0;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDir_ProductionCoordinator : public UActorComponent
{
    GENERATED_BODY()

public:
    UDir_ProductionCoordinator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Agent Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_AgentTask> ActiveTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_AgentTask> CompletedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 CurrentCycle;

    // Milestone Tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestones")
    TArray<FDir_MilestoneProgress> Milestones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestones")
    EDir_MilestonePhase CurrentPhase;

    // Production Metrics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalActorsInWorld;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 DinosaurCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 EnvironmentPropsCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    bool bPlayablePrototypeReady;

    // Task Management Functions
    UFUNCTION(BlueprintCallable, Category = "Production")
    void AssignTask(const FString& AgentName, const FString& TaskDescription, int32 Priority, int32 Deadline);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateTaskStatus(const FString& AgentName, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void CompleteTask(const FString& AgentName);

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentTask> GetTasksByStatus(EDir_AgentStatus Status);

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentTask> GetOverdueTasks();

    // Milestone Functions
    UFUNCTION(BlueprintCallable, Category = "Milestones")
    void InitializeMilestones();

    UFUNCTION(BlueprintCallable, Category = "Milestones")
    void UpdateMilestoneProgress(const FString& MilestoneName, float NewPercentage);

    UFUNCTION(BlueprintCallable, Category = "Milestones")
    void AddDeliverable(const FString& MilestoneName, const FString& Deliverable);

    UFUNCTION(BlueprintCallable, Category = "Milestones")
    void CompleteDeliverable(const FString& MilestoneName, const FString& Deliverable);

    UFUNCTION(BlueprintCallable, Category = "Milestones")
    bool IsMilestoneComplete(const FString& MilestoneName);

    // World State Analysis
    UFUNCTION(BlueprintCallable, Category = "Analysis")
    void AnalyzeWorldState();

    UFUNCTION(BlueprintCallable, Category = "Analysis")
    void CheckPlayablePrototypeRequirements();

    UFUNCTION(BlueprintCallable, Category = "Analysis")
    FString GenerateProductionReport();

    // Critical Path Management
    UFUNCTION(BlueprintCallable, Category = "Critical Path")
    TArray<FDir_AgentTask> GetCriticalPathTasks();

    UFUNCTION(BlueprintCallable, Category = "Critical Path")
    void IdentifyBlockedTasks();

    UFUNCTION(BlueprintCallable, Category = "Critical Path")
    void ReassignBlockedTasks();

    // Editor Functions
    UFUNCTION(CallInEditor, Category = "Production Tools")
    void RefreshProductionStatus();

    UFUNCTION(CallInEditor, Category = "Production Tools")
    void GenerateCycleReport();

    UFUNCTION(CallInEditor, Category = "Production Tools")
    void ValidatePlayablePrototype();

private:
    void InitializeDefaultTasks();
    void UpdateWorldMetrics();
    FString GetAgentStatusString(EDir_AgentStatus Status);
    FString GetMilestonePhaseString(EDir_MilestonePhase Phase);
};