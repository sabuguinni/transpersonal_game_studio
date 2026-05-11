#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "Dir_StudioDirector.generated.h"

/**
 * Studio Director Production Coordination System
 * Manages the 18-agent development pipeline for Transpersonal Game Studio
 * Tracks production metrics, agent deliverables, and milestone progress
 */

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction       UMETA(DisplayName = "Pre-Production"),
    PrototypeDevelopment UMETA(DisplayName = "Prototype Development"), 
    VerticalSlice       UMETA(DisplayName = "Vertical Slice"),
    Production          UMETA(DisplayName = "Production"),
    Alpha               UMETA(DisplayName = "Alpha"),
    Beta                UMETA(DisplayName = "Beta"),
    Release             UMETA(DisplayName = "Release")
};

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle                UMETA(DisplayName = "Idle"),
    Working             UMETA(DisplayName = "Working"),
    Completed           UMETA(DisplayName = "Completed"),
    Blocked             UMETA(DisplayName = "Blocked"),
    Failed              UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EDir_Priority : uint8
{
    Low                 UMETA(DisplayName = "Low"),
    Medium              UMETA(DisplayName = "Medium"),
    High                UMETA(DisplayName = "High"),
    Critical            UMETA(DisplayName = "Critical"),
    Emergency           UMETA(DisplayName = "Emergency")
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString ExpectedDeliverable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_Priority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float EstimatedTimeSeconds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float ActualTimeSeconds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FDateTime StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FDateTime CompletionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TArray<FString> BlockingIssues;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        ExpectedDeliverable = TEXT("");
        Priority = EDir_Priority::Medium;
        Status = EDir_AgentStatus::Idle;
        EstimatedTimeSeconds = 120.0f;
        ActualTimeSeconds = 0.0f;
        StartTime = FDateTime::Now();
        CompletionTime = FDateTime::MinValue();
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalActorsInLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CharacterActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 DinosaurActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 EnvironmentActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    bool bCompilationSuccessful;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    bool bLevelPlayable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float FrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float MemoryUsageMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CompletedAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 BlockedAgents;

    FDir_ProductionMetrics()
    {
        TotalActorsInLevel = 0;
        CharacterActors = 0;
        DinosaurActors = 0;
        EnvironmentActors = 0;
        bCompilationSuccessful = false;
        bLevelPlayable = false;
        FrameRate = 0.0f;
        MemoryUsageMB = 0.0f;
        CompletedAgents = 0;
        BlockedAgents = 0;
    }
};

USTRUCT(BlueprintType)
struct FDir_MilestoneProgress
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FString> RequiredDeliverables;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FString> CompletedDeliverables;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FDateTime TargetDate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bIsBlocking;

    FDir_MilestoneProgress()
    {
        MilestoneName = TEXT("");
        Description = TEXT("");
        CompletionPercentage = 0.0f;
        TargetDate = FDateTime::Now();
        bIsBlocking = false;
    }
};

/**
 * Studio Director Subsystem
 * Central coordination system for the 18-agent development pipeline
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDir_StudioDirectorSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Production Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeProductionPipeline();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void StartNewProductionCycle(const FString& CycleID);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void SetProductionPhase(EDir_ProductionPhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    EDir_ProductionPhase GetCurrentProductionPhase() const { return CurrentPhase; }

    // Agent Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void RegisterAgent(const FString& AgentName, int32 AgentNumber);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AssignTaskToAgent(const FString& AgentName, const FDir_AgentTask& Task);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus Status);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CompleteAgentTask(const FString& AgentName, const FString& DeliverableDescription);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void BlockAgent(const FString& AgentName, const FString& BlockingReason);

    // Metrics and Monitoring
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_ProductionMetrics GetCurrentMetrics() const { return CurrentMetrics; }

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void LogProductionEvent(const FString& EventDescription, EDir_Priority Priority);

    // Milestone Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CreateMilestone(const FString& Name, const FString& Description, const TArray<FString>& Deliverables);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateMilestoneProgress(const FString& MilestoneName, float CompletionPercentage);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool IsMilestoneComplete(const FString& MilestoneName) const;

    // Emergency Controls
    UFUNCTION(BlueprintCallable, Category = "Studio Director", CallInEditor = true)
    void EmergencyStopAllAgents();

    UFUNCTION(BlueprintCallable, Category = "Studio Director", CallInEditor = true)
    void ResetProductionPipeline();

    UFUNCTION(BlueprintCallable, Category = "Studio Director", CallInEditor = true)
    void GenerateProductionReport();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Production")
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    FString CurrentCycleID;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    TMap<FString, FDir_AgentTask> ActiveTasks;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    TMap<FString, int32> RegisteredAgents;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    FDir_ProductionMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    TArray<FDir_MilestoneProgress> ActiveMilestones;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    TArray<FString> ProductionLog;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    bool bEmergencyStop;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    FDateTime CycleStartTime;

private:
    void InitializeDefaultMilestones();
    void CalculateAgentMetrics();
    void CheckForBlockingIssues();
    void ValidateProductionState();
};