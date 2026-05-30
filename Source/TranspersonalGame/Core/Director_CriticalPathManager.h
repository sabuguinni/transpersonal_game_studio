#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Director_CriticalPathManager.generated.h"

UENUM(BlueprintType)
enum class EDir_TaskStatus : uint8
{
    NotStarted      UMETA(DisplayName = "Not Started"),
    InProgress      UMETA(DisplayName = "In Progress"),
    Blocked         UMETA(DisplayName = "Blocked"),
    Completed       UMETA(DisplayName = "Completed"),
    Failed          UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EDir_MilestoneType : uint8
{
    WalkAround      UMETA(DisplayName = "Walk Around"),
    BasicSurvival   UMETA(DisplayName = "Basic Survival"),
    CombatDanger    UMETA(DisplayName = "Combat & Danger"),
    WorldBuilding   UMETA(DisplayName = "World Building"),
    Polish          UMETA(DisplayName = "Polish")
};

USTRUCT(BlueprintType)
struct FDir_CriticalTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString TaskName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    int32 ResponsibleAgentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_TaskStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    int32 TargetCycle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    int32 Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TArray<int32> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString FilePath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString ValidationCriteria;

    FDir_CriticalTask()
    {
        TaskName = TEXT("");
        ResponsibleAgentID = 0;
        Status = EDir_TaskStatus::NotStarted;
        TargetCycle = 0;
        Priority = 0;
        FilePath = TEXT("");
        ValidationCriteria = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct FDir_MilestoneProgress
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    EDir_MilestoneType Type;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    int32 TargetCycle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    int32 CompletedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    int32 TotalTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bIsBlocked;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FString> BlockingReasons;

    FDir_MilestoneProgress()
    {
        Type = EDir_MilestoneType::WalkAround;
        Name = TEXT("");
        TargetCycle = 0;
        CompletedTasks = 0;
        TotalTasks = 0;
        CompletionPercentage = 0.0f;
        bIsBlocked = false;
    }
};

USTRUCT(BlueprintType)
struct FDir_AgentWorkload
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    int32 AgentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    int32 AssignedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    int32 CompletedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    int32 BlockedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    float WorkloadPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    bool bIsBottleneck;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    TArray<FString> CurrentTasks;

    FDir_AgentWorkload()
    {
        AgentID = 0;
        AgentName = TEXT("");
        AssignedTasks = 0;
        CompletedTasks = 0;
        BlockedTasks = 0;
        WorkloadPercentage = 0.0f;
        bIsBottleneck = false;
    }
};

/**
 * Critical Path Manager - Tracks and manages the critical path for Milestone 1 completion
 * Ensures all blocking tasks are identified and resolved for the "Walk Around" prototype
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDir_CriticalPathManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UDir_CriticalPathManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Critical Path Management
    UFUNCTION(BlueprintCallable, Category = "Critical Path")
    void InitializeCriticalTasks();

    UFUNCTION(BlueprintCallable, Category = "Critical Path")
    void UpdateTaskStatus(const FString& TaskName, EDir_TaskStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Critical Path")
    void ValidateCriticalPath();

    UFUNCTION(BlueprintCallable, Category = "Critical Path")
    TArray<FDir_CriticalTask> GetBlockingTasks() const;

    UFUNCTION(BlueprintCallable, Category = "Critical Path")
    TArray<FDir_CriticalTask> GetCriticalPathTasks() const;

    // Milestone Management
    UFUNCTION(BlueprintCallable, Category = "Milestone")
    void UpdateMilestoneProgress();

    UFUNCTION(BlueprintCallable, Category = "Milestone")
    FDir_MilestoneProgress GetCurrentMilestoneStatus() const;

    UFUNCTION(BlueprintCallable, Category = "Milestone")
    bool IsMilestoneOnTrack(EDir_MilestoneType MilestoneType) const;

    // Agent Workload Management
    UFUNCTION(BlueprintCallable, Category = "Agent Management")
    void AnalyzeAgentWorkloads();

    UFUNCTION(BlueprintCallable, Category = "Agent Management")
    TArray<FDir_AgentWorkload> GetAgentWorkloads() const;

    UFUNCTION(BlueprintCallable, Category = "Agent Management")
    TArray<int32> GetBottleneckAgents() const;

    UFUNCTION(BlueprintCallable, Category = "Agent Management")
    void RebalanceWorkload();

    // Validation and Monitoring
    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateTaskCompletion(const FString& TaskName);

    UFUNCTION(BlueprintCallable, Category = "Validation")
    void RunCriticalPathDiagnostics();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    void GenerateProgressReport();

    // Emergency Protocols
    UFUNCTION(BlueprintCallable, Category = "Emergency")
    void TriggerCriticalPathEmergency();

    UFUNCTION(BlueprintCallable, Category = "Emergency")
    void EscalateMilestoneDelay();

    UFUNCTION(BlueprintCallable, Category = "Emergency")
    void ActivateAgentReallocation();

protected:
    // Core Data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Critical Path")
    TArray<FDir_CriticalTask> CriticalTasks;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Milestone")
    TArray<FDir_MilestoneProgress> Milestones;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Agent Management")
    TArray<FDir_AgentWorkload> AgentWorkloads;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    int32 CurrentCycle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    int32 Milestone1TargetCycle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float CriticalPathThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEmergencyMode;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bAutoRebalance;

    // Monitoring
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monitoring")
    float LastValidationTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monitoring")
    int32 TotalBlockingTasks;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monitoring")
    int32 CompletedCriticalTasks;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monitoring")
    float OverallProgress;

private:
    // Internal Methods
    void InitializeMilestone1Tasks();
    void InitializeMilestone2Tasks();
    void InitializeMilestone3Tasks();
    
    void UpdateTaskDependencies();
    void CalculateCriticalPath();
    void IdentifyBottlenecks();
    
    bool ValidateCharacterMovement();
    bool ValidateTerrainGeneration();
    bool ValidateDinosaurPlacement();
    bool ValidateLightingSetup();
    
    void LogCriticalPathStatus();
    void LogMilestoneProgress();
    void LogAgentBottlenecks();
    
    FDir_CriticalTask* FindTaskByName(const FString& TaskName);
    FDir_AgentWorkload* FindAgentWorkload(int32 AgentID);
    
    void RecalculateProgress();
    void UpdateDependencyChain();
    void ValidateFileExistence();
};

#include "Director_CriticalPathManager.generated.h"