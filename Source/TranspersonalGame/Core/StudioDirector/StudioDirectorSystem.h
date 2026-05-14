#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Containers/Array.h"
#include "Containers/Map.h"
#include "UObject/ObjectMacros.h"
#include "StudioDirectorSystem.generated.h"

// Forward declarations
class AActor;
class ULevel;

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Working UMETA(DisplayName = "Working"), 
    Completed UMETA(DisplayName = "Completed"),
    Failed UMETA(DisplayName = "Failed"),
    Blocked UMETA(DisplayName = "Blocked")
};

UENUM(BlueprintType)
enum class EDir_TaskPriority : uint8
{
    Low UMETA(DisplayName = "Low"),
    Medium UMETA(DisplayName = "Medium"),
    High UMETA(DisplayName = "High"),
    Critical UMETA(DisplayName = "Critical"),
    Emergency UMETA(DisplayName = "Emergency")
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
    EDir_TaskPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float EstimatedDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float ActualDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString Output;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Priority = EDir_TaskPriority::Medium;
        Status = EDir_AgentStatus::Idle;
        EstimatedDuration = 0.0f;
        ActualDuration = 0.0f;
        Dependencies = TEXT("");
        Output = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalCycles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CompletedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 FailedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float AverageTaskDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 FilesCreated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 UE5CommandsExecuted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float BudgetUsed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float BudgetLimit;

    FDir_ProductionMetrics()
    {
        TotalCycles = 0;
        CompletedTasks = 0;
        FailedTasks = 0;
        AverageTaskDuration = 0.0f;
        FilesCreated = 0;
        UE5CommandsExecuted = 0;
        BudgetUsed = 0.0f;
        BudgetLimit = 100.0f;
    }
};

USTRUCT(BlueprintType)
struct FDir_AssetPipelineStatus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pipeline")
    bool bFBXImportReady;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pipeline")
    bool bTerrainExpanded;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pipeline")
    bool bAtmosphereStable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pipeline")
    int32 BiomesCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pipeline")
    int32 DinosaurActorsCreated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pipeline")
    bool bSurvivalHUDImplemented;

    FDir_AssetPipelineStatus()
    {
        bFBXImportReady = false;
        bTerrainExpanded = false;
        bAtmosphereStable = false;
        BiomesCompleted = 0;
        DinosaurActorsCreated = 0;
        bSurvivalHUDImplemented = false;
    }
};

/**
 * Studio Director System - Coordinates all 19 agents and manages production pipeline
 * Responsible for task distribution, progress tracking, and ensuring milestone delivery
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UStudioDirectorSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UStudioDirectorSystem();

    // Subsystem lifecycle
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Agent coordination functions
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeAgentTasks();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription, EDir_TaskPriority Priority);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CompleteAgentTask(const FString& AgentName, const FString& Output, float Duration);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentTask> GetAgentTasks() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_AgentTask GetAgentTask(const FString& AgentName) const;

    // Production metrics
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateProductionMetrics(int32 FilesCreated, int32 UE5Commands, float BudgetSpent);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_ProductionMetrics GetProductionMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void IncrementCycleCount();

    // Asset pipeline management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateAssetPipelineStatus(const FDir_AssetPipelineStatus& NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_AssetPipelineStatus GetAssetPipelineStatus() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool CheckMilestone1Criteria() const;

    // Coordination helpers
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CreateAgentTaskMarkers();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ValidateMinPlayableMap();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FString GenerateProductionReport() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void LogAgentActivity(const FString& AgentName, const FString& Activity);

protected:
    // Agent task management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director", meta = (AllowPrivateAccess = "true"))
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director", meta = (AllowPrivateAccess = "true"))
    TMap<FString, int32> AgentTaskIndices;

    // Production tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director", meta = (AllowPrivateAccess = "true"))
    FDir_ProductionMetrics ProductionMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director", meta = (AllowPrivateAccess = "true"))
    FDir_AssetPipelineStatus AssetPipelineStatus;

    // Activity logging
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director", meta = (AllowPrivateAccess = "true"))
    TArray<FString> ActivityLog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director", meta = (AllowPrivateAccess = "true"))
    int32 MaxLogEntries;

private:
    void InitializeDefaultAgents();
    void CleanupOldLogEntries();
    FString GetCurrentTimestamp() const;
};