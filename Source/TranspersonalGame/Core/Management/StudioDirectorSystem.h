#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "StudioDirectorSystem.generated.h"

/**
 * Production Priority Levels for Agent Task Management
 */
UENUM(BlueprintType)
enum class EDir_ProductionPriority : uint8
{
    CRITICAL    UMETA(DisplayName = "Critical"),
    HIGH        UMETA(DisplayName = "High"), 
    MEDIUM      UMETA(DisplayName = "Medium"),
    LOW         UMETA(DisplayName = "Low"),
    BLOCKED     UMETA(DisplayName = "Blocked")
};

/**
 * Agent Status Types for Production Tracking
 */
UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    READY       UMETA(DisplayName = "Ready"),
    WORKING     UMETA(DisplayName = "Working"),
    PENDING     UMETA(DisplayName = "Pending"),
    BLOCKED     UMETA(DisplayName = "Blocked"),
    COMPLETE    UMETA(DisplayName = "Complete"),
    ERROR       UMETA(DisplayName = "Error")
};

/**
 * Production Milestone Tracking
 */
UENUM(BlueprintType)
enum class EDir_MilestoneType : uint8
{
    LANDSCAPE_EXPANSION     UMETA(DisplayName = "Landscape Expansion"),
    ATMOSPHERE_SETUP        UMETA(DisplayName = "Atmosphere Setup"),
    FBX_PIPELINE           UMETA(DisplayName = "FBX Pipeline"),
    PLAYABLE_PROTOTYPE     UMETA(DisplayName = "Playable Prototype"),
    ASSET_INTEGRATION      UMETA(DisplayName = "Asset Integration")
};

/**
 * Agent Task Information Structure
 */
USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString AgentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    EDir_ProductionPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    float EstimatedDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    float ActualDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString LastUpdate;

    FDir_AgentTask()
    {
        AgentID = TEXT("");
        TaskDescription = TEXT("");
        Status = EDir_AgentStatus::READY;
        Priority = EDir_ProductionPriority::MEDIUM;
        EstimatedDuration = 0.0f;
        ActualDuration = 0.0f;
        LastUpdate = TEXT("");
    }
};

/**
 * Production Metrics Structure
 */
USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 TotalAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 ActiveAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 CompletedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 BlockedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    float CycleEfficiency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    float AverageTaskDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    FString CurrentMilestone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    float MilestoneProgress;

    FDir_ProductionMetrics()
    {
        TotalAgents = 19;
        ActiveAgents = 0;
        CompletedTasks = 0;
        BlockedTasks = 0;
        CycleEfficiency = 0.0f;
        AverageTaskDuration = 0.0f;
        CurrentMilestone = TEXT("Playable Prototype");
        MilestoneProgress = 0.0f;
    }
};

/**
 * Studio Director System Component
 * Manages agent coordination, task tracking, and production metrics
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UStudioDirectorSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UStudioDirectorSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Agent Task Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    FDir_ProductionMetrics ProductionMetrics;

    // Critical Path Tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Critical Path")
    bool bLandscapeExpansionComplete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Critical Path")
    bool bAtmosphereSetupComplete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Critical Path")
    bool bFBXPipelineComplete;

    // Production Control Functions
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeAgentTasks();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateAgentTask(const FString& AgentID, EDir_AgentStatus NewStatus, const FString& UpdateMessage);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CalculateProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_AgentTask GetAgentTask(const FString& AgentID);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentTask> GetTasksByPriority(EDir_ProductionPriority Priority);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentTask> GetBlockedTasks();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool CheckMilestoneCompletion(EDir_MilestoneType Milestone);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void LogProductionStatus();

    // Critical Path Management
    UFUNCTION(BlueprintCallable, Category = "Critical Path")
    void UpdateCriticalPathStatus();

    UFUNCTION(BlueprintCallable, Category = "Critical Path")
    float GetOverallProgress();

    UFUNCTION(BlueprintCallable, Category = "Critical Path")
    FString GetNextCriticalTask();

protected:
    // Internal tracking
    UPROPERTY()
    float LastMetricsUpdate;

    UPROPERTY()
    TMap<FString, float> AgentStartTimes;

    // Helper functions
    void InitializeDefaultTasks();
    void ValidateTaskDependencies();
    void UpdateTaskTimings();
};

/**
 * Studio Director Actor
 * Central coordination point for the entire production pipeline
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AStudioDirectorActor : public AActor
{
    GENERATED_BODY()

public:
    AStudioDirectorActor();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Studio Director")
    class UStudioDirectorSystem* DirectorSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    bool bAutoUpdateMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    float MetricsUpdateInterval;

    // Production Dashboard Functions
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Studio Director")
    void RefreshProductionDashboard();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Studio Director")
    void GenerateProductionReport();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Studio Director")
    void ResetAllAgentTasks();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Studio Director")
    void ValidateProjectStructure();

protected:
    UPROPERTY()
    float LastDashboardUpdate;
};