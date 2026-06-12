#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "StudioDirector.generated.h"

/**
 * Studio Director - Production Coordination and Agent Management System
 * Manages the 19-agent production pipeline for Transpersonal Game Studio
 * Tracks milestones, coordinates agent tasks, and monitors production metrics
 */

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    INACTIVE    UMETA(DisplayName = "Inactive"),
    PENDING     UMETA(DisplayName = "Pending"),
    ACTIVE      UMETA(DisplayName = "Active"),
    BLOCKED     UMETA(DisplayName = "Blocked"),
    COMPLETED   UMETA(DisplayName = "Completed")
};

UENUM(BlueprintType)
enum class EDir_TaskPriority : uint8
{
    LOW         UMETA(DisplayName = "Low"),
    MEDIUM      UMETA(DisplayName = "Medium"),
    HIGH        UMETA(DisplayName = "High"),
    CRITICAL    UMETA(DisplayName = "Critical")
};

UENUM(BlueprintType)
enum class EDir_MilestoneType : uint8
{
    PROTOTYPE   UMETA(DisplayName = "Playable Prototype"),
    ALPHA       UMETA(DisplayName = "Alpha Build"),
    BETA        UMETA(DisplayName = "Beta Build"),
    RELEASE     UMETA(DisplayName = "Release Build")
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    EDir_TaskPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString Deliverable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    float ProgressPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FDateTime LastUpdate;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        Status = EDir_AgentStatus::INACTIVE;
        Priority = EDir_TaskPriority::MEDIUM;
        Deliverable = TEXT("");
        ProgressPercentage = 0.0f;
        LastUpdate = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMilestone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    EDir_MilestoneType Type;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FString> RequiredDeliverables;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FDateTime TargetDate;

    FDir_ProductionMilestone()
    {
        MilestoneName = TEXT("");
        Type = EDir_MilestoneType::PROTOTYPE;
        CompletionPercentage = 0.0f;
        bIsCompleted = false;
        TargetDate = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 ActiveAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CompletedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float OverallProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 FilesCreated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 UE5CommandsExecuted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    FDateTime LastUpdate;

    FDir_ProductionMetrics()
    {
        TotalAgents = 19;
        ActiveAgents = 0;
        CompletedTasks = 0;
        TotalTasks = 0;
        OverallProgress = 0.0f;
        FilesCreated = 0;
        UE5CommandsExecuted = 0;
        LastUpdate = FDateTime::Now();
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AStudioDirector : public AActor
{
    GENERATED_BODY()

public:
    AStudioDirector();

protected:
    virtual void BeginPlay() override;

    // === AGENT MANAGEMENT ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Pipeline")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Pipeline")
    TArray<FDir_ProductionMilestone> ProductionMilestones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Pipeline")
    FDir_ProductionMetrics CurrentMetrics;

    // === MILESTONE TRACKING ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestones")
    bool bPrototypeCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestones")
    bool bAlphaBuildReady;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestones")
    bool bBetaBuildReady;

    // === PRODUCTION SETTINGS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MetricsUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bAutoUpdateMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bEnableDebugLogging;

public:
    // === CORE FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeProductionPipeline();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateAgentTask(const FString& AgentName, EDir_AgentStatus NewStatus, float Progress);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ValidateMilestones();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_ProductionMetrics CalculateProductionMetrics();

    // === AGENT COORDINATION ===
    UFUNCTION(BlueprintCallable, Category = "Agent Coordination")
    TArray<FDir_AgentTask> GetActiveAgentTasks();

    UFUNCTION(BlueprintCallable, Category = "Agent Coordination")
    TArray<FDir_AgentTask> GetBlockedAgentTasks();

    UFUNCTION(BlueprintCallable, Category = "Agent Coordination")
    void AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription, EDir_TaskPriority Priority);

    // === MILESTONE MANAGEMENT ===
    UFUNCTION(BlueprintCallable, Category = "Milestones")
    void CreateMilestone(const FString& Name, EDir_MilestoneType Type, const TArray<FString>& Requirements);

    UFUNCTION(BlueprintCallable, Category = "Milestones")
    float GetMilestoneProgress(const FString& MilestoneName);

    UFUNCTION(BlueprintCallable, Category = "Milestones")
    bool IsMilestoneCompleted(const FString& MilestoneName);

    // === PRODUCTION REPORTING ===
    UFUNCTION(BlueprintCallable, Category = "Reporting")
    FString GenerateProductionReport();

    UFUNCTION(BlueprintCallable, Category = "Reporting")
    void LogProductionStatus();

    UFUNCTION(BlueprintCallable, Category = "Reporting", CallInEditor = true)
    void RefreshProductionMetrics();

private:
    // === INTERNAL FUNCTIONS ===
    void SetupDefaultAgentTasks();
    void SetupDefaultMilestones();
    void UpdateMetricsTimer();
    
    FTimerHandle MetricsUpdateTimer;
};