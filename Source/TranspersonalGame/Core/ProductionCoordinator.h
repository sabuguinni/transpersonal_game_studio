#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "ProductionCoordinator.generated.h"

/**
 * Studio Director Production Coordination System
 * Manages agent task assignments, milestone tracking, and production metrics
 * for the Transpersonal Game Studio development pipeline
 */

UENUM(BlueprintType)
enum class EDir_AgentType : uint8
{
    StudioDirector      UMETA(DisplayName = "Studio Director"),
    EngineArchitect     UMETA(DisplayName = "Engine Architect"), 
    CoreSystems         UMETA(DisplayName = "Core Systems"),
    Performance         UMETA(DisplayName = "Performance Optimizer"),
    WorldGenerator      UMETA(DisplayName = "World Generator"),
    Environment         UMETA(DisplayName = "Environment Artist"),
    Architecture        UMETA(DisplayName = "Architecture & Interior"),
    Lighting            UMETA(DisplayName = "Lighting & Atmosphere"),
    Character           UMETA(DisplayName = "Character Artist"),
    Animation           UMETA(DisplayName = "Animation"),
    NPCBehavior         UMETA(DisplayName = "NPC Behavior"),
    CombatAI            UMETA(DisplayName = "Combat & Enemy AI"),
    CrowdSimulation     UMETA(DisplayName = "Crowd & Traffic"),
    QuestDesigner       UMETA(DisplayName = "Quest & Mission Designer"),
    Narrative           UMETA(DisplayName = "Narrative & Dialogue"),
    Audio               UMETA(DisplayName = "Audio"),
    VFX                 UMETA(DisplayName = "VFX"),
    QA                  UMETA(DisplayName = "QA & Testing"),
    Integration         UMETA(DisplayName = "Integration & Build")
};

UENUM(BlueprintType)
enum class EDir_TaskPriority : uint8
{
    Critical    UMETA(DisplayName = "Critical"),
    High        UMETA(DisplayName = "High"),
    Medium      UMETA(DisplayName = "Medium"),
    Low         UMETA(DisplayName = "Low")
};

UENUM(BlueprintType)
enum class EDir_TaskStatus : uint8
{
    Pending     UMETA(DisplayName = "Pending"),
    InProgress  UMETA(DisplayName = "In Progress"),
    Completed   UMETA(DisplayName = "Completed"),
    Blocked     UMETA(DisplayName = "Blocked")
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_AgentType AgentType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_TaskPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_TaskStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float EstimatedHours;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FDateTime CreatedTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FDateTime LastUpdated;

    FDir_AgentTask()
    {
        AgentType = EDir_AgentType::StudioDirector;
        Priority = EDir_TaskPriority::Medium;
        Status = EDir_TaskStatus::Pending;
        EstimatedHours = 1.0f;
        CompletionPercentage = 0.0f;
        CreatedTime = FDateTime::Now();
        LastUpdated = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FDir_MilestoneData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FString> RequiredDeliverables;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FDateTime TargetDate;

    FDir_MilestoneData()
    {
        bIsCompleted = false;
        CompletionPercentage = 0.0f;
        TargetDate = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalActorsInLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CompletedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 PendingTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float OverallProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 ActiveAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    FDateTime LastUpdate;

    FDir_ProductionMetrics()
    {
        TotalActorsInLevel = 0;
        CompletedTasks = 0;
        PendingTasks = 0;
        OverallProgress = 0.0f;
        ActiveAgents = 0;
        LastUpdate = FDateTime::Now();
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AProductionCoordinator : public AActor
{
    GENERATED_BODY()

public:
    AProductionCoordinator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // === AGENT TASK MANAGEMENT ===
    
    UFUNCTION(BlueprintCallable, Category = "Production|Tasks")
    void AssignTaskToAgent(EDir_AgentType AgentType, const FString& TaskDescription, EDir_TaskPriority Priority = EDir_TaskPriority::Medium);

    UFUNCTION(BlueprintCallable, Category = "Production|Tasks")
    void UpdateTaskStatus(EDir_AgentType AgentType, const FString& TaskDescription, EDir_TaskStatus NewStatus, float CompletionPercentage = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Production|Tasks")
    TArray<FDir_AgentTask> GetTasksForAgent(EDir_AgentType AgentType) const;

    UFUNCTION(BlueprintCallable, Category = "Production|Tasks")
    TArray<FDir_AgentTask> GetAllTasks() const;

    UFUNCTION(BlueprintCallable, Category = "Production|Tasks")
    void ClearCompletedTasks();

    // === MILESTONE TRACKING ===

    UFUNCTION(BlueprintCallable, Category = "Production|Milestones")
    void AddMilestone(const FString& MilestoneName, const TArray<FString>& RequiredDeliverables, const FDateTime& TargetDate);

    UFUNCTION(BlueprintCallable, Category = "Production|Milestones")
    void UpdateMilestoneProgress(const FString& MilestoneName, float CompletionPercentage, bool bMarkCompleted = false);

    UFUNCTION(BlueprintCallable, Category = "Production|Milestones")
    TArray<FDir_MilestoneData> GetAllMilestones() const;

    UFUNCTION(BlueprintCallable, Category = "Production|Milestones")
    float GetOverallMilestoneProgress() const;

    // === PRODUCTION METRICS ===

    UFUNCTION(BlueprintCallable, Category = "Production|Metrics")
    FDir_ProductionMetrics CalculateCurrentMetrics();

    UFUNCTION(BlueprintCallable, Category = "Production|Metrics")
    void UpdateLevelActorCount();

    UFUNCTION(BlueprintCallable, Category = "Production|Metrics")
    int32 GetActiveAgentCount() const;

    // === COORDINATION UTILITIES ===

    UFUNCTION(BlueprintCallable, Category = "Production|Coordination")
    void GenerateProductionReport();

    UFUNCTION(BlueprintCallable, Category = "Production|Coordination")
    TArray<EDir_AgentType> GetCriticalPathAgents() const;

    UFUNCTION(BlueprintCallable, Category = "Production|Coordination")
    void ValidateAgentDependencies();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Production|Debug")
    void InitializeDefaultTasks();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Production|Debug")
    void DebugPrintAllTasks();

protected:
    // === CORE DATA ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production", meta = (AllowPrivateAccess = "true"))
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production", meta = (AllowPrivateAccess = "true"))
    TArray<FDir_MilestoneData> ProjectMilestones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production", meta = (AllowPrivateAccess = "true"))
    FDir_ProductionMetrics CurrentMetrics;

    // === CONFIGURATION ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration", meta = (AllowPrivateAccess = "true"))
    float MetricsUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration", meta = (AllowPrivateAccess = "true"))
    bool bAutoGenerateReports;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration", meta = (AllowPrivateAccess = "true"))
    bool bEnableDebugLogging;

private:
    float LastMetricsUpdate;
    
    void InitializeMilestone1Tasks();
    void LogProductionMessage(const FString& Message) const;
};